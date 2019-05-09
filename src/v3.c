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
#include <errno.h>



// node structure for password stack
struct node {
    struct node *next;
    char *name;
};
struct arg {
    char **argv;
};

// functions
  /* readBinFile reads the next hash from the binary file and returns it */
  uint8_t* readBinFile(FILE* file, uint8_t * hash);
  /* function reading hashes from binary files */
  void * producer(void * arg);
  /* function reversing hashes to obtain passwords */
  void * consumer();
  /* function sorting passwords */
  void * sort();
  /* small function to modify sem_getValue() */
  int getSemValue(sem_t * sem);
  /* insert in table
   * (if resRH == true : ) */
  void insert(char * A, char * PC, int N, bool resRH);
  void removeHash(uint8_t* hash, uint8_t *ProdCons, int N);
  void removeResRH(char * resRH, char *ProdCons2, int N);
  int push(struct node **head, const char *value);
  int pop(struct node **head);
  int printStack(struct node **head);
  int strlenVo(char* candidat, bool consonant);
  int saveToFile(struct node ** head, FILE * OutputFile);
  bool sortCond();
  void intError (int err, char *msg);
  void stringError (char *msg);

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
  int finishCons;
  bool consonne = false;
  //CondSort
  pthread_mutex_t mutex3;
  int consFinish = 0;

  struct node * head;
  int numberoffiles;
  int err;


int main(int argc, char *argv[]){
    time_t start =time(0);
    //definir les les variables avec argv[]
    printf("argc = %d \n", argc);
    for(int i=0; i<argc; i++){
    printf("argv[%d] = %s, ",i, argv[i]);}
    printf("\n");

    //TODO check if with argument but no input file
    if(argc==1){
      stringError("the function needs at least one argument");
    }

    int opt;
    while((opt = getopt(argc, argv, "t:co:"))!=-1){
      switch (opt)
      {
        case 't':
          N = atoi(optarg);
          if(N<=0){
            stringError("the number of thread is incorrect, please try again");
          }
          break;
        case 'c':
          consonne = true;
          break;
        case 'o':
          OutputToFile = true;
          printf("the output of the program will be in the file : %s\n",optarg);
          outFile = fopen(optarg, "w");
          if(outFile==NULL){
            intError(0,"error with opening the output file"); //TODO can be better
          }
          break;
        case '?':
          stringError("error in the argument");
          break;
      }
    }
    printf("end of getopt ; optind = %d\n", optind);


  // Initialisation

    ProdCons = (uint8_t *) calloc(N, sizeof(uint8_t)*sizeofHash);//create the table
    if(ProdCons==NULL){
      stringError("calloc ProdCons fail");
    }
    ProdCons2 = (char *) calloc(N, sizeof(char)*sizeofString);
    if(ProdCons==NULL){
      stringError("calloc ProdCons2 fail");
    }
    head=NULL;
    finishCons=0;

    //init all mutex and sem
    err = pthread_mutex_init(&mutex, NULL);
    if(err!=0){intError(err, "pthread_mutex_init of mutex");}
    err = sem_init(&empty, 0 , N);  // empty buffer
    if(err!=0){intError(err, "sem_init of empty");}
    err = sem_init(&full, 0 , 0);   // empty buffer
    if(err!=0){intError(err, "sem_init of full");}
    finishProd = 0;   // counter for number of read files

    err = pthread_mutex_init(&mutex2, NULL);
    if(err!=0){intError(err, "pthread_mutex_init of mutex2");}
    err = sem_init(&empty2, 0 , N);  // empty buffer
    if(err!=0){intError(err, "sem_init of empty2");}
    err = sem_init(&full2, 0 , 0);   // empty buffer
    if(err!=0){intError(err, "sem_init of full2");}
    //var? // bruteforce is not yet finished

    err = pthread_mutex_init(&mutex3, NULL);
    if(err!=0){intError(err, "pthread_mutex_init of mutex3");}



    //TODO pas oublier de fermer le fichier en cas d'erreur

  //START OF CODE

  pthread_t prod;
  pthread_t cons [N];//thread pour reverseHash
  pthread_t cons2;

  //creation of threads
  printf("sizeof argv %d", (int)sizeof(strlen(argv[3])) );
  struct arg* ARG = (struct arg*) malloc(sizeof(int)*2+sizeof(char**)) ;
  if(ARG==NULL){printf("malloc error\n");}
  numberoffiles=argc - optind;

  printf("*argv[optind]=%s\n", argv[optind]);

  printf("numberoffiles = %d\n", numberoffiles );
  ARG->argv=malloc(sizeof(char*) * numberoffiles);
  if(ARG->argv==NULL){printf("malloc error\n");}

  //pour le bon nombre de fichier
  for(int i=0; i<numberoffiles; i++){
    ARG->argv[i]=malloc(strlen(argv[optind+i]));
    if(ARG->argv[i]==NULL){printf("malloc error\n");}
    printf("on est cool2\n");
    ARG->argv[i]=argv[optind+i];
    printf("%s\n",argv[optind+i] );
  }
  err = pthread_create(&prod, NULL, &producer, (void*)ARG);
  if(err!=0){
      intError(err, "pthread_create of producer");
  }

  for(int i=0; i<N; i++){
    err = pthread_create(&(cons[i]), NULL, &consumer, NULL);
    if(err!=0){
      intError(err, "pthread_create of consumer");
    }
    //printf("create a consumer [%d]\n",i );
  }
  err = pthread_create(&cons2, NULL, &sort, NULL);
  if(err!=0){
      intError(err, "pthread_create of sort");
  }

  //join of threads
  err = pthread_join(prod,NULL);
  if(err!=0){
    intError(err, "pthread_join of producer");
  }
  for(int i=0; i<N; i++){
    err = pthread_join(cons[i],NULL);
    if(err!=0){
      intError(err, "pthread_join of consumer");
    }
    //printf("fin de cons[%d]\n", i);
  }

  err= pthread_join(cons2,NULL);
  if(err!=0){
    intError(err, "pthread_join of sort");
  }

  printStack(&head); //TODO if in terminal
  if (OutputToFile) {
    saveToFile(&head, outFile);
    err = fclose(outFile);
    if(err!=0){
      pop(&head); // TODO on devrait faire le pop avant chaque return de la main enft...
      intError(err, "fclose(outFile)");
    }
  }
  pop(&head);

  time_t end =time(0);
  double TheTime = difftime(end, start);
  printf("program end in %f second\n",TheTime);

  //clean up

    free(ProdCons);
    err = pthread_mutex_destroy(&mutex);
    if(err!=0){
      intError(err, "pthread_mutex_destroy of mutex");
    }
    err = sem_destroy(&empty);
    if(err!=0){
      intError(err, "sem_destroy of empty");
    }
    err = sem_destroy(&full);
    if(err!=0){
      intError(err, "sem_destroy of full");
    }

    free(ProdCons2);
    err = pthread_mutex_destroy(&mutex2);
    if(err!=0){
      intError(err, "pthread_mutex_destroy of mutex2");
    }
    err = sem_destroy(&empty2);
    if(err!=0){
      intError(err, "sem_destroy of empty2");
    }
    err = sem_destroy(&full2);
    if(err!=0){
      intError(err, "sem_destroy of full2");
    }

    err = pthread_mutex_destroy(&mutex3);
    if(err!=0){
      intError(err, "pthread_mutex_destroy of mutex3");
    }


  return (EXIT_SUCCESS);
}

