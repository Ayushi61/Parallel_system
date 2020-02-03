#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include "my_mpi.h"
#include <netdb.h>
bool flag=false;
bool flag2=false;
void server_1();
int MPI_Barrier( MPI_Comm);
int *buff_glob_0;
int newsockfd_glob=-1;
bool barrier=false;
bool rcv_flag=false;
int **rcv_ptr;
int rcv_cnt;
MPI_Comm MPI_COMM_WORLD;
char * buffer_client;
typedef	struct topass{
		int portno;
		MPI_Comm comm;
	}arg;
void error(const char *msg)
{
    perror(msg);
    exit(1);
}
void getNumProcRank(char* rank,int* rank_ret,char** hostName)
{
	FILE *fp;
	char* fileName="nodefile.txt";
	fp=fopen(fileName,"r");
	char str[100];
	char **line;
	line=(char **) malloc(sizeof(char *)*128);
	int k;
	for(k=0;k<128;k++)
	{
		*(line+k)=(char *)malloc(sizeof(char)*10);
	}
	char c;
    	if (fp == NULL){
        	printf("Could not open file %s",fileName);
    	}
	int i=0;
    	while (fgets(line[i], 128, fp))
	{

		line[i][strlen(line[i])-1]='\0';

		
        	if(strcmp(rank,line[i])==0)
		{
			printf("%s and rank is %d\n",line[i],i);
			rank_ret[0]=i;
		}
			strcpy(hostName[i],line[i]);
			printf(" addr to mactch is %s\n",hostName[i]);

		i++;
	}
	printf("numproc=%d\n",i);
	rank_ret[1]=i;
    	fclose(fp);
	for(k=0;k<128;k++)
                free(*(line+k));
        free(line);

}

