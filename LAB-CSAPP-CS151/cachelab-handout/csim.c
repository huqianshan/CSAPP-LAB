#include "cachelab.h"
#include<unistd.h> // for getopt
#include<stdlib.h> // for atoi
#include<string.h>
#include<stdio.h> //fopen 

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
            case 'h': 
            default:
                printHelpMenu();
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

int initSimCache(int s,int e,int b,Sim_Cache* cache){
    if(s<0){
        printf("Set number error \n");
        exit(-1);
    }
    
    cache->set_num=2<<s;
    cache->line_num=e;
    cache->sets=(Set*)malloc(cache->set_num*sizeof(Set));

    if(!cache->set_num){
        printf("Set Memory error \n");
        exit(-2);
    }

    int i,j;
    for(i=0;i<cache->set_num;i++){
        cache->sets[i].lines=(Line*)malloc(e*sizeof(Line));

        if(!cache->set_num){
        printf("Set Memory error \n");
        exit(-2);
        }

        for(j=0;j<e;j++){
            cache->sets[i].lines[j].valid=0;
            cache->sets[i].lines[j].lru_num=0;
        }
    }

    return 0;
}

int main(int argc,char** argv)
{
    int s,E,b,is_verbose=0;
    char trace_name[100],opt[10];

    Sim_Cache cache;

    // 1. parse get_opt(argc,argv,)
    int tem=get_opt(argc,argv,&s,&E,&b,trace_name,&is_verbose);
    //printf("\n tem:%d \n",tem);
    //printf("s:%d E:%d b:%d filename:%s verbose:%d \n",s,E,b,trace_file,is_verbose);

    // 2. initilize cache struct
    int cache_tem=initSimCache(s,E,b,cache);
    
    // 3. open the trace file
    FILE* trace_file=fopen(trace_name,"r");

    // 4. read data from trace fle
    int addr,size;
    while(fscanf(trace_file,"%s %x %d",opt,&addr,&size)!=EOF){
        // 4.1 Instruction
        if(strcmp(opt,"I")==0) continue;

        int
    }
    return 0;
}
