TARGET = pollsvr pollcli

all: $(TARGET)

pollsvr:pollsvr.c
	gcc -g $< -o $@

pollcli:pollcli.c
	gcc -g $< -o $@

clean:
	rm -rf $(TARGET)
