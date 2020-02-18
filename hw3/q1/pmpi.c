/*
  Single Author info:
  arajend4 Ayushi Rajendra Kumar 
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int *send_event;
int *send_event_0;
int rank;
int numproc;

int MPI_Init(int *argc, char ***argv )
{
	
	PMPI_Init(argc, argv);
	PMPI_Comm_rank(MPI_COMM_WORLD, &rank);
	PMPI_Comm_size(MPI_COMM_WORLD, &numproc);
	send_event=(int *)calloc(numproc,sizeof(int));
	if(rank==0)
		send_event_0=(int *)malloc(numproc*numproc*sizeof(int));
	return 0;
}

int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
{
	PMPI_Send(buf,count,datatype,dest,tag,comm);
	send_event[dest]++;
	return 0;

}


int MPI_Isend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request * request)
{
	PMPI_Isend(buf,count,datatype,dest,tag,comm,request);
	send_event[dest]++;
        return 0;

}


int MPI_Finalize(void)
{
	FILE *matrix;
	int i,j;
	PMPI_Gather(send_event,numproc,MPI_INT,send_event_0,numproc,MPI_INT,0,MPI_COMM_WORLD);
	if(rank==0)
	{
		matrix=fopen("matrix.data","w");
		for(i=0;i<numproc;i++)
		{
			fprintf(matrix,"%d ",i);
			//printf("%d ",i);
			for(j=0;j<numproc;j++){
				fprintf(matrix, "%d ",send_event_0[i*numproc+j]);
				//printf("%d ",send_event_0[i*numproc+j]);
			}
			fprintf(matrix,"\n");	
			//printf("\n");
		}
		
		fclose(matrix);
	}	
	PMPI_Finalize();
	return 0;
}

