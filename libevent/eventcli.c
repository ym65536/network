#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <errno.h>

#define EVENT_SIZE	1024
#define BUF_SIZE	1024

int do_read(int epollfd, int fd, int sockfd, char* buf, int maxsize);
int do_write(int epollfd, int fd, int sockfd,  char* buf, int maxsize);

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
	int sockfd = -1;
	int epollfd = -1;
	struct sockaddr_in svraddr;
	struct epoll_event events[EVENT_SIZE];
	int nready = 0;
	char buf[BUF_SIZE];
	int i = 0;
	int fd = 0;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket error");
		return -1;
	}

	memset(&svraddr, 0, sizeof(svraddr));
	svraddr.sin_family = AF_INET;
	svraddr.sin_port = htons(44888);
	inet_pton(AF_INET, "127.0.0.1", &svraddr.sin_addr);

	if (connect(sockfd, (struct sockaddr* )&svraddr, sizeof(svraddr)) < 0)
	{
		perror("connect error");
		return -1;
	}
	printf("connect to server: <127.0.0.1:44888>\n");

	if ((epollfd = epoll_create(EVENT_SIZE)) < 0)
	{
		perror("bind error");
		return -1;
	}
	event_add(epollfd, STDIN_FILENO, EPOLLIN);

	while (1)
	{
		nready = epoll_wait(epollfd, events, EVENT_SIZE, -1);
		//printf("EVENT SIZE %d\n", nready);
		for (i = 0; i < nready; i++)
		{
			fd = events[i].data.fd;
			if (events[i].events & EPOLLIN)
			{
				//printf("IN EVENT %d:%d\n", fd, EPOLLIN);
				do_read(epollfd, fd, sockfd, buf, BUF_SIZE - 1);
			}
			else if (events[i].events & EPOLLOUT)
			{
				//printf("OUT EVENT %d:%d\n", fd, EPOLLOUT);
				do_write(epollfd, fd, sockfd, buf, BUF_SIZE - 1);
			}
			else
			{
				printf("unused fd %d, event %d", fd, events[i].events);
			}
		}
	}

	return 0;
}

int do_read(int epollfd, int fd, int sockfd, char* buf, int maxsize)
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

	if (fd == STDIN_FILENO) // std input
	{
		printf("cli input %d: %s", fd, buf);
		event_add(epollfd, sockfd, EPOLLOUT);
	}
	else
	{
		printf("read from sock %d: %s", fd, buf);
		event_del(epollfd, sockfd, EPOLLIN);
		event_add(epollfd, STDOUT_FILENO, EPOLLOUT);
	}
}

int do_write(int epollfd, int fd, int sockfd, char* buf, int maxsize)
{
	int nread = write(fd, buf, maxsize);
	if (nread < 0)
	{
		printf("client %d write error\n", fd);
		close(fd);
		event_del(epollfd, fd, EPOLLIN);
		return nread;
	}

	if (fd == STDOUT_FILENO)
	{
		printf("write to stdin %d: %s", fd, buf);
		event_del(epollfd, fd, EPOLLOUT);
	}
	else
	{
		printf("write to sock %d: %s", fd, buf);
		event_mod(epollfd, fd, EPOLLIN);
	}
}

