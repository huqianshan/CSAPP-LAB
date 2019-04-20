/*
 * 隐式链表　＋　首次分配器　＋　sbrk 　
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 * 
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
    "Ganchengyu",
    /* First member's full name */
    "gan",
    /* First member's email address */
    "2016060203",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/*basic constants and macros of mine*/
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1 << 12)

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* size plus alloc*/
#define PACK(size, alloc) ((size) | (alloc))

#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* bp is block pointer it pointers to the first valid byte*/
#define HDRP(bp) ((char *)(bp)-WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp)-WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE(((char *)(bp)-DSIZE)))

/* select allocate and coalesce strategy  
 * allocate:FIRST_FIT,NEXT_FIT and BEST_FIT;
 * coalesce:IMM_COAL and DEL_COAL
 */

//#define FIRST_FIT
//#define BEST_FIT
#define NEXT_FIT

//#define IMM_COAL
#define DEL_COAL

static char *heap_listp = 0;

static void *coalesce(void *bp);
static void *extend_heap(size_t words);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);

#ifdef NEXT_FIT
static char *rover;
#endif

#ifdef DEL_COAL
static void delay_coalesce()
{
    char *bp = heap_listp;
    size_t size;
    while ((size = GET_SIZE(HDRP(bp))) != 0)
    {
        if (!GET_ALLOC(HDRP(bp)))
        {
            bp = (char *)coalesce(bp);
        }
        bp = NEXT_BLKP(bp);
    }
}
#endif

/* Immediate coalesce */
static void *coalesce(void *bp)
{
    size_t bef_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t aft_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    /* case One both before and after block is malloced*/
    if (bef_alloc && aft_alloc)
    {
        return bp;
    }
    /* case Two Only before is malloced*/
    if (bef_alloc && !aft_alloc)
    {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        /*Here Attention!!! PUT has already changed the size
        * and the FTRP is calcaulate the footer blook 
        * by the size */
        PUT(FTRP(bp), PACK(size, 0));
    }
    /* case Three Only after block is malloced*/
    if (!bef_alloc && aft_alloc)
    {
        size += GET_SIZE(FTRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    /* case Four both before and after block is Not malloced*/
    if (!bef_alloc && !aft_alloc)
    {
        size += GET_SIZE(FTRP(NEXT_BLKP(bp))) + GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    return bp;
}

static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    size = (words % 2) ? (words + 1) * WSIZE : (words)*WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
    {
        return NULL;
    }

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

    return coalesce(bp);
}

static void *find_fit(size_t asize)
{
#ifdef FIRST_FIT
    /* first-fit search */
    void *bp;

    for (bp = heap_listp; GET_SIZE(HDRP(bp)) != 0; bp = NEXT_BLKP(bp))
    {
        if (!GET_ALLOC(HDRP(bp)) && asize <= GET_SIZE(HDRP(bp)))
        {
            return bp;
        }
    }
    return NULL;

#elif defined NEXT_FIT
    /* next-fit search */
    rover = heap_listp;
    char *oldrover = rover;

    /* Search from the rover to the end of list */
    for (; GET_SIZE(HDRP(rover)) > 0; rover = NEXT_BLKP(rover))
        if (!GET_ALLOC(HDRP(rover)) && (asize <= GET_SIZE(HDRP(rover))))
            return rover;

    /* search from start of list to old rover */
    for (rover = heap_listp; rover < oldrover; rover = NEXT_BLKP(rover))
        if (!GET_ALLOC(HDRP(rover)) && (asize <= GET_SIZE(HDRP(rover))))
            return rover;

    return NULL; /* no fit found */

#else
    /* best-fit search */
    char *bp = heap_listp;
    size_t size;
    char *best = NULL;
    size_t minsize = 0;

    while ((size = GET_SIZE(HDRP(bp))) != 0)
    {
        if (size >= asize && !GET_ALLOC(HDRP(bp)) && (!minsize || minsize > size))
        {
            best = bp;
            minsize = size;
        }
        bp = NEXT_BLKP(bp);
    }
    return best;

#endif
}

static void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));

    if ((csize - asize) >= (2 * DSIZE))
    {
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize - asize, 0));
        PUT(FTRP(bp), PACK(csize - asize, 0));
    }
    else
    {
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
}

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1)
    {
        return -1;
    }

    PUT(heap_listp, 0);
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));

    heap_listp += (2 * WSIZE);

    if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
    {
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
    size_t asize; /* adjusted block size*/
    size_t extendsize;
    char *bp;

    if (size == 0)
    {
        return NULL;
    }

    if (size <= DSIZE)
    {
        asize = 2 * DSIZE;
    }
    else
    {
        asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);
    }

    /* 
     * select delay coalesce strategy 
     * coalesce free space per malloc
     */
#ifdef DEL_COAL
    delay_coalesce();
#endif

    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL)
    {
        place(bp, asize);
        return bp;
    }

    /* NO fit found Get more memory and place the block*/
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
    {
        return NULL;
    }
    place(bp, asize);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));

    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));

#ifdef IMM_COAL
    coalesce(ptr);
#endif
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{

    if (ptr == NULL)
    {
        return mm_malloc(size);
    }
    if (size == 0)
    {
        mm_free(ptr);
        return NULL;
    }

    size_t asize;
    if (size <= DSIZE)
    {
        asize = 2 * DSIZE;
    }
    else
    {
        asize = DSIZE * (((size + (DSIZE) + (DSIZE - 1)) / DSIZE));
    }

    size_t old_size = GET_SIZE(HDRP(ptr));
    if (old_size == asize)
    {
        return ptr;
    }
    else if (old_size > asize)
    {
        PUT(HDRP(ptr), PACK(asize, 1));
        PUT(FTRP(ptr), PACK(asize, 1));

        PUT(HDRP(NEXT_BLKP(ptr)), PACK(old_size - asize, 0));
        PUT(FTRP(PREV_BLKP(ptr)), PACK(old_size - asize, 0));
        return ptr;
    }
    else if (old_size < asize)
    {
        /* Next Block can be used*/
        size_t next_size = GET_SIZE(HDRP(NEXT_BLKP(ptr)));
        size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));

        if (!next_alloc && asize <= (next_size + old_size))
        {
            size_t remain = (next_size + old_size) - asize;
            PUT(HDRP(ptr), PACK(asize, 1));
            PUT(FTRP(ptr), PACK(asize, 1));

            if (remain >= DSIZE)
            {
                PUT(HDRP(NEXT_BLKP(ptr)), PACK(remain, 0));
                PUT(FTRP(NEXT_BLKP(ptr)), PACK(remain, 0));
            }

            return ptr;
        }
        else
        {
            // malloc newly
            char *newptr = mm_malloc(asize);
            if (newptr == NULL)
            {
                return NULL;
            }
            memcpy(newptr, ptr, old_size - DSIZE);
            mm_free(ptr);
            return newptr;
        }
    }
}