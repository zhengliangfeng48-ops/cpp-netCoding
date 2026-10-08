#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<arpa/inet.h>

#define SERVER_ADDR "47.109.28.201"
#define SERVER_PORT 8888

int main(){
	int socketFd=socket(AF_INET,SOCK_STREAM,0);

	struct sockaddr_in serverAddr;
	serverAddr.sin_family=AF_INET;
	serverAddr.sin_port=htons(SERVER_PORT);
	inet_pton(AF_INET,SERVER_ADDR,&serverAddr.sin_addr.s_addr);

	connect(socketFd,(struct sockaddr*)&serverAddr,sizeof(serverAddr));

	while(1){
		char inputStr[1024];
		int ret=read(STDIN_FILENO,&inputStr,1024);
		write(socketFd,inputStr,ret);

		char outputStr[1024];
		read(socketFd,&outputStr,1024);

		printf("%s --> %s\n",inputStr,outputStr);
	}

	close(socketFd);
}
