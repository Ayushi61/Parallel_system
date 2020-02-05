#include "my_mpi.h"
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <math.h>
int main(int argc, char *argv[])
{
    int  numproc,rank,len, pair_send, pair_rcv;
    int *buffer, *buffer2;
    MPI_Request request;
    MPI_Status status;
    char  hostname[MPI_MAX_PROCESSOR_NAME]; 
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numproc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Get_processor_name(hostname, &len);
    int i,size,j;
    double avg,std;
    int rtt[10];
    struct timeval start, end;
    size=4;
    if(rank<(numproc/2))
        {
                pair_send=(numproc/2)+rank;
                pair_rcv=pair_send;
        }
    else
        {
                pair_send=rank%(numproc/2);
                pair_rcv=pair_send;
        }

    for(i=size;i<=262144;i*=2)
    {
     buffer=(int *)malloc(i*sizeof(int));
    buffer2=(int *)malloc(i*sizeof(int));
    *buffer=rank;
     
    for(j=0;j<10;j++)
    {
    gettimeofday(&start, NULL);
    MPI_Sendrecv(buffer, i, MPI_INT, pair_send, 123, buffer2, i, MPI_INT, pair_rcv, 123, MPI_COMM_WORLD, &status);
    gettimeofday(&end, NULL);
    rtt[j]=((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));
    }
    avg=0;
    for(j=1;j<10;j++)
    {
	avg+=rtt[j];
    }
    avg=avg/9;
    std=0;
    for(j=1;j<10;j++)
    {
       std+=pow(rtt[j]-avg,2);

    }
    std=sqrt(std/9);
    printf("avg is %lf and std is %lf for process %d size= %d \n",avg,std, rank,i*sizeof(int));
    free(buffer);
    free(buffer2);
    }
    MPI_Finalize();
    return 0;
}
