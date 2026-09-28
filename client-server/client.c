#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<arpa/inet.h>

#define SERVER_ADDR "172.19.93.251"

int main(int argc,char* argv[]){
	int socketFd=socket(AF_INET,SOCK_STREAM,0);

	struct sockaddr_in *serverAddr=malloc(sizeof(struct sockaddr_in));
	serverAddr->sin_family=AF_INET;
	serverAddr->sin_port=htons(8888);

	int sAddr;
	inet_pton(AF_INET,SERVER_ADDR,(void*)&sAddr);
	serverAddr->sin_addr.s_addr=sAddr;

	connect(socketFd,serverAddr,sizeof(*serverAddr));

	char* inputStr=argv[1];
	write(socketFd,inputStr,sizeof(inputStr));

	char *outputStr;
	read(socketFd,outputStr,sizeof(inputStr));

	printf("%s --> %s\n",inputStr,outputStr);

	close(socketFd);
}
