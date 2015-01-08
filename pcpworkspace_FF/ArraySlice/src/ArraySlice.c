/*
 ============================================================================
 Name        : SliceArray.c
 Author      : frafar
 Version     :
 Copyright   : Your copyright notice
 Description : Hello MPI World in C 
 ============================================================================
 */
#include <stdio.h>
#include <string.h>
#include "mpi.h"
#include <stdlib.h>
#include <time.h>

int main(int argc, char* argv[]){
	int  my_rank; /* rank of process */
	int  p;       /* number of processes */
	int destination;   /* rank of receiver */
	int source;   /* rank of source */
	int dest;     /* rank of receiver */
	int tag=0;    /* tag for messages */
	MPI_Status status ;   /* return status for receive */

	int SIZE = 100000;
	int array[SIZE];
	int* array_slice;
	int slice;
	int reminder;
	int left;
	int i;
	int my_slice;
	int partial_sum = 0;
	int total = 0;

	/* start up MPI */

	MPI_Init(&argc, &argv);

	/* find out process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); 

	/* find out number of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &p); 

	slice = SIZE / (p-1);

	if (my_rank !=0){ // worker
		dest = 0;
		//		printf("%d: slice = %d\n", my_rank, slice);
		reminder = SIZE % (p-1);
		my_slice = slice + (my_rank <= reminder? 1 : 0);
		array_slice = (int *) malloc(sizeof(int) * my_slice);
		//		printf("my_rank <= reminder = %d\n", my_rank <= reminder);
		//		printf("%d: my_slice = %d\n", my_rank, my_slice);

		MPI_Recv(array, my_slice, MPI_INT,
				0, tag, MPI_COMM_WORLD, &status);
		i = 0;
		for(i = 0; i < my_slice; i++){
			partial_sum += array[i];
		}
		MPI_Send(&partial_sum, 1, MPI_INT,
				dest, tag, MPI_COMM_WORLD);
	}
	else{ // master
		srand(time(NULL));
		for(i = 0; i < SIZE; i++){
			array[i] = rand() % 10;
		}
		reminder = SIZE % (p-1);
		left = reminder;
		for (destination = 1; destination < p; destination++) {
			//			printf("slice: %d\n", slice + (left? 1 : 0));
			MPI_Send(array + slice * (destination-1) + (reminder - left), slice + (left? 1 : 0), MPI_INT,
					destination, tag, MPI_COMM_WORLD);
			left = left - (left? 1 : 0);
		}

		for (source = 1; source < p; source++) {
			MPI_Recv(&partial_sum, 1, MPI_INT, source, tag,
			      MPI_COMM_WORLD, &status);
			printf("Partial sum from worker %d => %d\n", source, partial_sum);
			total += partial_sum;
		}
		printf("Total = %d\n", total);
	}
	/* shut down MPI */
	MPI_Finalize(); 


	return 0;
}
