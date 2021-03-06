#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, const char* argv[])
{
#if 0
	if (argc < 3)
	{
		fprintf(stderr, "usage: server <ip> <port>\n");
		exit(1);
	}
#endif

	const char *ip = "127.0.0.1"; //argv[1];
	unsigned short port = 1111; //(unsigned short)atoi(argv[2]);

	int listen_fd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	inet_aton(ip, &server_addr.sin_addr);
	server_addr.sin_port = htons(port);

	bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
	
	listen(listen_fd, 1024);

	struct sockaddr_in cliaddr;
	int conn_fd = accept(listen_fd, NULL, NULL);

	while (1)
	{
		//recv
		char recv_buf[1024] = {0};
		ssize_t recv_len = recv(conn_fd, recv_buf, sizeof(recv_buf), 0);
		if (recv_len < 0)
		{
			printf("recv ret %d, err_msg = [%s]\n", (int)recv_len, (char *)strerror(errno));
			exit(1);
		}
		printf("server_recv: [%s]\n", recv_buf);

		/*processing*/
		int seqno = 0;
		memcpy(&seqno, recv_buf, sizeof(int));
		printf("req seqno is %d, just sleep for a moment...\n", seqno++);
		usleep(1000 * 500);

		//send
		memcpy(recv_buf, &seqno, sizeof(int));
		ssize_t send_len = send(conn_fd, recv_buf, recv_len, 0);
		if (send_len != recv_len)
		{
			printf("send ret %d, err_msg = [%s]\n", (int)send_len, (char *)strerror(errno));
			exit(1);
		}
		printf("server_sent: [%s]\n", recv_buf);

		//close(conn_fd);
	}

	return 0;
}








