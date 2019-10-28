#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "structure.h"

// const int PACKET_SIZE = 512;

struct queue_node* new_node(char* data, int length){
	struct queue_node *node = (struct queue_node*)malloc(sizeof(struct queue_node));
	memset(node->data, 0, PACKET_SIZE);
	node->data_length = 0;
	node->data_length = length;
  memcpy(node->data, data, length);
  node->next = NULL;
  return node;
}

struct GBN_Receiver_Queue *create_queue(){
	struct GBN_Receiver_Queue *q = (struct GBN_Receiver_Queue*)malloc(sizeof(struct GBN_Receiver_Queue));
    q->front = q->rear = NULL;
    return q;
}

void enQueue(struct GBN_Receiver_Queue *q, char* data, int length){
	struct queue_node *the_node = new_node(data, length);
	if(q->rear == NULL){
		q->front = q->rear =  the_node;
		return;
	}

	q->rear->next = the_node;
	q->rear = the_node;
}

struct queue_node *deQueue(struct GBN_Receiver_Queue *q){
    if (q->front == NULL)
       return NULL;

    struct queue_node *temp = q->front;
    q->front = q->front->next;

    if (q->front == NULL)
       q->rear = NULL;

    return temp;
}
