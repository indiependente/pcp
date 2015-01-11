#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

#define MASTER 0
#define SIZE 10

int main(int argc, char** argv)
{
	int my_rank, p, tag = 0;
	int array[SIZE], outbuf;
	int i;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	for (i = 0; i < SIZE; ++i)
	{
		array[i] = my_rank;
	}

	MPI_Reduce(array, &outbuf, 1, MPI_INT, MPI_MAX, MASTER, MPI_COMM_WORLD);

	if (my_rank == MASTER)
	{
		printf("%d\n", outbuf);
	}


	MPI_Finalize();
	return 0;
}