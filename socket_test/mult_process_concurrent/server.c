#include "wrap.h"

#define SRV_PORT 9999

void catch_child(int signum){
	while((waitpid(0,NULL,WNOHANG))>0);
	return;
}

int main(){
	int lfd=Socket(AF_INET,SOCK_STREAM,0);

	struct sockaddr_in srv_addr;

	//memset(&srv_addr,0,sizeof(srv_addr));
	bzero(&srv_addr,sizeof(srv_addr));

	srv_addr.sin_family=AF_INET;
	srv_addr.sin_port=htons(SRV_PORT);
	srv_addr.sin_addr.s_addr=htonl(INADDR_ANY);

	Bind(lfd,(struct sockaddr*)&srv_addr,sizeof(srv_addr));

	Listen(lfd,128);

	struct sockaddr_in clt_addr;
	socklen_t clt_addr_len=sizeof(clt_addr);

	pid_t pid;
	int cfd;
	while(true){
		cfd=Accept(lfd,(struct sockaddr*)&clt_addr,&clt_addr_len);

		pid=fork();
		if(pid<0)
			perr_exit("fork error");

		if(pid==0){
			close(lfd);
			break;	
		}
		close(cfd);

		struct sigaction act;
		act.sa_handler=catch_child;
		sigemptyset(&act.sa_mask);
		act.sa_flags=0;
		int ret=sigaction(SIGCHLD,&act,NULL);
		if(ret)
			perr_exit("sigaction error");
	}

	char buf[BUFSIZ];

	if(pid==0){
		while(true){
			int ret=Read(cfd,buf,sizeof(buf));
			if(ret==0){
				close(cfd);
				exit(1);
			}

			for(int i=0;i<ret;++i){
				buf[i]=toupper(buf[i]);
			}
			Write(cfd,buf,ret);
			Write(STDOUT_FILENO,buf,ret);
		}
	}
}
