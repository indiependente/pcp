#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mpi.h"

#define NI 200
#define NJ 200
#define NSTEPS 500

int main(int argc, char *argv[]){
	int i, j, n, im, ip, jm, jp, nsum, isum, isum1, nprocs ,myid;
	int ig, jg, i1g, i2g, j1g, j2g, ninom, njnom, ninj,
	i1, i2, i2m, j1, j2, j2m, ni, nj;
	int niproc, njproc;
	int **oldM, **newM, *oldMatrix1d, *newMatrix1d;
	MPI_Status status;
	float x;

	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD,&nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD,&myid);

	// nominal number of points per proc.
	niproc = nprocs; njproc = 1;
	ninom = NI/niproc; njnom = NJ/njproc;

	// global starting and ending indices
	i1g = (myid*ninom) + 1;
	i2g = i1g+ninom-1;
	j1g = 1;
	j2g = NJ;

	// local starting and ending indices
	i1 = 0;
	i2 = ninom + 1;
	i2m = i2-1;
	j1 = 0;
	j2 = NJ+1;
	j2m = j2-1;

	ni = i2-i1+1;
	nj = j2-j1+1;
	ninj = ni*nj;

	oldMatrix1d = malloc(ninj*sizeof(int));
	newMatrix1d = malloc(ninj*sizeof(int));
	oldM = malloc(ni*sizeof(int*));
	newM = malloc(ni*sizeof(int*));

	for(i=0; i<ni; i++){
		oldM[i] = &oldMatrix1d[i*nj];
		newM[i] = &newMatrix1d[i*nj];
	}

	for(ig=1; ig<=NI; ig++){
		for(jg=1; jg<=NJ; jg++){
			x =	rand()/((float) RAND_MAX + 1);
			if (ig >= i1g && ig <= i2g ){
				i = ig - i1g + 1; j = jg;
				if(x<0.5){
					oldM[i][j] = 0;

				} else {
					oldM[i][j] = 1;
				}
			}
		}
	}

	for(n=0; n<NSTEPS; n++){
		for(i=1; i<i2; i++){
			oldM[i][0] = oldM[i][j2m];
			oldM[i][j2] = oldM[i][1];
		}
		if(nprocs == 1){ // only one processor
			for(j=1; j<j2; j++){
				oldM[0][j] = oldM[i2m][j];
				oldM[i2][j] = oldM[1][j];
			}
			oldM[0][0] = oldM[i2m][j2m];
			oldM[0][j2] = oldM[i2m][1];
			oldM[i2][0] = oldM[1][j2m];
			oldM[i2][j2] = oldM[1][1];
		} else { // more than one processor
			if (myid == 0){
				MPI_Send(&oldM[i2-1][0], nj, MPI_INT, 1, 0, MPI_COMM_WORLD);
				MPI_Recv(&oldM[i2][0], nj, MPI_INT, 1, 1, MPI_COMM_WORLD, &status);
				MPI_Send(&oldM[1][0], nj, MPI_INT, 1, 2, MPI_COMM_WORLD);
				MPI_Recv(&oldM[0][0], nj, MPI_INT, 1, 3, MPI_COMM_WORLD, &status);
				/* corners */
				MPI_Send(&oldM[1][1], 1, MPI_INT, 1, 10, MPI_COMM_WORLD);
				MPI_Recv(&oldM[0][0], 1, MPI_INT, 1, 11, MPI_COMM_WORLD, &status);
				MPI_Send(&oldM[1][j2m], 1, MPI_INT, 1, 12, MPI_COMM_WORLD);
				MPI_Recv(&oldM[0][j2], 1, MPI_INT, 1, 13, MPI_COMM_WORLD, &status);
			} else { // proc. 1
				/* top and bottom rows */
				MPI_Recv(&oldM[0][0], nj, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
				MPI_Send(&oldM[1][0], nj, MPI_INT, 0, 1, MPI_COMM_WORLD);
				MPI_Recv(&oldM[i2][0], nj, MPI_INT, 0, 2, MPI_COMM_WORLD, &status);
				MPI_Send(&oldM[i2-1][0], nj, MPI_INT, 0, 3, MPI_COMM_WORLD);
				/* corners */
				MPI_Recv(&oldM[i2][j2], 1, MPI_INT, 0, 10, MPI_COMM_WORLD, &status);
				MPI_Send(&oldM[i2m][j2m], 1, MPI_INT, 0, 11, MPI_COMM_WORLD);
				MPI_Recv(&oldM[i2][0], 1, MPI_INT, 0, 12, MPI_COMM_WORLD, &status);
				MPI_Send(&oldM[i2m][1], 1, MPI_INT, 0, 13, MPI_COMM_WORLD);
			} // end proc. 1
		} // end "more than one processor"
		for(i=1; i<i2; i++){
			for(j=1; j<j2; j++){
				im = i-1;
				ip = i+1;
				jm = j-1;
				jp = j+1;
				nsum=oldM[im][jp]+oldM[i][jp]+oldM[ip][jp] +oldM[im][j ]+oldM[ip][j]+oldM[im][jm] + oldM[i][jm] + oldM[ip][jm];
				switch(nsum){
				case 3:
					newM[i][j] = 1;
					break;
				case 2:
					newM[i][j] = oldM[i][j];
					break;
				default:
					newM[i][j] = 0;
					break;
					// end switch end for j
				}
			} //
		} // end for i
		/* copy newM state into oldMatrix state */
		for(i=0; i<ni; i++){
			for(j=0; j<nj; j++){
				oldM[i][j] = newM[i][j];
			}
		}
	} // end for(n=0; n<NSTEPS; n++){

	isum = 0;
	for(i=1; i<i2; i++){
		for(j=1; j<j2; j++){
			isum = isum + newM[i][j];
		}
	}

	if (nprocs > 1){
		if(myid == 0){
			MPI_Recv(&isum1, 1, MPI_INT, 1, 20,
					MPI_COMM_WORLD, &status);
			isum = isum + isum1;
		}else{
			MPI_Send(&isum, 1, MPI_INT, 0, 20,
					MPI_COMM_WORLD);
		}
	}

	if(myid == 0)
		printf("Number of live cells = %d\n", isum);

	MPI_Finalize();
	return 0;
}
