#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>

// bev对象的读写回调
void read_cb(struct bufferevent *bev, void *arg)
{
	char buf[1024];

	// 从客户端中读
	bufferevent_read(bev, buf, sizeof(buf));
	printf("client say:%s\n", buf);

	char *p = "我是服务器，已经成功收到你发送的数据";
	// 写数据给客户端
	bufferevent_write(bev, p, strlen(p) + 1);

	sleep(1);
}
void write_cb(struct bufferevent *bev, void *arg)
{
	printf("I'm 服务器,成功写数据给客户端了,写缓冲区回调函数被调用...\n");
}


// listener监听器的回调函数
void listener_cb(struct evconnlistener *listener,
		evutil_socket_t fd,
		struct sockaddr *addr,
		int len, void *ptr)
{
	printf("connect new client");

	// 接收监听器传进来的base
	struct event_base *base = (struct event_base *)ptr;

	// 创建bev对象,用于监听客户端的读写事件
	struct bufferevent *bev;
	bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);

	// 设置bev对象的读写回调
	bufferevent_setcb(bev, read_cb, write_cb, NULL, NULL);

	// 设置读的权限
	bufferevent_enable(bev, EV_READ);
}

int main()
{
	// 地址结构体
	struct sockaddr_in serve_addr;
	serve_addr.sin_family = AF_INET;
	serve_addr.sin_port = htons(9527);
	serve_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	// 1. 创建底座
	struct event_base *base = event_base_new();
	// 2. 创建listener监听对象,(监听是否有客户端连接)
	struct evconnlistener *listener;
	listener = evconnlistener_new_bind(base, listener_cb, base,
			LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
			-1, (struct sockaddr *)&serve_addr, sizeof(serve_addr));
	// 3. 启动循环监听
	event_base_dispatch(base);

	// 4. 释放
	event_base_free(base);         // 底座
	evconnlistener_free(listener); // 监听器

	return 0;
}
