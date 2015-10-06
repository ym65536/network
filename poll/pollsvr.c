#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <sys/stropts.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>       
#include <sys/socket.h>
#include <errno.h>

#define EVENT_SIZE	2
#define BUF_SIZE	1024

struct pollfd client[EVENT_SIZE];

int main(void)
{
	int listenfd = -1;
	struct sockaddr_in svraddr;
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
		
    client[0].fd = listenfd;
    client[0].events = POLLIN;
    for (i = 1; i < EVENT_SIZE; i++)
    {
        client[i].fd = -1; // unused client
    }

//    printf("%08x %08x %08x %08x %08x %08x\n", POLLIN, POLLRDNORM, POLLRDBAND, POLLOUT, POLLWRNORM, POLLWRBAND);
	printf("listen for sockets ...\n");
	while (1)
	{
		//nready = poll(client, EVENT_SIZE, INFTIM);
		nready = poll(client, EVENT_SIZE, -1);
		for (i = 0; i < EVENT_SIZE; i++)
		{
            if (client[i].fd < 0)
            {
                continue;
            }
			if ((client[i].fd == listenfd) && (client[i].revents & POLLIN)) /* new client */
			{
                int k = 0;
                struct sockaddr_in cliaddr;
                int clifd = -1;
                socklen_t len = sizeof(cliaddr);
                if ((clifd = accept(listenfd, (struct sockaddr* )&cliaddr, &len)) < 0)
                {
                    perror("accept error");
                    return -1;
                }
                printf("accept client:%d <%s:%d>\n", clifd, inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);
			    for (k = 1; k < EVENT_SIZE; k++)
                {
                    if (client[k].fd < 0)
                    {
                        client[k].fd = clifd;
                        break;
                    }
                }
                if (k >= EVENT_SIZE)
                {
                    printf("accept new client exceed. close sock %d.\n", clifd);
                    close(clifd);
                    continue;
                }
                client[k].events = POLLIN;
			}
            else if ((i > 0) && (client[i].revents & POLLIN))
			{
                int fd = client[i].fd;
                memset(buf, 0, BUF_SIZE);
                int nread = read(fd, buf, BUF_SIZE);
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
                    client[i].fd = -1;
                    continue;
                }
                printf("recv from client %d :%s", fd, buf);
                
                nread = write(fd, buf, strlen(buf));
                if (nread < 0)
                {
                    printf("client %d write error\n", fd);
                    close(fd);
                    client[i].fd = 0;
                    continue;
                }
                printf("send to client %d :%s", fd, buf);
			}
            else
            {
                //printf("unexpected event:<%d, %d>\n", client[i].fd, client[i].revents);
                continue;
            }
		}
	}

	return 0;
}

