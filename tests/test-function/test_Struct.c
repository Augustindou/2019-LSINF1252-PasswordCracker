#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//structure
struct node {
    struct node *next;
    char *name;
};
int push(struct node **head, const char *value);
int pop(struct node **head);
int printStack(struct node **head);

struct node * head;



int main() {
    head=NULL;
	char* c="abba";
	char* d="acca";
	push(*head, c);
	printf("push of c works\n");
	push(*head, d);
	printf("push of d works\n");
	printStack(*head);
	printf("printStack works\n");
	pop(*head);
	printf("pull works\n");
	return 0;
}

int push(struct node **head, const char *value){
	if(value==NULL){return 1;}
	char * varC = (char*)malloc(strlen(value)+1);
	if (varC==NULL){return 1;}
	varC = strcpy(varC, value);
	struct node* newNode = (struct node*) malloc(sizeof(struct node*)+sizeof(char*)) ;
	if(newNode==NULL){return 1;}
	newNode->next = &head;//work also for *head==NULL
	newNode->name = varC;
	*head=newNode;
	return 0;
}

int pop(struct node **head){
	while(*head!=NULL){
		struct node * first = (struct node*)malloc(sizeof(struct node*));
		if(first==NULL){return 1;};
		first=*head;

		head=*(first->next);
		free(first->name);
		free(first);
		if(first==NULL && first->name==NULL){
			return 1;
		}
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