#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/types.h>       
#include <sys/socket.h>
#include <errno.h>
#include <event.h>

#define BUF_SIZE	1024
char buf[BUF_SIZE];

void on_accept(int sock, short event, void* arg);
void on_read(int sock, short event, void* arg);

struct event_base* evbase = NULL;

int main(void)
{
	int listenfd = -1;
	struct sockaddr_in svraddr;
	int nready = 0;
	int i = 0;

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
		
    evbase = event_base_new();
    struct event evlisten;
    event_set(&evlisten, listenfd, EV_READ | EV_PERSIST, on_accept, (void* )evbase);
    event_base_set(evbase, &evlisten);
    event_add(&evlisten, NULL);

    event_base_dispatch(evbase);
    printf("The End.\n");

	return 0;
}

void on_accept(int sock, short event, void* arg)
{
    int fd;
    struct event_base* evbase = (struct event_base* )arg;
    struct sockaddr_in cliaddr;
    uint32_t slen = sizeof(cliaddr);
    fd = accept(sock, (struct sockaddr* )&cliaddr, &slen);
    if (fd < 0)
    {
        perror("accept");
        return;
    }
    printf("recv sock %d: <%s:%d>\n", fd, inet_ntoa(cliaddr.sin_addr), htons(cliaddr.sin_port));

    struct event* evread = (struct event* )malloc(sizeof(struct event));
    event_set(evread, fd, EV_READ | EV_PERSIST, on_read, (void* )evread);
    event_base_set(evbase, evread);
    event_add(evread, NULL);
}

void on_read(int sock, short event, void* arg)
{
    int rsize = 0;
    memset(buf, 0, BUF_SIZE);
    rsize = read(sock, buf, BUF_SIZE);
    if (rsize <= 0)
    {
        if (rsize == 0)
        {
            printf("client %d close socket.\n", sock);
        }
        else
        {
            perror("read:");
        }
        close(sock);
        event_del((struct event* )arg);
        free((struct event* )arg);
        return;
    }
    printf("recv:%s\n", buf);
    write(sock, buf, strlen(buf));
    printf("send:%s\n", buf);
}
