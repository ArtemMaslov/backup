#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>

#include <stdio.h>

#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "file.h"
#include "../logs/logs.h"

//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 
//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 

static bool FileInited       = false;
static File standardOutput = {};
static File standardInput  = {};
static File standardError  = {};

File* const StandardOutput = &standardOutput;
File* const StandardInput  = &standardInput;
File* const StandardError  = &standardError;

//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 
//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 

FileError FileInit()
{
    StandardInput->FileDescr  = STDIN_FILENO;
    StandardInput->State      = FILE_STATE_READ_ONLY;

    StandardOutput->FileDescr = STDOUT_FILENO;
    StandardOutput->State     = FILE_STATE_WRITE_ONLY;

    StandardError->FileDescr  = STDERR_FILENO;
    StandardOutput->State     = FILE_STATE_WRITE_ONLY;

    FileInited = true;
    return FILE_ERR_NO_ERRORS;
}

FileError FileOpen(File* const file, const char* const fileName, const FileOpenType openType)
{
    assert(FileInited);

    assert(file);
    assert(fileName);
    assert(file->State == FILE_STATE_INIT);

    int state     = FILE_STATE_INIT;
    int openFlags = 0;

    switch (openType)
    {
        case FILE_OPEN_READ_ONLY:
            state     = FILE_STATE_READ_ONLY;
            openFlags = O_RDONLY;
            break;

        case FILE_OPEN_WRITE_ONLY:
            state     = FILE_STATE_WRITE_ONLY;
            openFlags = O_WRONLY;
            break;

        case FILE_OPEN_READ_WRITE:
            state     = FILE_STATE_READ_WRITE;
            openFlags = O_RDWR;
            break;

        default:
            assert(!"FileOpen(). Unknown FileOpenType");
            break;
    }

    int fd = open(fileName, openFlags);
    if (fd == -1)
    {
        file->State = FILE_STATE_ERROR;
        PRINT_ERRNO_M("Ошибка открытия файлового дескриптора.");
        return FILE_ERR_OPEN_FILE;
    }

    file->FileDescr = fd;
    file->FileName  = fileName;
    file->State     = state;

    return FILE_ERR_NO_ERRORS;
}

FileError FileClose(File* const file)
{
    assert(FileInited);

    assert(file);
    //assert(file->State != FILE_STATE_ERROR);

    if (close(file->FileDescr) == -1)
    {
        file->State = FILE_STATE_ERROR;
        PRINT_ERRNO_M("Ошибка закрытия файла.");
        return FILE_ERR_CLOSE_FILE;
    }

    return FILE_ERR_NO_ERRORS;
}

//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 
//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***//  `

FileError FileRead(File* const file, char* const outBuffer, const size_t bufferSize)
{
    assert(FileInited);

    assert(file);
    assert(outBuffer);
    assert(bufferSize);
    assert(file->State == FILE_STATE_READ_ONLY || file->State == FILE_STATE_READ_WRITE);

    ssize_t readCount = read(file->FileDescr, outBuffer, bufferSize);
    if (readCount == -1)
    {
        file->State = FILE_STATE_ERROR;
        PRINT_ERRNO_M("Ошибка чтения файла.");
        return FILE_ERR_READ_FILE;
    }

    file->ReadByteCount = readCount;

    return FILE_ERR_NO_ERRORS;
}

FileError FileWrite(File* const file, const char* const buffer, const size_t bufferSize)
{
    assert(FileInited);

    assert(file);
    assert(buffer);
    assert(bufferSize);
    assert(file->State == FILE_STATE_WRITE_ONLY || file->State == FILE_STATE_READ_WRITE);

    ssize_t writtenCount = write(file->FileDescr, buffer, bufferSize);
    if (writtenCount == -1)
    {
        file->State = FILE_STATE_ERROR;
        PRINT_ERRNO_M("Ошибка записи в файл.");
        return FILE_ERR_READ_FILE;
    }

    file->WrittenByteCount = writtenCount;

    return FILE_ERR_NO_ERRORS;
}

//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 
//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 

FileError FileGetSize(File* const file, off_t* const outFileSize)
{
    assert(FileInited);

    assert(file);
    assert(file->State != FILE_STATE_ERROR && file->State != FILE_STATE_INIT);
    
    struct stat state = {};

    if (fstat(file->FileDescr, &state) == -1)
    {
        file->State = FILE_STATE_ERROR;
        PRINT_ERRNO_M("Ошибка чтения системной информации файла.");
        return FILE_ERR_READ_STATUS;
    }
    
    *outFileSize = state.st_size;

    return FILE_ERR_NO_ERRORS;
}

//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 
//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 

FileError DirectoryListStart(const char* const path, DirectoryInfo* const dirInfo)
{
    assert(path);
    assert(dirInfo);
    assert(dirInfo->State == DIR_STATE_CLOSED);

    DIR* dir = opendir(path);
    if (dir == NULL)
    {
        fprintf(stderr, "Ошибка открытия директории \"%s\"", path);
        PRINT_ERRNO();
        return FILE_ERR_OPEN_DIR;
    }

    dirInfo->Dir   = dir;
    dirInfo->State = DIR_STATE_OPENED;

    return FILE_ERR_NO_ERRORS;
}

FileError FileGetStat(FileInfo* const outFileInfo)
{
    assert(outFileInfo);

    struct stat statBuf = {};
    if (lstat(outFileInfo->FileName, &statBuf) == -1)
    {
        PRINT_ERRNO_M("Ошибка получения системной информации о файле");
        return FILE_ERR_READ_STATUS;
    }

    return FILE_ERR_NO_ERRORS;
}

