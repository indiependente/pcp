/*
             ____________________
            /\  _______  _______ \
           /  \ \     /\ \     /\ \
          / /\ \ \   /  \ \   /  \ \
         / /  \ \ \ /    \ \ /    \ \
        / /    \ \ \______\ \______\ \
       / /______\ \  _______  _______ \
      /  \      /  \ \     /\ \     /\ \
     / /\ \    / /\ \ \   /  \ \   /  \ \
    / /  \ \  / /  \ \ \ /    \ \ /    \ \
   / /    \ \/ /    \ \ \______\ \______\ \
  / /______\  /______\ \___________________\
  \ \      /  \      / / ______   ______   /
   \ \    / /\ \    / / /\     / /\     / /
    \ \  / /  \ \  / / /  \   / /  \   / /
     \ \/ /    \ \/ / /    \ / /    \ / /
      \  /______\  / /______/ /______/ /
       \ \      / / _______  _______  /
        \ \    / / /      / /      / /
         \ \  / / / \    / / \    / /
          \ \/ / /   \  / /   \  / /
           \  / /_____\/ /_____\/ /
            \/___________________/

 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "mpi.h"

#define MASTER 0
#define ROWS 64
#define COLS 64
#define DEPTH 64
#define PROCS 64
#define P_ROWS 4
#define P_COLS 4
#define P_DEPTH 4
#define DIMENSIONS 3
#define N 10000

int main(int argc, char** argv){
	int *data, ***matrix;			// data 1D, matrix 3D

	int my_rank, cart_rank, p,
		source, dest,
		max, global_max, recvd_max;

	int coords[DIMENSIONS];
	int dest_coords[DIMENSIONS];
	int source_coords[DIMENSIONS];

	int tag = 0;
	MPI_Status status;

	int	C_ROWS = ROWS / P_ROWS,		// chunk rows, cols and depth
		C_COLS = COLS / P_COLS,
		C_DEPTH = DEPTH / P_DEPTH;

	int	ndims = DIMENSIONS,
	 	dims[DIMENSIONS] = {P_ROWS, P_COLS, P_DEPTH},
	 	periods[DIMENSIONS] = {0, 0, 0},
		reorder = 0;

	int i,j,k,x,y,z;

	int buffer[C_ROWS * C_COLS * C_DEPTH];
	int submatrix[C_ROWS][C_COLS][C_DEPTH];
	int count = 0;

	int ngbrs[2], PREV=0, NEXT=1;	// shift

	MPI_Comm old_comm, cart_comm;


	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	old_comm = MPI_COMM_WORLD;

	MPI_Cart_create(old_comm, ndims, dims, periods, reorder, &cart_comm);
	MPI_Comm_rank(cart_comm, &cart_rank);
	MPI_Cart_coords(cart_comm, cart_rank, 4, coords);


	if (my_rank == MASTER)
	{
		data = malloc(ROWS * COLS * DEPTH * sizeof(int));
		srand(time(NULL));
		for(i=0; i<ROWS*COLS*DEPTH; i++)
			data[i] = rand() / (RAND_MAX / N + 1);
//			data[i]	= i;

		matrix = malloc(ROWS * sizeof(int**));
		for(i=0; i<ROWS; i++)
			matrix[i] = malloc(COLS * sizeof(int*));

		for(i=0; i<ROWS; i++)
			for(j=0; j<COLS; j++)
				matrix[i][j] = &data[(i * COLS * DEPTH) + (j * DEPTH)];

		matrix[0][0][0] = 123456789;

		for(x=0; x<P_ROWS; x++)
		{
			for(y=0; y<P_COLS; y++)
			{
				for(z=0; z<P_DEPTH; z++)
				{
					if (coords[0]!=x || coords[1]!=y || coords[2]!=z) // if <x,y,z> is different from master's coords
					{
						dest_coords[0] = x;
						dest_coords[1] = y;
						dest_coords[2] = z;
						count = 0;
						for(i=0; i<C_ROWS; i++)
						{
							for(j=0; j<C_COLS; j++)
							{
								MPI_Pack(&matrix[(x*C_ROWS)+i][(y*C_COLS)+j][z*C_DEPTH], C_DEPTH, MPI_INT,
									buffer, C_ROWS * C_COLS * C_DEPTH * sizeof(int), &count, MPI_COMM_WORLD);
							}
						}
						MPI_Cart_rank(cart_comm, dest_coords, &dest);
						MPI_Send(buffer, count, MPI_PACKED, dest, tag, MPI_COMM_WORLD);
					}
				}
			}
		}

		max = matrix[0][0][0];
		for(i=0; i<C_ROWS; i++)
			for(j=0; j<C_COLS; j++)
				for(k=0; k<C_DEPTH; k++)
				if (max < matrix[i][j][k])
					max = matrix[i][j][k];

	}
	else // slave
	{
		source_coords[0] = 0;
		source_coords[1] = 0;
		source_coords[2] = 0;
		MPI_Cart_rank(cart_comm, source_coords, &source);
		MPI_Recv(buffer, C_ROWS * C_COLS * C_DEPTH * sizeof(int), MPI_PACKED,
				source, tag, MPI_COMM_WORLD, &status);

		count = 0;
		for(i=0; i<C_ROWS; i++)
			for(j=0; j<C_COLS; j++)
				MPI_Unpack(buffer, C_ROWS * C_COLS * C_DEPTH * sizeof(int), &count,
							submatrix[i][j], C_DEPTH, MPI_INT, MPI_COMM_WORLD);

		max = submatrix[0][0][0];
		for(i=0; i<C_ROWS; i++)
			for(j=0; j<C_COLS; j++)
				for(k=0; k<C_DEPTH; k++)
				if (max < submatrix[i][j][k])
					max = submatrix[i][j][k];
	}





/** FASTER SOLUTION
 *	MPI_Reduce(&max, &global_max, 1, MPI_INT, MPI_MAX, MASTER, MPI_COMM_WORLD);
 *
 *	if (my_rank == MASTER)
 *		printf("Global max = %d\n", global_max);
 */


