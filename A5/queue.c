#include "queue.h"
 
// A utility function to create a new linked list node.
struct QNode* newNode(char *msg, int msg_len, int maxsize) {
    struct QNode* temp = (struct QNode*)malloc(sizeof(struct QNode));
    temp->buf = (char *)malloc(sizeof(char) * maxsize);
    for(int i=0; i<msg_len; i++){
        temp->buf[i] = msg[i];
    }
    temp->msg_len = msg_len;
    temp->next = NULL;
    return temp;
}
 
// A utility function to create an empty queue
struct Queue* createQueue() {
    struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue));
    q->front = q->rear = NULL;
    q->length = 0;
    return q;
}
 
// The function to add a key k to q
void push(struct Queue* q, char *msg, int msg_len, int maxsize){
    // Create a new LL node
    struct QNode* temp = newNode(msg, msg_len, maxsize);
 
    if (q->rear == NULL) {
        q->front = q->rear = temp;
        q->length = 1;
        return;
    }

    q->rear->next = temp;
    q->rear = temp;
    int temp1 = q->length;
    q->length = temp1 + 1;
}

struct QNode* peek(struct Queue* q){
    if (q->front == NULL)
        return NULL;
    
    struct QNode* temp = q->front;
    return temp;
}
 
// Function to remove a key from given queue q
void pop(struct Queue* q){
    if (q->front == NULL)
        return ;
 
    // Store previous front and move front one node ahead
    struct QNode* temp = q->front;
 
    q->front = q->front->next;
 
    // If front becomes NULL, then change rear also as NULL
    if (q->front == NULL)
        q->rear = NULL;
    
    int temp1 = q->length;
    q->length = temp1 - 1;

    free(temp->buf);
    free(temp);
}