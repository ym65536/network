TARGET = server client

OS = LINUX
BUILD = DBG
STDIR = /mnt/software/statethread
TARGETDIR   = $(STDIR)/$(OS)_$(shell uname -r)_$(BUILD)

LIB = $(TARGETDIR)/libst.a
CFLAG = -g -I$(TARGETDIR)/ -I$(STDIR)

all: $(TARGET)

test: test.c
	gcc $(CFLAG) $< -o $@ $(LIB)

server: server.c
	gcc $(CFLAG) $< -o $@ $(LIB)

client: client.c
	gcc -g $< -o $@ $(LIB) -pthread

hugest: hugest.c
	gcc $(CFLAG) $< -o $@ $(LIB)

clean:
	rm -rf $(TARGET)
