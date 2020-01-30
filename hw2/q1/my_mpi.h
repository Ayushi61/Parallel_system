#include <stdio.h>
#include <stdlib.h>

#define MPI_INT 4
#define MPI_DOUBLE 8

#define MPI_COMM_WORLD 0
typedef int MPI_Comm;
typedef int MPI_Status;

typedef int MPI_Datatype;
int MPI_Init(int *argc, char **argv[]);
int MPI_Barrier( MPI_Comm comm);
int MPI_Finalize(void);
int MPI_Comm_size(MPI_Comm comm, int *size);
int MPI_Comm_rank(MPI_Comm comm,int *rank);
int MPI_Get_processor_name(char* hostname,int *len);
int MPI_Sendrecv(const void *sendbuff, int sendcount, MPI_Datatype sendtype,int dest, int sendtag,void *recvbuf, int recvcount, MPI_Datatype recvtype,int source, int recvtag,MPI_Comm comm, MPI_Status *status);
int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);
int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status);
