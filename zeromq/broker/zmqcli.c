#include <zmq.h>
#include <stdio.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define MSG_SIZE    256
struct addrinfo
{
    uint32_t ip;
    uint16_t port;
    uint16_t pad;
};

int main(void)
{
    void* zmq_ctx = zmq_ctx_new();
    void* zmq_sock = zmq_socket(zmq_ctx, ZMQ_DEALER);
    zmq_connect(zmq_sock, "tcp://localhost:5555");
 //   int pid = getpid();
    char buf[MSG_SIZE] = {0};
    
    zmq_pollitem_t items[] = 
    {
        {zmq_sock, 0, ZMQ_POLLIN, 0},       // zmq socket
        {0, STDIN_FILENO, ZMQ_POLLIN, 0},   // posix fd: stdin
    };

    int index = 0;
    int nbyte = 0;
    printf("ym65536#");
    fflush(stdout);
    while (1)
    {
        zmq_poll(items, 2, -1);
        if (items[1].revents & ZMQ_POLLIN)
        {
            nbyte = read(STDIN_FILENO, buf, MSG_SIZE);
            nbyte = nbyte > 200 ? 200: nbyte;
 //           sprintf(&buf[nbyte], " id:<%d,%d>", pid, index);

            // send empty msg
            zmq_msg_t blankMsg;
            zmq_msg_init_size(&blankMsg, 0);
            zmq_msg_send(&blankMsg, zmq_sock, ZMQ_SNDMORE);
            zmq_msg_close(&blankMsg);

            // send addr info
            zmq_msg_t addrMsg;
            struct addrinfo addr;
            addr.ip = ntohl(inet_addr("10.10.15.15"));
            addr.port = htons(5555);
            addr.pad = 0;
            zmq_msg_init_size(&addrMsg, sizeof(struct addrinfo));
            memcpy(zmq_msg_data(&addrMsg), (char* )&addr, sizeof(addr));
            zmq_msg_send(&addrMsg, zmq_sock, ZMQ_SNDMORE);
            zmq_msg_close(&addrMsg);

            zmq_msg_t request;
            zmq_msg_init_size(&request, strlen(buf));
            memcpy(zmq_msg_data(&request), buf, strlen(buf));
            printf("send request: %s\n", buf);
            zmq_msg_send(&request, zmq_sock, 0);
            zmq_msg_close(&request);
            printf("ym65536#");
            fflush(stdout);
        }
      
        if (items[0].revents & ZMQ_POLLIN) // recv from broker
        {
            // recv empty msg
            zmq_msg_t blankMsg;
            zmq_msg_init(&blankMsg);
            zmq_msg_recv(&blankMsg, zmq_sock, 0);
            zmq_msg_close(&blankMsg);
            printf("recv: msgsize=%d\n", zmq_msg_size(&blankMsg));

            // recv addr info
            zmq_msg_t addrMsg;
            struct addrinfo addr;
            zmq_msg_init(&addrMsg);
            zmq_msg_recv(&addrMsg, zmq_sock, 0);
            memcpy((char*)&addr, zmq_msg_data(&addrMsg), sizeof(addr));
            zmq_msg_close(&addrMsg);
            printf("recv: ip:%08x, port:%d\n", addr.ip, addr.port);

            memset(buf, 0, sizeof(buf));
            zmq_msg_t reply;
            zmq_msg_init(&reply);
            zmq_msg_recv(&reply, zmq_sock, 0);
            memcpy(buf, zmq_msg_data(&reply), zmq_msg_size(&reply));
            printf("recv: %s\n", buf);
            zmq_msg_close(&reply);
        }
        index++;
    }

    sleep(1);
    zmq_close(zmq_sock);
    zmq_ctx_destroy(zmq_ctx);

    return 0;
}
