//----------------------Functions Delcarations-----------------------//

int sendData(char*, int);
int makeCalculations(char* , char* , char*);
void isitDigit(char*);
void clientdataCommunication(int, char*);



void clientdataCommunication(int sockID, char* NUID){

	//Variable Declarations.
	char dataBuff[BUFFSIZE], *dataReceived[BUFFSIZE], *dataSent;
	char *pointerCh;
	int result, i;
	int sentBytes, recvBytes;
	int operator = 3, number1 = 2, number2 = 4;


	//initiate the connection
	dataSent = NULL;
	dataSent = (char*) malloc(500);	
	sprintf(dataSent,"%s%s%s","cs5700fall2017 HELLO ", NUID, "\n");
	if((sentBytes = sendData(dataSent, sockID)) == -1){	 		
		fprintf(stderr, "Send error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
 	}	

	//receive data and split it into parts based on spaces.
        while(sockID !=  -1)
	{
	      	memset(dataReceived, 0, sizeof dataReceived);			//Set the memory of dataReceived array pointer to point 0;
		memset(dataBuff, 0, sizeof dataBuff);				//Set the memory of dataBuff pointer to point 0;

		//receive the messages from the server
 		if((recvBytes = recv(sockID, dataBuff, BUFFSIZE-1, 0)) == -1){
			fprintf(stderr, "Reading error: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
	 	}	
		
		//split it into parts
		if(dataBuff != NULL){
			i=0;
			dataBuff[strlen(dataBuff) - 1] = 0;
		 	pointerCh = strtok(dataBuff, " ");
			while(pointerCh != NULL){
				dataReceived[i] = pointerCh;
				pointerCh = strtok(NULL, " ");			
				i++;
 			}	
		}

		//check if it is STATUS message or BYE message
		if((strcmp(dataReceived[0], "cs5700fall2017")==0) && (strcmp(dataReceived[1], "STATUS") == 0)){
			dataSent = NULL;
			dataSent = (char* )malloc (500);
			result = makeCalculations(dataReceived[number1], dataReceived[operator], dataReceived[number2]);
                        sprintf(dataSent, "%s%d%s", "cs5700fall2017 ", result, " \n");
			if((sentBytes = sendData(dataSent, sockID)) == -1){	 		
				fprintf(stderr, "Send error: %s\n", strerror(errno));
				exit(EXIT_FAILURE);
 			}
		}
		else {
			//Handle "Bye" Message and Close the connection on receiving it.
			if((strcmp(dataReceived[0], "cs5700fall2017")==0) && (strcmp(dataReceived[2], "BYE") == 0)){
				 printf("%s\n", dataReceived[1]);
				 close(sockID);
                                 exit(EXIT_SUCCESS);
		        } else{
				close(sockID);
				printf("Error: Unidentified data sent\n");
				exit(EXIT_FAILURE);
			  }	
			
	       }
	}
exit(EXIT_SUCCESS);
 
}

int sendData(char* message, int sockID)
{
	return send(sockID, message, strlen(message), 0);
}

//The function to check whether the given operand is a number or not.
void isitDigit(char* val){
	int i=0;

	while(val[i]){
		if(!(val[i] >= '0' && val[i] <= '9')){
			printf("Error: Operands must be an Integer\n");
			exit(EXIT_FAILURE);		
		}
		i++;
	}
}

//function to perform calculations on input given
int makeCalculations(char* value1, char* value2, char* value3)
{
	
	isitDigit(value1);
	isitDigit(value3);	
	int operand1 = atoi(value1);
	int operand2 = atoi(value3);
	int i=0,m;
	int round, result;

	m = value2[0];

	switch(m){
		case 43: result = (operand1 + operand2);
	       		 return result;
			 break;
		case 45: result = (operand1 - operand2);
			 return result;
			 break;
		case 42: result = (operand1 * operand2);
			 return result;
  			 break;
		case 47: round = (operand1 / operand2);
			 return round;
			 break;
		default:
		printf("Error: Operator is out of the scope\n");
		exit(EXIT_FAILURE);
	}
 return 0;
}


