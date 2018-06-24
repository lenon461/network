#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <netdb.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include "mycache.h"

static int cur_cache_size = 0;
void error(char* msg)
{
		perror(msg);
			exit(1);
}
void run_server(int);
void parsing(char*,char*);
void proxy(int, char*, char*, linkedList*);
void logging(char*, char*, char*, int);
typedef struct {
	int fd;
	char *req_msg;
	char *url;
} proxy_args;

int main(int argc, char *argv[]){

	if (argc < 2) {
		printf("./proxy port");
		exit(1);
	}
	
	run_server(atoi(argv[1]));


	return 0;
}


void run_server(int port){

	int sockfd;
	int newsockfd;
	socklen_t clilen;

	char buffer[256];

//	pthread_t tid;
//	pthread_attr_t attr;

	struct sockaddr_in serv_addr, cli_addr;

	int n;

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY; 
	serv_addr.sin_port = htons(port); 
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0) error("ERROR opening socket");
	if((bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)) error("ERROR on binding");
	int val = 1;    
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &val, sizeof val) < 0) {
		error("ERROR setsockopt");
		close(sockfd);
	}
	
	listen(sockfd, 5);

	//int port;
	char url[100];
	linkedList *L = (linkedList *)malloc(sizeof(linkedList));
	L->cur = NULL;
	L->head = NULL;
	L->tail = NULL;
	
	
	
	while(1){
		printf("------------runserver-----------\n");

		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if(newsockfd < 0) error("ERROR on accept");

		n = read(newsockfd, buffer, 255); //Read is a block function. It will read at most 255 bytes
		if (n < 0) error("ERROR reading from socket");
	
		parsing(buffer, url);
		//printf("--\n%s\n",buffer);
		//printf("--\n%s\n",url);

		printf("1-------------------\n");
		

		printf("2-------------------\n");
//		pthread_create(&tid, &attr, (void*)proxy, (void *)&args);


		proxy(newsockfd, url, buffer, L);
		printf("------------end__server-----------\n");
		//close(newsockfd);
	//	close(sockfd);


	}
}

void parsing(char* resq, char*url){

	
	char buffer[256];
	char *p;
	
	char s1[100], s2[100], s3[100];
	char path[100];
	int port = 0, i;

	//printf("@@@%s\n",resq);
	
	sscanf(resq, "%s %s %s", s1, s2, s3);
	if ((strncmp(s1, "GET", 3) == 0) && (strncmp(s2, "http://", 7) == 0) && ((strncmp(s3, "HTTP/1.1", 8) == 0) || (strncmp(s3, "HTTP/1.0", 8) == 0))) {
	
		for(i = 7; i < strlen(s2); i++)
		{
			if(s2[i]==':')
			{
				port=1;
				break;
			}
		}

		p = strtok(s2, "//");
		
		if(port == 0){
			p = strtok(NULL, "/");
			strcpy(url,p);
		}
		else{
			p = strtok(NULL, ":");
			strcpy(url,p+1);
			p = strtok(NULL, "/");
			port = atoi(p);
		}
		
		p = strtok(NULL, " ");


		if(p != NULL){
			strcpy(path, p);
			sprintf(buffer, "GET /%s %s\r\nHost: %s\r\nConnection: close\r\n\r\n", path, s3, url);
		}
		else
			sprintf(buffer, "GET / %s\r\nHost: %s\r\nConnection: close\r\n\r\n", s3, url);
		
		//printf("%s\n",buffer);
		//printf("PORT : %d\n",port);


	}



	strcpy(resq, buffer);
}

void proxy(int newsockfd, char* url, char* buffer, linkedList *L)
{
	int n;

	char cache[MAX_OBJECT_SIZE];
	bzero((char*)cache, MAX_OBJECT_SIZE);

	//findNode(L,url);
	if(findNode(L, url) != NULL){ //hit
		printf("CACHE HIT!!\n");
		n = send(newsockfd, findNode(L,url), MAX_OBJECT_SIZE, 0); 
		printNodes(L);
		
		//printf("-----CACHE---\n%s\n-------CACHE-----\n",findNode(L,url));
		return;
	}




	printf("3-------------------\n");
	struct sockaddr_in serv_addr;
	struct hostent *server;
	int portno = 80;
	int datasock;
	datasock = socket(AF_INET, SOCK_STREAM, 0);
	if (datasock < 0)
		error("ERROR opening socket");
	server = gethostbyname(url); 
	printf("4-------------------\n");
	if (server == NULL) 
	{
		fprintf(stderr, "ERROR, no such host\n");
		exit(0);
	}

	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET; //initialize server's address
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);

	if (connect(datasock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR connecting");
	
	//printf("%s\n",buffer);
	
	printf("5-------------------\n");
	printf("6-------------------\n");
	

	n = send(datasock, buffer, strlen(buffer), 0);
	if (n < 0)
		error("Error writing to socket");

	printf("7-------------------\n");
	
	int cont = 0, size = 0;
	char log[10000];
	char ip1[100];
	
	strcpy(ip1,inet_ntoa(serv_addr.sin_addr));
	
	char newbuf[1500];
	bzero((char*)newbuf, 1500);
	
	while((n = recv(datasock, newbuf, sizeof(newbuf), 0)) > 0)
	{
		if(cont == 0){
			strcpy(log,newbuf);
			cont = 1;
		}

		size = size + n;

		printf("-----ORIG--%d-------------\n",n);
		send(newsockfd, newbuf, n, 0);
		if(size < MAX_OBJECT_SIZE) strcat(cache, newbuf);
		else bzero((char*)cache, MAX_OBJECT_SIZE);
	}
	printf("8-------------------\n");
	
	if(size < MAX_OBJECT_SIZE)
		createNode(L, cache, url, size);
	printf("total send size is : %d\n",size);

	cur_cache_size = cur_cache_size + size;
	printf("cur_cache_size : %d, add by %d\n",cur_cache_size, size);
	
	if(cur_cache_size > MAX_CACHE_SIZE) {
		printf("CACHE SIZE IS TOO BIG");
		deleteLastNode(L);
	}

	printNodes(L);
	printf("9-------------------\n");
	logging(log, url, ip1, size);

	//pthread_exit(NULL);

	close(datasock);
	close(newsockfd);
}

void logging(char* date, char* url, char* ip, int size){

	char *p, *q;
	p = strtok(date, "\r\n");
	while ((p = strtok(NULL, "\r\n")) != NULL) {
		if ((q = strstr(p, "Date: ")) != NULL) {
			strcpy(date, q + 6);									
		}
	}
	int fd;
	char log[200];
	sprintf(log, "%s: %s %s %d\n", date, ip, url, size);

	if ((fd = open("proxy.log", O_WRONLY | O_CREAT, 0644)) < 0) error("open error");

	if (lseek(fd, 0, SEEK_END) < 0) error("lseek error");

	if (write(fd, log, strlen(log)) < 0) error("write error");

	close(fd);
}



