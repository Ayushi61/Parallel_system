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
		if(i==0)
		{
//			printf("line[0][0] has %p %s\n",&line[0][0],line[0]);
			strcpy(*hostName,line[0]);
//			printf("%s\n",*hostName);
		}

		i++;
	}
	printf("numproc=%d\n",i);
	rank_ret[1]=i;
    fclose(fp);
//	printf("%p\n",rank_ret);
 //   return rank_ret;
}
int MPI_Init(int *argc, char **argv[])
{
	char* hostname;
	char hostbuffer[256];
    int num_rank[2];
      int numproc;
        int rank;
        char* host_char;
        host_char=(char *)malloc(sizeof(char)*10);
    hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    printf("Hostname: %s\n", hostbuffer);
    getNumProcRank(hostbuffer,num_rank,&host_char);
         rank=num_rank[0];
         numproc=num_rank[1];
         printf("numproc is %d, and rank is %d, and rank 0 is host %s\n",numproc,rank,host_char);
        
	pthread_t threads[1];
	int rc;
   	long t;
	/* check threads */
    	//for(t=0; t<numproc; t++){
		printf("In main: creating thread %ld\n", 0);
	       rc = pthread_create(&threads[0], NULL, server_1,NULL);
		if (rc){
        	  printf("ERROR; return code from pthread_create() is %d\n", rc);
         	 exit(-1);
      		 }
	//}
	//server_1();
	//while(flag2!=true);
	sleep(2);
	if(rank!=0)
	{
		int sockfd, portno, n;
   		struct sockaddr_in serv_addr;
		struct hostent *server2;
		char buffer[256];
		portno=30014;
		sockfd = socket(AF_INET, SOCK_STREAM, 0);

    		if (sockfd < 0) 
        		error("ERROR opening socket");
	//strcpy(server2,host_char);
		//strcpy(host_char,hostbuffer);
    		printf("%s\n",host_char);
    		server2=gethostbyname(&host_char);
	 
	//server2=gethostbyname(hostbuffer);
    		if (server2 == NULL) {
    			fprintf(stderr,"ERROR, no such host\n");
    			exit(0);
    		}


    		bzero((char *) &serv_addr, sizeof(serv_addr));
    		serv_addr.sin_family = AF_INET;
    		bcopy((char *)server2->h_addr,(char *)&serv_addr.sin_addr.s_addr,server2->h_length);
    		printf("iiiiin here\n");
    		serv_addr.sin_port = htons(portno);

	

    		if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        		error("ERROR connecting");
		bzero(buffer,256);
	//buffer=hostname;
		strcpy(buffer,hostname);
		n = write(sockfd,buffer,strlen(buffer));
		if (n < 0) 
         		error("ERROR writing to socket");
    		bzero(buffer,256);
	
		printf("in mpi_init\n");
    		n = read(sockfd,buffer,255);
		printf("acknowledged by %s\n",buffer);
		close(sockfd);
	}
 return 1;
}

void server_1()
{

 int sockfd, newsockfd, portno;
     socklen_t clilen;
     char buffer[256];
     struct sockaddr_in serv_addr, cli_addr;
     int n;
     /*if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }*/
	printf("here\n");
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = 30014;
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
	printf("ok till here\n");
	//while(flag==false)
//	{ 
    listen(sockfd,5);
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
     n = read(newsockfd,buffer,255);
     if (n < 0) error("ERROR reading from socket");
     printf("Here is the message: %s\n",buffer);
     n = write(newsockfd,"I got your message",18);
     if (n < 0) error("ERROR writing to socket");
//	}
     close(newsockfd);
     close(sockfd);
	pthread_exit(NULL);
     
}

int main(int argc,char *argv[])
{
    /*char hostbuffer[256]; 
    int num_rank[2];
      int numproc;
	int rank;
	char* host_char;
	host_char=(char *)malloc(sizeof(char)*10);
    hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    printf("Hostname: %s\n", hostbuffer);
    getNumProcRank("c40",num_rank,&host_char);
	//printf("%p\n",num_rank);	
	rank=num_rank[0];
	numproc=num_rank[1];
	printf("numproc is %d, and rank is %d, and rank 0 is host %s\n",numproc,rank,host_char);
	char* portno="30014";*/
	//server();
	MPI_Init(&argc,&argv);
	return 0;



}
