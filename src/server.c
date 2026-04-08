#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <stdbool.h>

#include "DSAFunctions.h"
#include "networkHelpers.h"

#define SERVER_PORT "3000"
#define SERVER_ADDRESS "0.0.0.0"
#define CMD_LEN 4 //dont change thes unless you are extremely aware of what the hell you are doing
#define LENBUFF_LEN 4 //this is the sent char array length of the length info. using atoi, this means a max length of 9999, which is inefficent but whatever
#define DELIMITER '-' //delimiter to fit between packages sent on tcp

#define PULL_AMOUNT 20 // total pull amount for recieve messga latest
#define USERNAMEDATABASE_SIZE 20

#pragma comment(lib, "ws2_32")

int num_of_users = 0;

//A function to get the command type from a string using the hasmap seen in neworkcommands.h
CommandType get_command(const char *input) {
    for (int i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
        if (strcmp(commands[i].command_str, input) == 0)
            return commands[i].command_type;
    }
    return UNKNOWN;
}

Channel* getChannel(ChannelNameId id) {
    for (int i = 0; channels[i].channel_name != NULL; i++) {
        if (channels[i].channel_id == id)
            return &channels[i];
    }
    return NULL;
}

//simple handshake using the protocol to ensure the client connected is actually a client
int handshakeServ(SOCKET clientSocket){
    char command[CMD_LEN+1]; // 4 chars + null terminator for safety
    int recv_err = recv(clientSocket, command, CMD_LEN, 0);
    if(recv_err == SOCKET_ERROR){
        printf("Socket error occurred while receiving handshake\n");
        return 1;
        
    }
    command[CMD_LEN] = '\0';
    if(get_command(command) == HANDSHAKE){
        printf("Handshake received from client, repsonding\n");
        send(clientSocket, "#HSK", CMD_LEN, 0);
    } 
    else {
        printf("Invalid command received from client - %s is not valid\n", command);
        return 1;
    }
    return 0;
}

//recvives the post message command to put into buffer.
int postMessage(CommandType *cmd_type, SOCKET clientSocket, User* username_database, Tree* BSTUser){
    printf("Post message command received\n");

    char lenbuf[LENBUFF_LEN+1]; // 4 chars for length + null terminator
    int msglen;
    int recv_err = recv(clientSocket, lenbuf, LENBUFF_LEN, 0);

    if(recv_err == SOCKET_ERROR){
        printf("Socket error occurred while receiving message, forcing close\n");
        printf("WSA Error: %d\n", WSAGetLastError());
        *cmd_type = CLOSE_CONNECTION;
        return 1;
    } 
    else if(recv_err == 0){
        printf("Connection closed by client while receiving message\n");
        *cmd_type = CLOSE_CONNECTION;
        return 1;
    }

    lenbuf[LENBUFF_LEN] = '\0';
    msglen = atoi(lenbuf);
    char *msgbuf = malloc(msglen + 1);
    recv_err = recv(clientSocket, msgbuf, msglen, 0);
    if(recv_err == SOCKET_ERROR){
        printf("Socket error occurred while receiving message, forcing close\n");
        printf("WSA Error: %d\n", WSAGetLastError());
        *cmd_type = CLOSE_CONNECTION;
        free(msgbuf);
        return 1;
    } 

    else if(recv_err == 0){
        printf("Connection closed by client while receiving message\n");
        *cmd_type = CLOSE_CONNECTION;
        free(msgbuf);
        return 1;
    }
    msgbuf[msglen] = '\0';
    printf("Raw received: %s\n", msgbuf);
    // parse out the parts
    char **msg_parts = dataParse(msgbuf, DELIMITER);
    if(msg_parts == NULL){
        printf("Failed to parse message parts!\n");
        free(msgbuf);
        return 1;
    }

    printf("parts: msg='%s' ts='%s' user='%s' channel='%s'\n",
    msg_parts[0], msg_parts[1], msg_parts[2], msg_parts[3]);

    //order should be - 0 message, 1 timestamp, 2 user, 3 channel.
    int key = findUser(BSTUser->root, atoi(msg_parts[2]));
    if(key == -1){ //cannot find user meaning invalid user sent message. (security kinda)
        printf("message sent by invalid user! \n");
        free(msgbuf);
        free(msg_parts);
        return 1;
    }
    //creatiung temp user to pass through as amessage
    User *temp_user = malloc(sizeof(User));
    temp_user->UUID = atoi(msg_parts[2]);
    temp_user->name = username_database[key].name;

    printf("User defined\n");

    char *msg_copy = malloc(strlen(msg_parts[0]) + 1);
    strcpy(msg_copy, msg_parts[0]);
    
    //makes message
    Channel *curr_channel = getChannel(atoi(msg_parts[3]));
    if(curr_channel == NULL){
        printf("channel returned null, exiting\n");
        free(msg_copy);
        free(temp_user);
        free(msg_parts);
        free(msgbuf);
        return 1;
    }
    Message *new_message = initMessage(msg_copy, temp_user, atoi(msg_parts[1]), curr_channel);
    if(new_message == NULL){
        printf("init message failed, exiting\n");
        free(msg_copy);
        free(temp_user);
        free(msg_parts);
        free(msgbuf);
        return 1;
    }
    Node *ennode = initNode(new_message);
    if(ennode == NULL){
        printf("initNode failed, exiting\n");
        free(msg_copy);
        free(temp_user);
        free(msg_parts);
        free(msgbuf);
        return 1;
    }
    printf("Enque starting\n");
    int q_err = enqueue(curr_channel->deque, ennode);
    if(q_err == 0)
        printf("queue failed\n");
    free(msg_parts);
    free(msgbuf);
    return 0;
}

