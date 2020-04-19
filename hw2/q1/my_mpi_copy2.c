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
int *buff_glob;

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
	char line[128][10];
	char c;
//	int rank_ret[2];
	printf("in func %p\n",hostName);
    if (fp == NULL){
        printf("Could not open file %s",fileName);
        //return 1;
    }
	int i=0;
    while (fgets(line[i], 128, fp))
	{

		line[i][strlen(line[i])-1]='\0';

		
        	if(strcmp(rank,line[i])==0)
		{
			printf("%s and ran is %d\n",line[i],i);
			rank_ret[0]=i;
		}
		//if(i==0)
		//{
//			printf("line[0][0] has %p %s\n",&line[0][0],line[0]);
			hostName[i]=&line[i];
			printf("%s\n",hostName[i]);
		//}

		i++;
	}
	printf("numproc=%d\n",i);
	rank_ret[1]=i;
    fclose(fp);
//	printf("%p\n",rank_ret);
 //   return rank_ret;
}

void client(char* hostname,int portno,MPI_Comm comm)
{
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    /*if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }*/
    //portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
//    printf("hsot is %s\n",hostname);
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
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    printf("Please enter the message: ");
    bzero(buffer,256);
    //fgets(buffer,255,stdin);
    //strcpy(buffer,"hi");
    //int a[]={1,2,3,4,5};
    int *buff;
    buff=(int *)malloc(sizeof(int)*(524288+comm.numproc));
    buff[comm.rank]=comm.rank;
    n = write(sockfd,buff,(2097152+(sizeof(int)*comm.numproc)));
    if (n < 0) 
         error("ERROR writing to socket");
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0) 
         error("ERROR reading from socket");
    printf("%s\n",buffer);
    close(sockfd);
	//return 1;


}

void server_1(arg * args )
{

 int sockfd, newsockfd;
     socklen_t clilen;
     char buffer[256];
     struct sockaddr_in serv_addr, cli_addr;
     int n;
	int portno=args->portno;
	MPI_Comm comm=args->comm;
	//printf("printing thread id=%d\n",pthread_self());
	printf("%d port num\n",portno);
     /*if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }*/
	printf("here\n");
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     //portno = 30014;
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR on binding");
	printf("ok till here\n");
	    listen(sockfd,5);
	while(flag==false)

	{ 

     		printf("checking \n");

		clilen = sizeof(cli_addr);
	//flag2=true;
   	  newsockfd = accept(sockfd, 
                 (struct sockaddr *) &cli_addr, 
                 &clilen);
		printf("inside server while\n");
	//flag2=true;
	     if (newsockfd < 0) 
        	  error("ERROR on accept");
     	bzero(buffer,256);

	buff_glob=(int *)malloc(sizeof(int)*(524288+comm.numproc));
	//n = read(newsockfd,buffer,255);
	n = read(newsockfd,buff_glob,2097152);
     	if (n < 0) error("ERROR reading from socket");
     	//printf("Here is the message: %s\n",buffer);
     	n = write(newsockfd,"I got your message\n",18);
     	if (n < 0) error("ERROR writing to socket");
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
        host_char=(char **)malloc(sizeof(char *)*10);
   	hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    	printf("Hostname: %s\n", hostbuffer);
    	getNumProcRank(hostbuffer,num_rank,host_char);
        rank=num_rank[0];
        numproc=num_rank[1];
	MPI_COMM_WORLD.rank=rank;
	MPI_COMM_WORLD.numproc=numproc;
	MPI_COMM_WORLD.hostnames=(char **) malloc(sizeof(char *)*(numproc));
	//MPI_COMM_WORLD.root_host=(char *)malloc(sizeof(char))
	int i;
        for(i=0;i<numproc;i++)
	{
		MPI_COMM_WORLD.hostnames[i]=host_char[i];
		printf("hostnames are %s\n",MPI_COMM_WORLD.hostnames[i]);
		
	}
	printf("numproc is %d, and rank is %d\n",numproc,rank);
        if(rank==0)
		sleep(2);
	pthread_t *threads;
	threads=(pthread_t *)malloc(sizeof(pthread_t)*numproc);
	int rc;
   	int t;
	/* check threads */

	//struct topass *arg;
	
		arg *args;
	if(rank==0)
	{
		for(t=0; t<numproc-1; t++){
			

		args=(arg *)malloc(sizeof(arg));
		args->comm=MPI_COMM_WORLD;
			args->portno=30014+t;
			printf("thread %d,%d\n",t,args->portno);
			rc = pthread_create(&threads[t], NULL, server_1,args);
			 if (rc){
                printf("ERROR; return code from pthread_create() is %d\n", rc);
                exit(-1);
        		}
		}

	}
	else
	{
    	//for(t=0; t<numproc; t++){
	printf("In main: creating thread %ld\n", 0);
	

	args=(arg *)malloc(sizeof(arg));
	args->comm=MPI_COMM_WORLD;
	args->portno=30014+rank-1;
	rc = pthread_create(&threads[0], NULL, server_1,args);
	if (rc){
        	printf("ERROR; return code from pthread_create() is %d\n", rc);
         	exit(-1);
      	}
	}
	//}
	//server_1();
	//while(flag2!=true);
	/*sleep(2);
	if(rank!=0)
	{
		client(host_char,30014+rank-1);
	}*/
	MPI_Barrier(MPI_COMM_WORLD);
//	pthread_cancel(&threads[0]);
//	(void) pthread_join(&threads[0],NULL);
 return 1;
}

int MPI_Barrier( MPI_Comm comm)
{
	sleep(2);
	char * host_char;
	int len=strlen(comm.hostnames[0]);
	host_char=(char *)malloc(len*sizeof(char));
        if(comm.rank!=0)
        {
	//	printf("client 0 is %s\n",comm.hostnames[0]);
		strcpy(host_char,comm.hostnames[0]);
		//printf("host name is %s, %d\n",host_char);
		//comm.hostnames[0][strlen(comm.hostnames[0])-1]='\0';
                client(host_char/*"c45"*/,30014+comm.rank-1,MPI_COMM_WORLD);
        }
	if(comm.rank==0)
	{
		int i;
		for(i=1;i<comm.numproc;i++)
		{
			printf("checking barrier %d\n",buff_glob[i]);
			//while(buff_glob[i]!=i);	
		}
	}
	
	return 1;


}

int main(int argc,char *argv[])
{
    /*char hostbuffer[256]; 
    int num_rank[2];
      int numproc;
	int rank;
	char* hostname;
	char** host_char;
	host_char=(char **)malloc(sizeof(char *)*10);
    hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    printf("Hostname: %s\n", hostbuffer);
    getNumProcRank("c45",num_rank,host_char);
	//printf("%p\n",num_rank);	
	rank=num_rank[0];
	numproc=num_rank[1];
	printf("numproc is %d, and rank is %d\n",numproc,rank);
	int i;
	for(i=0;i<numproc;i++)
		printf("%s,",host_char[i]);
	char* portno="30014";*/
	//server();
	MPI_Init(&argc,&argv);
	return 0;



}
