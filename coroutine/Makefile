
TARGET := client coroutine serverA serverB

all: $(TARGET)

client: client.c
	gcc -g $< -o $@

coroutine: coroutine.c
	gcc -g $< -o $@

serverA: serverA.c
	gcc -g $< -o $@

serverB: serverB.c
	gcc -g $< -o $@

clean:
	rm -rf $(TARGET)
