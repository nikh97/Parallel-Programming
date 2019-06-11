#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>


/* By Nikhil Gosike, NetID: ng1449 */

/**** Global Variables ****/
int * track_primes;

/**** Function declarations ****/
void find_primes(int floor, int N, int t);
void gen_output(char * out_file, int N);

/*
* This is a function that finds the prime numbers
* from 1 to N with t threads
* After this method:
* it fills the boolean array track_primes with 1 and 0s
* where 1s are primes
*/
void find_primes(int floor, int N, int t){

	int i;

	// This creates the number of necessary threads
	#pragma omp parallel num_threads(t)
	{	
		// This will parallelize the out for loop
		#pragma omp parallel for

		//go from 2 to the floor
		for(i = 2; i <= floor; i++){

			//make sure the number is not marked as prime before entering
			if (track_primes[i - 1] == 1){

				int j;

				// zero out every multiple of i in the boolean array
				for(j = i*2 - 1; j < N; j = j+i){

					track_primes[j] = 0;
				}
			}
		}
	}
}

/*
* This is a helper method to generate the output
* It will generate a N.txt file with the primes
* their ranks, and their distance from the last prime
*/ 

void gen_output(char * out_file, int N){

	// create the file and open it
	FILE * fp;

	fp = fopen(out_file, "w");

	if(!fp){
		printf("Cannot create the file %s\n", out_file);
     	exit(1);
	}

	// go through the track_primes array and write to output file
	int count = 1;
	int i;
	int last_prime = 2;
	for(i = 0; i < N; i++){

		if(track_primes[i]){

			fprintf(fp, "%d, %d, %d\n", count, i + 1, i + 1 - last_prime);
			count++;
			last_prime = i + 1;
		}
	}

	//close file
	fclose(fp);
}

/******************************************/

int main(int argc, char *argv[]){

	// initiate time variables to measure runtime
	double tstart = 0.0, ttaken;

	//take in commandline arguments
	if (argc != 3){

		printf("Usage: ./genprime N t\n");
		exit(1);
	}

	int N = atoi(argv[1]);
	int t = atoi(argv[2]);

	//check arguments based on specifications
	if(N > 100000){
		printf("N cannot exceed 100,000.\n");
		exit(1);
	}

	if(t > 100){

		printf("t cannot exceed 100.\n");
		exit(1);
	}

	//get the floor based on input
	int floor = (int)((N+1)/2);

	//allocate track primes array
	track_primes = (int *) malloc(N * sizeof(int));

	if( !track_primes){
		printf("Cannot allocate track_primes!");
		exit(1);
	} 

	// number 1 is not prime automatically
	track_primes[0] = 0;

	// assume everything is prime at first
	int i;
	for(i = 1; i < N; i++) track_primes[i] = 1;

	// start timer
	tstart = omp_get_wtime();
	
	//find primes
	find_primes(floor, N, t);

	//get final time
	ttaken = omp_get_wtime() - tstart;

	//make output file name and generate it
	char out_file[100] = "";

	sprintf(out_file, "%d.txt", N);
	gen_output(out_file, N);

	//print time, free track primes and exit
	printf("Time taken for the main part: %f\n", ttaken);

	free(track_primes);

	exit(0);
}