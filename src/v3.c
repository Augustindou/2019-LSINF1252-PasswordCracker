#define SIZE_OF_HASH 32
#define SIZE_OF_STRING 17

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

#include "v3.h"

/*---------------------------------------------------------------------------*/
/*     Variables                                                             */
/*---------------------------------------------------------------------------*/

/* file for input files ; outFile for output file */
FILE* file;
FILE* outFile;
bool OutputToFile = false;
/* Number of threads */
int N = 1;
/* First buffer (before reverseHash) and related mutex and semaphores */
uint8_t * ProdCons;
pthread_mutex_t mutex;
sem_t empty;
sem_t full;
/* Second buffer (after reverseHash) and related mutex and semaphores */
char * ProdCons2;
pthread_mutex_t mutex2;
sem_t empty2;
sem_t full2;
/* counter for number of files read */
int finishProd=0;
/* consonne == true if case of consonants */
bool consonne = false;
/* Mutex to handle sort condition */
pthread_mutex_t mutex3;
/* Number of threads that finished reverseHash */
int consFinish = 0;
/* Head of the password stack */
struct node * head;
/* Number of input files */
int numberOfFiles;
/* Error handling variable */
int err;

//TODO : COMMENT MAIN ??? ------------------------------------------------------
int main(int argc, char *argv[]){
    time_t start =time(0);
    //print variables of argv[]
    printf("argc = %d \n", argc);
    for(int i=0; i<argc; i++){
    printf("argv[%d] = %s, ",i, argv[i]);}
    printf("\n");

    //TODO check if with argument but no input file -------------------------------------
    if(argc<=1){
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

    ProdCons = (uint8_t *) calloc(N, sizeof(uint8_t)*SIZE_OF_HASH);//create the table
    if(ProdCons==NULL){
      stringError("calloc ProdCons fail");
    }
    ProdCons2 = (char *) calloc(N, sizeof(char)*SIZE_OF_STRING);
    if(ProdCons==NULL){
      stringError("calloc ProdCons2 fail");
    }
    head=NULL;

    //init all mutex and sem
    err = pthread_mutex_init(&mutex, NULL);
    if(err!=0){intError(err, "pthread_mutex_init of mutex");}
    err = sem_init(&empty, 0 , N);  // empty buffer
    if(err!=0){intError(err, "sem_init of empty");}
    err = sem_init(&full, 0 , 0);   // empty buffer
    if(err!=0){intError(err, "sem_init of full");}


    err = pthread_mutex_init(&mutex2, NULL);
    if(err!=0){intError(err, "pthread_mutex_init of mutex2");}
    err = sem_init(&empty2, 0 , N);  // empty buffer
    if(err!=0){intError(err, "sem_init of empty2");}
    err = sem_init(&full2, 0 , 0);   // empty buffer
    if(err!=0){intError(err, "sem_init of full2");}

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
  if(ARG==NULL){stringError("malloc ARG error");}
  numberOfFiles=argc - optind;

  printf("*argv[optind]=%s\n", argv[optind]);

  printf("numberOfFiles = %d\n", numberOfFiles );
  ARG->argv=malloc(sizeof(char*) * numberOfFiles);
  if(ARG->argv==NULL){stringError("malloc ARG->argv error");}

  //pour le bon nombre de fichier
  for(int i=0; i<numberOfFiles; i++){
    ARG->argv[i]=malloc(strlen(argv[optind+i]));
    if(ARG->argv[i]==NULL){stringError("malloc ARG->argv[i] error\n");}
    ARG->argv[i]=argv[optind+i];
  }
  err = pthread_create(&prod, NULL, &producer, (void*)ARG);
  if(err!=0){intError(err, "pthread_create of producer");}

  for(int i=0; i<N; i++){
    err = pthread_create(&(cons[i]), NULL, &consumer, NULL);
    if(err!=0){intError(err, "pthread_create of consumer");}
    //printf("create a consumer [%d]\n",i );
  }
  err = pthread_create(&cons2, NULL, &sort, NULL);
  if(err!=0){intError(err, "pthread_create of sort");}

  //join of threads
  err = pthread_join(prod,NULL);
  if(err!=0){intError(err, "pthread_join of producer");}
  for(int i=0; i<N; i++){
    err = pthread_join(cons[i],NULL);
    if(err!=0){intError(err, "pthread_join of consumer");}
    //printf("fin de cons[%d]\n", i);
  }

  err= pthread_join(cons2,NULL);
  if(err!=0){intError(err, "pthread_join of sort");}


  if (OutputToFile) {
    saveToFile(&head, outFile);
    err = fclose(outFile);
    if(err!=0){
      pop(&head); // TODO on devrait faire le pop avant chaque return de la main enft...
      intError(err, "fclose(outFile)");
    }
  }
  else{
    printStack(&head);
  }
  pop(&head);


  //clean up

    free(ProdCons);
    err = pthread_mutex_destroy(&mutex);
    if(err!=0){intError(err, "pthread_mutex_destroy of mutex");}
    err = sem_destroy(&empty);
    if(err!=0){intError(err, "sem_destroy of empty");}
    err = sem_destroy(&full);
    if(err!=0){intError(err, "sem_destroy of full");}

    free(ProdCons2);
    err = pthread_mutex_destroy(&mutex2);
    if(err!=0){intError(err, "pthread_mutex_destroy of mutex2");}
    err = sem_destroy(&empty2);
    if(err!=0){intError(err, "sem_destroy of empty2");}
    err = sem_destroy(&full2);
    if(err!=0){intError(err, "sem_destroy of full2");}

    err = pthread_mutex_destroy(&mutex3);
    if(err!=0){intError(err, "pthread_mutex_destroy of mutex3");}


    time_t end =time(0);
    double TheTime = difftime(end, start);
    printf("program end in %f second\n",TheTime);

  return (EXIT_SUCCESS);
}

//readFile
uint8_t* readBinFile(FILE* file, uint8_t * hash){
  if(fread(hash, sizeof(uint8_t), SIZE_OF_HASH, file)==SIZE_OF_HASH){
    return hash;
  }
  else{
    printf("close file\n");
    err=fclose(file);
    if(err!=0){intError(err, "fclose failed");}
    finishProd++;
    return NULL;
  } //end of the file

  return hash;
}

// Producteur, Hash
void * producer(void * arg){

  //get fileName for input files
  char **ARGV=((struct arg*) arg)->argv;
  for(int i=0; i<numberOfFiles; i++){
    printf("Prod; ARGV[%d] = %s\n", i, ARGV[i]);
  }

  for(int i =0; i<numberOfFiles;i++){
    //file opens
    file = fopen(ARGV[i], "rb");
    if(!file){stringError("input file didn't open correctly");}
    printf("file is open[%d], %s\n",i, ARGV[i] );

    uint8_t * hash = malloc(sizeof(char)*SIZE_OF_HASH);
    if(!hash){stringError("malloc ARG error");}
    bool test=true;
    while(test){
      hash=readBinFile(file, hash); //readf()
      if(hash!=NULL){
        err = sem_wait(&empty); // wait for an empty slot
        if(err!=0){intError(err, "sem_wait empty error");}
        err = pthread_mutex_lock(&mutex);
        if(err!=0){intError(err, "pthread_mutex_lock mutex error");}
          // critical zone 1
          insertInBuffer((char*)hash, (char*)ProdCons, N, false);
        err = pthread_mutex_unlock(&mutex);
        if(err!=0){intError(err, "pthread_mutex_unlock mutex error");}
        err = sem_post(&full); // a slot has been filled
        if(err!=0){intError(err, "sem_post full error");}
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

// Consomer, most of prossesor time
void * consumer(){
  uint8_t * hash = malloc(sizeof(char)*SIZE_OF_HASH);
  if(hash==NULL){stringError("malloc hash error");}
  char * resRH = malloc(sizeof(char)*SIZE_OF_STRING);
  if(resRH==NULL){stringError("malloc resRH error");}

  err = pthread_mutex_lock(&mutex3);
  if(err!=0){intError(err, "pthread_mutex_lock mutex3 error");}
  //check if production is finished and if the buffer is empty
  //TODO check error of getSemValue
  while(finishProd<numberOfFiles || getSemValue(&full) )
  {
    err = pthread_mutex_unlock(&mutex3);
    if(err!=0){intError(err, "pthread_mutex_unlock mutex3 error");}
    err = sem_wait(&full); //wait for a filled slot
    if(err!=0){intError(err, "sem_wait full error");}
    err = pthread_mutex_lock(&mutex);
    if(err!=0){intError(err, "pthread_mutex_lock mutex error");}
      // critical zone 1
      removeFromBuffer((char*) hash, (char*) ProdCons, N, false);
    err = pthread_mutex_unlock(&mutex);
    if(err!=0){intError(err, "pthread_mutex_unlock mutex error");}
    err = sem_post(&empty); // a slot has been cleaned
    if(err!=0){intError(err, "sem_post empty error");}

    err = reversehash(hash, resRH, sizeof(char)*SIZE_OF_STRING);
    if(!err){stringError("reverseHash error");} //need to refer to reverse.c

    err = sem_wait(&empty2); // wait for an empty slot
    if(err!=0){intError(err, "sem_wait empty2 error");}
    err = pthread_mutex_lock(&mutex3);
    if(err!=0){intError(err, "pthread_mutex_lock mutex3 error");}
    err = pthread_mutex_lock(&mutex2);
    if(err!=0){intError(err, "pthread_mutex_lock mutex2 error");}
      // critical zone 2
      insertInBuffer(resRH, ProdCons2, N, true);
    err = pthread_mutex_unlock(&mutex2);
    if(err!=0){intError(err, "pthread_mutex_unlock mutex2 error");}
    err = sem_post(&full2); // a slot has been filled
    if(err!=0){intError(err, "sem_post full2 error");}
  }

  consFinish++;
  err = pthread_mutex_unlock(&mutex3);
  if(err!=0){intError(err, "pthread_mutex_unlock mutex3 error");}
  //printf("End consumer, full: %d, empty: %d\n", getSemValue(&full),getSemValue(&empty));
  //printf("End consumer, full2: %d, empty2: %d\n", getSemValue(&full2),getSemValue(&empty2));

  free(hash);
  return NULL;
}

void * sort(){
  char * resRH = malloc(sizeof(char)*SIZE_OF_STRING);
  if(!resRH) {stringError("malloc resRH error");}
  while(sortCond())  //check if production of contion ended and if the buffer is empty
  {
    //printf("sortWhile\n");
    //printf("sort, full2:%d\n", getSemValue(&full2));
    err = sem_wait(&full2); // wait for a filled slot
    if(err!=0){intError(err, "sem_wait full2 error");}
    err = pthread_mutex_lock(&mutex2);
    if(err!=0){intError(err, "pthread_mutex_lock mutex2 error");}
      // critical zone 2
      removeFromBuffer(resRH, ProdCons2, N, true);
      //printf("mot: %s\n", resRH);
    err = pthread_mutex_unlock(&mutex2);
    if(err!=0){intError(err, "pthread_mutex_unlock mutex2 error");}
    err = sem_post(&empty2); // a slot has been cleaned
    if(err!=0){intError(err, "sem_post empty2 error");}
    if(head==NULL){
      push(&head, resRH); //TODO malloc, arg
    }
    else if(strlenVo(head->name, consonne)<strlenVo(resRH, consonne)){
      //printf("erased from stack:\n");
      printStack(&head);
      //printf("\n");
      pop(&head);
      push(&head, resRH);
    }
    else if(strlenVo(head->name, consonne)==strlenVo(resRH, consonne)){
      push(&head, resRH);
    }
    // else smaller not need to be saved
  }

  //TODO free
  //printf("End sort\n");
  return NULL;
}

int getSemValue(sem_t * sem){
  int value;
  err = sem_getvalue(sem, &value);
  if(err!=0){intError(err, "sem_getvalue error");}
  return value;
}


void removeFromBuffer(char* A, char *PC, int N, bool resRH){
  int counter=0;
  int sz = SIZE_OF_HASH;
  if(resRH){sz = SIZE_OF_STRING;}

  for(int i=0; (i<N) & !counter; i++){
    counter=0;
    for(int j=0; j<sz; j++){
      counter = counter + *(PC+i*sz+j);
    }
    if (counter){
      for(int j=0; j<sz; j++){
        *(A+j)=*(PC+i*sz+j);
        *(PC+i*sz+j)=0;
      }
    }
  }
  return;
}


void insertInBuffer(char * A, char * PC, int N, bool resRH){
  int counter;
  int sz = SIZE_OF_HASH;
  if(resRH){sz = SIZE_OF_STRING;}

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

//TODO set in void
int push(struct node **head, const char *value){
  if(value==NULL){stringError("try to pop an empty stack");}
  char * varC = (char*)malloc(strlen(value)+1);
  if(varC==NULL){stringError("malloc varC error");}
  struct node* newNode = (struct node*) malloc(sizeof(struct node*)+sizeof(char*)) ;
  if(newNode==NULL){stringError("malloc newNode error");}
  varC = strcpy(varC, value);
  newNode->next = *head;//work also for *head==NULL
  newNode->name = varC;
  *head=newNode;
  return 0;
}

void pop(struct node ** head){
  while(*head){
    struct node * first = *head;
    first=*head;
    *head = first->next;
    free(first->name);
    free(first);
  }
  return;
}

void printStack(struct node **head){
  struct node * first = *head;
  while(first != NULL){
    printf("%s\n",(first->name));
    first = (first->next);
  }
  return;
}

int strlenVo(char* candidate, bool consonant){
  int vowels=0;
  int len=strlen(candidate);
  for(int i=0; i<len; i++){
        if(candidate[i]=='a' || candidate[i]=='e' || candidate[i]=='i' ||
           candidate[i]=='o' || candidate[i]=='u' || candidate[i]=='y'){
            vowels++;}
  }
  if(consonant){return len-vowels;}
  return vowels;
}

//TODO from int to void
int saveToFile(struct node ** head, FILE * outputFile){
  struct node * first = *head;
  while(first){
    err=fprintf(outputFile, "%s\n", first->name);
    if(err<0){intError(err, "writing error");}
    first = (first->next);
  }
  return 0;
}


bool sortCond(){
  //printf("I'm checking sortCond\n");
  if(getSemValue(&empty2) != N){
    //printf("SV\n");
    return true;
  }
  else if(pthread_mutex_trylock(&mutex3) == 0) {
    err=pthread_mutex_unlock(&mutex3);
    if(err!=0){intError(err, "pthread_mutex_unlock mutex3 error");}
    //printf("TL, consFinish = %d\n",consFinish);
    if(consFinish >= N){
      //printf("return false in CondSort\n");
      return false;
    }
    return true;
  }
  else
  {
    sleep(1); //for better performance
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
