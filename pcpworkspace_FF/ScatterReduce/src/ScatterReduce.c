/*
 ============================================================================
 Name        : ScatterReduce.c
 Author      : Nobody
 Version     :
 Copyright   : Your copyright notice
 Description : Hello MPI World in C 
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <time.h>

#define MASTER 0
#define ROWS 256
#define COLS 256
#define PROCS 16
#define N 100

int main(int argc, char* argv[]){
	int  my_rank; /* rank of process */
	int  p;       /* number of processes */
	int source;   /* rank of sender */
	int dest;     /* rank of receiver */
	int tag=0;    /* tag for messages */
	int matrix[ROWS * COLS];	/* int data allocated contiguously*/
	int i, local_max, global_max;
	int send_count = (ROWS * COLS) / PROCS;
	int recv_data[send_count];
	MPI_Status status ;   /* return status for receive */
	
	/* start up MPI */
	
	MPI_Init(&argc, &argv);
	
	/* find out process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); 
	
	/* find out number of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &p); 

	if (my_rank == MASTER) /* fill the matrix and scatter it */
	{
		srand(time(NULL));
		for(i=0; i<ROWS*COLS; i++)
			matrix[i] = rand() / (RAND_MAX / N + 1);
	}

	/* scatter the data */
	MPI_Scatter(matrix, send_count, MPI_INT,
				recv_data, send_count, MPI_INT,
				MASTER, MPI_COMM_WORLD);

	/* find the local max */
	local_max = recv_data[0];
	for(i=1; i<send_count; i++)
		local_max = (local_max < recv_data[i] ? recv_data[i] : local_max);

	printf("%d: Local max = %d\n", my_rank, local_max);

	/* share the local max and reduce to the global max */
	MPI_Reduce(&local_max, &global_max, 1,
			MPI_INT, MPI_MAX, MASTER, MPI_COMM_WORLD);

	if (my_rank == MASTER)
	{
		printf("%d: Global max = %d\n", my_rank, global_max);
	}

	/* shut down MPI */
	MPI_Finalize(); 
	
	
	return 0;
}
