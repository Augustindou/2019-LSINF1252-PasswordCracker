#include "v3.h"

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
    if(!err){stringError("no inverse found to hash");} //need to refer to reverse.c

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
      printf("mot: %s\n", resRH);
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
