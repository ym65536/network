#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, const char* argv[])
{
	if (argc < 3)
	{
		fprintf(stderr, "usage: client <ip> <port>\n");
		exit(1);
	}

	const char *ip = argv[1];
	unsigned short port = (unsigned short)atoi(argv[2]);

	int sock = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	inet_aton(ip, &server_addr.sin_addr);
	server_addr.sin_port = htons(port);

	if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("connect fail");
		exit(1);
	}

	char line[1024] = {0};
	while (fgets(line, sizeof(line), stdin) != NULL)
	{
		//for better output
		line[strlen(line) - 1] = 0;

		ssize_t send_len = send(sock, line, strlen(line), 0);
		if (send_len < 0)
		{
			perror("send fail");
			exit(1);
		}
		printf("client sent: [%s]\n", line);

		char recv_buf[255] = {0};
		ssize_t recv_len = recv(sock, recv_buf, sizeof(recv_buf), 0);
		if (recv_len < 0)
		{
			perror("recv fail");
			exit(1);
		}
		printf("client recv: [%s]\n", recv_buf);
	}

	return 0;
}








