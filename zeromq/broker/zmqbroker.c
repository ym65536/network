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
        int more = 0;
        zmq_poll(items, 2, -1);
        if (items[0].revents & ZMQ_POLLIN)
        {
            int idx = 0;
            zmq_msg_t message[10];
            while (1)
            {
                zmq_msg_init(&message[idx]);
                zmq_msg_recv(&message[idx], front_sock, 0);
                printf("router recv/send: size = %d\n", zmq_msg_size(&message[idx]));
                uint32_t more_size = sizeof(more);
                zmq_getsockopt(front_sock, ZMQ_RCVMORE, &more, &more_size);
                if (!more)
                    break;
                idx++;
            }
            int i = 0;
            for (i = 0; i < idx; i++)
            {
                zmq_msg_send(&message[i], back_sock, (i == (idx - 1)) ? 0 : ZMQ_SNDMORE);
                zmq_msg_close(&message[i]);
            }
        }
        if (items[1].revents & ZMQ_POLLIN)
        {
            zmq_msg_t message;
            while (1)
            {
                zmq_msg_init(&message);
                zmq_msg_recv(&message, back_sock, 0);
                printf("dealer recv/send: size = %d\n", zmq_msg_size(&message));
                uint32_t more_size = sizeof(more);
                zmq_getsockopt(back_sock, ZMQ_RCVMORE, &more, &more_size);
                zmq_msg_send(&message, front_sock, more ? ZMQ_SNDMORE | ZMQ_NOBLOCK : 0);
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
