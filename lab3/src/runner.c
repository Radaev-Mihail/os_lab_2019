#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 3) { // Ожидаем два аргумента: seed и array_size
        fprintf(stderr, "Usage: %s seed array_size\n", argv[0]);
        return 1;
    }

    pid_t pid = fork(); // Создаем новый процесс

    if (pid < 0) {
        perror("Fork failed");
        return 1;
    }

    if (pid == 0) {
        // Дочерний процесс: запускаем sequential_min_max
        char *path = "./sequential_min_max"; 
        char *args[argc + 1]; // +1 для завершающего NULL

        args[0] = path; // Первый аргумент - путь к исполняемому файлу
        for (int i = 1; i < argc; i++) {
            args[i] = argv[i]; // Копируем аргументы
        }
        args[argc] = NULL; // Завершаем массив NULL

        // Для отладки: выводим переданные аргументы
        printf("Executing: %s ", path);
        for (int i = 1; i < argc; i++) {
            printf("%s ", args[i]);
        }
        printf("\n");

        // Запуск программы с помощью execv
        execv(path, args);

        // Если execv возвращает, значит произошла ошибка
        perror("execv failed");
        return 1;
    } else {
        // Родительский процесс: ждем завершения дочернего
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            printf("Child process exited with status %d\n", WEXITSTATUS(status));
        } else {
            printf("Child process did not terminate normally\n");
        }
    }

    return 0;
}