void client(char* hostname,int portno,MPI_Comm comm,int bytes,int *msg)
{
    	int sockfd, n;
   	struct sockaddr_in serv_addr;
    	struct hostent *server;
    	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	printf("entered client ---%d,%s\n",portno,hostname);
    	if (sockfd < 0) 
        	error("ERROR opening socket");
    	server = gethostbyname(hostname);
    	if (server == NULL) {
        	fprintf(stderr,"ERROR, no such host\n");
        	exit(0);
    	}
   	bzero((char *) &serv_addr, sizeof(serv_addr));
    	serv_addr.sin_family = AF_INET;
    	bcopy((char *)server->h_addr, 
        (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);
    	serv_addr.sin_port = htons(portno);
	int ctr=0;

	while(ctr!=3)
	{
	
    		if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
		{ 
        		sleep(1);	
			ctr++;
		}
		else
			break;
	}
	if(ctr==3)
		error("ERROR connecting");
    	printf("Please enter the message: ");
	printf("message is %d to %s\n",msg[0],hostname);
    	if(barrier==true)
	{
		int *buff;
    		buff=(int *)malloc(sizeof(int)+bytes);
    		buff[0]=comm.rank;
    		int i,size;
		size=bytes/sizeof(int);
	

	    	for(i=0;i<size;i++)
		{
			buff[i+1]=msg[i];
		}
    		n = write(sockfd,buff,(bytes+(sizeof(int))));
		
		free(buff);
    	}
	else
	{
		n = write(sockfd,msg,bytes);
	}
	if (n < 0) 
		error("ERROR writing to socket");
	
    	close(sockfd);


}

void server_1(arg * args )
{
	int sockfd, newsockfd,pid;
     	socklen_t clilen;
    	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
     	int n;
	int portno=args->portno;
	MPI_Comm comm=args->comm;
	printf("%d port num\n",portno);
	printf("here rank is %d\n",comm.rank);
     	sockfd = socket(AF_INET, SOCK_STREAM, 0);
     	if (sockfd < 0) 
        	error("ERROR opening socket");
     	bzero((char *) &serv_addr, sizeof(serv_addr));
     	serv_addr.sin_family = AF_INET;
     	serv_addr.sin_addr.s_addr = INADDR_ANY;
     	serv_addr.sin_port = htons(portno);
     	if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
       		error("ERROR on binding");
	printf("ok till here\n");
	

	listen(sockfd,5);
	while(flag==false)
	{
	 	

     		printf("checking--------------------- \n");
		clilen = sizeof(cli_addr);
		if(portno!=30014)
		{
   	 		newsockfd = accept(sockfd, 
                 	(struct sockaddr *) &cli_addr, &clilen);
			printf("inside server while\n");
	     		if (newsockfd < 0) 
        	  		error("ERROR on accept");
  			if(barrier==true)
			{           
     				bzero(buffer,256);
				int * buff_glob;
				buff_glob=(int *)malloc(sizeof(int)*(524288+1));
				n = read(newsockfd,buff_glob,(2097152+sizeof(int)));
				printf("rcvd!!!!!!! %d\n",buff_glob[0]);
				if(comm.rank==0)
				{
					printf("comm.portno-30014+1 is %d\n",args->portno-30014);
					buff_glob_0[args->portno-30014]=buff_glob[0];
				}
				printf("server got---> %d\n",buff_glob[1]);
     				if (n < 0) error("ERROR reading from socket");
				free(buff_glob);
			}
			else
			{

				while(rcv_flag==false);
				n = read(newsockfd,*rcv_ptr,rcv_cnt);
				printf("i recieved\n");
				rcv_flag=false;

			}
		}	
		else
		{
	 		newsockfd = accept(sockfd,
         	        (struct sockaddr *) &cli_addr, &clilen);
                	printf("inside server while\n");
             		if (newsockfd < 0)
                  		error("ERROR on accept");
			if(barrier==true)
			{
        			bzero(buffer,256);
        			int * buff_glob;
     		   		buff_glob=(int *)malloc(sizeof(int)*(524288+1));
        			n = read(newsockfd,buff_glob,(2097152+sizeof(int)));
				printf("#####################is #################%d\n",buff_glob[0]);
				buff_glob_0[0]=buff_glob[0];
	        		printf("server got---> %d\n",buff_glob[1]);
        			if (n < 0) error("ERROR reading from socket");
        			free(buff_glob);
			}
			else
			{

				while(rcv_flag==false);
                                n = read(newsockfd,*rcv_ptr,rcv_cnt);
                                printf("i recieved\n");
				rcv_flag=false;


			}
		}
		
	}
     	close(newsockfd);
     	close(sockfd);
	pthread_exit(NULL);
     
}
int MPI_Init(int *argc, char **argv[])
{
	char* hostname;
	char hostbuffer[256];
    	int num_rank[2];
      	int numproc;
        int rank;
        const char** host_char;
        host_char=(char **)malloc(sizeof(char *)*128);
   	int k;
	for(k=0;k<128;k++)
	{
		*(host_char+k)=(char *)malloc(sizeof(char)*10);
	}

	hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    	printf("Hostname: %s\n", hostbuffer);
    	getNumProcRank(hostbuffer,num_rank,host_char);
        rank=num_rank[0];
        numproc=num_rank[1];
	MPI_COMM_WORLD.rank=rank;
	MPI_COMM_WORLD.numproc=numproc;
	MPI_COMM_WORLD.hostnames=(char **) malloc(sizeof(char *)*(numproc));
	int i;
        for(i=0;i<numproc;i++)
	{
		MPI_COMM_WORLD.hostnames[i]=host_char[i];
		
	}
        if(rank!=0)
		sleep(1);
	pthread_t *threads;
	threads=(pthread_t *)malloc(sizeof(pthread_t)*numproc);
	int rc;
   	int t;
	arg *args;

	buff_glob_0=(int *)malloc(sizeof(int)*(numproc));
	for(i=0;i<numproc;i++)
        {
                buff_glob_0[i]=-1;
        }
	if(rank==0)
	{
		
		for(t=1; t<numproc; t++){
			args=(arg *)malloc(sizeof(arg));
			args->comm=MPI_COMM_WORLD;
			args->portno=30014+t;
			printf("thread %d,%d\n",t,args->portno);
			rc = pthread_create(&threads[t-1], NULL, server_1,args);
			if (rc){
                		printf("ERROR; return code from pthread_create() is %d\n", rc);
                		exit(-1);
        		}
		}

	}
	args=(arg *)malloc(sizeof(arg));
	args->comm=MPI_COMM_WORLD;
	args->portno=30014;
	rc = pthread_create(&threads[0], NULL, server_1,args);
	if (rc){
        	printf("ERROR; return code from pthread_create() is %d\n", rc);
         	exit(-1);
      	}
	MPI_Barrier(MPI_COMM_WORLD);
	return 1;
}

int MPI_Barrier( MPI_Comm comm)
{
	barrier=true;
	char * host_char;
	int len=strlen(comm.hostnames[0]);
	int *msg;
	msg=(int *)malloc(sizeof(int));
	*msg=9999;
	host_char=(char *)malloc(len*sizeof(char));
        if(comm.rank!=0)
        {
		strcpy(host_char,comm.hostnames[0]);
                client(host_char/*"c45"*/,30014+comm.rank,MPI_COMM_WORLD,sizeof(int),msg);
        }
	if(comm.rank==0)
	{
		int i;
		for(i=1;i<comm.numproc;i++)
		{
			printf("checking barrier %d\n",buff_glob_0[i]);
			while(buff_glob_0[i]!=i);
			printf("--> post checking barrier %d\n",buff_glob_0[i]);	
		}
		
		for(i=1;i<comm.numproc;i++)
		{
			strcpy(host_char,comm.hostnames[i]);
			printf("sending ack %s \n",host_char);
			client(host_char,30014,MPI_COMM_WORLD,sizeof(int),msg);

		}
	}
	else
	{

		printf("%d recieved buffer_client\n",buff_glob_0[0]);
		while(buff_glob_0[0]!=0);	
		printf("-->post sleep %d recieved buffer_client\n",buff_glob_0[0]);

	}
	int i;
	for(i=0;i<MPI_COMM_WORLD.numproc;i++)
	{
		buff_glob_0[i]=-1;
	}
	free(host_char);
	free(msg);
	barrier=false;
	return 1;


}
int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
{
	char * host_char;
	
	printf("hostname %d is %s\n",dest,comm.hostnames[dest]);
	printf("!!!!!1 addr to mactch is %p\n",comm.hostnames[0]);
	host_char=(char *)malloc(strlen(comm.hostnames[dest])*sizeof(char));
	strcpy(host_char,comm.hostnames[dest]);
	
	printf("%s host_char\n",host_char);
	client(host_char,30014,comm,count,buf);
	free(host_char);
	return 1;


}

int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status)
{
	rcv_flag=true;
	rcv_ptr=&buf;
	rcv_cnt=count;
//	rcv_flag=false;
	return 1;

}

