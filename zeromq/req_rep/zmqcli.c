#include <zmq.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#define MSG_SIZE    256

int main(void)
{
    void* zmq_ctx = zmq_ctx_new();
    void* zmq_sock = zmq_socket(zmq_ctx, ZMQ_REQ);
    zmq_connect(zmq_sock, "tcp://localhost:5555");
    int pid = getpid();
    char buf[MSG_SIZE] = {0};
    
    int index = 0;
    printf("ym65536#");
    while (gets(buf) && strcmp(buf, "quit"))
    {
        int len = strlen(buf) > 200 ? 200: strlen(buf);
        sprintf(&buf[len], " id:<%d,%d>", pid, index);
        zmq_msg_t request;
        zmq_msg_init_size(&request, strlen(buf));
        memcpy(zmq_msg_data(&request), buf, strlen(buf));
        printf("send request: %s\n", buf);
        zmq_msg_send(&request, zmq_sock, 0);
        zmq_msg_close(&request);

        memset(buf, 0, sizeof(buf));
        zmq_msg_t reply;
        zmq_msg_init(&reply);
        zmq_msg_recv(&reply, zmq_sock, 0);
        memcpy(buf, zmq_msg_data(&reply), zmq_msg_size(&reply));
        printf("recv reply: %s\n", buf);
        zmq_msg_close(&reply);
        index++;
        printf("ym65536#");
    }

    sleep(1);
    zmq_close(zmq_sock);
    zmq_ctx_destroy(zmq_ctx);

    return 0;
}
