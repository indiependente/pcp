/*
 ============================================================================
 Name        : MANNAGGIA.c
 Author      : Nobody
 Version     :
 Copyright   : Your copyright notice
 Description : Hello MPI World in C 
 ============================================================================
 */
#include <stdio.h>
#include <string.h>
#include "mpi.h"

int main(int argc, char* argv[]){
	int  my_rank; /* rank of process */
	int  p;       /* number of processes */
	int source;   /* rank of sender */
	int dest;     /* rank of receiver */
	int tag=0;    /* tag for messages */
//	char message[100];        /* storage for message */
	MPI_Status status ;   /* return status for receive */
	
	int array[100];
	int i;

	/* start up MPI */
	
	MPI_Init(&argc, &argv);
	
	/* find out process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); 
	
	/* find out number of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &p); 
	
	
	if (my_rank !=0){
		/* create message */
		//sprintf(message, "MANNAGGIA! Hello MPI World from process %d!", my_rank);
		dest = 0;
		for(i=0; i<100; i++){
			array[i] = my_rank;
		}
		/* use strlen+1 so that '\0' get transmitted */
		MPI_Send(array, 100, MPI_INT,
		   dest, tag, MPI_COMM_WORLD);
	}
	else{
		//printf("Hello MPI World From process 0: Num processes: %d\n",p);
		for (source = 1; source < p; source++) {
			MPI_Recv(array, 100, MPI_INT, source, tag,
			      MPI_COMM_WORLD, &status);
			i = 0;
			for(i=0; i<100; i++){
				printf("%d \n",array[i]);
			}
		}
	}
	/* shut down MPI */
	MPI_Finalize(); 
	
	
	return 0;
}
