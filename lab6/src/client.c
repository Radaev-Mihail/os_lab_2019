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
#include <arpa/inet.h>
#include "utils.h"

// структура для хранения информации о сервере
struct server {
    char ip[64];
    int port;
};

// чтение списка серверов из файла
int read_servers(const char *path, struct server **servers) {
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        fprintf(stderr, "ошибка: не удалось открыть файл %s\n", path);
        return -1;
    }

    size_t count = 0;
    size_t capacity = 4;
    *servers = malloc(capacity * sizeof(struct server));

    // чтение ip и порта каждого сервера из файла
    while (fscanf(file, "%63s %d", (*servers)[count].ip, &(*servers)[count].port) == 2) {
        count++;
        if (count >= capacity) {
            capacity *= 2;
            *servers = realloc(*servers, capacity * sizeof(struct server));
        }
    }

    fclose(file);
    return count;
}

// отправка задачи на сервер и получение результата
int send_task(const struct server *server, uint64_t begin, uint64_t end, uint64_t mod, uint64_t *result) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        fprintf(stderr, "ошибка: не удалось создать сокет\n");
        return -1;
    }

    // настройка адреса сервера
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server->port);
    inet_pton(AF_INET, server->ip, &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "ошибка: не удалось подключиться к серверу %s:%d\n", server->ip, server->port);
        close(sock);
        return -1;
    }

    // отправка задачи на сервер
    uint64_t task[3] = {begin, end, mod};
    if (send(sock, task, sizeof(task), 0) < 0) {
        fprintf(stderr, "ошибка: не удалось отправить данные на сервер\n");
        close(sock);
        return -1;
    }

    // получение результата от сервера
    if (recv(sock, result, sizeof(*result), 0) < 0) {
        fprintf(stderr, "ошибка: не удалось получить данные от сервера\n");
        close(sock);
        return -1;
    }

    close(sock);
    return 0;
}

int main(int argc, char **argv) {
    uint64_t k = 0;
    uint64_t mod = 0;
    char *servers_path = NULL;

    // разбор аргументов командной строки
    while (true) {
        int current_optind = optind ? optind : 1;

        static struct option options[] = {
            {"k", required_argument, 0, 'k'},
            {"mod", required_argument, 0, 'm'},
            {"servers", required_argument, 0, 's'},
            {0, 0, 0, 0}};

        int option_index = 0;
        int c = getopt_long(argc, argv, "", options, &option_index);

        if (c == -1)
            break;

        switch (c) {
        case 'k':
            k = strtoull(optarg, NULL, 10);
            break;
        case 'm':
            mod = strtoull(optarg, NULL, 10);
            break;
        case 's':
            servers_path = optarg;
            break;
        default:
            fprintf(stderr, "использование: %s --k <значение> --mod <значение> --servers <путь>\n", argv[0]);
            return 1;
        }
    }

    // проверка обязательных аргументов
    if (k == 0 || mod == 0 || servers_path == NULL) {
        fprintf(stderr, "использование: %s --k <значение> --mod <значение> --servers <путь>\n", argv[0]);
        return 1;
    }

    // загрузка списка серверов
    struct server *servers = NULL;
    int servers_count = read_servers(servers_path, &servers);
    if (servers_count <= 0) {
        fprintf(stderr, "ошибка: нет доступных серверов\n");
        return 1;
    }

    // распределение работы между серверами
    uint64_t chunk_size = k / servers_count;
    uint64_t remaining = k % servers_count;

    uint64_t total = 1;
    for (int i = 0; i < servers_count; i++) {
        uint64_t begin = i * chunk_size + 1;
        uint64_t end = (i + 1) * chunk_size;

        // добавление остатка к последнему серверу
        if (i == servers_count - 1)
            end += remaining;

        uint64_t result = 0;
        if (send_task(&servers[i], begin, end, mod, &result) < 0) {
            fprintf(stderr, "ошибка: задача не выполнена на сервере %s:%d\n", servers[i].ip, servers[i].port);
            free(servers);
            return 1;
        }

        total = MultModulo(total, result, mod);
    }

    printf("итоговый результат: %lu\n", total);

    free(servers);
    return 0;
}