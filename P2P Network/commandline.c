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
#include<openssl/sha.h>
#include<openssl/bn.h>


#include "structure.h"
#include "convert.h"

#define UNUSED(x) (void)(x)

void test(char*, char*);

int main(int argc, char* argv[]){

	//Implementation-1
	//system("ls");

	//Implementation-2
	/*FILE *pf;
    char command[100];
    char data[512];

    // Execute a process listing
    sprintf(command, "echo -n 10.110.120.180 | sha1sum | awk '{print $1}'");

    // Setup our pipe for reading and execute our command.
    pf = popen(command,"r");

    // Error handling

    // Get the data from the process execution
    fgets(data, 512 , pf);
    printf("%s\n", data);

    // the data is now in 'data'

    if (pclose(pf) != 0)
        fprintf(stderr," Error: Failed to close command stream \n");

    return;*/

    //Implementation-3
    test(argv[1], argv[2]);

	return 1;
}

void test(char* ip1, char* ip2){
	char ibuf[100];
	unsigned char obuf[20];

	BIGNUM *bn =NULL;
	bn = BN_new();
	BIGNUM *bn1 = NULL;
	bn1 = BN_new();

	memset(ibuf, 0, 100);
	memcpy(ibuf, ip1, strlen(ip1));
	printf("INput string lengith:%s %ld\n",ibuf, strlen(ibuf));
	SHA1((unsigned char*)ibuf, strlen(ibuf), obuf);

	int i;
	for(i=0;i<20;i++){
		printf("%x", obuf[i]);
	}
	printf("\n");

	BN_dec2bn(&bn, (char*)obuf);
	BN_print_fp(stdout, bn);
	printf("\nNext number: \n");

	memset(ibuf, 0, 100);
	memcpy(ibuf, ip2, strlen(ip2));
	printf("INput string length:%s size: %ld\n",ibuf, strlen(ibuf));
	SHA1((unsigned char*)ibuf, strlen(ibuf), obuf);

	for(i=0;i<20;i++){
		printf("%x", obuf[i]);
	}
	printf("\n");

	BN_dec2bn(&bn1, (char*)obuf);
	BN_print_fp(stdout, bn1);

	printf("\n");

	int result = BN_cmp(bn, bn1);
	if(result == -1){
		printf("less-than\n");
	} else if(result == 0){
		printf("Equal\n");
	} else if(result == 1){
		printf("Greater\n");
	}
}
