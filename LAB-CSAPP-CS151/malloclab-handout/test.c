#include<stdio.h>

int main(){
    int tem = 0;
    void *test = &tem;
    printf("void * %x\n", test);
    printf("char * %x\n", (char **)test+1);

    printf("void * %x\n", test+1);
    printf("void * %x %d\n", ((char *)test+1),*(char*)test+1);
}