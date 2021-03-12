#ifndef QUEUE_H_   /* Include guard */
#define QUEUE_H_

#include <stdio.h>
#include <stdlib.h>

typedef struct node {
    struct node *next;
    void *data;
    char name[20];
    int life; 
    int arrival;
} Node;

typedef struct queue {
    Node *first;
    Node *last;
    int length;
} Queue;

Queue* newQueue() {

    Queue *q = (Queue *) malloc(sizeof(Queue));
    q->first = NULL;
    q->last = NULL;
    q->length = 0;

    return q;
}

int isEmpty(Queue *q) { 
    if ( q->length != 0 ) {
        return 0;
    } else if ( q->first != NULL || q->last != NULL ) {
        return 0;
    } else {
        return 1;
    }
}

void pushNode(Queue *q, Node *n) {
    
    if ( isEmpty(q) ) {
        q->first = n;
        q->last = n;
        q->length = 1;
    } else {
        q->last->next = n;
        q->last = q->last->next;
        q->length++;
    }     
}

Node *popNode(Queue *q) {
    if ( isEmpty(q) ) {
        return NULL;
    } else {
        Node *aux = q->first;
        q->first = q->first->next;
        q->length--;

        if ( q->length == 0 ) { q->last = NULL; }
        
        aux->next = NULL;
        return aux;
    }
}

void printQueue(Queue *q) {
    if ( !isEmpty(q) ) {
        Node *aux = NULL;
        aux = q->first;
        while (aux) {
            printf("%s::%d ", aux->name, aux->life);
            aux = aux->next;
        }
    } else {
        printf("-empty-");
    }

}

#endif // QUEUE_H_