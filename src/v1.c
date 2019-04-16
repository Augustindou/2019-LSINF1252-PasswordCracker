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
#include <stdio.h>
#include <stdlib.h>
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

	// case given number of threads             If no given number, recursive call to main with -t 1 ?? (idea)
	char ch[] = "-t";
	if(argc>2 && !strcmp(argv[index], ch)){
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
	char ch2[] = "-c";
	if(argc>index && !strcmp(argv[index], ch2)){
		index=index+1;
		printf("cas des consonnes\n");
	}
	//encore à faire! cas ou pas le prochain n'est pas un -o mais l'argument de consonnes n'est pas bien respecté

// ./cracker [-t NTHREADS] [-c] [-o FICHIEROUT] FICHIER1 [FICHIER2 ... FICHIERN]

	// cas d'output dans un document
	char ch3[] = "-o";
	// a ajouter la condition pour eviter de prendre comme fichier out en fichier in
	if(argc>index+1 && !strcmp(argv[index], ch3)){
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
	char ch4[] = ".bin";
	if(index<argc && strlen(argv[index]) && !strcmp(argv[index]+ strlen(argv[index])-4, ch4)){
		//open the file
		printf("reading the file\n");
		int fp = open(argv[index], O_RDONLY);
		if(fp == -1){
			printf("reading fail");
			return -1;
		}

		//get stat
		struct stat *buf = malloc(sizeof(struct stat));
		if(buf==NULL){return -1;}
		if(stat(argv[1], buf)==-1){return -1;}

		//if help needed look at https://stackoverflow.com/questions/979816/whats-a-binary-file-and-how-do-i-create-one
		//read the file
		char *n1= malloc(sizeof(char));
		for(int i=0; i*sizeof(char) < buf->st_size; i++){
			if(read(fp, (int *) n1 +i, sizeof(unsigned char))==-1){return -2;}
			printf("%x",n1);
			if((i+1)%32==0){printf("\n");}
		}


		//print to check if correctly read
		//printf("temp = %s\n",temp);
		//free the memory
		//free(temp);
		free(buf);

		//close the file
		if(close(fp)==-1){return -3;}
	}
	else{
		printf("please select a file\n");
		return -1;
	}

return 0;
}
