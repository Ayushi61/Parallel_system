#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<dirent.h>
#include<math.h>
#include<mpi.h>
#include <stdbool.h>
#define MAX_WORDS_IN_CORPUS 32
#define MAX_FILEPATH_LENGTH 16
#define MAX_WORD_LENGTH 16
#define MAX_DOCUMENT_NAME_LENGTH 8
#define MAX_STRING_LENGTH 64

typedef char word_document_str[MAX_STRING_LENGTH];

typedef struct o {
	char word[32];
	char document[8];
	int wordCount;
	int docSize;
	int numDocs;
	int numDocsWithWord;
} obj;

typedef struct w {
	char word[32];
	int numDocsWithWord;
	int currDoc;
} u_w;

static int myCompare (const void * a, const void * b)
{
    return strcmp (a, b);
}

int main(int argc , char *argv[]){
	DIR* files;
	struct dirent* file;
	int i,j;
	int numDocs = 0, docSize, contains;
	char filename[MAX_FILEPATH_LENGTH], word[MAX_WORD_LENGTH], document[MAX_DOCUMENT_NAME_LENGTH];
	
	// Will hold all TFICF objects for all documents
	obj TFICF[MAX_WORDS_IN_CORPUS];
	int TF_idx = 0;
	
	// Will hold all unique words in the corpus and the number of documents with that word
	u_w unique_words[MAX_WORDS_IN_CORPUS];
	int uw_idx = 0;
	/*MPI_Datatype TF_Type;
	MPI_Datatype type1[6]={MPI_CHAR,MPI_CHAR,MPI_INT,MPI_INT,MPI_INT,MPI_INT};
	int blocklen1[6]={32,8,1,1,1,1};
	MPI_Aint disp1[6];
	disp1[0]=&TFICF[0].word-&TFICF[0]
	disp1[1]=&TFICF[0].document-&TFICF[0]
	disp1[2]=&TFICF[0].wordCount-&TFICF[0]
	disp1[3]=&TFICF[0].docSize-&TFICF[0]
	disp1[4]=&TFICF[0].numDocs-&TFICF[0]
	disp1[5]=&TFICF[0].numDocsWithWord-&TFICF[0]
	MPI_Type_create_struct(6, blocklen1, disp1, type1, &TF_Type);
    	MPI_Type_commit(&TF_Type);
	*/
	
	//mpi init
	int numproc, rank;
    	MPI_Init(&argc, &argv);
    	MPI_Comm_size(MPI_COMM_WORLD, &numproc);
    	MPI_Comm_rank(MPI_COMM_WORLD, &rank);	
	MPI_Status status;	
	
	MPI_Datatype unique;
	MPI_Datatype type2[3]={MPI_CHAR,MPI_INT,MPI_INT};
	int blocklen2[3]={32,1,1};
	MPI_Aint disp2[3]={0,sizeof(char)*32,sizeof(char)*32+sizeof(int)};
	MPI_Type_create_struct(3, blocklen2, disp2, type2, &unique);
	MPI_Type_commit(&unique);	
	// Will hold the final strings that will be printed out
	word_document_str strings[MAX_WORDS_IN_CORPUS];
	

	MPI_Datatype str;
	MPI_Datatype type[1]={MPI_CHAR};
	int blocklen[1]={MAX_STRING_LENGTH};
	MPI_Aint disp[1]={0};
	MPI_Type_create_struct(1, blocklen, disp, type, &str);
    	MPI_Type_commit(&str);
	//Count numDocs
	int *numDoc_r;
	//numDoc=(int *)malloc(sizeof(int)* numproc);
	numDoc_r=(int *)malloc(sizeof(int));
	if(rank==0)
	{
		if((files = opendir("input")) == NULL){
			printf("Directory failed to open\n");
			exit(1);
		}
		while((file = readdir(files))!= NULL){
		// On linux/Unix we don't want current and parent directories
			if(!strcmp(file->d_name, "."))	 continue;
			if(!strcmp(file->d_name, "..")) continue;
			numDocs++;
		}
	//	int i_in;
		numDoc_r[0]=numDocs;
	/*	int dist=(int)(numDocs/(numproc-1));
		int rem=numDocs%(numproc-1);
		for(i_in=1;i_in<numproc;i_in++)
		{
			numDoc[i_in]=dist;
		}
		if(rem!=0)
		{
			for(i_in=1;i_in<rem;i++)
				 numDoc[i_in]+=1
		}*/
	}
	
	
	MPI_Bcast(numDoc_r,1,MPI_INT,0,MPI_COMM_WORLD);
	
	// Loop through each document and gather TFICF variables for each word
	if(rank!=0)
	{
		numDocs=numDoc_r[0];
		int dist=(int)(numDocs/(numproc-1));
                int rem=numDocs%(numproc-1);
		int start,end,i;
		if(rank<=rem && rem!=0)
		{
			dist+=1;
		}
		for(i=rank;i<=numDocs;i+=numproc-1)
		{
			//printf("rank %d and docNum=%d\n",rank,i);
			sprintf(document, "doc%d", i);	
			sprintf(filename,"input/%s",document);
			FILE* fp = fopen(filename, "r");
			if(fp == NULL){
				printf("Error Opening File: %s\n", filename);
                        	exit(0);	
			}
			docSize = 0;
	                while((fscanf(fp,"%s",word))!= EOF)
        	                docSize++;
			fseek(fp, 0, SEEK_SET);	
			while((fscanf(fp,"%s",word))!= EOF){
				contains = 0;	
				// If TFICF array already contains the word@document, just increment wordCount and break
				for(j=0; j<TF_idx; j++) {
	                                if(!strcmp(TFICF[j].word, word) && !strcmp(TFICF[j].document, document)){
        	                                contains = 1;
                	                        TFICF[j].wordCount++;
                 	       	                break;
                        	        }
				}
				
				//If TFICF array does not contain it, make a new one with wordCount=1
				if(!contains) {
					strcpy(TFICF[TF_idx].word, word);
					strcpy(TFICF[TF_idx].document, document);
					TFICF[TF_idx].wordCount = 1;
					TFICF[TF_idx].docSize = docSize;
					TFICF[TF_idx].numDocs = numDocs;
					TF_idx++;
				}
			
				contains = 0;
				// If unique_words array already contains the word, just increment numDocsWithWord
				for(j=0; j<uw_idx; j++) {
				
					if(!strcmp(unique_words[j].word, word)){
						contains = 1;
						if(unique_words[j].currDoc != i) {
							unique_words[j].numDocsWithWord++;
							unique_words[j].currDoc = i;
						}
						break;
					}
				}
			
				// If unique_words array does not contain it, make a new one with numDocsWithWord=1 
				if(!contains) {
					strcpy(unique_words[uw_idx].word, word);
					unique_words[uw_idx].numDocsWithWord = 1;
					unique_words[uw_idx].currDoc = i;
					uw_idx++;
				}
			}
			fclose(fp);
	
	
		}
		int *idx=(int*)malloc(sizeof(int));
		//idx[0]=TF_idx;
		idx[0]=uw_idx;
		MPI_Send(idx,1,MPI_INT,0,rank,MPI_COMM_WORLD);
		//MPI_Send(TFICF,TF_idx,TF_Type,0,rank,MPI_COMM_WORLD);
		MPI_Send(unique_words,uw_idx,unique,0,rank,MPI_COMM_WORLD);
	}	
	int *unique_count=(int *)malloc(sizeof(int));
	if(rank==0)
	{
		int *idx=(int*)malloc((numproc-1)*sizeof(int));

		int i,tot_u=0,tot_tf=0;
		for(i=1;i<numproc;i++)
		{
			MPI_Recv(idx+(i-1),1,MPI_INT,i,i,MPI_COMM_WORLD,&status);
			tot_u=tot_u+idx[(i-1)];
		}
		u_w *unique_words_r=(u_w *)malloc(sizeof(u_w)*tot_u);
	//	obj *TF_r=(obj *)malloc(sizeof(obj)*tot_tf);
		//MPI_Recv(unique_words_r,idx[0],unique,1,1,MPI_COMM_WORLD,&status);
		int disp=0;
		for(i=1;i<numproc;i++)
                {
			MPI_Recv(unique_words_r+disp,idx[i-1],unique,i,i,MPI_COMM_WORLD,&status);
			//printf("rcved from----rank %d\n" , i);
			int k;
		//	for(k=0;k<idx[i-1];k++)
		//	{
		//		printf("value = %s\n",unique_words_r[disp+k].word);
		//	}
			
			disp+=idx[i-1];
		}
		for(i=0;i<idx[0];i++)
		{
			strcpy(unique_words[i].word,unique_words_r[i].word);
			unique_words[i].numDocsWithWord=unique_words_r[i].numDocsWithWord;
                        unique_words[i].currDoc=unique_words_r[i].currDoc;
			//printf("rank 1 unique words %s\n",unique_words[i].word);
		}
		int end=idx[0],k;
		bool flag;	
		for(i=idx[0];i<tot_u;i++)
		{
			flag=false;
			//printf("rcved here-====== %s\n",unique_words_r[i].word);
			for(k=0;k<end;k++)
			{
				if(!strcmp(unique_words_r[i].word, unique_words[k].word)){
                                             unique_words[k].numDocsWithWord+=unique_words_r[i].numDocsWithWord;
                                             //unique_words[k].currDoc = i;
                                             flag=true;   
                                             break;
                                        }
			}
			if(flag==false)
			{	
				strcpy(unique_words[end].word,unique_words_r[i].word);
				unique_words[end].numDocsWithWord=unique_words_r[i].numDocsWithWord;
				unique_words[end].currDoc=unique_words_r[i].currDoc;
				end++;
			}
				
		}
		//for(i=0;i<end;i++)
			
			//printf("unique Words is rank 0 %s, numdocswith word %d\n", unique_words[i].word,unique_words[i].numDocsWithWord);
		
		unique_count[0]=end;
	}	
	MPI_Bcast(unique_count,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(unique_words,*unique_count,unique,0,MPI_COMM_WORLD);
	if(rank!=0)
	{
		
	
	// Print TF job similar to HW4/HW5 (For debugging purposes)
		printf("-------------TF Job rank %d-------------\n",rank);
		for(j=0; j<TF_idx; j++)
			printf("%s@%s\t%d/%d\n", TFICF[j].word, TFICF[j].document, TFICF[j].wordCount, TFICF[j].docSize);
		
		// Use unique_words array to populate TFICF objects with: numDocsWithWord
		for(i=0; i<TF_idx; i++) {
			for(j=0; j<unique_count[0]; j++) {
			//	printf("unique Words is rank 0 %s, numdocswith word %d\n", unique_words[j].word,unique_words[j].numDocsWithWord);
				if(!strcmp(TFICF[i].word, unique_words[j].word)) {
					TFICF[i].numDocsWithWord = unique_words[j].numDocsWithWord;	
					break;
				}
			}
		}
	
	// Print ICF job similar to HW4/HW5 (For debugging purposes)
		printf("------------ICF Job rank %d-------------\n",rank);
		for(j=0; j<TF_idx; j++)
			printf("%s@%s\t%d/%d\n", TFICF[j].word, TFICF[j].document, TFICF[j].numDocs, TFICF[j].numDocsWithWord);
		
		// Calculates TFICF value and puts: "document@word\tTFICF" into strings array
		for(j=0; j<TF_idx; j++) {
			double TF = log( 1.0 * TFICF[j].wordCount / TFICF[j].docSize + 1 );
			double ICF = log(1.0 * (TFICF[j].numDocs + 1) / (TFICF[j].numDocsWithWord + 1) );
			double TFICF_value = TF * ICF;
			sprintf(strings[j], "%s@%s\t%.16f", TFICF[j].document, TFICF[j].word, TFICF_value);
		}
		MPI_Send(&TF_idx,1,MPI_INT,0,rank,MPI_COMM_WORLD);
		MPI_Send(strings,TF_idx,str,0,rank,MPI_COMM_WORLD);
	}
	if(rank==0)
	{
		int i;
		int dis=0;
		int tot=0;
		int *TF_idx_r=(int *)malloc((numproc-1)*sizeof(int));
		for(i=1;i<numproc;i++)
		{
			MPI_Recv(TF_idx_r+(i-1),1,MPI_INT,i,i,MPI_COMM_WORLD,&status);
			tot+=TF_idx_r[i-1];	
		}
		for(i=1;i<numproc;i++)
		{
			MPI_Recv(strings+dis,TF_idx_r[i-1],str,i,i,MPI_COMM_WORLD,&status);
				
			dis+=TF_idx_r[i-1];
		}
		// Sort strings and print to file
		qsort(strings, tot, sizeof(char)*MAX_STRING_LENGTH, myCompare);
		FILE* fp = fopen("output.txt", "w");
		if(fp == NULL){
			printf("Error Opening File: output.txt\n");
			exit(0);
		}
		for(i=0; i<tot; i++)
			fprintf(fp, "%s\n", strings[i]);
		fclose(fp);
	}
	MPI_Finalize();	
	return 0;	
}
