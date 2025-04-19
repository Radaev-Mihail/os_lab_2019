#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <getopt.h>
#include <signal.h>  // Добавлено для работы с сигналами

#include "find_min_max.h"
#include "utils.h"

int timeout_flag = 0;

void handle_alarm(int sig) {
    timeout_flag = 1;  // Устанавливаем флаг таймаута
}

int main(int argc, char **argv) {
    int seed = -1;
    int array_size = -1;
    int pnum = -1;
    bool with_files = false;
    int timeout = -1; // Переменная для хранения таймаута

    while (true) {
        int current_optind = optind ? optind : 1;

        static struct option options[] = {
            {"seed", required_argument, 0, 0},
            {"array_size", required_argument, 0, 0},
            {"pnum", required_argument, 0, 0},
            {"by_files", no_argument, 0, 'f'},
            {"timeout", required_argument, 0, 0}, // Опция для таймаута
            {0, 0, 0, 0}
        };

        int option_index = 0;
        int c = getopt_long(argc, argv, "f", options, &option_index);

        if (c == -1) break;

        switch (c) {
            case 0:
                switch (option_index) {
                    case 0:
                        seed = atoi(optarg);
                        if (seed <= 0) {
                            printf("Seed should be a positive number\n");
                            return 1;
                        }
                        break;
                    case 1:
                        array_size = atoi(optarg);
                        if (array_size <= 0) {
                            printf("Array size should be a positive number\n");
                            return 1;
                        }
                        break;
                    case 2:
                        pnum = atoi(optarg);
                        if (pnum <= 0 || pnum > array_size) {
                            printf("Pnum should be a positive number less than or equal to array size\n");
                            return 1;
                        }
                        break;
                    case 3:
                        with_files = true;
                        break;
                    case 4:
                        timeout = atoi(optarg);
                        if (timeout <= 0) {
                            printf("Timeout should be a positive number\n");
                            return 1;
                        }
                        break;
                    default:
                        printf("Index %d is out of options\n", option_index);
                }
                break;
            case 'f':
                with_files = true;
                break;
            case '?':
                break;
            default:
                printf("getopt returned character code 0%o?\n", c);
        }
    }

    if (seed == -1 || array_size == -1 || pnum == -1) {
        printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" [--timeout \"num\"]\n", argv[0]);
        return 1;
    }

    int *array = malloc(sizeof(int) * array_size);
    GenerateArray(array, array_size, seed);
    int active_child_processes = 0;
    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    int pipefd[2];
    if (!with_files) {
        if (pipe(pipefd) == -1) {
            perror("pipe");
            return 1;
        }
    }

    int segment_size = array_size / pnum;

    // Установка обработчика сигнала
    signal(SIGALRM, handle_alarm);

    if (timeout > 0) {
        alarm(timeout); // Устанавливаем таймер
    }

    for (int i = 0; i < pnum; i++) {
        pid_t child_pid = fork();
        if (child_pid >= 0) {
            active_child_processes += 1;
            if (child_pid == 0) {
                struct MinMax min_max = GetMinMax(array, i * segment_size, (i + 1) * segment_size);

                if (with_files) {
                    char filename[16];
                    snprintf(filename, sizeof(filename), "temp%d.txt", i);
                    FILE *file = fopen(filename, "w");
                    if (file == NULL) {
                        perror("File opening failed");
                        return 1;
                    }
                    fprintf(file, "%d %d\n", min_max.min, min_max.max);
                    fclose(file);
                } else {
                    write(pipefd[1], &min_max, sizeof(struct MinMax));
                }
                return 0;
            }
        } else {
            printf("Fork failed!\n");
            return 1;
        }
    }

    if (!with_files) {
        close(pipefd[1]);
    }

    // Основной цикл ожидания дочерних процессов
    while (active_child_processes > 0) {
        pid_t pid = waitpid(-1, NULL, WNOHANG);
        if (pid > 0) {
            active_child_processes--;
        }

        // Проверка флага таймаута
        if (timeout_flag) {
            // Отправляем SIGKILL всем дочерним процессам
            for (int j = 0; j < pnum; j++) {
                kill(j, SIGKILL);
            }
            printf("Timeout reached! Child processes have been killed.\n");
            break;
        }
    }

    struct MinMax min_max;
    min_max.min = INT_MAX;
    min_max.max = INT_MIN;

    for (int i = 0; i < pnum; i++) {
        int min = INT_MAX;
        int max = INT_MIN;

        if (with_files) {
            char filename[16];
            snprintf(filename, sizeof(filename), "temp%d.txt", i);
            FILE *file = fopen(filename, "r");
            if (file == NULL) {
                perror("File opening failed");
                return 1;
            }
            fscanf(file, "%d %d", &min, &max);
            fclose(file);
            remove(filename);
        } else {
            struct MinMax temp;
            read(pipefd[0], &temp, sizeof(struct MinMax));
            min = temp.min;
            max = temp.max;
        }
        if (min < min_max.min) min_max.min = min;
        if (max > min_max.max) min_max.max = max;
    }

    if (!with_files) {
        close(pipefd[0]);
    }

    struct timeval finish_time;
    gettimeofday(&finish_time, NULL);

    double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
    elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

    free(array);

    printf("Min: %d\n", min_max.min);
    printf("Max: %d\n", min_max.max);
    printf("Elapsed time: %fms\n", elapsed_time);
    fflush(NULL);
    return 0;
}