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

/*---------------------------------------------------------------------------*/
/*     Thread-based functions                                                */
/*---------------------------------------------------------------------------*/
  /**
   * Adds new hashes from binary file to first buffer
   *
   * @arg : ** first bin file
   *
   * @return //TODO --------------------------------------------------------------------
   */
  void * producer(void * arg);
  /**
   * Gets hashes from first buffer
   * Bruteforce hash => password
   * Adds passwords to second buffer
   *
   * @return //TODO --------------------------------------------------------------------
   */
  void * consumer();
  /**
   * Gets passwords from second buffer
   * Checks if password is better than last ones
   * If better : frees the stack and add new password
   * If equal : adds password to stack
   * If worse : discards new password
   *
   * @return //TODO --------------------------------------------------------------------
   */
  void * sort();
/*---------------------------------------------------------------------------*/
/*     Buffer-based functions                                                */
/*---------------------------------------------------------------------------*/

  /**
   * Inserts byte array in buffer
   *
   * @A : *value to be inserted
   * @PC : *buffer
   * @N : Size of buffer (== number of threads)
   * @resRH : if true => A is a candidate ; if false => A is a hash
   */
  void insertInBuffer(char * A, char * PC, int N, bool resRH);
  /**
   * Removes byte array from buffer
   *
   * @A : *removed value
   * @PC : *buffer
   * @N : Size of buffer (== number of threads)
   * @resRH : if true => A is a candidate ; if false => A is a hash
   */
  void removeFromBuffer(char* A, char *PC, int N, bool resRH);

/*---------------------------------------------------------------------------*/
/*     Stack-based functions                                                 */
/*---------------------------------------------------------------------------*/

  /**
   * Add @value at the "top" of the stack.
   *
   * @head : pointer to the top of the stack
   * @value : the string to be placed in the element at the top of the stack
   *
   * @return 0 if no error, -1 otherwise
   */
  int push(struct node **head, const char *value);
  /**
   * Free all elements of the stack.
   *
   * @head : pointer to the top of the stack
   *
   * @return 0 if no error, -1 otherwise
   */
  int pop(struct node **head);
  /**
   * Print all elements of the stack.
   *
   * @head : pointer to the top of the stack
   *
   * @return 0 if no error, -1 otherwise
   */
  int printStack(struct node **head);
  /**
   * Saves stack in outputFile
   *
   * @head : top of the stack
   * @outputFile : *FILE where the passwords should be written
   *
   * @return 0 if no error, -1 otherwise
   */
  int saveToFile(struct node ** head, FILE * outputFile);

/*---------------------------------------------------------------------------*/
/*     Small functions                                                       */
/*---------------------------------------------------------------------------*/

  /**
   * Calculate number of vowels or consonants for a given candidate
   *
   * @candidate : string to be checked
   * @consonant : if true => count consonants ; else => count vowels
   *
   * @return number of vowels/consonants in candidate
   */
  int strlenVo(char* candidate, bool consonant);
  /**
   * Checks if program should continue sorting
   *
   * @return true if sort should continue ; false otherwise
   */
  bool sortCond();
  /**
   * transforms sem_getValue() to have value returned
   *
   * @sem : semaphore to check
   *
   * @return value of semaphore
   */
  int getSemValue(sem_t * sem);
  /**
   * Reads the next hash from the binary file and returns it
   * Closes file if no more hashes left
   *
   * @file : //TODO -------------------------------------------------------------------
   * @hash : //TODO -------------------------------------------------------------------
   *
   * @return 0 if no error, -1 otherwise
   */
  uint8_t* readBinFile(FILE* file, uint8_t * hash);

/*---------------------------------------------------------------------------*/
/*     Error handling functions                                              */
/*---------------------------------------------------------------------------*/

  /**
   * Print error and EXIT_FAILURE
   *
   * @err : error depending on the function that caused the error
   * @msg : explanation of the error
   */
  void intError (int err, char *msg);
  /**
   * Print error and EXIT_FAILURE
   *
   * @msg : explanation of the error
   */
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
  int finishProd=0; // counter for number of files read
  //ProdCons2
  char * ProdCons2;
  pthread_mutex_t mutex2;
  sem_t empty2;
  sem_t full2;
  bool consonne = false;
  //CondSort
  pthread_mutex_t mutex3;
  int consFinish = 0;

  struct node * head;
  int numberoffiles;
  int err;