int genUserBST(User* username_database, Tree** BSTUser){
    User admin;
    admin.UUID = 5000;
    admin.name = "admin";

    username_database[0] = admin;
    *BSTUser = initTree(5000, 0);
    num_of_users++;

    return 0;
}

int genUserHelper(char* username, User* username_database, Tree* BSTUser){
    User* new_user = malloc(sizeof(User));
    new_user->name = username;

    int retval;
    int random_num; 
    //uses random values to find a valid UUID that is NOT taken in the BST user array. this gets more and more inefficent to generate new users - but as a tradeoff is the best method we have so far. 
    do {
        random_num = rand() % 10000;
        retval = findUser(BSTUser->root, random_num);
    } while(retval != -1);

    new_user->UUID = random_num;

    username_database[num_of_users] = *new_user;

    num_of_users++;
    TreeNode* tree_node = initTreenode(new_user->UUID, num_of_users);
    insert(BSTUser->root, tree_node);

	printf("made a new user! - @ UUID %d\n",new_user->UUID);

    return new_user->UUID;
}

int genUser(CommandType* cmd_type, SOCKET clientSocket,Tree* BSTUser, User* username_database){
    printf("Account requested to generate, generating...\n");

    char lenbuf[LENBUFF_LEN+1]; // 4 chars for length + null terminator
    int msglen;

    recv(clientSocket, lenbuf, LENBUFF_LEN, 0);
    lenbuf[LENBUFF_LEN] = '\0';
    msglen = atoi(lenbuf);
    char *gac_buf = malloc(msglen + 1);
    int recv_err = recv(clientSocket, gac_buf, msglen, 0);

    //Error handle conditions for failure of socket recv
    if(recv_err == SOCKET_ERROR){
        printf("Socket error occurred while receiving message, forcing close\n");
        printf("WSA Error: %d\n", WSAGetLastError());
        *cmd_type = CLOSE_CONNECTION;
        free(gac_buf);
        return 1;
    } 
    else if(recv_err == 0){
        printf("Connection closed by client while receiving message\n");
        printf("WSA Error: %d\n", WSAGetLastError());
        *cmd_type = CLOSE_CONNECTION;
        free(gac_buf);
        return 1;
    } 

    else {
        gac_buf[msglen] = '\0';
        printf("Raw received: %s\n", gac_buf);
        int new_UUID = genUserHelper(gac_buf, username_database, BSTUser);
        char *uuid_buf = intToArray(new_UUID,LENBUFF_LEN);
        int err_res = sendOnSock(&clientSocket, uuid_buf, LENBUFF_LEN);
        free(uuid_buf);
        if(err_res == 1){
            printf("issue sending UUID\n");
            free(gac_buf);
            return 1;
        }
    }
    free(gac_buf);
    return 0;
}

