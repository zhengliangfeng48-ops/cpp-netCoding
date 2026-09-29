#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<sys/socket.h>
#include<fcntl.h>
#include<unistd.h>
#include<arpa/inet.h>

#define SERV_PORT 9527

int main(){
	int cfd=socket(AF_INET,SOCK_STREAM,0);
	if(cfd==-1){
		perror("socket error");
		exit(1);
	}

	struct sockaddr_in serv_addr;
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(SERV_PORT);
	
	//inet_pton(AF_INET,"127.0.0.1",&serv_addr.sin_addr);
	inet_pton(AF_INET,"127.0.0.1",&serv_addr.sin_addr.s_addr);

	int ret=connect(cfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr));
	if(ret){
		perror("connect error");
		exit(1);
	}

	int counter=10;
	while(--counter){
		write(cfd,"hello\n",6);

		char buf[BUFSIZ];
		ret=read(cfd,buf,sizeof(buf));

		write(STDOUT_FILENO,buf,ret);
		sleep(1);
	}
	close(cfd);
}
