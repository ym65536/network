#include "zmq.h"
#include <string.h>
#include <unistd.h>

#define MSG_SIZE 256

int main(void)
{
    void* zmq_ctx = zmq_ctx_new();
    void* zmq_sock = zmq_socket(zmq_ctx, ZMQ_ROUTER);
    zmq_bind(zmq_sock, "tcp://*:5555");

    printf("start listen for socket...\n");
    int seq = 0;
    zmq_msg_t message[10];
    while (1)
    {
        printf("\nloop seq: %d\n", seq++);
        int k = 0;
        int idx = 0;
        int msg_size = 0;
        char buf[MSG_SIZE + 1] = {0};
        uint32_t more = 0;
        uint32_t more_size = sizeof(more);
        while (1)
        {
            zmq_msg_init(&message[idx]);
            zmq_msg_recv(&message[idx], zmq_sock, 0);
            zmq_getsockopt(zmq_sock, ZMQ_RCVMORE, &more, &more_size);
            msg_size = zmq_msg_size(&message[idx]);
            memcpy(buf, zmq_msg_data(&message[idx]), msg_size);
            printf("recv size: %d\n", msg_size);
            idx++;
            if (!more)
                break;
        }

        sleep(1);

        for (k = 0; k < idx; k++)
        {
            printf("send size: %d\n", zmq_msg_size(&message[k]));
            zmq_msg_send(&message[k], zmq_sock, k == (idx - 1) ? 0 : ZMQ_SNDMORE);
            zmq_msg_close(&message[k]);
        }
    }

    sleep(1);
    zmq_close(zmq_sock);
    zmq_ctx_destroy(zmq_ctx);

    return 0;
}
