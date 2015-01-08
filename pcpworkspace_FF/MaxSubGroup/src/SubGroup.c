/*
 ============================================================================
 Name        : SubGroup.c
 Author      : Nobody
 Version     :
 Copyright   : Your copyright notice
 Description : Hello MPI World in C 
 ============================================================================
 */
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mpi.h"

#define MASTER		0
#define PROCS		16
#define SUBGROUPS	5

int main(int argc, char* argv[]){
	int  my_rank; /* rank of process */
	int new_rank;	/* rank in the new group */
	int  p;       /* number of processes */
	int source;   /* rank of sender */
	int dest;     /* rank of receiver */
	int tag=0;    /* tag for messages */
	int *ranks[SUBGROUPS]; 	/* pointers to rank vectors */
	int dims[SUBGROUPS];	/* dynamic dimensions of rank vectors */
	int i, j, group, mygroup;
	int idx[SUBGROUPS];
	int random;
	MPI_Group orig_group, new_group;	/* group handlers */
	MPI_Comm new_comm;		/* new communicator */
	MPI_Status status ;   /* return status for receive */
	
	/* start up MPI */
	
	MPI_Init(&argc, &argv);
	
	/* find out process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); 
	
	/* find out number of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &p); 
	
	/* extract group handler */
	MPI_Comm_group(MPI_COMM_WORLD, &orig_group);

	/* zero fill dimensions */
	for(i=0; i<SUBGROUPS; i++)
		dims[i] = 0;
	/* for every process find his group and increment the dimension of the rank vectors */
	group = 0;
	for(i=0; i<PROCS; i++)
	{
		group = (i / (PROCS / SUBGROUPS));
		if (group >= SUBGROUPS)
			group = SUBGROUPS - 1;
		dims[group]++;
		if (i == my_rank)
			mygroup = group;
	}

	/* allocate the memory for the "matrix" */
	for(i=0; i<SUBGROUPS; i++)
	{
		ranks[i] = malloc(dims[i] * sizeof(int));
	}
	
	/* zero fill the indexes */
	for(i=0; i<SUBGROUPS; i++)
		idx[i] = 0;

	/* place every process in the "matrix" of ranks */
	for(i=0; i<PROCS; i++)
	{
		group = (i / (PROCS / SUBGROUPS));
		if (group >= SUBGROUPS)
			group = SUBGROUPS - 1;
		ranks[group][idx[group]++] = i;
	}

	/* join your group */
	MPI_Group_incl(orig_group, dims[mygroup], ranks[mygroup], &new_group);

	/* create the communicator for the group */
	MPI_Comm_create(MPI_COMM_WORLD, new_group, &new_comm);

	/* extract the new rank in the group */
	MPI_Group_rank(new_group, &new_rank);


	if (new_rank == MASTER) /* broadcast a random number in your group */
	{
		srand(time(NULL) + my_rank);
		random = rand() % 100;
		MPI_Bcast(&random, 1, MPI_INT,
			new_rank, new_comm);
		printf("%d - %d - %d\t -----> %d\n", mygroup, new_rank, my_rank, random);
	}
	else /* receive a random number broadcasted from the groupmaster */
	{
		MPI_Bcast(&random, 1, MPI_INT,
				MASTER, new_comm);
		printf("%d - %d - %d\t <----- %d\n", mygroup, new_rank, my_rank, random);

	}

	for(i=0; i<SUBGROUPS; i++)
		free(ranks[i]);

	/* shut down MPI */
	MPI_Finalize(); 
	
	
	return 0;
}
