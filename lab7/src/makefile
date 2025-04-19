# Определяем компилятор и флаги
CC = gcc
CFLAGS = -Wall

# Определяем имена выходных файлов
TCP_CLIENT = tcpclient
TCP_SERVER = tcpserver
UDP_CLIENT = udpclient
UDP_SERVER = udpserver

# Исходные файлы
TCP_CLIENT_SRC = tcpclient.c
TCP_SERVER_SRC = tcpserver.c
UDP_CLIENT_SRC = udpclient.c
UDP_SERVER_SRC = udpserver.c

# Целевая установка по умолчанию
all: $(TCP_CLIENT) $(TCP_SERVER) $(UDP_CLIENT) $(UDP_SERVER)

# Правила для компиляции TCP клиента
$(TCP_CLIENT): $(TCP_CLIENT_SRC)
	$(CC) $(CFLAGS) -o $(TCP_CLIENT) $(TCP_CLIENT_SRC)

# Правила для компиляции TCP сервера
$(TCP_SERVER): $(TCP_SERVER_SRC)
	$(CC) $(CFLAGS) -o $(TCP_SERVER) $(TCP_SERVER_SRC)

# Правила для компиляции UDP клиента
$(UDP_CLIENT): $(UDP_CLIENT_SRC)
	$(CC) $(CFLAGS) -o $(UDP_CLIENT) $(UDP_CLIENT_SRC)

# Правила для компиляции UDP сервера
$(UDP_SERVER): $(UDP_SERVER_SRC)
	$(CC) $(CFLAGS) -o $(UDP_SERVER) $(UDP_SERVER_SRC)

# Правила для очистки скомпилированных файлов
clean:
	rm -f $(TCP_CLIENT) $(TCP_SERVER) $(UDP_CLIENT) $(UDP_SERVER)

.PHONY: all clean