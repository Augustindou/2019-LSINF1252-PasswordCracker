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
#include <getopt.h>
#include <time.h>



// structure
struct node {
    struct node *next;
    char *name;
};

// functions
  uint8_t* readBinFile(FILE* file, uint8_t * hash);
  void * producer();
  void * consumer();
  void * sort();
  int getSemValue(sem_t * sem);
  void insert(char * A, char * PC, int N, bool resRH);
  void removeHash(uint8_t* hash, uint8_t *ProdCons, int N);
  void removeResRH(char * resRH, char *ProdCons2, int N);
  int push(struct node **head, const char *value);
  int pop(struct node **head);
  int printStack(struct node **head);
  int strlenVo(char* candidat, bool consonant);
  int saveToFile(struct node ** head, FILE * OutputFile);
  bool sortCond();

//variables
  // define a la place de sizeofHash et sizeofString
  int sizeofHash = 32;
  int sizeofString = 17; // 16 + '\0'
  FILE* file;
  FILE* outFile;
  bool OutputToFile = false;
  int N = 1;
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
  int finishCons;
  bool consonne = false;
  //CondSort
  pthread_mutex_t mutex3;
  int consFinish = 0;

  struct node * head;



int main(int argc, char *argv[]){
    time_t start =time(0);
    //definir les les variables avec argv[], comme dans v1.c
    printf("argc = %d \n", argc);
    for(int i=0; i<argc; i++){
    printf("argv[%d] = %s, ",i, argv[i]);}
    printf("\n");

    if(argc==1){
      printf("the function needs at least one argument\n");
      return -1;
    }

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
          outFile = fopen(optarg, "w");
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


  // Initialisation

    ProdCons = (uint8_t *) calloc(N, sizeof(uint8_t)*sizeofHash);//create the table
    if(ProdCons==NULL){
      printf("calloc ProdCons fail\n");
      return -1;
    }
    ProdCons2 = (char *) calloc(N, sizeof(char)*sizeofString);
    if(ProdCons==NULL){
      printf("calloc ProdCons2 fail\n");
      return -1;
    }
    head=NULL;
    finishCons=0;

    //pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_init(&mutex, NULL);
    sem_init(&empty, 0 , N);  // buffer vide
    sem_init(&full, 0 , 0);   // buffer vide
    finishProd = 0;   // pas encore la fin du fichier

    pthread_mutex_init(&mutex2, NULL);
    sem_init(&empty2, 0 , N);  // buffer vide
    sem_init(&full2, 0 , 0);   // buffer vide
    finishProd2 = 0;    // pas encore la fin du bruteforce

    pthread_mutex_init(&mutex3, NULL);

    //ouverture de fichier
    file = fopen(argv[optind], "rb");
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
  for(int i=0; i<N; i++){
    if(pthread_join(cons[i],NULL)!=0){
      printf("error while cons[%d] pthread_join\n",i);
      return -1;
    }//check errors
    printf("fin de cons[%d]\n", i);
    /*
    printf("cons done before the if, finishProd2: %d and i: %d", finishProd2, i);
    if(i==0){
      printf("cons done before, finishProd2: %d", finishProd2);
      finishProd2=1;
      printf("cons done, finishProd2: %d", finishProd2);
    }
    */
  }

  if(pthread_join(cons2,NULL)!=0){
    printf("error while cons2 pthread_join\n");
    return -1;
  }

  printStack(&head);
  if (OutputToFile) {
    saveToFile(&head, outFile);
    if (fclose(outFile)) {
      printf("close output file error\n");
      pop(&head); // on devrait faire le pop avant chaque return de la main enft...
      return -1;
    }
  }
  pop(&head);

  time_t end =time(0);
  double TheTime = difftime(end, start);
  printf("program end in %f second\n",TheTime);

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

    pthread_mutex_destroy(&mutex3);


  return 0;
}

//readFile for threads
uint8_t* readBinFile(FILE* file, uint8_t * hash){
  if(fread(hash, sizeof(uint8_t), sizeofHash, file)==sizeofHash){
    return hash;
  }//read file
  else{
    finishProd=1;
    printf("close file\n");
    if(fclose(file)){
      printf("error while closing\n");
      return NULL;
    }
    //attention double le dernier hash trouver une methode pour eviter ca!!
  } //end of the file

  return hash;
}//return unint8_t* with hash

// Producteur, Hash
void * producer(){
  uint8_t * hash = malloc(sizeof(char)*sizeofHash);
  if(!hash){
    printf("malloc fail\n");
    return NULL;
  }

  while(!finishProd)
  {
    hash=readBinFile(file, hash); //readf()

    sem_wait(&empty); // attente d'un slot libre
    pthread_mutex_lock(&mutex);
      // section critique 1
      insert(hash, ProdCons, N, false);  //ajout dans le tableau la chaine de sizeofHash (32) byte
    pthread_mutex_unlock(&mutex);
    sem_post(&full); // il y a un slot rempli en plus
  }
  //printf("End producer, full: %d, empty: %d\n", getSemValue(&full),getSemValue(&empty));

  free(hash);
  return (NULL);
}

