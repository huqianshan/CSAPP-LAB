/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 * hjl fucking Uestc's cold on 2018 12 12
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "alone in 615",
    /* First member's full name */
    "hujinlei",
    /* First member's email address */
    "1196455147@qq.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/*basic constants and macros of mine*/
#define  WSIZE 4
#define  DSIZE 8
#define CHUNKSIZE (1<<12)

#define  GET(p)        (*(unsigned int*)(p))
#define  PUT(p,val)    (*(unsigned int*)(p) = (val))

/* size plus alloc*/
#define  PACK(size,alloc) ((size)|(alloc))

#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* bp is block pointer it pointers to the first valid byte*/
#define HDRP(bp) ((char*)(bp)-WSIZE)
#define FTRP(bp) ((char*)(bp)+GET_SIZE(HDRP(bp))-DSIZE)

#define NEXT_BLKP(bp) ((char*)(bp)+GET_SIZE((HDRP(bp))))
#define PREV_BLKP(bp) ((char*)(bp)-GET_SIZE(((char*)(bp)-DSIZE)))

static char *heap_listp=0;

static void* coalesce(void* bp){
    size_t bef_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t aft_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size=GET_SIZE(HDRP(bp));

    /* case One both before and after block is malloced*/
    if(bef_alloc==1 && aft_alloc==1){
        return bp;
    }
    /* case Two Only before is malloced*/
    if(bef_alloc==1 && aft_alloc==0){
        size+=GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp),PACK(size,0));
        /*Here Attention!!! PUT has already changed the size
        * and the FTRP is calcaulate the footer blook 
        * by the size */
       PUT(FTRP(bp),PACK(size,0));
    }
    /* case Three Only after block is malloced*/
    if(bef_alloc==0 && aft_alloc==1){
        size+=GET_SIZE(FTRP(PREV_BLKP(bp)));
        PUT(FTRP(bp),PACK(size,0));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        bp=PREV_BLKP(bp);
    }
    /* case Four both before and after block is Not malloced*/
    if(bef_alloc==0 && aft_alloc ==0){
        size+=GET_SIZE(HDRP(NEXT_BLKP(bp)))+GET_SIZE(FTRP(PREV_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        PUT(FTRP(NEXT_BLKP(bp)),PACK(size,0));
        bp=PREV_BLKP(bp);
    }

    return bp;
}

static void *extend_heap(size_t words){
    char *bp;
    size_t size;

    size = (words%2)?(words+1)*WSIZE:(words)*WSIZE;
    if((long)(bp=mem_sbrk(size))==-1){
        return NULL;
    }

    PUT(HDRP(bp),PACK(size,0));
    PUT(FTRP(bp),PACK(size,0));
    PUT(HDRP(NEXT_BLKP(bp)),PACK(0,1));

    return coalesce(bp);
}


/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{   
    if((heap_listp=mem_sbrk(4*WSIZE))==(void*)-1){
        return -1;
    } 

    PUT(heap_listp,0);
    PUT(heap_listp+(1*WSIZE),PACK(DSIZE,1));
    PUT(heap_listp+(2*WSIZE),PACK(DSIZE,1));
    PUT(heap_listp+(3*WSIZE),PACK(0,1));

    heap_listp+=(2*WSIZE);

    if(extend_heap(CHUNKSIZE/WSIZE)==NULL){
        return -1;
    }
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    int newsize = ALIGN(size + SIZE_T_SIZE);
    void *p = mem_sbrk(newsize);
    if (p == (void *)-1)
	return NULL;
    else {
        *(size_t *)p = size;
        return (void *)((char *)p + SIZE_T_SIZE);
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}













