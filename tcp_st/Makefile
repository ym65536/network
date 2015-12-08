TARGET = server client

STDIR = /mnt/software/st-1.9

LIB = -L $(STDIR)/obj/libst.a -pthread
CFLAG = -g -I $(STDIR)/include

all: $(TARGET)

server: server.c
	gcc -g $< -o $@ $(LIB)

client: client.c
	gcc -g $< -o $@ $(LIB)

clean:
	rm -rf $(TARGET)
