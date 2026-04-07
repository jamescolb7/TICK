#include <winsock2.h>


void closeSocket(char *message, struct addrinfo *addr, SOCKET *socket);

char* intToArray(int number);

int sendOnSock(SOCKET *socket, char *message, int len);

int dataParse(char* buff, char *msg_part, char *usr_part, char *tmp_part, char* cnl_part);

