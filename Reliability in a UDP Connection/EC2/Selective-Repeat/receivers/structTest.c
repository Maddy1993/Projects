#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "structure.h"

int main(){
  char val[10];
  int n=0;
  memset(val, 0, 10);
  struct GBN_Receiver_Queue *q = create_queue();

  while(1){
    printf("Enter the value to store: ");
    scanf("%d", &n);
    printf("\n");
    memcpy(val, &n, sizeof(int));
    memcpy(val+sizeof(int), &n, sizeof(int));
    printf("Value copied\n");
    enQueue(q, val, 8);
    display(q);
  }
}
