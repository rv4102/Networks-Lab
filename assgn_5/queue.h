#ifndef _QUEUE
#define _QUEUE

#include <stdio.h>
#include <stdlib.h>
 
// A linked list (LL) node to store a queue entry
struct QNode {
    char *buf;
    int msg_len;
    struct QNode* next;
};
 
// The queue, front stores the front node of LL and rear
// stores the last node of LL
struct Queue {
    struct QNode *front, *rear;
    int length;
};

struct Queue* createQueue();
void push(struct Queue* q, char *msg, int msg_len, int maxsize);
struct QNode* peek(struct Queue* q);
void pop(struct Queue* q);

#endif