typedef struct message{

    char *message;
    struct user *senderID;
    struct user *receiverID;
    int timestamp;

}message;

typedef struct node{
    struct message *mess;
    struct node *pleft;
    struct node *pright;
}node;

typedef struct messageDeque{
    struct node *head;
    struct node *tail;
}messageDeque;


//Function to initialise a message node. When a message is input this will be run to generate the parameters of that message.
//Takes a string (the input string), the senderID or who is sending the message, and the receiverID or who it is being sent to.
message *initMessage (char *string, struct user *sID, struct user *rID);

//Function to initialise the messageDeque. Takes no inputs as it simply mallocs space for the deque on heap, 
//sets the head and tail as NULL, then returns a pointer to that struct.
messageDeque *initDeque();

//Initialises a node, t128950ihese are set when the node is enqueued.
node *initNode(message *mess);

//Dequeues the front of the deque and returns a pointer to it, will also free the node it was stored at.
//Only needs the messageDeque as it simply returns what is at the head.
message *dequeueF(messageDeque *deque);

//Dequeues the rear of the deque and returns a pointer to it, will also free the node it was stored at.
//This is to be used for like unsending a message (As messages will be sent from the front and added to the rear).
message *dequeueR(messageDeque *deque);

//Enqueues a new value at the end of the deque, takes the node that it will enqueue and the deque it is being added to
//Returns an int which is just a flag of failure or success to add it to the queue.
int enqueue(messageDeque *deque,node *node);


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

//Friend finder is a function which (when the command for it is run) tells you if you have friends of friends with a particular UUID
//Also tells you the degree of the friendship (That is, how many friends are inbetween both of you)
//Returns the degree (-1 = no connection, 0 = you, 1 = direct friend, 2+ = 1,2,3,etc. friends in between)
int friendFinder(int *connectionMatrix, user *user);

//Struct for tree node, stores everything that the node needs to have. Key is the user ID, the "foreign key" is 
//The associated index number in the user array
typedef struct tree_node
{
    int key; //This is the USER ID
    int foreignKey; //This is the index of this user ID in the associated array
    struct tree_node *pleft, *pright;
}tree_node;

typedef struct tree{

    struct tree_node *root;

}tree;

//initTree initialises the tree with a root node of what is passed to the function
tree *initTree(int key, int foreign);

//initTreenode initialises a tree node with the data
tree_node *initTreenode(int key, int foreign);

//insert puts a node into the tree with the correct BST property
//Returns an int to determine success. A 1 is a success, a 0 is a failure.
//tree is the tree it is being put into (Only has the root of the tree), and node is the node being inserted
int insert(tree *tree, tree_node *node);

//findUser looks for a node with a specific key, and then returns the foreign_key of that node
//This means it returns an int. Tree is the tree to search, goal is the desired user ID.
int findUser(tree *tree, int goal);