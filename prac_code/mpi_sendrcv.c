#include "mpi.h"
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
int main(int argc, char *argv[])
{
    int  numproc,rank,len, pair_send, pair_rcv;
    int *buffer, *buffer2;
    MPI_Request request;
    MPI_Status status;
    //printf("size of int is %d\n",sizeof(int));
    char  hostname[MPI_MAX_PROCESSOR_NAME]; 
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numproc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Get_processor_name(hostname, &len);
    int i,size;
    struct timeval start, end;
    size=8;
    for(i=size;i<512;i*=2)
    {
    buffer=(int *)malloc(i*sizeof(int));
    buffer2=(int *)malloc(i*sizeof(int));
    *buffer=rank;
    printf ("Hello from task %d on %s!\n", rank, hostname);
    //pair_rcv = (rank + 1) % numproc;
    //pair_send = rank - 1;
    //if (pair_send < 0)
    //    pair_send = numproc - 1;
    if(rank<(numproc/2))
	{
//		buffer[0]=rank;
		pair_send=(numproc/2)+rank;
		pair_rcv=pair_send;
	}
    else
	{
//		buffer[0]=rank;
		pair_send=rank%(numproc/2);
		pair_rcv=pair_send;
	}
    //printf("received message is %d\n",buffer2[0]);
    printf("message sending prco number : %d\n", rank);
    gettimeofday(&start, NULL);
    MPI_Sendrecv(buffer, i, MPI_INT, pair_send, 123, buffer2, i, MPI_INT, pair_rcv, 123, MPI_COMM_WORLD, &status);
    gettimeofday(&end, NULL);
    printf("time taken for pair %d-%d is %ld for size %d\n", rank,pair_send, ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec)),i*sizeof(int));
    printf("received message is %d from %d\n",*buffer2,rank); 
    }
    MPI_Finalize();
    return 0;
}
