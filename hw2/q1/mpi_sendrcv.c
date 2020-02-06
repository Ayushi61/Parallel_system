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
    FILE *fp;
    char  hostname[MPI_MAX_PROCESSOR_NAME]; 
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numproc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Get_processor_name(hostname, &len);
    double avg_proc[numproc];
    double std_proc[numproc];
    int i,size,j;
    double avg,std;
    double rtt[11];
    //double tii
    if(rank==0)
    {
	    fp=fopen("op_data.dat","w+");
	    fprintf(fp,"size p1 p2 p3 p4 p5 p6 p7 p8\n");
    }
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
     
    for(j=0;j<=10;j++)
    {
    gettimeofday(&start, NULL);
    MPI_Sendrecv(buffer, i, MPI_INT, pair_send, 123, buffer2, i, MPI_INT, pair_rcv, 123, MPI_COMM_WORLD, &status);
    gettimeofday(&end, NULL);
    rtt[j]=((end.tv_sec + (double)(end.tv_usec)/1000000) - (start.tv_sec + (double)start.tv_usec/1000000));
    //rtt[j]=(double)(end-start)
    //rtt[j]=(end.tv_sec - start.tv_sec) * 1e6; 
    rtt[j]=(rtt[j] + (end.tv_usec -  start.tv_usec))*1e-6;
    
    }
    avg=0;
    for(j=0;j<=10;j++)
    {
	if(j!=0)
		avg+=rtt[j];
    }
    avg=avg/10;
    std=0;
    for(j=0;j<=10;j++)
    {
	if(j!=0)
       		std+=pow(rtt[j]-avg,2);

    }
    std=sqrt(std/10);
    printf("avg is %lf and std is %lf for process %d size= %d \n",avg,std, rank,i*sizeof(int));
    /*if(rank!=0)
	{
		MPI_Send(&avg,1,MPI_DOUBLE,0,rank,MPI_COMM_WORLD);
		MPI_Send(&std,1,MPI_DOUBLE,0,rank*2,MPI_COMM_WORLD);
	}
    else
	{
	avg_proc[0]=avg;
	std_proc[0]=std;
	

        int j;
	fprintf(fp,"%d ",i*sizeof(int));
	
	fprintf(fp,"%e %e ",avg_proc[0],std_proc[0]);
	for(j=1;j<numproc;j++)
	{
		MPI_Recv(&avg_proc[j],1,MPI_DOUBLE,j,j,MPI_COMM_WORLD,&status);
		MPI_Recv(&std_proc[j],1,MPI_DOUBLE,j,j*2,MPI_COMM_WORLD,&status);
		fprintf(fp,"%e %e ",avg_proc[j],std_proc[j]);
	}
	fprintf(fp,"\n");
	}
    if(rank==0)
	{
		printf("1avg1 is %lf and std is %lf for process %d size= %d \n",avg,std, rank,i*sizeof(int)+1);
		
	}*/
    }
   /*if(rank==0)
	
    fclose(fp);*/

    MPI_Finalize();
    return 0;
}
