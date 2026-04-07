#include <winsock2.h>

int initializeClient(SOCKET *socket, char* SERVER_ADDRESS);

int genUser(SOCKET *connectSocket, char* username);

int sendMessage(SOCKET *socket, Message* m_message);

int sockShutdown(SOCKET connectSocket);

int recieveMsgLatest(SOCKET *socket, int channel_id, ScreenMessage *messages);