#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<arpa/inet.h>

#define SERVER_ADDR "0.0.0.0"
#define SERVER_PORT 8888

int main(){
	int socketFd=socket(AF_INET,SOCK_STREAM,0);

	struct sockaddr_in serverAddr;
	serverAddr.sin_family=AF_INET;
	serverAddr.sin_port=htons(SERVER_PORT);
	inet_pton(AF_INET,SERVER_ADDR,(void*)&serverAddr.sin_addr.s_addr);

	bind(socketFd,(struct sockaddr*)&serverAddr,sizeof(serverAddr));

	listen(socketFd,64);

	struct sockaddr_in *clientAddr=malloc(sizeof(struct sockaddr_in));

	int addrLen=sizeof(*clientAddr);
	int acceptFd=accept(socketFd,(struct sockaddr*)clientAddr,(socklen_t*)&addrLen);

	while(1){
		char inputStr[1024]="";
		read(acceptFd,&inputStr,1024);

		char outputStr[1024]="";
		for(int i=0;i<sizeof(inputStr)&&inputStr[i]!='\0';++i){
			if(inputStr[i]>='a'&&inputStr[i]<='z'){
				outputStr[i]=inputStr[i]+('A'-'a');
			}else{
				outputStr[i]=inputStr[i];
			}
		}

		printf(" Read: %s Transform to: %s \n",inputStr,outputStr);

		write(acceptFd,&outputStr,1024);
	}

	close(acceptFd);
	close(socketFd);
}
