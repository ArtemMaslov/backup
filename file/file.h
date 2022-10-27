#ifndef FILE_H
#define FILE_H

#include <stddef.h>
#include <stdbool.h>

#include <dirent.h>

#include <sys/types.h>

#include "../ext_array/ext_array.h"

//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 
//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 

typedef enum file_state_e
{
    // File не был инициализирован.
    FILE_STATE_INIT       = 0,
    // В File возникла ошибка.
    FILE_STATE_ERROR      = 1,

    // Можно только читать информацию.
    FILE_STATE_READ_ONLY  = 10,
    // Можно только записывать информацию.
    FILE_STATE_WRITE_ONLY = 11,
    // Можно записывать и читать информацию.
    FILE_STATE_READ_WRITE = 12
} FileState;

typedef enum file_open_type_e
{
    FILE_OPEN_READ_ONLY  = 0,
    FILE_OPEN_WRITE_ONLY = 1,
    FILE_OPEN_READ_WRITE = 2
} FileOpenType;

typedef enum file_error_e
{
    FILE_ERR_NO_ERRORS,

    FILE_ERR_OPEN_FILE,
    FILE_ERR_CLOSE_FILE,

    FILE_ERR_OPEN_DIR,
    FILE_ERR_DIR_LIST_END,
    FILE_ERR_CLOSE_DIR,
    
    FILE_ERR_READ_FILE,
    FILE_ERR_WRITE_FILE,
    
    FILE_ERR_READ_STATUS,
    FILE_ERR_LOCALE,

    FILE_ERR_MEM
} FileError;

typedef struct file_t
{
    int         FileDescr;
    const char* FileName;
    FileState   State;
    union
    {
        ssize_t ReadByteCount;
        ssize_t WrittenByteCount;
    };
} File;

typedef enum directory_state_e
{
    DIR_STATE_CLOSED = 0,
    DIR_STATE_OPENED = 1,
} DirectoryState;

typedef struct directory_info_t
{
    DIR* Dir;
    DirectoryState State;
} DirectoryInfo;

typedef enum file_type_e
{
    FILE_TYPE_UNKNOWN = 0,
    FILE_TYPE_DIR,
    FILE_TYPE_FILE,
} FileType;

#define FILE_NAME_SIZE 256
#define FILE_PATH_SIZE 4096
#define FILE_INFO_CHILDREN_CAPACITY 32

typedef struct file_info_t
{
    char      FileName[FILE_NAME_SIZE];

    char*     FilePath;
    size_t    FilePathSize;

    FileType  FileType;
    time_t    Time;

    ExtArray  Children;

    bool      Exist;
    bool      Gzip;
} FileInfo;

//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 
//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 

extern File* const StandardOutput;
extern File* const StandardInput;
extern File* const StandardError;

//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 
//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 

FileError FileInit();

FileError FileOpen(File* const file, const char* const fileName, const FileOpenType openType);

FileError FileRead(File* const file, char* const outBuffer, const size_t readCount);

FileError FileWrite(File* const file, const char* const buffer, const size_t writeCount);

FileError FileGetStat(FileInfo* const fileInfo);

FileError FileClose(File* const file);

FileError FileGetSize(File* const file, off_t* const outFileSize);

//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 
//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 

FileError DirectoryListStart(const char* const path, DirectoryInfo* const dirInfo);

FileError DirectoryListNext(DirectoryInfo* const dirInfo);

FileError DirectoryListEnd(DirectoryInfo* const dirInfo);

bool DirectoryCheckExist(FileInfo *dirInfo);

//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 
//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 

FileError FileInfoConstructor(FileInfo* const fileInfo, const char* const path);

FileError FileInfoAddChild(FileInfo* const parent, FileInfo* const child);

void FileInfoDestructor(FileInfo* const fileInfo);

FileError FileGetInfo(struct dirent* dirEntry, FileInfo* const fileInfo);

void FileConcatPath_(char* const path, const size_t pathSize, const char* const name, const size_t nameSize);

FileError FileConcatPath(FileInfo* const fileInfo, const char* name);

//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 
//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 

#endif // !FILE_H