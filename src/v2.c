#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "reverse.h"
#include "reverse.c"
#include "sha256.h"
#include "sha256.c"﻿

int main(int argc, char *argv[]){
	//définir les les variables avec argv[], comme dans v1.c

	// Initialisation
	
		N=10 //à modifier selon le nombre de thread
		unit8_t * ProdCons = (unit8_t *) calloc(N, sizeof(unit8_t)*32);//create the table
	
		pthread_mutex_t mutex;
		sem_t empty;
		sem_t full;
		//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
		pthread_mutex_init(&mutex, NULL);
		sem_init(&empty, 0 , N);  // buffer vide
		sem_init(&full, 0 , 0);   // buffer vide
		bool finishProd = 0;	  // pas encore la fin du fichier

		//ouverture de fichier
		FILE * file = fopen(argv[index], "rb");
		if(!file){
			printf("reading fail");
			return -1;
		}
		//pas oublie de fermer le fichier en cas d'erreur

	//le vrai code

	pthread_t prod;
	pthread_t cons [N];//thread pour reverseHash

	//creation des threads
	if(pthread_create(&(prod[i]), NULL, &prod, NULL)){
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
	if(!pthread_join(prod,NULL){
			printf("error while pthread_join\n");
			return -1;
	}
	for(int i=0; i<N; i++){
		if(!pthread_join(cons[i],NULL){
			printf("error while pthread_join\n");
			return -1;
		}//check errors
	}

	//terminasion
		free(ProdCons);
		pthread_mutex_destroy(mutex);
		sem_destroy(empty);
		sem_destroy(full);

	return 0;
}

//readFile for threads
unint8_t* readBinFile(FILE* file){
	uint8_t * hash = malloc(sizeof(char)*32);
	if(!hash){
		free(hash);
		printf("malloc fail\n");
		return -1;
	}//malloc errors

	if(fread(hash, sizeof(uint8_t), 32, file)==32){
		return (unit8_t *) hash;
	}//read file 
	else{
		finishProd=1;
		printf("close file\n");
		if(!fclose(file)){return -1;}
	}//end of the file

	return hash;
}//return unint8_t* with hash

// Producteur, Hash
void * producer(FILE* file){
	unint8_t* hash;
	while(!finishProd)
	{
		hash=readBinFile(file);	//readf() 
		
		sem_wait(&empty); // attente d'un slot libre
		pthread_mutex_lock(&mutex);
			// section critique
			insertHash(hash, ProdCons, N);	//ajout dans le tableau la chaine de 32 byte
		pthread_mutex_unlock(&mutex);
		sem_post(&full); // il y a un slot rempli en plus
	}

	return (NULL);
}

// Consommateur, reverseHash
void * consumer(void){
	unint8_t* hash; //tableau de 32 byte (voir Producteur)
	char * resRH = malloc(sizeof(char)*16);
	while(!finishProd || getSemValue(&full) )	//check si la production est terminée & vérifie si le tableau est vide 
	{
		sem_wait(&full); // attente d'un slot rempli
		pthread_mutex_lock(&mutex);
			// section critique
			hash = removeHash(tab, N); //retirer la chaine de 32 bytes du tableau
		pthread_mutex_unlock(&mutex);
		sem_post(&empty); // il y a un slot libre en plus

		if(reversehash(hash, resRH, sizeof(char)*16)){
			printf("%s\n", resRH);
		}//else, le mot de passe ne respecte pas les consignes
		//puis l'enregistrer dans le prochain
	}
	return (NULL);
}

//permet d'obtenir la valuer du semaphore passer en argument
int getSemValue(sem_t * sem){
	int value;
	sem_getvalue(&sem, &value);
	return value;
}

//////////////////////////////////////////////////////////////////////
//encore à implementer

void insertHash(unint8_t* resRH, unit8_t *ProdCons, N){
	int counter=0;
	for(int i=0; i<N & !counter; i++){
	counter=0;
		//check if the 32 byte equal zero
		for(int j=0; j<32; j++){
			counter = counter + *(ProdCons+i*32+j);
		}
		if (counter==0){
			for(int j=0; j<32; j++){
				*(ProdCons+j)=*(resRH+j);
			}
		}
	}
	return NULL;
}

void removeHash(unit8_t *ProdCons[], N){
	//ne pas oublie de remettre la valuer à zéro dans le tableau après de l'avoir extraite
}
