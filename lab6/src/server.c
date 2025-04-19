#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include "utils.h"

// Структура для передачи аргументов в поток
struct FactorialArgs {
    uint64_t begin;  // Начало диапазона вычислений
    uint64_t end;    // Конец диапазона вычислений
    uint64_t mod;    // Модуль для вычислений
};

// Функция вычисления факториала в заданном диапазоне по модулю
uint64_t Factorial(const struct FactorialArgs *args) {
    uint64_t ans = 1;
    // Умножение всех чисел в диапазоне [begin, end] по модулю mod
    for (uint64_t i = args->begin; i <= args->end; i++) {
        ans = MultModulo(ans, i, args->mod);  // Функция из utils.h
    }
    return ans;
}

// Функция, выполняемая в потоке
void *ThreadFactorial(void *args) {
    struct FactorialArgs *fargs = (struct FactorialArgs *)args;
    // Выделяем память для результата (чтобы вернуть из потока)
    uint64_t *result = malloc(sizeof(uint64_t));
    *result = Factorial(fargs);  // Вычисляем факториал для своего диапазона
    return (void *)result;       // Возвращаем указатель на результат
}

int main(int argc, char **argv) {
    int tnum = -1;  // Количество потоков (по умолчанию -1)
    int port = -1;   // Порт сервера (по умолчанию -1)

    // Парсинг аргументов командной строки
    while (true) {
        int current_optind = optind ? optind : 1;

        static struct option options[] = {
            {"port", required_argument, 0, 0},  // Опция для порта
            {"tnum", required_argument, 0, 0},  // Опция для количества потоков
            {0, 0, 0, 0}
        };

        int option_index = 0;
        int c = getopt_long(argc, argv, "", options, &option_index);

        if (c == -1)  // Конец аргументов
            break;

        switch (c) {
        case 0: {
            switch (option_index) {
            case 0:
                port = atoi(optarg);  // Парсинг порта
                break;
            case 1:
                tnum = atoi(optarg);  // Парсинг количества потоков
                break;
            default:
                printf("Index %d is out of options\n", option_index);
            }
        } break;

        case '?':
            printf("Unknown argument\n");
            break;
        default:
            fprintf(stderr, "getopt returned character code 0%o?\n", c);
        }
    }

    // Проверка обязательных аргументов
    if (port == -1 || tnum == -1) {
        fprintf(stderr, "Using: %s --port 20001 --tnum 4\n", argv[0]);
        return 1;
    }

    // Создание TCP сокета
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Unable to create socket");
        return 1;
    }

    // Настройка адреса сервера
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;         // IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY; // Принимаем соединения на все интерфейсы
    server_addr.sin_port = htons(port);       // Указанный порт

    // Привязка сокета к адресу
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        return 1;
    }

    // Ожидание подключений (максимум 3 в очереди)
    listen(server_fd, 3);

    // Основной цикл сервера
    while (1) {
        // Принятие нового подключения
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int new_socket = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);

        // Получение задачи от клиента (begin, end, mod)
        uint64_t task[3];
        recv(new_socket, task, sizeof(task), 0);

        // Подготовка аргументов для вычислений
        struct FactorialArgs fargs;
        fargs.begin = task[0];
        fargs.end = task[1];
        fargs.mod = task[2];

        pthread_t threads[tnum];    // Массив идентификаторов потоков
        uint64_t results[tnum];    // Массив для результатов потоков

        // Создание потоков
        for (int i = 0; i < tnum; i++) {
            // Вычисление диапазона для текущего потока
            uint64_t range = (fargs.end - fargs.begin + 1) / tnum;
            uint64_t start = fargs.begin + i * range;
            uint64_t end = (i == tnum - 1) ? fargs.end : (start + range - 1);

            // Создание аргументов для потока
            struct FactorialArgs *thread_args = malloc(sizeof(struct FactorialArgs));
            *thread_args = (struct FactorialArgs){.begin = start, .end = end, .mod = fargs.mod};

            // Создание потока
            pthread_create(&threads[i], NULL, ThreadFactorial, thread_args);
        }

        // Ожидание завершения всех потоков и сбор результатов
        for (int i = 0; i < tnum; i++) {
            void *result;
            pthread_join(threads[i], &result);
            results[i] = *(uint64_t *)result;
            free(result);  // Освобождаем память, выделенную в потоке
        }

        // Объединение результатов всех потоков
        uint64_t final_result = 1;
        for (int i = 0; i < tnum; i++) {
            final_result = MultModulo(final_result, results[i], fargs.mod);
        }

        // Отправка результата клиенту
        send(new_socket, &final_result, sizeof(final_result), 0);
        close(new_socket);  // Закрытие соединения
    }

    return 0;
}