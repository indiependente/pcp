#include "matrix_utils.h"
#include <stdio.h>
#include <stdlib.h>

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