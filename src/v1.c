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

// ./cracker [-t NTHREADS] [-c] [-o FICHIEROUT] FICHIER1 [FICHIER2 ... FICHIERN]
int main( int argc, char*argv[]){
	
	//cas aucun d'argument
	if(argc==0){printf("the function need at least one argument\n");}
	
	int index = 1;

	//cas 1 argument
	//c'est mieux de créer une méthode pour ouvrir les fichier
	if(argc==2){
		//open the file
		int fp = open(argv[index], O_RDONLY);
		if(fp == -1){return -1;}
		
		//get stat
		struct stat *buf = malloc(sizeof(struct stat));
		if(buf==NULL){return -1;}
		if(stat(argv[1], buf)==-1){return -1;}

		//read the file
		char* temp= malloc(buf->st_size);
		if(temp==NULL){return -1;}
		if(read(fp, temp, buf->st_size)!=buf->st_size){return -1;}
		
		//print to check if correctly read
		printf("%s",temp);

		//free the memory
		free(temp);
		free(buf);
		
		//close the file
		if(close(fp)==-1){return -3;}
	}

/*
	if(index<argc){
		printf("is testing\n");
		if(isdigit(*argv[index])){printf("now need to get this number\n");}	
	}
*/

return 0;
}
