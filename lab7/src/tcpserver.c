#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SADDR struct sockaddr

int main(int argc, char *argv[]) {
  // проверка количества аргументов
  if (argc < 3) {
    printf("использование: %s <порт> <размер буфера>\n", argv[0]);
    exit(1);
  }

  // преобразование аргументов в числовые значения
  const int serv_port = atoi(argv[1]);
  const int bufsize = atoi(argv[2]);
  
  // вычисление размера структуры адреса
  const size_t kSize = sizeof(struct sockaddr_in);

  int lfd, cfd;
  int nread;
  char buf[bufsize];  // теперь размер буфера задается аргументом
  struct sockaddr_in servaddr;
  struct sockaddr_in cliaddr;

  // создание сокета
  if ((lfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    exit(1);
  }

  memset(&servaddr, 0, kSize);
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(serv_port);  // порт теперь из аргументов

  // привязка сокета
  if (bind(lfd, (SADDR *)&servaddr, kSize) < 0) {
    perror("bind");
    exit(1);
  }

  // переход в режим прослушивания
  if (listen(lfd, 5) < 0) {
    perror("listen");
    exit(1);
  }

  printf("сервер запущен на порту %d\n", serv_port);

  // основной цикл сервера
  while (1) {
    unsigned int clilen = kSize;

    // принятие соединения
    if ((cfd = accept(lfd, (SADDR *)&cliaddr, &clilen)) < 0) {
      perror("accept");
      exit(1);
    }
    printf("установлено соединение\n");

    // чтение данных от клиента
    while ((nread = read(cfd, buf, bufsize)) > 0) {
      write(1, buf, nread);
    }

    if (nread == -1) {
      perror("read");
      exit(1);
    }
    close(cfd);
  }
}