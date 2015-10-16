#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

#define MAX_EVENTS 		500
#define DEFAULT_PORT	12345

typedef struct _event {
	int fd;
	void (*event_handler)(int fd, int events, void *arg);
	int events;
	void *arg;
	int status; //1:in epoll wait list, 0 not in
	char buff[128];
	int len, s_offset;
	long last_active;
} event;

int epfd; 							//epoll例程
event event_list[MAX_EVENTS + 1]; 	//fd事件列表, 最后一个是用于服务器的fd

void event_set(event *ev, int fd, void (*event_handler)(int, int, void*), void *arg);
void event_add(int efd, int events, event *ev);
void event_del(int efd, event *ev);

void listen_socket(short port);
void accep_connent(int fd, int events, void *arg);
void send_data(int fd, int events, void *arg);
void recv_data(int fd, int events, void *arg);

int main(int argc, char **argv)
{
	unsigned short port = DEFAULT_PORT;

	if (argc == 2) {
		port = getLong(argv[1], 0, "port-num");
	}

	//创建epoll例程
	epfd = epoll_create(MAX_EVENTS);

	if (epfd <= 0) {
		errExit("[%s]:create epoll failed.epfd[%d].\n", __func__, epfd);
	}

	listen_socket(port);

	printf ("[%s]:server running: port[%d]\n", __func__, port);

	struct epoll_event events[MAX_EVENTS];
	int check_pos = 0;

	while(1)
	{
		int i;
		long now = time(NULL);

		/*
		 * 一个简单的超时检测
		 * 最好是采用libevent的定时器设计, 最小堆(堆顶事件)确定最小超时时间
		 *
		 */
		for (i = 0; i < 100; i++, check_pos++) {
			if (check_pos == MAX_EVENTS)
				check_pos = 0;

			if (event_list[check_pos].status != 1)
				continue;

			long duration = now - event_list[check_pos].last_active;
			if (duration >= 60) {
				close(event_list[check_pos].fd);
				printf("[fd=%d] timeout[%d--%d].\n", event_list[check_pos].fd, 
					event_list[check_pos].last_active, now);

				event_del(epfd, &event_list[check_pos]);
			}
		}

		/*
		 * 我们知道select和poll文件描述符集会在内核态和用户态相互拷贝.
		 * epoll比select和poll省略了内存拷贝的原因:
		 * 		int epoll_create(int size)时, 其操作早请一块内存, 存储
		 *   	fd相关信息, epoll_wait()调用时, epoll_event *events参数
		 *    	会使用mmap放进去.
		 */

		//等待事件发生
		int event_cnt = epoll_wait(epfd, events, MAX_EVENTS, 1000);

		if (event_cnt < 0) {
			errExit("[%s]:epoll_wait error\n", __LINE__);
		}

		for (i = 0; i < event_cnt; i++) {
			event *ev = (struct _event*)events[i].data.ptr;

			//read事件
			if ( (events[i].events & EPOLLIN) && (ev->events & EPOLLIN) ) {
				ev->event_handler(ev->fd, events[i].events, ev->arg);
			}
			//write事件
			if ( (events[i].events & EPOLLOUT) && (ev->events & EPOLLOUT) ) {
				ev->event_handler(ev->fd, events[i].events, ev->arg);
			}
		}
	}

	close(epfd);
	//关闭server的socket
	close(event_list[MAX_EVENTS].fd);

	return 0;
}

void event_set(event *ev, int fd, void (*event_handler)(int, int, void*), void *arg)
{
	ev->fd = fd;
	ev->event_handler = event_handler;
	ev->arg = arg;
	ev->status = 0;
	memset(ev->buff, 0, sizeof(ev->buff));
	ev->s_offset = 0;
	ev->len = 0;
	ev->last_active = time(NULL);
}

void event_add(int efd, int events, event *ev)
{
	struct epoll_event epv = {0, {0}};
	int op;
	epv.data.ptr = ev;
	epv.events = ev->events = events;

	if (ev->status == 1) {
		op = EPOLL_CTL_MOD;
	} else {
		op = EPOLL_CTL_ADD;
		ev->status = 1;
	}

	if (epoll_ctl(efd, op, ev->fd, &epv) < 0)
		printf("[%s]:event add failed[fd=%d], events[%d]\n", __func__, ev->fd, events);
	else
		printf("[%s]:event add ok[fd=%d], op=%d, events[%0x]\n", __func__, ev->fd, op, events);
}

