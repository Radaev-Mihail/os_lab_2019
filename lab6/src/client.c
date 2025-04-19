#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include "utils.h"

// Структура для хранения информации о сервере
struct Server {
    char ip[255];  // IP-адрес сервера
    int port;      // Порт сервера
};

// Функция для преобразования строки в uint64_t с проверкой ошибок
bool ConvertStringToUI64(const char *str, uint64_t *val) {
    char *end = NULL;
    unsigned long long i = strtoull(str, &end, 10);  // Преобразование строки в число
    
    // Проверка на переполнение
    if (errno == ERANGE) {
        fprintf(stderr, "Out of uint64_t range: %s\n", str);
        return false;
    }
    
    // Проверка других ошибок
    if (errno != 0)
        return false;
    
    *val = i;  // Сохранение результата
    return true;
}

// Функция для чтения списка серверов из файла
void ReadServersFromFile(const char *filename, struct Server **servers, int *num_servers) {
    FILE *file = fopen(filename, "r");  // Открытие файла
    if (!file) {
        perror("Failed to open servers file");
        exit(EXIT_FAILURE);
    }

    char line[255];
    int count = 0;
    // Чтение файла построчно
    while (fgets(line, sizeof(line), file)) {
        // Динамическое выделение памяти для серверов
        *servers = realloc(*servers, sizeof(struct Server) * (count + 1));
        // Парсинг IP и порта из строки
        sscanf(line, "%s %d", (*servers)[count].ip, &(*servers)[count].port);
        count++;
    }

    *num_servers = count;  // Сохранение количества серверов
    fclose(file);  // Закрытие файла
}

int main(int argc, char **argv) {
    uint64_t k = -1;          // Число для вычисления факториала
    uint64_t mod = -1;        // Модуль
    char servers_file[255] = {'\0'};  // Путь к файлу с серверами

    // Парсинг аргументов командной строки
    while (true) {
        int current_optind = optind ? optind : 1;

        static struct option options[] = {
            {"k", required_argument, 0, 0},        // Опция для числа k
            {"mod", required_argument, 0, 0},      // Опция для модуля
            {"servers", required_argument, 0, 0},   // Опция для файла серверов
            {0, 0, 0, 0}
        };

        int option_index = 0;
        int c = getopt_long(argc, argv, "", options, &option_index);

        if (c == -1)  // Конец аргументов
            break;

        switch (c) {
        case 0: {
            switch (option_index) {
            case 0:  // Обработка --k
                ConvertStringToUI64(optarg, &k);
                break;
            case 1:  // Обработка --mod
                ConvertStringToUI64(optarg, &mod);
                break;
            case 2:  // Обработка --servers
                memcpy(servers_file, optarg, strlen(optarg));
                break;
            default:
                printf("Index %d is out of options\n", option_index);
            }
        } break;

        case '?':  // Неизвестная опция
            printf("Arguments error\n");
            break;
        default:  // Другие ошибки
            fprintf(stderr, "getopt returned character code 0%o?\n", c);
        }
    }

    // Проверка обязательных аргументов
    if (k == -1 || mod == -1 || !strlen(servers_file)) {
        fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n", argv[0]);
        return 1;
    }

    // Чтение списка серверов из файла
    struct Server *servers = NULL;
    int num_servers = 0;
    ReadServersFromFile(servers_file, &servers, &num_servers);

    uint64_t total_result = 1;  // Итоговый результат
    pthread_t *threads = malloc(sizeof(pthread_t) * num_servers);  // Потоки

    // Обработка каждого сервера
    for (int i = 0; i < num_servers; i++) {
        // Настройка адреса сервера
        struct sockaddr_in server;
        server.sin_family = AF_INET;  // IPv4
        server.sin_port = htons(servers[i].port);  // Порт
        struct hostent *hostname = gethostbyname(servers[i].ip);  // Получение IP по имени
        if (hostname == NULL) {
            fprintf(stderr, "gethostbyname failed with %s\n", servers[i].ip);
            exit(1);
        }
        server.sin_addr.s_addr = *((unsigned long *)hostname->h_addrtype);

        // Создание сокета
        int sck = socket(AF_INET, SOCK_STREAM, 0);
        if (sck < 0) {
            fprintf(stderr, "Socket creation failed!\n");
            exit(1);
        }

        // Подключение к серверу
        if (connect(sck, (struct sockaddr *)&server, sizeof(server)) < 0) {
            fprintf(stderr, "Connection failed\n");
            exit(1);
        }

        // Распределение задач между серверами
        uint64_t chunk_size = k / num_servers;
        uint64_t begin = i * chunk_size + 1;
        uint64_t end = (i == num_servers - 1) ? k : (begin + chunk_size - 1);

        // Формирование задачи для сервера
        char task[sizeof(uint64_t) * 3];
        memcpy(task, &begin, sizeof(uint64_t));  // Начало диапазона
        memcpy(task + sizeof(uint64_t), &end, sizeof(uint64_t));  // Конец диапазона
        memcpy(task + 2 * sizeof(uint64_t), &mod, sizeof(uint64_t));  // Модуль

        // Отправка задачи серверу
        if (send(sck, task, sizeof(task), 0) < 0) {
            fprintf(stderr, "Send failed\n");
            exit(1);
        }

        // Получение результата от сервера
        uint64_t response;
        if (recv(sck, &response, sizeof(response), 0) < 0) {
            fprintf(stderr, "Receive failed\n");
            exit(1);
        }

        // Обновление общего результата
        total_result = MultModulo(total_result, response, mod);
        close(sck);  // Закрытие сокета
    }

    // Вывод итогового результата
    printf("Final result: %llu\n", total_result);
    
    // Освобождение памяти
    free(servers);
    free(threads);
    return 0;
}