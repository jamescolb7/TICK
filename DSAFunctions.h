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

Message *initMessage(char *message, User *userID, int timestamp, Channel *channel);
MessageDeque *initDeque();
Node *initNode(Message *mess);
Message *dequeueF(MessageDeque *deque);
Message *readallF(Node *curr_node, int i);
Message *dequeueR(MessageDeque *deque);
int enqueue(MessageDeque *deque, Node *node);
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

Tree *initTree(int key, int foreign);
TreeNode *initTreenode(int key, int foreign);
int insert(TreeNode *tree, TreeNode *node);
int findUser(TreeNode *tree, int goal);