/*
 ============================================================================
 Name        : OrderedArray.c
 Author      : Nobody
 Version     :
 Copyright   : Your copyright notice
 Description : Hello MPI World in C 
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

#define MASTER 0
#define SIZE 15
#define N 100

int main(int argc, char* argv[]){
	int  my_rank; /* rank of process */
	int  p;       /* number of processes */
	int source;   /* rank of sender */
	int dest;     /* rank of receiver */
	int tag=0;    /* tag for messages */
	int *array;        /* storage for message */
	MPI_Status status ;   /* return status for receive */
	int i, chunk, left, reminder;
	int *send_counts, *displ, *recvbuf;
	int ordered = 1, global_ordered;
	/* start up MPI */
	
	MPI_Init(&argc, &argv);
	
	/* find out process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); 
	
	/* find out number of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &p); 
	
	chunk = SIZE / p;
	reminder = SIZE % p;
	left = reminder;

	send_counts = alloca(p * sizeof(int));
	displ = alloca(p * sizeof(int));

	if (my_rank == MASTER)
	{

		srand(time(NULL));
		array = malloc(SIZE * sizeof(int));


		for(i=0; i<SIZE; i++){
			array[i] = i;
//			array[i] = rand() / (RAND_MAX / N + 1);
//			printf("%d\n", array[i]);
		}
		array[3] = 100;
	}

	for(i=0; i<p; i++)
	{
		send_counts[i] = chunk + ((i<p-1)?1:reminder) ;
		displ[i] = (i*chunk) ;
	}
	printf("%d: count = %d displ = %d\n", my_rank, send_counts[my_rank], displ[my_rank]);
	recvbuf = malloc(send_counts[my_rank] * sizeof(int));
	MPI_Scatterv(array, send_counts, displ, MPI_INT,
			recvbuf, send_counts[my_rank], MPI_INT,
			MASTER, MPI_COMM_WORLD);

	for(i=0; i<send_counts[my_rank]; i++)
		printf("%d: %d\n", my_rank, recvbuf[i]);

	for(i=0; i<send_counts[my_rank] - 1; i++)
	{
		if (recvbuf[i] >= recvbuf[i+1])
		{
			ordered = 0;
			break;
		}
	}
	MPI_Reduce(&ordered, &global_ordered, 1, MPI_INT, MPI_LAND, MASTER, MPI_COMM_WORLD);
	if (my_rank == MASTER)
		printf("Array is %s!\n", (global_ordered)?"ordered":"unordered");
	/* shut down MPI */
	MPI_Finalize(); 
	
	
	return 0;
}
