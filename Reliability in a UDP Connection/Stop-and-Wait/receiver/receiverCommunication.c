//////////////////////////////////////////////////////////////////////////

char* tokenizer(char*, int);
void sendAck(int, int, char*, struct sockaddr_storage, socklen_t);
void writetoFile(char*, char*, long, FILE*);

int receiverCommunication(int sockID, char* mode){

	//-------------------VARIABLE DECLARATIONS ----------------------//

	//Variables
	const int BUFFSIZE = 512;
	int p=0, prev_seq =0, curr_seq=0;;
	char packet_type;
	int data_length = 0;
	long mode_length;
	int window_size=0, receiver_window_size=0;

	//Buffers
	char dataBuff[BUFFSIZE], dataSent[BUFFSIZE], data[BUFFSIZE];
	char *currAddress;

	//Host Address
	struct sockaddr_storage peer_addr;
	socklen_t peer_addr_len;

	//File
	ssize_t sentBytes, recvBytes;
	int file_length = 0;
	long file_name_length;
	FILE *writeFile;
	pid_t processID;
	char file_name[MAX_FILE_SIZE], new_name[MAX_FILE_SIZE];
	
	
	//---------------END VARIABLE DECLARATIONS ----------------------//

	while(sockID != -1){
		
		memset(dataSent, 0, BUFFSIZE);
		memset(dataBuff, 	0, BUFFSIZE);

		if((recvBytes = recvfrom(sockID, dataBuff, BUFFSIZE, 0, (struct sockaddr *) &peer_addr, &peer_addr_len)) == -1){
				fprintf(stderr, "Ignoring failed request: %s\n", strerror(errno));
				continue;
		}

		receiver_window_size = atoi(mode);

        /*char host[NI_MAXHOST], service[NI_MAXSERV];
		p = getnameinfo((struct sockaddr *) &peer_addr,
                               peer_addr_len, host, sizeof(host),
                               service, sizeof(service),NI_NUMERICHOST | NI_NUMERICSERV);

		if(p==0){
			printf("Received data from %s\n", host);
		} else
               fprintf(stderr, "getnameinfo: %s\n", gai_strerror(p));*/		//Uncomment if the sender address details are to be printed.

        currAddress = dataBuff;
		memcpy(&packet_type, currAddress, sizeof(char));						
		currAddress += sizeof(char);

		switch(packet_type){
			case '0': 
			{																		//INIT Message
				memset(file_name, 0, sizeof file_name);
				memset(new_name, 0, sizeof new_name);

				memcpy(&file_length, currAddress, sizeof(int));					 	// header packet: file_length
				currAddress += sizeof(int);

				memcpy(&file_name_length, currAddress, sizeof(long));				// header packet: file_name_length
				currAddress += sizeof(long);

				memcpy(&window_size, currAddress, sizeof(int));						// window_size of the sender.
				currAddress += sizeof(int);

				memcpy(&file_name, currAddress, file_name_length);					// header packet: file name
				currAddress += mode_length;

				processID = getpid();
				sprintf(new_name, "%d%s%s", processID,"-", file_name);				// append pid() to the given filename

				if(window_size != receiver_window_size){							// When receiver and sender window size doesn't match
					printf("%d\n", receiver_window_size);
					printf("Window size of sender and receiver do not match\n");    // For Stop-and-Wait, the window size should be 1.
					close(sockID);
					exit(EXIT_FAILURE);
				} else{
					sendAck(sockID, curr_seq, dataSent, peer_addr, peer_addr_len);      // Send acknowledgment for the packet received.
				}
			}
			break;

			case '1':
				{																	// DATA Message
					memset(data, 0, BUFFSIZE);

					memcpy(&data_length, currAddress, sizeof(int));					// Data length in the packet
					currAddress += sizeof(int);

					memcpy(&curr_seq, currAddress, sizeof(int));					// Sequence number of the packet
					currAddress += sizeof(int);

					memcpy(&data, currAddress, data_length);						// Data in the current packet
					currAddress += data_length;
					
					if(curr_seq == (prev_seq+1)){											// Checking if the packets are in order.
						writetoFile(new_name, data, data_length, writeFile);				// Writing the data to file.
						sendAck(sockID, curr_seq, dataSent, peer_addr, peer_addr_len);		// and sending acknowledgment for the same packet
						prev_seq++;
					} else
						sendAck(sockID, prev_seq, dataSent, peer_addr, peer_addr_len);		// If the packet is not in order, 
																							// send acknowledgment for the previous packet
				}
			break;

			case '3':
				{
					close(sockID);
					printf("Connection closing. Initiated by sender\n");
					exit(EXIT_SUCCESS);
				}
			break;

			default:
				{
					printf("Connection closing. Unknown Packet type\n");
					sendAck(sockID, prev_seq, dataSent, peer_addr, peer_addr_len);		// If the packet is not in order, 
																							// send acknowledgment for the previous packet
				}
		}

	}

	exit(EXIT_SUCCESS);
}

void writetoFile(char* file_name, char* data, long data_length, FILE* writeFile){
			if((writeFile = fopen(file_name, "a")) == NULL){
				fprintf(stderr, "%s\n", strerror(errno));
				exit(EXIT_FAILURE);
			}

			if(feof(writeFile) == 0){
				fwrite(data,sizeof(char), data_length, writeFile);					// Writing data to the file.
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

void sendAck(int sockID, int sequence, char* dataSent, struct sockaddr_storage peer_addr, socklen_t peer_addr_len){
			const int ack_bytes = 5;											// ack always sends acknowledgment message type and 
																				// sequence number. So, sizeof(char) + sizeof(int) = 5;
			memset(dataSent, 0, sizeof dataSent);
        	dataSent[0] = '2';
        	memcpy(dataSent+sizeof(char), &sequence, sizeof(int));

        	if(ack_bytes != sendto(sockID, dataSent, ack_bytes, 0, (struct sockaddr *) &peer_addr, peer_addr_len)){
        		fprintf(stderr, "Error sending response\n");
        	}
}