#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<arpa/inet.h>

#define SERVER_ADDR "172.19.93.251"

int main(){
	int socketFd=socket(AF_INET,SOCK_STREAM,0);

	struct sockaddr_in serverAddr;
	serverAddr.sin_family=AF_INET;
	serverAddr.sin_port=htons(8888);

	int sAddr;
	inet_pton(AF_INET,SERVER_ADDR,(void*)&sAddr);
	serverAddr.sin_addr.s_addr=sAddr;

	bind(socketFd,(struct sockaddr*)&serverAddr,sizeof(serverAddr));

	listen(socketFd,5);

	struct sockaddr_in *clientAddr=malloc(sizeof(struct sockaddr_in));

	while(1){
		int acceptFd=accept(socketFd,(struct sockaddr*)clientAddr,sizeof(*clientAddr));

		char inputStr[1024];
		read(acceptFd,&inputStr,1024);

		char outputStr[1024];
		for(int i=0;i<sizeof(outputStr);++i)
			outputStr[i]=inputStr[i]+('A'-'a');

		printf("Read: %s, transform to: %s \n",inputStr,outputStr);

		write(acceptFd,&outputStr,1024);

		close(acceptFd);
	}

	close(socketFd);
}
