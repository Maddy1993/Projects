#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <time.h>
#include <stdbool.h>

#include "receiverCommunication.h"
#include "structure.h"

char* tokenizer(char*, int);
void sendAck(int, int, char*, struct sockaddr_storage, socklen_t);
void writetoFile(char*, char*, int, FILE*, struct GBN_Receiver_Queue*);
void add(int*, int);
bool has(int*, int);
void initialize(int**, int, int);
void print(int*);
bool rangedCheck(int*, int, int, int);

//global Variables
const int INIT = 0;
const int ALLOK = 00;
const int DATA = 1;
const int ACK = 2;
const int CLOSE = 3;
int *sequences = NULL;
int *Esequences = NULL;
int sizeofEArray = 0;
int sizeofArray = 0;
int Bflag = 1;
int Eflag = 1;


int receiverCommunication(int sockID, char* mode){

	//-------------------VARIABLE DECLARATIONS ----------------------//

	const int BUFFSIZE = 512;
	int receiver_window_size=0;

	// Data buffers and pointers to current address of input buffer.
	char dataBuff[BUFFSIZE], dataSent[BUFFSIZE], data[BUFFSIZE];
	char *currAddress;

	// To store the address of the host
	struct sockaddr_storage peer_addr;
	socklen_t peer_addr_len;

	// Variables to store data
	ssize_t recvBytes;
	int curr_seq=0,prev_seq = 0, base_seq = 0;
	int p=0;
	int last_in_order_packet =0;

	// Control packet types.
	int packet_type;
	int file_length, data_length, curr_pos=0;
	int window_size=0;
	long file_name_length;
	char file_name[MAX_FILE_SIZE], new_name[MAX_FILE_SIZE];
	int resend_flag = 0;
	int increment=0;

	// FILE Pointers.
	FILE *writeFile;
	pid_t processID;

	//---------------END VARIABLE DECLARATIONS ----------------------//

	struct GBN_Receiver_Queue *queue = create_queue();

	printf("All set. Waiting to receive...\n");
	while(sockID != -1){

		memset(dataSent, 0, BUFFSIZE);
		memset(dataBuff, 0, BUFFSIZE);

		peer_addr_len = sizeof(struct sockaddr_storage);

		if((recvBytes = recvfrom(sockID, dataBuff, BUFFSIZE, 0, (struct sockaddr *) &peer_addr, &peer_addr_len)) == -1){
			fprintf(stderr, "Ignoring failed request: %s\n", strerror(errno));
			continue;
		}

		receiver_window_size = atoi(mode);

		//Uncomment if the sender address details are to be printed.
    // char host[NI_MAXHOST], service[NI_MAXSERV];
		// p = getnameinfo((struct sockaddr *) &peer_addr,
    //                            peer_addr_len, host, sizeof(host),
    //                            service, sizeof(service),NI_NUMERICHOST | NI_NUMERICSERV);
		//
		// if(p==0){
		// 		printf("Received data from %s\n", host);
		// } else
    //    fprintf(stderr, "getnameinfo: %s\n", gai_strerror(p));


    currAddress = dataBuff;
		memcpy(&packet_type, currAddress, sizeof(int));
		currAddress += sizeof(int);

		switch(packet_type){
			case 0: {
				// If message-type is INIT
				memset(file_name, 0, sizeof file_name);
				memset(new_name, 0, sizeof new_name);

				memcpy(&window_size, currAddress, sizeof(int));										//Header-control-packet: window_size
				currAddress += sizeof(int);

				memcpy(&file_length, currAddress, sizeof(int));										//Header-control-packet: file_length
				currAddress += sizeof(int);

				memcpy(&file_name_length, currAddress, sizeof(long));							//Header-control-packet: file_name_length
				currAddress += sizeof(long);

				memcpy(&file_name, currAddress, file_name_length);								//Header-control-packet: file_name
				currAddress += file_name_length;

				processID = getpid();
				sprintf(new_name, "%d%s%s", processID,"-", file_name);
				printf("New file_name: %s\n", new_name);

				if(window_size != receiver_window_size){													// When receiver and sender window size doesn't match
					printf("Window size of sender and receiver do not match\n");    // For Stop-and-Wait, the window size should be 1.
					close(sockID);
					exit(EXIT_FAILURE);
				} else{
					sendAck(sockID, curr_seq, dataSent, peer_addr, peer_addr_len);	//send ACK for INIT Message.
	        																																//Sequence Number will be 0
				}

				break;
			}

			case 1: {
				// If message-type is DATA
				memset(data, 0, BUFFSIZE);
				//printf("packet received\n");

				memcpy(&curr_seq, currAddress, sizeof(int));
				currAddress +=  sizeof(int);
				printf("received packet: %d\n",curr_seq);

				memcpy(&data_length, currAddress, sizeof(int));
				currAddress += sizeof(int);

				memcpy(data, currAddress, data_length);
				//printf("data receivd is: \n\n%s\n\n",data);
				currAddress +=  data_length;

				if(curr_pos+ data_length == file_length){
					curr_pos += data_length;
				}
				// If the current sequence is the next sequence in
				// order, if three packets are receivd in order, then update the base_seq
				if(curr_seq > base_seq && curr_seq <= receiver_window_size){
					
					if(!has(Esequences, curr_seq)){
						initialize(&Esequences, sizeofEArray, Eflag);
						//curr_pos += data_length;
						++increment;
						add(Esequences, curr_seq);
						enQueue(queue, data, data_length);
					}

					if(increment == receiver_window_size){
						if(rangedCheck(Esequences, sizeofEArray, base_seq, base_seq + receiver_window_size)){
							writetoFile(new_name, data, data_length, writeFile, queue);
							if(!has(Esequences, curr_seq)){
								initialize(&sequences, sizeofArray, Bflag);
								add(sequences, base_seq + receiver_window_size);
							}

							curr_pos += (PACKET_SIZE-12) * receiver_window_size;
							
							sendAck(sockID, ALLOK, dataSent, peer_addr, peer_addr_len);
						}
					}
				}
				// } else if(curr_pos == file_length){
				// 	if(!has(Esequences, curr_seq)){
				// 		initialize(&Esequences, sizeofEArray, Eflag);
				// 		//curr_pos += data_length;
				// 		++increment;
				// 		add(Esequences, curr_seq);
				// 		enQueue(queue, data, data_length);
				// 	}
				// }

				
				// if(curr_seq == (prev_seq + 1)){
				// 	resend_flag = 1;
				// 	last_in_order_packet = curr_seq;
				// 	enQueue(queue, data, data_length);																		// Add data value to queue.
				// 	if((curr_seq - base_seq) == receiver_window_size){
				// 		base_seq = curr_seq;
				// 		if(!has(sequences, base_seq)){
				// 			initialize(&sequences);
				// 			add(sequences, base_seq);
				// 			display(sequences);
				// 			curr_pos += receiver_window_size * (PACKET_SIZE-12);
				// 			writetoFile(new_name, data, data_length, writeFile, queue);
				// 		}
				// 		// sendAck for the last packet in order. Else, send the ack for last
				// 		// correct packet in the sequence.
				// 		sendAck(sockID, curr_seq, dataSent, peer_addr, peer_addr_len);
				//
				// 		printf("Sent acknowledgment for window: %d\n",curr_seq);
				// 	}
				//
				// 	prev_seq = curr_seq;
				//
				// 	// When the packet received is the last packet and the frames received are less than
				// 	// window size, write the packet to the file.
				// 	printf("curr_pos: %d file_length: %d\n",curr_pos, file_length);
				// 	if(curr_pos == file_length){
				// 		writetoFile(new_name, data, data_length, writeFile, queue);					// sendAck for the last packet in order. Else, send the ack for last
				// 		sendAck(sockID, curr_seq, dataSent, peer_addr, peer_addr_len);			// correct packet in the sequence.
				// 		printf("\nSent acknowledgment for %d\n",curr_seq);
				// 		//curr_pos += data_length;
				// 	} else if(curr_pos > file_length)
				// 			exit(EXIT_FAILURE);
				// } else if(resend_flag == 1){
				// 	printf("Expected to receive packet: %d. Instead received packet: %d\n",last_in_order_packet+1, curr_seq);
				// 	printf("Sending acknowledgment for the last received in-order packet: %d\n",base_seq);
				// 	last_in_order_packet = base_seq;
				// 	prev_seq = base_seq;
				// 	queue = create_queue();
				// 	resend_flag= 0;
				// 	sendAck(sockID, base_seq, dataSent, peer_addr, peer_addr_len);
				// }

			break;
		}

			case 3: {
				printf("File Transfer is completed.\nClosing connection. Initiated by sender\n");
				free(sequences);
				close(sockID);
				exit(EXIT_SUCCESS);
			}

			default: {
				queue = create_queue();
				sendAck(sockID, base_seq, dataSent, peer_addr, peer_addr_len);
				printf("Sent acknowledgment for %d\n",base_seq);
				break;
			}
		}
	}

	exit(EXIT_SUCCESS);
}

