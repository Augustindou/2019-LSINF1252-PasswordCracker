#include "reverse.h"
#include "reverse.c"
#include "sha256.c"
#include "sha256.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <semaphore.h>

//structure
struct node {
    struct node *next;
    char *name;
};

//functions
uint8_t* readBinFile(FILE* file, uint8_t * hash);
void * producer();
void * consumer();
void * sort();
int getSemValue(sem_t * sem);
void insertHash(uint8_t * hash, uint8_t *ProdCons, int N);
void removeHash(uint8_t* hash, uint8_t *ProdCons, int N);
void insertResRH(char* resRH, char * ProdCons2, int N);//combiner insertHash avec insertResRH en ajoutant une variable!
void removeResRH(char * resRH, char *ProdCons2, int N);
int push(struct node **head, const char *value);
int pop(struct node **head);
int printStack(struct node **head);
int strlenVo(char* candidat, bool consonant);

//variables
	FILE* file;
	int N;
	//prodCons1
	uint8_t * ProdCons;
	pthread_mutex_t mutex;
	sem_t empty; //tested function in test_Semaphore.c
	sem_t full;
	int finishProd;
	//ProdCons2
	char * ProdCons2;
	pthread_mutex_t mutex2;
	sem_t empty2; 
	sem_t full2;
	int finishProd2;

	struct node * head;



int main(int argc, char *argv[]){
		//définir les les variables avec argv[], comme dans v1.c
		printf("argc = %d \n", argc);
		for(int i=0; i<argc; i++){
		printf("argv[%d] = %s, ",i, argv[i]);}
		printf("\n");

		// case no argument
		if(argc==1){
			printf("the function needs at least one argument\n");
			return -1;
		}

		int index = 1;

		// case given number of threads
		if(argc>2 && !strcmp(argv[index], "-t")){
			int nbThread = atoi(argv[index+1]);
			if(!nbThread){
				printf("le nombre de thread est incorrect, veillez reessayer\n");
				return -1;
			}
			else{
				printf("le nombre de thread est de %d\n", nbThread);
				index=index+2; //car le "-t" et le nombre
			}
		}

		// case consonnes
		if(argc>index && !strcmp(argv[index], "-c")){
			index=index+1;
			printf("cas des consonnes\n");
		}

		// cas d'output dans un document
		// a ajouter la condition pour eviter de prendre comme fichier out en fichier in
		if(argc>index+1 && !strcmp(argv[index], "-o")){
			if(1){
				printf("la sortie du programme se fera dans le document %s\n",argv[index+1]);
				index=index+2;
			}
			else{
                		printf("le nom fichier de sortie est inexistant, veillez reessayer\n");
                		return -1;
        		}

		}
		printf("index = %d, argc = %d\n",index, argc);

	// Initialisation
	
		N=4; //à modifier selon le nombre de thread
		ProdCons = (uint8_t *) calloc(N, sizeof(uint8_t)*32);//create the table
		if(ProdCons==NULL){
			printf("calloc ProdCons fail\n");
			return -1;
		}
		ProdCons2 = (char *) calloc(N, sizeof(char)*17);//max 16 lettres long, mieux de definir une variables au debut
		if(ProdCons==NULL){
			printf("calloc ProdCons2 fail\n");
			return -1;
		}
		head=NULL;
	
		//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
		pthread_mutex_init(&mutex, NULL);
		sem_init(&empty, 0 , N);  // buffer vide
		sem_init(&full, 0 , 0);   // buffer vide
		finishProd = 0;	  // pas encore la fin du fichier

		pthread_mutex_init(&mutex2, NULL);
		sem_init(&empty2, 0 , N);  // buffer vide
		sem_init(&full2, 0 , 0);   // buffer vide
		finishProd2 = 0;	  // pas encore la fin du bruteforce

		//ouverture de fichier
		file = fopen(argv[index], "rb");
		if(!file){
			printf("reading fail\n");
			return -1;
		}
		//pas oublier de fermer le fichier en cas d'erreur

	//le vrai code

	pthread_t prod;
	pthread_t cons [N];//thread pour reverseHash
	pthread_t cons2;

	//creation des threads
	if(pthread_create(&prod, NULL, &producer, NULL)){
			printf("error while creating production thread\n");
			return -1;
	}
	for(int i=0; i<N; i++){
		if(pthread_create(&(cons[i]), NULL, &consumer, NULL)){
			printf("error while creating consumer threads\n");
			return -1;
		}
	}
	if(pthread_create(&cons2, NULL, &sort, NULL)){
			printf("error while creating production thread\n");
			return -1;
	}
	
	//join de threads
	if(pthread_join(prod,NULL)!=0){
		printf("error while prod pthread_join\n");
		return -1;
	}
	for(int i=N-1; i>=0; i--){
		if(pthread_join(cons[i],NULL)!=0){
			printf("error while cons[%d] pthread_join\n",i);
			return -1;
		}//check errors
		printf("fin de cons[%d]\n", i);
		if(i==0){
			printf("cons done before, finishProd2: %d", finishProd2);
			finishProd2=1;
			printf("cons done, finishProd2: %d", finishProd2);
		}
	}
	
	if(pthread_join(cons2,NULL)!=0){
		printf("error while cons2 pthread_join\n");
		return -1;
	}

	printStack(&head);
	pop(&head);

	//terminasion
		//maybe check if errors
		free(ProdCons);
		pthread_mutex_destroy(&mutex);
		sem_destroy(&empty);
		sem_destroy(&full);

		free(ProdCons2);
		pthread_mutex_destroy(&mutex2);
		sem_destroy(&empty2);
		sem_destroy(&full2);

	return 0;
}

