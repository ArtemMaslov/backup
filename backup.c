#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include <stdio.h>

#include <sys/types.h>

#include "file/file.h"
#include "logs/logs.h"
#include "backup.h"
#include "bash.h"

//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***//
//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***//

static bool BackupCopy(FileInfo *src, FileInfo *dest);

static bool BackupReadDir(FileInfo *dirInfo);

static bool BackupClear(FileInfo* srcDir, FileInfo* destDir);

static void BackupTreeDump(FileInfo *dirInfo, size_t level);

static bool CheckNeedCopy(FileInfo* srcFile, FileInfo* destFile);

static bool FileMaskGzip(FileInfo* fileInfo);

static bool FileCheckGzip(const FileInfo* const fileInfo);

static bool DirectoryAddPath(FileInfo* dirInfo, const char* const path, const char* const fileName);

static bool DirectoryComparatorDest2(const void* const elem1, const void* const elem2);

static bool DirectoryComparatorDest1(const void* const elem1, const void* const elem2);

//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***//
//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***//

void DirectoryBackup(const char *const src, const char *const dest)
{
    assert(src);
    assert(dest);

    FileInfo srcDir = {};
    if (FileInfoConstructor(&srcDir, src) != FILE_ERR_NO_ERRORS)
    {
        fprintf(stderr, "Ошибка иницализации.\n");
        return;
    }

    FileInfo destDir = {};
    if (FileInfoConstructor(&destDir, dest) != FILE_ERR_NO_ERRORS)
    {
        FileInfoDestructor(&srcDir);
        fprintf(stderr, "Ошибка иницализации.\n");
        return;
    }

    if (!DirectoryCheckExist(&srcDir))
    {
        fprintf(stdout, "Внимание! Директория, для которой нужно сделать резервную копию не существует.");
        FileInfoDestructor(&srcDir);
        FileInfoDestructor(&destDir);
        return;
    }

    if (!BackupReadDir(&srcDir))
    {
        fprintf(stderr, "Ошибка чтения исходной директории.\n");
        FileInfoDestructor(&srcDir);
        FileInfoDestructor(&destDir);
        return;
    }

    if (DirectoryCheckExist(&destDir))
    {
        if (!BackupReadDir(&destDir))
        {
            fprintf(stderr, "Ошибка чтения директории назначения.\n");
            FileInfoDestructor(&srcDir);
            FileInfoDestructor(&destDir);
            return;
        }
        
        if (!BackupClear(&srcDir, &destDir))
        {
            FileInfoDestructor(&srcDir);
            FileInfoDestructor(&destDir);
            return;
        }

        if (!BackupCopy(&srcDir, &destDir))
        {
            FileInfoDestructor(&srcDir);
            FileInfoDestructor(&destDir);
            return;
        }
    }
    else
    {
        printf("Рекурсивное копирование \"%s\" в \"%s\".\n", srcDir.FilePath, destDir.FilePath);
        if (!BashCp(srcDir.FilePath, destDir.FilePath))
        {
            fprintf(stderr, "Ошибка копирования файлов.\n");
            FileInfoDestructor(&srcDir);
            FileInfoDestructor(&destDir);
            return;
        }

        printf("Рекурсивное сжатие \"%s\".\n", destDir.FilePath);
        if (!BashGzip(destDir.FilePath))
        {
            fprintf(stderr, "Ошибка сжатия файлов.\n");
            FileInfoDestructor(&srcDir);
            FileInfoDestructor(&destDir);
            return;
        }
    }

    FileInfoDestructor(&srcDir);
    FileInfoDestructor(&destDir);
}

//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***//
//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***//

static bool DirectoryComparatorDest2(const void* const elem1, const void* const elem2)
{
    assert(elem1);
    assert(elem2);

    FileInfo* srcDir = (FileInfo*)elem1;
    FileInfo* destDir = (FileInfo*)elem2;

    if (srcDir->FileType != destDir->FileType)
        return false;

    // Пропускаем удаленные файлы.
    if (destDir->FileName[0] == '\0')
        return false;

    // У файла игнорируем расширение ".gz"
    // У директории нет
    if (srcDir->FileType == FILE_TYPE_DIR)
        return strncmp(srcDir->FileName, destDir->FileName, FILE_NAME_SIZE) == 0;
    else
    {
        size_t len = strlen(destDir->FileName) - 3;
        return strncmp(srcDir->FileName, destDir->FileName, len) == 0;
    }
}

