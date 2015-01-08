/*Write a program working only with 2 MPI processes.

For each process, define a square matrix A (nXn).
Rank 0 fills the matrix with 0, while rank 1 fills it with 1.
Define a datatype that handles a column (if C) or a row (If Fortran) of A.

Extract size and extent of this type: is the result what you expect?

Now begin the communication:
	rank 0 sends the first column/row of A to rank 1, overwriting its own first column/row.
	Check the results by printing the matrix on the screen.
	Modify the code by sending the first nb columns/rows of A:
		do you have to change the type? Can you send two items of the new type?
*/

#include <stdlib.h>
#include <stdio.h>
#include "mpi.h"

#define N 4
#define PROCS 2

void fill_matrix(int m[N][N], int value)
{
	int i,j;
	for(i=0; i<N; i++)
		for(j=0; j<N; j++)
			m[i][j] = value;
}

void print_matrix(int m[N][N], int rank)
{
	int i,j;
	printf("RANK %d\n", rank);
	for(i=0; i<N; i++)
	{
		for(j=0; j<N; j++)
			printf("%d\t", m[i][j]);
		printf("\n");
	}
}


int main(int argc, char** argv)
{
	int my_rank, p, tag = 0;
	int size, nb;
	int A[N][N];
	MPI_Datatype COLUMN, RESIZED_COLUMN;
	MPI_Aint lb, extent;
	MPI_Status status;


	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	MPI_Type_vector(N, 1, N, MPI_INT, &COLUMN);

	MPI_Type_size(COLUMN, &size);
	MPI_Type_get_extent(COLUMN, &lb, &extent);

	MPI_Type_create_resized(COLUMN, lb, sizeof(int), &RESIZED_COLUMN);
	MPI_Type_commit(&RESIZED_COLUMN);
	MPI_Type_size(RESIZED_COLUMN, &size);
	MPI_Type_get_extent(RESIZED_COLUMN, &lb, &extent);

	if(!my_rank)
		printf("size = %d\textent = %ld\tlb = %ld\n", size, extent, lb);

	fill_matrix(A, my_rank);
	print_matrix(A, my_rank);

	nb = N;

	if (!my_rank)
	{
		MPI_Send(&A[0][0], nb, RESIZED_COLUMN, !my_rank, tag, MPI_COMM_WORLD);
	}
	else
	{
		MPI_Recv(&A[0][0], nb, RESIZED_COLUMN, !my_rank, tag, MPI_COMM_WORLD, &status);
		print_matrix(A, my_rank);
	}

	MPI_Type_free(&RESIZED_COLUMN);
	MPI_Finalize();
	return 0;
}