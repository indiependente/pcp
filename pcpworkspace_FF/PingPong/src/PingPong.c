/*
 ============================================================================
 Name        : PingPong.c
 Author      : Nobody
 Version     :
 Copyright   : Your copyright notice
 Description : Hello MPI World in C 
 ============================================================================
 */
/**
 *Scrivere un programma che fa il ping pong di messaggi tra il master e ciascuno dei k slave:
 * - il master invia il primo messaggio al processo 1, e poi aspetta la sua risposta.
 * - Ogni messaggio ricevuto dal processo i viene poi inviato al processo i+1
 *   (ovviamente ricominciando dal processo 1 quando si riceve dal k-esimo processo)
 */

#include <stdio.h>
#include <string.h>
#include "mpi.h"

#define MASTER 0 /* master process' rank*/

int main(int argc, char* argv[]){
	int  my_rank; /* rank of process */
	int  p;       /* number of processes */
	int source;   /* rank of sender */
	int dest;     /* rank of receiver */
	int tag=0;    /* tag for messages */
	//char message[100];        /* storage for message */
	MPI_Status status ;   /* return status for receive */
	int ball = 0;


	/* start up MPI */
	
	MPI_Init(&argc, &argv);
	
	/* find out process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); 
	
	/* find out number of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &p); 
	
	
	if (my_rank != MASTER){ // slave
		source = (((my_rank - 1) % p) == 0 ) ? ((p - 1) % p) : ((my_rank - 1) % p);
		dest = (((my_rank + 1) % p) == 0 ) ? ((my_rank + 2) % p) : ((my_rank + 1) % p);

		if (my_rank == (MASTER + 1) % p)
		{
			MPI_Recv(&ball, 1, MPI_INT, MASTER, tag, MPI_COMM_WORLD, &status);
			printf("%d: ball received from master process %d.\n", my_rank, MASTER);
			MPI_Send(&ball, 1, MPI_INT,
				MASTER, tag, MPI_COMM_WORLD);
			printf("%d: ball thrown to master process %d.\n", my_rank, MASTER);
			printf("%d: i'm throwing the ball to process %d.\n", my_rank, dest);
			MPI_Send(&ball, 1, MPI_INT,
					dest, tag, MPI_COMM_WORLD);
		}

		while(1)
		{
			sleep(1);
			printf("%d: i'm waiting the ball from process %d.\n", my_rank, source);
			MPI_Recv(&ball, 1, MPI_INT, source,
					tag, MPI_COMM_WORLD, &status);
			printf("%d: ball received from process %d.\n", my_rank, source);
			ball++; // this could break the program
			printf("%d: ball = %d\n", my_rank, ball);
			printf("%d: i'm throwing the ball to process %d.\n", my_rank, dest);
			MPI_Send(&ball, 1, MPI_INT,
					dest, tag, MPI_COMM_WORLD);
		}
	}
	else{ // master
			printf("PING PONG\n%d: Hi all. I'm the master.\n", my_rank);
			printf("%d: i'm throwing the ball to process %d.\n", my_rank, (my_rank+1)%p);
			MPI_Send(&ball, 1, MPI_INT,
					(my_rank + 1)%p, tag, MPI_COMM_WORLD);
			printf("%d: i'm waiting the ball from process %d.\n", my_rank, (my_rank+1)%p);
			MPI_Recv(&ball, 1, MPI_INT,
					(my_rank+1)%p, tag, MPI_COMM_WORLD, &status);
			printf("%d: ball received from process %d.\n", my_rank, (my_rank+1)%p);


	}
	/* shut down MPI */
	MPI_Finalize(); 
	
	
	return 0;
}
