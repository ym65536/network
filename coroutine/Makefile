CC := gcc
CFLAGS := -g -fno-stack-protector
TARGET := client coroutine serverA serverB

all: $(TARGET)

#$(TARGETDIR)/md.o: md.S
#	$(CC) $(CFLAGS) -c $< -o $@

#$(TARGETDIR)/%.o: %.c common.h md.h
#	$(CC) $(CFLAGS) -c $< -o $@

client: client.c 
	gcc -g $< -o $@

coroutine: coroutine.c md.S 
	$(CC) $(CFLAGS) $? -o $@

serverA: serverA.c
	gcc -g $< -o $@

serverB: serverB.c
	gcc -g $< -o $@

clean:
	rm -rf $(TARGET)
