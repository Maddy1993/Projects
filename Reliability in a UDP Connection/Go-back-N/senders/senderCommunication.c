#define ACK_BYTES 5

struct queue_node{
	char data[PACKET_SIZE];
	struct queue_node *next;
};

struct GBN_Queue{
	struct queue_node *front, *rear;
};

struct queue_node* new_node(char* data, long size){
	struct queue_node *node = (struct queue_node*)malloc(sizeof(struct queue_node));
	memset(node->data, 0, PACKET_SIZE);
	char d[PACKET_SIZE];
	memset(d, 0, PACKET_SIZE);
  //strcpy(node->data, data);
	int bytes;
	memcpy(node->data, data, sizeof(char)* size);
	memcpy(&bytes, node->data+ sizeof(int), sizeof(int));
	// memcpy(d, node->data+ sizeof(int)+ sizeof(int)+sizeof(int), bytes);
	//printf("sequence in the node is:%d\n",bytes);
  node->next = NULL;
  return node;
}

struct GBN_Queue *create_queue(){
	struct GBN_Queue *q = (struct GBN_Queue*)malloc(sizeof(struct GBN_Queue));
    q->front = q->rear = NULL;
    return q;
}

void enQueue(struct GBN_Queue *q, char* data, long size){
	struct queue_node *the_node = new_node(data, size);
	if(q->rear == NULL){
		q->front = q->rear = the_node;
		return;
	}

	q->rear->next = the_node;
	q->rear = the_node;
}

void display(struct GBN_Queue *q){

	int bytes;
	struct queue_node *n = q->front;
	struct GBN_Queue *m;
	while(n != NULL){
		memcpy(&bytes, n->data+ sizeof(int), sizeof(int));
		//printf("sequence in the node is: %d ", bytes);
		n = n->next;
	}
	//printf("\n");
}

struct queue_node *deQueue(struct GBN_Queue *q)
{
    if (q->front == NULL)
       return NULL;

    struct queue_node *temp = q->front;
    q->front = q->front->next;

    if (q->front == NULL)
       q->rear = NULL;

    return temp;
}

char* send_INIT_Packet(int, int, char*, long, int, char*, char*);
char* read_packet(int, char*, struct GBN_Queue*, long, char*, char*, int);
char* send_packet(int,char*, long, char*, char*);
void  send_N_packets(int, int, int, int, FILE*, int, char*, char*);
struct GBN_Queue* resend(struct GBN_Queue *, int, long, char*, char*, int);

//global Variables
const int INIT = 0;
const int DATA = 1;
const int ACK = 2;
const int CLOSE = 3;
int stop_flag =0;
int incrementer =0;
struct GBN_Queue *queue;

void senderCommunication(int sockID, char* mode, char* filename, char* address, char* port){

	//Variable  Declarations
	int window_size;
	const int ACK_SIZE = 10;
	char* currAddress;

	//Input-file related variables
	int file_size;
	long file_name_length;
	FILE *input_file;

	//Data-type: 0 for INIT
	//					 1 for Data
	//           2 for ACK
	//           3 for Closing Packet
	int messageType;
	int packet_buffer_size;
	char dataReceived[ACK_SIZE];

	//Sequence-Numbers
	int last_sequence_number;

	//End Declarations
	//--------------------------------------------------------------------------------------------//

	memset(dataReceived, 0, ACK_SIZE);
	queue = create_queue();

	window_size = atoi(mode);

	if((input_file = fopen(filename, "r")) == NULL){				//Open input file to read.
			fprintf(stderr, "Error opening file %s: %s\n", filename, strerror(errno));
			exit(EXIT_FAILURE);
	}

	//Get file size
	fseek(input_file, 0, SEEK_END);
	file_size = ftell(input_file);
	//printf("%d\n", file_size);
	fseek(input_file, 0, SEEK_SET);

	file_name_length = strlen(filename);

	//Send INIT Message
	printf("All set. Sending INIT message to receiver %s\n",address);
	memcpy(dataReceived, send_INIT_Packet(sockID, window_size, filename, file_name_length, file_size, address, port), ACK_BYTES);

	currAddress = dataReceived;
	memcpy(&messageType, currAddress, sizeof(int));				//Get data-type from the acknkowledgment message

	currAddress = currAddress + sizeof(int);
	memcpy(&last_sequence_number, currAddress, sizeof(int));		//Get sequence-number of last packet from the acknkowledgment message

	send_N_packets(sockID, last_sequence_number, file_size, messageType, input_file, window_size, address, port);
}

