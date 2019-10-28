#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "structure.h"

// const int PACKET_SIZE = 512;
void bubbleSort(struct GBN_Receiver_Queue *);
void swap(struct queue_node *a, struct queue_node *);

struct queue_node* new_node(char* data, int length, int sequence){
	struct queue_node *node = (struct queue_node*)malloc(sizeof(struct queue_node));
	memset(node->data, 0, PACKET_SIZE);
	node->data_length = 0;
	node->data_length = length;
	memcpy(node->data, data, length);
	node->seq_num = sequence;
	node->next = NULL;

  	return node;
}

struct GBN_Receiver_Queue *create_queue(){
	struct GBN_Receiver_Queue *q = (struct GBN_Receiver_Queue*)malloc(sizeof(struct GBN_Receiver_Queue));
    q->front = q->rear = NULL;
    return q;
}

void enQueue(struct GBN_Receiver_Queue *q, char* data, int length, int sequence){
	int s;
	struct queue_node *the_node = new_node(data, length, sequence);
	if(q->rear == NULL){
		q->front = q->rear =  the_node;
		return;
	}

	q->rear->next = the_node;
	q->rear = the_node;

	//printf("Sequence is: %d\n", the_node->seq_num);
	bubbleSort(q);
}

/* Bubble sort the given linked lsit */
void bubbleSort(struct GBN_Receiver_Queue *start)
{
    int swapped, i, sequence1, sequence2;
    struct queue_node *ptr1, *ptr2, *temp;
    struct queue_node *lptr = NULL;
	int m=0;

	do{
		swapped = 0;
		ptr1 = start->front;
		ptr2 = start->front->next;
		while(ptr2 != NULL){
			// memcpy(&sequence1, ptr1->data + sizeof(int), sizeof(int));
			// memcpy(&sequence2, ptr2->data + sizeof(int), sizeof(int));
			sequence1 = ptr1->seq_num;
			sequence2 = ptr2->seq_num;
			//printf("sequence1 %d sequence2 %d\n",sequence1, sequence2);

			if(sequence1 > sequence2){
				swap(ptr1, ptr2);
				swapped = 1;
			}

			ptr1 =  ptr2;
			ptr2 = ptr2->next;
		}	
	} while(swapped);
}

/* function to swap data of two nodes a and b*/
void swap(struct queue_node *ptr1, struct queue_node *ptr2)
{
	char data[PACKET_SIZE];
	struct queue_node *temp;
	memset(data, 0, PACKET_SIZE);
	temp = new_node(data, 0, 0);

	memcpy(temp->data, ptr1->data, ptr1->data_length);
	temp->data_length = ptr1->data_length;
	temp->seq_num = ptr1->seq_num;

	memcpy(ptr1->data, ptr2->data, ptr2->data_length);
	ptr1->data_length = ptr2->data_length;
	ptr1->seq_num = ptr2->seq_num;

	memcpy(ptr2->data, temp->data, temp->data_length);
	ptr2->data_length = temp->data_length;
	ptr2->seq_num = temp->seq_num;
}

void display(struct GBN_Receiver_Queue *q){

	int bytes;
	struct queue_node *n = q->front;
	//printf("Sequence of data in the queue is: ");
	while(n != NULL){
		//memcpy(&bytes, n->data+ sizeof(int), sizeof(int));
		bytes = n->seq_num;
	//	printf("%d ", bytes);
		n = n->next;
	}
//	printf("\n");
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
