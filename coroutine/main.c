/*
 * a coroutine demo that implements the following transaction:
 *   client      logic_server     upper_server     reverse_server
 *     |------------->|                 |                |
 *     |  'abc'       |---------------->|                |
 *     |              |<----------------|                |
 *     |              |     'ABC'                        |
 *     |              |                                  |
 *     |              |--------------------------------->|
 *     |              |<---------------------------------|
 *     |              |
 *     |<-------------|       
 *        'CBA'
 */

#include "common.h"

static const char *g_server_ip, *g_ip_A, *g_ip_B;
static unsigned short g_server_port, g_port_A, g_port_B;
static int g_epoll_fd;

extern st_thread_t st_thread_pool[MAX_COROUTINE];  //indexed by accepted socket fd

extern int accepted_sock[MAX_EVENTS]; //accepted socket id indexed by event fd

int CreateListenSocket(const char *ip, unsigned short port)
{
	int listen_fd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	inet_aton(ip, &server_addr.sin_addr);
	server_addr.sin_port = htons(port);
	
	int yes = 1;
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
	{
		perror("setsockopt error");
		exit(1);
	}

	if (bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("bind error");
		exit(1);
	}
	
	if (listen(listen_fd, 1024) < 0)
	{
		perror("listen error");
		exit(1);
	}

	return listen_fd;
}

void AddEvents(int epoll_fd, int sock_fd, int events)
{
	struct epoll_event ev;
	ev.events = events;
	ev.data.fd = sock_fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_fd, &ev) == -1) 
	{
		perror("epoll_ctl: listen_sock");
		exit(1);
	}
}


void RecvRequest(int conn_fd)
{
	st_thread_t *pctx = &st_thread_pool[conn_fd];

	pctx->data_len = recv(conn_fd, pctx->data, sizeof(pctx->data), 0);
	if (pctx->data_len < 0)
	{
		perror("recv error");
		exit(1);
	}
	pctx->data[pctx->data_len] = 0;

	if (pctx->data_len == 0)
	{
		printf("[UserRoutine %d] Client Closed, Reschedule\n", conn_fd);
		close(conn_fd);
		jmp_buf jmpto;
		memcpy(jmpto, pctx->context, sizeof(jmp_buf));
		memset(pctx, 0, sizeof(st_thread_t));
		MT_LONGJMP(jmpto, 1);
		return;
	}

	printf("[UserRoutine %d] Recv Request Data: [%s]\n", conn_fd, pctx->data);
}

void SendRequestA(int conn_fd)
{
	st_thread_t *pctx = &st_thread_pool[conn_fd];

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in remote_addr;
	remote_addr.sin_family = AF_INET;
	inet_aton(g_ip_A, &remote_addr.sin_addr);
	remote_addr.sin_port = htons(g_port_A);

	if (connect(sock, (struct sockaddr*)&remote_addr, sizeof(remote_addr)) < 0)
	{
		perror("connect server A fail");
		exit(1);
	}

	ssize_t send_len = send(sock, pctx->data, pctx->data_len, 0);
	if (send_len < 0)
	{
		perror("send server A fail");
		exit(1);
	}

	printf("[UserRoutine %d] Request sent to Server_A, msg = [%s]\n", conn_fd, pctx->data);
	//printf("sock_A = %d\n", sock);

	AddEvents(g_epoll_fd, sock, EPOLLIN);
	accepted_sock[sock] = conn_fd;
	pctx->event_fd = sock;
}

void SendRequestB(int conn_fd)
{
	st_thread_t *pctx = &st_thread_pool[conn_fd];

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in remote_addr;
	remote_addr.sin_family = AF_INET;
	inet_aton(g_ip_B, &remote_addr.sin_addr);
	remote_addr.sin_port = htons(g_port_B);

	if (connect(sock, (struct sockaddr*)&remote_addr, sizeof(remote_addr)) < 0)
	{
		perror("connect server B fail");
		exit(1);
	}

	ssize_t send_len = send(sock, pctx->data, pctx->data_len, 0);
	if (send_len < 0)
	{
		perror("send server B fail");
		exit(1);
	}

	printf("[UserRoutine %d] Request sent to Server_B, msg = [%s]\n", conn_fd, pctx->data);
	//printf("sock_B = %d\n", sock);

	AddEvents(g_epoll_fd, sock, EPOLLIN);
	accepted_sock[sock] = conn_fd;
	pctx->event_fd = sock;
}

