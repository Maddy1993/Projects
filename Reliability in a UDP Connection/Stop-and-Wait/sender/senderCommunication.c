//----------------------Functions Delcarations-----------------------//

int sendData(char*, int);
int makeCalculations(char* , char* , char*);
void isitDigit(char*);
void clientdataCommunication(int, char*);
void writeData(int, char*, clock_t*, long, char*, char*);
char* readData(int, char*, char*, clock_t*, long, char*, char*);


void senderCommunication(int sockID, char* mode, char* filename, char* address, char* port){

	//Variable Declarations.
	const int CONTROL_PACKET_LENGTH = 9;
	const int ACK_SIZE = 5;
	size_t len, mode_len;
	clock_t *beginTime;


	//Buffers
	char writedataBuff[PACKET_SIZE], readDataBuff[PACKET_SIZE], dataRead[PACKET_SIZE], *dataSent;
	long bufferSize;

	//File
	FILE *readFile;
	int file_size = 0;
	int bytes_read = 0;
	int curr_pos = 0, remainingChar = 0;
	int window_size = 0;

	//Acknowledgment and Sequence Number
	char messageType, *curAddress;
	int ack_seq=0, curr_seq=0, sequence_number=0;

	//-----End Declarations--------//

    while(sockID !=  -1){

		memset(writedataBuff, 0, PACKET_SIZE);				    //Set the memory of write data buffer pointer to point 0;
		memset(readDataBuff, 0, PACKET_SIZE);				    //Set the memory of read data buffer  pointer to point 0;
		memset(dataRead, 0, PACKET_SIZE);				       //Set the memory of data read from file pointer to point 0;

		window_size = atoi(mode);								//Convert mode value to an integer.

		if((readFile = fopen(filename, "r")) == NULL){			//Open the given input file
			fprintf(stderr, "%s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}

		//Defining INIT Message
		{
			messageType = '0';
			curAddress  = writedataBuff;
			
			memcpy(curAddress, &messageType, sizeof(char));			////Data-type: 0 for INIT
																	//			 1 for Data
																	//           2 for ACK
																	//           3 for Closing Packet
			curAddress += sizeof(char);

			fseek(readFile, 0, SEEK_END);
			file_size = ftell(readFile);
			fseek(readFile, 0, SEEK_SET);

			memcpy(curAddress, &file_size, sizeof(int));			// file size of the given input file
			curAddress += sizeof(int);

			len = strlen(filename);
			mode_len= strlen(mode);
			
			memcpy(curAddress, &len, sizeof(long));					// length of file name 
			curAddress += sizeof(long);

			memcpy(curAddress, &window_size, sizeof(int));			// window size: equal to 1 for stop and wait
			curAddress += sizeof(int);

			memcpy(curAddress, filename, strlen(filename));			// filename of the given input file
			curAddress += strlen(filename);
		}

		bufferSize = (long)(curAddress - writedataBuff);

		//send the messages to the receiver
		writeData(sockID, writedataBuff, beginTime, bufferSize, address, port);

		memcpy(readDataBuff, readData(sockID, readDataBuff, writedataBuff, beginTime, bufferSize, address, port), ACK_SIZE);
		memcpy(&messageType, readDataBuff, sizeof(char));

		while(1){
				if(messageType == '2' && (ack_seq == curr_seq)){

						memset(writedataBuff, 0, PACKET_SIZE);				   
						memset(readDataBuff, 0, PACKET_SIZE);				  
						messageType = '1';								//Set data-type to 1: representing the data
						remainingChar = (file_size - curr_pos);

						curAddress = writedataBuff;

						if(feof(readFile) == 0){

								curr_seq++;
								bytes_read = fread(dataRead, sizeof(char), PACKET_SIZE- CONTROL_PACKET_LENGTH, readFile);
																									// Control Packet Size includes: 
																									// messageType
																									// bytes read from the file descriptor
																									// current sequence number
																									// data read from the file
								if(ferror(readFile) != 0){
									fprintf(stderr, "Error: %s\n", strerror(errno));
									break;
								} else{
									memcpy(curAddress, &messageType, sizeof(char));
									curAddress += sizeof(char);

									memcpy(curAddress, &bytes_read, sizeof(int));
									curAddress += sizeof(int);

									memcpy(curAddress, &curr_seq, sizeof(int));
									curAddress += sizeof(int);

									memcpy(curAddress, dataRead, bytes_read);
									curAddress += bytes_read;

									bufferSize = (long)(curAddress - writedataBuff);
									writeData(sockID, writedataBuff, beginTime, bufferSize, address, port);		// Write data read to the socket
								}
						}

						if(feof(readFile) != 0){
							fprintf(stderr, "End of file: %s\n", strerror(errno));
							break;																	// Break the loop when End-of-File reached.
						} 

						curr_pos = ftell(readFile);

						memcpy(readDataBuff, readData(sockID, readDataBuff, writedataBuff, beginTime, bufferSize, address, port), ACK_SIZE);

						memcpy(&messageType, readDataBuff, sizeof(char));
						memcpy(&ack_seq, readDataBuff + sizeof(char), sizeof(int));
						
				} else if(messageType == '2' && ack_seq != curr_seq){								//When message received is an acknowledgment
						printf("Acknowledgments did not match\n");                                  // and when ack sequence and current sequence 
						printf("Resending %d because acknowledgment received is: %d\n", curr_seq,ack_seq);             // are not equal
						writeData(sockID, writedataBuff, beginTime, bufferSize, address, port);		// Write data read to the socket
						break;
				} else{
					printf("Unknown Header Packet. Ending program.\n");
					close(sockID);
					exit(EXIT_FAILURE);
					break;
				}
		}

		memset(writedataBuff, 0, PACKET_SIZE);
		curAddress = writedataBuff;
		messageType = '3';									//Closing the connection
		memcpy(curAddress, &messageType, sizeof(char));
		curAddress = curAddress + sizeof(char);
		bufferSize = (long)(curAddress - writedataBuff);

		writeData(sockID, writedataBuff, beginTime, bufferSize, address, port);		// Write data read to the socket

		printf("Sending closing packket\n");

 		close(sockID);																				// close socket when the pointer is out the loop
 		printf("Socket Closed\n");																	// and exit the program.
		exit(EXIT_SUCCESS);
	}
}

void writeData(int sockID, char* data, clock_t* beginTime, long bufferSize, char* address, char* port){

	struct sockaddr_in ip4addr;
	struct hostent *sender_addr;
	unsigned short int port_in4;

	port_in4 = atoi(port);

	 if ((sender_addr = gethostbyname(address)) == NULL) {  // get the host info
        herror("address resolution failed.\n");
        exit(EXIT_FAILURE);
    }

    ip4addr.sin_family = AF_INET;
	ip4addr.sin_port = htons(port_in4);
	inet_pton(AF_INET, 										// assign the ip_address to the structure
		inet_ntoa(*(struct in_addr*) sender_addr->h_addr), 
		&ip4addr.sin_addr);

	if(bufferSize != sendto(sockID, data, bufferSize, 0, (struct sockaddr *) &ip4addr, sizeof ip4addr)){
		fprintf(stderr, "Error sending response\n");
	}

 	beginTime = (clock_t*)clock();
}

char* readData(int sockID, char* readMessage, char* writeMessage, clock_t* beginTime, long bufferSize, char* address, char* port){

	struct  timeval timer;
	fd_set set;
	int status=0, retry=0;
	long recvBytes;
	time_t timeOutPeriod = 5;

	while(retry <= 4){

		timer.tv_sec = timeOutPeriod;

		FD_ZERO(&set);
		FD_SET(sockID, &set);

		status = select(sockID + 1, &set, NULL, NULL, &timer);
		retry++;
		if(status == 0 && retry <= 3){
				printf("CONNECTION TIMEDOUT. Retrying...\n");
				writeData(sockID, writeMessage, beginTime, bufferSize, address, port);
				continue;
		} else if(status < 0){
			fprintf(stderr, "%s\n",strerror(errno));
			exit(EXIT_FAILURE);
		} else if(status > 0){
			memset(readMessage, 0, PACKET_SIZE);				//Set the memory of dataBuff pointer to point 0;
			recvBytes = read(sockID, readMessage, PACKET_SIZE);
			if (recvBytes > 0){
		 			return readMessage;
			 } else{
			 	fprintf(stderr, "%s\n", strerror(errno));
			 	continue;
			 }
 		}
 	}
 	printf("Reached maximum number of retries. Exiting the program\n");
 	close(sockID);
 	exit(EXIT_SUCCESS);
}