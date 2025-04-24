// server.c
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include "multmodulo.h"
struct FactorialArgs {
    uint64_t begin;
    uint64_t end;
    uint64_t mod;
};



uint64_t Factorial(const struct FactorialArgs *args) {
    uint64_t ans = 1;
    for (uint64_t i = args->begin; i <= args->end; i++) {
        ans = MultModulo(ans, i, args->mod);
    }
    return ans;
}

void *ThreadFactorial(void *args) {
    struct FactorialArgs *fargs = (struct FactorialArgs *)args;
    uint64_t *result = malloc(sizeof(uint64_t));
    *result = Factorial(fargs);
    return result;
}

int main(int argc, char **argv) {
    int tnum = -1;
    int port = -1;

    while (true) {
        static struct option options[] = {
            {"port", required_argument, 0, 0},
            {"tnum", required_argument, 0, 0},
            {0, 0, 0, 0}};

        int option_index = 0;
        int c = getopt_long(argc, argv, "", options, &option_index);

        if (c == -1)
            break;

        switch (c) {
        case 0:
            if (strcmp(options[option_index].name, "port") == 0) {
                port = atoi(optarg);
            } else if (strcmp(options[option_index].name, "tnum") == 0) {
                tnum = atoi(optarg);
            }
            break;
        default:
            fprintf(stderr, "Unknown argument\n");
            return 1;
        }
    }

    if (port == -1 || tnum == -1) {
        fprintf(stderr, "Usage: %s --port <port> --tnum <threads>\n", argv[0]);
        return 1;
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(server_fd, 128) < 0) {
        perror("listen");
        return 1;
    }

    printf("Server listening at %d\n", port);

    while (true) {
        struct sockaddr_in client;
        socklen_t client_len = sizeof(client);
        int client_fd = accept(server_fd, (struct sockaddr *)&client, &client_len);

        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        uint64_t args[3];
        if (recv(client_fd, args, sizeof(args), 0) <= 0) {
            perror("recv");
            close(client_fd);
            continue;
        }

        uint64_t begin = args[0];
        uint64_t end = args[1];
        uint64_t mod = args[2];
        printf("Receive: %lu %lu %lu\n", begin, end, mod);

        pthread_t threads[tnum];
        struct FactorialArgs fargs[tnum];
        uint64_t chunk_size = (end - begin + 1) / tnum;
        uint64_t total = 1;

        for (int i = 0; i < tnum; i++) {
            fargs[i].begin = begin + i * chunk_size;
            fargs[i].end = (i == tnum - 1) ? end : (begin + (i + 1) * chunk_size - 1);
            fargs[i].mod = mod;

            if (pthread_create(&threads[i], NULL, ThreadFactorial, &fargs[i]) != 0) {
                perror("pthread_create");
                close(client_fd);
                return 1;
            }
        }

        for (int i = 0; i < tnum; i++) {
            uint64_t *result;
            pthread_join(threads[i], (void **)&result);
            total = MultModulo(total, *result, mod);
            free(result);
        }

        printf("Total: %lu\n", total);

        if (send(client_fd, &total, sizeof(total), 0) <= 0) {
            perror("send");
        }

        close(client_fd);
    }

    return 0;
}