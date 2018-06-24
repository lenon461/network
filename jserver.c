/* 
   A simple server in the internet domain using TCP
   Usage:./server port (E.g. ./server 10000 )
*/
#include <stdio.h>
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <unistd.h>

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd; //descriptors rturn from socket and accept system calls
     int portno; // port number
     socklen_t clilen;
     
     char resq[256];
     
     /*sockaddr_in: Structure Containing an Internet Address*/
     struct sockaddr_in serv_addr, cli_addr;
     
     int n;
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     
     /*Create a new socket
       AF_INET: Address Domain is Internet 
       SOCK_STREAM: Socket Type is STREAM Socket */
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]); //atoi converts from String to Integer
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY; //for the server the IP address is always the address that the server is running on
     serv_addr.sin_port = htons(portno); //convert from host to network byte order
     
     if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) //Bind the socket to the server address
              error("ERROR on binding");
     
     listen(sockfd,5); // Listen for socket connections. Backlog queue (connections to wait) is 5
     
     clilen = sizeof(cli_addr);
     /*accept function: 
       1) Block until a new connection is established
       2) the new socket descriptor will be used for subsequent communication with the newly connected client.
     */
     newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
     if (newsockfd < 0) 
          error("ERROR on accept");
     
     bzero(resq,256);
     n = read(newsockfd,resq,255); //Read is a block function. It will read at most 255 bytes
     if (n < 0) error("ERROR reading from socket");
     //   printf("Here is the message: %s\n",buffer);
     
  //   n = write(newsockfd,"I got your message",18); //NOTE: write function returns the number of bytes actually sent out Ñ> this might be less than the number you told it to send
     if (n < 0) error("ERROR writing to socket");
		
	printf("resqust : %s\n",resq);
	
	char *p;
	p = strtok(resq, " ");

	if(strcmp(p,"GET") && strcmp(p,"get"))
		error("Error there is no get method");
	p = strtok(NULL, " ");
	
	printf("p : %s\n",p);
	
	 
	char uri[100], c_type[20];
	if(!strcmp(p,"/"))
		sprintf(uri, "/index.html");
	else
		sprintf(uri,"%s",  p);
	
	char uri2[100];
	for(int u = 0; u < strlen(uri); u++)
	{
		uri2[u] = uri[u+1];
	}
	
	strcpy(c_type, "text/plain");
	
	printf("uri2 : %s c_type : %s\n",uri2,c_type);
	struct {
		char *ext;
		char *filetype;
	} extensions[] = {
		{"html","text/html"},
		{"gif","image/gif"},
		{"jpg","image/jpeg"},
		{"jpeg","image/jpeg"},
		{"mp3","audio/mpeg"},
		{"pdf","application/pdf"},
		{0,0}
	};
	int len;

//	printf("%s %s\n",uri,c_type);
	
	for(int i = 0; extensions[i].ext != 0; i++){
		len = strlen(extensions[i].ext);
		if( !strncmp(uri + strlen(uri) - len, extensions[i].ext, len)){
			strcpy(c_type, extensions[i].filetype);
			break;
		}
	}

	printf("uri2 : %s c_type : %s\n",uri2,c_type);
		

	FILE *fd;
//	int fd_size, code = 200;
	//char pharse[20] = "OK";
	
	
/*	   if((fd = fopen(uri,"rb")) < 0) {
		code = 404;
		printf("ERORORORO");
		strcpy(pharse, "FILE NOT FOUND\n");
	}
	//else {
*/
	p = strtok(NULL,"\r\n ");
	printf("uri2 : %s p: %s\n",uri2,p);
	printf("------------------------------\n");
	
	 
	char header_head[256];

	size_t fsize = 0, fpsize = 0;
	
	char buf[256];
	memset(buf, 0x00, 256);

	if(	(fd = fopen(uri2,"rb")) == NULL){
		perror("file open error");
		exit(1);
	}
	fseek(fd,0,SEEK_END);	
	fsize = ftell(fd);
	fseek(fd,0,SEEK_SET);	

	sprintf(header_head,"HTTP/1.1 200 OK\nContent-Type: %s\nContent-Length: %lu\n\r\n", c_type,fsize);
	printf("%s",header_head);
	printf("--------------------------------------\n");	
	send(newsockfd, header_head, strlen(header_head), 0);
		
//	size_t fsize2 = htonl(fsize);
//	send(newsockfd, &fsize2, sizeof(fsize), 0); 
	int x = 1;	
	while(  1/*total < header_len + fsize+1*/   ) { 
		fpsize = fread(buf, 1, 256, fd); 
		if(x == 0 ) 
		{
			break;	
		}
		else{
		x =	send(newsockfd, buf, fpsize, 0);
		}
	}
	
	fclose(fd);
	close(sockfd);
    close(newsockfd);

     return 0; 
}
