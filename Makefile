CC = gcc
CFLAGS = -Izlib/include -Lzlib/lib/
all:
	$(CC) tiny_webserver.c $(CFLAGS) -o tiny_websrv -lz
