#include "wrap.h"

void perr_exit(const char* s){
	perror(s);
	exit(1);
}

int Socket(int domain,int type,int protocol){
	int ret=socket(domain,type,protocol);
	if(ret==-1)
		perr_exit("socket error");
	return ret;
}

int Accept(int fd,struct sockaddr *sa,socklen_t *salenptr){
	int n=0;
again:
	n=accept(fd,sa,salenptr);
	if(n<0){
		if((errno==ECONNABORTED)||(errno==EINTR))
			goto again;
		else 
			perr_exit("accept error");
	}
	return n;
}

int Bind(int fd,const struct sockaddr *sa,socklen_t salen){
	int n=bind(fd,sa,salen);
	if(n<0)
		perr_exit("bind error");
	return n;
}

int Connect(int fd,const struct sockaddr *sa,socklen_t salen){
	int n=connect(fd,sa,salen);
	if(n<0)
		perr_exit("connect error");
	return n;
}

int Listen(int sockfd,int backlog){
	int ret=listen(sockfd,backlog);
	if(ret)
		perr_exit("listen error");
	return 0;
}


ssize_t Read(int fd,void* buf,size_t count){
	int ret=read(fd,buf,count);
	if(ret==-1)
		perr_exit("read error");
	return ret;
}

ssize_t Write(int fd,const void* buf,size_t count){
	int ret=write(fd,buf,count);
	if(ret==-1)
		perr_exit("write error");
	return ret;
}

