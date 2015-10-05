#include <zmq.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#define MSG_SIZE    32

int main(void)
{
    void* zmq_ctx = zmq_ctx_new();
    void* zmq_sock = zmq_socket(zmq_ctx, ZMQ_REQ);
    zmq_connect(zmq_sock, "tcp://localhost:5555");
    int pid = getpid();

    int index = 0;
    for (index = 0; index < 10; index++)
    {
        printf("\nloop seq:%d\n", index);
        char buf[MSG_SIZE] = {0};
        sprintf(buf, "pid:%d hello%d", pid, index);
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
    }

    sleep(1);
    zmq_close(zmq_sock);
    zmq_ctx_destroy(zmq_ctx);

    return 0;
}
