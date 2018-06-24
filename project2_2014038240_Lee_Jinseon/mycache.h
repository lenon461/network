#include <stdio.h> 
#include <stdlib.h> 
#define MAX_CACHE_SIZE 5242880 //5MB
#define MAX_OBJECT_SIZE 524288 //512KB

typedef struct __list {
    struct __node *cur;
    struct __node *head;
    struct __node *tail;
} linkedList;
 
typedef struct __node {    
    char* data;
	char* url;
	int size;
    struct __node *next;
} node;

void createNode(linkedList *L, char* data, char* url, int size) {
	//printf("Data---\n%s\n",data);

	//printf("Data---\n%d\n",strlen(data));
    node *newNode = (node *)malloc(sizeof(node));
	
//newNode->data = data;
    newNode->data = (char*)malloc(strlen(data));
	strcpy(newNode->data,data);//,strlen(data));
	
//newNode->url = url;
	newNode->url = (char*)malloc(strlen(url));
	strcpy(newNode->url,url);//,strlen(data));
    
//newNode->size = strlen(data);	
	newNode ->size = size;
	newNode->next = NULL;
    if(L->head == NULL && L->tail == NULL)
        L->head = L->tail = newNode;    
    else {
	node *p = L->head;
   	 while(p->next != NULL) p = p->next;
        p->next = newNode;
	//L->tail->next = newNode;
        //L->tail = newNode;
    }
	//printf("@HEAD URL : ##%s@\n",L->head->url);
	//printf("@NEW NODE URL : ##%s@\n",newNode->url);
	//printf("Data2@@2@2---\n%s\n",newNode->data);
	
}

void deleteLastNode(linkedList *L) {
    node *p = L->head;
    while(p->next->next != NULL) p = p->next;
    p->next = p->next->next;
    L->tail = p;
}
void changeheadNode(linkedList *L, node *hit, node *prev){
	if(L->head == hit) {
		printf("current head is hit\n");
		return;
	}
	//printf("head : %s////hit : %s/////prev : %s\n",L->head->url,hit->url,prev->url);
	//node *p = L->head;
	prev->next = hit->next;
	
	hit->next = L->head;
	L->head = hit;
	printf("head is changed\n");
	

}

char* findNode(linkedList *L, char* url){
	node *p = L->head;
	node *tmp = p;
	if(p != NULL)
	{	
		while(1){
			
			if(strcmp(p->url,url) == 0){
				
				changeheadNode(L,p,tmp);
				
				return p->data;
						}
			else{
				if(p->next != NULL){
					tmp = p;
					 p = p->next;
				}
				else return NULL;
				}
			}
	}
	return NULL;
	

	
}
void printNodes(linkedList *L) {
    node *p = L->head;    
    putchar('[');
    while(p != NULL) {
        printf("%s(%d), ", p->url, p->size);
        p = p->next;
    }
    putchar(']');
    putchar('\n');
}

