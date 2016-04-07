#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include "st.h"

#define SVRPORT		44898
#define BACK_LOG	5
#define BUFFER_SIZE	1024

void* handle_request(void *arg);

int main(int argc, char* argv[])
{
	int listenfd;
	st_netfd_t svrnfd, clinfd;
	socklen_t socklen;
	pthread_t tid;
	struct sockaddr_in cliaddr;
	struct sockaddr_in svraddr;
	int i = 0;
	int nret = 0;
  
	/* Initialize the ST library */
	if (st_init() < 0) 
	{
		perror("st_init");
		exit(1);
	}

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd < 0)
	{
		perror("socket:");
		exit(0);
	}
	int n = 1;
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char *)&n, sizeof(n)) < 0) 
	{
		perror("setsockopt");
		exit(1);
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

	if ((svrnfd = st_netfd_open_socket(listenfd)) == NULL) 
	{
		perror("st_netfd_open");
		exit(1);
	}

	while (1)
	{
        n = sizeof(cliaddr);
        clinfd = st_accept(svrnfd, (struct sockaddr *)&cliaddr, &n, ST_UTIME_NO_TIMEOUT);
        if (clinfd == NULL) 
        {
            perror("st_accept");
            exit(1);
        }

        if (st_thread_create(handle_request, clinfd, 0, 0) == NULL) 
        {
            perror("st_thread_create");
            exit(1);
        }
    }
}

void* handle_request(void *arg)
{
	st_netfd_t connfd = (st_netfd_t)arg;
	char buf[BUFFER_SIZE] = {0};
	int nbyte = 0;
	int i = 0;

	while (1)
	{
		nbyte = st_read(connfd, buf, BUFFER_SIZE, ST_UTIME_NO_TIMEOUT);
		if (nbyte <= 0)
		{
			if (nbyte == 0)
			{
				printf("client %d close socket.\n", st_netfd_fileno(connfd));
			}
			else
			{
				perror("read");
			}
			st_netfd_close(connfd);
			break;
		}
		buf[nbyte] = '\0';
		printf("recv %d: %s\n", nbyte, buf);
		
		for (i = 0; i < 10; i++)
		{
			printf("deal something ...\n");
			st_sleep(1);
		}
		if (strncmp(buf, "quit", 4) == 0)
		{
			printf("client  send quit, close.\n");
			st_netfd_close(connfd);
			break;
		}
		st_write(connfd, buf, strlen(buf), ST_UTIME_NO_TIMEOUT);
		printf("send %d: %s\n", nbyte, buf);
	}
	st_thread_exit(NULL);

	return NULL;
}
