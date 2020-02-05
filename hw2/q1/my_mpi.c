
/*
 Single Author info:
 arajend4 Ayushi Rajendra Kumar 

*/



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

/*setting global variables for use throughout the functions*/
bool flag=false;
bool flag2=false;
void server_1();
int MPI_Barrier( MPI_Comm);
int *buff_glob_0;
int newsockfd_glob=-1;
bool barrier=false;
bool rcv_flag=false;
int *rcv_ptr;
int rcv_cnt;
int cnt_rcv;
MPI_Comm MPI_COMM_WORLD;
char * buffer_client;
/*structure to pass to server thread */
typedef	struct topass{
		int portno;
		MPI_Comm comm;
	}arg;
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

/*this function stores in the pointer the respective ranks, list of hostnames and numproc by reading the nodefile*/
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

/*client or sender code, this code makes tcp connect, and sends messages */ 
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

/*server thread, spawned by init for all the ranks, for rank 0 nproc number of server threads on different processes. on all other ranks, 1 port -30014 is spawned,. Rank 0 listens to nproc other processes for barrier. therefore nproc number of ports from 30014 to 30014+numproc */
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
			while(barrier==false);
  			if(barrier==true)
			{           
     				bzero(buffer,256);
				int * buff_glob;
				buff_glob=(int *)malloc(100*sizeof(int));
				n = read(newsockfd,buff_glob,(100*sizeof(int)));
				
				printf("rcvd %d bytes \n",n);
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
		}	
		else
		{
	 		newsockfd = accept(sockfd,
         	        (struct sockaddr *) &cli_addr, &clilen);
                	printf("inside server while\n");
             		if (newsockfd < 0)
                  		error("ERROR on accept");
			while(barrier==false && rcv_flag==false);
			if(barrier==true)
			{
        			bzero(buffer,256);
        			int * buff_glob;
     		   		buff_glob=(int *)malloc(100*sizeof(int));
        			n = read(newsockfd,buff_glob,(100*sizeof(int)));
				printf("rcvd %d bytes \n",n);
				printf("#####################is #################%d\n",buff_glob[0]);
				buff_glob_0[0]=buff_glob[0];
	        		printf("server got---> %d\n",buff_glob[1]);
        			if (n < 0) error("ERROR reading from socket");
        			free(buff_glob);
			}
			else
			{

				/*int itr=0;
				int ii=rcv_cnt;
				while(ii>262144)
				{
					n = read(newsockfd,*rcv_ptr+itr*65536,262144);
					newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
					printf("2nd-----msg\n");
					itr++;
					ii-=262144;
				}*/
				bzero(rcv_ptr,rcv_cnt);
				printf(" void size %d\n",sizeof(void));
				void *buffer_rcv=(void *)malloc(rcv_cnt);
				//int * bff=(int *)malloc(rcv_cnt/sizeof(int));
				bzero(buffer_rcv,rcv_cnt);
				n=read(newsockfd,buffer_rcv,rcv_cnt);
				//bcopy(buffer_rcv,bff,n);
				//memcpy(bff,(int *)buffer_rcv,n/sizeof(int));
				//printf("%d is %d rcv\n",*(bff+127),n/sizeof(int));
				//printf()		
				while(n<0)
				{

					n=read(newsockfd,buffer_rcv,rcv_cnt);
				}
						
				int n1=n;
				buffer_rcv+=n;
                                while(n1<rcv_cnt)
				{
					n=read(newsockfd,buffer_rcv,rcv_cnt);
					printf("rcvd %d bytes and %d \n",n1,n);
					while(n<0)
                                {

                                        n=read(newsockfd,buffer_rcv,rcv_cnt);
                                }

					n1+=n;
					//printf("ptr is %p\n",rcv_ptr);
					//bcopy(buffer_rcv,bff,n);
					buffer_rcv+=n;
					
				}
				memcpy(rcv_ptr,(int *)(buffer_rcv-n1),rcv_cnt/sizeof(int));
				printf("rcvd %d bytes \n",n1);
				printf("i recieved %d\n",*(rcv_ptr+127));
				rcv_flag=false;
				free(buffer_rcv-n1);
				//free(bff);


			}
		}
		
	}
     	close(newsockfd);
     	close(sockfd);
	pthread_exit(NULL);
     
}
/*this func initializes all the nodes ad calls barrier internally to let all functions start the server thread*/
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
		sleep(5);
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
/*blocking and syncying call within inti and finalize to let all functions finish there server threads spawinging, comminicates to rank 0 and gets acknowledged to move ahead */
int MPI_Barrier( MPI_Comm comm)
{
	sleep(1);
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
		//cnt_rcv=0;
		for(i=1;i<comm.numproc;i++)
		{
			printf("checking barrier %d\n",buff_glob_0[i]);
			while(buff_glob_0[i]!=i);
			printf("--> post checking barrier %d\n",buff_glob_0[i]);	
			//cnt_rcv++;
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
	barrier=false;
	free(host_char);
	free(msg);

	sleep(1);
	return 1;


}
/*used to send message to all other nodes , uses client call interally*/
int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
{
	if(dest!=comm.rank)
	{	
	char * host_char;
	
	host_char=(char *)malloc(strlen(comm.hostnames[dest])*sizeof(char));
	strcpy(host_char,comm.hostnames[dest]);
	int buffer=*((int *)buf+127);
	printf("%s host_char and mesg sent %d\n",host_char,buffer);
	int i=count;
	int itr=0;
	while(i>262144)
	{
		client(host_char,30014,comm,262144,buf+itr*(65536));
		//sleep(3);
		i-=262144;
		itr++;
	}
	client(host_char,30014,comm,i,buf+itr*(65536));
	free(host_char);
	}
	return 1;


}
/*checks flags that server sets. */
int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status)
{
	if(source!=comm.rank)
	{
	int i=count;
	int itr=0;
	while(i>262144)
	{
		rcv_ptr=buf+itr*65536;
		rcv_cnt=262144;
		rcv_flag=true;
		while(rcv_flag!=false);
		itr++;
		i-=262144;
	}
	rcv_ptr=buf+itr*65536;
	rcv_cnt=i;
	rcv_flag=true;
	while(rcv_flag!=false);
	}
	return 1;

}
/*returns number of nodes*/
int MPI_Comm_size(MPI_Comm comm, int *size)
{
	*size=comm.numproc;
	return 1;
}
/*return rank ofr the nodes*/
int MPI_Comm_rank(MPI_Comm comm,int *rank)
{
	*rank=comm.rank;
	return 1;
}
/*returns processor name*/
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
/*blocking call before all servers threads can be killed */
int MPI_Finalize(void)
{
	//sleep(5);
	MPI_Barrier(MPI_COMM_WORLD);
	return 1;
}


