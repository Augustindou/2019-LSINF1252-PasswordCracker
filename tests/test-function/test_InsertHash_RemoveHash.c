#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    int N=10;
    
	u_int8_t * ProdCons = (u_int8_t *) calloc(N, sizeof(u_int8_t)*32);
	if(ProdCons==NULL){
		printf("calloc ProdCons fail\n");
		return -1;
	}
	
	u_int8_t * hash = malloc(sizeof(char)*32);
	if(!hash){
		free(hash);
		printf("malloc fail\n");
		return -1;
	}
	
	//hash1
	for(int i=0; i<32; i++){
	    *(hash+i)=i;
	}
	insertHash(hash, ProdCons, N);
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
	insertHash(hash, ProdCons, N);
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

void insertHash(u_int8_t* hash, u_int8_t *ProdCons, int N){
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

void removeHash(u_int8_t* hash, u_int8_t *ProdCons, int N){
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