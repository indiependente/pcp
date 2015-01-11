/*
 * - Master creates a matrix NxN
 * - Master sends submatrices to workers
 * - Each worker computes the sum of its submatrix
 * - Then it sends the sum to the MASTER
 * - The master finds out the max sum
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <mpi.h>

#define MASTER 0
#define N 8
#define NUMBER 10

int** allocate2Dint(int n, int m)
{
	int i;
	int *data = calloc(n * m, sizeof(int));
	int **matrix = calloc(n, sizeof(int*));
	for (i = 0; i < n; ++i)
	{
		matrix[i] = &data[i * m];
	}
	return matrix;
}
void fill_matrix(int **matrix, int rows, int cols, int value)
{
	int i, j;
	for (i = 0; i < rows; ++i)
	{
		for (j = 0; j < cols; ++j)
		{
			matrix[i][j] = value;
		}
	}
}
void rand_fill_matrix(int **matrix, int rows, int cols)
{
	int i, j;
	srand(time(NULL));
	for (i = 0; i < rows; ++i)
	{
		for (j = 0; j < cols; ++j)
		{
			matrix[i][j] = rand() / (RAND_MAX / NUMBER + 1);
		}
	}
}
int matrix_sum(int **matrix, int rows, int cols)
{
	int sum = 0, i, j;
	for (i = 0; i < rows; ++i)
	{
		for (j = 0; j < cols; ++j)
		{
			sum += matrix[i][j];
		}
	}
	return sum;
}
void print_matrix(int **matrix, int rows, int cols, int rank)
{
	int i, j;
	printf("RANK %d\n", rank);
	for (i = 0; i < rows; ++i)
	{
		for (j = 0; j < cols; ++j)
		{
			printf("%d\t", matrix[i][j]);
		}
		printf("\n");
	}
}


int main(int argc, char** argv)
{
	int my_rank, p, tag = 0;
	int subrows, subcols;
	int *buffer, **submatrix, **matrix;
	int sqrtp;
	int i, j, k, count, my_sum, max_sum;
	MPI_Status status;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	sqrtp = (int)sqrt(p);

	if (N % (sqrtp) != 0)
	{
		MPI_Finalize();
		return 1;
	}

	subrows = N / sqrtp;
	subcols = N / sqrtp;
	buffer = calloc(subrows*subcols, sizeof(int));

	if (my_rank == MASTER)
	{
		matrix = allocate2Dint(N, N);
		fill_matrix(matrix, N, N, 1);
		// rand_fill_matrix(matrix, N, N);
		print_matrix(matrix, N, N, my_rank);


		for (i = 0; i < sqrtp; ++i)
		{
			for ( j = 0; j < sqrtp; ++j)
			{
				if (i!=0 || j!=0)
				{
					count = 0;
					for (k = 0; k < subrows; ++k)
					{
						MPI_Pack(&matrix[i * subrows + k][j * subcols], subcols, MPI_INT,
								buffer, subrows * subcols * sizeof(int), &count, MPI_COMM_WORLD);
					}
					MPI_Send(buffer, count, MPI_PACKED, i*sqrtp + j, tag, MPI_COMM_WORLD);
				}
			}
		}
		print_matrix(matrix, subrows, subcols, my_rank);
		my_sum = matrix_sum(matrix, subrows, subcols);
	}
	else
	{
		MPI_Recv(buffer, subrows * subcols * sizeof(int), MPI_PACKED, MASTER, tag, MPI_COMM_WORLD, &status);
		count = 0;
		for (i = 0; i < subrows; ++i)
		{
			MPI_Unpack(buffer, subrows * subcols * sizeof(int),
				&count, &submatrix[i][0], subcols, MPI_INT, MPI_COMM_WORLD);
		}
		print_matrix(submatrix, subrows, subcols, my_rank);
		my_sum = matrix_sum(submatrix, subrows, subcols);
	}


	printf("%d: local sum = %d\n", my_rank, my_sum);
	MPI_Reduce(&my_sum, &max_sum, 1, MPI_INT, MPI_MAX, MASTER, MPI_COMM_WORLD);

	if (my_rank == MASTER)
	{
		printf("Global max sum = %d\n", max_sum);
	}

	MPI_Finalize();
	return 0;
}