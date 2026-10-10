#include "wrap.h"
#include<sys/select.h>

#define SRV_PORT 6666

int main(){
	int i,j,n,nready;
	int maxfd=0;
	int listenfd,connfd;
	char buf[BUFSIZ];

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

	fd_set rset,allset;			//定义读集合、备份集合
	maxfd=listenfd;

	FD_ZERO(&allset);			//清空监听集合
	FD_SET(listenfd,&allset);

	while(true){
		rset=allset;
		nready=select(maxfd+1,&rset,NULL,NULL,NULL);
		if(nready<0)
			perr_exit("select error");

		if(FD_ISSET(listenfd,&rset)){	//listen满足监听的读事件
			clie_addr_len=sizeof(clie_addr);
			connfd=Accept(listenfd,(struct sockaddr*)&clie_addr,&clie_addr_len);	//已经有连接请求，不会阻塞了

			FD_SET(connfd,&allset);

			if(connfd>maxfd)
				maxfd=connfd;

			if(nready==1)		//只有listenfd
				continue;
		}

		for(i=listenfd+1;i<=maxfd;++i){
			if(FD_ISSET(i,&rset)){
				if((n=Read(i,buf,sizeof(buf)))==0){		//客户端关闭连接
					close(i);
					FD_CLR(i,&allset);
				}else if(n>0){
					for(j=0;j<n;++j)
						buf[j]=toupper(buf[j]);

					Write(i,buf,n);
				}
			}
		}
	}
}
