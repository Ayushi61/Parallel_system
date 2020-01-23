#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpi.h"

/* first grid point */
#define XI      1.0
#define XF      100.0
#define BLK     0
#define NBLK    1
#define MPI_G   0
#define MAN_G   1
#define XC_TAG  13
#define YC_TAG  17
#define DYC_TAG 23

/* function declarations */
double  fn(double);
void    print_function_data(int, double*, double*, double*);

int main (int argc, char *argv[])
{
    unsigned int NGRID, p2p_typ, gat_typ;
    NGRID = atoi(argv[1]);
    p2p_typ = atoi(argv[2]);
    gat_typ = atoi(argv[3]);

    int numproc, rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numproc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_Status status;
    MPI_Request send_dummy, left_rcv, right_rcv;
    MPI_Request gather_data[3*numproc];

    /* When all arguments are correct */
    if( argc==4 && NGRID!=0 && (gat_typ==MAN_G || gat_typ==MPI_G) && (p2p_typ==BLK || p2p_typ==NBLK) )
    {
        /* Argument's feedback for user */
        if( rank==0 )
        {
            printf("Parmeters provided are:\nNGRID: %d\n", NGRID );

            printf("Point to point communication type: ");
            p2p_typ==0 ? printf("Blocking\n") : printf("Non-blocking\n");

            printf("Gathering version: ");
            gat_typ==0 ? printf("MPI_Gather\n") : printf("Manual gather\n");
        }

        double start_time = MPI_Wtime();
        double end_time, average_time;
        
        int i, my_start, my_end;
        int *start  =   (int *)malloc(sizeof(int) * numproc);
        int *end    =   (int *)malloc(sizeof(int) * numproc);
        int *counts =   (int *)malloc(sizeof(int) * numproc);
        int *disp   =   (int *)malloc(sizeof(int) * numproc);

        /* Distribution of grid points among the ranks */
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
        
            /* Calculation of recieve counts of data and offset for MPI_Gatherv */
            counts[i] = end[i] - start[i] + 1;
            disp[i] = start[i] - 1;
        }
        my_start = start[rank], my_end = end[rank];
        free(start);
        free(end);
        /* Workload distribution section ends */

        double dx;
        double *xc =    (double *)malloc(sizeof(double) * (my_end-my_start+3));
        double *yc =    (double *)malloc(sizeof(double) * (my_end-my_start+3));
        double *dyc =   (double *)malloc(sizeof(double) * (my_end-my_start+2));
        double *fxc =   (double *)malloc(sizeof(double) * NGRID);
        double *fyc =   (double *)malloc(sizeof(double) * NGRID);
        double *fdyc =  (double *)malloc(sizeof(double) * NGRID);
        
        /* XC calcualtion section */
        for(i=my_start; i<=my_end; i++)
            xc[i-my_start+1] = XI + (XF - XI) * (double)(i-1)/(double)(NGRID - 1);
        dx = xc[2] - xc[1];
        xc[0] = xc[1] - dx;
        xc[my_end-my_start+2] = xc[my_end-my_start+1] + dx;
        /* XC calculation section ends. */

        /* Start sending XC if manual gather and non-blocking */
        if( gat_typ == MAN_G && p2p_typ==NBLK )
        {
            if( rank != 0 )
                    //MPI_Isend( &xc[1], counts[rank], MPI_DOUBLE, 0, XC_TAG*rank, MPI_COMM_WORLD, &send_dummy);
                    MPI_Isend( &xc[1], counts[rank], MPI_DOUBLE, 0, 456, MPI_COMM_WORLD, &send_dummy);
            else
                for( i=1; i<numproc; i++ )
                    MPI_Irecv( fxc+disp[i], counts[i], MPI_DOUBLE, i, 456, MPI_COMM_WORLD, &gather_data[3*(i-1)]);
                    //MPI_Irecv( fxc+disp[i], counts[i], MPI_DOUBLE, i, XC_TAG*i, MPI_COMM_WORLD, &gather_data[3*(i-1)]);
        }

        /* YC calculation section */
        yc[0] = rank==0 ? fn(xc[0]) : 0;
        yc[my_end-my_start+2] = rank==numproc-1 ? fn(xc[my_end-my_start+2]) : 0;

        /* setting up non-blocking recieve for yc edge values */
        if( p2p_typ == NBLK )
        {
            if( rank != 0 )
                MPI_Irecv(&yc[0], 1, MPI_DOUBLE, rank-1, 123457, MPI_COMM_WORLD, &left_rcv);

            if( rank != numproc-1 )
                MPI_Irecv(&yc[my_end-my_start+2], 1, MPI_DOUBLE, rank+1, 123456, MPI_COMM_WORLD, &right_rcv);
        }
        
        /* Calculating my own YC */
        for(i=my_start; i<=my_end; i++)
            yc[i-my_start+1] = fn(xc[i-my_start+1]);

        /* All YC calcs are done, so start sending them over to root */
        if( gat_typ == MAN_G && p2p_typ==NBLK )
        {
            if( rank != 0 )
		{    
			MPI_Isend( &yc[1], counts[rank], MPI_DOUBLE, 0, 456, MPI_COMM_WORLD, &send_dummy);
        		MPI_Wait(&send_dummy, &status);
	        }
		    //MPI_Isend( &yc[1], counts[rank], MPI_DOUBLE, 0, YC_TAG*rank, MPI_COMM_WORLD, &send_dummy);
            else
                for( i=1; i<numproc; i++)
                    MPI_Irecv( fyc+disp[i], counts[i], MPI_DOUBLE, i, 456, MPI_COMM_WORLD, &gather_data[(3*(i-1))+1] );
                    //MPI_Irecv( fyc+disp[i], counts[i], MPI_DOUBLE, i, YC_TAG*i, MPI_COMM_WORLD, &gather_data[(3*(i-1))+1] );
        }
        

	/* Send my edge YC for other nodes */    
        if( p2p_typ == BLK ) /* When blocking send and recieve here - ayushi working on resolving deadlock risks */
        {
            if( rank != 0 )
            {
                MPI_Send(&yc[1], 1, MPI_DOUBLE, rank-1, 123, MPI_COMM_WORLD);
                MPI_Recv(&yc[0], 1, MPI_DOUBLE, rank-1, 123, MPI_COMM_WORLD, &status);
            }

            if( rank != numproc-1 )
            {
                MPI_Send(&yc[my_end-my_start+1], 1, MPI_DOUBLE, rank+1, 123, MPI_COMM_WORLD);
                MPI_Recv(&yc[my_end-my_start+2], 1, MPI_DOUBLE, rank+1, 123, MPI_COMM_WORLD, &status);
            }
        }
        else /* Non-blocking sending of boundary values */
        {
            if( rank != 0 )
                MPI_Isend(&yc[1], 1, MPI_DOUBLE, rank-1, 123456, MPI_COMM_WORLD, &send_dummy);

            if(rank!=numproc-1)
                MPI_Isend(&yc[my_end-my_start+1], 1, MPI_DOUBLE, rank+1, 123457, MPI_COMM_WORLD, &send_dummy);
        }
        /* YC calculation section ends */

        /* wait for left and right to be recieved before proceeding for dyc */
        if( p2p_typ==NBLK ) /* Only applicable for non-blocking, blocking already garunteed to be receieved */
        {
            /* Non blocking manual gather to be setup here - ayushi working on it. */
            if(rank != 0)
                MPI_Wait(&left_rcv, &status);

            if(rank != numproc-1)
                MPI_Wait(&right_rcv, &status);
        }

        for(i=my_start; i<=my_end; i++)
            dyc[i-my_start+1] = (yc[i-my_start+2] - yc[i-my_start])/(2.0 * dx);


        if( gat_typ == MAN_G && p2p_typ==NBLK )
        {
            if(rank!=0)
                MPI_Isend( &dyc[1], counts[rank], MPI_DOUBLE, 0, 456, MPI_COMM_WORLD, &send_dummy ); // need to use diff tags
                //MPI_Isend( &dyc[1], counts[rank], MPI_DOUBLE, 0, DYC_TAG*rank, MPI_COMM_WORLD, &send_dummy ); // need to use diff tags
            else
                for(i=1;i<numproc; i++)
                    MPI_Irecv( fdyc+disp[i], counts[i], MPI_DOUBLE, i, 456, MPI_COMM_WORLD, &gather_data[(3*(i-1))+2] );
                    //MPI_Irecv( fdyc+disp[i], counts[i], MPI_DOUBLE, i, DYC_TAG*i, MPI_COMM_WORLD, &gather_data[(3*(i-1))+2] );
        }

        /* need to setup non-bloaking manual  */

        if( gat_typ == MPI_G ) /* Blocking MPI gathers (0,0)*/
        {
            MPI_Gatherv( &xc[1], my_end-my_start+1, MPI_DOUBLE, fxc, counts, disp, MPI_DOUBLE, 0, MPI_COMM_WORLD);
            MPI_Gatherv( &yc[1], my_end-my_start+1, MPI_DOUBLE, fyc, counts, disp, MPI_DOUBLE, 0, MPI_COMM_WORLD);
            MPI_Gatherv( &dyc[1], my_end-my_start+1, MPI_DOUBLE, fdyc, counts, disp, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        }
        else /* Blocking manual gathers (0,1)*/
        {
            if( p2p_typ==BLK )
            {
                if( rank!=0 )
                {
                    MPI_Send(&xc[1], my_end-my_start+1, MPI_DOUBLE, 0, 123, MPI_COMM_WORLD);
                    MPI_Send(&yc[1], my_end-my_start+1, MPI_DOUBLE, 0, 124, MPI_COMM_WORLD);
                    MPI_Send(&dyc[1], my_end-my_start+1, MPI_DOUBLE, 0, 125, MPI_COMM_WORLD);
                }
                else
                    for(i=1; i<numproc; i++)
                    {
                        MPI_Recv( fxc+disp[i], counts[i], MPI_DOUBLE, i, 123, MPI_COMM_WORLD, &status);
                        MPI_Recv( fyc+disp[i], counts[i], MPI_DOUBLE, i, 124, MPI_COMM_WORLD, &status);
                        MPI_Recv( fdyc+disp[i], counts[i], MPI_DOUBLE, i, 125, MPI_COMM_WORLD, &status);
                    }
            }

            if(rank == 0 )
                for(i=0; i<my_end; i++)
                {
                    fxc[i] = xc[i+1];
                    fyc[i] = yc[i+1];
                    fdyc[i] = dyc[i+1];
                }
        }



        if( gat_typ == MAN_G && p2p_typ==NBLK && rank == 0 )
	{
		for(i=0;i<=numproc*3-4;i++)
	        	MPI_Wait(&gather_data[i], &status);
	}
        /*
        if( gat_typ == MAN_G && p2p_typ==NBLK && rank == 0 )
        {
		int count_flag = 0;
		i = 0;
		int flag;
		while(1)
		{
			if(gather_data[i] != MPI_REQUEST_NULL)
			{
				MPI_Test(&gather_data[i], &flag, &status);
				count_flag += flag;
					
			}
			if(count_flag == (3*numproc-3))
			{
				break;
			}
			if(i == 3*numproc -4)
			{
				i = -1;
			}
			i++;
		}
        }
	*/
	
        end_time = MPI_Wtime()-start_time;
        MPI_Reduce( &end_time, &average_time, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
        average_time /= numproc;

        if( rank==0 )
            print_function_data(NGRID, fxc, fyc, fdyc);
        
        /* Freeing all the the allocated variables */
        free(counts);
        free(disp);
        free(xc);
        free(yc);
        free(dyc);
        free(fxc);
        free(fyc);
        free(fdyc);
        
        if( rank == 0 )
            printf("Processing time for %d parallels - %f seconds\n", numproc, end_time);
    }
    else
    {
        /* When arguments are incorrect, notify user of syntax and exit */
        if( rank==0 )
        {
            printf("Invalid syntax.\n");
            printf("Please follow the syntax as: %s <number of grid points> <point-to-point_type> <gather_type>\n", argv[0]);
            printf("<number of grid points> must be a positive number.\n");
            printf("<point-to-point_type> must be a 0 or 1\n\t0: For blocking communication.\n\t1: For non-blocking communication\n");
            printf("<gather_type> must be a 0 or 1\n\t0: With MPI_Gather\n\t1: With Manual gather\n");
            printf("Exiting...\n");
        }
    }

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
