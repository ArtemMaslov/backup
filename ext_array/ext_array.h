#ifndef EXT_ARRAY_H
#define EXT_ARRAY_H

#include <stddef.h>
#include <stdbool.h>

//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 
//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 

#define EXT_ARRAY_RESIZE_COEF 2

typedef enum ext_array_error_e
{
    EXT_ARRAY_ERR_NO_ERRORS = 0,
    EXT_ARRAY_ERR_MEM       = 1
} ExtArrayError;

typedef struct ext_array_t
{
    void*  Data;
    size_t ElemSize;
    size_t Size;
    size_t Capacity;
} ExtArray;

//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 
//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 

ExtArrayError ExtArrayConstructor(ExtArray* const extArray, const size_t capacity, const size_t elemSize);

void ExtArrayDestructor(ExtArray* const extArray);

void* ExtArrayGetElemAt(ExtArray* const extArray, const size_t index);

size_t ExtArrayFindElem(ExtArray* const extArray, const void* const elem, bool (*comp)(const void* const elem1, const void* const elem2));

void* ExtArrayAddElem(ExtArray* const extArray, const void* const elem);

//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 
//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 

#endif // !EXT_ARRAY_H