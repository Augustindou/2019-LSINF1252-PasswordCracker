bool finishProd = 0;

﻿// Initialisation
//à modifier selon le nombre de thread
#define N 10 // slots du buffer
pthread_mutex_t mutex;
sem_t empty;
sem_t full;

pthread_mutex_init(&mutex, NULL);
sem_init(&empty, 0 , N);  // buffer vide
sem_init(&full, 0 , 0);   // buffer vide



// Producteur, Hash
void producer(void)
{
  int item; //plutot un tableau de 32 bytes
  while(!finishProd)
  {
    item=produce(item);	//readf() & mettre finishProd à 1 à la fin
    sem_wait(&empty); // attente d'un slot libre
    pthread_mutex_lock(&mutex);
     // section critique
     insert_item();	//ajout dans le tableau
    pthread_mutex_unlock(&mutex);
    sem_post(&full); // il y a un slot rempli en plus
  }
}

// Consommateur, reverseHash
void consumer(void)
{
 int item; //tableau de 32 byte (voir Producteur)
 while(!finishProd || getSemValue(&full) )	//check si la production est terminée & vérifie si le tableau est vide 
 {
   sem_wait(&full); // attente d'un slot rempli
   pthread_mutex_lock(&mutex);
    // section critique
    item=remove(item);
   pthread_mutex_unlock(&mutex);
   sem_post(&empty); // il y a un slot libre en plus
 }
}

int getSemValue(sem_t * sem)
{
	int value;
	sem_getvalue(&sem, &value);
	return value;
}
