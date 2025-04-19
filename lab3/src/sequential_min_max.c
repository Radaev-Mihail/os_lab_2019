#include <stdio.h>   // Для ввода-вывода (printf)
#include <stdlib.h>  // Для работы с памятью (malloc, free) и конвертации (atoi)

// Подключение пользовательских заголовочных файлов
#include "find_min_max.h"  // Содержит объявление структуры MinMax и функции GetMinMax
#include "utils.h"         // Содержит объявление функции GenerateArray

// Главная функция программы
int main(int argc, char **argv) {
  // Проверка количества аргументов командной строки
  // Ожидается 2 аргумента (seed и array_size) + имя программы (argv[0])
  if (argc != 3) {
    printf("Usage: %s seed arraysize\n", argv[0]);  // Вывод правильного формата вызова
    return 1;  // Возврат кода ошибки
  }

  // Преобразование первого аргумента (seed) в число
  int seed = atoi(argv[1]);
  // Проверка что seed - положительное число
  if (seed <= 0) {
    printf("seed is a positive number\n");
    return 1;  // Возврат кода ошибки
  }

  // Преобразование второго аргумента (array_size) в число
  int array_size = atoi(argv[2]);
  // Проверка что array_size - положительное число
  if (array_size <= 0) {
    printf("array_size is a positive number\n");
    return 1;  // Возврат кода ошибки
  }

  // Выделение памяти под массив
  int *array = malloc(array_size * sizeof(int));
  // Проверка успешности выделения памяти (в реальном коде должна быть)
  // if (array == NULL) { ... }

  // Генерация массива случайных чисел
  GenerateArray(array, array_size, seed);

  // Поиск минимального и максимального элементов во всем массиве
  // (от индекса 0 до array_size-1)
  struct MinMax min_max = GetMinMax(array, 0, array_size - 1);

  // Освобождение выделенной памяти
  free(array);

  // Вывод результатов
  printf("min: %d\n", min_max.min);
  printf("max: %d\n", min_max.max);

  // Успешное завершение программы
  return 0;
}