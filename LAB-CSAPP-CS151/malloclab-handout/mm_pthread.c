#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <time.h>
#include "mm.h"

#define TRUE 1
#define CAS(ptr,oldvalue,newvalue) __sync_bool_compare_and_swap(ptr,oldvalue,newvalue)

// get the thread id of current thread by syscall
pid_t get_tid(){
    return syscall(__NR_gettid);
}

struct Node{
    int data;
    struct Node* next;
};
typedef struct Node Node, *Nodeptr;

typedef struct{
    Nodeptr head;
    Nodeptr tail;
} Queue,*Queueptr;

Queueptr queueNew(void){
    Nodeptr tmp = malloc(sizeof(Node));
    Queueptr que = malloc(sizeof(Queue));
    que->head = que->tail = tmp;
    return que;
}
void queueFree(Queueptr myque){

   free(myque->tail);
   free(myque);
}

#define MAX_THREAD 2
#define NUM 2
int c = 0;
int d = 0;
Queueptr myque = NULL;
pthread_t push_tid[MAX_THREAD]={0};
pthread_t pop_tid[MAX_THREAD]={0};

void test_malloc(void){
    /*
    srand((unsigned int)time(NULL));
    void *arr[10] = {0};
    for (int i = 0; i < NUM; i++)
    {
        int size = rand() % 50;
        arr[i] = mm_malloc(size);
        printf("tid %d malloc size %d at addr %x\n", get_tid(),size,arr[i]);
        sleep(rand() % 2);
    }


    for (int i = 0; i < NUM;i++){
        mm_free(arr[i]);
        //Nodeptr end = myque->tail;
        //int tem = queuePop();
        printf("           tid %d free at addr %x\n", get_tid(),arr[i]);
        sleep(rand() % 2);
    }*/
    int size = rand() % 20;
    void *ptr = pp_malloc(size);
    printf("tid %d malloc size %d at addr %x\n", get_tid(),size,ptr);
    sleep(rand() % 2);
    pp_free(ptr);
    printf("           tid %d free at addr %x\n", get_tid(),ptr);
}

void queuePush(int data){
    Nodeptr p;
    Nodeptr oldp;
    //1. get space
    Nodeptr newNode = malloc(sizeof(Node));
    newNode->data = data;
    newNode->next = NULL;

    //2. add newNode to que's end
    p = myque->tail;
    oldp = p;
    do{
        while(p->next!=NULL){
            p = p->next;
        }
    } while (CAS(&p->next, NULL, newNode) != TRUE);

    //3.update the queue tail ptr to newNode
    CAS(&myque->tail, oldp, newNode);
    
    printf("%d tid %d push at tail %p -> data = %d\n",
            c++,get_tid(),(unsigned int)myque->tail, myque->tail->data);

}

int queuePop(){
    Nodeptr head;

    // First In First Out
    int tem;
    do
    {
        head = myque->head;
        if(head->next==NULL){
            fprintf(stderr, "%s\n", "when pop queue null");
           // return -1;
        }
    } while (CAS(&myque->head, head, head->next) != TRUE);

    tem = head->next->data;
    free(head);

    printf("          %d tid %d pop at head %p -> data = %d\n",
           d++,get_tid(), (unsigned int)myque->head, tem);
    return tem;
}



int main()
{
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

    /*
    int s = 123;
    void *ptr;
    ptr = &s;
    printf("%x\n", ptr);

    printf("%x\n", PRED_PTR(ptr)+1);

    printf("%x\n", (char **)ptr+1);

    printf("%d\n", PRED(ptr));*/

    // test the cas

    /*
    printf("\nbefore init myque addr at %x\n", myque);
    myque = queueNew();
    printf("myque addr is %x\n", myque);
    queuePush(myque, 1);
    printf("%d %x\n", myque->tail->data, myque->tail);
    queuePush(myque, 2);
    printf("%d %x\n", myque->tail->data, myque->tail);
    int tem = queuePop(myque);
    printf("return %d addr at %x %x\n", tem, myque->tail, myque->head);
    tem = queuePop(myque);
    printf("return %d addr at %x %x\n", tem, myque->tail, myque->head);
    tem = queuePop(myque);
    printf("return %d addr at %x %x que %x\n", tem, myque->tail, myque->head,myque);*/
    /*
    for (int i = 0; i < MAX_THREAD;i++){
        int *tem = malloc(sizeof(int));
        *tem = i;
        int err = pthread_create(&push_tid[i], NULL, queuePush,tem);
        if (err != 0)
        {
            fprintf(stderr, "create file failed  ");
            printf("%s\n", strerror(err));
        }
    }

    
    for (int i = 0; i < MAX_THREAD;i++){

        int err = pthread_create(&pop_tid[i], NULL, queuePop, NULL);
        if(err!=0){
            fprintf(stderr, "create file failed  ");
            printf("%s\n", strerror(err));
        }
    }
    
    for (int i = 0; i < MAX_THREAD;i++){

        pthread_join(push_tid[i],NULL);

        pthread_join(pop_tid[i], NULL);
    }
    
    */
        mem_init();
       for (int i = 0; i < MAX_THREAD;i++){
        int err = pthread_create(&push_tid[i],NULL,test_malloc,NULL);
        if (err != 0)
        {
            fprintf(stderr, "create file failed  ");
            printf("%s\n", strerror(err));
        }
    }

    for (int i = 0; i < MAX_THREAD;i++){
        pthread_join(push_tid[i],NULL);
    }

}