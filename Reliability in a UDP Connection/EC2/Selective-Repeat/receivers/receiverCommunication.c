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
void sendAck(int, int, char*, struct sockaddr_storage, socklen_t, int);
void writetoFile(char*, char*, int, FILE*, struct GBN_Receiver_Queue*);
void sendSelectiveAcks(int, char*, struct sockaddr_storage, socklen_t, int*, int, int);
void add(int*, int);
bool has(int*, int);
void initialize(int**);
void print(int*);
bool rangedCheck(int*, int, int);
int max(int*);

//global Variables
const int INIT = 0;
const int DATA = 1;
const int ACK = 2;
const int CLOSE = 3;
const int ALLOK = 4;
const int NOTALLOK = 5;
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
					sendAck(sockID, curr_seq, dataSent, peer_addr, peer_addr_len, ACK);	//send ACK for INIT Message.
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
				//printf("\nreceived packet: %d\n",curr_seq);

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
				if(curr_seq > base_seq && curr_seq <= base_seq + receiver_window_size){
					
					if(!has(Esequences, curr_seq)){
						//printf("sizeofEArray: %d\n",sizeofEArray);
						initialize(&Esequences);
						//curr_pos += data_length;
						++increment;
						add(Esequences, curr_seq);
						enQueue(queue, data, data_length, curr_seq);
						//display(queue);
						//print(Esequences);
						//printf("incrementer: %d\n", increment);
					}

					if(increment == base_seq + receiver_window_size){
						//printf("Entered\n");
						if(rangedCheck(Esequences, base_seq, base_seq + receiver_window_size)){
							writetoFile(new_name, data, data_length, writeFile, queue);
							// if(!has(Esequences, curr_seq)){
							// 	initialize(&sequences, sizeofArray, Bflag);
							// 	add(sequences, base_seq + receiver_window_size);
							// }

							prev_seq = base_seq;
							base_seq = max(Esequences);
							//printf("Updated base_seq is %d\n", base_seq);
							curr_pos += (PACKET_SIZE-12) * receiver_window_size;
							
							//sendAck(sockID, base_seq, dataSent, peer_addr, peer_addr_len, ACK);
						}
					} else if(curr_pos == file_length){
						base_seq = curr_seq;
						writetoFile(new_name, data, data_length, writeFile, queue);
						sendAck(sockID, curr_seq, dataSent, peer_addr, peer_addr_len,ACK);
					}
				}

			break;
		}

			case 3: {
				printf("File Transfer is completed.\nClosing connection. Initiated by sender\n");
				free(sequences);
				close(sockID);
				exit(EXIT_SUCCESS);
			}

			case 4: {

				//printf("Received all ok packet\n");
				if(curr_pos == file_length){
					if(rangedCheck(Esequences, base_seq, curr_seq)){
						sendAck(sockID, base_seq, dataSent, peer_addr, peer_addr_len, ALLOK);	
					} else{
				//		printf("Everything is not ok\n");
						sendAck(sockID, base_seq, dataSent, peer_addr, peer_addr_len, NOTALLOK);
						sendSelectiveAcks(sockID, dataSent, peer_addr, peer_addr_len, Esequences, base_seq, curr_seq);
					}
				} else{
					if(rangedCheck(Esequences, base_seq, base_seq + receiver_window_size)){
						sendAck(sockID, base_seq, dataSent, peer_addr, peer_addr_len, ALLOK);
					} else{
				//		printf("Everything is not ok\n");
						sendAck(sockID, base_seq, dataSent, peer_addr, peer_addr_len, NOTALLOK);
						sendSelectiveAcks(sockID, dataSent, peer_addr, peer_addr_len, Esequences, base_seq, base_seq + receiver_window_size);
					}
				}
				break;
			}

			default: {
				queue = create_queue();
				sendAck(sockID, base_seq, dataSent, peer_addr, peer_addr_len, ACK);
				//printf("Sent acknowledgment for %d\n",base_seq);
				break;
			}
		}
	}

	exit(EXIT_SUCCESS);
}

void sendSelectiveAcks(int sockID, char* dataSent, struct sockaddr_storage peer_addr, socklen_t peer_addr_len, int* arr, int base, int max){
	int i;


	for(i = base+1; i<= max; i++){
		if(!has(arr, i)){
			sendAck(sockID, i, dataSent, peer_addr, peer_addr_len, ACK);
		}
	}
	sendAck(sockID, i, dataSent, peer_addr, peer_addr_len, ALLOK);
} 

void initialize(int **arr){
	if(sizeofEArray == 0){
		 *arr = (int*) malloc(1000 * sizeof(int));
		  //printf("Memory allocated\n");
	} else if(sizeofEArray == 1000*Eflag){
			*arr = (int*) realloc(*arr, 1000 * sizeof(int));
		  //printf("Memory allocated\n");
			++Eflag;
	}

  return;
}

void add(int* arr, int val){
  arr[sizeofEArray++] = val;
  //printf("No. of Elements of array %ld\n",sizeofEArray);
}

bool has(int* arr, int val){
  int i;

  if(arr == NULL)
    return false;

  for(i=0;i<sizeofEArray;i++){
    if(arr[i] == val)
      return true;
  }

  return false;
}

void print(int* arr){
  int i;

  printf("Elements of array: ");
  for(i= 0; i< sizeofEArray; i++){
    printf("%d ",arr[i]);
  }
  printf("\n");
}

bool rangedCheck(int* arr, int base, int max){
	int i;

	for(i = base+1; i<= max; i++){
		if(!has(arr, i)){
			return false;
		}
	}
	return true;
} 

int max(int* arr){
	int i, m=0, high=0;

	for(i=0;i<sizeofEArray;i++){
		m = arr[i];
		if(high == 0)
			high =m;
		else if(high < m)
			high = m;

	}

	return high;
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

void sendAck(int sockID, int sequence, char* dataSent, struct sockaddr_storage peer_addr, socklen_t peer_addr_len, int type){
		char* currAddress;
		int ack_bytes=8;

		memset(dataSent, 0, sizeof dataSent);

		currAddress = dataSent;
    	dataSent[0] = type;
    	currAddress += sizeof(int);

    	memcpy(currAddress, &sequence, sizeof(int));
    	currAddress += sizeof(int);

    	if(ack_bytes != sendto(sockID, dataSent, ack_bytes, 0, (struct sockaddr *) &peer_addr, peer_addr_len)){
    		fprintf(stderr, "Error sending response %s\n", strerror(errno));
    	}
}
