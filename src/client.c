/*
FILENAME:   client.c
COURSE:     MREN 178

~ Group 1 ~

Beric Dengler            STUDENT ID: 20515669
James Colbourne          STUDENT ID: 20523893
Nicolas Kaye             STUDENT ID: 20506269
Lukas Nuzzi              STUDENT ID: 20524153

*/

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <string.h>

#include "DSAFunctions.h"
#include "networkHelpers.h"

#define SERVER_PORT "3000"
// #define SERVER_ADDRESS "10.218.56.137"
#define CMD_LEN 4 //dont change thes unless you are extremely aware of what the hell you are doing
#define LENBUFF_LEN 4 //this is the sent char array length of the length info. using atoi, this means a max length of 9999, which is inefficent but whatever
#define DELIMITER '\r' //delimiter to fit between packages sent on tcp

#define PULL_AMOUNT 20 // total pull amount for recieve messga latest
#define USERNAMEDATABASE_SIZE 20

#pragma comment(lib, "ws2_32")

Channel* getChannel(ChannelNameId id) {
    for (int i = 0; channels[i].channel_name != NULL; i++) {
        if (channels[i].channel_id == id)
            return &channels[i];
    }
    return NULL;
}

 //sends command, then the combinded message itself w username, channelid w -- inbetween, returns 1 on failure
int sendMessage(SOCKET *socket, Message *m_message) {
    if (socket == NULL || m_message == NULL || m_message->sender == NULL || m_message->channel == NULL) {
        return 1;
    }

    char *command = "#POM";

    int send_err = sendOnSock(socket, command, (int)strlen(command));
    if (send_err == 1) {
        return 1;
    }

    char *channel_string   = intToArray(m_message->channel->channel_id, LENBUFF_LEN);
    char *timestamp_string = intToArray(m_message->timestamp, LENBUFF_LEN);
    char *UUID_string      = intToArray(m_message->sender->UUID,LENBUFF_LEN);

    // order should be - 0 message, 1 timestamp, 2 user, 3 channel.
    char *msg_values[4] = {m_message->message, timestamp_string, UUID_string, channel_string};
    char *msg_packed = dataPackage(msg_values, 4, DELIMITER);

    if(msg_packed == NULL){
        free(channel_string);
        free(timestamp_string);
        free(UUID_string);
        return 1;
    }

    char *lenbuf = intToArray((int)strlen(msg_packed), LENBUFF_LEN);
    send_err = sendOnSock(socket, lenbuf, LENBUFF_LEN);
    free(lenbuf);
    if (send_err == 1){
        free(channel_string);
        free(timestamp_string);
        free(UUID_string);
        free(msg_packed);
        return 1;
    }

    send_err = sendOnSock(socket, msg_packed, (int)strlen(msg_packed));
    free(channel_string);
    free(timestamp_string);
    free(UUID_string);
    free(msg_packed);
    if (send_err == 1)
        return 1;
    return 0;
}

void freeMessages(Message *msgs, int count) {
    for (int i = 0; i < count; i++) {
        if (msgs[i].message != NULL) 
            free(msgs[i].message);
        if (msgs[i].sender != NULL) {
            if (msgs[i].sender->name != NULL) 
                free(msgs[i].sender->name);
            free(msgs[i].sender);
        }
    }
    free(msgs);
}

//it this will send back the latest like 20 messages along with their timestamps, who they were sent by, and their timestamp, given which channel it was sent. ALSO the return is the length of the array returned, w -1 being failure
int recieveMsgLatest(SOCKET *socket, int channel_id, Message **messages, int *messageCount){
    
    char *lms = "#LMS";
    int clie_err = sendOnSock(socket, lms, (int)strlen(lms));
    if (clie_err == 1)
        return -1;
    char *channel_str = intToArray(channel_id, LENBUFF_LEN);
    clie_err = sendOnSock(socket, channel_str, LENBUFF_LEN);
    free(channel_str);
    if (clie_err == 1)
        return -1;
    // recv count first
    char count_str[LENBUFF_LEN+1];
    clie_err = recvAll(*socket, count_str, LENBUFF_LEN);
    if(clie_err != LENBUFF_LEN) return -1;
    count_str[LENBUFF_LEN] = '\0';
    int count = atoi(count_str);
    
    if (*messages != NULL) {
        freeMessages(*messages, *messageCount);
        *messages = NULL;
    }
     
    if (count == 0) {
        *messages = NULL;
        *messageCount = 0;
        return 0;
    }

    *messageCount = count;
    
    *messages = calloc(count,sizeof(Message));

    //Adjust the count available to interface
    
    

    for(int i = 0; i < count; i++){
        int content_len;
        char lenbuff[LENBUFF_LEN+1];
        clie_err = recvAll(*socket, lenbuff, LENBUFF_LEN);
        if (clie_err != LENBUFF_LEN) return -1;
        lenbuff[LENBUFF_LEN] = '\0';
        content_len = atoi(lenbuff);
        char *buff = malloc(content_len + 1);
        if (buff == NULL) return -1;
        clie_err = recvAll(*socket, buff, content_len);
        if (clie_err != content_len){
            free(buff);
            return -1;
        }
        buff[content_len] = '\0';
        char **parts = dataParse(buff, DELIMITER);
        if(parts == NULL){
            free(buff);
            return -1;
        }
        // order should be - 0 message, 1 timestamp, 2 user, 3 channel.
        (*messages)[i].sender = malloc(sizeof(User)); //need to allocate a user first
        (*messages)[i].message = malloc(strlen(parts[0]) + 1);
        strcpy((*messages)[i].message, parts[0]);
        (*messages)[i].sender->name = malloc(strlen(parts[2]) + 1);
        strcpy((*messages)[i].sender->name, parts[2]);
        (*messages)[i].timestamp = atoi(parts[1]);
        free(buff);
        free(parts);
    }
    return count;
}

