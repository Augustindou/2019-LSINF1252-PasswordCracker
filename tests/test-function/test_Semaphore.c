
//use for compiling: gcc -pthread -o test test.c 
#include <semaphore.h>
#include <stdio.h>
int main() {
    sem_t empty;
    sem_init(&empty, 0 , 10);
    sem_wait(&empty); //value to 9
	sem_wait(&empty); //value to 10
	int value;
	sem_getvalue(&empty, &value);
    printf("%d\n", value);//get 10
	return 0;
}