/**
 * 0,0,0 	invia a next
 * 0,0,3 	ricevi da prev ed invia a 0,1,0
 * 0,1,0	ricevi da 0,0,63 ed invia a next
 * 0,3,3	ricevi da prev ed invia a 1,0,0
 * 1,0,0	ricevi da 0,3,3 ed invia a next
 * 3,3,3	ricevi da prev
 * i,j,k	ricevi da prev ed invia a next
 */

	if(coords[0] == 0 && coords[1] == 0 && coords[2] == 0)								// 0 0 0
	{
		MPI_Cart_shift(cart_comm, DIMENSIONS-1, 1, &ngbrs[PREV], &ngbrs[NEXT]);
		MPI_Send(&max, 1, MPI_INT, ngbrs[NEXT], tag, MPI_COMM_WORLD);
	}
	else if(coords[0] == P_ROWS-1 && coords[1] == P_COLS-1 && coords[2] == P_DEPTH-1)	// 3 3 3
		{
			MPI_Cart_shift(cart_comm, DIMENSIONS-1, 1, &ngbrs[PREV], &ngbrs[NEXT]);
			MPI_Recv(&recvd_max, 1, MPI_INT, ngbrs[PREV], tag, MPI_COMM_WORLD, &status);
			max = (max < recvd_max) ? recvd_max : max;
			printf("Global max = %d\n", max);
		}
		else if(coords[1] == P_COLS-1 && coords[2] == P_DEPTH-1)						// x 3 3
			{
				MPI_Cart_shift(cart_comm, DIMENSIONS-1, 1, &ngbrs[PREV], &ngbrs[NEXT]);
				MPI_Recv(&recvd_max, 1, MPI_INT, ngbrs[PREV], tag, MPI_COMM_WORLD, &status);
				max = (max < recvd_max) ? recvd_max : max;
				dest_coords[0] = coords[0] + 1;
				dest_coords[1] = 0;
				dest_coords[2] = 0;
				MPI_Cart_rank(cart_comm, dest_coords, &dest);
				MPI_Send(&max, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
			}
			else if(coords[2] == P_DEPTH-1)												// x y 3
				{
					MPI_Cart_shift(cart_comm, DIMENSIONS-1, 1, &ngbrs[PREV], &ngbrs[NEXT]);
					MPI_Recv(&recvd_max, 1, MPI_INT, ngbrs[PREV], tag, MPI_COMM_WORLD, &status);
					max = (max < recvd_max) ? recvd_max : max;
					dest_coords[0] = coords[0];
					dest_coords[1] = coords[1] + 1;
					dest_coords[2] = 0;
					MPI_Cart_rank(cart_comm, dest_coords, &dest);
					MPI_Send(&max, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
				}
				else if(coords[1] == 0 && coords[2] == 0)								// x 0 0
					{
						MPI_Cart_shift(cart_comm, DIMENSIONS-1, 1, &ngbrs[PREV], &ngbrs[NEXT]);
						source_coords[0] = coords[0] - 1;
						source_coords[1] = P_COLS - 1;
						source_coords[2] = P_DEPTH - 1;
						MPI_Cart_rank(cart_comm, source_coords, &source);
						MPI_Recv(&recvd_max, 1, MPI_INT, source, tag, MPI_COMM_WORLD, &status);
						max = (max < recvd_max) ? recvd_max : max;
						MPI_Send(&max, 1, MPI_INT, ngbrs[NEXT], tag, MPI_COMM_WORLD);
					}
					else if(coords[2] == 0)												// x y 0
						{
							MPI_Cart_shift(cart_comm, DIMENSIONS-1, 1, &ngbrs[PREV], &ngbrs[NEXT]);
							source_coords[0] = coords[0];
							source_coords[1] = coords[1] - 1;
							source_coords[2] = P_DEPTH - 1;
							MPI_Cart_rank(cart_comm, source_coords, &source);
							MPI_Recv(&recvd_max, 1, MPI_INT, source, tag, MPI_COMM_WORLD, &status);
							max = (max < recvd_max) ? recvd_max : max;
							MPI_Send(&max, 1, MPI_INT, ngbrs[NEXT], tag, MPI_COMM_WORLD);
						}
						else
							{															// x y z
								MPI_Cart_shift(cart_comm, DIMENSIONS-1, 1, &ngbrs[PREV], &ngbrs[NEXT]);
								MPI_Recv(&recvd_max, 1, MPI_INT, ngbrs[PREV], tag, MPI_COMM_WORLD, &status);
								max = (max < recvd_max) ? recvd_max : max;
								MPI_Send(&max, 1, MPI_INT, ngbrs[NEXT], tag, MPI_COMM_WORLD);
							}





	MPI_Finalize();

}
