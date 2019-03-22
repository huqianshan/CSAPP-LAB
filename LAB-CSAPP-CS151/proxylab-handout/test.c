#include <stdio.h>

int main(){
    char buf[256]="hujinlei oj ok";
    char a[14],b[54],c[23];
    //sscanf(buf,"%s %s %s",a,b,c);
    sprintf(a, "%s %d", "fuck", 42);
    int two = 2;
    fprintf(stderr, buf);
    printf("%s", a);
    printf("\n");
}