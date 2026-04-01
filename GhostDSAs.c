#include "GhostDSAs.h"

//Function definitions to be used broadly in the code base, can be easily adapted for the most part

message *initMessage (char *string, struct user *sID, struct user *rID){
    message *ptemp =(struct message)malloc(sizeof(struct message));
    if (ptemp == NULL){
        return NULL;
    }
    else{
        ptemp->message=string;
        ptemp->senderID = sID;
        ptemp->receiverID = rID;
        return ptemp;
    }
}

node *initNode(message *mess){
    node *ptemp = (struct node)malloc(sizeof(struct node));
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

messageDeque *initDeque(){
    messageDeque *ptemp=(struct messageDeque)malloc(sizeof(struct messageDeque));
    if (ptemp == NULL){
        return NULL;
    }
    else{
        ptemp->head=NULL;
        ptemp->tail=NULL;
        return ptemp;
    }
}

message *dequeueF(messageDeque* deque){
    //For mem safety need to check if the deque is empty, if it is return a NULL
    
    //If the head pointer points to nothing, there is nothing at the front to remove
    if (deque->head == NULL){
        return NULL;
    }

    else{
        //Gets a pointer to the message struct in the head node
        message *proxy = deque->head->mess;
        //Sets a temp pointer to the current head node
        messageDeque *ptemp = deque->head;
        //Sets the head as the right pointer of the current head
        deque->head = ptemp->pright;
        //Frees the old head node
        free(ptemp);
        //Returns the pointer to the message
        return proxy;
    }
}

message *dequeueR(messageDeque* deque){
    //For mem safety need to check if the deque is empty, if it is return a NULL

    //If the tail pointer points to nothing, there is nothing to remove from rear
    if (deque->tail == NULL){
        return NULL;
    }

    else{
        //Gets a pointer to the message struct in the tail node
        message *proxy = deque->tail->mess;
        //Sets a temp pointer to the current tail node
        messageDeque *ptemp = deque->tail;
        //Sets the tail as the left pointer of the current tail
        deque->tail = ptemp->pleft;
        //Frees the old tail node
        free(ptemp);
        //Returns the pointer to the message
        return proxy;
    }
}

//Return 1 if successful, 0 if failure
int enqueue(messageDeque *deque,node *node){
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
        proxyp=deque->tail;
        deque->tail=node;
        node->pleft=proxyp;
        proxyp->pright=deque->tail;
        return 1;
    }
}


tree *innitTree(int key, int foreign){

    tree *ptemp = (struct tree)malloc(sizeof(struct tree));
    ptemp->root = initTreenode(key, foreign);

}

tree_node *initTreenode(int key, int foreign){

}

int insert(tree *tree, tree_node *node){

}

int findUser(tree *tree, int goal){

}