static bool DirectoryComparatorDest1(const void* const elem1, const void* const elem2)
{
    return DirectoryComparatorDest2(elem2, elem1);
}

static bool BackupClear(FileInfo* srcDir, FileInfo* destDir)
{
    assert(srcDir);
    assert(destDir);

    const size_t destFilesCount = destDir->Children.Size;
    for (size_t destIndex = 0; destIndex < destFilesCount; destIndex++)
    {
        FileInfo *destFile = (FileInfo *)ExtArrayGetElemAt(&destDir->Children, destIndex);
        size_t srcIndex = ExtArrayFindElem(&srcDir->Children, destFile, DirectoryComparatorDest2);

        if (destFile->FileType != FILE_TYPE_DIR && FileCheckGzip(destFile) == false)
        {
            printf("Удаление не сжатого файла \"%s\".\n", destFile->FilePath);
            if (!BashRm(destFile->FilePath))
            {
                fprintf(stderr, "Ошибка копирования файлов.\n");
                return false;
            }
            // Файл не сжат, его нужно убрать из дерева файлов. Иначе он может быть перепутан сжатым (при сравнении файлов расширение .gz игнорируется).
            destFile->FileName[0] = '\0';
        }
        else if (srcIndex == -1)
        {
            printf("Удаление файла \"%s\".\n", destFile->FilePath);
            if (!BashRm(destFile->FilePath))
            {
                fprintf(stderr, "Ошибка копирования файлов.\n");
                return false;
            }
            // Этот файл был удалён в исходной директории. Отбрасываем его для более быстрого сравнения.
            destFile->FileName[0] = '\0';
        }
        else
        {
            FileInfo *srcFile = (FileInfo *)ExtArrayGetElemAt(&srcDir->Children, srcIndex);
            if (srcFile->FileType == FILE_TYPE_DIR)
            {
                if (!BackupClear(srcFile, destFile))
                {
                    return false;
                }
            }
        }
    }

    return true;
}

static bool CheckNeedCopy(FileInfo* srcFile, FileInfo* destFile)
{
    assert(srcFile);
    assert(destFile);

    return srcFile->Time > destFile->Time;
}

static bool FileCheckGzip(const FileInfo* const fileInfo)
{
    assert(fileInfo);

    if (fileInfo->FilePathSize < 3)
        return false;

    char* ptr = fileInfo->FilePath;
    size_t st = fileInfo->FilePathSize - 3;
    if (ptr[st] == '.' && ptr[st + 1] == 'g' && ptr[st + 2] == 'z')
        return true;

    return false;
}

static bool FileMaskGzip(FileInfo* fileInfo)
{
    assert(fileInfo);

    if (fileInfo->FilePathSize < 3)
        return false;

    char* ptr = fileInfo->FilePath;
    size_t st = fileInfo->FilePathSize - 3;
    if (ptr[st] == '.' && ptr[st + 1] == 'g' && ptr[st + 2] == 'z')
    {
        ptr[st] = '\0';
        fileInfo->FilePathSize -= 3;
        return true;
    }

    return false;
}

static bool DirectoryAddPath(FileInfo* dirInfo, const char* const path, const char* const fileName)
{
    if (FileInfoConstructor(dirInfo, path) != FILE_ERR_NO_ERRORS)
    {
        fprintf(stderr, "Ошибка создания FileInfo.\n");
        return false;
    }

    if (FileConcatPath(dirInfo, fileName) != FILE_ERR_NO_ERRORS)
    {
        fprintf(stderr, "Ошибка конкатенации путей.\n");
        return false;
    }

    return true;
}

