
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFSIZE 100
#define SADDR struct sockaddr

int main(int argc, char *argv[]) {
    int lfd, cfd;
    int nread;
    char buf[BUFSIZE];
    struct sockaddr_in servaddr;
    struct sockaddr_in cliaddr;

    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    const size_t kSize = sizeof(struct sockaddr_in);
    
    if ((lfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    memset(&servaddr, 0, kSize);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[1]));

    if (bind(lfd, (SADDR *)&servaddr, kSize) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(lfd, 5) < 0) {
        perror("listen");
        exit(1);
    }

    while (1) {
        unsigned int clilen = kSize;

        if ((cfd = accept(lfd, (SADDR *)&cliaddr, &clilen)) < 0) {
            perror("accept");
            exit(1);
        }
        printf("Connection established\n");

        while ((nread = read(cfd, buf, BUFSIZE)) > 0) {
            write(1, buf, nread);
        }

        if (nread == -1) {
            perror("read");
            exit(1);
        }
        close(cfd);
    }
}
