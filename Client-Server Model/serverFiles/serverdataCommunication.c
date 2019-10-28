char* tokenizer(char*, int);

int serverdataCommunication(int clientID, char* NUID, char* secret_flag){

	//-------------------VARIABLE DECLARATIONS ----------------------//

	int sentBytes, recvBytes;
	const int BUFFSIZE = 500;
	int i, p=0, numSolution = 0;
	int loopSol = rand() % 5;
	int operand1, operator, operand2;

	char *dataSent, dataBuff[BUFFSIZE], *dataReceived[BUFFSIZE];
	char **solutions;
	char *pointerCh;

	srand((unsigned)time(NULL));
	
	char c1, c2, c3;
	char *operators[4] = {"+", "-", "*", "/"};
	
	//---------------END VARIABLE DECLARATIONS ----------------------//

	while(clientID != -1){
		solutions  = (char**) malloc((p+1) * sizeof(char));			//dynamically assign memory to solutions[] for
											//for stroing solutions for any future use.

		memset(dataReceived, 0, sizeof dataReceived);
		memset(dataBuff, 0, sizeof dataBuff);
		//printf("Waiting to receive...\n");
		if((recvBytes = recv(clientID, dataBuff, BUFFSIZE-1, 0)) == -1){	//receive data from the client: HELLO Message	
			fprintf(stderr, "Reading error: %s\n", strerror(errno));
 		}
		//printf("%s\n", dataBuff);

		//split it into parts
		if(dataBuff != NULL){
			i=0;
			dataBuff[recvBytes - 1] = 0;
			//printf("%s\n", dataBuff);
		//	printf("Loop: %d:  ", i);
		 	pointerCh = strtok(dataBuff, " ");
			while(pointerCh != NULL){
				dataReceived[i] = pointerCh;
		//		printf("token: %d: %s\n", i, dataReceived[i]);
				pointerCh = strtok(NULL, " ");	
				i++;		
 			}	
		}

		//sprintf(*dataReceived, "%s", tokenizer(dataBuff, recvBytes));

		//printf("Exited split parts  if  loop\n");
		//printf("dataReceived[1]: %s %ld\n",dataReceived[1], strlen(dataReceived[1]));
		if(strcmp(dataReceived[1], "HELLO")==0){
			while(numSolution < loopSol){
				operand1 = rand() % 1000;
				operand2 = rand() %  1000;
				operator = rand() % 3;
		//		printf("Entered \"HELLO\" \n");  
		//		printf("operand1: %d\n", operand1);
		//		printf("operand2: %d\n", operand2);
		//		printf("operator: %s\n", operators[0]);
				dataSent = NULL;
				dataSent = (char*) malloc(100);
				sprintf(dataSent, "%s%d%s%s%s%d%s", "cs5700fall2017 STATUS ", operand1, " ", operators[operator], " ", operand2, "\n");
		//		printf("Data being  sent: %s\n", dataSent);
				//Sending Status Message.
				if((sentBytes = send(clientID, dataSent, strlen(dataSent), 0)) == -1){
					fprintf(stderr, "Send error: %s\n", strerror(errno));
					return 1;
				}
				//Receiving Result Message.
				if((recvBytes = recv(clientID, dataBuff, BUFFSIZE-1, 0)) == -1){
					fprintf(stderr, "Reading error: %s\n", strerror(errno));
 				}
				strcpy(*dataReceived, tokenizer(dataBuff, recvBytes));
				solutions[p] = dataReceived[1];
		//		printf("Solution %d is: %s\n", p, solutions[p]);
				p++;
			numSolution++;
			}
		} 
		//printf("Skipped if statement\n");
		

		dataSent = NULL;
		dataSent = (char*) malloc(200);
		sprintf(dataSent, "%s%s%s", "cs5700fall2017 ", secret_flag, " BYE\n");
		printf("Final data being sent: %s\n", dataSent);
	//	printf("clientID as of now: %d\n", clientID);
		if((sentBytes = send(clientID, dataSent, strlen(dataSent), 0)) == -1){
			fprintf(stderr, "Send error: %s\n", strerror(errno));
			return 1;
		}
		close(clientID);
		exit(EXIT_SUCCESS);
		
	}

        return 1;
 }

char* tokenizer(char* data, int bytes){

	char* datatoken[500];
	int i;
	char *pointerCh;
	
	memset(datatoken, 0, sizeof datatoken);
	//printf("Entered tokenizer\n");
	if(data != NULL){
		i=0;
		data[bytes - 1] = 0;
//		printf("%s\n", data);
		//printf("Loop: %d:  ", i);
		pointerCh = strtok(data, " ");
		while(pointerCh != NULL){
			datatoken[i] = pointerCh;
//			printf("token: %d: %s\n", i, datatoken[i]);
			pointerCh = strtok(NULL, " ");
			i++;
		}
	}
//	printf("%s", *datatoken);
	return *datatoken;
}	

