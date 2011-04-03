CFLAGS=-Wall -std=c99

all: mtn-server mtn-client

mtn-server: mtn-server.o request.o transmission.o common.o
	$(CC) $(CFLAGS) -o $@ $^

mtn-client:

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@
