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
// arg structure for valid argument in pthread_create
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
 * @return NULL
 */
void * producer(void * arg);
/**
 * Gets hashes from first buffer
 * Bruteforce hash => password
 * Adds passwords to second buffer
 *
 * @return NULL
 */
void * consumer();
/**
 * Gets passwords from second buffer
 * Checks if password is better than last ones
 * If better : frees the stack and add new password
 * If equal : adds password to stack
 * If worse : discards new password
 *
 * @return NULL
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
 */
void push(struct node **head, const char *value);
/**
 * Free all elements of the stack.
 *
 * @head : pointer to the top of the stack
 */
void pop(struct node ** head);
/**
 * Print all elements of the stack.
 *
 * @head : pointer to the top of the stack
 */
void printStack(struct node **head);
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
 * @file : pointer to input file
 * @hash : pointer to hash
 *
 * @return pointer to hash
 */
u_int8_t* readBinFile(FILE* file, u_int8_t * hash);

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

/*---------------------------------------------------------------------------*/
/*     Variables                                                             */
/*---------------------------------------------------------------------------*/
#define SIZE_OF_HASH 32
#define SIZE_OF_STRING 17

/* file for input files ; outFile for output file */
FILE* file;
FILE* outFile;

/* First buffer (before reverseHash)related mutex and semaphores */
pthread_mutex_t mutex;
sem_t empty;
sem_t full;
/* Second buffer (after reverseHash) and related mutex and semaphores */
char * ProdCons2;
pthread_mutex_t mutex2;
sem_t empty2;
sem_t full2;


/* Mutex to handle sort condition */
pthread_mutex_t mutex3;
/* Head of the password stack */
struct node * head;
/* Number of input files */
int numberOfFiles;
/* Error handling variable */
int err;
