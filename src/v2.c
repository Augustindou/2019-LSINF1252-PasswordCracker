#include "reverse.h"
#include "reverse.c"
#include "sha256.c"﻿
#include "sha256.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <semaphore.h>

//functions
uint8_t* readBinFile(FILE* file, uint8_t * hash);
void * producer();
void * consumer();
int getSemValue(sem_t * sem);
void insertHash(uint8_t * hash, uint8_t *ProdCons, int N);
void removeHash(uint8_t* hash, uint8_t *ProdCons, int N);

//variables
uint8_t * ProdCons;
pthread_mutex_t mutex;
sem_t empty; //tested function in test_Semaphore.c
sem_t full;
int N;
int finishProd;
FILE* file;

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
	
		N=10; //à modifier selon le nombre de thread
		ProdCons = (uint8_t *) calloc(N, sizeof(uint8_t)*32);//create the table
		if(ProdCons==NULL){
			printf("calloc ProdCons fail\n");
			return -1;
		}
	
		//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
		pthread_mutex_init(&mutex, NULL);
		sem_init(&empty, 0 , N);  // buffer vide
		sem_init(&full, 0 , 0);   // buffer vide
		finishProd = 0;	  // pas encore la fin du fichier

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
	
	//join de threads
	if(!pthread_join(prod,NULL)){
			printf("error while pthread_join\n");
			return -1;
	}
	for(int i=0; i<N; i++){
		if(!pthread_join((cons[i]),NULL)){
			printf("error while pthread_join\n");
			return -1;
		}//check errors
	}

	//terminasion
		free(ProdCons);
		pthread_mutex_destroy(&mutex);
		sem_destroy(&empty);
		sem_destroy(&full);

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
		if(!fclose(file)){
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
			// section critique
			insertHash(hash, ProdCons, N);	//ajout dans le tableau la chaine de 32 byte
		pthread_mutex_unlock(&mutex);
		sem_post(&full); // il y a un slot rempli en plus
	}
	printf("End producer, full: %d, empty: %d\n", getSemValue(&full),getSemValue(&empty));

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
	char * resRH = malloc(sizeof(char)*16);
	while(!finishProd || getSemValue(&full) )	//check si la production est terminee et vérifie si le tableau est vide 
	{
		sem_wait(&full); // attente d'un slot rempli
		pthread_mutex_lock(&mutex);
			// section critique
			removeHash(hash, ProdCons, N);
		pthread_mutex_unlock(&mutex);
		sem_post(&empty); // il y a un slot libre en plus

		if(reversehash(hash, resRH, sizeof(char)*16)){
			printf("%s\n", resRH);
		}//else, le mot de passe ne respecte pas les consignes
		//puis l'enregistrer dans le prochain
	}

	printf("End consumer, full: %d, empty: %d\n", getSemValue(&full),getSemValue(&empty));
	free(hash);
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