int main(int argc, char *argv[]){
    time_t start =time(0);
    //print variables of argv[]
    printf("argc = %d \n", argc);
    for(int i=0; i<argc; i++){
    printf("argv[%d] = %s, ",i, argv[i]);}
    printf("\n");

    //TODO check if with argument but no input file -------------------------------------
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
  numberoffiles=argc - optind;

  printf("*argv[optind]=%s\n", argv[optind]);

  printf("numberoffiles = %d\n", numberoffiles );
  ARG->argv=malloc(sizeof(char*) * numberoffiles);
  if(ARG->argv==NULL){stringError("malloc ARG->argv error");}

  //pour le bon nombre de fichier
  for(int i=0; i<numberoffiles; i++){
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
  if(fread(hash, sizeof(uint8_t), sizeofHash, file)==sizeofHash){
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
  for(int i=0; i<numberoffiles; i++){
    printf("Prod; ARGV[%d] = %s\n", i, ARGV[i]);
  }

  for(int i =0; i<numberoffiles;i++){
    //file opens
    file = fopen(ARGV[i], "rb");
    if(!file){stringError("input file didn't open correctly");}
    printf("file is open[%d], %s\n",i, ARGV[i] );

    uint8_t * hash = malloc(sizeof(char)*sizeofHash);
    if(!hash){stringError("malloc ARG error");}
    bool test=true;
    while(test){
      hash=readBinFile(file, hash); //readf()
      if(hash!=NULL){
        sem_wait(&empty); // wait for an empty slot
        pthread_mutex_lock(&mutex);
          // critical zone 1
          insertInBuffer((char*)hash, (char*)ProdCons, N, false);
        pthread_mutex_unlock(&mutex);
        sem_post(&full); // a slot has been filled
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
  uint8_t * hash = malloc(sizeof(char)*sizeofHash);
  if(hash==NULL){stringError("malloc hash error");}
  char * resRH = malloc(sizeof(char)*sizeofString);
  if(resRH==NULL){stringError("malloc resRH error");}

  pthread_mutex_lock(&mutex3);
  //check if production is finished and if the buffer is empty
  while(finishProd<numberoffiles || getSemValue(&full) )
  {
    pthread_mutex_unlock(&mutex3);
    sem_wait(&full); //wait for a filled slot
    pthread_mutex_lock(&mutex);
      // critical zone 1
      removeFromBuffer((char*) hash, (char*) ProdCons, N, false);
    pthread_mutex_unlock(&mutex);
    sem_post(&empty); // a slot has been cleaned

    err = reversehash(hash, resRH, sizeof(char)*sizeofString)
    if(err!=0)){stringError("reverseHash error");} //need to refer to reverse.c

    sem_wait(&empty2); // wait for an empty slot
    pthread_mutex_lock(&mutex3);
    pthread_mutex_lock(&mutex2);
      // critical zone 2
      insertInBuffer(resRH, ProdCons2, N, true);
    pthread_mutex_unlock(&mutex2);
    sem_post(&full2); // a slot has been filled
  }

  consFinish++;
  pthread_mutex_unlock(&mutex3);
  //printf("End consumer, full: %d, empty: %d\n", getSemValue(&full),getSemValue(&empty));
  //printf("End consumer, full2: %d, empty2: %d\n", getSemValue(&full2),getSemValue(&empty2));

  free(hash);
  return NULL;
}

void * sort(){
  char * resRH = malloc(sizeof(char)*sizeofString);
  if(!resRH) {stringError("malloc resRH error");}
  while(sortCond())  //check if production of contion ended and if the buffer is empty
  {
    //printf("sortWhile\n");
    //printf("sort, full2:%d\n", getSemValue(&full2));
    sem_wait(&full2); // wait for a filled slot
    pthread_mutex_lock(&mutex2);
      // critical zone 2
      removeFromBuffer(resRH, ProdCons2, N, true);
      printf("mot: %s\n", resRH);
    pthread_mutex_unlock(&mutex2);
    sem_post(&empty2); // a slot has been cleaned

    if(head==NULL){
      push(&head, resRH); //malloc, arg
    }

    if(head==NULL){
      //printf("add first %s\n", resRH);
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

// if resRH == true => removeResRH ; else => removeHash
void removeFromBuffer(char* A, char *PC, int N, bool resRH){
  int counter=0;
  int sz = sizeofHash;
  if(resRH){sz = sizeofString;}

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

// if resRH == true => insertResRH ; else => insertHash
void insertInBuffer(char * A, char * PC, int N, bool resRH){
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

int saveToFile(struct node ** head, FILE * outputFile){
  struct node * first = *head;
  int wErr = 0;
  while(first){
    if(fprintf(outputFile, "%s\n", first->name) < 0){
      wErr++;
    }
    first = (first->next);
  }
  if (wErr > 0) {
    printf("Number of writing errors : %d\n", wErr);
    return wErr;
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
