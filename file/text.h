#ifndef TEXT_H
#define TEXT_H

//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 
//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 

typedef enum text_error_e
{
    TEXT_ERR_NO_ERRORS        = 0,
    TEXT_ERR_OPEN_FILE     = 1,
    TEXT_ERR_MEM           = 2,
    TEXT_ERR_GET_FILE_SIZE = 3,
    TEXT_ERR_READ_FILE     = 4,
    TEXT_ERR_CLOSE_FILE    = 5
} TextError;

typedef struct text_t
{
    char*  Text;
    size_t TextSize;
} Text;

TextError TextConstructor(Text* const text);

void TextDestructor(Text* const text);

TextError TextReadFile(Text* const outText, const char* const fileName);

TextError TextCopy(const Text* const src, Text* const dst);

//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 
//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 

#endif // !TEXT_H