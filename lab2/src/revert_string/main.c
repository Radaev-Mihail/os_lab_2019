#include <stdio.h>	// Для работы с вводом-выводом (printf)
#include <stdlib.h>	// Для работы с динамической памятью (malloc/free)
#include <string.h>	// Для работы со строками (strlen, strcpy)

#include "revert_string.h"

int main(int argc, char *argv[])
{
	// Проверка количества аргументов (должен быть ровно 1 аргумент + имя программы)
	if (argc != 2)
	{
		// Вывод сообщения об использовании программы
		printf("Usage: %s string_to_revert\n", argv[0]);
		return -1;	 // Возврат кода ошибки
	}

    // Выделение памяти для копии строки:
    // 1. strlen(argv[1]) - длина введённой строки
    // 2. +1 для нуль-терминатора ('\0')
    // 3. sizeof(char) - размер одного символа (обычно 1 байт)
	char *reverted_str = malloc(sizeof(char) * (strlen(argv[1]) + 1));

	// Копирование исходной строки в выделенную память
	strcpy(reverted_str, argv[1]);

	// Вызов функции реверсирования строки (определена в revert_string.c)
	RevertString(reverted_str);

	// Вывод результата
	printf("Reverted: %s\n", reverted_str);

	// Освобождение выделенной памяти
	free(reverted_str);

	return 0;
}

