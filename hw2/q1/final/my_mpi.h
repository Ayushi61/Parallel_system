#include <stdio.h>
#include <stdlib.h>
#include  <stdbool.h>
#define MPI_MAX_PROCESSOR_NAME 128
#define MPI_INT 4
#define MPI_DOUBLE 8
#define HOST_LEN 256
//#define MPI_COMM_WORLD 0
typedef struct mpi_Comm{
	int rank;
	int numproc;
	char **hostnames;
	//char *root_host;

}MPI_Comm;
MPI_Comm MPI_COMM_WORLD;
typedef int MPI_Status;
typedef int MPI_Request;
typedef int MPI_Datatype;
extern int MPI_Init(int *argc, char **argv[]);
extern int MPI_Barrier( MPI_Comm comm);
extern int MPI_Finalize(void);
extern int MPI_Comm_size(MPI_Comm comm, int *size);
extern int MPI_Comm_rank(MPI_Comm comm,int *rank);
extern int MPI_Get_processor_name(char* hostname,int *len);
extern int MPI_Sendrecv(const void *sendbuff, int sendcount, MPI_Datatype sendtype,int dest, int sendtag,void *recvbuf, int recvcount, MPI_Datatype recvtype,int source, int recvtag,MPI_Comm comm, MPI_Status *status);
extern int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);
extern int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status);

