#include "wrap.h"
#include<sys/epoll.h>

#define MAXLINE 8192
#define SRV_PORT 8000
#define OPEN_MAX 5000

int main(){
	int i,j,listenfd,connfd,sockfd;
	int n,num=0;
	ssize_t nready,efd,res;
	char buf[MAXLINE],str[INET_ADDRSTRLEN];

	struct sockaddr_in clie_addr,serv_addr;
	socklen_t clie_addr_len;

	struct epoll_event tep,ep[OPEN_MAX];

	listenfd=Socket(AF_INET,SOCK_STREAM,0);

	int opt=1;
	setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(void*)&opt,sizeof(opt));

	bzero(&serv_addr,sizeof(serv_addr));

	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(SRV_PORT);
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);

	Bind(listenfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr));
	Listen(listenfd,20);

	efd=epoll_create(OPEN_MAX);
	if(efd==-1)
		perr_exit("epoll_create error");

	tep.events=EPOLLIN;
	tep.data.fd=listenfd;

	res=epoll_ctl(efd,EPOLL_CTL_ADD,listenfd,&tep);
	if(res==-1)
		perr_exit("epoll_ctl error");

	while(true){
		nready=epoll_wait(efd,ep,OPEN_MAX,-1);
		if(nready<0)
			perr_exit("epoll error");

		for(i=0;i<nready;++i){
			if(!(ep[i].events & EPOLLIN))
				continue;

			if(ep[i].data.fd==listenfd){
				clie_addr_len=sizeof(clie_addr);
				connfd=Accept(listenfd,(struct sockaddr*)&clie_addr,&clie_addr_len);	//已经有连接请求，不会阻塞了

				printf("received from %s at PORT %d\n",
						inet_ntop(AF_INET,&clie_addr.sin_addr,str,sizeof(str)),
						ntohs(clie_addr.sin_port));
				printf("cfd %d---client %d\n",connfd,++num);

				tep.events=EPOLLIN;
				tep.data.fd=connfd;

				res=epoll_ctl(efd,EPOLL_CTL_ADD,connfd,&tep);
				if(res==-1)
					perr_exit("epoll_ctl error");
			}else{
				sockfd=ep[i].data.fd;
				n=Read(sockfd,buf,MAXLINE);
				if(n==0){			//客户端关闭连接
					res=epoll_ctl(efd,EPOLL_CTL_DEL,sockfd,NULL);
					if(res==-1)
						perror("epoll_ctl error");

					close(sockfd);
					printf("client[%d] closed connection\n",i);
				}else if(n<0){
					perror("read n<0 error: ");
					res=epoll_ctl(efd,EPOLL_CTL_DEL,sockfd,NULL);
					close(sockfd);
				}else{
					for(j=0;j<n;++j)
						buf[j]=toupper(buf[j]);

					Write(sockfd,buf,n);
					Write(STDOUT_FILENO,buf,n);
				}
			}
		}
	}
	close(listenfd);
}
