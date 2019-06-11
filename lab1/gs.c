#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

/* By Nikhil Gosike, NetID: ng1449 */

/***** Globals ******/
float **a; /* The coefficients */
float *x;  /* The unknowns */
float *b;  /* The constants */
float err; /* The absolute relative error */
int num = 0;  /* number of unknowns */

/****** Function declarations */
void check_matrix(); /* Check whether the matrix will converge */
void get_input();  /* Read input from file */
int error(); /* This a helper method to calculate the absolute relative errors */
int calc_unknowns(); /* This is the actual method to calculate the unknowns */
void freeall();
/********************************/



/* Function definitions: functions are ordered alphabetically ****/
/*****************************************************************/

/* This calculates the unknowns
* After this function returns:
* x[] will be filled with the estimations for the unknowns
* it will return the number of iterations to calculate the unknowns
* back to the main method
*/

int calc_unknowns(int comm_sz, int my_rank){

  // initialize the nit (to keep track of the number of iterations) and local num
  int nit = 0;
  // this helps with the range of unknowns each process will deal with
  int local_num = ceil((double) num / comm_sz);
  int i, j;

  // this gives the lower and upper bounds on which x's in x[] will be worked on
  int low = my_rank * local_num;
  int high = low + local_num;

  // This is a new array to keep track of all the new x's calculate within one iteration
  float* x_new = (float *) malloc(num * sizeof(float));

  // this is for each process to place the unknowns they calculated into
  float* local_x_new = (float *) malloc(local_num * sizeof(float));

  // this represents a boolean as an int to exit out of the while loop
  int finished = 0;

  while(!finished){

      // initialize the outer index
      int outer_index = 0;

      // initialize the new x to the corresponding b value
      for(i = low; i < high; i++){

        local_x_new[outer_index] = b[i];

        // substract the coeffecients times the x value
        for(j = 0; j < num; j++){

          if (j != i){

            local_x_new[outer_index] -= a[i][j] * x[j];
          }
        }

        // then divide by the unknown x's coefficient
        local_x_new[outer_index] /= a[i][i];

        // go to the next unknown
        outer_index++;
      }

      // use a barrier to wait for all processes to finish their calculations
      MPI_Barrier(MPI_COMM_WORLD);

      // then gather all local_new_x[] calculated to go into x_new 
      MPI_Allgather(local_x_new, local_num, MPI_FLOAT, x_new, local_num, MPI_FLOAT, MPI_COMM_WORLD);
      
      // use another barrier
      MPI_Barrier(MPI_COMM_WORLD);

      // call the error function to check the rel. abs. error and see if we are done
      finished = error(x_new, num);

      //after store the new x's calculated in the original x[] array
      for(i = 0; i < num; i++){

        x[i] = x_new[i];
      }


      // increment the number of iterations
      nit++;

  }

  // once done free data structures used and return nit
  free(x_new);
  free(local_x_new);

  return nit;

}

/* 
   Conditions for convergence (diagonal dominance):
   1. diagonal element >= sum of all other elements of the row
   2. At least one diagonal element > sum of all other elements of the row
 */
void check_matrix()
{
  int bigger = 0; /* Set to 1 if at least one diag element > sum  */
  int i, j;
  float sum = 0;
  float aii = 0;
  
  for(i = 0; i < num; i++)
  {
    sum = 0;
    aii = fabs(a[i][i]);
    
    for(j = 0; j < num; j++)
       if( j != i)
	 sum += fabs(a[i][j]);
       
    if( aii < sum)
    {
      printf("The matrix will not converge.\n");
      exit(1);
    }
    
    if(aii > sum)
      bigger++;
    
  }
  
  if( !bigger )
  {
     printf("The matrix will not converge\n");
     exit(1);
  }
}

/*
* This is a helper function to check the abs. rel. errors
* After it is completed:
* it will return 0 (false) or 1 (true) to tell the program it is done
*/

