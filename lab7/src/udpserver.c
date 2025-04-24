#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define SADDR struct sockaddr
#define SLEN sizeof(struct sockaddr_in)

int main(int argc, char *argv[]) {
  // проверка аргументов командной строки
  if (argc != 3) {
    printf("использование: %s <порт> <размер_буфера>\n", argv[0]);
    exit(1);
  }

  int sockfd, n;
  const int port = atoi(argv[1]);          // порт из аргументов
  const int bufsize = atoi(argv[2]);       // размер буфера из аргументов
  char mesg[bufsize], ipadr[16];           // буферы динамического размера
  struct sockaddr_in servaddr, cliaddr;

  // создание UDP сокета
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("ошибка создания сокета");
    exit(1);
  }

  // настройка адреса сервера
  memset(&servaddr, 0, SLEN);
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);         // порт из аргументов

  // привязка сокета
  if (bind(sockfd, (SADDR *)&servaddr, SLEN) < 0) {
    perror("ошибка привязки");
    exit(1);
  }

  printf("сервер запущен на порту %d\n", port);

  // основной цикл сервера
  while (1) {
    unsigned int len = SLEN;

    // получение сообщения от клиента
    if ((n = recvfrom(sockfd, mesg, bufsize, 0, (SADDR *)&cliaddr, &len)) < 0) {
      perror("ошибка получения");
      exit(1);
    }
    mesg[n] = '\0';  // завершающий ноль для строки

    // вывод информации о запросе
    printf("запрос: %s\tот %s:%d\n", mesg,
           inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr, ipadr, sizeof(ipadr)),
           ntohs(cliaddr.sin_port));

    // отправка ответа клиенту
    if (sendto(sockfd, mesg, n, 0, (SADDR *)&cliaddr, len) < 0) {
      perror("ошибка отправки");
      exit(1);
    }
  }
}