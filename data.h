typedef struct message{

    char *message;
    struct user *senderID;
    struct user *receiverID;
    int timestamp;

}message;

//This struct will be held by the client and server. The server will also have an array of all users that 
//it will store users in when it initialises them
typedef struct user{

    //Will be assigned starting at 0 by the server. "Unique User ID"
    int UUID;
    char *name;
    //Since starting at 0, each index can literally refer to each other user
    //A 0 indicates not friends, a 1 indicates they are friends
    int friends[];


}user;