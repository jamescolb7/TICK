#define WIN32_LEAN_AND_MEAN


#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>

#define SERVER_PORT "3000"

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "lWs2_32")

//A function to report an error message and close the sockets and addresses if applicable, while also printing an error message
void closeSocket(char *message, struct addrinfo *addr, SOCKET *socket) {
	printf("%s\n", message);
	if (addr != NULL) freeaddrinfo(addr);
	if (socket != NULL) closesocket(*socket);
	WSACleanup();
}

//Initialize t
int main() {
  	WSADATA wsaData;
  	SOCKET listenSocket = INVALID_SOCKET, clientSocket = INVALID_SOCKET;

	struct addrinfo *localAddress = NULL, clientAddress;

	printf("WSA Startup\n");

	int servErr = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (servErr != 0) {
		printf("Server failed to start");
    return 1;
	}
	printf("WSA Startup successful\n");

	ZeroMemory(&clientAddress, sizeof(clientAddress));
	clientAddress.ai_family = AF_INET;
	clientAddress.ai_socktype = SOCK_STREAM;
	clientAddress.ai_protocol = IPPROTO_TCP;
	clientAddress.ai_flags = AI_PASSIVE;

	printf("Getting address info\n");

	servErr = getaddrinfo(NULL, SERVER_PORT,&clientAddress,&localAddress);
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
		closeSocket("Failed to listen to socket", NULL, &listenSocket);
		return 1;
	}

	printf("Client found on port %s\n", SERVER_PORT);
	
	clientSocket = accept(listenSocket, NULL, NULL);
	if(clientSocket == INVALID_SOCKET){
		closeSocket("Client socket is invalid", NULL, &listenSocket);
		return 1;
	}

	printf("Client connected\n");

	closesocket(listenSocket);
	WSACleanup();

	// localAddress.sin_adr.s_addr = INADDR_ANY;
	// localAddress.pin_port = 3000;
	

	return 0;
}