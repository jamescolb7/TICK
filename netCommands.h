typedef enum{
    HANDSHAKE,
    CLOSE_CONNECTION,
    POST_MSG,
    LATEST_MESSAGES,
    ALL_MSG,
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
    {NULL, UNKNOWN} //sentinel value
};
