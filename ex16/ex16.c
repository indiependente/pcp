/*
 * Write a program that performs the transpose of a square matrix A of arbitrary dimension (multiple of the number of tasks).
 *
 * The A is split among the tasks and initialized so that each element is unique (use row-column number and task rank).
 *
 * Thus you need to evaluate B of the form: B = A^T
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <mpi.h>
#include <math.h>

#define N 4

#define NUMBER 10
#define MASTER 0

void rand_fill_matrix(int **m);
void ordered_fill_matrix(int **m);
void print_matrix(int **m, int rows, int cols, int rank);
void print_array_as_matrix(int *array, int count, int blocklength, int rank);
int** transpose_matrix(int** array, int rows, int cols);
int** allocate2Dint(int n, int m);
void free2Dint(int** array);
int** multiply_matrices(int** A, int** B, int n, int m);

int main(int argc, char** argv)
{
	int my_rank, p;
	int **A, **B, **C, **A_buffer, **B_buffer, **tB_buffer, **C_buffer;

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
		A = allocate2Dint(N, N);
		B = allocate2Dint(N, N);
		C = allocate2Dint(N, N);
		ordered_fill_matrix(A);
		rand_fill_matrix(B);
		tB_buffer = transpose_matrix(B, N, N);
		printf("Matrix A\n");
		print_matrix(A, N, N, my_rank);
		printf("Matrix B\n");
		print_matrix(B, N, N, my_rank);
		printf("Matrix B TRANSPOSED\n");
		print_matrix(tB_buffer, N, N, my_rank);
	}


	A_buffer = allocate2Dint(count, blocklength);
	B_buffer = allocate2Dint(count, blocklength);

	int* A_ptr = (my_rank == MASTER) ? &A[0][0] : NULL;
	int* B_ptr = (my_rank == MASTER) ? &tB_buffer[0][0] : NULL;
	int* C_ptr = (my_rank == MASTER) ? &C[0][0] : NULL;

	MPI_Scatter(A_ptr, count, ROW, &A_buffer[0][0], blocklength * count, MPI_INT, MASTER, MPI_COMM_WORLD);
	MPI_Scatter(B_ptr, count, ROW, &B_buffer[0][0], blocklength * count, MPI_INT, MASTER, MPI_COMM_WORLD);

	print_matrix(A_buffer, count, blocklength, my_rank);
	print_matrix(B_buffer, count, blocklength, my_rank);

	C_buffer = multiply_matrices(A_buffer, B_buffer, count, blocklength);
	print_matrix(C_buffer, count, blocklength, my_rank);

	MPI_Gather(&C_buffer[0][0], count * blocklength, MPI_INT, C_ptr, count, ROW, MASTER, MPI_COMM_WORLD);


	if (my_rank == MASTER)
	{
		print_matrix(C, N, N, my_rank);
		free2Dint(A);
		free2Dint(B);
		free2Dint(C);
		free2Dint(tB_buffer);
	}


	free2Dint(A_buffer);
	free2Dint(B_buffer);
	free2Dint(C_buffer);

	MPI_Type_free(&ROW);
	MPI_Type_free(&COLUMN);
	MPI_Finalize();
}


int** multiply_matrices(int** A, int** B, int n, int m)
{
	int i,j,k;
	int** C = allocate2Dint(n, m);
	int** tB = transpose_matrix(B,n,m);

	for (i = 0; i < n; i++)
	{


		for(j = 0; j < m; j++)
		{

			// int tmp = 0;
			for(k = 0; k < n; k++)
			{
				C[i][j] += A[i][k]*B[k][j];
			}
			// C[i][j] = tmp;
		}


	}
	return C;
}

int** transpose_matrix(int** array, int rows, int cols)
{
	int i, j, **data;
	data = allocate2Dint(cols, rows);

	for (i = 0; i < rows; i++)
	{
		for (j = 0; j < cols; j++)
		{
			data[j][i] = array[i][j];
		}
	}
	return data;
}
void rand_fill_matrix(int **m)
{
	int i,j;
	srand(time(NULL));
	for(i=0; i<N; i++)
		for(j=0; j<N; j++)
			m[i][j] = rand() / (RAND_MAX / NUMBER + 1);
}
void ordered_fill_matrix(int **m)
{
	int i,j;
	srand(time(NULL));
	for(i=0; i<N; i++)
		for(j=0; j<N; j++)
			// m[i][j] = i*N+j;
			m[i][j] = 1;
}

void print_matrix(int **m, int rows, int cols, int rank)
{
	int i,j;
	printf("RANK %d\n", rank);
	for(i=0; i<rows; i++)
	{
		for(j=0; j<cols; j++)
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

int** allocate2Dint(int n, int m)
{
	int i;
	int *data = calloc(n * m, sizeof(int));
	int **A = calloc(n, sizeof(int*));

	for (int i = 0; i < n; ++i)
	{
		A[i] = &data[i*m];
	}
	return A;
}
void free2Dint(int** A)
{
	free(&A[0][0]);
	free(A);
}