int initializeClient(SOCKET *out_socket, char* SERVER_ADDRESS) {
    WSADATA wsaData;
    SOCKET connectSocket = INVALID_SOCKET;

    struct addrinfo *localAddress = NULL, desiredAddress;
    struct sockaddr_in clientAddress;
    int client_addr_len = sizeof(clientAddress);

    // printf("WSA Startup\n");

    int clie_err = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (clie_err != 0) {
        printf("Client failed to start");
        return 1;
    }
    // printf("WSA Startup successful\n");

    ZeroMemory(&desiredAddress, sizeof(desiredAddress));
    desiredAddress.ai_family = AF_UNSPEC;
    desiredAddress.ai_socktype = SOCK_STREAM;
    desiredAddress.ai_protocol = IPPROTO_TCP;

    // printf("Getting address info\n");

    clie_err = getaddrinfo(SERVER_ADDRESS, SERVER_PORT, &desiredAddress, &localAddress);
    if(clie_err != 0) {
        closeSocket("Failed on port 3000", NULL, &connectSocket);
        return 1;
    }

    struct addrinfo *ptr = NULL;

    for(ptr=localAddress; ptr != NULL; ptr=ptr->ai_next) {
        connectSocket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (connectSocket == INVALID_SOCKET) {
            closeSocket("Socket error", NULL, &connectSocket);
            return 1;
        }

        clie_err = connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (clie_err == SOCKET_ERROR) {
            closesocket(connectSocket);
            connectSocket = INVALID_SOCKET;
        }
    }

    freeaddrinfo(localAddress);

    if (connectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    char *hsk = "#HSK";
    if (sendOnSock(&connectSocket, hsk, (int)strlen(hsk)))
    return 1;

    // wait for server handshake response
    char hsk_resp[CMD_LEN+1];
    recv(connectSocket, hsk_resp, CMD_LEN, 0);
    hsk_resp[CMD_LEN] = '\0';
    // printf("Server handshake response: %s\n", hsk_resp);

    *out_socket = connectSocket;
    return 0;
}

int sockShutdown(SOCKET connectSocket){
    char *ccn = "#CCN";
    if (sendOnSock(&connectSocket, ccn, (int)strlen(ccn)))
        return 1;

    int client_err = shutdown(connectSocket, SD_SEND);
    if (client_err == SOCKET_ERROR) {
        closeSocket("Shutdown failed", NULL, &connectSocket);
        return 1;
    }
    closeSocket("Normal close", NULL, &connectSocket);
    return 0;
}

int genUser(SOCKET *connectSocket, char* username){
    char *gac = "#GAC";
    char gac_resp[5];
    if (sendOnSock(connectSocket, gac, (int)strlen(gac)))
        return -1;

    char *length = intToArray((int)strlen(username),LENBUFF_LEN);
    int err_result = sendOnSock(connectSocket, length, LENBUFF_LEN);
    free(length);
    if(err_result == 1){
        printf("failed to gen user (length)");
        return -1;
    }

    err_result = sendOnSock(connectSocket, username, (int)strlen(username));
    if(err_result == 1){
        printf("failed to gen user (username)");
        return -1;
    }

    err_result = recv(*connectSocket, gac_resp, 4, 0);
    gac_resp[4] = '\0';
    if(err_result == SOCKET_ERROR){
        printf("failed to get result");
        return -1;
    }

    return atoi(gac_resp);
}