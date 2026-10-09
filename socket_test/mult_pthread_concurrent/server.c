#include "wrap.h"

#define SRV_PORT 9999
#define MAX_PTHREAD_NUM 1024

int cntPthread=0;
pthread_t pthreadsId[MAX_PTHREAD_NUM];

void* catch_child(void* avg){
	while(true){
		for(int i=0;i<cntPthread;++i){
			pthread_join(pthreadsId[i],NULL);	
		}
	}
	return NULL;
}

void* sentenceToUpper(void* cfd){
	int clientFd=*(int*)cfd;
	char buf[BUFSIZ];

	while(true){
		int ret=Read(clientFd,buf,sizeof(buf));
		if(ret==0){
			close(clientFd);
			exit(1);
		}

		for(int i=0;i<ret;++i){
			buf[i]=toupper(buf[i]);
		}
		Write(clientFd,buf,ret);
		Write(STDOUT_FILENO,buf,ret);
	}	
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

	pthread_t join_pthreadId;
	pthread_create(&join_pthreadId,NULL,catch_child,NULL);

	int cfds[MAX_PTHREAD_NUM];
	pthread_t c_pthreadId;	

	while(true){
		cfds[cntPthread]=Accept(lfd,(struct sockaddr*)&clt_addr,&clt_addr_len);

		pthread_create(&c_pthreadId,NULL,sentenceToUpper,(void*)&cfds[cntPthread]);
		pthreadsId[cntPthread++]=c_pthreadId;
	}
}
