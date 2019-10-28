#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

void add(int*, int);
bool has(int*, int);
void display(int*);
void initialize(int**);

int *sequences = NULL;
int sizeofArray=0;

int main(){
  int n;
  while(1){
    printf("Enter the number to add: ");
    scanf("%d", &n);
    if(!has(sequences, n)){
      initialize(&sequences);
      add(sequences, n);
      display(sequences);
    } else
        printf("\nValue is already present in the network\n");
  }

  return 0;
}

void initialize(int **arr){
  *arr = (int*) realloc(*arr, sizeof(int));
  printf("Memory allocated\n");
  return;
}

void add(int* arr, int val){
  arr[sizeofArray++] = val;
  printf("No. of Elements of array %ld\n",sizeofArray);
}

bool has(int* arr, int val){
  int i;

  if(arr == NULL)
    return false;

  for(i=0;i<sizeofArray;i++){
    if(arr[i] == val)
      return true;
  }

  return false;
}

void display(int* arr){
  int i;

  printf("Elements of array: ");
  for(i= 0; i< sizeofArray; i++){
    printf("%d ",arr[i]);
  }
  printf("\n\n");
}
