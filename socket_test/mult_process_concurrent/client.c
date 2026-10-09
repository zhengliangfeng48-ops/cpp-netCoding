#include "wrap.h"

#define SERV_PORT 9999

int main(){
	int cfd=socket(AF_INET,SOCK_STREAM,0);

	struct sockaddr_in serv_addr;
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(SERV_PORT);
	
	//inet_pton(AF_INET,"127.0.0.1",&serv_addr.sin_addr);
	inet_pton(AF_INET,"127.0.0.1",&serv_addr.sin_addr.s_addr);

	int ret=Connect(cfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr));

	while(true){
		char inputStr[BUFSIZ];
		scanf("%s",inputStr);
		int len;
		for(len=0;inputStr[len]!=0;++len);
		inputStr[len++]='\n';
		write(cfd,inputStr,len);

		char buf[BUFSIZ];
		ret=read(cfd,buf,sizeof(buf));

		write(STDOUT_FILENO,buf,ret);
		sleep(1);
	}
	close(cfd);
}
