#include "wrap.h"

#define SERV_ADDR "serv.socket"
#define CLIE_ADDR "clie.socket"

int main(){
    int cfd,len;
    struct sockaddr_un servaddr,cliaddr;
    char buf[4096];

    cfd=Socket(AF_UNIX,SOCK_STREAM,0);

    //客户端初始化
    bzero(&cliaddr,sizeof(cliaddr));
    cliaddr.sun_family=AF_UNIX;
    strcpy(cliaddr.sun_path,CLIE_ADDR);

    len=offsetof(struct sockaddr_un,sun_path)+strlen(cliaddr.sun_path);

    unlink(CLIE_ADDR);
    Bind(cfd,(struct sockaddr*)&cliaddr,len);

    //服务端初始化

    bzero(&servaddr,sizeof(servaddr));
    servaddr.sun_family=AF_UNIX;
    strcpy(servaddr.sun_path,SERV_ADDR);

    len=offsetof(struct sockaddr_un,sun_path)+strlen(servaddr.sun_path);

    Connect(cfd,(struct sockaddr*)&servaddr,len);
    
    while(fgets(buf,sizeof(buf),stdin)){
        write(cfd,buf,strlen(buf));
        len=read(cfd,buf,sizeof(buf));
        write(STDOUT_FILENO,buf,len);
    }
    close(cfd);
}