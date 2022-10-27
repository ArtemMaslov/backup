#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ext_array.h"

static ExtArrayError ExtArrayResize(ExtArray* const extArray);

//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 
//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 

ExtArrayError ExtArrayConstructor(ExtArray* const extArray, const size_t capacity, const size_t elemSize)
{
    assert(extArray);
    assert(capacity);
    assert(elemSize);

    void* data = calloc(capacity, elemSize);
    if (!data)
    {
        assert(!"ExtArrayConstructor(). Ошибка выделения памяти.");
        return EXT_ARRAY_ERR_MEM;
    }

    extArray->Data     = data;
    extArray->Size     = 0;
    extArray->Capacity = capacity;
    extArray->ElemSize = elemSize;

    return EXT_ARRAY_ERR_NO_ERRORS;
}

void ExtArrayDestructor(ExtArray* const extArray)
{
    assert(extArray);

    free(extArray->Data);
    
    memset(extArray, 0, sizeof(ExtArray));
}

void* ExtArrayGetElemAt(ExtArray* const extArray, const size_t index)
{
    return extArray->Data + index * extArray->ElemSize;
}

size_t ExtArrayFindElem(ExtArray* const extArray, const void* const elem, bool (*comp)(const void* const elem1, const void* const elem2))
{
    assert(extArray);
    assert(elem);

    const size_t elemCount = extArray->Size;
    for (size_t st = 0; st < elemCount; st++)
    {
        void* arrayElem = ExtArrayGetElemAt(extArray, st);
        if (comp(arrayElem, elem))
            return st;
    }

    return -1;
}

void* ExtArrayAddElem(ExtArray* const extArray, const void* const elem)
{
    assert(extArray);
    assert(elem);

    if (extArray->Size >= extArray->Capacity)
    {
        if (ExtArrayResize(extArray) != EXT_ARRAY_ERR_NO_ERRORS)
            return NULL;
    }

    void* const dest = ExtArrayGetElemAt(extArray, extArray->Size);
    memcpy(dest, elem, extArray->ElemSize);
    extArray->Size++;

    return dest;
}

static ExtArrayError ExtArrayResize(ExtArray* const extArray)
{
    assert(extArray);

    const size_t oldCapacity = extArray->Capacity;
    const size_t newCapacity = oldCapacity * EXT_ARRAY_RESIZE_COEF;

    // Целочисленное переполнение
    if (newCapacity < oldCapacity)
    {
        assert(!"ExtArrayResize(). Ошибка выделения памяти.");
        return EXT_ARRAY_ERR_MEM;
    }

    void* newData = realloc(extArray->Data, newCapacity * extArray->ElemSize);
    if (!newData)
    {
        assert(!"ExtArrayResize(). Ошибка выделения памяти.");
        return EXT_ARRAY_ERR_MEM;
    }

    extArray->Data     = newData;
    extArray->Capacity = newCapacity;

    void* const clearMemory = ExtArrayGetElemAt(extArray, oldCapacity);
    memset(clearMemory, 0, (newCapacity - oldCapacity) * extArray->ElemSize);

    return EXT_ARRAY_ERR_NO_ERRORS;
}

//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 
//***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***// 