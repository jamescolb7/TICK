#include <stdlib.h>

typedef struct {
    int UUID;
    char *name;
} User;

typedef enum{
    GENERAL,
    CHILL,
    MEMES,
    QUOTES,
    NEWS,
    OFF_TOPIC,
    UNKNOWN_CH
} ChannelNameId;

typedef struct {
    struct Node *head;
    struct Node *tail;
} MessageDeque;

typedef struct{
    const char *channel_name;
    MessageDeque *deque;
    ChannelNameId channel_id;
} Channel;

typedef struct {
    ChannelNameId id;
    const char *channel_name;
} ScreenChannel;

extern Channel channels[];

typedef struct {
    char *message;
    User *sender;
    Channel *channel;
    int timestamp;
} Message;

typedef struct {
    char *message;
    char *username;
    int timestamp;
} ScreenMessage;

typedef struct Node{
    Message *mess;
    struct Node *pleft;
    struct Node *pright;
} Node;

//Function to initialise message, requires all fields of a message struct to be passed 
Message *initMessage(char *message, User *userID, int timestamp, Channel *channel);

//Function to initialise a deque, requires no input
MessageDeque *initDeque();

//Function to initialise a node for a deqeue
Node *initNode(Message *mess);

//Dequeues from the front of the deque
Message *dequeueF(MessageDeque *deque);

//Reads most recent messages (up to 20) from the deque, curr_node is the node to start at (should
//be the tail node), and len is the number of elements to be returned (1-20)
Message **readallF(Node *curr_node, int *len);

//Dequeues from the end of the deque
Message *dequeueR(MessageDeque *deque);

//Adds an element to the deque, requires a node to be given to it, and a deque to add it to
int enqueue(MessageDeque *deque, Node *node);

//Defunct, left in just in case
ScreenMessage *initScreenMessage(Message *mess);

typedef struct TreeNode{
    int key;
    int foreignKey;
    struct TreeNode *pleft;
    struct TreeNode *pright;
}TreeNode;

typedef struct{
    TreeNode *root;
} Tree;

//Function to initialise a tree, just returns a pointer to a tree variable
Tree *initTree(int key, int foreign);

//Function to intialise a treenode, requires a key (randomly generated), and the array index the node refers to
TreeNode *initTreenode(int key, int foreign);

//Inserts a node into the BST
int insert(TreeNode *tree, TreeNode *node);

//Finds a node based on key, goal is the key being searched for.
int findUser(TreeNode *tree, int goal);