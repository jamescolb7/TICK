#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <stdbool.h>

#include "netData.h"
#include "DSAFunctions.h"
#include "networkHelpers.h"

#define SERVER_PORT "3000"
#define SERVER_ADDRESS "0.0.0.0"
#define CMD_LEN 4 //dont change thes unless you are extremely aware of what the hell you are doing
#define LENBUFF_LEN 4 //this is the sent char array length of the length info. using atoi, this means a max length of 9999, which is inefficent but whatever

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
        send(clientSocket, "#HSK", 4, 0);
    } 
    else {
        printf("Invalid command received from client - %s is not valid\n", command);
        return 1;
    }
}

//recvives the post message command to put into buffer.
int postMessage(CommandType *cmd_type, SOCKET clientSocket){
    printf("Post message command received\n");

    char lenbuf[LENBUFF_LEN+1]; // 4 chars for length + null terminator
    int msglen;

    int recv_err = recv(clientSocket, lenbuf, LENBUFF_LEN, 0);
    if(recv_err == SOCKET_ERROR){
        printf("Socket error occurred while receiving message, forcing close\n");
        printf("WSA Error: %d\n", WSAGetLastError());
        cmd_type = CLOSE_CONNECTION;
    } else if(recv_err == 0){
        printf("Connection closed by client while receiving message\n");
        cmd_type = CLOSE_CONNECTION;
    }

    lenbuf[LENBUFF_LEN] = '\0';
    msglen = atoi(lenbuf);
    char *msgbuf = malloc(msglen + 1);
    recv_err = recv(clientSocket, msgbuf, msglen, 0);
    if(recv_err == SOCKET_ERROR){
        printf("Socket error occurred while receiving message, forcing close\n");
        printf("WSA Error: %d\n", WSAGetLastError());
        cmd_type = CLOSE_CONNECTION;
    } else if(recv_err == 0){
        printf("Connection closed by client while receiving message\n");
        cmd_type = CLOSE_CONNECTION;
    } else {
        msgbuf[msglen] = '\0';
        printf("Raw received: %s\n", msgbuf);

        // parse out the parts
        char *msg_part,tmp_part,usr_part,cnl_part;
        dataParse(msgbuf, msg_part, tmp_part, usr_part, cnl_part);

        //creatiung temp user to pass through as amessage
        User *temp_user = malloc(sizeof(User));
        temp_user->UUID = atoi(username_part);
        int key = findUser(BSTUser->root, atoi(username_part))
        if(key == -1){ //cannot find user meaning invalid user sent message. (security kinda)
            printf("message sent by invalid user! \n")
            return 1;
        }
        temp_user->name = username_database[key].name;

        printf("User defined\n");

        char *msg_copy = malloc(strlen(msg_part) + 1);
        strcpy(msg_copy, msg_part);

        //makes message
        Channel *curr_channel = getChannel(atoi(channel_part));
        Message *new_message  = initMessage(msg_copy, temp_user, atoi(timestamp_part), curr_channel);
        Node *ennode          = initNode(new_message);

        printf("Enque starting\n");

        int q_err = enqueue(curr_channel->deque, ennode);
        if(q_err == 0)
            printf("queue failed\n");
    }
    free(msgbuf);
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

    recv(clientSocket, lenbuf, 4, 0);
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
        char *uuid_buf = intToArray(new_UUID);
        int err_res = sendOnSock(&clientSocket, uuid_buf, 4);
        free(uuid_buf);
        if(err_res == 1){
            printf("issue sending UUID\n");
            free(gac_buf);
            return 1;
        }
    }
    free(gac_buf);
}

int latestMessage(CommandType cmd_type,SOCKET clientSocket,Tree* BSTUser, User* username_database){
    printf("Latest messages command received\n");

    char lenbuf[LENBUFF_LEN+1]; // 4 chars for length + null terminator
    int msglen;

    int recv_err = recv(clientSocket, lenbuf, 4, 0);
    lenbuf[4] = '\0';
    int channel_id = atoi(lenbuf);
    if(recv_err == SOCKET_ERROR){
        printf("Socket error occurred while receiving message, forcing close\n");
        printf("WSA Error: %d\n", WSAGetLastError());
        cmd_type = CLOSE_CONNECTION;
        return 1;
    }
    Channel *req_channel = getChannel(channel_id);
    if(req_channel == NULL || req_channel->deque == NULL){
        printf("Channel or deque not found for id %d\n", channel_id);
        // send 0 count so client doesn't block
        char *zero = intToArray(0);
        sendOnSock(&clientSocket, zero, 4);
        free(zero);
        return 1;
    }
    // count messages first
    int msg_count = 0;
    Node *counter = req_channel->deque->head;
    while(counter != NULL && msg_count < 10){
        msg_count++;
        counter = counter->pright;
    }
    char *count_buf = intToArray(msg_count);
    sendOnSock(&clientSocket, count_buf, 4);
    free(count_buf);

    Node *current = req_channel->deque->head;
    int sent = 0;
    while(current != NULL && sent < 10){
        Message *msg = current->mess;
        if(msg == NULL) break;
        int channel_id = msg->channel->channel_id;
        int timestamp = msg->timestamp;
        int UUID = msg->sender->UUID;

        char *channel_string = intToArray(channel_id);
        char *timestamp_string = intToArray(timestamp);
        char *UUID_string = username_database[findUser(BSTUser->root,UUID)].name;
        

        int len = strlen(msg->message) + strlen(UUID_string) + 4 + 4 + 3;
        char *out = malloc(len);
        //messy way to combine 3 char arrays into one, separating them with a '-' for ease of parsing on the other end
        int i = 0;
        memcpy(out + i, msg->message, strlen(msg->message));
        i += strlen(msg->message);
        out[i++] = '-';
        memcpy(out + i, timestamp_string, 4);
        i += 4;
        out[i++] = '-';
        memcpy(out + i, UUID_string, strlen(UUID_string));
        i += strlen(UUID_string);
        
        char *msglen_buf = intToArray((int)strlen(out));
        recv_err = sendOnSock(&clientSocket, msglen_buf, 4);
        if(recv_err == 1) break;
        recv_err = sendOnSock(&clientSocket, msg->message, (int)strlen(out));
        if(recv_err == 1) break;
        free(msglen_buf);
        free(out);
        current = current->pright;
        sent++;
    }
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

int WSASServInit(WSADATA wsaData, SOCKET listenSocket){
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
        closeSocket("Failed on port 3000", NULL, &listenSocket);
        return 1;
    }

    printf("Address info valid\n");
    printf("Creating listening socket\n");

    listenSocket = socket(localAddress->ai_family, localAddress->ai_socktype, localAddress->ai_protocol);
    if(listenSocket == INVALID_SOCKET){
        closeSocket("Invalid Socket", NULL, NULL);
        return 2;
    }

    printf("Listening socket created\n");
    printf("Binding on port %s\n", SERVER_PORT);

    servErr = bind(listenSocket, localAddress->ai_addr, (int)localAddress->ai_addrlen);
    if(servErr == SOCKET_ERROR){
        closeSocket("Failed to bind", localAddress, &listenSocket);
        return 1;
    }

    printf("Bind successful\n");
    freeaddrinfo(localAddress);
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

    if (WSAServInit(wsaData, listenSocket)){
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
                    if(postMessage(&cmd_type))
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