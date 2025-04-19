/********************************************************
 * Пример работы с мьютексами в многопоточном приложении
 * Демонстрирует:
 * - создание потоков
 * - разделение общей переменной
 * - синхронизацию с помощью мьютекса
 * - состояние гонки (при отключенном мьютексе)
 ********************************************************/

/* Подключаем необходимые заголовочные файлы */
#include <errno.h>      // Для работы с кодами ошибок
#include <pthread.h>    // Функции работы с потоками POSIX
#include <stdio.h>      // Стандартный ввод/вывод
#include <stdlib.h>     // Стандартные функции (exit)

/* Объявляем прототипы функций */
void do_one_thing(int *);       // Функция первого потока
void do_another_thing(int *);   // Функция второго потока
void do_wrap_up(int);           // Функция завершения

/* Глобальные переменные */
int common = 0;  // Общая переменная для двух потоков
int r1 = 0, r2 = 0, r3 = 0;  // Дополнительные переменные (не используются)

/* Инициализация мьютекса с помощью макроса PTHREAD_MUTEX_INITIALIZER.
   Это статический способ инициализации мьютекса. */
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

/* Основная функция */
int main() {
    pthread_t thread1, thread2;  // Идентификаторы потоков

    /* Создаем первый поток.
       Аргументы:
       - &thread1: указатель на идентификатор потока
       - NULL: атрибуты потока по умолчанию
       - (void *)do_one_thing: функция для выполнения в потоке
       - (void *)&common: аргумент для функции потока (указатель на common) */
    if (pthread_create(&thread1, NULL, (void *)do_one_thing,
              (void *)&common) != 0) {
        perror("pthread_create");  // Выводим сообщение об ошибке
        exit(1);                   // Выходим с кодом ошибки
    }

    /* Создаем второй поток (аналогично первому) */
    if (pthread_create(&thread2, NULL, (void *)do_another_thing,
                     (void *)&common) != 0) {
        perror("pthread_create");
        exit(1);
    }

    /* Ожидаем завершения первого потока */
    if (pthread_join(thread1, NULL) != 0) {
        perror("pthread_join");
        exit(1);
    }

    /* Ожидаем завершения второго потока */
    if (pthread_join(thread2, NULL) != 0) {
        perror("pthread_join");
        exit(1);
    }

    /* Выводим итоговое значение общей переменной */
    do_wrap_up(common);

    return 0;
}

/* Функция, выполняемая в первом потоке */
void do_one_thing(int *pnum_times) {
    int i, j, x;
    unsigned long k;
    int work;
    
    /* Каждый поток выполняет 50 итераций */
    for (i = 0; i < 50; i++) {
        /* Блокируем мьютекс перед доступом к общей переменной */
        pthread_mutex_lock(&mut);
        
        printf("doing one thing\n");  // Сообщение от первого потока
        work = *pnum_times;          // Читаем значение общей переменной
        printf("counter = %d\n", work);
        work++;  // Увеличиваем значение
        
        /* Искусственная задержка (имитация работы) */
        for (k = 0; k < 500000; k++)
            ;
            
        *pnum_times = work;  // Записываем новое значение обратно
        
        /* Разблокируем мьютекс */
        pthread_mutex_unlock(&mut);
    }
}

/* Функция, выполняемая во втором потоке (аналогична первой) */
void do_another_thing(int *pnum_times) {
    int i, j, x;
    unsigned long k;
    int work;
    for (i = 0; i < 50; i++) {
        pthread_mutex_lock(&mut);
        printf("doing another thing\n");  // Сообщение от второго потока
        work = *pnum_times;
        printf("counter = %d\n", work);
        work++;
        for (k = 0; k < 500000; k++)
            ;
        *pnum_times = work;
        pthread_mutex_unlock(&mut);
    }
}

/* Функция вывода итогового результата */
void do_wrap_up(int counter) {
    int total;
    printf("All done, counter = %d\n", counter);
}