void  send_N_packets(int sockID, int last_sequence_number, int file_size, int messageType, FILE* input_file, int window_size, char* address, char* port){

	char dataBuffer[window_size][PACKET_SIZE], data_read[PACKET_SIZE];
	char data_received_buffer[PACKET_SIZE], data_sent_buffer[PACKET_SIZE], d[PACKET_SIZE];
	char *currAddress, *in_loop_address;
	int curr_sequence_number = 0;
	int curr_pos = 0, remaining_char = 0, data_length = 0;
	long bufferSize;
	int bytes_read =0;
	int print_flag=0;

	memset(data_read, 0, PACKET_SIZE);
	memset(d, 0, PACKET_SIZE);
	memset(data_received_buffer, 0, PACKET_SIZE);
	memset(data_sent_buffer, 0, PACKET_SIZE);

	while(1){
		if(messageType == INIT || messageType == ACK){
			memset(dataBuffer, 0, sizeof(dataBuffer[0][0]) * window_size * PACKET_SIZE);
			while(curr_sequence_number - last_sequence_number < window_size){

				if(print_flag == 0){
					printf("Handshake Successful. Sending data to %s\n",address);
					++print_flag;
				}

				in_loop_address = dataBuffer[curr_sequence_number - last_sequence_number];
				//printf("%d %d %d\n",curr_sequence_number, last_sequence_number, window_size);
				messageType = DATA;																					//Set data-type to "Data"
				memcpy(in_loop_address, &messageType, sizeof(int));
				in_loop_address = in_loop_address + sizeof(int);

				//in_loop_address = currAddress;
				bufferSize = 0;

				if(feof(input_file) == 0){
					bytes_read = fread(data_read, sizeof(char), PACKET_SIZE-12, input_file);
					if(bytes_read == 0){
						break;
					}
					curr_pos = ftell(input_file);
					curr_sequence_number++;


					if(ferror(input_file) != 0){
						fprintf(stderr, "Error: %s\n", strerror(errno));
						break;
					} else{
						memcpy(in_loop_address, &curr_sequence_number, sizeof(int));		//Set sequence number

						in_loop_address += sizeof(int);
						memcpy(in_loop_address, &bytes_read, sizeof(int));					//Set file_length

						in_loop_address += sizeof(int);
						memcpy(in_loop_address, data_read, bytes_read);					// Set the characters read from file

						in_loop_address += bytes_read;
						bufferSize = (long) (in_loop_address - dataBuffer[(curr_sequence_number-1) - last_sequence_number]);					// Calculate the size of the buffer.

						enQueue(queue, dataBuffer[(curr_sequence_number-1) - last_sequence_number], bufferSize);
						//printf("In reading loop\n");
						display(queue);
						//printf("buffer pos: %d\n",curr_sequence_number-1 - last_sequence_number);
						//printf("%d\n",bufferSize);
						send_packet(sockID, dataBuffer[(curr_sequence_number-1) - last_sequence_number], bufferSize, address, port);

						if(feof(input_file) != 0){
							break;
						}
					}
				}
				//printf("Reached window_size packets break point\n");
			}
		}

		//printf("before reading from network\n");
		memcpy(data_received_buffer, read_packet(sockID, data_received_buffer, queue, bufferSize, address, port, window_size), ACK_BYTES);
		currAddress = data_received_buffer;
		memcpy(&messageType, currAddress, sizeof(int));							//Get data-type from the acknkowledgment message
		//printf("messageType %d\n",messageType);
		currAddress = currAddress + sizeof(int);
		memcpy(&last_sequence_number, currAddress, sizeof(int));		//Get sequence-number of last packet from the acknkowledgment message
		currAddress += sizeof(int);

		//printf("acknowledment received for %d\n",last_sequence_number);
		// When the sent acknowledment doesn't have the same number as the last frame
		// of the window
		if(curr_sequence_number != last_sequence_number){
			queue = resend(queue, sockID, bufferSize, address, port, window_size);
			continue;
		} else{
			free(queue);
			queue = create_queue();
		}

		if(feof(input_file) != 0){
			if(strcmp(strerror(errno), "Success") == 0)
				printf("File transferred Successfully\n");
			else
				fprintf(stderr, "End of file: %s\n", strerror(errno));

			break;
		}
	}

	memset(dataBuffer, 0, sizeof(dataBuffer[0][0]) * window_size * PACKET_SIZE);
	currAddress = dataBuffer[0];
	messageType = CLOSE;
	memcpy(currAddress, &messageType, sizeof(int));
	currAddress = currAddress + sizeof(int);
	send_packet(sockID, dataBuffer[0], 1, address, port);
	printf("Sending closing packet\n");
	printf("Connection Closed\n");
	close(sockID);
	exit(EXIT_SUCCESS);
}

struct GBN_Queue* resend(struct GBN_Queue *queue, int sockID, long bufferSize, char* address, char* port, int window_size){
	//Variables
	struct GBN_Queue* q = queue;
	struct queue_node *n;
	char data_sent_buffer[window_size][PACKET_SIZE];
	int num, i;

