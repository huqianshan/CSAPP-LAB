#include <stdio.h>

int main(){
    char buf[256]="hujinlei oj ok";
    char a[14],b[54],c[23];
    sscanf(buf,"%s %s %s",a,b,c);
    printf(a,b,c);
    printf("\n");
}