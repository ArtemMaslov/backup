#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>

#include <stdio.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "logs/logs.h"
#include "bash.h"

//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***//
//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***//

bool BashCp(const char *const src, const char *const dest)
{
    assert(src);
    assert(dest);

    pid_t id = fork();
    if (id == -1)
    {
        PRINT_ERRNO_M("Ошибка создания fork().\n");
        exit(-1);
    }
    else if (id == 0) // Ребёнок
    {
        char cmd[4096] = "";
        snprintf(cmd, 4096, "cp -r %s %s", src, dest);
        int status = execl("/bin/bash", "/bin/bash", "-c", cmd, NULL);
        if (status == -1)
        {
            fprintf(stderr, "Ошибка вызова команды \"/bin/bash -c \"%s\"\"\n", cmd);
            PRINT_ERRNO();
            exit(-1);
        }

        exit(0);
    }
    else // Родитель
    {
        int exitCode = 0;
        waitpid(id, &exitCode, 0);
        if (WEXITSTATUS(exitCode) == -1)
        {
            fprintf(stderr, "Ошибка выполнения дочернего процесса.\n");
            exit(-1);
        }
    }

    return true;
}

bool BashGzip(const char *const dir)
{
    assert(dir);

    pid_t id = fork();
    if (id == -1)
    {
        PRINT_ERRNO_M("Ошибка создания fork().\n");
        exit(-1);
    }
    else if (id == 0) // Ребёнок
    {
        char cmd[4096] = "";
        snprintf(cmd, 4096, "gzip -r %s", dir);
        int status = execl("/bin/bash", "/bin/bash", "-c", cmd, NULL);
        if (status == -1)
        {
            fprintf(stderr, "Ошибка вызова команды \"/bin/bash -c \"%s\"\"\n", cmd);
            PRINT_ERRNO();
            exit(-1);
        }

        exit(0);
    }
    else // Родитель
    {
        int exitCode = 0;
        waitpid(id, &exitCode, 0);
        if (WEXITSTATUS(exitCode) == -1)
        {
            fprintf(stderr, "Ошибка выполнения дочернего процесса.\n");
            exit(-1);
        }
    }

    return true;
}

bool BashMkdir(const char *const dir)
{
    assert(dir);

    pid_t id = fork();
    if (id == -1)
    {
        PRINT_ERRNO_M("Ошибка создания fork().\n");
        exit(-1);
    }
    else if (id == 0) // Ребёнок
    {
        char cmd[4096] = "";
        snprintf(cmd, 4096, "mkdir %s", dir);
        int status = execl("/bin/bash", "/bin/bash", "-c", cmd, NULL);
        if (status == -1)
        {
            fprintf(stderr, "Ошибка вызова команды \"/bin/bash -c \"%s\"\"\n", cmd);
            PRINT_ERRNO();
            exit(-1);
        }

        exit(0);
    }
    else // Родитель
    {
        int exitCode = 0;
        waitpid(id, &exitCode, 0);
        if (WEXITSTATUS(exitCode) == -1)
        {
            fprintf(stderr, "Ошибка выполнения дочернего процесса.\n");
            exit(-1);
        }
    }

    return true;
}

bool BashRm(const char *const dir)
{
    assert(dir);

    pid_t id = fork();
    if (id == -1)
    {
        PRINT_ERRNO_M("Ошибка создания fork().\n");
        exit(-1);
    }
    else if (id == 0) // Ребёнок
    {
        char cmd[4096] = "";
        snprintf(cmd, 4096, "rm -r %s", dir);
        int status = execl("/bin/bash", "/bin/bash", "-c", cmd, NULL);
        if (status == -1)
        {
            fprintf(stderr, "Ошибка вызова команды \"/bin/bash -c \"%s\"\"\n", cmd);
            PRINT_ERRNO();
            exit(-1);
        }

        exit(0);
    }
    else // Родитель
    {
        int exitCode = 0;
        waitpid(id, &exitCode, 0);
        if (WEXITSTATUS(exitCode) == -1)
        {
            fprintf(stderr, "Ошибка выполнения дочернего процесса.\n");
            exit(-1);
        }
    }

    return true;
}

//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***//
//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***//