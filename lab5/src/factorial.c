#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

// Глобальные переменные
unsigned long long result = 1;  // Результат вычисления факториала
int k;                          // Число, факториал которого вычисляем
int mod;                        // Модуль
int num_threads;                // Количество потоков
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  // Мьютекс для синхронизации

// Структура для передачи данных в поток
typedef struct {
    int thread_id;
} thread_data;

// Функция, выполняемая каждым потоком
void* compute_factorial(void* arg) {
    thread_data* data = (thread_data*)arg;
    int thread_id = data->thread_id;
    
    // Вычисляем диапазон чисел для обработки этим потоком
    int start = thread_id * (k / num_threads) + 1;
    int end = (thread_id == num_threads - 1) ? k : (thread_id + 1) * (k / num_threads);
    
    unsigned long long partial_result = 1;
    
    // Вычисляем частичный факториал для своего диапазона
    for (int i = start; i <= end; i++) {
        partial_result = (partial_result * i) % mod;
    }
    
    // Захватываем мьютекс для обновления общего результата
    pthread_mutex_lock(&mutex);
    result = (result * partial_result) % mod;
    pthread_mutex_unlock(&mutex);
    
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    // Парсинг аргументов командной строки
    if (argc != 7) {
        printf("Использование: %s -k <число> -pnum <потоки> -mod <модуль>\n", argv[0]);
        return 1;
    }
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-k") == 0) {
            k = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-pnum") == 0) {
            num_threads = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-mod") == 0) {
            mod = atoi(argv[++i]);
        }
    }
    
    if (k < 0 || num_threads <= 0 || mod <= 0) {
        printf("Неверные параметры: k, pnum и mod должны быть положительными числами\n");
        return 1;
    }
    
    // Особые случаи
    if (k == 0 || k == 1) {
        printf("%d! mod %d = 1\n", k, mod);
        return 0;
    }
    
    // Создаем потоки
    pthread_t threads[num_threads];
    thread_data thread_args[num_threads];
    
    for (int i = 0; i < num_threads; i++) {
        thread_args[i].thread_id = i;
        int rc = pthread_create(&threads[i], NULL, compute_factorial, (void*)&thread_args[i]);
        if (rc) {
            printf("Ошибка при создании потока %d\n", i);
            exit(-1);
        }
    }
    
    // Ждем завершения всех потоков
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Выводим результат
    printf("%d! mod %d = %llu\n", k, mod, result);
    
    // Уничтожаем мьютекс
    pthread_mutex_destroy(&mutex);
    
    return 0;
}