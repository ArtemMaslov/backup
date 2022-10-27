#ifndef LOGS_H
#define LOGS_H

#include <stdbool.h>

//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 
//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 

typedef enum log_error_e
{
    LOGS_ERR_NO_ERRORS = 0,
    LOGS_ERR_LOCALE    = 1,
} LogError;

extern bool LogsInited;

void PrintErrno(const char* const message, 
    const char* const funct_name, const char* const file, const size_t line);

#define PRINT_ERRNO_M(message) PrintErrno(message, __PRETTY_FUNCTION__, __FILE__, __LINE__);
#define PRINT_ERRNO() PrintErrno("", __PRETTY_FUNCTION__, __FILE__, __LINE__);

LogError LogsInit();

//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 
//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 

#endif // !LOGS_H