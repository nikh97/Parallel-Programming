#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cuda.h>

unsigned int getmax(unsigned int *, unsigned int);

#define TPB 1024

__global__ void get_cuda_max(unsigned int* dev_num, unsigned int size){

  unsigned int id = (blockDim.x * blockIdx.x) + threadIdx.x;
  unsigned int size_cp = size;

  unsigned int ten = size_cp/10;

  if(id < ten){

    for(unsigned int i = 1; i < 10; i++){

      if(dev_num[ten*i + id] > dev_num[id])
        dev_num[id] = dev_num[ten*i + id];
    }
  }
}

int main(int argc, char *argv[])
{
    unsigned int size = 0;  // The size of the array
    unsigned int i;  // loop index
    unsigned int * numbers; //pointer to the array
    
    if(argc !=2)
    {
       printf("usage: maxseq num\n");
       printf("num = size of the array\n");
       exit(1);
    }
   
    size = atol(argv[1]);

    numbers = (unsigned int *)malloc(size * sizeof(unsigned int));
    if( !numbers )
    {
       printf("Unable to allocate mem for an array of size %u\n", size);
       exit(1);
    }    

    srand(time(NULL)); // setting a seed for the random number generator
    // Fill-up the array with random numbers from 0 to size-1 
    for( i = 0; i < size; i++)
       numbers[i] = rand()  % size;

    // for( i = 0; i < size; i++)
    //   printf("%u\n", numbers[i]);

    unsigned int num_blocks = (size + TPB - 1)/TPB;

    unsigned int* dev_num;

    cudaMalloc((void**) &dev_num, size*sizeof(unsigned int));
    cudaMemcpy(dev_num, numbers, size*sizeof(unsigned int), cudaMemcpyHostToDevice);

    unsigned int size_cp = size;

    while(size_cp > 1){
      get_cuda_max<<<num_blocks, TPB>>>(dev_num, size_cp);
      size_cp = size_cp/10;
    }

    cudaMemcpy(numbers, dev_num, size*sizeof(unsigned int), cudaMemcpyDeviceToHost);

    unsigned int ans = numbers[0];

    cudaFree(dev_num);
    
    printf(" The maximum number in the array is: %u\n", 
           ans);

    printf("The max num sequentially is: %u\n", getmax(numbers, size));

    free(numbers);

    exit(0);
}

/*
   input: pointer to an array of long int
          number of elements in the array
   output: the maximum number of the array
*/
unsigned int getmax(unsigned int num[], unsigned int size)
{
  unsigned int i;
  unsigned int max = num[0];

  for(i = 1; i < size; i++)
  if(num[i] > max)
     max = num[i];

  return( max );

}


