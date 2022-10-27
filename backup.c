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
        printf("Копирование \"%s\" в \"%s\".\n", srcDir.FilePath, destDir.FilePath);
        if (!BashCp(srcDir.FilePath, destDir.FilePath))
        {
            fprintf(stderr, "Ошибка копирования файлов.\n");
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

static bool DirectoryComparator(const void* const elem1, const void* const elem2)
{
    assert(elem1);
    assert(elem2);

    FileInfo* dir1 = (FileInfo*)elem1;
    FileInfo* dir2 = (FileInfo*)elem2;

    return strncmp(dir1->FileName, dir2->FileName, FILE_NAME_SIZE) == 0;
}

static bool BackupClear(FileInfo* srcDir, FileInfo* destDir)
{
    assert(srcDir);
    assert(destDir);

    const size_t destFilesCount = destDir->Children.Size;
    for (size_t destIndex = 0; destIndex < destFilesCount; destIndex++)
    {
        FileInfo *destFile = (FileInfo *)ExtArrayGetElemAt(&destDir->Children, destIndex);
        size_t srcIndex = ExtArrayFindElem(&srcDir->Children, destFile, DirectoryComparator);

        if (srcIndex == -1)
        {
            printf("Удаление файла \"%s\".\n", destFile->FilePath);
            if (!BashRm(destFile->FilePath))
            {
                fprintf(stderr, "Ошибка копирования файлов.\n");
                return false;
            }
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

static bool BackupCopy(FileInfo *src, FileInfo *dest)
{
    assert(src);
    assert(dest);

    const size_t srcFilesCount = src->Children.Size;
    for (size_t srcIndex = 0; srcIndex < srcFilesCount; srcIndex++)
    {
        FileInfo *srcFile = (FileInfo *)ExtArrayGetElemAt(&src->Children, srcIndex);
        size_t destIndex = ExtArrayFindElem(&dest->Children, srcFile, DirectoryComparator);

        if (destIndex == -1)
        {
            printf("Рекурсивное копирование \"%s\" в \"%s\".\n", srcFile->FilePath, dest->FilePath);
            if (!BashCp(srcFile->FilePath, dest->FilePath))
            {
                fprintf(stderr, "Ошибка копирования файлов.\n");
                return false;
            }

            // FileInfo destInfo = {};
            // if (FileInfoConstructor(&destInfo, dest->FilePath) != FILE_ERR_NO_ERRORS)
            // {
            //     fprintf(stderr, "Ошибка создания FileInfo.\n");
            //     return false;
            // }

            // if (FileConcatPath(&destInfo, srcFile->FileName) != FILE_ERR_NO_ERRORS)
            // {
            //     fprintf(stderr, "Ошибка конкатенации путей.\n");
            //     return false;
            // }

            // FileInfoDestructor(&destInfo);
        }
        else
        {
            FileInfo *destFile = (FileInfo *)ExtArrayGetElemAt(&dest->Children, destIndex);
            if (srcFile->FileType == FILE_TYPE_DIR)
            {
                // printf("Создание директории \"%s\".\n", destFile->FilePath);
                // if (!BashMkdir(destFile->FilePath))
                // {
                //     fprintf(stderr, "Ошибка создания директории.\n");
                //     return false;
                // }

                // Если нет изменений времени в папке, то это не значит, что файлы не менялись
                //if (!CheckNeedCopy(srcFile, destFile))
                //    continue;

                printf("Умное копирование в директорию \"%s\".\n", destFile->FilePath);
                if (!BackupCopy(srcFile, destFile))
                {
                    fprintf(stderr, "Ошибка копирования файлов.\n");
                    return false;
                }
            }
            else
            {
                if (!CheckNeedCopy(srcFile, destFile))
                {
                    //printf("Пропускаем копирование \"%s\" в \"%s\".\n", srcFile->FilePath, destFile->FilePath);
                    continue;
                }
                    
                printf("Копирование файла \"%s\" в \"%s\".\n", srcFile->FilePath, destFile->FilePath);
                if (!BashCp(srcFile->FilePath, destFile->FilePath))
                {
                    fprintf(stderr, "Ошибка копирования файлов.\n");
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