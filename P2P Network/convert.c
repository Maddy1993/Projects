/*This code snippet is developed by referencing jayjayswal/stohi.c @
https://gist.github.com/jayjayswal/fc435fe261af9e45ccaf
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>

#define IP_ADDR_SEGMENTS 4

union
{
    unsigned int numeric_value;
    unsigned char byte[4];
} myP;

char* ip_int_to_str(unsigned int addr){

	char* return_value = malloc(sizeof (char) * 100);
	sprintf(return_value, "%u",addr);
	//memset(return_value,0, 100);

	myP.numeric_value = addr;
	
	sprintf(return_value, "%u%c%u%c%u%c%u", myP.byte[0],'.', myP.byte[1], '.', myP.byte[2], '.', myP.byte[3]);

    return return_value;
}

int ip_str_to_int(char* ip_addr){
	char numeric_value;
	numeric_value = *ip_addr;

	unsigned int result;
	int numeric_val;
	int i,j=0;

	for (j=0;j<IP_ADDR_SEGMENTS;j++) {
		if (!isdigit(numeric_value)){  //first numeric_valuehar is 0
			return (0);
		}
		numeric_val=0;
		for (i=0;i<3;i++) {
			if (isdigit(numeric_value)) {
				numeric_val = (numeric_val * 10) + (numeric_value - '0');
				numeric_value = *++ip_addr;
			} else
				break;
		}
		if(numeric_val<0 || numeric_val>255){
			return (0);	
		}	
		if (numeric_value == '.') {
			result=(result<<8) | numeric_val;
			numeric_value = *++ip_addr;
		} 
		else if(j==3 && numeric_value == '\0'){
			result=(result<<8) | numeric_val;
			break;
		}
			
	}
	if(numeric_value != '\0'){
		return (0);	
	}
	return (htonl(result));
}