//readFile for threads
uint8_t* readBinFile(FILE* file, uint8_t * hash){
	if(fread(hash, sizeof(uint8_t), 32, file)==32){
		return hash;
	}//read file 
	else{
		finishProd=1;
		printf("close file\n");
		if(fclose(file)){
			printf("error while closing\n");
			return NULL;
		}
	}//end of the file

	return hash;
}//return unint8_t* with hash

// Producteur, Hash
void * producer(){
	uint8_t * hash = malloc(sizeof(char)*32);
	if(!hash){
		free(hash);
		printf("malloc fail\n");
		return NULL;
	}

	while(!finishProd)
	{
		hash=readBinFile(file, hash);	//readf() 
		
		sem_wait(&empty); // attente d'un slot libre
		pthread_mutex_lock(&mutex);
			// section critique 1
			insertHash(hash, ProdCons, N);	//ajout dans le tableau la chaine de 32 byte
		pthread_mutex_unlock(&mutex);
		sem_post(&full); // il y a un slot rempli en plus
	}
	//printf("End producer, full: %d, empty: %d\n", getSemValue(&full),getSemValue(&empty));

	free(hash);
	return (NULL);
}

// Consommateur, reverseHash
void * consumer(){
	uint8_t * hash = malloc(sizeof(char)*32);
	if(!hash){
		free(hash);
		printf("malloc fail\n");
		return NULL;
	}
	char * resRH = malloc(sizeof(char)*16);//16 ou 17?, definir au debut!
	while(!finishProd || getSemValue(&full) )	//check si la production est terminee et vérifie si le tableau est vide 
	{
		sem_wait(&full); // attente d'un slot rempli
		pthread_mutex_lock(&mutex);
			// section critique 1
			removeHash(hash, ProdCons, N);
		pthread_mutex_unlock(&mutex);
		sem_post(&empty); // il y a un slot libre en plus

		if(!reversehash(hash, resRH, sizeof(char)*16)){//16 ou 17?, definir au debut!
			printf("petite erreur dans reverseHash!\n");
		}//le mot de passe ne respecte pas les consignes

		sem_wait(&empty2); // attente d'un slot libre
		pthread_mutex_lock(&mutex2);
			// section critique 2
			insertResRH(resRH, ProdCons2, N);	
		pthread_mutex_unlock(&mutex2);
		sem_post(&full2); // il y a un slot rempli en plus

	}

	//printf("End consumer, full: %d, empty: %d\n", getSemValue(&full),getSemValue(&empty));
	printf("End consumer, full2: %d, empty2: %d, finishProd2:%d\n", getSemValue(&full2),getSemValue(&empty2), finishProd2);
	
	free(hash);
	return NULL;
}

void * sort(){
	char * resRH = malloc(sizeof(char)*16);//16 ou 17?, definir au debut!
	while(!finishProd2 || getSemValue(&full2) )	//check si la production est terminee et vérifie si le tableau est vide 
	{
		if(finishProd2){printf("sort, finishProd2: %d\n", finishProd2);}
		//if(!getSemValue(&full2)){printf("sort, full2: %d\n", getSemValue(&full2));}
		sem_wait(&full2); // attente d'un slot rempli
		pthread_mutex_lock(&mutex2);
			// section critique 2
			removeResRH(resRH, ProdCons2, N);
		pthread_mutex_unlock(&mutex2);
		sem_post(&empty2); // il y a un slot libre en plus
		if(head==NULL){
			push(&head, resRH);
		}
		bool consonne=false;//a definir dans la premiere partie du code
		if(head==NULL){
			push(&head, resRH);
		}
		else if(strlenVo(head->name, consonne)<strlenVo(resRH, consonne)){
			pop(&head);
			push(&head, resRH);
		}
		else if(strlenVo(head->name, consonne)==strlenVo(resRH, consonne)){
			push(&head, resRH);
		}
		if(finishProd2){printf("sort, finishProd2: %d\n", finishProd2);}
	}

	printf("End sort, full2: %d, empty2: %d\n", getSemValue(&full2),getSemValue(&empty2));
	return NULL;
}

