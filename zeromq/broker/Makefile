TARGET = zmqsvr zmqcli zmqbroker

all: $(TARGET)

CFLAGS = -g -I/usr/local/zeromq/include/ -Wall
LDFLAGS = -L/usr/local/zeromq/lib/

zmqsvr:zmqsvr.c
	gcc $(CFLAGS) $< -o $@ $(LDFLAGS) -lzmq

zmqcli:zmqcli.c
	gcc $(CFLAGS) $< -o $@ $(LDFLAGS) -lzmq

zmqbroker:zmqbroker.c
	gcc $(CFLAGS) $< -o $@ $(LDFLAGS) -lzmq

clean:
	rm -rf $(TARGET)