int MPI_Comm_size(MPI_Comm comm, int *size)
{
	*size=comm.numproc;
	return 1;
}

int MPI_Comm_rank(MPI_Comm comm,int *rank)
{
	*rank=comm.rank;
	return 1;
}

int MPI_Get_processor_name(char* hostname,int *len)
{
	char line[1024];
	FILE *cpuinfo = fopen("/proc/cpuinfo","rb");
	while(fgets(line,1024,cpuinfo) != NULL)
	{
	if(strstr(line,"model name") != NULL)
		{
			strcpy(hostname,line);
			*len=strlen(line);
			break;	
		}
	}
	return 1;

}
int MPI_Finalize(void)
{

	MPI_Barrier(MPI_COMM_WORLD);
	return 1;
}
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
	bufferSend=(int *)malloc(32*sizeof(int));
	bufferRecv=(int *)malloc(32*sizeof(int));
	*bufferSend=32;
	MPI_Send(bufferSend,32*sizeof(int),MPI_INT,pair_send,123,MPI_COMM_WORLD);
	printf("data sent---------------------\n");
	MPI_Recv(bufferRecv,32*sizeof(int),MPI_INT,pair_rcv,123,MPI_COMM_WORLD,&status);
	printf("data recvd---------------\n");
	 MPI_Finalize();
	return 0;



}