// Consommateur, reverseHash
void * consumer(){
  uint8_t * hash = malloc(sizeof(char)*sizeofHash);
  char * resRH = malloc(sizeof(char)*sizeofString);
  if(!hash || !resRH){
    free(hash); free(resRH);
    printf("malloc fail\n");
    return NULL;
  }

  pthread_mutex_lock(&mutex3);
  while(!finishProd || getSemValue(&full) ) //check si la production est terminee et verifie si le tableau est vide
  {
    pthread_mutex_unlock(&mutex3);
    sem_wait(&full); // attente d'un slot rempli
    pthread_mutex_lock(&mutex);
      // section critique 1
      removeHash(hash, ProdCons, N);
    pthread_mutex_unlock(&mutex);
    sem_post(&empty); // il y a un slot libre en plus

    if(!reversehash(hash, resRH, sizeof(char)*sizeofString)){
      printf("petite erreur dans reverseHash!\n");
    }//le mot de passe ne respecte pas les consignes

    sem_wait(&empty2); // attente d'un slot libre
    pthread_mutex_lock(&mutex3);
    pthread_mutex_lock(&mutex2);
      // section critique 2
      insert(resRH, ProdCons2, N, true);
    pthread_mutex_unlock(&mutex2);
    sem_post(&full2); // il y a un slot rempli en plus

  }
  consFinish++;
  printf("End consumer before closing mutex3\n");
  pthread_mutex_unlock(&mutex3);
  //printf("End consumer, full: %d, empty: %d\n", getSemValue(&full),getSemValue(&empty));
  printf("End consumer, full2: %d, empty2: %d\n", getSemValue(&full2),getSemValue(&empty2));

  free(hash);
  return NULL;
}

void * sort(){
  char * resRH = malloc(sizeof(char)*sizeofString);
  if(!resRH) {
    printf("malloc fail\n");
    return NULL;
  }
  while(sortCond())  //check si la production est terminee et verifie si le tableau est vide
  {
    printf("sortWhile\n");
    // printf("sort, finishCons: %d, full2:%d\n", finishProd2, getSemValue(&full2));
    //if(!getSemValue(&full2)){printf("sort, full2: %d\n", getSemValue(&full2));}
    sem_wait(&full2); // attente d'un slot rempli
    pthread_mutex_lock(&mutex2);
      // section critique 2
      removeResRH(resRH, ProdCons2, N);
      printf("mot: %s\n", resRH);
    pthread_mutex_unlock(&mutex2);
    sem_post(&empty2); // il y a un slot libre en plus
    if(head==NULL){
      push(&head, resRH);
    }
    // bool consonne=false;//a definir dans la premiere partie du code
    if(head==NULL){
      printf("add first %s\n", resRH);
      push(&head, resRH);
    }
    else if(strlenVo(head->name, consonne)<strlenVo(resRH, consonne)){
      printf("stack erased:\n");
      printStack(&head);
      printf("\n");
      pop(&head);
      push(&head, resRH);
    }
    else if(strlenVo(head->name, consonne)==strlenVo(resRH, consonne)){
      push(&head, resRH);
    }
    //else est plus petit
  }

  printf("End sort\n");
  return NULL;
}

//permet d'obtenir la valeur du semaphore passe en argument
int getSemValue(sem_t * sem){
  int value;
  sem_getvalue(sem, &value);
  return value;
} // it works :D

void removeHash(uint8_t* hash, uint8_t *ProdCons, int N){
  int counter=0;
  for(int i=0; i<N & !counter; i++){
    counter=0;
    for(int j=0; j<sizeofHash; j++){
      counter = counter + *(ProdCons+i*sizeofHash+j);
    }
    if (counter){
      for(int j=0; j<sizeofHash; j++){
        *(hash+j)=*(ProdCons+i*sizeofHash+j);
          *(ProdCons+i*sizeofHash+j)=0;
      }
    }
  }
  return;
}

// if resRH == true => insertResRH ; else => insertHash
void insert(char * A, char * PC, int N, bool resRH){
  int counter;
  int sz = sizeofHash;
  if(resRH){sz = sizeofString;}

  for(int i=0; i<N; i++){
    counter=0;
    for(int j=0; j<sz; j++){
      counter = counter + *(PC+i*sz+j);
    }
    if (!counter){
      for(int j=0; j<sz; j++){
        *(PC+i*sz+j)=*(A+j);
      }
      return;
    }
  }
  return;
}

void removeResRH(char * resRH, char *ProdCons2, int N){
  int counter=0;
  int nbLettre=sizeofString;
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
* @return 0 if no error, -1 otherwise
*/

int push(struct node **head, const char *value){
  if(value==NULL){return -1;}
  char * varC = (char*)malloc(strlen(value)+1);
  struct node* newNode = (struct node*) malloc(sizeof(struct node*)+sizeof(char*)) ;
  if(!newNode || !varC){
    free(newNode); free(varC);
    return -1;
  }
  varC = strcpy(varC, value);
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
* @return 0 if no error, -1 otherwise // enft y a pas moy de detecter des erreurs pck free() est une fct void... donc en soi autant mettre une void pop(struct node** head);
*
* pre :
* post :
*/
int pop(struct node **head){
  while(*head){
    struct node * first = *head;
    first=*head;
    *head = first->next;
    free(first->name);
    free(first);
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

int saveToFile(struct node ** head, FILE * OutputFile){
  struct node * first = *head;
  int wErr = 0;
  while(first){
    if(fprintf(OutputFile, "%s\n", first->name) < 0){
      wErr++;
    }
    first = (first->next);
  }
  if (wErr > 0) {
    printf("Number of writing errors : %d\n", wErr);
  }
  else
  {
    printf("No writing errors !\n");
  }
  return 0;
}

// return true if sort should continue sorting ; false if not
bool sortCond(){
  if(getSemValue(&empty2) != N){
    printf("SV\n");
    return true;
  }
  else if(pthread_mutex_trylock(&mutex3) == 0) {
    pthread_mutex_unlock(&mutex3);
    printf("TL, consFinish = %d\n",consFinish);
    if(consFinish >= N){//changed
      return false;
    }
    return true;
  }
  else
  {
    sleep(1);
    return sortCond();
  }
}
