#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <pthread.h>

#define SVRPORT		44898
#define BACK_LOG	5
#define BUFFER_SIZE	1024

void* copyto(void* arg);

int g_sockfd = 0;

int main(int argc, char* argv[])
{
	int sockfd;
	socklen_t socklen = sizeof(struct sockaddr_in);
	struct sockaddr_in svraddr;
	char recvbuf[BUFFER_SIZE];
	int nbyte = 0;
	pthread_t tid;

	if (argc != 2)
	{
		printf("Usage:./client <IP>\n");
		exit(0);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		perror("socket:");
		exit(0);
	}

	memset(&svraddr, 0, sizeof(svraddr));
	svraddr.sin_family = AF_INET;
	svraddr.sin_port = htons(SVRPORT);
	inet_pton(AF_INET, argv[1], &svraddr.sin_addr);

	if (connect(sockfd, (struct sockaddr*)&svraddr, socklen) < 0)
	{
		perror("connect");
		exit(0);
	}
	printf("connect to server ... success\n");

	g_sockfd = sockfd;
	pthread_create(&tid, NULL, copyto, NULL);
	
	printf("ym65536#");
	fflush(stdout);
	while (1)
	{
		nbyte = read(sockfd, recvbuf, BUFFER_SIZE);
		if (nbyte <= 0)
		{
			if (nbyte == 0)
			{
				printf("server close socket.\n");
			}
			else
			{
				perror("read");
			}
			close(sockfd);
			exit(0);
		}
		recvbuf[nbyte] = '\0';
		fputs(recvbuf, stdout);
		printf("ym65536#");
		fflush(stdout);
	}

	return 0;
}

void* copyto(void* arg)
{
	int len = 0;
	char sendbuf[BUFFER_SIZE];
	while (fgets(sendbuf, BUFFER_SIZE, stdin) != NULL)
	{
		if (strncmp(sendbuf, "quit", 4) == 0)
		{
			close(g_sockfd);
			exit(0);
		}
		len = strlen(sendbuf);
		sendbuf[len - 1] = '\0';
		printf("<read %s from stdin>\n", sendbuf);
		write(g_sockfd, sendbuf, strlen(sendbuf));
	}
}
