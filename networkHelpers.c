#include <stdio.h>

#include "netData.h"

void closeSocket(char *message, struct addrinfo *addr, SOCKET *socket) {
    printf("%s\n", message);
    if (addr != NULL) freeaddrinfo(addr);
    if (socket != NULL) closesocket(*socket);
    WSACleanup();
}

char* intToArray(int number){
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

//simple fucntion to count # of smth in character array. not too complex but a nice helper
int stringCounter(char* buff, char des){
    int count = 0;
    for(int i  = 0; i<strlen(buff);i++){
        if(buff[i] == des)
            count++
    }
    return count;
}

//parses the datapacket made

int dataParse(char* buff, char **out){
    f_count = stringCounter(buff, '\f');
    if(f_count > 0){
        void *tmp = realloc(out, f_count);
        if (tmp != NULL)
            out = tmp;
        else {
            printf("Data parse failed! Memory allocation failure...");
            return 1;
        }

        char* token = strtok(buff,"\f")
        while(token != NULL)
            token = strtok(buff,"\f");
    }
    else{
        printf("Data parse failed! Invalid packet. Exiting data parse...\n");
        return 1;
    }
    

    return 0;
}

int dataPackage()