#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <string.h>

#include "netCommands.h"
#include "data.h"

#define SERVER_PORT "3000"
#define SERVER_ADDRESS "10.216.50.21"

#pragma comment(lib, "ws2_32")

void closeSocket(char *message, struct addrinfo *addr, SOCKET *socket) {
    printf("%s\n", message);
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
    printf("Bytes Sent: %ld\n", send_err);
    return 0;
}

 //sends command, then the combinded message itself w uuid, channelid w -- inbetween, returns 1 on failure
int sendMessage(SOCKET *socket, char *message, int timestamp, int user_id, int channel_id) {
    char *command = "#POM";
    char *id_string = intToArray(user_id);
    char *channel_string = intToArray(channel_id);
    char *timestamp_string = intToArray(timestamp);

    int send_err = sendOnSock(socket, command, (int)strlen(command));
    if (send_err == 1)
        return 1;
    //msg, timestmap size, uuid size, channel id size, and the 3 - s
    int len = strlen(message) + 4 + 4 + 4 + 3;
    char out[len];

    send_err = sendOnSock(socket, intToArray(len), (int)strlen(intToArray(len)));
    if (send_err == 1)
        return 1;

    //messy way to combine 3 char arrays into one, separating them with a '-' for ease of parsing on the other end
    int i = 0;
    memcpy(out + i, message, strlen(message)); 
    i += strlen(message);
    out[i++] = '-';
    memcpy(out + i, timestamp_string, 4); 
    i += 4;
    out[i++] = '-';
    memcpy(out + i, id_string, 4); 
    i += 4;
    out[i++] = '-';
    memcpy(out + i, channel_string, 4); 
    i += 4;

    send_err = sendOnSock(socket, out, len);
    if (send_err == 1)
        return 1;
    return 0;
}

//it this will send back the latest like 20 messages along with their timestamps, who they were sent by, and their timestamp, given which channel it was sent im
int recieveMsgLatest(SOCKET *socket, int channel_id, message* messages){ 
    message messages[20];
    char *lms = "#LMS";
    memset(messages, 0, sizeof(messages))
    for(int i = 0; i<20; i++){
        int uuid, timestamp, content_len;
        char* uuid_str, timestamp_str, content_len_str,content;

        int clie_err = sendOnSock(socket, lms, (int)strlen(lms)) //sends code ofr latest messages
        if clie_err
            return 1;

        clie_err = recv(socket,uuid_str,4,0);
        if clie_err
            return 1;
        uuid = atoi(uuid_str);

        clie_err = recv(socket,timestamp_str,4,0);
        if clie_err
            return 1;
        timestamp = atoi(timestamp_str);
        
        clie_err = recv(socket,content_len_str,4,0);
        if clie_err
            return 1;
        content_len = atoi(content_len_str);
        
        clie_err = recv(socket,content,content_len,0);
        if clie_err
            return 1;

        messages[i].message = 
    }
    return 0;
}

int main() {
    WSADATA wsaData;
    SOCKET connectSocket = INVALID_SOCKET;

    struct addrinfo *localAddress = NULL, desiredAddress;
    struct sockaddr_in clientAddress;
    int client_addr_len = sizeof(clientAddress);

    char recvbuf[4];
    int recvbuflen = 4;

    printf("WSA Startup\n");

    int clie_err = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (clie_err != 0) {
        printf("Client failed to start");
        return 1;
    }
    printf("WSA Startup successful\n");

    ZeroMemory(&desiredAddress, sizeof(desiredAddress));
    desiredAddress.ai_family = AF_UNSPEC;
    desiredAddress.ai_socktype = SOCK_STREAM;
    desiredAddress.ai_protocol = IPPROTO_TCP;

    printf("Getting address info\n");

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

    char *ccn = "#CCN";
    char *hsk = "#HSK";

    if (sendOnSock(&connectSocket, hsk, (int)strlen(hsk)))
        return 1;

    if (sendMessage(&connectSocket, "runnning", 6, 4, 9))
        return 1;

    if (sendOnSock(&connectSocket, ccn, (int)strlen(ccn)))
        return 1;

    clie_err = shutdown(connectSocket, SD_SEND);
    if (clie_err == SOCKET_ERROR) {
        closeSocket("Shutdown failed", NULL, &connectSocket);
        return 1;
    }

    closeSocket("Normal close", NULL, &connectSocket);
    return 0;
}  