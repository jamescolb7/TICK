#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h> 

void closeSocket(char *message, struct addrinfo *addr, SOCKET *socket) {
    printf("%s\n", message);
    if (addr != NULL) freeaddrinfo(addr);
    if (socket != NULL) closesocket(*socket);
    WSACleanup();
}

char* intToArray(int number, int size){
    char *numberArray = calloc(size + 1, sizeof(char));  // +1 for null terminator
    for (int i = size-1; i >= 0; --i){
        numberArray[i] = (number % 10) + '0';
        number /= 10;
    }
    numberArray[size] = '\0';  // null terminate
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
            count++;
    }
    return count;
}

//parses the datapacket made
 //order should be - 0 message, 1 timestamp, 2 user, 3 channel.
char** dataParse(char* buff, char delimiter){
    int f_count = stringCounter(buff, delimiter);

    if(f_count > 0){
        char** tmp = malloc(f_count);
        char delimiter_str[2] = {delimiter,'\n'}; //i hate this solution but it works
        if (tmp == NULL){
            printf("Data parse failed! Memory allocation failure...");
            free(tmp);
            return NULL;
        }
        char* token = strtok(buff,delimiter_str);
        int count = 0;
        while(token != NULL){
            tmp[count] = token;
            count++;
            token = strtok(NULL,delimiter_str);
        }
        return tmp;
    }
    else{
        printf("Data parse failed! Invalid packet. Exiting data parse...\n");
        return NULL;
    }
    return NULL;
}

// packages data into datapacket seperated by \f or wahtever delimiter u want ig
 //order should be - 0 message, 1 timestamp, 2 user, 3 channel.
char* dataPackage(char** msg_parts, int count, char delimiter){
    // Calculate total length needed
    int total_len = 0;
    for(int i = 0; i < count; i++){
        total_len += strlen(msg_parts[i]);
    }
    total_len += count - 1; //delimiters between each part
    total_len += 1; //for the null terminator

    char *result = calloc(total_len, sizeof(char));

    for(int i = 0; i < count; i++){
        strcat(result, msg_parts[i]);
        if(i < count - 1){
            strncat(result, &delimiter, 1); //put delimiter between parts
        }
    }

    return result;
}