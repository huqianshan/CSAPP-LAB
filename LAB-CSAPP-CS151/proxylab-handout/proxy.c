#include "csapp.h"
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define CACHE_NUMBER 10
#define NTHREADS 4
#define SBUFSIZE 16

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

typedef struct{
    char *name;
    char *objest;
} Cache_line;

typedef struct{
    int cnt;
    Cache_line *objs;
} Cache;

sbuf_t sbuf;
int readcnt;
static sem_t mutex;
static sem_t cache_mutex,W;
Cache cache;

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);
char* find_sig(char** buf,char** ptr,char* word,size_t n);
int parser_and_proxy(int fd);
int parser_uri(rq_line *line);
int parser_header(int fd,rq_line *line,rq_body *body);
int parser_body(char *buf,rq_body *body);

int send_to_server(rq_line rq_head,rq_body *rq_body,int num);

void *thread(void *arg);
static void init_parser_cnt(void){
    Sem_init(&mutex, 0, 1);
}
void init_cache();

int reader(int fd, char *url);
int writer(char *buf, char *url);

int main(int argc,char **argv)
{   

    int i,listenfd,connfd;
    char hostname[MAXLINE],port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;

    if(argc!=2){
        fprintf(stderr,"proxy usage: %s <port>\n",argv[0]);
        exit(1);
    }

    init_cache();
    listenfd = Open_listenfd(argv[1]);

    sbuf_init(&sbuf, SBUFSIZE);
    for (i = 0; i < NTHREADS;i++){
        Pthread_create(&tid, NULL, thread, NULL);
    }
    while (1)
    {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE,
                    port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        sbuf_insert(&sbuf, connfd);
        //parser_and_proxy(connfd);        
    }
    return 0;
}

void *thread(void *arg){
    Pthread_detach(pthread_self());
    while(1){
        int connfd = sbuf_remove(&sbuf);
        parser_and_proxy(connfd);
        Close(connfd);
    }
}

/*
 * parser_and_proxy - handle one HTTP  transaction
 * input: client_file_descriptor
 * output: 0 for corrent -1 for some situation
 * redirect request to server through proxy
 */
/* $begin parser */
int parser_and_proxy(int client_fd){
    int bd_num;
    int total_size,n;
    int server_fd;

    char buf[MAXLINE],url[MAXLINE],obj_buf[MAX_OBJECT_SIZE];

    rq_line rq_header;
    rq_body rq_main[50];

    rio_t rio;
   
    bd_num = parser_header(client_fd, &rq_header, rq_main);
    if(bd_num<=0){
        fprintf(stderr, "parser headr fail %d\n",bd_num);
        return -1;
    }
    
    // only deal with "GET"
    if (strcasecmp(rq_header.method, "GET")) {                    
        clienterror(client_fd, rq_header.method, "501", "Not Implemented",
                    "Tiny does not implement this method");
        return -2;
    }

    strcpy(url, rq_header.hostname);
    strcpy(url + strlen(url), rq_header.path);
    printf("debug url for cache is %s\n", url);
    if (reader(client_fd, url))
    {
        //fprintf(stdout, "%s from cache\n", url);
        //fflush(stdout);
        printf("url %s from cache\n", url);
        return 0;
    }


    server_fd = send_to_server(rq_header, rq_main, bd_num);
    if(server_fd<=0){
        fprintf(stderr, "some error during aimed server \n");
        return -3;
    }

    // send to clientfd
    total_size = 0;
    Rio_readinitb(&rio, server_fd);   
    while((n=Rio_readlineb(&rio,buf,MAXLINE))){
        Rio_writen(client_fd, buf,n);
        strcpy(obj_buf + total_size, buf);
        total_size += n;
    }
    printf("response total size is %d byte\n", total_size);


    if(total_size<MAX_OBJECT_SIZE){
        writer(obj_buf, url);
        printf("%s in cache \n",url);
    }

    Close(server_fd);
    return 0;
}

/*
* parser_header -- parser to get http rquest body
* input; client_file_descriptor, request_information,
* output : infomation number
*/

