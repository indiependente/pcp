/*
 * Ogni worker crea una sottomatrice e la invia al master
 * Il master riempie la sua matrice globale (NxN) con le sottomatrici ricevute
 * Attenzione alle dimensioni
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <mpi.h>
#include <math.h>

#define SIZE 16
#define NUMBER 100
#define MASTER 0

int** allocate2Dint(int n, int m)
{
	int *data = calloc(n * m, sizeof(int));
	int **matrix = calloc(n, sizeof(int*));
	int i;

	for(i = 0; i < n; i++)
	{
		matrix[i] = &data[i * m];
	}
	return matrix;
}
void fill_matrix(int **matrix, int rows, int cols, int value)
{
	int i,j;
	for (i = 0; i < rows; ++i)
	{
		for (j = 0; j < cols; ++j)
		{
			matrix[i][j] = value;
		}
	}
}
void print_matrix(int **matrix, int rows, int cols, int rank)
{
	int i,j;
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

	int **submatrix, **matrix;
	int rows, cols;
	int i;
	MPI_Datatype TMP_MATRIX, MATRIX;
	MPI_Status status;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	rows = SIZE / (sqrt(p-1));
	cols = rows;

	MPI_Type_vector(rows, cols, SIZE, MPI_INT, &TMP_MATRIX);
	MPI_Type_create_resized(TMP_MATRIX, 0, cols * sizeof(int), &MATRIX);
	MPI_Type_commit(&MATRIX);


	if	((SIZE % (int)sqrt(p-1)) != 0)
	{
		MPI_Finalize();
		return 2;
	}

	if (my_rank == MASTER)
	{
		printf("rows=%d cols=%d\n", rows, cols);
		matrix = allocate2Dint(SIZE, SIZE);
		int x = 0, y = 0;
		for (i = 1; i < p; ++i)
		{
			printf("x = %d\ty = %d\n", x, y);
			MPI_Recv(&matrix[x][y], 1, MATRIX, i, tag, MPI_COMM_WORLD, &status);
			y += cols;
			if (y == SIZE)
			{
				y = 0;
				x += rows;
			}
		}
		print_matrix(matrix, SIZE, SIZE, my_rank);
	}
	else
	{
		submatrix = allocate2Dint(rows, cols);
		fill_matrix(submatrix, rows, cols, my_rank);
		// print_matrix(submatrix, rows, cols, my_rank);
		MPI_Send(&submatrix[0][0], rows * cols, MPI_INT, MASTER, tag, MPI_COMM_WORLD);
	}

	MPI_Finalize();
}