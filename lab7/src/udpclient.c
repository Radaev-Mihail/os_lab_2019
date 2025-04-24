#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define SADDR struct sockaddr
#define SLEN sizeof(struct sockaddr_in)

int main(int argc, char **argv) {
  // проверка количества аргументов (теперь нужно 3: IP, порт, размер буфера)
  if (argc != 4) {
    printf("использование: %s <IP_сервера> <порт> <размер_буфера>\n", argv[0]);
    exit(1);
  }

  int sockfd, n;
  const int bufsize = atoi(argv[3]); // получаем размер буфера из аргументов
  char sendline[bufsize], recvline[bufsize + 1]; // буферы динамического размера
  struct sockaddr_in servaddr;

  // настройка адреса сервера
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(atoi(argv[2])); // порт из аргументов

  // преобразование IP-адреса
  if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) < 0) {
    perror("ошибка преобразования IP-адреса");
    exit(1);
  }

  // создание UDP сокета
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("ошибка создания сокета");
    exit(1);
  }

  write(1, "введите строку для отправки:\n", 29);

  // основной цикл работы клиента
  while ((n = read(0, sendline, bufsize)) > 0) {
    // отправка данных на сервер
    if (sendto(sockfd, sendline, n, 0, (SADDR *)&servaddr, SLEN) == -1) {
      perror("ошибка отправки");
      exit(1);
    }

    // получение ответа от сервера
    if (recvfrom(sockfd, recvline, bufsize, 0, NULL, NULL) == -1) {
      perror("ошибка получения");
      exit(1);
    }

    printf("ответ от сервера: %s\n", recvline);
  }
  
  close(sockfd);
  return 0;
}