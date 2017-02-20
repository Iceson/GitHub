CROSS_COMPILE = 

CC = $(CROSS_COMPILE)gcc

CFLAGS= -Wall -g 

TARGET= server client

all: $(TARGET)

server:server.c
	$(CC) server.c  $(CFLAGS)   -o server -lsqlite3
	
client: client.c
	$(CC) client.c  $(CFLAGS)   -o client -lsqlite3

.PHONY: clean 
clean:
	rm -rf a.out $(TARGET)
