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
	int my_rank, p, tag = 0;
	MPI_Status status;
	int chunk, reminder;
	int *array,
		i,
		*locals,
		max,
		my_chunk,
		local_max;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	chunk = SIZE / (p-1);
	reminder = SIZE % (p-1);

	if (my_rank == MASTER)
	{
		array = malloc(SIZE * sizeof(int));
		rand_fill_array(array, SIZE);
		for (i = 1; i < p; i++)
		{
			MPI_Send(array + (chunk * (i-1)) + ((i<=reminder)?(i-1):reminder),
					chunk + (i<=reminder), MPI_INT, i, tag, MPI_COMM_WORLD);
		}
		locals = malloc((p-1) * sizeof(int));
		for (i = 0; i < p-1; i++)
		{
			MPI_Recv(&locals[i], 1, MPI_INT, i+1, tag, MPI_COMM_WORLD, &status);
		}
		max = find_max(locals, p-1);
		printf("Il massimo è: %d\n", max);
		free(locals);
	}
	else	// worker
	{
		my_chunk = (my_rank <= reminder) ? chunk+1 : chunk;
		array = malloc(my_chunk * sizeof(int));
		MPI_Recv(array, my_chunk, MPI_INT, MASTER, tag, MPI_COMM_WORLD, &status);
		local_max = find_max(array, my_chunk);
		MPI_Send(&local_max, 1, MPI_INT, MASTER, tag, MPI_COMM_WORLD);
	}

	free(array);
	MPI_Finalize();
	return 0;
}

int find_max(int* array, int size)
{
	int i, max;
	max = array[0];
	for (i = 1; i < size; i++)
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