int error(float *x_new, int num){

  // initialize abs_err and finished
  int i;
  float abs_err;
  int finished = 1;

  // go through x_new array and compare each value to the orginal x[] (or old x's)
  for (i = 0; i < num; i++){

    abs_err = fabs((x_new[i] - x[i])/x_new[i]);

    if(abs_err > err) finished = 0;

  }

  // return the boolean variable

  return finished;
}

/*
* This is another helper method to free all the global arrays
*/

void freeall(){

  // first free all the inner arrays of a[][]
  int i;
  for(i = 0; i < num; i++){
    free(a[i]);
  }

  //then free the outer array a[] and x[] and b[]
  free(a);
  free(x);
  free(b);

}

/******************************************************/
/* Read input from file */
/* After this function returns:
 * a[][] will be filled with coefficients and you can access them using a[i][j] for element (i,j)
 * x[] will contain the initial values of x
 * b[] will contain the constants (i.e. the right-hand-side of the equations
 * num will have number of variables
 * err will have the absolute error that you need to reach
 */
void get_input(char filename[])
{
  FILE * fp;
  int i,j;  
 
  fp = fopen(filename, "r");
  if(!fp)
  {
    printf("Cannot open file %s\n", filename);
    exit(1);
  }

 fscanf(fp,"%d ",&num);
 fscanf(fp,"%f ",&err);

 /* Now, time to allocate the matrices and vectors */
 a = (float**)malloc(num * sizeof(float*));
 if( !a)
  {
	printf("Cannot allocate a!\n");
	exit(1);
  }

 for(i = 0; i < num; i++) 
  {
    a[i] = (float *)malloc(num * sizeof(float)); 
    if( !a[i])
  	{
		printf("Cannot allocate a[%d]!\n",i);
		exit(1);
  	}
  }
 
 x = (float *) malloc(num * sizeof(float));
 if( !x)
  {
	printf("Cannot allocate x!\n");
	exit(1);
  }


 b = (float *) malloc(num * sizeof(float));
 if( !b)
  {
	printf("Cannot allocate b!\n");
	exit(1);
  }

 /* Now .. Filling the blanks */ 

 /* The initial values of Xs */
 for(i = 0; i < num; i++)
	fscanf(fp,"%f ", &x[i]);
 
 for(i = 0; i < num; i++)
 {
   for(j = 0; j < num; j++)
     fscanf(fp,"%f ",&a[i][j]);
   
   /* reading the b element */
   fscanf(fp,"%f ",&b[i]);
 }
 
 fclose(fp); 

}


/************************************************************/


int main(int argc, char *argv[])
{

 int i;
 int nit = 0; /* number of iterations */
 FILE * fp;
 char output[100] ="";
  
 if( argc != 2)
 {
   printf("Usage: ./gsref filename\n");
   exit(1);
 }
  
 /* Read the input file and fill the global data structure above */ 
 get_input(argv[1]);
 
 /* Check for convergence condition */
 /* This function will exit the program if the coffeicient will never converge to 
  * the needed absolute error. 
  * This is not expected to happen for this programming assignment.
  */
 check_matrix();

 //initialize the MPI processes 
 int comm_sz;
 int my_rank;
 MPI_Init(&argc, &argv);
 MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
 MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

 // call on calc_unknowns to run calculations and get the number of iterations
 nit = calc_unknowns(comm_sz, my_rank);
 
 /* Writing results to file, if the rank is zero */
 if(my_rank == 0){
   sprintf(output,"%d.sol",num);
   fp = fopen(output,"w");
   if(!fp)
   {
     printf("Cannot create the file %s\n", output);
     exit(1);
   }
      
   for( i = 0; i < num; i++)
     fprintf(fp,"%f\n",x[i]);
   
   printf("total number of iterations: %d\n", nit);
   
   fclose(fp);
}
 //call freeall to cleanup the allocations
 freeall();  

 //close MPI and exit
 MPI_Finalize();
 
 exit(0);

}
