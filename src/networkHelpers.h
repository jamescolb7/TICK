#include <winsock2.h>

typedef enum{
    HANDSHAKE,
    CLOSE_CONNECTION,
    POST_MSG,
    LATEST_MESSAGES,
    ALL_MSG,
    GEN_ACC,
    UNKNOWN
}CommandType;

typedef struct{
    const char *command_str; //key
    CommandType command_type; //value
}Command;

//hashmap of command strings used for switchcase statements in server.c

Command commands[] = {
    {"#HSK", HANDSHAKE},
    {"#CCN", CLOSE_CONNECTION},
    {"#POM", POST_MSG},
    {"#LMS", LATEST_MESSAGES},
    {"#ALM", ALL_MSG},
    {"#GAC", GEN_ACC},
    {NULL, UNKNOWN} //sentinel value
};


void closeSocket(char *message, struct addrinfo *addr, SOCKET *socket);

char* intToArray(int number, int size);

int sendOnSock(SOCKET *socket, char *message, int len);

char** dataParse(char* buff, char delimiter);

char* dataPackage(char** msg_parts, char delimiter);

