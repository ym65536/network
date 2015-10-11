TARGET = eventsvr eventcli

CFLAGS = -g -O0 -I/usr/local/libevent/include
LDFLAGS= -L /usr/local/libevent/lib -levent

all: $(TARGET)

eventsvr:eventsvr.c
	gcc $(CFLAGS) $< -o $@ $(LDFLAGS)

eventcli:eventcli.c
	gcc $(CFLAGS) $< -o $@

clean:
	rm -rf $(TARGET)
