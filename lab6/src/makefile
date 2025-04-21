CC = gcc
CFLAGS = -Wall -pthread

CLIENT = client
SERVER = server

CLIENT_SRC = client.c utils.c
SERVER_SRC = server.c utils.c

all: $(CLIENT) $(SERVER)

$(CLIENT): $(CLIENT_SRC)
	$(CC) $(CFLAGS) -o $(CLIENT) $(CLIENT_SRC)

$(SERVER): $(SERVER_SRC)
	$(CC) $(CFLAGS) -o $(SERVER) $(SERVER_SRC)

clean:
	rm -f $(CLIENT) $(SERVER)