TARGET = server client
LIB = -pthread

all: $(TARGET)

server: server.c
	gcc -g $< -o $@ $(LIB)

client: client.c
	gcc -g $< -o $@ $(LIB)

clean:
	rm -rf $(TARGET)