FileError DirectoryListNext(DirectoryInfo* const dirInfo)
{
    assert(dirInfo);
    assert(dirInfo->State == DIR_STATE_OPENED);

    struct dirent* dirEntry = NULL;
    if ((dirEntry = readdir(dirInfo->Dir)) == NULL)
    {
        if (errno != 0)
        {
            PRINT_ERRNO_M("Ошибка чтения записи в директории");
            return FILE_ERR_READ_FILE;
        }
    }

    return FILE_ERR_NO_ERRORS;
}

FileError DirectoryListEnd(DirectoryInfo* const dirInfo)
{
    assert(dirInfo);
    assert(dirInfo->State == DIR_STATE_OPENED);

    if (closedir(dirInfo->Dir) == -1)
    {
        PRINT_ERRNO_M("Ошибка закрытия директории");
        return FILE_ERR_CLOSE_DIR;
    }

    dirInfo->Dir   = NULL;
    dirInfo->State = DIR_STATE_CLOSED;

    return FILE_ERR_NO_ERRORS;
}

bool DirectoryCheckExist(FileInfo *dirInfo)
{
    assert(dirInfo);

    DIR *dir = opendir(dirInfo->FilePath);
    if (dir != NULL)
    {
        if (closedir(dir) == -1)
        {
            PRINT_ERRNO_M("Ошибка закрытия директории");
            return false;
        }
        return true;
    }

    return false;
}

//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 
//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 

FileError FileInfoConstructor(FileInfo* const fileInfo, const char* const path)
{
    assert(fileInfo);
    assert(path);

    if (ExtArrayConstructor(&fileInfo->Children, FILE_INFO_CHILDREN_CAPACITY, sizeof(FileInfo)))
    { 
        fprintf(stderr, "Ошибка создания ExtArray.\n");
        return FILE_ERR_MEM;
    }

    const size_t pathSize = strlen(path);
    if (pathSize + 1 > FILE_PATH_SIZE)
    {
        fprintf(stderr, "Ошибка выделения памяти.\n");
        return FILE_ERR_MEM;
    }

    void* buffer = calloc(FILE_PATH_SIZE, sizeof(char));
    if (!buffer)
    {
        fprintf(stderr, "Ошибка выделения памяти.\n");
        return FILE_ERR_MEM;
    }

    memcpy(buffer, path, pathSize);

    fileInfo->FileType     = FILE_TYPE_UNKNOWN;
    fileInfo->FilePath     = buffer;
    fileInfo->FilePathSize = pathSize;
    fileInfo->Exist        = true;

    return FILE_ERR_NO_ERRORS;
}

void FileConcatPath_(char* const path, const size_t pathSize, const char* const name, const size_t nameSize)
{
    char* str = path + pathSize;
    *(str++) = '/';
    memcpy(str, name, nameSize);
}

FileError FileConcatPath(FileInfo* const fileInfo, const char* name)
{
    assert(fileInfo);
    assert(name);

    const size_t nameSize = strlen(name);

    // "/" + "name" + "\0"
    if (fileInfo->FilePathSize + nameSize + 2 >= FILE_PATH_SIZE)
    {
        fprintf(stderr, "Переполнение буфера пути");
        return FILE_ERR_MEM;
    }

    FileConcatPath_(fileInfo->FilePath, fileInfo->FilePathSize, name, nameSize);
    fileInfo->FilePathSize += nameSize + 1;

    return FILE_ERR_NO_ERRORS;
}

FileError FileGetInfo(struct dirent* dirEntry, FileInfo* const outFileInfo)
{
    assert(dirEntry);
    assert(outFileInfo);

    const char* const fileName = dirEntry->d_name;
    const size_t fileNameSize  = strlen(fileName);

    memcpy(outFileInfo->FileName, fileName, fileNameSize);

    if (FileConcatPath(outFileInfo, fileName) != FILE_ERR_NO_ERRORS)
    {
        return FILE_ERR_MEM;
    }

    const char* const filePath = outFileInfo->FilePath;
    
    struct stat statBuf = {};
    if (stat(filePath, &statBuf) == -1)
    {
        fprintf(stderr, 
            "fileName = \"%s\"\n"
            "filePath = \"%s\"\n",
            fileName, filePath);
        PRINT_ERRNO_M("Ошибка получения системной информации о файле");
        return FILE_ERR_READ_STATUS;
    }

    time_t ctime = statBuf.st_ctim.tv_sec;
    time_t mtime = statBuf.st_mtim.tv_sec;
    time_t maxTime = (ctime > mtime) ? ctime : mtime;
    outFileInfo->Time = maxTime;

    if (S_ISDIR(statBuf.st_mode))
        outFileInfo->FileType = FILE_TYPE_DIR;
    else
        outFileInfo->FileType = FILE_TYPE_FILE;
    
    return FILE_ERR_NO_ERRORS;
}

FileError FileInfoAddChild(FileInfo* const parent, FileInfo* const child)
{
    assert(parent);
    assert(child);

    if (ExtArrayAddElem(&parent->Children, child) == NULL)
    {
        fprintf(stderr, "Ошибка добавление элемента в ExtArray.\n");
        return FILE_ERR_MEM;
    }

    return FILE_ERR_NO_ERRORS;
}

void FileInfoDestructor(FileInfo* const fileInfo)
{
    assert(fileInfo);

    free(fileInfo->FilePath);

    const size_t childrenCount = fileInfo->Children.Size;
    for (size_t st = 0; st < childrenCount; st++)
    {
        FileInfoDestructor((FileInfo*)ExtArrayGetElemAt(&fileInfo->Children, st));
    }

    ExtArrayDestructor(&fileInfo->Children);
    memset(fileInfo, 0, sizeof(FileInfo));
}

//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 
//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 