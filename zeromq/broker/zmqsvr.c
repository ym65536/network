#include "zmq.h"
#include <string.h>
#include <unistd.h>

#define MSG_SIZE 256

int main(void)
{
    void* zmq_ctx = zmq_ctx_new();
    void* zmq_sock = zmq_socket(zmq_ctx, ZMQ_REP);
    zmq_connect(zmq_sock, "tcp://localhost:5560");

    int seq = 0;
    while (1)
    {
        printf("\nloop seq: %d\n", seq++);
   //     char buf[MSG_SIZE + 1] = {0};
        int more = 1;
        zmq_msg_t message;
        while (1)
        {
            zmq_msg_init(&message);
            zmq_msg_recv(&message, zmq_sock, 0);
            printf("recv/send: size = %d\n", zmq_msg_size(&message));
            uint32_t more_size = sizeof(more);
            zmq_getsockopt(zmq_sock, ZMQ_RCVMORE, &more, &more_size);
            zmq_msg_send(&message, zmq_sock, more ? ZMQ_SNDMORE : 0);
            zmq_msg_close(&message);
            if (!more)
                break;
        }
        sleep(1);
    }

    zmq_close(zmq_sock);
    zmq_ctx_destroy(zmq_ctx);

    return 0;
}
