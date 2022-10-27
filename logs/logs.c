#include <assert.h>
#include <errno.h>
#include <string.h>
#include <locale.h>
#include <unistd.h>

#include <stdio.h>

#include <sys/types.h>

#include "logs.h"

//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 
//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 

locale_t SystemLocale = NULL;
bool LogsInited = false;

//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 
//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 

void PrintErrno(const char* const message, 
    const char* const funct_name, const char* const file, const size_t line)
{
    assert(LogsInited);
    assert(message);
    assert(funct_name);
    assert(file);

    fprintf(stderr,
        "%s at %zd in %s:\n"
        "\t%s\n"
        "\t%s.\n", 
        funct_name, line, file, message, strerror_l(errno, SystemLocale));
}

LogError LogsInit()
{
    SystemLocale = newlocale(LC_CTYPE_MASK | LC_NUMERIC_MASK | LC_TIME_MASK | 
        LC_COLLATE_MASK | LC_MONETARY_MASK | LC_MESSAGES_MASK, "", (locale_t)0);

    if (SystemLocale == (locale_t)0)
    {
        fprintf(stderr, 
            "FileInit().\n"
            "\tНе удаётся получить локализацию по умолчанию.\n"
            "\t%s.\n", strerror(errno));
        return LOGS_ERR_LOCALE;
    }

    LogsInited = true;
    return LOGS_ERR_NO_ERRORS;
}

//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 
//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 