void event_del(int efd, event *ev)
{
	struct epoll_event epv = {0, {0}};

	if (ev->status)
		return;
	epv.data.ptr = ev;
	ev->status = 0;
	epoll_ctl(efd, EPOLL_CTL_DEL, ev->fd, &epv);
}


void recv_data(int fd, int events, void *arg)
{
	struct _event *ev = (struct _event*) arg;
	int len;
	len = recv(fd, ev->buff + ev->len, sizeof(ev->buff) - 1 - ev->len, 0);

	event_del(epfd, ev);

	if (len > 0) {
		ev->len += len;
		ev->buff[ev->len] = '\0';
		printf("C[%d]:%s\n", fd, ev->buff);

		//chage to send event
		event_set(ev, fd, send_data, ev);
		event_add(epfd, EPOLLOUT, ev);
	}
	else if (len == 0) {
		close(ev->fd);
		printf("[%s]:[fd=%d], pos[%d], close gracefully.\n", __func__, fd, ev - event_list);
	}
	else {
		close(ev->fd);
		printf("[%s]:recv[fd=%d] error[%d]:%s\n", __func__, fd, errno, strerror(errno));
	}
}

void send_data(int fd, int events, void *arg)
{
	struct _event *ev = (struct _event*) arg;
	int len;

	//send data
	len = send(fd, ev->buff + ev->s_offset, ev->len - ev->s_offset, 0);

	if (len > 0) {
		printf("[%s]:send [fd=%d], [%d<->%d]%s\n", __func__, fd, len, ev->len, ev->buff);
		ev->s_offset += len;

		if (ev->s_offset == ev->len) {
			//change to receive event
			event_del(epfd, ev);
			event_set(ev, fd, recv_data, ev);
			event_add(epfd, EPOLLIN, ev);
		}
	}
	else {
		close(ev->fd);
		event_del(epfd, ev);
		printf("[%s]:send[fd=%d] error[%d]\n", __func__, fd, errno);
	}
}


void accep_connent(int fd, int events, void *arg)
{
	struct sockaddr_in sin;
	socklen_t len = sizeof(struct sockaddr_in);
	int nfd /*客户端请求过的fd*/, i;

	//accept
	if ((nfd = accept(fd, (struct sockaddr*)&sin, &len)) == -1) {
		errExit("%s: accept, %d", __LINE__, errno);
	}

	for (i =0; i < MAX_EVENTS; i++) {
		if (event_list[i].status == 0)
			break;
	}

	if (i == MAX_EVENTS) {
		printf("[%s]:max connection limit[%d].", __LINE__, MAX_EVENTS);
		close(nfd);//达到最大连接数关闭 nfd
		goto output;
	}

	//set nonblocking
	int iret = 0;
	if ((iret = fcntl(nfd, F_SETFL, O_NONBLOCK)) < 0) {
		printf("[%s]: fcntl nonblocking failed:%d", __LINE__, iret);
		goto output;
	}

	/*
	 * 这里第一个和第四个参数基本一样的.所以会认为这里函数原型只需要3个参数即可.
	 * 逻辑意义上而言, 参数1表示在哪个events上设置callback, 参数4是传递给callback的参数.
	 *
	 */
	event_set(&event_list[i], nfd, recv_data, &event_list[i]);
	event_add(epfd, EPOLLIN, &event_list[i]);

	output:
	printf("[%s]:new conn[%s:%d][time:%d],pos[%d]\n", __func__, inet_ntoa(sin.sin_addr), 
		ntohs(sin.sin_port), event_list[i].last_active, i);
}

void listen_socket(short port)
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	fcntl(fd, F_SETFL, O_NONBLOCK);

	printf("[%s]:server listen on fd = %d\n", __func__, fd);

	//在event_list列表尾添加server的fd监听事件
	event_set(&event_list[MAX_EVENTS], fd, accep_connent, &event_list[MAX_EVENTS]);

	//add listen socket
	event_add(epfd, EPOLLIN, &event_list[MAX_EVENTS]);

	//bind & listen
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(port);

	if (bind(fd, (struct sockaddr *) &sin, sizeof(sin)) == -1) {
		errExit("[%s]:bind, error[%d]\n", __LINE__, errno);
	}

	if (listen(fd, 5) == -1) {
		errExit("[%s]:listen, error[%d]\n", __LINE__, errno);
	}
}

// var url = 'http://blog.csdn.net/sparkliang/article/details/4770655';