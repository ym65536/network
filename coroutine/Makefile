CC := gcc
CFLAGS := -g -fno-stack-protector
TOOLS := client serverA serverB
TARGET := coroutine

all: $(TARGET)

#$(TARGETDIR)/md.o: md.S
#	$(CC) $(CFLAGS) -c $< -o $@

#$(TARGETDIR)/%.o: %.c common.h md.h
#	$(CC) $(CFLAGS) -c $< -o $@

#coroutine: main.c md.S schedule.c 
coroutine: md.S schedule.c test.c 
	$(CC) $(CFLAGS) $? -o $@

tools: $(TOOLS)

tools/client: tools/client.c 
	gcc -g $< -o $@

tools/serverA: tools/serverA.c
	gcc -g $< -o $@

tools/serverB: tools/serverB.c
	gcc -g $< -o $@

clean:
	rm -rf $(TARGET) && cd tools && rm -rf $(TOOLS)
