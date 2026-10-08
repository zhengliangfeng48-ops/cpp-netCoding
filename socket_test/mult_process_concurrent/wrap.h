#ifndef _WRAP_H_
#define _WRAP_H_

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

void perr_exit(const char* s);

int Socket(int domain,int type,int protocol);
int Accept(int fd,struct sockaddr *sa,socklen_t *salenptr);
int Bind(int fd,const struct sockaddr *sa,socklen_t salen);
int Connect(int fd,const struct sockaddr *sa,socklen_t salen);
int Listen(int sockfd,int backlog);

ssize_t Read(int fd,void* buf,size_t count);
ssize_t Write(int fd,const void* buf,size_t count);

#endif
