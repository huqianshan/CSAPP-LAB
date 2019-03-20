#include <stdio.h>
#include "csapp.h"
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this 
long line in your code */
static const char *user_agent_hdr =  "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3 HuJinlei\r\n";

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


void parser_and_send(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg);
int parser_header(int fd,rq_line *line,rq_body *body,int *bd_num);
rq_body parser_body(char *buf);
char* find_sig(char *buf,char* ptr,char* word,size_t n);
int send_to_server(rq_line rq_head,rq_body *rq_body,int num);

void debug(char* buf){
    printf("\n----------------this is for debug %s--------------\n",buf);
    printf("\n\n");
}

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
	    parser_and_send(connfd);
	    Close(connfd);
    }

    return 0;
}
/*
 * parser - handle one HTTP request/response transaction
 */
/* $begin parser */
void parser_and_send(int client_fd){
    int body_num;
    int total_size,n;
    int server_fd;

    char buf[MAXLINE];

    rq_line rq_header;
    rq_body rq_main[50];

    rio_t rio;

    int t = parser_header(client_fd, &rq_header, &rq_main, &body_num);
    if(t!=0){
        printf("parser headr fail \n");
        return ;
    }
  
    if (strcasecmp(rq_header.method, "GET")) {                     //line:netp:doit:beginrequesterr
        clienterror(client_fd, rq_header.method, "501", "Not Implemented",
                    "Tiny does not implement this method");
        return;
    }                                                    //line:netp:doit:endrequesterr

    total_size = 0;
    server_fd = send_to_server(rq_header, rq_main, body_num);
    Rio_readinitb(&rio, server_fd);

    // send to clientfd
    while((n=Rio_readlineb(&rio,buf,MAXLINE))){
        Rio_writen(client_fd, buf, MAXLINE);
    }
    Close(server_fd);
}

/*
: parser_header
:  parser to get http rquest body
*/
int parser_header(int fd,rq_line *line,rq_body *body,int *bd_num)
{
    rio_t rio;
    char buf[MAXBUF];

    Rio_readinitb(&rio,fd);
    if(Rio_readlineb(&rio,buf,MAXLINE)==0){
        return;
    }
    printf("raw request header : %s\n",buf);
    // spilt into three parts
    sscanf(buf,"%s %s %s",line->method,line->uri,line->version);
    
    if (strlen(line->uri) <= 0)
    {
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
    return 0;
}

char* find_sig(char *buf,char* ptr,char* word,size_t n){
    buf = ptr + n;
    ptr = strstr(buf,word);
    *ptr='\0';
    printf("debug url:%s \n", buf);
    return buf;
}

int parser_uri(rq_line *line){
    char *url=line->uri;
    char *ptr = url;
    //printf("debug url:%s \n", url);
    if (strstr(url, "http://") != url)
    {
        printf("mustbe here %s\n", strstr(url, "http://"));
        fprintf(stderr, "error: parser_uri invalid url headr\n");
        exit(0);
    }

    strcpy(line->hostname,find_sig(url,ptr,":",strlen("http://")));
    strcpy(line->port,find_sig(url,ptr,"/",1));
    strcpy(line->path,find_sig(url,ptr,"/",1));

    printf("header parser host:%s port:%s path : %s\n", 
            line->hostname, line->port, line->path);
    return 0;
}

rq_body parser_body(char *buf){
    rq_body body;
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
/*
   send rq_head to  hostname and port
*/
int send_to_server(rq_line rq_head,rq_body* rq_main,int num){
    int clientfd;
    char buf[MAXLINE],*buf_ptr = buf;
    rio_t rio;

    clientfd = Open_clientfd(rq_head.hostname, rq_head.port);
    Rio_readinitb(&rio, clientfd);
    sprintf(buf,"%s %s %s\r\n",rq_head.method,rq_head.path,rq_head.version);
    
    debug("send to server");
    printf("\nrq_line is %s\n", buf);

    buf_ptr = buf + strlen(buf);

    for (int i = 0; i < num; i++)
    {
        rq_body tem = rq_main[i];
        sprintf(buf_ptr, "%s: %s", tem.name, tem.value);
        buf_ptr = buf + strlen(buf);
    }
    sprintf(buf_ptr, "\r\n");
    Rio_writen(clientfd, buf, MAXLINE);

    return clientfd;
}