#include<stdio.h>

int main(){
    int tem = 0;
    void *test = &tem;
    printf("void * %x %d\n", test,sizeof(void));
    printf("char * %x\n", (char **)test+1);

    printf("void * %x %d \n", test+1);
    printf("void * %x %d\n", ((char *)test+1),*(char*)test+1);
}