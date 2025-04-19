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
#include "find_min_max.h"
#include "utils.h"

int main(int argc, char **argv) {
  // Инициализация параметров
  int seed = -1;         // Seed для генератора случайных чисел
  int array_size = -1;   // Размер массива
  int pnum = -1;         // Количество процессов
  bool with_files = false; // Флаг использования файлов для синхронизации

  // Обработка аргументов командной строки
  while (true) {
    int current_optind = optind ? optind : 1;

    // Определение возможных опций
    static struct option options[] = {
      {"seed", required_argument, 0, 0},       // --seed число
      {"array_size", required_argument, 0, 0}, // --array_size число
      {"pnum", required_argument, 0, 0},       // --pnum число
      {"by_files", no_argument, 0, 'f'},       // --by_files
      {0, 0, 0, 0}
    };

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break; // Выход при окончании аргументов

    switch (c) {
      case 0: // Обработка длинных опций
        switch (option_index) {
          case 0: // --seed
            seed = atoi(optarg);
            if (seed <= 0) {
              printf("Seed should be a positive number\n");
              return 1;
            }
            break;
          case 1: // --array_size
            array_size = atoi(optarg);
            if (array_size <= 0) {
              printf("Array size should be a positive number\n");
              return 1;
            }
            break;
          case 2: // --pnum
            pnum = atoi(optarg);
            if (pnum <= 0 || pnum > array_size) {
              printf("Pnum should be a positive number less than or equal to array size\n");
              return 1;
            }
            break;
          case 3: // --by_files
            with_files = true;
            break;
          default:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'f': // Краткая форма --by_files
        with_files = true;
        break;
      case '?': // Нераспознанная опция
        break;
      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  // Проверка обязательных аргументов
  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" \n", argv[0]);
    return 1;
  }

  // Создание и заполнение массива
  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
  
  int active_child_processes = 0;
  struct timeval start_time;
  gettimeofday(&start_time, NULL); // Замер времени начала

  // Создание pipe (если используется pipe-синхронизация)
  int pipefd[2];
  if (!with_files) {
    if (pipe(pipefd) == -1) {
      perror("pipe");
      return 1;
    }
  }

  // Разделение работы между процессами
  int segment_size = array_size / pnum;

  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork(); // Создание дочернего процесса
    
    if (child_pid >= 0) {
      active_child_processes += 1;
      
      if (child_pid == 0) { // Код, выполняемый в дочернем процессе
        // Поиск min/max в своем сегменте массива
        struct MinMax min_max = GetMinMax(array, i * segment_size, (i + 1) * segment_size);

        if (with_files) { // Вариант с файлами
          char filename[16];
          snprintf(filename, sizeof(filename), "temp%d.txt", i);
          FILE *file = fopen(filename, "w");
          if (file == NULL) {
            perror("File opening failed");
            return 1;
          }
          fprintf(file, "%d %d\n", min_max.min, min_max.max);
          fclose(file);
        } else { // Вариант с pipe
          write(pipefd[1], &min_max, sizeof(struct MinMax));
        }
        return 0; // Завершение дочернего процесса
      }
    } else {
      printf("Fork failed!\n");
      return 1;
    }
  }

  // Закрытие записи в pipe (в родительском процессе)
  if (!with_files) {
    close(pipefd[1]);
  }

  // Ожидание завершения всех дочерних процессов
  while (active_child_processes > 0) {
    wait(NULL);
    active_child_processes -= 1;
  }

  // Агрегация результатов
  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    int min = INT_MAX;
    int max = INT_MIN;

    if (with_files) { // Чтение из файлов
      char filename[16];
      snprintf(filename, sizeof(filename), "temp%d.txt", i);
      FILE *file = fopen(filename, "r");
      if (file == NULL) {
        perror("File opening failed");
        return 1;
      }
      fscanf(file, "%d %d", &min, &max);
      fclose(file);
      remove(filename); // Удаление временного файла
    } else { // Чтение из pipe
      struct MinMax temp;
      read(pipefd[0], &temp, sizeof(struct MinMax));
      min = temp.min;
      max = temp.max;
    }
    
    // Обновление глобальных min/max
    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
  }

  // Закрытие чтения из pipe
  if (!with_files) {
    close(pipefd[0]);
  }

  // Замер и вывод времени выполнения
  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  // Освобождение ресурсов
  free(array);

  // Вывод результатов
  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}