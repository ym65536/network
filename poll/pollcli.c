#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <errno.h>

#define EVENT_SIZE  2	
#define BUF_SIZE	1024

struct pollfd events[EVENT_SIZE];

int main(void)
{
	int sockfd = -1;
	int epollfd = -1;
	struct sockaddr_in svraddr;
	int nready = 0;
	char buf[BUF_SIZE];
	int i = 0;

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
	printf("connect to server:%d <127.0.0.1:44888>\n", sockfd);
    
    events[0].fd = STDIN_FILENO;
    events[0].events = POLLIN;
    events[1].fd = sockfd;
    events[1].events = POLLIN;

    printf("ym65536$");
    fflush(stdout);
	while (1)
	{
        int fd = -1;
		nready = poll(events, EVENT_SIZE, -1);
		//printf("EVENT SIZE %d\n", nready);
		for (i = 0; i < EVENT_SIZE; i++)
		{
			fd = events[i].fd;
			if ((fd == STDIN_FILENO) && (events[i].revents & POLLIN)) // read from stdin
			{
                memset(buf, 0, sizeof(buf));
                int nread = read(fd, buf, BUF_SIZE);
                if (nread <= 0)
                {
                    perror("read:");
                    continue;
                }
                write(sockfd, buf, strlen(buf));
			}
			else if ((fd > 2) && (events[i].events & POLLIN))
			{
                memset(buf, 0, sizeof(buf));
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
                    return nread;
                }
                printf("recv from svr:%s\nym65536$", buf);
                fflush(stdout);
			}
			else
			{
				printf("unused fd %d, event %d", fd, events[i].events);
			}
		}
	}

	return 0;
}

