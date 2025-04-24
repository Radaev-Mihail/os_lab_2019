#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SADDR struct sockaddr
#define SIZE sizeof(struct sockaddr_in)

int main(int argc, char *argv[]) {
  // проверка количества аргументов (теперь нужно 4: IP, порт, размер буфера)
  if (argc < 4) {
    printf("использование: %s <IP> <порт> <размер_буфера>\n", argv[0]);
    exit(1);
  }

  int fd;
  int nread;
  const int bufsize = atoi(argv[3]); // размер буфера из аргументов
  char buf[bufsize]; // динамический буфер
  struct sockaddr_in servaddr;

  // создание TCP сокета
  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("ошибка создания сокета");
    exit(1);
  }

  // настройка адреса сервера
  memset(&servaddr, 0, SIZE);
  servaddr.sin_family = AF_INET;

  // преобразование IP-адреса
  if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
    perror("неверный IP-адрес");
    exit(1);
  }

  // установка порта сервера
  servaddr.sin_port = htons(atoi(argv[2]));

  // подключение к серверу
  if (connect(fd, (SADDR *)&servaddr, SIZE) < 0) {
    perror("ошибка подключения");
    exit(1);
  }

  write(1, "введите сообщение для отправки:\n", 31);
  
  // чтение из stdin и отправка на сервер
  while ((nread = read(0, buf, bufsize)) > 0) {
    if (write(fd, buf, nread) < 0) {
      perror("ошибка записи");
      exit(1);
    }
  }

  close(fd);
  exit(0);
}