int MPI_Sendrecv(const void *sendbuff, int sendcount, MPI_Datatype sendtype,int dest, int sendtag,void *recvbuf, int recvcount, MPI_Datatype recvtype,int source, int recvtag,MPI_Comm comm, MPI_Status *status)
{

	MPI_Send(sendbuff,sendcount,sendtype,dest,sendtag,comm);
	MPI_Recv(recvbuf,recvcount,recvtype,source,recvtag,comm,status);

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
	bufferSend=(int *)malloc(1024*1024);
	bufferRecv=(int *)malloc(1024*1024);
	*(bufferSend+127)=32;
	MPI_Sendrecv(bufferSend, 1024*1024, MPI_INT, pair_send, 123, bufferRecv, 1024*1024, MPI_INT, pair_rcv, 123, MPI_COMM_WORLD, &status);
	//MPI_Send(bufferSend,1024*1024,MPI_INT,pair_send,123,MPI_COMM_WORLD);
	printf("data sent---------------------\n");
	//MPI_Recv(bufferRecv,1024*1024,MPI_INT,pair_rcv,123,MPI_COMM_WORLD,&status);
	printf("data recvd-------------%d\n",bufferRecv[127]);
	free(bufferSend);
	free(bufferRecv);
	 MPI_Finalize();
	return 0;



}
