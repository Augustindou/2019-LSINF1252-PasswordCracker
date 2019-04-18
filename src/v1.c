// Include
#include "reverse.h"
#include "reverse.c"
#include "sha256.h"
#include "sha256.c"

//standard input/output
#include <stdio.h>

//get type of variable
#include <ctype.h>

//for open the file
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// stat of a file
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

//malloc
#include <stdlib.h>

//atoi
// #include <stdio.h>
// #include <stdlib.h>
#include <string.h>

int main( int argc, char*argv[]){

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
	// char ch2[] = "-c";
	if(argc>index && !strcmp(argv[index], "-c")){
		index=index+1;
		printf("cas des consonnes\n");
	}
	//encore à faire! cas ou pas le prochain n'est pas un -o mais l'argument de consonnes n'est pas bien respecté

// ./cracker [-t NTHREADS] [-c] [-o FICHIEROUT] FICHIER1 [FICHIER2 ... FICHIERN]

	// cas d'output dans un document
	// char ch3[] = "-o";
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
	// c'est mieux de créer une méthode pour ouvrir les fichier

	/* need to check each file
	while(index<argc){
		printf("file\n");
		index++;
	}
	*/

	//check if there is still something and that the file is a .bin file
	// Check length of filenam
	if(index<argc && strlen(argv[index])>=4 && !strcmp(argv[index] + strlen(argv[index])-4, ".bin")){
		//open the file
		printf("reading the file\n");

		FILE * file = fopen(argv[index], "rb");
		if(!file){
			printf("reading fail");
			return -1;
		}

		//read the file
		int sizeread;
		uint8_t * hash = malloc(sizeof(char)*32);
		char * resRH = malloc(sizeof(char)*16);
		if(!hash || !resRH){
			free(hash); free(resRH);
			printf("malloc fail\n");
			return -1;
		}
		do{
			sizeread = fread(hash, sizeof(uint8_t), 32, file);
			//print in hex
			// printf("0x ");
			// for(int i = 0; i < 32; i++)
			// {printf("%x ", hash[i]); }
			// printf("\n");

			if(reversehash(hash, resRH, sizeof(char)*16)){
				printf("%s\n", resRH);
			}
			// 2x printf(lastPassWord)
		} while(sizeread==32);


		//free the memory
		//free(temp);

		//close the file
		printf("close file\n");
		if(fclose(file)==-1){return -3;}
	}
	else{
		printf("please select a file\n");
		return -1;
	}

return 0;
}