#include"cachelab.h"
#include<unistd.h> // for getopt
#include<getopt.h>
#include<stdlib.h> // for atoi
#include<string.h>
#include<stdio.h> //fopen 


#define Magic_Max_Lru 9999
int miss_total=0;
int hit_total=0;
int eviction_total=0;

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


int get_opt(int argc,char **argv,int *s,int* E,int *b,char* trace_filename,int* is_verbose);

void checkOptarg(char* arg);

void printHelpMenu();

int initSimCache(int s,int e,int b,Sim_Cache* cache);

int getSet(int addr,int s,int b);

int getTag(int addr,int s,int b);

int findMinLru(Sim_Cache* cache,int set_bits);

void updateLruNum(Sim_Cache* cache,int set_bits,int hit_index);

int isMiss(Sim_Cache* cache,int set_bits,int tag_bits);

int updateCache(Sim_Cache* cache,int set_bits,int tag_bits);

void loadData(Sim_Cache* cache,int set_bits,int tag_bits,int is_verbose);

void storeData(Sim_Cache* cache,int set_bits,int tag_bits,int is_verbose);

void modifyData(Sim_Cache* cache,int set_bits,int tag_bits,int is_verbose);



int main(int argc,char** argv)
{
    int s,E,b,is_verbose=0;
    char trace_name[100],opt[10];

    Sim_Cache cache;

    // 1. parse get_opt(argc,argv,)
    //int tem=get_opt(argc,argv,&s,&E,&b,trace_name,&is_verbose);
    get_opt(argc,argv,&s,&E,&b,trace_name,&is_verbose);
    //printf("\n tem:%d \n",tem);
    //printf("s:%d E:%d b:%d filename:%s verbose:%d \n",s,E,b,trace_file,is_verbose);

    // 2. initilize cache struct
    //int cache_tem=initSimCache(s,E,b,&cache);
    initSimCache(s,E,b,&cache);
    // 3. open the trace file
    FILE* trace_file=fopen(trace_name,"r");

    // 4. read data from trace fle
    int addr,size;
    while(fscanf(trace_file,"%s %x %d",opt,&addr,&size)!=EOF){
        // 4.1 Instruction
        if(strcmp(opt,"I")==0) continue;

       // 4.2  get set tag'
       int set_bits=getSet(addr,s,b);
       int tag_bits=getTag(addr,s,b);
       if(is_verbose==1){
           printf("%s %x %d \n",opt,addr,tag_bits);
        }
       // 4.3 
       if(strcmp(opt,"S")==0){
           storeData(&cache,set_bits,tag_bits,is_verbose);
       }
       if(strcmp(opt,"M")==0){
           modifyData(&cache,set_bits,tag_bits,is_verbose);
       }
       if(strcmp(opt,"L")==0){
           loadData(&cache,set_bits,tag_bits,is_verbose);
       }
       if(is_verbose==1){
           printf("\n");
       }
    }
    printSummary(hit_total,miss_total,eviction_total);
    return 0;
}


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
        cache->sets[i].line=(Line*)malloc(e*sizeof(Line));

        if(!cache->set_num){
        printf("Set Memory error \n");
        exit(-2);
        }

        for(j=0;j<e;j++){
            cache->sets[i].line[j].valid=0;
            cache->sets[i].line[j].lru_num=0;
        }
    }

    return 0;
}


int getSet(int addr,int s,int b){
    addr=addr>>b;
    int mask=(1<<s)-1;
    return addr&mask;
}

int getTag(int addr,int s,int b){
    return addr>>(s+b);
}

/* find the Minium Line in cache to Replace
   the less the lru_num,the less frequent of it
*/
int findMinLru(Sim_Cache* cache,int set_bits){
    int i=0;
    int min_index=0;
    int min_lru=Magic_Max_Lru;
    int max_line_num=cache->line_num;
    
    for(;i<max_line_num;i++){
        int tem=cache->sets[set_bits].line[i].lru_num;
        if(tem<min_lru){
            min_lru=tem;
            min_index=i;
        }
    }
    return min_index;
}



/* determinate if or not miss
    if hit ,the hitting line's lru_num is set  to Magic_Max_lru
    other line's lru_num minus one;
    if miss and the set is  not full, new line's lru_num is Magic_max_lru,other minus one;
    if miss but set is full, the minum line will be replaced,
    new line will be Magic_max_lru,other minus one
*/
void updateLruNum(Sim_Cache* cache,int set_bits,int hit_index){
    cache->sets[set_bits].line[hit_index].lru_num=Magic_Max_Lru;
    int i=0;
    int max_line=cache->line_num;

    for(;i<max_line;i++){
        if(i!=hit_index){
            cache->sets[set_bits].line[i].lru_num-=1;
        }
    }
}



int isMiss(Sim_Cache* cache,int set_bits,int tag_bits){
    int i=0;
    int max_line=cache->line_num;
    int miss=1;

    for(;i<max_line;i++){
        int valid_flag=cache->sets[set_bits].line[i].valid;
        int tag_flag=cache->sets[set_bits].line[i].tag;

        if(valid_flag==1 && tag_flag==tag_bits){
            miss=0;
            updateLruNum(cache,set_bits,i);
        }
    }

    return miss;
}

int updateCache(Sim_Cache* cache,int set_bits,int tag_bits){
    int i=0;
    int max_line=cache->line_num;
    int full=1;

    for(;i<max_line;i++){
        int tem=cache->sets[set_bits].line[i].valid;
        if(tem==0){
            full=0;
            break;
        }
    }

    if(full){ // sets are full replace the viction line
        int index_Min_Lru=findMinLru(cache,set_bits);
        cache->sets[set_bits].line[index_Min_Lru].valid=1;
        cache->sets[set_bits].line[index_Min_Lru].tag=tag_bits;
        updateLruNum(cache,set_bits,index_Min_Lru);
    }else{
        cache->sets[set_bits].line[i].valid=1;
        cache->sets[set_bits].line[i].tag=tag_bits;
        updateLruNum(cache,set_bits,i);
    }
    return full;
}

// instruction
void loadData(Sim_Cache* cache,int set_bits,int tag_bits,int is_verbose){
    if(isMiss(cache,set_bits,tag_bits)==1){
        miss_total++;
        if(is_verbose==1){
            printf("miss ");
        }
        if(updateCache(cache,set_bits,tag_bits)==1){
            eviction_total++;
            if(is_verbose==1){
                printf("eviction ");
            }
        }
    }else{
        hit_total++;
        if(is_verbose==1){
            printf("hit ");
        }
    }
}

void storeData(Sim_Cache* cache,int set_bits,int tag_bits,int is_verbose){
    loadData(cache,set_bits,tag_bits,is_verbose);
}

void modifyData(Sim_Cache* cache,int set_bits,int tag_bits,int is_verbose){
    loadData(cache,set_bits,tag_bits,is_verbose);
    storeData(cache,set_bits,tag_bits,is_verbose);
}