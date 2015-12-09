TARGET = server client

OS = LINUX
BUILD = DBG
STDIR = /mnt/software/st-1.9
TARGETDIR   = $(STDIR)/$(OS)_$(shell uname -r)_$(BUILD)

LIB = $(TARGETDIR)/libst.a
CFLAG = -g -I$(TARGETDIR)/

all: $(TARGET)

server: server.c
	gcc $(CFLAG) $< -o $@ $(LIB)

client: client.c
	gcc -g $< -o $@ $(LIB) -pthread

clean:
	rm -rf $(TARGET)
