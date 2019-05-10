#include "functions.c"


int main(int argc, char *argv[]){
    //set a timer
    //time_t start =time(0);

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
          outFile = fopen(optarg, "w");
          if(outFile==NULL){
            intError(0,"error with opening the output file");
          }
          break;
        case '?':
          stringError("error in the argument");
          break;
      }
    }

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


  //START OF CODE

  pthread_t prod;
  pthread_t cons [N];//thread pour reverseHash
  pthread_t cons2;

  //creation of threads
  struct arg* ARG = (struct arg*) malloc(sizeof(int)*2+sizeof(char**)) ;
  if(ARG==NULL){stringError("malloc ARG error");}
  numberOfFiles=argc - optind;
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
  }
  err = pthread_create(&cons2, NULL, &sort, NULL);
  if(err!=0){intError(err, "pthread_create of sort");}

  //join of threads
  err = pthread_join(prod,NULL);
  if(err!=0){intError(err, "pthread_join of producer");}
  for(int i=0; i<N; i++){
    err = pthread_join(cons[i],NULL);
    if(err!=0){intError(err, "pthread_join of consumer");}
  }

  err= pthread_join(cons2,NULL);
  if(err!=0){intError(err, "pthread_join of sort");}


  if (OutputToFile) {
    saveToFile(&head, outFile);
    err = fclose(outFile);
    if(err!=0){
      pop(&head);
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


    //time_t end =time(0);
    //double TheTime = difftime(end, start);
    //printf("program end in %f second\n",TheTime);

  return (EXIT_SUCCESS);
}
