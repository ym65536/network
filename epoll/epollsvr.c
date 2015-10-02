#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/types.h>       
#include <sys/socket.h>
#include <errno.h>

#define EVENT_SIZE	1024
#define BUF_SIZE	1024

int do_accept(int epollfd, int listenfd);
int do_read(int epollfd, int fd, char* buf, int maxsize);
int do_write(int epollfd, int fd, char* buf, int maxsize);

int event_add(int epollfd, int fd, int event)
{
	struct epoll_event ev;
	ev.events = event;
	ev.data.fd = fd;

	return epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}

int event_del(int epollfd, int fd, int event)
{
	struct epoll_event ev;
	ev.events = event;
	ev.data.fd = fd;

	return epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
}

int event_mod(int epollfd, int fd, int event)
{
	struct epoll_event ev;
	ev.events = event;
	ev.data.fd = fd;

	return epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
}

int main(void)
{
	int listenfd = -1;
	int epollfd = -1;
	struct sockaddr_in svraddr;
	struct epoll_event events[EVENT_SIZE];
	int nready = 0;
	char buf[BUF_SIZE];
	int i = 0;
	int fd = 0;

	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket error");
		return -1;
	}

	memset(&svraddr, 0, sizeof(svraddr));
	svraddr.sin_family = AF_INET;
	svraddr.sin_port = htons(44888);
	svraddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(listenfd, (struct sockaddr* )&svraddr, sizeof(svraddr)) < 0)
	{
		perror("bind error");
		return -1;
	}

	if (listen(listenfd, 5) < 0)
	{
		perror("listen error");
		return -1;
	}
		
	if ((epollfd = epoll_create(EVENT_SIZE)) < 0)
	{
		perror("bind error");
		return -1;
	}
	event_add(epollfd, listenfd, EPOLLIN);

	printf("listen for sockets ...\n");
	while (1)
	{
		nready = epoll_wait(epollfd, events, EVENT_SIZE, -1);
		for (i = 0; i < nready; i++)
		{
			fd = events[i].data.fd;
			if ((fd == listenfd) && (events[i].events & EPOLLIN))
			{
				do_accept(epollfd, listenfd);	
			}
			else if (events[i].events & EPOLLIN)
			{
				do_read(epollfd, fd, buf, BUF_SIZE - 1);
			}
			else if (events[i].events & EPOLLOUT)
			{
				do_write(epollfd, fd, buf, BUF_SIZE - 1);
			}
			else
			{
				printf("unused fd %d, event %d", fd, events[i].events);
			}
		}
	}

	return 0;
}

int do_accept(int epollfd, int listenfd)
{
	struct sockaddr_in cliaddr;
	int clifd = -1;
	socklen_t len = sizeof(cliaddr);
	if ((clifd = accept(listenfd, (struct sockaddr* )&cliaddr, &len)) < 0)
	{
		perror("accept error");
		return -1;
	}

	printf("accept client: <%s:%d>\n", inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);
	// add client in read list
	event_add(epollfd, clifd, EPOLLIN);
}

int do_read(int epollfd, int fd, char* buf, int maxsize)
{
	memset(buf, 0, maxsize);
	int nread = read(fd, buf, maxsize);
	if (nread <= 0)
	{
		if (nread == 0)
		{
			printf("client %d close socket.\n", fd);
		}
		else
		{
			printf("client %d read error\n", fd);
		}
		close(fd);
		event_del(epollfd, fd, EPOLLIN);
		return nread;
	}

	printf("recv from client %d :%s", fd, buf);
	// set read to write
	event_mod(epollfd, fd, EPOLLOUT);
}

int do_write(int epollfd, int fd, char* buf, int maxsize)
{
	int nread = write(fd, buf, maxsize);
	if (nread < 0)
	{
		printf("client %d write error\n", fd);
		close(fd);
		event_del(epollfd, fd, EPOLLIN);
		return nread;
	}

	printf("send to client %d :%s", fd, buf);
	// set write to read
	event_mod(epollfd, fd, EPOLLIN);
}
