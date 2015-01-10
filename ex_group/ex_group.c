/*
 - Creare due gruppi
 - Ogni processo del gruppo crea un random
 - Ogni gruppo calcola il massimo locale e lo invia al master locale
 - Il master globale calcola il massimo globale
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <mpi.h>

#define MASTER 0
#define NUMBER 100

int main(int argc, char** argv)
{
	int my_rank, g_rank, g1_master, g2_master, p, tag = 0, a_rand_num, localmax, max;
	int ranks1[4] = {0, 1, 2, 3}, ranks2[4] = {4, 5, 6, 7};
	MPI_Group orig_group, new_group, master_group;
	MPI_Comm g_comm, mg_comm;
	MPI_Status status;

	int masters[2];

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	MPI_Comm_group(MPI_COMM_WORLD, &orig_group);

	if (my_rank < 4)
	{
		MPI_Group_incl(orig_group, 4, ranks1, &new_group);
	}
	else
	{
		MPI_Group_incl(orig_group, 4, ranks2, &new_group);
	}

	MPI_Comm_create(MPI_COMM_WORLD, new_group, &g_comm);
	MPI_Comm_rank(g_comm, &g_rank);

	srand(time(NULL) + (my_rank * NUMBER * 10));
	a_rand_num = rand() / (RAND_MAX / NUMBER + 1);
	MPI_Reduce(&a_rand_num, &localmax, 1, MPI_INT, MPI_MAX, MASTER, g_comm);

	if (g_rank == MASTER)
		printf("%d: local max = %d\n", my_rank, localmax);

	if (g_rank == MASTER)
	{
		if (my_rank < 4)
		{
			g1_master = my_rank;
			MPI_Send(&g1_master, 1, MPI_INT, MASTER, tag, MPI_COMM_WORLD);
		}
		else
		{
			g2_master = my_rank;
			MPI_Send(&g2_master, 1, MPI_INT, MASTER, tag, MPI_COMM_WORLD);
		}
	}

	if (my_rank == MASTER)
	{
		MPI_Recv(&g1_master, 1, MPI_INT, ); // from who? :\
	}


	if (g_rank == MASTER)
	{
		masters[0] = g1_master;
		masters[1] = g2_master;
		printf("Master[0] = %d\tMaster[1] = %d\n", masters[0], masters[1]);

		MPI_Comm_group(MPI_COMM_WORLD, &master_group);
		MPI_Group_incl(orig_group, 2, masters, &master_group);
		MPI_Comm_create(MPI_COMM_WORLD, master_group, &mg_comm);

		MPI_Reduce(&localmax, &max, 1, MPI_INT, MPI_MAX, MASTER, mg_comm);
		if (my_rank == MASTER)
			printf("Global max = %d\n", max);
	}

	MPI_Finalize();
	return 0;
}