#include <ctime>
#include <stdio.h>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include "ptlcalls.h"
#define LAP_DEVICE_FILE "/dev/lap"

void set_matrix_zero(double *A, int m, int n)
{
   for (int i =0; i < m; ++i)
     for (int j = 0; j < n; ++j)
          A[i*n+j] = 0;
}
 
void init_matrix(double *A, int m, int n, int init)
{
     for (int i =0; i < m; ++i)
       for (int j = 0; j < n; ++j)
          //A[i*m+j] = rand();
         A[i*n+j] = j+ 0.01*i + init;
}
 
void print_matrix(double *A, int m, int n)
{
     for (int i =0; i < m; ++i) {
       for (int j = 0; j < n; ++j)
         printf("%f ", A[i*n+j]);
       printf("\n");
     }
}
     
typedef struct gemm_data{
     int m;
     int n;
     int k;
     double *A;
     double *B;
     double *C;
 
     int *data_pointer;
     void do_gemm();
} gemm_data;
 
void gemm_data::do_gemm()
{
   if (m<=0 || n <= 0) {
       printf("Invalid argument! m,n must be larger than 0.\n");
       return;
    } 

    // Allocate memory for meta data 
    double *alpha = (double *)malloc(sizeof(double));
    double *betha = (double *)malloc(sizeof(double));
    *alpha = 1.00;
    *betha = 1.00;

    // Transpose convention in cblas_dgemm API
    // Ignore this for the moment : Not doing any transpose
    int cblas_order = 10;
    int cblas_transa = 11;
    int cblas_transb = 12;
    
    // Leading dimensions
    int lda = m;
    int ldb = n;
    int ldc = m;
 
    // Count the size for meta data & matrices 
    size_t data_size =  sizeof(int)*3   +       //cblas_order, cblas_transa_and_b
                        sizeof(int)*3   +       //m, n and k
                        sizeof(double)     +       //alpha
                        sizeof(double)*m*n     +    //matrix a address 
                        sizeof(int)     +       //lda
                        sizeof(double)*m*n     +   //matrix b address
                        sizeof(int)     +       //ldb
                        sizeof(double)     +       //beta
                        sizeof(double)*m*n     +    //matrix c address
                        sizeof(int)            //ldc
                        ;

    // Assing meta data in data _pointer 
    data_pointer = (int *) malloc(data_size);
    *(data_pointer) = (int) cblas_order;
    *(data_pointer+1) = (int) cblas_transa;
    *(data_pointer+2) = (int) cblas_transb;
    *(data_pointer+3) = m;
    *(data_pointer+4) = n;
    *(data_pointer+5) = k;
    *(data_pointer+6) = lda;
    *(data_pointer+7) = ldb;
    *(data_pointer+8) = ldc;

    // assign alpha
    double * A = (double *)(data_pointer +9);
    *A = *alpha;

    //assign betha
    double * B = A+1;
    *B = *betha;

    // Declare pointers to the corresponding matrices
     
    // the next element of B is matrix a
    double *a = (B + 1)     ;       
    
    // the next element of m*n of matrix a is matrix b
    double *b = (a + m*n)   ;

    // the next element of m*n of matrix b is matrix c
    double *c = (b + m*n)   ;
    
    init_matrix(a, m, n, 100);
    init_matrix(b, m, n, 1);
    init_matrix(c, m, n, 0);

    // Open the LAP accelerator    
    int fd = open(LAP_DEVICE_FILE, O_RDWR);
    if (fd < 0) {
       printf("Error during open the device file.\n");
       return;
    }

    // Write the meta-data and matrices to LAP space
    
    int s;
    s = write(fd, data_pointer, data_size);
    if (s != data_size) {
     printf("Error during write: %lu expected, %d written.\n", data_size, s);
    }


    // Trigger the LAP accelerator 
    // Wait until the acceleration is done
    // Then read the results from LAP space to user space
    
    s = read(fd, data_pointer, data_size);
    if (s != data_size) {
        printf("Error during read: %lu expected, %d written.\n", data_size, s);
     }

}

int main(int argc, char **argv)
{
     gemm_data data;
     srand(time(NULL));

     // Set the problem size
     data.m = 512;
     data.n = data.m;
     
     // Uncomment code below to take problem size as an argument from terminal 

     /*
      * if (argc < 2) {
         printf("Usage: ./a.out problem_size \n");
         return 0;
     }

     data.m = atoi(argv[1]);
     data.n = data.m;
     *
     */

     data.k = data.m/4;
     printf("compute gemm %d\n", data.m);
     
     data.do_gemm();

     return 0;
}

