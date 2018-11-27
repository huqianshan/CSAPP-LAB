#include "cachelab.h"
#include<unistd.h> // for getopt
#include<stdlib.h> // for atoi
#include<string.h>
#include<stdio.h>

typedef struct{
    int valid;
    int tag;
    int lru_num;
}Line;

typedef struct{
    Line*  line;
}Set;

typedef struct{
    int set_num;
    int line_num;
    Set* sets;
}Sim_Cache;

int get_opt(int argc,char **argv,int *s,int* E,int *b,char* trace_filename,int* is_verbose){
    int c;
    while((c=getopt(argc,argv,"hvs:E:b:t:"))!=-1){
        switch(c){
            case 'v': *is_verbose=1;
            break;
            case 's': *s=atoi(optarg);
            break;
            case 'E': *E=atoi(optarg);
            break;
            case 'b': *b=atoi(optarg);
            break;
            case 't': checkOptarg(optarg);
            strcpy(trace_filename,optarg);
            break;
            case 'h': printHelpMenu();
            exit(0);
            //default:
                //printHelpMenu();
                exit(0);
        }
    }
    return 1;
}

void checkOptarg(char* arg){
    if (arg[0]=='-'){
        printf("./csim Missing Required command line argument\n");
        printHelpMenu();
        exit(0);
    }
}

void printHelpMenu(){
    printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n");
    printf("Options:\n");
    printf("-h         Print this help message.\n");
    printf("-v         Optional verbose flag.\n");
    printf("-s <num>   Number of set index bits.\n");
    printf("-E <num>   Number of lines per set.\n");
    printf("-b <num>   Number of block offset bits.\n");
    printf("-t <file>  Trace file.\n\n\n");
    printf("Examples:\n");
    printf("linux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.trace\n");
    printf("linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}

int main(int argc,char** argv)
{
    int s,E,b,is_verbose=0;
    char trace_file[100],opt[10];

    Sim_Cache cache;

    // 1. parse get_opt(argc,argv,)
    int tem=get_opt(argc,argv,&s,&E,&b,trace_file,&is_verbose);
    printf("\n tem:%d",tem);
    printf("s:%d E:%d b:%d filename:%s verbose:%d",s,E,b,trace_file,is_verbose);
    return 0;
}
