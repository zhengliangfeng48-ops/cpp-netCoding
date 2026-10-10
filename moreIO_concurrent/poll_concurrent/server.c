#include "wrap.h"
#include<poll.h>

#define MAXLINE 80
#define SRV_PORT 8000
#define OPEN_MAX 1024

int main(){
	int i,j,maxi;
	int listenfd,connfd,sockfd;
	int nready;
	ssize_t n;
	char buf[BUFSIZ],str[INET_ADDRSTRLEN];

	struct pollfd client[OPEN_MAX];

	struct sockaddr_in clie_addr,serv_addr;
	socklen_t clie_addr_len;

	listenfd=Socket(AF_INET,SOCK_STREAM,0);

	int opt=1;
	setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(void*)&opt,sizeof(opt));

	bzero(&serv_addr,sizeof(serv_addr));

	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(SRV_PORT);
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);

	Bind(listenfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr));
	Listen(listenfd,128);

	client[0].fd=listenfd;
	client[0].events=POLLIN;

	for(i=1;i<OPEN_MAX;++i)
		client[i].fd=-1;

	maxi=0;

	while(true){
		nready=poll(client,maxi+1,-1);

		if(nready<0)
			perr_exit("poll error");

		if(client[0].revents & POLLIN){		//listen满足监听的读事件
			clie_addr_len=sizeof(clie_addr);
			connfd=Accept(listenfd,(struct sockaddr*)&clie_addr,&clie_addr_len);	//已经有连接请求，不会阻塞了

			printf("received from %s at PORT %d\n",
					inet_ntop(AF_INET,&clie_addr.sin_addr,str,sizeof(str)),
					ntohs(clie_addr.sin_port));

			for(i=1;i<OPEN_MAX;++i){
				if(client[i].fd<0){
					client[i].fd=connfd;
					break;
				}
			}
			if(i==OPEN_MAX)
				perr_exit("too many clients");

			client[i].events=POLLIN;

			if(i>maxi)
				maxi=i;

			if(--nready==0)		//只有listenfd
				continue;
		}

		for(i=1;i<=maxi;++i){
			if((sockfd=client[i].fd)<0)
				continue;

			if(client[i].revents & POLLIN){
				if((n=Read(sockfd,buf,MAXLINE))<0){
					if(errno==ECONNRESET){
						printf("client[%d] aborted connection\n",i);
						close(sockfd);
						client[i].fd=-1;
					}else{
						perr_exit("read error");
					}
				}else if(n==0){			//客户端关闭连接
					printf("client[%d] closed connection\n",i);
					close(sockfd);
					client[i].fd=-1;
				}else{
					for(j=0;j<n;++j)
						buf[j]=toupper(buf[j]);

					Write(sockfd,buf,n);
					Write(STDOUT_FILENO,buf,n);
				}
				if(--nready<=0)
					break;
			}
		}
	}
	close(listenfd);
}
