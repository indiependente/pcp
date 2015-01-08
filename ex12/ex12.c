/*
 * Each task initializes a square nxn matrix (n is the total number of the tasks) with 0s,
 * except for the diagonal elements of the matrix that are initialized with the task's rank number.
 *
 * Each task sends to rank 0 an array containing all the elements of its diagonal.
 * Task 0 overwrites the array sent by process i on the i-th row (column if Fortran) of its local matrix.
 * At the end, task 0 prints its final matrix, on which each element should be the number of its row (or column).
 */

#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

#define N 4

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
	int matrix[N][N];
	int i,j;

	int count = N,
		block_length = 1,
		stride = N + 1;

	MPI_Status status;
	MPI_Datatype DIAGONAL;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	MPI_Type_vector(count, block_length, stride, MPI_INT, &DIAGONAL);
	MPI_Type_commit(&DIAGONAL);

	fill_matrix(matrix, 0);

	for(i=0; i<N; i++)
		matrix[i][i] = my_rank;

	MPI_Gather(&matrix[0][0], 1, DIAGONAL,
				&matrix[0][0], N, MPI_INT, 0, MPI_COMM_WORLD);

	if(my_rank == 0)
		print_matrix(matrix, my_rank);

	MPI_Type_free(&DIAGONAL);
	MPI_Finalize();
	return 0;
}