static bool BackupCopy(FileInfo *src, FileInfo *dest)
{
    assert(src);
    assert(dest);

    const size_t srcFilesCount = src->Children.Size;
    for (size_t srcIndex = 0; srcIndex < srcFilesCount; srcIndex++)
    {
        FileInfo *srcFile = (FileInfo *)ExtArrayGetElemAt(&src->Children, srcIndex);
        size_t destIndex = ExtArrayFindElem(&dest->Children, srcFile, DirectoryComparatorDest1);

        if (destIndex == -1)
        {
            printf("Файл не найден. Рекурсивное копирование \"%s\" в \"%s\".\n", srcFile->FilePath, dest->FilePath);
            if (!BashCp(srcFile->FilePath, dest->FilePath))
            {
                fprintf(stderr, "Ошибка копирования файлов.\n");
                return false;
            }

            FileInfo destInfo = {};
            if (!DirectoryAddPath(&destInfo, dest->FilePath, srcFile->FileName))
            {
                fprintf(stderr, "Ошибка инициализации.\n");
                FileInfoDestructor(&destInfo);
                return false;
            }

            if (!BashGzip(destInfo.FilePath))
            {
                fprintf(stderr, "Ошибка сжатия файлов.\n");
                FileInfoDestructor(&destInfo);
                return false;
            }

            FileInfoDestructor(&destInfo);
        }
        else
        {
            FileInfo *destFile = (FileInfo *)ExtArrayGetElemAt(&dest->Children, destIndex);
            if (srcFile->FileType == FILE_TYPE_DIR)
            {
                // Если нет изменений времени в папке, то это не значит, что файлы не менялись
                //if (!CheckNeedCopy(srcFile, destFile))
                //    continue;

                printf("Анализ вложенной директории \"%s\".\n", destFile->FilePath);
                if (!BackupCopy(srcFile, destFile))
                {
                    fprintf(stderr, "Ошибка копирования файлов.\n");
                    return false;
                }
            }
            else
            {
                if (!CheckNeedCopy(srcFile, destFile))
                    continue;
                
                printf("Копирование файла \"%s\" в \"%s\".\n", srcFile->FilePath, destFile->FilePath);
                
                if (!BashRm(destFile->FilePath))
                {
                    fprintf(stderr, "Ошибка удаления файла.\n");
                    return false;
                }

                FileMaskGzip(destFile);

                if (!BashCp(srcFile->FilePath, destFile->FilePath))
                {
                    fprintf(stderr, "Ошибка копирования файла.\n");
                    return false;
                }

                if (!BashGzip(destFile->FilePath))
                {
                    fprintf(stderr, "Ошибка удаления файла.\n");
                    return false;
                }
            }
        }
    }

    return true;
}

static bool BackupReadDir(FileInfo *dirInfo)
{
    assert(dirInfo);

    DIR *dir = opendir(dirInfo->FilePath);
    if (dir == NULL)
    {
        fprintf(stderr, "Ошибка открытия директории \"%s\"\n", dirInfo->FilePath);
        PRINT_ERRNO();
        return false;
    }

    struct dirent *dirEntry = NULL;
    while ((dirEntry = readdir(dir)) != NULL)
    {
        // Пропускаем указатели на текущую и предыдущую директории.
        if (strncmp(dirEntry->d_name, "..", 3) == 0 || strncmp(dirEntry->d_name, ".", 2) == 0)
            continue;

        FileInfo child = {};
        if (FileInfoConstructor(&child, dirInfo->FilePath) != FILE_ERR_NO_ERRORS)
        {
            fprintf(stderr, "Ошибка инициализации\n");
            return false;
        }

        if (FileGetInfo(dirEntry, &child) != FILE_ERR_NO_ERRORS)
        {
            fprintf(stderr, "Ошибка определения типа файла.\n");
            return false;
        }

        if (child.FileType == FILE_TYPE_DIR)
        {
            if (!BackupReadDir(&child))
            {
                closedir(dir);
                FileInfoDestructor(&child);
                fprintf(stderr, "Ошибка создания дерева директорий\n");
                return false;
            }
        }

        if (FileInfoAddChild(dirInfo, &child) != FILE_ERR_NO_ERRORS)
        {
            fprintf(stderr, "Ошибка создания дерева директорий\n");
            return false;
        }
    }

    if (errno != 0)
    {
        PRINT_ERRNO_M("Ошибка чтения записи в директории");
        return false;
    }

    if (closedir(dir) == -1)
    {
        PRINT_ERRNO_M("Ошибка закрытия директории");
        return false;
    }

    return true;
}

//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***//
//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***//

static void BackupTreeDump(FileInfo *dirInfo, size_t level)
{
    assert(dirInfo);

    for (size_t st = 0; st < level; st++)
        fputs("    ", stdout);

    puts(dirInfo->FileName);

    const size_t childrenCount = dirInfo->Children.Size;
    for (size_t st = 0; st < childrenCount; st++)
    {
        BackupTreeDump(ExtArrayGetElemAt(&dirInfo->Children, st), level + 1);
    }
}

//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***//
//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***//