#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

struct check{
	int i;
};

void test();
void test1(struct check *);
void test2(struct check *);
void* runner(void *);

pthread_mutex_t mutexsum;

int main(){
	
	struct check *p;
	test1(p);
	return 0;
}

void  test1(struct check *p){
	p = malloc(sizeof(struct check));
	p->i = 1;
	test2(p);
}

void test2(struct check *p){
	pthread_t thread;
 	pthread_attr_t attr;
	int rc;
	int counter=0;

 	pthread_mutex_init(&mutexsum, NULL);

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

   rc = pthread_create(&thread, &attr, runner, p);
   if (rc){
      printf("ERROR; return code from pthread_create() is %d\n", rc);
      exit(-1);
   }

   pthread_attr_destroy(&attr);

   for(int  i=0;i<100;i++){
   	printf("thread %d %d\n", i, p->i);
   }

   rc = pthread_join(thread, NULL);

   if (rc) {
      printf("ERROR; return code from pthread_join() is %d\n", rc);
      exit(-1);
  	}


    /* Last thing that main() should do */
 	pthread_mutex_destroy(&mutexsum);
 	//free(p);
    //pthread_exit(NULL);
}

void *runner(void* counter){
	struct check* c = (struct check*)counter;


	pthread_mutex_lock (&mutexsum);
	c->i = 11;
	for(int  i=0;i<10;i++){
   	printf("Hello World! It's me, 1thread %d\n", c->i);
   }
 	pthread_mutex_unlock(&mutexsum);

    pthread_exit(NULL);
}