/*
FILENAME:   DSAFunctions.c
COURSE:     MREN 178

~ Group 1 ~

Nicolas Kaye             STUDENT ID: 20506269
Lukas Nuzzi              STUDENT ID: 20524153

*/

#include <stdlib.h>
#include <stdio.h>

#include "DSAFunctions.h"

//Function definitions to be used broadly in the code base, can be easily adapted for the most part

Channel channels[] = {
    {"general", NULL, GENERAL},
    {"chill",   NULL, CHILL},
    {"memes",   NULL, MEMES},
    {"quotes",  NULL, QUOTES},
    {"news",    NULL, NEWS},
    {"off_topic", NULL, OFF_TOPIC},
    {NULL, NULL, UNKNOWN_CH}
};

Message *initMessage (char *message, User *UserID, int timestamp, Channel *channel){
    Message *ptemp = malloc(sizeof(Message));
    if (ptemp == NULL){
        return NULL;
    }
    else{
        ptemp->message= message;
        ptemp->sender = UserID;
        ptemp->channel = channel;
        ptemp->timestamp = timestamp;
        return ptemp;
    }
}

Node *initNode(Message *mess){
    Node *ptemp = malloc(sizeof(Node));
    if (ptemp == NULL){
        return NULL;
    }
    else{
        ptemp->mess=mess;
        ptemp->pleft=NULL;
        ptemp->pright=NULL;
        return ptemp;
    }
}

MessageDeque *initDeque(){
    MessageDeque *ptemp=malloc(sizeof(MessageDeque));
    if (ptemp == NULL){
        return NULL;
    }
    else{
        ptemp->head=NULL;
        ptemp->tail=NULL;
        return ptemp;
    }
}

Message *dequeueF(MessageDeque* deque){
    //For mem safety need to check if the deque is empty, if it is return a NULL
    
    //If the head pointer points to nothing, there is nothing at the front to remove
    if (deque->head == NULL){
        return NULL;
    }

    else{
        //Gets a pointer to the message struct in the head node
        Message *proxy = deque->head->mess;
        //Sets a temp pointer to the current head node
        Node *ptemp = deque->head;
        //Sets the head as the right pointer of the current head
        deque->head = ptemp->pright;
        //Frees the old head node
        free(ptemp);
        //Returns the pointer to the message
        return proxy;
    }
}

//Reads the most recent "*len" messages from the deque (starts at the end as they are added 
//from the back) and then goes forward. Curr_node should be passes as the tail node of the deque.
Message **readallF(Node* curr_node, int *len){
    if (*len<1){
        *len = 0; //set to 0 for invalid arrays
        return NULL; //Cannot have 0 len or lower, this will mess up the array generation
    }
    if (curr_node==NULL){
        *len = 0; //set to 0 for invalid arrays
        return NULL; //Not operating on empty nodes
    }
    Message **mess_arr = malloc(*len*sizeof(Message*)); //must be malloc due to dynamic sizing
    Node *ptemp = curr_node;
    int temp = 0;
    for (int i = 0; i<*len; i++){
        if (ptemp == NULL){
            break;
        }
        mess_arr[i] = ptemp->mess;
        ptemp = ptemp->pleft;
        temp = i+1;
    }
    *len = temp; //now we can tell how long the array is!
    return mess_arr;
}

Message *dequeueR(MessageDeque* deque){
    //For mem safety need to check if the deque is empty, if it is return a NULL

    //If the tail pointer points to nothing, there is nothing to remove from rear
    if (deque->tail == NULL){
        return NULL;
    }

    else{
        //Gets a pointer to the message struct in the tail node
        Message *proxy = deque->tail->mess;
        //Sets a temp pointer to the current tail node
        Node *ptemp = deque->tail;
        //Sets the tail as the left pointer of the current tail
        deque->tail = ptemp->pleft;
        //Frees the old tail node
        free(ptemp);
        //Returns the pointer to the message
        return proxy;
    }
}

//Return 1 if successful, 0 if failure
int enqueue(MessageDeque *deque, Node *node){
    //Need two cases - Deque is empty (head == NULL), and Deque has elements (head != NULL)
    if (deque==NULL || node==NULL){
        return 0;
    }
    if (deque->head==NULL){
        deque->head=node;
        deque->tail=node;
        return 1;
    }
    else if (deque->head!=NULL){
        Node *proxyp=deque->tail;
        deque->tail=node;
        node->pleft=proxyp;
        proxyp->pright=node;
        return 1;
    }
}

//Tree stuff

Tree *initTree(int key, int foreign){

    Tree *ptemp = malloc(sizeof(Tree));
    if (ptemp==NULL){
        return NULL; //Basically an error code if it fails
    }
    ptemp->root = initTreenode(key, foreign);
    return ptemp;

}

TreeNode *initTreenode(int key, int foreign){
    
    TreeNode *ptemp = malloc(sizeof(TreeNode));
    if (ptemp==NULL){
        return NULL;
    }
    ptemp->key=key;
    ptemp->foreignKey = foreign;  
    ptemp->pleft = NULL;
    ptemp->pright = NULL;
    return ptemp;
}

//If necessary, can also modify this function to pass key and foreign, and then just pass it to
//initTreenode which will then pass out the node, just how you want it to go. Returns a 1 if successful
int insert(TreeNode *root, TreeNode *node){
    //Should never happen, but just incase the tree is passed and has no root,
    //this sets the root as the node
    if (root==NULL){
        root=node;
        return 1;
    }
    //If the node has a larger key, should be inserted on the right
    else if (node->key > root->key){
        //If the right pointer is empty, then make it point to the node
        if (root->pright==NULL){
            root->pright=node;
            return 1;
        }
        //If the right pointer isn't empty, then it means you need to recurse into the insert function
        else
        {
            insert(root->pright, node);
        }
    }
    else if (node->key < root->key)
    {
        if(root->pleft==NULL)
        {
            root->pleft = node;
            return 1;
        }
        else
        {
            insert(root->pleft, node);
        }
    }
}

int findUser(TreeNode *node, int goal)
{
    if(node == NULL)
        return -1;
    if(goal == node->key) {
        return node->foreignKey;
    } else if (goal > node->key) {
        if(node->pright == NULL)
            return -1;
        else
        {
           return findUser(node->pright, goal);
        }

    }

    else
    {
        if(node->pleft == NULL)
            return -1;
        else
        {
            return findUser(node->pleft, goal);
        }
        
    }
}

//Converts a message to a screen message type (which is the kind that can be printed!!!)
ScreenMessage *initScreenMessage(Message *mess){
    ScreenMessage *ptemp = malloc(sizeof(ScreenMessage));
    if (ptemp==NULL){
        return NULL; //This means malloc failed
    }
    ptemp->message = mess->message;
    ptemp->username = mess->sender->name;
    ptemp->timestamp = mess->timestamp;
    return ptemp;
}