//readFile for threads
uint8_t* readBinFile(FILE* file, uint8_t * hash){
  if(fread(hash, sizeof(uint8_t), sizeofHash, file)==sizeofHash){
    //print in hex
    printf("0x ");
    for(int i = 0; i < 32; i++){
      printf("%x", hash[i]);
    }
    printf("\n");
    return hash;
  }//read file
  else{
    printf("close file\n");
    if(fclose(file)){
      printf("error while closing");
      printf("finishProd %d\n", finishProd);
      return NULL;
    }
    finishProd++;
    return NULL;
    //attention double le dernier hash trouver une methode pour eviter ca!! //? comment to delete ?
  } //end of the file

  return hash;
}//return unint8_t* with hash

// Producteur, Hash
void * producer(void * arg){

  char **ARGV=((struct arg*) arg)->argv;
  for(int i=0; i<numberoffiles; i++){
    printf("Prod; ARGV[%d] = %s\n", i, ARGV[i]);
  }



  for(int i =0; i<numberoffiles;i++){
    //ouverture de fichier
    file = fopen(ARGV[i], "rb");
    printf("file is open[%d], %s\n",i, ARGV[i] );
    if(!file){
      printf("reading fail\n");
      return NULL;
    }
    uint8_t * hash = malloc(sizeof(char)*sizeofHash);
    if(!hash){
      printf("malloc fail\n");
      return NULL;
    }
    bool test=true;
    while(test){
      hash=readBinFile(file, hash); //readf()
      if(hash!=NULL){
        sem_wait(&empty); // attente d'un slot libre
        pthread_mutex_lock(&mutex);
          // section critique 1
          insert((char*)hash, (char*)ProdCons, N, false);  //ajout dans le tableau la chaine de sizeofHash (32) byte
        pthread_mutex_unlock(&mutex);
        sem_post(&full); // il y a un slot rempli en plus
      }
      else{
        test=false;
      }
    }
    test=true;
    free(hash);
  }

  printf("end of producer\n");
  return NULL;
}

// Consommateur, reverseHash
void * consumer(){
  uint8_t * hash = malloc(sizeof(char)*sizeofHash);
  if(hash==NULL){printf("malloc error\n");}
  char * resRH = malloc(sizeof(char)*sizeofString);
  if(!hash || !resRH){
    free(hash); free(resRH);
    printf("malloc fail\n");
    return NULL;
  }
  pthread_mutex_lock(&mutex3);
  while(finishProd<numberoffiles || getSemValue(&full) ) //check si la production est terminee et verifie si le tableau est vide
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
    // printf("sort, full2:%d\n", getSemValue(&full2));
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
  for(int i=0; (i<N) & !counter; i++){
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
  for(int i=0; (i<N) & !counter; i++){
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
  if(varC==NULL){printf("malloc error\n");}
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
  //printf("I'm checking sortCond\n");
  if(getSemValue(&empty2) != N){
    //printf("SV\n");
    return true;
  }
  else if(pthread_mutex_trylock(&mutex3) == 0) {
    pthread_mutex_unlock(&mutex3);
    //printf("TL, consFinish = %d\n",consFinish);
    if(consFinish >= N){
      //printf("return false in CondSort\n");
      return false;
    }
    return true;
  }
  else
  {
    sleep(1); //for better permorfmence
    return sortCond();
  }
}



void intError (int err, char *msg){
  fprintf(stderr, "%s has returned %d, error message : %s\n", msg, err, strerror(errno));
  exit(EXIT_FAILURE);
}
void stringError (char *msg){
  fprintf(stderr, "%s\n", msg);
  exit(EXIT_FAILURE);
}