void initialize(int **arr, int sizeofArray, int flag){
	if(sizeofArray == 0){
			*arr = (int*) malloc(1000 * sizeof(int));
		  printf("Memory allocated\n");
	} else if(sizeofArray == 1000*flag){
			*arr = (int*) realloc(*arr, 1000 * sizeof(int));
		  printf("Memory allocated\n");
			++flag;
	}

  return;
}

void add(int* arr, int val){
  arr[sizeofArray++] = val;
  printf("No. of Elements of array %ld\n",sizeofArray);
}

bool has(int* arr, int val){
  int i;

  if(arr == NULL)
    return false;

  for(i=0;i<sizeofArray;i++){
    if(arr[i] == val)
      return true;
  }

  return false;
}

void print(int* arr){
  int i;

  printf("Elements of array: ");
  for(i= 0; i< sizeofArray; i++){
    printf("%d ",arr[i]);
  }
  printf("\n\n");
}

bool rangedCheck(int* arr, int size, int base, int max){
	int i;

	for(i = base+1; i<= max; i++){
		if(!has(arr, i))
			return false;
	}
} 

void writetoFile(char* file_name, char* data, int data_length, FILE* writeFile, struct GBN_Receiver_Queue *queue){

			struct queue_node *n;
			while((n = deQueue(queue)) != NULL){

				if((writeFile = fopen(file_name, "a")) == NULL){
					fprintf(stderr, "Error opening file %s: %s\n", file_name, strerror(errno));
					exit(EXIT_FAILURE);
				}

				if(feof(writeFile) == 0){
					fwrite(n->data, sizeof(char), n->data_length, writeFile);
					fflush(writeFile);
				}

				if(ferror(writeFile) != 0){
					fprintf(stderr, "Error while writing : %s\n", strerror(errno));
				}
				else{
					if(fclose(writeFile) != 0){
						fprintf(stderr, "%s\n", strerror(errno));
						exit(EXIT_FAILURE);
					}
				}
			}
}

void sendAck(int sockID, int sequence, char* dataSent, struct sockaddr_storage peer_addr, socklen_t peer_addr_len){
		char* currAddress;
		int ack_bytes=8;

		memset(dataSent, 0, sizeof dataSent);

		currAddress = dataSent;
    	dataSent[0] = ACK;
    	currAddress += sizeof(int);

    	memcpy(currAddress, &sequence, sizeof(int));
    	currAddress += sizeof(int);

    	if(ack_bytes != sendto(sockID, dataSent, ack_bytes, 0, (struct sockaddr *) &peer_addr, peer_addr_len)){
    		fprintf(stderr, "Error sending response %s\n", strerror(errno));
    	}
}
