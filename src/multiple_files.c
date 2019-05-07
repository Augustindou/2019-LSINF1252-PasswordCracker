#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <semaphore.h>


u_int8_t* readBinFile(FILE* file, u_int8_t * hash);
//void saveToFile2(u_int8_t hash, FILE * OutputFile);
FILE* file;
FILE* outFile;
int sizeofHash;
int N;
bool consonne;
bool OutputToFile;

int main(int argc, char *argv[]){
  time_t start =time(0);

  printf("argc = %d \n", argc);
  for(int i=0; i<argc; i++){
    printf("argv[%d] = %s, ",i, argv[i]);
  }
  printf("\n");

  int opt;
  while((opt = getopt(argc, argv, "t:co:"))!=-1){
    switch (opt)
    {
      case 't':
        N = atoi(optarg);
        if(!N || N<=0){
          printf("le nombre de thread est incorrect, veillez reessayer\n");
          return -1;
        }
        break;
      case 'c':
        consonne = true;
        break;
      case 'o':
        OutputToFile = true;
        printf("la sortie du programme se fera dans le fichier %s\n",optarg);
        outFile = fopen(optarg, "bw");
        if(!outFile){
          printf("creating output file fail\n");
          return -1;
        }
        break;
      case '?':
        printf("erreur argument");
        break;
    }
  }
  printf("end of getopt ; optind = %d\n", optind);

  int positionFiles=optind;
  printf("first file at position: %d\n", positionFiles);
  sizeofHash=32;
  u_int8_t * hash = (u_int8_t *) malloc(sizeof(u_int8_t)*sizeofHash);
  if(hash==NULL){
    printf("malloc hash fail\n");
    return -1;
  }

  //ouverture de fichier
  while (positionFiles<argc) {
    printf("File %d is open\n", positionFiles-optind);
    file = fopen(argv[positionFiles], "rb");
    if(!file){
      printf("reading fail\n");
      return -1;
    }
    while (readBinFile(file,hash)) {
      //print in hex
			printf("0x ");
			for(int i = 0; i < 32; i++)
			{printf("%x", hash[i]); }
			printf("\n");
    }
    positionFiles++;
  }
  time_t end =time(0);
  double TheTime = difftime(end, start);
  printf("program end in %f second\n",TheTime);
  return 0;
}

u_int8_t* readBinFile(FILE* file, u_int8_t * hash){
  if(fread(hash, sizeof(u_int8_t), sizeofHash, file)==sizeofHash){
    return hash;
  }//read file
  else{
    printf("close file\n");
    if(fclose(file)){
      printf("error while closing\n");
      return NULL;
    }
    return NULL;
  }

  return hash;
}

/*
void saveToFile2(char * hash, FILE * OutputFile){
  if(fprintf(OutputFile, "%s", &hash) < 0){
    printf("Writing error occure\n");
  }
}
*/
