#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mutex1;
pthread_mutex_t mutex2;

void* thread_func1(void* arg) {
    // Блокировка первого мьютекса
    pthread_mutex_lock(&mutex1);
    printf("Thread 1: Locked mutex1\n");

    // Задержка для увеличения вероятности deadlock
    sleep(1);

    // Попытка блокировки второго мьютекса
    pthread_mutex_lock(&mutex2);
    printf("Thread 1: Locked mutex2\n");

    // Освобождение мьютексов
    pthread_mutex_unlock(&mutex2);
    pthread_mutex_unlock(&mutex1);
    
    return NULL;
}

void* thread_func2(void* arg) {
    // Блокировка второго мьютекса
    pthread_mutex_lock(&mutex2);
    printf("Thread 2: Locked mutex2\n");

    // Задержка для увеличения вероятности deadlock
    sleep(1);

    // Попытка блокировки первого мьютекса
    pthread_mutex_lock(&mutex1);
    printf("Thread 2: Locked mutex1\n");

    // Освобождение мьютексов
    pthread_mutex_unlock(&mutex1);
    pthread_mutex_unlock(&mutex2);
    
    return NULL;
}

int main() {
    pthread_t thread1, thread2;

    // Инициализация мьютексов
    pthread_mutex_init(&mutex1, NULL);
    pthread_mutex_init(&mutex2, NULL);

    // Создание потоков
    pthread_create(&thread1, NULL, thread_func1, NULL);
    pthread_create(&thread2, NULL, thread_func2, NULL);

    // Ожидание завершения потоков
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    // Освобождение ресурсов
    pthread_mutex_destroy(&mutex1);
    pthread_mutex_destroy(&mutex2);

    return 0;
}