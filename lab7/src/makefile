CC = gcc
CFLAGS = -I.

TCP_SERVER = tcpserver
TCP_CLIENT = tcpclient
UDP_SERVER = udpserver
UDP_CLIENT = udpclient

all: $(TCP_SERVER) $(TCP_CLIENT) $(UDP_SERVER) $(UDP_CLIENT)

$(TCP_SERVER): tcpserver.c
	$(CC) $(CFLAGS) -o $@ $<

$(TCP_CLIENT): tcpclient.c
	$(CC) $(CFLAGS) -o $@ $<

$(UDP_SERVER): udpserver.c
	$(CC) $(CFLAGS) -o $@ $<

$(UDP_CLIENT): udpclient.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TCP_SERVER) $(TCP_CLIENT) $(UDP_SERVER) $(UDP_CLIENT)