//permet d'obtenir la valuer du semaphore passer en argument
int getSemValue(sem_t * sem){
	int value;
	sem_getvalue(sem, &value);
	return value;
} // it works :D

//see test in test_InsertHash_RemoveHash
void insertHash(uint8_t * hash, uint8_t *ProdCons, int N){
	int counter;
	for(int i=0; i<N; i++){
		counter=0;
		for(int j=0; j<32; j++){
			counter = counter + *(ProdCons+i*32+j);
		}
		if (!counter){
			for(int j=0; j<32; j++){
				*(ProdCons+i*32+j)=*(hash+j);
			}
			return;
		}
	}
	return;
}

void removeHash(uint8_t* hash, uint8_t *ProdCons, int N){
	int counter=0;
	for(int i=0; i<N & !counter; i++){
		counter=0;
		for(int j=0; j<32; j++){
			counter = counter + *(ProdCons+i*32+j);
		}
		if (counter){
			for(int j=0; j<32; j++){
				*(hash+j)=*(ProdCons+i*32+j);
			    *(ProdCons+i*32+j)=0;
			}
		}
	}
	return;
}

//peut etre combiner avec le precedant avec une variables supplementaire, taille de chaque element
void insertResRH(char * resRH, char *ProdCons2, int N){
	int counter;
	int nbLettre=16;//16 ou 17?, definir au debut!
	for(int i=0; i<N; i++){
		counter=0;
		for(int j=0; j<1; j++){
			counter = counter + *(ProdCons2+i*nbLettre+j);
		}
		if (!counter){
			for(int j=0; j<nbLettre; j++){
				*(ProdCons2+i*nbLettre+j)=*(resRH+j);
			}
			return;
		}
	}
	return;
}

void removeResRH(char * resRH, char *ProdCons2, int N){
	int counter=0;
	int nbLettre=16;//16 ou 17?, definir au debut!
	for(int i=0; i<N & !counter; i++){
		counter=0;
		for(int j=0; j<nbLettre; j++){
			counter = counter + *(ProdCons2+i*nbLettre+j);
		}
		if (counter){
			for(int j=0; j<nbLettre; j++){
				*(resRH+j)=*(ProdCons2+i*nbLettre+j);
			    *(ProdCons2+i*nbLettre+j)=0;
			}
		}
	}
	return;
}

/**
* Add @name at the "top" of the stack.
*
* @head : pointer to the top of the stack
* @name : the string to be placed in the element at the top of the stack
*
* @return 0 if no error, 1 otherwise
*/

int push(struct node **head, const char *value){
	if(value==NULL){return 1;}
	char * varC = (char*)malloc(strlen(value)+1);
	if (varC==NULL){return 1;}
	varC = strcpy(varC, value);
	struct node* newNode = (struct node*) malloc(sizeof(struct node*)+sizeof(char*)) ;
	if(newNode==NULL){return 1;}
	newNode->next = *head;//work also for *head==NULL
	newNode->name = varC;
	*head=newNode;
	return 0;
}

/**
* Free all elements of the stack.
*
* @head : pointer to the top of the stack
*
* @return 0 if no error, 1 otherwise
*
* pre : 
* post : 
*/

int pop(struct node **head){
	while(*head!=NULL){
		struct node * first = (struct node*)malloc(sizeof(struct node*));
		if(first==NULL){return 1;};
		first=*head;

		*head=(first->next);
		free(first->name);
		free(first);
		if(first==NULL && first->name==NULL){
			return 1;
		}
	}
	return 0;
}

int printStack(struct node **head){
	struct node * first = *head;
	while(first != NULL){
		printf("%s\n",(first->name));
		first = (first->next);
	}
	return 0;
}

int strlenVo(char* candidat, bool consonant){
	int vowels=0;
	int len=strlen(candidat);
	for(int i=0; i<len; i++){
        if(candidat[i]=='a' || candidat[i]=='e' || candidat[i]=='i' ||
           candidat[i]=='o' || candidat[i]=='u' || candidat[i]=='y'){
            vowels++;}
	}
    if(consonant){return len-vowels;}
	return vowels;
}