/*
 ============================================================================
 Name        : Life.c
 Author      : Vittorio Scarano
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>

#define NI 200        /* array sizes */
#define NJ 200
#define NSTEPS 500    /* number of time steps */


int main(int argc, char **argv) {
	int i, j, n, im, ip, jm, jp, ni, nj, nsum, isum;
	int **oldMatrix, **newMatrix;
	float x;

	/* allocate arrays */
	ni = NI + 2;  /* add 2 for left and right ghost cells */
	nj = NJ + 2;

	oldMatrix = malloc(ni * sizeof(int*));
	newMatrix = malloc(ni * sizeof(int*));

	for(i=0; i<ni; i++){
		oldMatrix[i] = malloc(nj*sizeof(int));
		newMatrix[i] = malloc(nj*sizeof(int));
	}

	/*  initialize elements of oldMatrix to 0 or 1 */
	for(i=1; i<=NI; i++){
		for(j=1; j<=NJ; j++){
			x = rand()/((float)RAND_MAX + 1);
			if(x<0.5){
				oldMatrix[i][j] = 0;
			} else {
				oldMatrix[i][j] = 1;
			}
		}
	}

	/*  time steps */
	for(n=0; n<NSTEPS; n++){
		/* corner boundary conditions */
		oldMatrix[0][0] = oldMatrix[NI][NJ];
		oldMatrix[0][NJ+1] = oldMatrix[NI][1];
		oldMatrix[NI+1][NJ+1] = oldMatrix[1][1];
		oldMatrix[NI+1][0] = oldMatrix[1][NJ];

		/* left-right boundary conditions */
		for(i=1; i<=NI; i++){
			oldMatrix[i][0] = oldMatrix[i][NJ];
			oldMatrix[i][NJ+1] = oldMatrix[i][1];
		}

		/* top-bottom boundary conditions */
		for(j=1; j<=NJ; j++){
			oldMatrix[0][j] = oldMatrix[NI][j];
			oldMatrix[NI+1][j] = oldMatrix[1][j];
		}

		for(i=1; i<=NI; i++){
			for(j=1; j<=NJ; j++){
				im = i - 1;
				ip = i + 1;
				jm = j - 1;
				jp = j + 1;
				nsum =
					oldMatrix[im][jp] + oldMatrix[i][jp] + oldMatrix[ip][jp] +
					oldMatrix[im][j] +                     oldMatrix[ip][j] +
					oldMatrix[im][jm] + oldMatrix[i][jm] + oldMatrix[ip][jm];

				switch(nsum){
					case 3:
						newMatrix[i][j] = 1;
						break;
					case 2:
						newMatrix[i][j] = oldMatrix[i][j];
						break;
					default:
						newMatrix[i][j] = 0;
						break;
				}
			}
		}

		/* copy newMatrix state into oldMatrix state */
		for(i = 1; i <= NI; i++){
			for(j = 1; j <= NJ; j++){
				oldMatrix[i][j] = newMatrix[i][j];
			}
		}
	}

	/*  Iterations are done; sum the number of live cells */
	isum = 0;

	for(i = 1; i <= NI; i++){
		for(j = 1; j <= NJ; j++){
			isum = isum + newMatrix[i][j];
		}
	}

	printf("\nNumber of live cells = %d\n", isum);
	return 0;
}
