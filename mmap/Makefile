TARGET = mmapfile 

CFLAG = -g -I ../include
LDFLAG = -lpthread 

all: $(TARGET)

mmapfile: mmapfile.c
	gcc $(CFLAG) $< -o $@ $(LDFLAG)

clean:
	rm -rf $(TARGET)
