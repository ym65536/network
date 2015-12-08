#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <pthread.h>

#define SVRPORT		44898
#define BACK_LOG	5
#define BUFFER_SIZE	1024

void* event_deal(void *arg);

int main(int argc, char* argv[])
{
	int listenfd, connfd;
	socklen_t socklen;
	pthread_t tid;
	struct sockaddr_in cliaddr;
	struct sockaddr_in svraddr;
	int i = 0;
	int nret = 0;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd < 0)
	{
		perror("socket:");
		exit(0);
	}

	memset(&svraddr, 0, sizeof(svraddr));
	svraddr.sin_family = AF_INET;
	svraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	svraddr.sin_port = htons(SVRPORT);

	if (bind(listenfd, (struct sockaddr*)&svraddr, sizeof(svraddr)) < 0)
	{
		perror("bind:");
		exit(0);
	}

	listen(listenfd, 5);
	printf("listen for socket ... \n");
	while (1)
	{
		int* ipfd = (int* )malloc(sizeof(int));
		if (!ipfd)
		{
			printf("alloc error\n");
			return -1;
		}
		*ipfd = accept(listenfd, (struct sockaddr* )&cliaddr, &socklen);
		if (*ipfd < 0)
		{
			perror("accept");
			return -1;
		}
		printf("client %d connect...success\n", *ipfd);
		pthread_create(&tid, NULL, event_deal, ipfd);
	}
}

void* event_deal(void *arg)
{
	int connfd = *(int* )arg;
	char buf[BUFFER_SIZE] = {0};
	int nbyte = 0;
	free(arg);

	while (1)
	{
		nbyte = read(connfd, buf, BUFFER_SIZE);
		if (nbyte <= 0)
		{
			if (nbyte == 0)
			{
				printf("client %d close socket.\n", connfd);
			}
			else
			{
				perror("read");
			}
			close(connfd);
			break;
		}
		printf("recv %d: %s", nbyte, buf);
		if (strncmp(buf, "quit", 4) == 0)
		{
			printf("client  send quit, close.\n");
			close(connfd);
			break;
		}
		write(connfd, buf, strlen(buf));
		printf("send %d: %s", nbyte, buf);
	}
	
	return NULL;
}
