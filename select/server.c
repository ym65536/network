#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>

#define SVRPORT		44898
#define BACK_LOG	5

int client[BACK_LOG];

int main(int argc, char* argv[])
{
	int listenfd, connfd;
	socklen_t socklen;
	fd_set rset, allset;
	struct sockaddr_in cliaddr;
	struct sockaddr_in svraddr;
	int maxfd = 0;
	int maxidx = -1;
	int i = 0;

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
	maxfd = listenfd;
	maxidx = -1;
	for (i = 0; i < BACK_LOG; i++)
		client[i] = -1;

	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);
	while (true)
	{
		rset = allset;
		nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
		if (nready < 0)
		{
			perror("select:");
			continue;
		}

		if (FD_ISSET(listenfd, &rset)) /* new client */
		{
			connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &socklen);
			if (connfd < 0)
			{
				perror("accept:");
				continue;
			}

			for (i = 0; i < BLACK_LOG; i++)
			{
				if (client[i] < 0)
				{
					client[i] = connfd;
					break;
				}
			}
			if (i == BLACK_LOG)
			{
				printf("ERROR: too many client\n");
				exit(0);
			}

			FD_SET(connfd, &allset);
			maxfd = (connfd > maxfd) ? connfd : maxfd;
			maxidx = (maxidx > i) ? maxidx : i;
			if (--nready <= 0)
				continue;
		}

		for (i = 0; i <= maxidx; i++)
		{
			if ((sockfd = client[i]) < 0)
				continue;
			if (FD_ISSET(sockfd, &rset))
			{
				nbyte = read(sockfd, buffer, MAXLINE);
				if (nbyte <= 0)
				{
					if (nbyte == 0) // client close socket
					{
						printf("client %d close socket.\n", sockfd);
					}
					else
					{
						perror("read:");
					}
					close(sockfd);
					FD_CLR(sockfd, &allset);
					client[i] = -1;
				}
				else
				{
					write(sockfd, buffer, nbyte);
				}

				if (--nready <= 0)
					break;
			}
		}
	}
}
