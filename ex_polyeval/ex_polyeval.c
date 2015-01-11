/*
 * Polyeval
 * - Input x
 * - Array di coefficienti random [1 - 10]
 * - Ogni processo evaluta la sua parte di polinomio
 * - Si effettua la somma delle parti di polinomio
 * - Valore finale al master
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <mpi.h>

#define MASTER 0
#define NUMBER 10

int main(int argc, char** argv)
{
	int my_rank, p, tag = 0;
	int x, i;
	int *coeffs, my_coeff;
	int value, eval;

	x = atoi(argv[1]);

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	if (my_rank == MASTER)
	{
		coeffs = calloc(p, sizeof(int));
		srand(time(NULL));
		for (i = 0; i < p; ++i)
		{
			coeffs[i] = rand() / (RAND_MAX / NUMBER + 1);
		}
	}


	MPI_Bcast(&x, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
	MPI_Scatter(coeffs, 1, MPI_INT, &my_coeff, 1, MPI_INT, MASTER, MPI_COMM_WORLD);

	value = my_coeff * pow(x, my_rank);

	MPI_Reduce(&value, &eval, 1, MPI_INT, MPI_SUM, MASTER, MPI_COMM_WORLD);
	if (my_rank == MASTER)
	{
		for (i = 0; i < p; ++i)
		{
			printf("%dx^%d ", coeffs[i], i);
			if (i<p-1)
			{
				printf("+ ");
			}
			else
			{
				printf("= ");
			}
		}
		printf("%d\n", eval);
	}

	MPI_Finalize();
	return 0;
}