int latestMessages(CommandType* cmd_type, SOCKET clientSocket, Tree* BSTUser, User* username_database){
    printf("Latest messages command received\n");
    char lenbuf[LENBUFF_LEN+1]; // 4 chars for length + null terminator
    //recives channel id
    int recv_err = recv(clientSocket, lenbuf, LENBUFF_LEN, 0);
    if(recv_err == SOCKET_ERROR){
        printf("Socket error occurred while receiving message, forcing close\n");
        printf("WSA Error: %d\n", WSAGetLastError());
        *cmd_type = CLOSE_CONNECTION;
        return 1;
    }
    lenbuf[LENBUFF_LEN] = '\0';
    int channel_id = atoi(lenbuf);
    Channel *req_channel = getChannel(channel_id);
    if(req_channel == NULL || req_channel->deque == NULL){
        printf("Channel or deque not found for id %d\n", channel_id);
        // send 0 count so client doesn't block
        char *zero = intToArray(0, LENBUFF_LEN);
        sendOnSock(&clientSocket, zero, LENBUFF_LEN);
        free(zero);
        return 1;
    }
    // count messages first
    int len = PULL_AMOUNT;
    Message **message_array = readallF(req_channel->deque->head, &len);
    char *count_buf = intToArray(len, LENBUFF_LEN);
    sendOnSock(&clientSocket, count_buf, LENBUFF_LEN);
    free(count_buf);
    if(message_array == NULL){
        printf("Failed to read messages from deque - empty queue! (0 message history)\n");
        return 0;
    }
    Node *current = req_channel->deque->head;
    printf("Only %i found, sending to client\n", len);
    for(int i = 0; i < len; i++){
        char *channel_string   = intToArray(message_array[i]->channel->channel_id,   LENBUFF_LEN);
        char *timestamp_string = intToArray(message_array[i]->timestamp, LENBUFF_LEN);
        char *UUID_string      = username_database[findUser(BSTUser->root, message_array[i]->sender->UUID)].name;
        char *msg_string       = message_array[i]->message;
        // order should be - 0 message, 1 timestamp, 2 user, 3 channel.
        char *msg_values[4] = {msg_string, timestamp_string, UUID_string, channel_string};
        char *msg_packed = dataPackage(msg_values, 4, DELIMITER);
        if(msg_packed == NULL){
            printf("Message packaging error, exiting packaging!\n");
            free(channel_string);
            free(timestamp_string);
            free(message_array);
            return 1;
        }
        //needs to send length first
        char *msglen_buf = intToArray((int)strlen(msg_packed), LENBUFF_LEN);
        recv_err = sendOnSock(&clientSocket, msglen_buf, LENBUFF_LEN);
        if(recv_err == 1){
            printf("Socket error on msglen_buf for client pull.\n");
            free(msglen_buf);
            free(msg_packed);
            free(channel_string);
            free(timestamp_string);
            free(message_array);
            return 1;
        }
        recv_err = sendOnSock(&clientSocket, msg_packed, (int)strlen(msg_packed));
        if(recv_err == 1){
            printf("Socket error on Message data for client pull.\n");
            free(msglen_buf);
            free(msg_packed);
            free(channel_string);
            free(timestamp_string);
            free(message_array);
            return 1;
        }
        free(msglen_buf);
        free(msg_packed);
        free(channel_string);
        free(timestamp_string);
    }
    free(message_array);
    return 0;
}

//a simple definition func to be run to intialize the channels. Note that they are currently fixed, something that can be changed throughout development
void channel_init(){
    getChannel(GENERAL)->deque = initDeque();
    getChannel(CHILL)->deque = initDeque();
    getChannel(MEMES)->deque = initDeque();
    getChannel(QUOTES)->deque = initDeque();
    getChannel(NEWS)->deque = initDeque();
    getChannel(OFF_TOPIC)->deque = initDeque();
}

