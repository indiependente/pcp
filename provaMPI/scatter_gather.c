#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <time.h>

#define NUMBER 100
#define MASTER 0
#define SIZE 1000

int find_max(int* array, int size);
void rand_fill_array(int* array, int size);

int main(int argc, char** argv)
{
	int my_rank, p;
	int *buffer;
	int *array, local_max, max;
	int *locals;
	int chunk;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	chunk = SIZE / p;

	if (my_rank == MASTER)
	{
		array = malloc(SIZE * sizeof(int));
		rand_fill_array(array, SIZE);
		locals = malloc(p * sizeof(int));
	}

	buffer = malloc(chunk * sizeof(int));

	MPI_Scatter(array, chunk, MPI_INT, buffer, chunk, MPI_INT, MASTER, MPI_COMM_WORLD);
	local_max = find_max(buffer, chunk);
	MPI_Gather(&local_max, 1, MPI_INT, locals, 1, MPI_INT, MASTER, MPI_COMM_WORLD);

	if (my_rank == MASTER)
	{
		max = find_max(locals, p);
		printf("Il massimo è: %d\n", max);
		free(locals);
		free(array);
	}

	free(buffer);
	MPI_Finalize();
	return 0;

}

int find_max(int* array, int size)
{
	int i, max;
	max = array[0];
	for (i = 0; i < size; i++)
	{
		if (array[i] > max)
			max = array[i];
	}
	return max;
}
void rand_fill_array(int* array, int size)
{
	int i;
	srand(time(NULL));
	for (i = 0; i < size; i++)
	{
		array[i] = rand() / (RAND_MAX / NUMBER + 1);
	}
}


