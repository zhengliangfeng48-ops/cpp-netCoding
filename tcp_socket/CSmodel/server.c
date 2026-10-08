#include "wrap.h"

#define SERV_PORT 9527

int main(){
	int lfd=Socket(AF_INET,SOCK_STREAM,0);

	struct sockaddr_in serv_addr;
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(SERV_PORT);
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);

	int ret=Bind(lfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr));

	ret=Listen(lfd,128);

	struct sockaddr_in clit_addr;
	socklen_t clit_addr_len=sizeof(clit_addr);

	int cfd=Accept(lfd,(struct sockaddr*)&clit_addr,&clit_addr_len);

	char client_IP[1024];

	printf("client ip:%s port:%d\n",
			inet_ntop(AF_INET,&clit_addr.sin_addr.s_addr,client_IP,sizeof(client_IP)),
			ntohs(clit_addr.sin_port));

	while(true){
		char buf[BUFSIZ];
		ret=read(cfd,buf,sizeof(buf));
		write(STDOUT_FILENO,buf,ret);

		for(int i=0;i<ret;++i){
			buf[i]=toupper(buf[i]);
		}

		write(cfd,buf,ret);
	}

	close(cfd);
	close(lfd);
}
