#include <stdio.h>

#ifndef STRUCT_H
#define STRUCT_H

#define PACKET_SIZE 512

///////////////////////////////////////////////////////////////////////////
// Queue Structure.

struct queue_node{
	char data[PACKET_SIZE];
	int data_length;
	struct queue_node *next;
};

struct GBN_Receiver_Queue{
	struct queue_node *front, *rear;
};

struct queue_node* new_node(char*, int);
struct GBN_Receiver_Queue *create_queue();
void enQueue(struct GBN_Receiver_Queue *, char*, int);
struct queue_node *deQueue(struct GBN_Receiver_Queue *);

#endif
