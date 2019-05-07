#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

int sizeofHash = 32;
int sizeofString = 17; // 16 + '\0'

void insertHash(uint8_t* hash, uint8_t *ProdCons, int N);
void insert(uint8_t * A, uint8_t * PC, int N, bool resRH);
void removeHash(uint8_t* hash, uint8_t *ProdCons, int N);

int main() {
  int N=10;

	uint8_t * ProdCons = (uint8_t *) calloc(N, sizeof(uint8_t)*32);
	if(!ProdCons){
		printf("calloc ProdCons fail\n");
		return -1;
	}

	uint8_t * hash = malloc(sizeof(char)*32);
	if(!hash){
		free(hash);
		printf("malloc fail\n");
		return -1;
	}

	//hash1
	for(int i=0; i<32; i++){
	    *(hash+i)=i;
	}
	insert(hash, ProdCons, N, false);
	printf("add1:\n");
	printf("position 1: ");
	for(int i=0; i<32; i++){
	    printf("%d ",*(ProdCons+i));
	}
	printf("\n");
	printf("position 2: ");
	for(int i=0; i<32; i++){
	    printf("%d ",*(ProdCons+32+i));
	}
	printf("\n");



	//hash2
	printf("add2:\n");
	printf("position 1: ");
	for(int i=0; i<32; i++){
	    *(hash+i)=i*2;
	}
	for(int i=0; i<32; i++){
	    printf("%d ",*(ProdCons+i));
	}
	printf("\n");
	printf("position 2: ");
	insert(hash, ProdCons, N, false);
	for(int i=0; i<32; i++){
	    printf("%d ",*(ProdCons+32+i));
	}
	printf("\n");

	//remove hash1
	for(int i=0; i<32; i++){
	    *(hash+i)=i*2;
	}
	removeHash(hash, ProdCons, N);
	printf("remove:\n");
	printf("position 1: ");
	for(int i=0; i<32; i++){
	    printf("%d ",*(ProdCons+i));
	}
	printf("\n");

	printf("position 2: ");
	for(int i=0; i<32; i++){
	    printf("%d ",*(ProdCons+32+i));
	}
	printf("\n");
	printf("last remove hash: ");
	for(int i=0; i<32; i++){
	    printf("%d ",*(hash+i));
	}
	printf("\n");

	//remove hash2
	for(int i=0; i<32; i++){
	    *(hash+i)=i*2;
	}
	removeHash(hash, ProdCons, N);
	printf("remove:\n");
	printf("position 1: ");
	for(int i=0; i<32; i++){
	    printf("%d ",*(ProdCons+i));
	}
	printf("\n");

	printf("position 2: ");
	for(int i=0; i<32; i++){
	    printf("%d ",*(ProdCons+32+i));
	}
	printf("\n");
	printf("last remove hash: ");
	for(int i=0; i<32; i++){
	    printf("%d ",*(hash+i));
	}
	printf("\n");
	return 0;
}

void insertHash(uint8_t* hash, uint8_t *ProdCons, int N){
	int counter;
	for(int i=0; i<N; i++){
		counter=0;
		//check if the 32 byte equal zero
		for(int j=0; j<32; j++){
			counter = counter + *(ProdCons+i*32+j);
		}
		if (!counter){
			for(int j=0; j<32; j++){
				*(ProdCons+i*32+j)=*(hash+j);
			}
			return;
		}
	}
}

// if resRH == true => insertResRH ; else => insertHash
void insert(uint8_t * A, uint8_t * PC, int N, bool resRH){
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

void removeHash(uint8_t* hash, uint8_t *ProdCons, int N){
	//ne pas oublie de remettre la valuer à zéro dans le tableau après de l'avoir extraite
	int counter=0;
	for(int i=0; i<N & !counter; i++){
		counter=0;
		//check if the 32 byte equal zero
		for(int j=0; j<32; j++){
			counter = counter + *(ProdCons+i*32+j);
		}
		if (counter){
			for(int j=0; j<32; j++){
			    *(hash+j)=*(ProdCons+i*32+j);
			    *(ProdCons+i*32+j)=0;
			}
		}
	}
	return;
}