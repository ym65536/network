TARGET = shmget shmread

CFLAG = -g -I ../include

all: $(TARGET)

shmget: shmget.c
	gcc $(CFLAG) $< -o $@

shmread: shmread.c
	gcc $(CFLAG) $< -o $@

clean:
	rm -rf $(TARGET)
