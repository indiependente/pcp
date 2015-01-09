/*
 * http://nf.nci.org.au/training/MPI/MPICourse/slides/mpislide.078.html
 * 1.
 * Construct a set of processes in a ring (so that 0 passes to 1 passes to ... n-2 passes n-1 passes to 0).
 * Have each processor pass its rank to it's neighbour and keep passing each message it receives
 * until it gets it' s own rank back.
 * Let each processor keep a sum of the messages it receives and print the sum out when done.
 * Use non-blocking communication to ensure safety .
 */
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>


#define MASTER 0

int main(int argc, char** argv)
{
	int my_rank, p, tag = 0, recvd_rank = -1, count = 0, dest, src;
	MPI_Request request;
	MPI_Status status;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	dest = (my_rank + 1)%p;
	src = (my_rank == MASTER)? (p-1) : ((my_rank - 1)%p);
	printf("%d: src = %d\n", my_rank, src);
	printf("%d: dest = %d\n", my_rank, dest);

	MPI_Isend(&my_rank, 1, MPI_INT,
			dest, tag, MPI_COMM_WORLD, &request);

	while(recvd_rank != my_rank)
	{

		MPI_Irecv(&recvd_rank, 1, MPI_INT,
           src, tag, MPI_COMM_WORLD, &request);
		MPI_Wait(&request, &status);
		count++;
		if (recvd_rank == my_rank)
			break;
		MPI_Isend(&recvd_rank, 1, MPI_INT,
			dest, tag, MPI_COMM_WORLD, &request);
	}

	printf("%d: count = %d\n", my_rank, count);
	MPI_Finalize();
	return 0;

}