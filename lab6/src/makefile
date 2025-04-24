CC = gcc
CFLAGS = -I. -lpthread

all: client server

client: client.o multmodulo.o
	$(CC) -o client client.o multmodulo.o $(CFLAGS)

server: server.o multmodulo.o
	$(CC) -o server server.o multmodulo.o $(CFLAGS)

client.o: client.c multmodulo.h
	$(CC) -c client.c $(CFLAGS)

server.o: server.c multmodulo.h
	$(CC) -c server.c $(CFLAGS)

multmodulo.o: multmodulo.c multmodulo.h
	$(CC) -c multmodulo.c $(CFLAGS)

clean:
	rm -f *.o client server