int parser_header(int fd,rq_line *line,rq_body *body)
{
    rio_t rio;
    char buf[MAXBUF];
    int bd_num;

    Rio_readinitb(&rio,fd);
    if(Rio_readlineb(&rio,buf,MAXLINE)==0){
        fprintf(stderr, "can't read request information  in %d \n", fd);
        return -3;
    }
    printf("raw request header : \n%s",buf);

    // spilt into three parts
    int st=sscanf(buf,"%s %s %s",line->method,line->uri,line->version);
    if(st<=0||strlen(line->method)==0||strlen(line->uri)==0||strlen(line->version)==0){
        fprintf(stderr, "can't parser request information in \n%s \n",buf);
        return -2;
    }
    
    // get host and port
    int n;
    if((n = parser_uri(line)) <0){
        fprintf(stderr, "parser host or port or path in uri[%s] false\n",line->uri);
        return -1;
    };

    // forward http/1.1 to http/1.0
    if(strcmp(line->version,"HTTP/1.1")==0){
        strcpy(line->version,"HTTP/1.0");
        strcpy(line->version+strlen("HTTP/1.1"),"\r\n");
        printf("replace HTTP/1.1 to HTTP/1.0\n");
    }

    // parser other request key-value
    bd_num = 0;
    Rio_readlineb(&rio,buf,MAXLINE);

    while (strcmp(buf, "\r\n"))
    {
        //printf("%s\n", buf);
        int pt = parser_body(buf, &body[bd_num++]);
        if(pt!=0){
            fprintf(stderr, "parser body fail %s\n", buf);
        }
        Rio_readlineb(&rio,buf,MAXLINE);
    }

    return bd_num;
}

char* find_sig(char** buf,char** ptr,char* word,size_t n){
    *buf = *ptr + n;
    *ptr = strstr(*buf,word);
    if(*ptr==NULL){
        fprintf(stderr, "can't in %s find %s\n", *buf, word);
        return "\0";
    }
    **ptr='\0';
    return *buf;
}

int parser_uri(rq_line *line){
    char *url=line->uri;
    char *ptr = url;

    // only deal with "http://hostname:port/path"
    strcpy(line->hostname, find_sig(&url, &ptr, ":", strlen("http://"))); // get host
    strcpy(line->port,find_sig(&url,&ptr,"/",1));                         // get port
    strcpy(line->path, "/");                                              // add "/"
    strcpy(line->path+1, ptr + 1);                                        // get path
    printf("header parser host:%s port:%s path : %s\n",
           line->hostname, line->port, line->path);
    return 0;
}

int  parser_body(char *buf,rq_body* body){

    char *ptr = strstr(buf,":");
    if(ptr==NULL){
        fprintf(stderr, "Error: invalid body: %s\n", buf);
        return -1;
    }
    *ptr= '\0';
    strcpy(body->name,buf);
    strcpy(body->value, ptr + 2);
    strcpy(body->value + strlen(body->value), "\r\n");
    return 0;
}
/*
   send rq_head to  hostname and port
*/
int send_to_server(rq_line rq_head,rq_body* rq_main,int num){
    int serverfd;
    char buf[MAXLINE],*buf_ptr = buf;
    rio_t rio;

    // open server's fd
    serverfd = Open_clientfd(rq_head.hostname, rq_head.port);


    static pthread_once_t once = PTHREAD_ONCE_INIT;
    Pthread_once(&once, init_parser_cnt);
    Rio_readinitb(&rio, serverfd);
    P(&mutex);

    sprintf(buf, "%s %s %s\r\n", rq_head.method, rq_head.path, rq_head.version);

    buf_ptr = buf + strlen(buf);
    for (int i = 0; i < num; i++)
    {   
        rq_body tem = rq_main[i];
        sprintf(buf_ptr, "%s: %s", tem.name, tem.value);
        buf_ptr = buf + strlen(buf);
    }
    sprintf(buf_ptr, "\r\n");
    printf("\nrequest for server is \n%s\n", buf);
    Rio_writen(serverfd, buf, MAXLINE);

    V(&mutex);

    return serverfd;
}

void init_cache(){
    Sem_init(&cache_mutex, 0, 1);
    Sem_init(&W, 0, 1);
    readcnt = 0;

    cache.objs = (Cache_line *)Malloc(sizeof(Cache_line) * CACHE_NUMBER);
    cache.cnt = 0;
    for (int i = 0; i < CACHE_NUMBER;i++){
        cache.objs[i].name = Malloc(sizeof(char) * MAXLINE);
        cache.objs[i].objest = Malloc(sizeof(char) * MAX_OBJECT_SIZE);
    }
}

int reader(int fd,char *url){
    int in_cache = 0;
    P(&cache_mutex);
    readcnt++;
    if(readcnt==1){
        P(&W);
    }
    V(&cache_mutex);

    for (int i = 0; i <CACHE_NUMBER;i++){
        if(strcmp(cache.objs[i].name,url)==0){
            Rio_writen(fd, cache.objs[i].objest, MAX_OBJECT_SIZE);
            in_cache = 1;
            break;
        }
    }

    P(&cache_mutex);
    readcnt--;
    if(readcnt==0){
        V(&W);
    }
    V(&cache_mutex);
    return in_cache;
}

int writer(char *buf,char *url){

    P(&W);
    strcpy(cache.objs[cache.cnt].name, url);
    strcpy(cache.objs[cache.cnt].objest, buf);
    cache.cnt++;
    V(&W);
    return 1;
    
}