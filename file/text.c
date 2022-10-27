#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

#include "file.h"
#include "text.h"

//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 
//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 

TextError TextConstructor(Text* const text)
{
    assert(text);
    assert(!text->Text);
    assert(!text->TextSize);

    return TEXT_ERR_NO_ERRORS;
}

void TextDestructor(Text* const text)
{
    assert(text);
    assert(text->Text);

    free(text->Text);

    text->Text     = NULL;
    text->TextSize = 0;
}

//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 
//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 

TextError TextReadFile(Text* const outText, const char* const fileName)
{
    assert(fileName);
    assert(outText);

    File file = {};
    if (FileOpen(&file, fileName, FILE_OPEN_READ_ONLY) != FILE_ERR_NO_ERRORS)
    {
        fprintf(stderr, "TextReadFile(). Ошибка открытия файла \"%s\".", fileName);
        return TEXT_ERR_OPEN_FILE;
    }

    off_t fileOffset = 0;
    if (FileGetSize(&file, &fileOffset) != FILE_ERR_NO_ERRORS)
    {
        assert(!"TextReadFile(). Ошибка получения размера файла.");
        return TEXT_ERR_GET_FILE_SIZE;
    }
    size_t fileSize = (size_t)fileOffset;

    char* buffer = (char*)calloc(fileSize + 1, sizeof(char));
    if (!buffer)
    {
        assert(!"TextReadFile(). Ошибка выделения памяти.");
        if (FileClose(&file) != FILE_ERR_NO_ERRORS)
        {
            assert(!"TextReadFile(). Ошибка закрытия файла.");
        }
        return TEXT_ERR_MEM;
    }

    if (FileRead(&file, buffer, fileSize) != FILE_ERR_NO_ERRORS)
    {
        assert(!"TextReadFile(). Ошибка чтения файла.");
        return TEXT_ERR_READ_FILE;
    }

    outText->TextSize = fileSize;
    outText->Text     = buffer;

    if (FileClose(&file) != FILE_ERR_NO_ERRORS)
    {
        assert(!"TextReadFile(). Ошибка закрытия файла.");
        return TEXT_ERR_CLOSE_FILE;
    }

    return TEXT_ERR_NO_ERRORS;
}

TextError TextCopy(const Text* const src, Text* const dst)
{
    assert(src);
    assert(dst);

    const char* const srcText = src->Text;
    const size_t      srcSize = src->TextSize;

    char* buffer = (char*)calloc(srcSize + 1, sizeof(char));
    if (!buffer)
    {
        assert(!"TextCopy(). Ошибка выделения памяти.");
        return TEXT_ERR_MEM;
    }

    memcpy(buffer, srcText, srcSize);

    dst->Text     = buffer;
    dst->TextSize = srcSize;
    return TEXT_ERR_NO_ERRORS;
}

//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 
//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 