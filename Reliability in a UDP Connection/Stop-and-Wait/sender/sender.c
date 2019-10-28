#include<stdio.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<errno.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<netdb.h>
#include<netinet/in.h>
#include<unistd.h>
#include<time.h>
#include<sys/stat.h>
#include<sys/select.h>

#define PORT "15095"
#define WAIT_CONN 0
#define BUFFSIZE 500
#define PACKET_SIZE 512
#define INIT 0
#define DATA 1
#define ACK 2

#include "senderCommunication.c"

//function declarations
int initializeConnections(struct addrinfo * );

int main(int argc, char *argv[])
{

//----------------------Variable Declarations------------------------//
 struct addrinfo Caddr;
 struct addrinfo *rlist;
 int status, socket_desc; 
 char mode[50], filename[255], address[255], port[5];
 char hostname[255];



//-------------------------Initializing the Struct addrinfo--------------------------//

 memset(&Caddr, '\0', sizeof Caddr); 	//Set 0 to Caddr for sizeof(struct addrinfo)  bits
 Caddr.ai_family = AF_INET;		//For IPv4 or IPv6
 Caddr.ai_socktype = SOCK_DGRAM;	//UDP	 Stream
 //Caddr.ai_flags = AI_PASSIVE;		// Self-IP
 //Caddr.ai_flags = AI_NUMERICHOST;     // For numerical network address.
//// Caddr.ai_flags = 0;
 Caddr.ai_flags = AI_CANONNAME;		//For looking up the canonical name of the DNS Server.
 Caddr.ai_protocol = 0;
//---------------------------Handle Command line arguments---------------------------//

memset(filename, 0,255);
memset(mode, 0, 50);
memset(address, 0,255);
memset(port, 0,0);
memset(hostname, 0,255);

 if(argc < 5){
  fprintf(stderr, "Usage: ./sender <-m mode> <-p port> <-h hostname> <-f filename> \n");
  exit(EXIT_FAILURE);
 }
 else{	
		if(argc == 5){

			gethostname(hostname, 255);

			 if(status = (getaddrinfo(hostname, PORT, &Caddr, &rlist)) != 0){		//getaddrinfo function initializes the Struct addrinfo with relevat data
				fprintf(stderr,  "getaddrinfo error: %s\n", strerror(status));
				exit(EXIT_FAILURE);
		 	}
        	if(strlen(argv[1]) > 1){
        		printf("The maximum window size is 1\n");
        		exit(EXIT_FAILURE);
        	} else{
        		strcpy(mode, argv[1]);
        		strcpy(port, argv[2]);
        		strcpy(address, argv[3]);
        	}

        if(strlen(argv[4]) > 255){
        	fprintf(stderr, "Filename greater than permitted length: 255\n");
        	exit(EXIT_FAILURE);
        } else{
        		strcpy(filename, argv[4]);
      		  }
		}
     } 

//-----------------Call to Functions to Bind, Listen and Calaculate-----------------//

 socket_desc = initializeConnections(rlist);  //Initialize the socket, connect on the Port
 senderCommunication(socket_desc, mode, filename, address, port);	      //Receive Data from the server and send data to the server.	
 freeaddrinfo(rlist);			      //free the memory occupied by the Struct addrinfo
 exit(EXIT_SUCCESS);
}


 int initializeConnections(struct addrinfo *res){
	int optName_value= 1;
	struct addrinfo *p; 
	int socket_desc;

	//loop through the results list for addresses
	for(p = res ; p != NULL ; p = p -> ai_next){
		
		//make a socket
  		if((socket_desc = socket(p -> ai_family, p-> ai_socktype, p -> ai_protocol))== -1){
			fprintf(stderr, "Call to the function Socket(): %s\n", strerror(errno));
			continue;
  		}
		
		//manipulate the socket to release the "PORT"
		if (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &optName_value, sizeof(int)) == -1) {
            		perror("setsockopt");
            		exit(EXIT_FAILURE);
        }

        if((bind(socket_desc, p->ai_addr, p->ai_addrlen))== 0){
			fprintf(stderr, "Bind: %s\n", strerror(errno));
			break;
 		} else{
 			fprintf(stderr, "Bind: %s\n", strerror(errno));
 			continue;
 		}

	close(socket_desc);		
	}

	//check for address list
 	if(p == NULL){
		fprintf(stderr, "Connection failed\n");
		exit(EXIT_FAILURE);
 	}

  return socket_desc;	
 }
