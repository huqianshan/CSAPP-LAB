#include <stdio.h>
#include "csapp.h"
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this 
long line in your code */
static const char *user_agent_hdr =  \
"User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) \
 Gecko/20120305 Firefox/10.0.3 HuJinlei\r\n";

typedef struct {
    char method[MAXLINE];
    char uri[MAXLINE];
    char hostname[MAXLINE];
    char port[MAXLINE];
    char version[MAXLINE];
    char path[MAXLINE];
}request_line;

typedef struct {
    char name[MAXLINE];
    char value[MAXLINE];
}request_body;


void parser(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg);
int parser_header(int fd,request_line *line,request_body *body,int *bd_num);
request_body parser_body(char *buf);


int main(int argc,char **argv)
{   
    //printf("%s", user_agent_hdr);
    int listenfd,connfd;
    char hostname[MAXLINE],port[MAXLINE];
    
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    if(argc!=2){
        fprintf(stderr,"proxy usage: %s <port>\n",argv[0]);
        exit(1);
    }
    
    listenfd = Open_listenfd(argv[1]);
    while(1){
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd,(SA*)&clientaddr,&clientlen);
        Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, 
                    port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
	    parser(connfd);
	    Close(connfd);
    }

    return 0;
}
/*
 * parser - handle one HTTP request/response transaction
 */
/* $begin parser */
void parser(int fd){
    int is_static;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio;

    request_line req_line;
    request_body req_cont[50];

    int cont_num;
    int n=parser_header(fd,&req_line,&req_cont,&cont_num);
    if(n==0){
        printf("parser headr fail \n");
        return ;
    }
  
    if (strcasecmp(method, "GET")) {                     //line:netp:doit:beginrequesterr
        clienterror(fd, method, "501", "Not Implemented",
                    "Tiny does not implement this method");
        return;
    }                                                    //line:netp:doit:endrequesterr
    

    
}

/*
: parser_header
:
*/
int parser_header(int fd,request_line *line,request_body *body,
                  int *bd_num){
    // parser to get first line 
    // read first line from fd
    rio_t rio;
    char buf[MAXBUF];

    Rio_readinitb(&rio,fd);
    if(Rio_readlineb(&rio,buf,MAXLINE)==0){
        return;
    }
    printf("%s",buf);
    // spilt into three parts
    sscanf(buf,"%s %s %s",line->method,line->uri,line->version);
    if(strlen(line->uri)<=0){
        printf("parsrer headr to uri faile \n");
        return -1;
    }

    int n;
    if((n = parser_uri(line))<=0){
        printf("uri parser faile\n");
        return -2;
    };

    // forward http/1.1 to http/1.0
    if(strcmp(line->version,"HTTP/1.1")==0){
        strcpy(line->version,"HTTP/1.0");
        strcpy(line->version+strlen("HTTP/1.1"),"\r\n");
    }

    // parser other request key-value
    *bd_num = 0;
    Rio_readlineb(&rio,buf,MAXLINE);
    while(strcmp(buf,"\r\n")){
        body[*(bd_num)++] = parser_body(buf);
        Rio_readlineb(&rio,buf,MAXLINE);
    }

}

int parser_uri(request_line *line){
    char *url=line->uri;

    if(strstr(url,"http://")!=url){
        fprintf(stderr,"error: parser_uri invalid url headr\n");
        exit(0);
    }

    url+=strlen("http://");
    // flag
    char *ptr = strstr(url,":");
    *ptr='\0';
    strcpy(line->hostname,url);
    
    url = ptr+1;
    ptr = strstr(url,"/");
    *ptr='\0';
    strcpy(line->port,url);

    url = ptr+1;
    ptr = strstr(url,"/") ;
    *ptr = "/";
    strcpy(line->path,url);
    
    return 0;
}

request_body parser_body(char *buf){
    request_body body;
    char *ptr = strstr(buf,": ");
    if(ptr==NULL){
        fprintf(stderr, "Error: invalid header: %s\n", buf);
        exit(0);
    }
    *ptr= '\0';
    strcpy(body.name,buf);
    strcpy(body.value,buf+2);
    return body;

}