#include <stdio.h> 

int main( int argc, char*argv[]){
    if(!strcmp(argv[2],"-eq")){
    	if(atoi(argv[1]) == atoi(argv[3])){
		printf("printf marche!\n");
		return 0;}
    	fprintf(stdout,"fprintf marche!\n");
	return 1;}
    if(!strcmp(argv[2],"-ge")){
    	if(atoi(argv[1]) >= atoi(argv[3])){return 0;}
    	return 1;}       
    if(!strcmp(argv[2],"-gt")){
    	if(atoi(argv[1]) > atoi(argv[3])){return 0;}
    	return 1;}   
	if(!strcmp(argv[2],"-le")){
    	if(atoi(argv[1]) <= atoi(argv[3])){return 0;}
    	return 1;}  
    if(!strcmp(argv[2],"-lt")){
    	if(atoi(argv[1]) < atoi(argv[3])){return 0;}
    	return 1;}  
    if(!strcmp(argv[2],"-ne")){
    	if(atoi(argv[1]) != atoi(argv[3])){return 0;}
    	return 1;}  
       
}
