#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <arpa/inet.h>

// 读写回调
void read_cb(struct bufferevent *bev, void *arg)
{
	char buf[1024] = {0};
	bufferevent_read(bev, buf, sizeof(buf));

	printf("服务器 say:%s\n", buf);
	bufferevent_write(bev, buf, strlen(buf) + 1);
	sleep(1);
}

void write_cb(struct bufferevent *bev, void *arg)
{
	printf("----------我是客户端的写回调函数,没卵用\n");
}

void event_cb(struct bufferevent *bev, short events, void *arg)
{
	if (events & BEV_EVENT_EOF)
	{
		printf("connection closed\n");
	}
	else if (events & BEV_EVENT_ERROR)
	{
		printf("some other error\n");
	}
	else if (events & BEV_EVENT_CONNECTED)
	{
		printf("已经连接服务器...\\(^o^)/...\n");
		return;
	}
	//  释放资源  
	bufferevent_free(bev);
}

int main()
{
	// 1. 创建base底座
	struct event_base *base = event_base_new();

	// 描述符
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	// 地址结构
	struct sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(client_addr));
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(9527);
	inet_pton(AF_INET, "127.0.0.1", &client_addr.sin_addr.s_addr);

	// 2. 创建bev对象
	struct bufferevent *bev = NULL;
	bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);

	// 3. 连接服务器
	bufferevent_socket_connect(bev, (struct sockaddr *)&client_addr, sizeof(client_addr));

	// 4. 设置回调
	bufferevent_setcb(bev, read_cb, write_cb, event_cb, NULL);

	// 设置读回调生效
	bufferevent_enable(bev, EV_READ);

	// 5. 启动循环监听
	event_base_dispatch(base);

	// 6. 销毁
	event_base_free(base);
	bufferevent_free(bev);

	return 0;
}
