#include<stdio.h>
#include<string.h>
#define MAXLINE 256

typedef struct {
    char method[MAXLINE];
    char uri[MAXLINE];
    char hostname[MAXLINE];
    char port[MAXLINE];
    char version[MAXLINE];
    char path[MAXLINE];
}rq_line;

typedef struct {
    char name[MAXLINE];
    char value[MAXLINE];
}rq_body;

char* find_sig(char** buf,char** ptr,char* word,size_t n){
    *buf = *ptr + n;
    *ptr = strstr(*buf,word);
    **ptr='\0';
    printf("debug url:%s \n", *buf);
    return *buf;
}

int parser_uri(rq_line *line){
    char *url=line->uri;
    char *ptr = url;
    //printf("debug url:%s \n", url);

    strcpy(line->hostname,find_sig(&url,&ptr,":",strlen("http://")));
    
    strcpy(line->port,find_sig(&url,&ptr,"/",1));
    //strcpy(line->path,find_sig(url,ptr,"\0",1));
    strcpy(line->path, ptr + 1);
    printf("header parser host:%s port:%s path : %s\n",
           line->hostname, line->port, line->path);
    return 0;
}

int main(){
    rq_line line;
    //http://localhost:58963/home.html"
    strcpy(line.uri,"http://www.cmu.edu:8080/hub/index.html");

    parser_uri(&line);
}