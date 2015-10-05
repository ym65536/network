#include "zmq.h"
#include <string.h>
#include <unistd.h>

int main(void)
{
    void* zmq_ctx = zmq_ctx_new();
    void* front_sock = zmq_socket(zmq_ctx, ZMQ_ROUTER);
    void* back_sock = zmq_socket(zmq_ctx, ZMQ_DEALER);
    zmq_bind(front_sock, "tcp://*:5555");
    zmq_bind(back_sock, "tcp://*:5560");

    zmq_pollitem_t items[] = 
    {
        { front_sock, 0, ZMQ_POLLIN, 0},
        { back_sock, 0, ZMQ_POLLIN, 0},
    };

    int seq = 0;
    while (1)
    {
        printf("\nloop seq: %d\n", seq++);
        zmq_msg_t message;
        int more = 0;
        zmq_poll(items, 2, -1);
        if (items[0].revents & ZMQ_POLLIN)
        {
            while (1)
            {
                zmq_msg_init(&message);
                zmq_msg_recv(&message, front_sock, 0);
                uint32_t more_size = sizeof(more);
                zmq_getsockopt(front_sock, ZMQ_RCVMORE, &more, &more_size);
                zmq_msg_send(&message, back_sock, more ? ZMQ_SNDMORE : 0);
                zmq_msg_close(&message);
                if (!more)
                    break;
            }
        }
        if (items[1].revents & ZMQ_POLLIN)
        {
            while (1)
            {
                zmq_msg_init(&message);
                zmq_msg_recv(&message, back_sock, 0);
                uint32_t more_size = sizeof(more);
                zmq_getsockopt(back_sock, ZMQ_RCVMORE, &more, &more_size);
                zmq_msg_send(&message, front_sock, more ? ZMQ_SNDMORE : 0);
                zmq_msg_close(&message);
                if (!more)
                    break;
            }
        }
    }

    zmq_close(front_sock);
    zmq_close(back_sock);
    zmq_ctx_destroy(zmq_ctx);

    return 0;
}
