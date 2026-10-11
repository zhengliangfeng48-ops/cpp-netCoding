#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>
#include<strings.h>
#include<unistd.h>
#include<pthread.h>
#include<errno.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/wait.h>
#include<signal.h>
#include<sys/epoll.h>
#include<fcntl.h>
#include<time.h>

#define MAX_EVENTS 1024
#define SERV_PORT 8000
#define BUFLEN 4096

void recvdata(int fd,int events,void *arg);
void senddata(int fd,int events,void *arg);

struct myevent_s{
	int fd;												//监听
	int events;											//对应事件
	void *arg;											//泛型参数
	void (*call_back)(int fd,int events,void *arg);		//回调函数
	bool status;										//是否在监听
	char buf[BUFLEN];
	int len;
	long last_active;									//记录每次加入红黑树 g_efd 的时间值
};

int g_efd;									//保存epoll_create返回的fd
struct myevent_s g_events[MAX_EVENTS+1];	

//初始化
void eventset(struct myevent_s *ev,int fd,void (*call_back)(int,int,void*),void* arg){
	ev->fd=fd;
	ev->call_back=call_back;
	ev->events=0;
	ev->arg=arg;
	ev->status=0;
	memset(ev->buf,0,sizeof(ev->buf));
	ev->len=0;
	ev->last_active=time(NULL);
}

void eventadd(int efd,int events,struct myevent_s *ev){
	struct epoll_event epv={0,{0}};
	int op;
	epv.data.ptr=ev;
	epv.events=ev->events=events;

	if(ev->status==0){
		op=EPOLL_CTL_ADD;
		ev->status=1;
	}
	
	if(epoll_ctl(efd,op,ev->fd,&epv)<0)
		printf("event add failed [fd=%d], events[%d]\n",ev->fd,events);
	else
		printf("event add OK [fd=%d], op=%d, events[%0X]\n",ev->fd,op,events);
}

void acceptconn(int lfd,int events,void *arg){
	struct sockaddr_in cin;			//客户端地址结构
	socklen_t len=sizeof(cin);
	int cfd,i;

	if((cfd=accept(lfd,(struct sockaddr*)&cin,&len))==-1){
		if(errno!=EAGAIN&&errno!=EINTR){

		}
		printf("%s: accpet, %s\n",__func__,strerror(errno));
		return;
	}

	do{
		for(i=0;i<MAX_EVENTS;++i)
			if(g_events[i].status==0)
				break;

		if(i==MAX_EVENTS){
			printf("%s max connect limit[%d]\n",__func__,strerror(errno));
			break;
		}

		int flag=0;
		if((flag=fcntl(cfd,F_SETFL,O_NONBLOCK))<0){
			printf("%s: fcntl nonblocking failed, %s\n",__func__,strerror(errno));
			break;
		}

		//给cfd设置一个myevent_s结构体，回调函数设置为recvdata
		eventset(&g_events[i],cfd,recvdata,&g_events[i]);
		eventadd(g_efd,EPOLLIN,&g_events[i]);
	}while(false);
}

void initlistensocket(int efd,short port){
	int lfd=socket(AF_INET,SOCK_STREAM,0);
	fcntl(lfd,F_SETFL,O_NONBLOCK);

	struct sockaddr_in sin;
	memset(&sin,0,sizeof(sin));
	sin.sin_family=AF_INET;
	sin.sin_addr.s_addr=INADDR_ANY;
	sin.sin_port=htons(port);

	bind(lfd,(struct sockaddr*)&sin,sizeof(sin));

	listen(lfd,20);

	eventset(&g_events[MAX_EVENTS],lfd,acceptconn,&g_events[MAX_EVENTS]);

	eventadd(efd,EPOLLIN,&g_events[MAX_EVENTS]);
}

void eventdel(int efd,struct myevent_s *ev){
	struct epoll_event epv={0,{0}};

	if(ev->status!=1)
		return;

	epv.data.ptr=NULL;
	ev->status=0;
	epoll_ctl(efd,EPOLL_CTL_DEL,ev->fd,&epv);
}

int main(int argc,char *argv[]){
	unsigned short port=SERV_PORT;

	if(argc==2)
		port=atoi(argv[1]);

	g_efd=epoll_create(MAX_EVENTS+1);
	if(g_efd<=0){
		perror("epoll_create error");
		exit(1);
	}

	initlistensocket(g_efd,port);

	struct epoll_event events[MAX_EVENTS+1];
	printf("server running:port[%d]\n",port);

	int checkpos=0,i;
	while(true){
		//超时验证
		long now=time(NULL);
		for(i=0;i<100;++i,++checkpos){
			if(checkpos==MAX_EVENTS)
				checkpos=0;
			if(g_events[checkpos].status!=1)
				continue;

			long duration=now-g_events[checkpos].last_active;
			if(duration>=60){
				close(g_events[checkpos].fd);
				printf("[fd=%d] timeout\n",g_events[checkpos].fd);
				eventdel(g_efd,&g_events[checkpos]);
			}
		}

		int nfd=epoll_wait(g_efd,events,MAX_EVENTS+1,1000);
		if(nfd<0){
			printf("epoll_wait error,exit\n");
			break;
		}

		for(i=0;i<nfd;++i){
			struct myevent_s *ev=(struct myevent_s *)events[i].data.ptr;

			if((events[i].events & EPOLLIN)&&(ev->events & EPOLLIN)){	//读就绪事件
				ev->call_back(ev->fd,events[i].events,ev->arg);
			}
			if((events[i].events & EPOLLOUT)&&(ev->events & EPOLLOUT)){	//写就绪事件
				ev->call_back(ev->fd,events[i].events,ev->arg);	
			}
		}
	}
}

void recvdata(int fd,int events,void *arg){
	struct myevent_s *ev=(struct myevent_s *)arg;
	int len=recv(fd,ev->buf,sizeof(ev->buf),0);

	eventdel(g_efd,ev);

	if(len>0){
		ev->len=len;
		ev->buf[len]='\0';
		printf("C[%d]:%s\n",fd,ev->buf);

		eventset(ev,fd,senddata,ev);
		eventadd(g_efd,EPOLLOUT,ev);
	}else if(len==0){
		close(ev->fd);
		printf("[fd=%d] pos[%ld], closed\n",fd,ev-g_events);
	}else{
		close(ev->fd);
		printf("recv[fd=%d] error[%d]:%s\n",fd,errno,strerror(errno));
	}
}

void senddata(int fd,int events,void *arg){
	struct myevent_s *ev=(struct myevent_s *)arg;
	int len=send(fd,ev->buf,ev->len,0);

	eventdel(g_efd,ev);

	if(len>0){
		printf("send[fd=%d], [%d]%s\n",fd,len,ev->buf);
		eventset(ev,fd,recvdata,ev);
		eventadd(g_efd,EPOLLIN,ev);
	}else{
		close(ev->fd);
		printf("send[fd=%d] error %s\n",fd,strerror(errno));
	}
}
