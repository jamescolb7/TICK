#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <string.h>

#include "netData.h"
#include "DSAFunctions.h"

#define SERVER_PORT "3000"
// #define SERVER_ADDRESS "10.218.56.137"

#pragma comment(lib, "ws2_32")

void closeSocket(char *message, struct addrinfo *addr, SOCKET *socket) {
    // printf("%s\n", message);
    if (addr != NULL) freeaddrinfo(addr);
    if (socket != NULL) closesocket(*socket);
    WSACleanup();
}

char* intToArray(int number)
{
    char *numberArray = calloc(4, sizeof(char));
    for (int i = 3; i >= 0; --i){
        numberArray[i] = (number % 10) + '0';
        number /= 10;
    }
    return numberArray;
}

int sendOnSock(SOCKET *socket, char *message, int len) {
    int send_err = send(*socket, message, len, 0);
    if (send_err == SOCKET_ERROR) {
        closeSocket("Send failed", NULL, socket);
        return 1;
    }
    // printf("Bytes Sent: %ld\n", send_err);
    return 0;
}

Channel* getChannel(ChannelNameId id) {
    for (int i = 0; channels[i].channel_name != NULL; i++) {
        if (channels[i].channel_id == id)
            return &channels[i];
    }
    return NULL;
}

 //sends command, then the combinded message itself w username, channelid w -- inbetween, returns 1 on failure
int sendMessage(SOCKET *socket, Message* m_message) {
    char *command = "#POM";
    int channel_id = m_message->channel->channel_id;
    int timestamp = m_message->timestamp;
    int UUID = m_message->sender->UUID;

    char *message = m_message->message;
    char *channel_string = intToArray(channel_id);
    char *timestamp_string = intToArray(timestamp);
    char *UUID_string = intToArray(UUID);

    int send_err = sendOnSock(socket, command, (int)strlen(command));
    if (send_err == 1){
        free(channel_string);
        free(timestamp_string);
        free(UUID_string);
        return 1;
    }
    //msg, timestmap size, user size, channel id size, and the 3 - s
    int len = strlen(message) + 4 + 4 + 4 + 3;
    char out[len];

    char *lenbuf = intToArray(len);
    send_err = sendOnSock(socket, lenbuf, 4);
    free(lenbuf);
    if (send_err == 1){
        free(channel_string);
        free(timestamp_string);
        free(UUID_string);
        return 1;
    }
        

    //messy way to combine 3 char arrays into one, separating them with a '-' for ease of parsing on the other end
    int i = 0;
    memcpy(out + i, message, strlen(message));
    i += strlen(message);
    out[i++] = '-';
    memcpy(out + i, timestamp_string, 4);
    i += 4;
    out[i++] = '-';
    memcpy(out + i, UUID_string, 4);
    i += 4;
    out[i++] = '-';
    memcpy(out + i, channel_string, 4);
    i += 4;

    send_err = sendOnSock(socket, out, len);
    free(channel_string);
    free(timestamp_string);
    free(UUID_string);
    if (send_err == 1)
        return 1;
    return 0;
}

//it this will send back the latest like 20 messages along with their timestamps, who they were sent by, and their timestamp, given which channel it was sent im
int recieveMsgLatest(SOCKET *socket, int channel_id, ScreenMessage *messages){
    char *lms = "#LMS";
    memset(messages, 0, sizeof(ScreenMessage) * 10);
    int clie_err = sendOnSock(socket, lms, (int)strlen(lms));
    if (clie_err == 1)
        return 1;

    char *channel_str = intToArray(channel_id);
    clie_err = sendOnSock(socket, channel_str, 4);
    free(channel_str);
    if (clie_err == 1)
        return 1;

    // recv count first
    char count_str[5];
    clie_err = recv(*socket, count_str, 4, 0);
    if(clie_err == SOCKET_ERROR) return 1;
    count_str[4] = '\0';
    int count = atoi(count_str);

    for(int i = 0; i < count; i++){
        int content_len;
        char content_len_str[5];

        clie_err = recv(*socket, content_len_str, 4, 0);
        if (clie_err == SOCKET_ERROR) return 1;
        if (clie_err == 0) return 0;
        content_len_str[4] = '\0';
        content_len = atoi(content_len_str);

        char *buff = malloc(content_len + 1);
        clie_err = recv(*socket, buff, content_len, 0);
        if (clie_err == SOCKET_ERROR){
            free(buff);
            return 1;
        }
        buff[content_len] = '\0';

        char *msg_part       = strtok(buff, "-");
        char *timestamp_part = strtok(NULL, "-");
        char *uuid_part      = strtok(NULL, "-");

        messages[i].message   = malloc(strlen(msg_part) + 1);
        strcpy(messages[i].message, msg_part);
        messages[i].username  = "Test";
        messages[i].timestamp = 0;
        free(buff);
    }

    // Fill remaining empty slots with empty structures
    for(int i = count; i < 10; i++){
        messages[i].message = malloc(1);
        messages[i].message[0] = '\0';
        messages[i].username = malloc(1);
        messages[i].username[0] = '\0';
        messages[i].timestamp = 0;
    }

    return 0;
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
    char hsk_resp[5];
    recv(connectSocket, hsk_resp, 4, 0);
    hsk_resp[4] = '\0';
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

    char *length = intToArray((int)strlen(username));
    int err_result = sendOnSock(connectSocket, length, 4);
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


    
    
    
    // User *temp_user = malloc(sizeof(User));
    // temp_user->UUID = 8;
    // temp_user->name = NULL;

    // Message* temp_message = initMessage("penis", temp_user, 6, getChannel(GENERAL));

    // for(int i = 0; i < 40; i++){
    //     printf("Sending message %d\n", i);
    //     if(sendMessage(&connectSocket, temp_message)){
    //         printf("sendMessage failed on iteration %d\n", i);
    //         break;
    //     }
    // }

    // Message messages[20];

    // clie_err = recieveMsgLatest(&connectSocket, GENERAL, messages);
    // if(clie_err != 0){
    //     printf("something bad happened\n");
    // }

    // printf("||%s||\n", messages[6].message);
