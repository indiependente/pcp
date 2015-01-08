/*
 * Write a program that performs the transpose of a square matrix A of arbitrary dimension (multiple of the number of tasks).
 *
 * The matrix is split among the tasks and initialized so that each element is unique (use row-column number and task rank).
 *
 * Thus you need to evaluate B of the form: B = A^T
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <mpi.h>
#include <math.h>

#define N 16

#define NUMBER 100
#define MASTER 0

void rand_fill_matrix(int m[N][N]);
void ordered_fill_matrix(int m[N][N]);
void print_matrix(int m[N][N], int rank);
void print_array_as_matrix(int *array, int count, int blocklength, int rank);
void transpose_matrix(int* array, int rows, int cols);



int main(int argc, char** argv)
{
	int my_rank, p;
	int matrix[N][N], *buffer;

	int count, blocklength, stride;
	int i,j;

	MPI_Datatype TMP_COLUMN, COLUMN, ROW;
	MPI_Aint lb, extent;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	if (N % p != 0)
		{MPI_Finalize(); return 1;}


	count = N / p;
	blocklength = N;

	MPI_Type_contiguous(blocklength, MPI_INT, &ROW);
	MPI_Type_commit(&ROW);
	MPI_Type_vector(blocklength, 1, N, MPI_INT, &TMP_COLUMN);
	MPI_Type_create_resized(TMP_COLUMN, 0, sizeof(int), &COLUMN);
	MPI_Type_commit(&COLUMN);

	if (my_rank == MASTER)
	{
		MPI_Type_get_extent(ROW, &lb, &extent);
		printf("ROW lb = %ld extent = %ld\n", lb, extent);
		MPI_Type_get_extent(COLUMN, &lb, &extent);
		printf("COLUMN lb = %ld extent = %ld\n", lb, extent);
		MPI_Type_get_extent(TMP_COLUMN, &lb, &extent);
		printf("TMP_COLUMN lb = %ld extent = %ld\n", lb, extent);
		ordered_fill_matrix(matrix);
		print_matrix(matrix, my_rank);
	}


	buffer = malloc(count * blocklength * sizeof(int));

	MPI_Scatter(matrix, count, ROW, buffer, count * blocklength, MPI_INT, MASTER, MPI_COMM_WORLD);

	MPI_Gather(buffer, count * blocklength, MPI_INT, matrix, count, COLUMN, MASTER, MPI_COMM_WORLD);

	if (my_rank == MASTER)
		print_matrix(matrix, my_rank);


	free(buffer);
	MPI_Type_free(&ROW);
	MPI_Type_free(&COLUMN);
	MPI_Finalize();
}

void transpose_matrix(int* array, int rows, int cols)
{
	int i, j, tmp;
	for (i = 0; i < rows; i++)
		for (j = i; j < cols; j++)
		{
			tmp = array[i*cols + j];
			array[i*cols + j] = array[j*cols + i];
			array[j*cols + i] = tmp;
		}
}
void rand_fill_matrix(int m[N][N])
{
	int i,j;
	srand(time(NULL));
	for(i=0; i<N; i++)
		for(j=0; j<N; j++)
			m[i][j] = rand() / (RAND_MAX / NUMBER + 1);
}
void ordered_fill_matrix(int m[N][N])
{
	int i,j;
	srand(time(NULL));
	for(i=0; i<N; i++)
		for(j=0; j<N; j++)
			m[i][j] = i*N+j;
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
void print_array_as_matrix(int *array, int count, int blocklength, int rank)
{	int i,j;
	printf("RANK %d\n", rank);
	for (i = 0; i < count; i++)
	{
		for (j = 0; j < blocklength; j++)
				printf("%d\t", array[i * blocklength + j]);
		printf("\n");
	}
}
