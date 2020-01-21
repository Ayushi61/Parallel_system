#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpi.h"

/* first grid point */
#define   XI	1.0
#define   XF	100.0

/* function declarations */
double	fn(double);
void	print_function_data(int, double*, double*, double*);
int	main(int, char**);

int main (int argc, char *argv[])
{
        int NGRID, p2p_typ, gat_typ;

	if( argc==4 )
	{
		NGRID = atoi(argv[1]);
		p2p_typ = atoi(argv[2]);
		gat_typ = atoi(argv[3]);
		
		/*set up boundary for p2p and gat*/
        }
        else
        {
                printf("Please specify the number of grid points.\n");
                exit(0);
        }

	int numproc, rank;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numproc);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	double start_time = MPI_Wtime();
	
	int i;
	int *start= (int *)malloc(sizeof(int)*numproc);
	int *end = (int *)malloc(sizeof(int)*numproc);
	int *counts = (int *)malloc(sizeof(int)*numproc);
	int *disp = (int *)malloc(sizeof(int)*numproc);

	for(i=0; i<numproc; i++)
	{
		if(i < NGRID%numproc)
        	{
            		start[i] = 1 + (NGRID/numproc)*i + i;
            		end[i] = start[i] + (NGRID/numproc);
        	}
        	else
        	{
            		start[i] = 1 + (NGRID/numproc)*i +(NGRID%numproc);
            		end[i] = start[i] + (NGRID/numproc) -1;
        	}
	
		counts[i] = end[i] - start[i] + 1;
		disp[i] = start[i] - 1;
	}

	double dx;
	double *xc =	(double *)malloc(sizeof(double) * (end[rank]-start[rank]+3));
	double *yc =	(double *)malloc(sizeof(double) * (end[rank]-start[rank]+3));
	double *dyc =	(double *)malloc(sizeof(double) * (end[rank]-start[rank]+2));
	double *fxc =	(double *)malloc(sizeof(double) * NGRID);
	double *fyc =	(double *)malloc(sizeof(double) * NGRID);
	double *fdyc =	(double *)malloc(sizeof(double) * NGRID);
	
	for(i=start[rank]; i<=end[rank]; i++)
                xc[i-start[rank]+1] = XI + (XF - XI) * (double)(i-1)/(double)(NGRID - 1);

	dx = xc[2] - xc[1];
	xc[0] = xc[1] - dx;
	xc[end[rank]-start[rank]+2]= xc[end[rank]-start[rank]+1] + dx;

	for(i=start[rank]-1; i<=end[rank]+1; i++)
                yc[i-start[rank]+1] = fn(xc[i-start[rank]+1]);

	for(i=start[rank]; i<=end[rank]; i++)
                dyc[i-start[rank]+1] = (yc[i-start[rank]+2] - yc[i-start[rank]])/(2.0 * dx);

	if( gat_typ == 0 )
	{
		MPI_Gatherv( &xc[1], end[rank]-start[rank]+1, MPI_DOUBLE, fxc, counts, disp, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		MPI_Gatherv( &yc[1], end[rank]-start[rank]+1, MPI_DOUBLE, fyc, counts, disp, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		MPI_Gatherv( &dyc[1], end[rank]-start[rank]+1, MPI_DOUBLE, fdyc, counts, disp, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	}

	if(rank==0)
		print_function_data(NGRID, fxc, fyc, fdyc);
	
	free(start);
	free(end);
	free(counts);
	free(disp);
	free(xc);
	free(yc);
	free(dyc);
	free(fxc);
	free(fyc);
	free(fdyc);

	printf("%d - %f\n", rank, MPI_Wtime()-start_time);

	MPI_Finalize();
}

void print_function_data(int np, double *x, double *y, double *dydx)
{
        int   i;

        char filename[1024];
        sprintf(filename, "fn-%d.dat", np);

        FILE *fp = fopen(filename, "w");

        for(i = 0; i < np; i++)
                fprintf(fp, "%f %f %f\n", x[i], y[i], dydx[i]);

        fclose(fp);
}
