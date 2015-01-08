/*
 ============================================================================
 Name        : MaxGrid.c
 Author      : Nobody
 Version     :
 Copyright   : Your copyright notice
 Description : Hello MPI World in C 
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

#define ROWS 256
#define COLS 256
#define G_ROWS 4
#define G_COLS 4
#define MASTER 0
#define PROCS 16

int coordToRank(int, int, int[16][2]);

int main(int argc, char* argv[]){
	int  my_rank; /* rank of process */
	int  p;       /* number of processes */
	int source;   /* rank of sender */
	int dest;     /* rank of receiver */
	int tag=0;    /* tag for messages */
	int **matrix;        /* 1D matrix */
//	int **m;		/* pointer to matrix */
	int rows, cols;
	int i, j, k, l, idx;
	int count;
	int buffer[(ROWS / G_ROWS) * (COLS / G_COLS)], buffsize;
	int coords[16][2];
	int mycoords[2], prevnext[2];
	int myslice[(ROWS / G_ROWS)][(COLS / G_COLS)];
	MPI_Status status ;   /* return status for receive */
	/* topology */
	MPI_Comm old_comm, cart_comm;
	int ndims, reorder, periods[2], dim_size[2];
	int position = 0;
	int max, temp, theMax, from, newMax;
	int my_x, my_y;
	/* start up MPI */
	
	MPI_Init(&argc, &argv);
	
	/* find out process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); 
	
	/* find out number of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &p); 
	
//	if (p != 16)
//		return 1;

	rows = ROWS / G_ROWS;
	cols = COLS / G_COLS;
	old_comm = MPI_COMM_WORLD;
	ndims = 2;
	dim_size[0] = G_ROWS;
	dim_size[1] = G_COLS;
	periods[0] = 0;
	periods[1] = 0;
	reorder = 1;

	MPI_Cart_create(old_comm, ndims, dim_size, periods, reorder, &cart_comm);

	for(i=0; i<p; i++)
	{
		MPI_Cart_coords(cart_comm, i, 2, coords[i]);
	}
	mycoords[0] = coords[my_rank][0];
	mycoords[1] = coords[my_rank][1];

//	for(i=0; i<p; i++)
		//if (my_rank == MASTER)
//			printf("%d: <%d,%d>\n", i, coords[i][0], coords[i][1]);

	if (my_rank == MASTER){

		matrix = malloc(ROWS * sizeof(int*));
		for(i=0; i<ROWS; i++)
			matrix[i] = malloc(COLS * sizeof(int));

		// fill the matrix
		srand(time(NULL));
		for(i=0; i<ROWS; i++)
			for(j=0; j<COLS; j++)
//				matrix[i][j] = (rand() % 1001 );// * (((rand()%2)==0)?1:-1); // segmentation fault
				matrix[i][j] = i+j;

		matrix[0][0] = RAND_MAX;
		//buffer = malloc(rows * cols * sizeof(int));
		for(i=0; i<G_ROWS; i++)
			for(j=0; j<G_COLS; j++)
			{
				if((i==mycoords[0]) && (j==mycoords[1]))
				{
					// save my indexes
					my_x = i*rows;
					my_y = j*cols;
//					printf("%d: %d-%d\n",my_rank,my_x,my_y);
				}
				else
				{
					count = 0;
					for(k=0; k<rows; k++)
					{
//						printf("%d\n",matrix[i*rows + k][j*cols]);
						//if(i*rows + k > 255 || j*cols > 255)
						//	printf("%d %d\n", (i*rows+k),(j*cols));
						MPI_Pack(&matrix[i*rows + k][j*cols], cols, MPI_INT,
								buffer, rows * cols * sizeof(int), &count, MPI_COMM_WORLD);
					}
					dest = coordToRank(i, j, coords);

//					printf("dest=%d\n",dest);
					MPI_Send(buffer, count, MPI_PACKED,
							dest, tag, MPI_COMM_WORLD);
				}
			}
		// calcolare max
		max = matrix[my_x][my_y];
		for(i=my_x; i<my_x+rows; i++)
			for(j=my_y; j<my_y+cols; j++)
				if(matrix[i][j] > max)
					max = matrix[i][j];
		printf("%d: max = %d\n", my_rank, max);
		// send max to next


		// wait for max from (3,3)
//		from = coordToRank(G_ROWS, G_COLS, coords);
//		MPI_Recv(&localMax, 1, MPI_INT,
//				from, tag, MPI_COMM_WORLD, &status);

	}
	else
	{ // slave
		//buffer = malloc(rows * cols * sizeof(int));
		MPI_Recv(buffer, rows*cols * sizeof(int), MPI_PACKED,
				MASTER, tag, MPI_COMM_WORLD, &status);
		for(i=0; i<rows; i++)
		{
			MPI_Unpack(buffer, rows * cols * sizeof(int), &position, myslice[i], cols, MPI_INT, MPI_COMM_WORLD);

		}
		max = myslice[0][0];
		for(i=0; i<rows; i++)
			for(j=0; j<cols; j++)
				if(myslice[i][j] > max)
					max = myslice[i][j];
		printf("%d: max = %d\n", my_rank, max);
		//send max to next coord


	}

	if (mycoords[0] == 0 && mycoords[1] == 0) // (0,0)
	{
		MPI_Cart_shift(cart_comm, 1, 1, &prevnext[0], &prevnext[1]);
		MPI_Send(&max, 1, MPI_INT,
						prevnext[1], tag, MPI_COMM_WORLD);
	}
	else
		if (mycoords[0] == G_ROWS - 1 && mycoords[1] == G_COLS - 1) // (3,3)
		{
			MPI_Cart_shift(cart_comm, 1, 1, &prevnext[0], &prevnext[1]);
			MPI_Recv(&theMax, 1, MPI_INT,
					prevnext[0], tag, MPI_COMM_WORLD, &status);
			if(max < theMax) max = theMax;
			printf("That's your fucking maximum = %d.\n", max);
		}
		else
			if (mycoords[1] == G_COLS-1) // (0,3) - (1,3) - (2,3)
			{
				from = coordToRank(mycoords[0], mycoords[1] - 1, coords);
				MPI_Recv(&theMax, 1, MPI_INT,
						from, tag, MPI_COMM_WORLD, &status);
				if(max < theMax) max = theMax;
				dest = coordToRank(mycoords[0]+1, 0, coords);
				MPI_Send(&max, 1, MPI_INT,
						dest, tag, MPI_COMM_WORLD);
			}
			else
				if (mycoords[1] == 0) // (1,0) - (2,0) - (3,0)
				{
					from = coordToRank(mycoords[0]-1, G_COLS - 1, coords);
					MPI_Recv(&theMax, 1, MPI_INT,
							from, tag, MPI_COMM_WORLD, &status);
					if(max < theMax) max = theMax;
					dest = coordToRank(mycoords[0], mycoords[1]+1, coords);
					MPI_Send(&max, 1, MPI_INT,
						dest, tag, MPI_COMM_WORLD);
				}
				else // centres
				{
					from = coordToRank(mycoords[0], mycoords[1]-1, coords);
					MPI_Recv(&theMax, 1, MPI_INT,
							from, tag, MPI_COMM_WORLD, &status);
					if(max < theMax) max = theMax;
					dest = coordToRank(mycoords[0], mycoords[1]+1, coords);
					MPI_Send(&max, 1, MPI_INT,
						dest, tag, MPI_COMM_WORLD);
				}
	/* shut down MPI */
	MPI_Finalize(); 
	
	
	return 0;
}

int coordToRank(int i, int j, int coords[16][2])
{	int l;
	for(l=0; l<PROCS; l++)
		if(coords[l][0]==i && coords[l][1]==j)
			{ return l; }
	return -1;
}
