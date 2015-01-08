/*
 ============================================================================
 Name        : MaxArray.c
 Author      : Nobody
 Version     :
 Copyright   : Your copyright notice
 Description : Hello MPI World in C 
 ============================================================================
 */
#include <stdio.h>
#include <string.h>
#include "mpi.h"

/* Il master inizializza random un array di n interi e ripartisce tra i p-1 slave.
 * Ogni slave calcola il massimo del proprio sub-array e lo invia al master.
 * Il master provvede a calcolare il massimo dell'array.
 * Calcolare il tempo di esecuzione con diverse Send e SSend,
 * su differenti sistemi con I/O e senza.
 *
 * */
#define MASTER 0
#define SIZE 255

int main(int argc, char* argv[]){
	int  my_rank; /* rank of process */
	int  p;       /* number of processes */
	int source;   /* rank of sender */
	int dest;     /* rank of receiver */
	int tag=0;    /* tag for messages */
	int *array;        /* storage for message */
	MPI_Status status ;   /* return status for receive */
	int slice;
	int reminder;
	int i;
	int chunk;
	int max;
	int tempMax;
	double start, end;
	/* start up MPI */
	
	MPI_Init(&argc, &argv);
	
	/* find out process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); 
	
	/* find out number of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &p); 
	
	slice = SIZE / (p - 1);
	reminder = SIZE % (p - 1);

	if (my_rank != MASTER){
		chunk = slice;
		if (my_rank <= reminder)
			chunk++;
		array = (int *) malloc(chunk * sizeof(int));
		MPI_Recv(array, chunk, MPI_INT,
				MASTER, tag, MPI_COMM_WORLD, &status);
		max = array[0];
		for(i=1; i<chunk; i++)
		{
			if (array[i] > max)
				max = array[i];
		}
//		printf("%d: local max = %d\n", my_rank, max);
		MPI_Ssend(&max, 1, MPI_INT,
				MASTER, tag, MPI_COMM_WORLD);
		free(array);
	}
	else{ // master
		start = MPI_Wtime();
//		printf("slice = %d\nreminder = %d\n", slice, reminder);
		array = (int *) malloc(SIZE * sizeof(int));
		srand(time(0));
		for(i=0; i<SIZE; i++)
			array[i] = rand() % 101;

		for(i=1; i<p; i++)
		{
			MPI_Ssend(array + (slice * (i-1)) + ((i<=reminder)?(i-1):reminder), (i <= reminder) ? slice+1 : slice, MPI_INT,
					i, tag, MPI_COMM_WORLD);
		}
		MPI_Recv(&max, 1, MPI_INT,
				(MASTER + 1)%p, tag, MPI_COMM_WORLD, &status);
		for(i=2; i<p; i++)
		{
			MPI_Recv(&tempMax, 1, MPI_INT,
							i, tag, MPI_COMM_WORLD, &status);
			if (tempMax > max)
				max = tempMax;
		}
//		printf("The max is: %d.\n", max);

		end = MPI_Wtime();
		free(array);
		printf("Time elapsed: %f\n",end - start);
	}
	/* shut down MPI */
	MPI_Finalize(); 
	
	
	return 0;
}