	//Initializations
	memset(data_sent_buffer, 0, sizeof(data_sent_buffer[0][0]) * window_size * PACKET_SIZE);
	num=0;
	i=0;
	while(i < window_size){
		n = deQueue(q);
		memcpy(data_sent_buffer[i], send_packet(sockID, n->data, bufferSize, address, port), PACKET_SIZE);
		memcpy(&num, n->data, sizeof(int));
		//printf("type is %d ",num);
		memcpy(&num, n->data+sizeof(int), sizeof(int));
		//printf("sequence is %d",num);
		memcpy(&num, n->data+sizeof(int)+sizeof(int), sizeof(int));
		//printf("bytes %d \n",num);
		enQueue(queue, n->data, bufferSize);
		display(queue);
		++i;
		// memcpy(d, n->data+sizeof(int)+sizeof(int)+sizeof(int), num);
		// printf("data is \n\n%s\n\n",d);
	}

	return queue;

}

char* send_INIT_Packet(int sockID, int window_size, char* filename, long file_name_length, int file_size, char* address, char* port){
	char dataBuffer[PACKET_SIZE], sentData[PACKET_SIZE], *currAddress;
	char receivedData[PACKET_SIZE];
	int type;
	long buffer_size;


	memset(dataBuffer, 0, PACKET_SIZE);		//Initialize the data-buffer.
	memset(sentData, 0, PACKET_SIZE);		//Initialize the data-buffer.
	memset(receivedData, 0, PACKET_SIZE);		//Initialize the data-buffer.

	type=INIT;

	//INIT Header: Data-type, Mode, file_size, file_name_length, file_name
	currAddress = dataBuffer;
	memcpy(currAddress, &type, sizeof(int));

	currAddress =  currAddress + sizeof(int);
	memcpy(currAddress, &window_size, sizeof(int));

	currAddress = currAddress + sizeof(int);
	memcpy(currAddress, &file_size, sizeof(int));

	currAddress = currAddress + sizeof(int);
	memcpy(currAddress, &file_name_length, sizeof(long));

	currAddress = currAddress + sizeof(long);
	memcpy(currAddress, filename, file_name_length);
	//printf("file_name: %s\n", filename);

	currAddress = currAddress + file_name_length;
	buffer_size = (long)(currAddress - dataBuffer);

	memcpy(sentData, send_packet(sockID, dataBuffer, buffer_size, address, port), buffer_size);
	return read_packet(sockID, receivedData, NULL, buffer_size, address, port, window_size);
}

char* send_packet(int sockID, char* data, long bufferSize, char* address, char* port){
	int bytes_returned = 0;
	struct sockaddr_in ip4addr;
	struct hostent *sender_addr;
	unsigned short int port_in4;
	int var = rand() %10;
	srand((unsigned)time(NULL));

	port_in4 = atoi(port);

	 if ((sender_addr = gethostbyname(address)) == NULL) {  // get the host info
      herror("address resolution failed.\n");
      exit(EXIT_FAILURE);
    }

  ip4addr.sin_family = AF_INET;
	ip4addr.sin_port = htons(port_in4);
	inet_pton(AF_INET, 																		// assign the ip_address to the structure
		inet_ntoa(*(struct in_addr*) sender_addr->h_addr),
		&ip4addr.sin_addr);

	// if(stop_flag < 2 && (incrementer == 3 || incrementer == 30)){
	// 	++stop_flag;
	// 	++incrementer;
	// 	printf("Packet dropped\n");
	// } else{
		++incrementer;
		bytes_returned = sendto(sockID, data, bufferSize, 0, (struct sockaddr *) &ip4addr, sizeof ip4addr);
		if( bytes_returned != bufferSize){
				fprintf(stderr, "partial write: %s %d\n", strerror(errno), bufferSize);
				exit(EXIT_FAILURE);
		}
	//}
		//printf("Packet sent\n");

 	return data;
}

char* read_packet(int sockID, char* readMessage, struct GBN_Queue *q, long bufferSize, char* address, char* port, int window_size){

	struct  timeval timer;
	fd_set set;
	int status=0, retry=0;
	size_t recvBytes;
	time_t timeOutPeriod = 10;

	while(retry <= 4){

		timer.tv_sec = 10;
		timer.tv_usec = 0;

		FD_ZERO(&set);
		FD_SET(sockID, &set);

		status = select(sockID + 1, &set, NULL, NULL, &timer);
		retry++;
		if(status == 0 && retry <= 3){
				printf("CONNECTION TIMEDOUT. Retrying...\n");
				if(q != NULL)
					queue = resend(q, sockID, bufferSize, address, port, window_size);
				//send_packet(sockID, writeMessage, bufferSize, address, port);
				continue;
		} else if(status < 0){
			fprintf(stderr, "%s\n",strerror(errno));
			exit(EXIT_FAILURE);
		} else if(status > 0){
			memset(readMessage, 0, sizeof readMessage);				//Set the memory of dataBuff pointer to point 0;
			if ((recvBytes = read(sockID, readMessage, BUFFSIZE)) > 0){
			 			return readMessage;
	 		}
 		}
 	}
 	printf("Reached maximum number of retries. Exiting the program\n");
 	close(sockID);
 	exit(EXIT_SUCCESS);
}
