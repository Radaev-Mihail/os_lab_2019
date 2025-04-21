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
#include "utils.h"

// аргументы для вычисления факториала
struct factorial_args {
    uint64_t begin;
    uint64_t end;
    uint64_t mod;
};

// вычисление факториала в заданном диапазоне
uint64_t calculate_factorial(const struct factorial_args *args) {
    uint64_t result = 1;
    for (uint64_t i = args->begin; i <= args->end; i++) {
        result = MultModulo(result, i, args->mod);
    }
    return result;
}

// функция для выполнения в потоке
void *thread_factorial(void *args) {
    struct factorial_args *fargs = (struct factorial_args *)args;
    uint64_t *result = malloc(sizeof(uint64_t));
    *result = calculate_factorial(fargs);
    return result;
}

int main(int argc, char **argv) {
    int tnum = -1;
    int port = -1;

    // разбор аргументов командной строки
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
            fprintf(stderr, "неизвестный аргумент\n");
            return 1;
        }
    }

    // проверка обязательных аргументов
    if (port == -1 || tnum == -1) {
        fprintf(stderr, "использование: %s --port <порт> --tnum <количество потоков>\n", argv[0]);
        return 1;
    }

    // создание сокета
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("ошибка создания сокета");
        return 1;
    }

    // настройка адреса сервера
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("ошибка привязки сокета");
        return 1;
    }

    if (listen(server_fd, 128) < 0) {
        perror("ошибка прослушивания");
        return 1;
    }

    printf("сервер запущен на порту %d\n", port);

    // основной цикл сервера
    while (true) {
        struct sockaddr_in client;
        socklen_t client_len = sizeof(client);
        int client_fd = accept(server_fd, (struct sockaddr *)&client, &client_len);

        if (client_fd < 0) {
            perror("ошибка принятия соединения");
            continue;
        }

        // получение данных от клиента
        uint64_t args[3];
        if (recv(client_fd, args, sizeof(args), 0) <= 0) {
            perror("ошибка получения данных");
            close(client_fd);
            continue;
        }

        uint64_t begin = args[0];
        uint64_t end = args[1];
        uint64_t mod = args[2];
        printf("получено: %lu %lu %lu\n", begin, end, mod);

        // распределение работы между потоками
        pthread_t threads[tnum];
        struct factorial_args fargs[tnum];
        uint64_t chunk_size = (end - begin + 1) / tnum;
        uint64_t total = 1;

        for (int i = 0; i < tnum; i++) {
            fargs[i].begin = begin + i * chunk_size;
            fargs[i].end = (i == tnum - 1) ? end : (begin + (i + 1) * chunk_size - 1);
            fargs[i].mod = mod;

            if (pthread_create(&threads[i], NULL, thread_factorial, &fargs[i]) != 0) {
                perror("ошибка создания потока");
                close(client_fd);
                return 1;
            }
        }

        // сбор результатов от потоков
        for (int i = 0; i < tnum; i++) {
            uint64_t *result;
            pthread_join(threads[i], (void **)&result);
            total = MultModulo(total, *result, mod);
            free(result);
        }

        printf("результат: %lu\n", total);

        // отправка результата клиенту
        if (send(client_fd, &total, sizeof(total), 0) <= 0) {
            perror("ошибка отправки данных");
        }

        close(client_fd);
    }

    return 0;
}