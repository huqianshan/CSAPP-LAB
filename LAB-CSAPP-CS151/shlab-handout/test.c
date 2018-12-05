#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<signal.h>
const size_t HUGE=1024*1024*1024;


void sigint_hadler(int sig){
    printf("catch you\n");
    exit(0);
}
int main(void){
   /*
   pid_t pid;
    pid=fork();
    printf("%p - %d \n",&pid,pid);


    char* args[3]={"/bin/echo","Hi Im hujinlei",NULL};
    execv(args[0],args);
    printf("No,you are not hujinlei \n");
*/
 /*   char* buf=malloc(HUGE*HUGE);

    if(buf==NULL){
        fprintf(stderr,"Failure at %u \n",__LINE__);
        exit(1);
    }

    printf("buf at %p \n",buf);
    free(buf);*/

    /*pid_t pid=getpid();
    printf("current pid is %d\n",pid);*/

   /* pid_t pid=fork();
    if(pid==0){
        exit(getpid());
    }else{
        int status=0;
        waitpid(pid,&status,0);
        printf("0x%x exited with 0x%x \n",pid,WEXITSTATUS(status));
        printf("status %d\n",status);
    }*/
    /*if(signal(SIGINT,sigint_hadler)==SIG_ERR){
        printf("signal error\n");
    }
    pause();*/
    for(int i=0;i<3;i++){
        fork();
        printf("%d\n",i);
    }

    return 0;
}