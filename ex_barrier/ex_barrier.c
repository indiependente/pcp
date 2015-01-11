/*
 * Every process prints "Hello, I'm process rank".
 * Print it in rank order.
 */

#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

int main(int argc, char** argv)
{
	int my_rank, p;
	int i;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	for (i = 0; i < p; ++i)
	{
		if (my_rank == i)
		{
			printf("Hello, I'm process %d\n", my_rank);
		}
		MPI_Barrier(MPI_COMM_WORLD);
	}

	MPI_Finalize();
	return 0;
}