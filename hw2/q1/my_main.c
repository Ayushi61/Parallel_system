#include "my_mpi.h"
int main(int argc,char *argv[])
{
        int  numproc,rank,len, pair_send, pair_rcv;
        int *bufferSend;
        int *bufferRecv;
        char hostname[MPI_MAX_PROCESSOR_NAME];
        printf("before init\n");
        MPI_Init(&argc,&argv);
        printf("contents of hostname is %s\n",MPI_COMM_WORLD.hostnames[0]);
        printf("%d is rank global mpi\n",MPI_COMM_WORLD.rank);
        MPI_Comm_size(MPI_COMM_WORLD, &numproc);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Get_processor_name(hostname, &len);
        MPI_Status status;
        MPI_Request request;
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
        bufferSend=(int *)malloc(2*1024*1024);
        bufferRecv=(int *)malloc(2*1024*1024);
        *(bufferSend+127)=32;
        MPI_Sendrecv(bufferSend, 2*1024*1024, MPI_INT, pair_send, 123, bufferRecv, 2*1024*1024, MPI_INT, pair_rcv, 123, MPI_COMM_WORLD, &status);
        //MPI_Send(bufferSend,1024*1024,MPI_INT,pair_send,123,MPI_COMM_WORLD);
        printf("data sent---------------------\n");
        //MPI_Recv(bufferRecv,1024*1024,MPI_INT,pair_rcv,123,MPI_COMM_WORLD,&status);
        printf("data recvd-------------%d\n",bufferRecv[127]);
        free(bufferSend);
        free(bufferRecv);
        MPI_Finalize();
        return 0;
        
        
        
}
