#include "zmq.h"
#include <string.h>
#include <unistd.h>

#define MSG_SIZE 256

int main(void)
{
    void* zmq_ctx = zmq_ctx_new();
    void* zmq_sock = zmq_socket(zmq_ctx, ZMQ_REP);
    zmq_bind(zmq_sock, "tcp://*:5555");

    printf("start listen for socket...\n");
    int seq = 0;
    while (1)
    {
        printf("\nloop seq: %d\n", seq++);
        int msg_size = 0;
        char buf[MSG_SIZE + 1] = {0};
        zmq_msg_t request;
        zmq_msg_init(&request);
        zmq_msg_recv(&request, zmq_sock, 0);
        msg_size = zmq_msg_size(&request);
        memcpy(buf, zmq_msg_data(&request), msg_size);
        printf("recv request: %s\n", buf);
        zmq_msg_close(&request);

        sleep(1);

        zmq_msg_t reply;
        zmq_msg_init_size(&reply, msg_size);
        memcpy(zmq_msg_data(&reply), buf, msg_size);
        printf("send reply: %s\n", buf);
        zmq_msg_send(&reply, zmq_sock, 0);
        zmq_msg_close(&reply);
    }

    sleep(1);
    zmq_close(zmq_sock);
    zmq_ctx_destroy(zmq_ctx);

    return 0;
}
