#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <getopt.h>
#include <time.h>

struct SumArgs {
  int *array;
  int begin;
  int end;
};

int Sum(const struct SumArgs *args) {
  int sum = 0;
  for (int i = args->begin; i < args->end; i++) {
    sum += args->array[i];
  }
  return sum;
}

void *ThreadSum(void *args) {
  struct SumArgs *sum_args = (struct SumArgs *)args;
  return (void *)(size_t)Sum(sum_args);
}

int main(int argc, char *argv[]) {
  uint32_t threads_num = 0;
  uint32_t array_size = 0;
  uint32_t seed = 0;

  // Обработка аргументов командной строки
  while (1) {
    static struct option long_options[] = {
      {"threads_num", required_argument, 0, 't'},
      {"array_size", required_argument, 0, 'a'},
      {"seed", required_argument, 0, 's'},
      {0, 0, 0, 0}
    };

    int option_index = 0;
    int c = getopt_long(argc, argv, "t:a:s:", long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {
      case 't':
        threads_num = atoi(optarg);
        break;
      case 'a':
        array_size = atoi(optarg);
        break;
      case 's':
        seed = atoi(optarg);
        break;
      default:
        printf("Usage: %s --threads_num <num> --array_size <size> --seed <num>\n", argv[0]);
        return 1;
    }
  }

  // Проверка на правильность ввода
  if (threads_num == 0 || array_size == 0 || seed == 0) {
    printf("Usage: %s --threads_num <num> --array_size <size> --seed <num>\n", argv[0]);
    return 1;
  }

  // Инициализация массива и генерация случайных чисел
  int *array = malloc(sizeof(int) * array_size);
  if (array == NULL) {
    printf("Error: unable to allocate memory for array\n");
    return 1;
  }
  
  srand(seed);
  for (uint32_t i = 0; i < array_size; i++) {
    array[i] = rand() % 100; // Числа в диапазоне от 0 до 99
  }

  // Динамическое выделение памяти для потоков и аргументов
  pthread_t *threads = malloc(sizeof(pthread_t) * threads_num);
  struct SumArgs *args = malloc(sizeof(struct SumArgs) * threads_num);

  if (threads == NULL || args == NULL) {
    printf("Error: unable to allocate memory for threads or args\n");
    free(array);
    return 1;
  }

  // Разбиение массива на части для потоков
  uint32_t chunk_size = array_size / threads_num;
  for (uint32_t i = 0; i < threads_num; i++) {
    args[i].array = array;
    args[i].begin = i * chunk_size;
    if (i == threads_num - 1) {
      args[i].end = array_size;  // Последний поток обрабатывает остаток
    } else {
      args[i].end = (i + 1) * chunk_size;
    }
    if (pthread_create(&threads[i], NULL, ThreadSum, (void *)&args[i])) {
      printf("Error: pthread_create failed!\n");
      free(array);
      free(threads);
      free(args);
      return 1;
    }
  }

  // Сбор результатов от потоков
  int total_sum = 0;
  for (uint32_t i = 0; i < threads_num; i++) {
    void *sum_ptr;
    pthread_join(threads[i], &sum_ptr);
    int sum = (int)(size_t)sum_ptr;
    total_sum += sum;
  }

  free(array);
  free(threads);
  free(args);

  printf("Total sum: %d\n", total_sum);
  return 0;
}