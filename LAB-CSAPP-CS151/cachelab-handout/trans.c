/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.  
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    
if(M==32){
   for(int i=0;i<4;i++){
       for(int j=0;j<4;j++){
           for(int z=0;z<8;z++){
               int t1=A[i*8+z][j*8];
               int t2=A[i*8+z][j*8+1];
               int t3=A[i*8+z][j*8+2];
               int t4=A[i*8+z][j*8+3];

               int t5=A[i*8+z][j*8+4];
               int t6=A[i*8+z][j*8+5];
               int t7=A[i*8+z][j*8+6];
               int t8=A[i*8+z][j*8+7];


               B[j*8][i*8+z]=t1;
               B[j*8+1][i*8+z]=t2;
               B[j*8+2][i*8+z]=t3;
               B[j*8+3][i*8+z]=t4;

               B[j*8+4][i*8+z]=t5;
               B[j*8+5][i*8+z]=t6;
               B[j*8+6][i*8+z]=t7;
               B[j*8+7][i*8+z]=t8;
               
           }
       }
   }
}else if(M==64){
     for(int i=0;i<16;i++){
         for(int j=0;j<16;j++){
             for(int z=0;z<4;z++){
                 int t1=A[i*4+z][j*4];
                 int t2=A[i*4+z][j*4+1];
                 int t3=A[i*4+z][j*4+2];
                 int t4=A[i*4+z][j*4+3];

                 B[j*4][i*4+z]=t1;
                 B[j*4+1][i*4+z]=t2;
                 B[j*4+2][i*4+z]=t3;
                 B[j*4+3][i*4+z]=t4;
             }
         }
     }
}else{
    for(int i=0;i<N;i+=16){
        for(int j=0;j<M;j+=16){
            for(int z=i;z<i+16&&z<N;z++){
                for(int k=j;k<j+16&&k<M;k++){
                    B[k][z]=A[z][k];
                }
            }
        }
    }
}


      

} 


/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

char trans_new[]="4*4 For 64*64";
void trans_no_tem(int M,int N,int A[N][M],int B[M][N]){
     for(int i=0;i<16;i++){
         for(int j=0;j<16;j++){
             for(int z=0;z<4;z++){
                 int t1=A[i*4+z][j*4];
                 int t2=A[i*4+z][j*4+1];
                 int t3=A[i*4+z][j*4+2];
                 int t4=A[i*4+z][j*4+3];

                 B[j*4][i*4+z]=t1;
                 B[j*4+1][i*4+z]=t2;
                 B[j*4+2][i*4+z]=t3;
                 B[j*4+3][i*4+z]=t4;

             }
         }
     }
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

    //registerTransFunction( trans_no_tem,trans_new); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

