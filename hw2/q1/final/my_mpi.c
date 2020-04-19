
/*
 *  Single Author info:
 *   arajend4 Ayushi Rajendra Kumar
 *
 *   */



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
int *rcv_flag;
int *rcv_ptr;
int rcv_cnt;
int cnt_rcv;
extern  MPI_Comm MPI_COMM_WORLD;
char * buffer_client;
/*structure to pass to server thread */
typedef struct topass{
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
        if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
		error("ERROR connecting");
	printf("message is %d to %s\n",msg[0],hostname);
	n = write(sockfd,msg,bytes);
		
	close(sockfd);


}


void server_1(arg * args )
{
        int sockfd, newsockfd,pid;
        socklen_t clilen;
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
	//while(flag==false)
        //{

		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd,
                (struct sockaddr *) &cli_addr, &clilen);
                printf("#######inside server while\n");
                if (newsockfd < 0)
 	               error("ERROR on accept");
		while(rcv_flag==0);
		printf("test thread \n");
		//exit(0);
		bzero(rcv_ptr,rcv_cnt);
                printf(" void size %d\n",sizeof(void));
                void *buffer_rcv=(void *)malloc(rcv_cnt);
		bzero(buffer_rcv,rcv_cnt);
                n=read(newsockfd,buffer_rcv,rcv_cnt);
		int n1=n;
                buffer_rcv+=n;
                while(n1<rcv_cnt)
                {
                 	n=read(newsockfd,buffer_rcv,rcv_cnt);
                        printf("rcvd %d bytes and %d \n",n1,n);
			n1+=n;
			buffer_rcv+=n;
		}
		memcpy(rcv_ptr,(int *)(buffer_rcv),rcv_cnt/sizeof(int));	
		printf("rcvd %d bytes \n",n1);
                //printf("i recieved %d\n",*(rcv_ptr+127));
                *rcv_flag=1;
                free(buffer_rcv-n1);
	//}
	 close(newsockfd);
        close(sockfd);
        pthread_exit(NULL);

	//}
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
	rcv_flag=(int *)malloc(sizeof(int));
	*rcv_flag=0;
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
	sleep(10);
        //MPI_Barrier(MPI_COMM_WORLD);
	return 1;
}

 int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
{
	 char * host_char;

        host_char=(char *)malloc(strlen(comm.hostnames[dest])*sizeof(char));
        strcpy(host_char,comm.hostnames[dest]);
        int buffer=*((int *)buf+127);
        printf("%s host_char and mesg sent %d\n",host_char,buffer);
	client(host_char,30014,comm,count,buf);
	free(host_char);
	return 1;


}



int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status)
{
	printf("in rcv\n");
	//exit(0);	
	rcv_ptr=buf;
	rcv_cnt=count;
	*rcv_flag=1;
	//sleep(10);
	//exit(0);
	while(*rcv_flag!=0);
	sleep(10);
	exit(0);
	printf("flag is set to\n");
	return 1;


}
