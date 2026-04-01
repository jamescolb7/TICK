#define WIN32_LEAN_AND_MEAN


#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>

#include "netCommands.h"

#define SERVER_PORT "3000"
#define SERVER_ADDRESS "0.0.0.0"
#define CMD_LEN 4

#pragma comment(lib, "ws2_32")
//#pragma comment(lib, "lWs2_32")

//A function to report an error message and close the sockets and addresses if applicable, while also printing an error message
void closeSocket(char *message, struct addrinfo *addr, SOCKET *socket) {
	printf("%s\n", message);
	if (addr != NULL) freeaddrinfo(addr);
	if (socket != NULL) closesocket(*socket);
	WSACleanup();
}


//A function to get the command type from a string using the hasmap seen in neworkcommands.h
CommandType get_command(const char *input) {
    for (int i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
        if (strcmp(commands[i].command_str, input) == 0)
            return commands[i].command_type;
    }
    return UNKNOWN;
}

int main() {
  	WSADATA wsaData;
  	SOCKET listenSocket = INVALID_SOCKET, clientSocket = INVALID_SOCKET;

	struct addrinfo *localAddress = NULL, desiredAddress;
	struct sockaddr_in clientAddress;
	int client_addr_len = sizeof(clientAddress);

	printf("WSA Startup\n");

	int servErr = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (servErr != 0) {
		printf("Server failed to start");
    return 1;
	}
	printf("WSA Startup successful\n");

	ZeroMemory(&desiredAddress, sizeof(desiredAddress));
	desiredAddress.ai_family = AF_INET;
	desiredAddress.ai_socktype = SOCK_STREAM;
	desiredAddress.ai_protocol = IPPROTO_TCP;
	desiredAddress.ai_flags = AI_PASSIVE;

	printf("Getting address info\n");

	servErr = getaddrinfo(NULL, SERVER_PORT,&desiredAddress,&localAddress);
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

	printf("Listening on port %s\n", SERVER_PORT);
	servErr = listen(listenSocket, SOMAXCONN);
	if(servErr == SOCKET_ERROR){
		closeSocket("Failed to find client on socket", NULL, &listenSocket);
		return 1;
	}
	
	do {

		printf("Waiting for client on port %s\n", SERVER_PORT);
		
		clientSocket = accept(listenSocket, (struct sockaddr*)&clientAddress, &client_addr_len);
		if(clientSocket == INVALID_SOCKET){
			printf("Client socket is invalid\n");
			Sleep(1000);
			continue;
		}

		printf("A Client has connected with address: %s\n", inet_ntoa(clientAddress.sin_addr));

		char command[5]; // 4 chars + null terminator for safety	
		int recv_err = recv(clientSocket, (char*)&command, sizeof(HANDSHAKE), 0);
		if(recv_err == SOCKET_ERROR){
			printf("Socket error occurred while receiving handshake, retrying...\n");
			Sleep(1000);
			continue;
		}
		command[4] = '\0';
		if(get_command(command) == HANDSHAKE){
			printf("Handshake received from client, repsonding\n");
			send(clientSocket, "#HSK", 4, 0);
		} 

		else {
			printf("Invalid command received from client - %s is not valid\n", command);
			Sleep(1000);
			continue;
		}

		printf("Connection valid! Entering command loop\n");
		CommandType cmd_type;
		int i = 0;
		do{
			recv_err = recv(clientSocket, (char*)&command, CMD_LEN, 0);
			if(recv_err == SOCKET_ERROR){
				printf("Socket error occurred while receiving command, forcing close\n");
				printf("WSA Error: %d\n", WSAGetLastError());
				break;
			}
			if(recv_err == 0){
				printf("Connection closed by client\n");
				cmd_type = CLOSE_CONNECTION;
			}
			command[4] = '\0'; // null-terminate the received command string
			cmd_type = get_command(command);
			switch(cmd_type) {
				case POST_MSG:
					printf("Post message command received\n");
					char lenbuf[5]; // 4 chars for length + null terminator
					recv(clientSocket, lenbuf, 4, 0);
					int msglen = atoi(lenbuf);
					lenbuf[4] = '\0';
					char *msgbuf = malloc(msglen + 1);  // allocate ONCE
    				recv_err = recv(clientSocket, msgbuf, msglen, 0);
					if(recv_err == SOCKET_ERROR){
						printf("Socket error occurred while receiving message, forcing close\n");
						printf("WSA Error: %d\n", WSAGetLastError());
						cmd_type = CLOSE_CONNECTION;
					}
					else if(recv_err == 0){
						printf("Connection closed by client while receiving message\n");
						cmd_type = CLOSE_CONNECTION;
					}
					else{
						msgbuf[msglen] = '\0';
						printf("Raw received: %s\n", msgbuf);

						// parse out the parts
						char *msg_part     = strtok(msgbuf, "-");
						char *timestamp_part  = strtok(NULL, "-");
						char *userid_part  = strtok(NULL, "-");
						char *channel_part = strtok(NULL, "-");

						printf("Message: %s\n", msg_part);
						printf("Timestamp: %s\n", timestamp_part);
						printf("User ID: %s\n", userid_part);
						printf("Channel ID: %s\n", channel_part);
					}
					free((char*)malloc(msglen));
					break;
				case LATEST_MESSAGES:
					printf("Latest messages command received\n");
					break;
				case ALL_MSG:
					printf("All messages command received\n");
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

		}while(cmd_type != CLOSE_CONNECTION);

		printf("Returning to listening\n");

	} while (true);

	closeSocket("Closing server connection!\n",NULL,&listenSocket);

	// localAddress.pin_port = 3000;

	return 0;
}