int WSAServInit(WSADATA wsaData, SOCKET *listenSocket){
    printf("WSA Startup\n");
    int servErr = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (servErr != 0) {
        printf("Server failed to start");
        return 1;
    }
    printf("WSA Startup successful\n");

    struct addrinfo *localAddress = NULL, desiredAddress;

    ZeroMemory(&desiredAddress, sizeof(desiredAddress));
    desiredAddress.ai_family   = AF_INET;
    desiredAddress.ai_socktype = SOCK_STREAM;
    desiredAddress.ai_protocol = IPPROTO_TCP;
    desiredAddress.ai_flags    = AI_PASSIVE;

    printf("Getting address info\n");

    servErr = getaddrinfo(NULL, SERVER_PORT, &desiredAddress, &localAddress);
    if(servErr != 0) {
        closeSocket("Failed on port 3000", NULL, listenSocket);
        return 1;
    }

    printf("Address info valid\n");
    printf("Creating listening socket\n");

    *listenSocket = socket(localAddress->ai_family, localAddress->ai_socktype, localAddress->ai_protocol);
    if(*listenSocket == INVALID_SOCKET){
        closeSocket("Invalid Socket", NULL, NULL);
        return 2;
    }

    printf("Listening socket created\n");
    printf("Binding on port %s\n", SERVER_PORT);

    servErr = bind(*listenSocket, localAddress->ai_addr, (int)localAddress->ai_addrlen);
    if(servErr == SOCKET_ERROR){
        closeSocket("Failed to bind", localAddress, listenSocket);
        return 1;
    }

    printf("Bind successful\n");
    freeaddrinfo(localAddress);
    return 0;
}



int main() {
    channel_init();
    User username_database[USERNAMEDATABASE_SIZE];
    Tree* BSTUser;
    genUserBST(username_database, &BSTUser);

    WSADATA wsaData;
    SOCKET listenSocket = INVALID_SOCKET, clientSocket = INVALID_SOCKET;

    struct sockaddr_in clientAddress;
    int client_addr_len = sizeof(clientAddress);

    if (WSAServInit(wsaData, &listenSocket)){
        printf("WSA Intilaization Failed\n");
        return 1;
    }


    printf("Listening on port %s\n", SERVER_PORT);
    int servErr = listen(listenSocket, SOMAXCONN);
    if(servErr == SOCKET_ERROR){
        closeSocket("Failed to find client on socket", NULL, &listenSocket);
        return 1;
    }

    do {
        printf("Waiting for client on port %s\n", SERVER_PORT);
        clientSocket = accept(listenSocket, (struct sockaddr*)&clientAddress, &client_addr_len);
        if(clientSocket == INVALID_SOCKET){
            printf("Client socket is invalid\n");
            continue;
        }

        printf("A Client has connected with address: %s\n", inet_ntoa(clientAddress.sin_addr));

        if(handshakeServ(clientSocket)){
            printf("Retrying...\n");
            continue;
        }

        printf("Connection valid! Entering command loop\n");
        CommandType cmd_type;
        int i = 0;
        do {
            char command[CMD_LEN+1]; // 
            int recv_err = recv(clientSocket, command, CMD_LEN, 0);
            if(recv_err == SOCKET_ERROR){
                printf("Socket error occurred while receiving command, forcing close\n");
                printf("WSA Error: %d\n", WSAGetLastError());
                break;
            }
            if(recv_err == 0){
                printf("Connection closed by client\n");
                cmd_type = CLOSE_CONNECTION;
                continue;
            }
            command[CMD_LEN] = '\0'; // null-terminate the received command string
            cmd_type = get_command(command);
            switch(cmd_type) {
                case POST_MSG:
                    if(postMessage(&cmd_type,clientSocket,username_database,BSTUser))
                        printf("Posting Message Error\n");
                    break;
                case LATEST_MESSAGES:
                    if(latestMessages(&cmd_type,clientSocket,BSTUser,username_database))
                        printf("Latest Messages Error\n");
                    break;
                case GEN_ACC:
                    if(genUser(&cmd_type,clientSocket,BSTUser,username_database))
                        printf("Generation Account Error\n");
                    break;
                case CLOSE_CONNECTION:
                    printf("Close connection command received, closing client socket\n");
                    closesocket(clientSocket);
                    break;
                case UNKNOWN:
                    printf("Null command received, LIKELY DEAD, exiting\n");
                    cmd_type = CLOSE_CONNECTION;
                    break;
                default:
                    printf("Invalid command received from client '%s' must be a valid command\n", command);
                    i++;
                    if(i > 5){
                        printf("Too many invalid commands received, closing connection\n");
                        cmd_type = CLOSE_CONNECTION;
                    }
                    Sleep(500);
            }
        } while(cmd_type != CLOSE_CONNECTION);

        printf("Returning to listening\n");

    } while (true);

    closeSocket("Closing server connection!\n", NULL, &listenSocket);

    return 0;
}