TARGET = epollsvr epollcli

all: $(TARGET)

epollsvr:epollsvr.c
	gcc -g $< -o $@

epollcli:epollcli.c
	gcc -g $< -o $@

clean:
	rm -rf $(TARGET)