void RecvResponseA(int conn_fd)
{
	//printf("RecvResponseA conn_fd = %d\n", conn_fd);

	st_thread_t *pctx = &st_thread_pool[conn_fd];

	pctx->data_len = recv(pctx->event_fd, pctx->data, sizeof(pctx->data), 0);
	if (pctx->data_len < 0)
	{
		perror("recv from A fail");
		exit(1);
	}
	pctx->data[pctx->data_len] = 0;

	printf("[UserRoutine %d] Response from Server_A, msg = [%s]\n", conn_fd, pctx->data);
}

void RecvResponseB(int conn_fd)
{
	//printf("RecvResponseB conn_fd = %d\n", conn_fd);

	st_thread_t *pctx = &st_thread_pool[conn_fd];

	pctx->data_len = recv(pctx->event_fd, pctx->data, sizeof(pctx->data), 0);
	if (pctx->data_len < 0)
	{
		perror("recv from B fail");
		exit(1);
	}
	pctx->data[pctx->data_len] = 0;

	printf("[UserRoutine %d] Response from Server_B, msg = [%s]\n", conn_fd, pctx->data);
}

void SendResponse(int conn_fd)
{
	st_thread_t *pctx = &st_thread_pool[conn_fd];

	ssize_t send_len = send(conn_fd, pctx->data, pctx->data_len, 0);
	if (send_len < 0)
	{
		perror("send response fail");
		exit(1);
	}

	printf("[UserRoutine %d] Send Response, msg = [%s]\n", conn_fd, pctx->data);
}

static int current_conn_fd;

int UserRoutine(void *ptr)
{
	StackProtect

	int i = 0;
	current_conn_fd = (int)(long)ptr;	
	printf("[UserRoutine %d] Start\n", current_conn_fd);

	st_thread_t *pctx = &st_thread_pool[current_conn_fd];

	pctx->started = 1;
	RecvRequest(current_conn_fd);

	SendRequestA(current_conn_fd);
	i++;
	Schedule(current_conn_fd);
	i++;
	RecvResponseA(current_conn_fd);

	SendRequestB(current_conn_fd);
	i++;
	Schedule(current_conn_fd);
	i++;
	RecvResponseB(current_conn_fd);

	SendResponse(current_conn_fd);
	pctx->started = 0;

	printf("[UserRoutine %d] End\n", current_conn_fd);
	return i;
}

int Go(int (*UserRoutine)(void *ptr), int event_fd)
{
//	StackProtect


	int conn_fd = accepted_sock[event_fd];
	st_thread_t *pctx = &st_thread_pool[conn_fd];

	if (pctx->started == 0)
	{
		if (MT_SETJMP(pctx->context) == 0)	
		{
			//Start New Routine
			UserRoutine((void*)(long)conn_fd);
		}
		printf("UserRoutine[%d] resumed in Go\n", conn_fd);
        return 0;
	} 
		
    //Awaken coresponding thread
	printf("[Main Loop] Events Arrived, Wake Up UserRoutine[%d]\n", conn_fd);
	Schedule(conn_fd);
}


void ParseParams(const char* argv[])
{
	g_server_ip = "127.0.0.1"; //argv[1];
	g_server_port = 3333;//(unsigned short)atoi(argv[2]);
	g_ip_A = "127.0.0.1";//argv[3];
	g_port_A = 1111;//(unsigned short)atoi(argv[4]);
	g_ip_B = "127.0.0.1";//argv[5];
	g_port_B = 2222;//(unsigned short)atoi(argv[6]);
}

int main(int argc, const char *argv[])
{
	if (argc < 7)
	{
//		fprintf(stderr, "usage: server <ip> <port> <leaf_A_ip> <leaf_A_port> <leaf_B_ip> <leaf_B_port>\n");
//		exit(1);
	}
	
	ParseParams(argv);

	int listen_fd = CreateListenSocket(g_server_ip, g_server_port);
	g_epoll_fd = epoll_create(MAX_COROUTINE);
	AddEvents(g_epoll_fd, listen_fd, EPOLLIN);

	for(;;)
	{
		struct epoll_event events[MAX_EVENTS];
		printf("[Main Loop] Wating For Events...\n");
		int nfds = epoll_wait(g_epoll_fd, events, MAX_EVENTS, -1);
		int i;
		for (i = 0; i < nfds; ++i)
		{
			if (events[i].data.fd == listen_fd)
			{
				int conn_fd = accept(listen_fd, NULL, NULL);
				printf("[Main Loop] New Connection, socket fd = %d\n", conn_fd);
				AddEvents(g_epoll_fd, conn_fd, EPOLLIN | EPOLLET);
				accepted_sock[conn_fd] = conn_fd;
				memset(&st_thread_pool[conn_fd], 0, sizeof(st_thread_t));
			} 
			else
			{
				//printf("events on conn_fd = %d\n", events[i].data.fd);
				Go(UserRoutine, events[i].data.fd);
			}
		}
	}
}


