#include <assert.h>
#include <stdio.h>

#include "logs/logs.h"
#include "file/file.h"
#include "backup.h"

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        puts("Не верное число аргументов");
        return -1;
    }

    if (FileInit() != FILE_ERR_NO_ERRORS)
    {
        puts("Не удаётся инициализировать библиотеку работы с файлами.");
        return -1;
    }

    if (LogsInit() != LOGS_ERR_NO_ERRORS)
    {
        puts("Ошибка инициализации логов");
        return -1;
    }

    DirectoryBackup(argv[1], argv[2]);

    return 0;
}