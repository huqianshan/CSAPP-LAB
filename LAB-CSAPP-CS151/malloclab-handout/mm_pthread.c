#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
//#include "mm.h"

int size[256] = {1, 23, 5, 12, 6, 1424, 43, 342};

void *Malloc1(){

}

pthread_mutex_t lock;
int share_data;
void * p_lock(void * arg) {
    int i;
    for(i = 0; i < 24; i++) {
        pthread_mutex_lock(&lock);
        share_data++;
        //printf("p_lock +1 %d\n", share_data);
        pthread_mutex_unlock(&lock);
    }
    printf("p_lock finished\n");
    return NULL;
}
void test_lock() {
    pthread_t thread_id;
    void *exit_status;
    int i;
    pthread_mutex_init(&lock, NULL);
    pthread_create(&thread_id, NULL, p_lock, NULL);
    for(i = 0; i < 3; i++) {
        //sleep(0.5);
        pthread_mutex_lock(&lock);
        printf("          Test lock Shared integer's value = %d\n", share_data);
        pthread_mutex_unlock(&lock);
    }
    
    pthread_join(thread_id, & exit_status);
    printf("\n%d\n",share_data);
    pthread_mutex_destroy(&lock);
}


void * hello(void * args) {
    long  rank = (long) args;
    printf("Hello form sub thread %ld\n", rank);
    return NULL;
}

#define PRED(ptr) (*(char *)(ptr))
#define PRED_PTR(ptr) ((char *)(ptr))
int main() {
    /*int thread_num = 5;
    long thread_index;
    pthread_t * thread_handles;
    thread_handles =(pthread_t *) malloc(sizeof(pthread_t ) * thread_num);
    
    for(thread_index = 0; thread_index < thread_num; thread_index++) {
        pthread_create(&thread_handles[thread_index], NULL, test_lock,NULL);
    }
    printf("hello from main thread\n");
    for(thread_index = 0; thread_index < thread_num; thread_index++) {
        pthread_join(thread_handles[thread_index], NULL);
    }
    free(thread_handles);*/
    //test_lock();
    //return 0;
    int s = 123;
    void *ptr;
    ptr = &s;
    printf("%x\n", ptr);

    printf("%x\n", PRED_PTR(ptr)+1);

    printf("%x\n", (char **)ptr+1);

    printf("%d\n", PRED(ptr));


    

}