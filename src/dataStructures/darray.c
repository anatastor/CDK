
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

#include "dataStructures/darray.h"
#include "core/logger.h"


_CDKDArrayHeader*
_cdk_darray_get_header (void* array)
{
    uint64 headerSize = sizeof (_CDKDArrayHeader);
    return (_CDKDArrayHeader*)(array - headerSize);
}



void*
_cdk_darray_create (uint64 capacity, uint64 dataSize)
{
    uint64 headerSize = sizeof (_CDKDArrayHeader);
    uint64 arraySize = capacity * dataSize;
    uint64 totalSize = headerSize + arraySize;
    
    void* array = malloc (totalSize);
    memset (array, 0, totalSize);
    
    _CDKDArrayHeader* header = array;
    header->capacity = capacity;
    header->length = 0;
    header->dataSize = dataSize;

    return array + headerSize;
}


void*
cdk_darray_destroy (void* array)
{   
    _CDKDArrayHeader* header = _cdk_darray_get_header (array);
    free (header);
    return NULL;
}


void*
_cdk_darray_resize (void* array)
{
    _CDKDArrayHeader* header = _cdk_darray_get_header (array);

    void* newArray = _cdk_darray_create ((header->capacity * CDK_DARRAY_RESIZE_FACTOR), header->dataSize);
    _CDKDArrayHeader* newHeader = _cdk_darray_get_header (newArray);
    newHeader->length = header->length;

    memcpy (newArray, array, header->length * header->dataSize);

    cdk_darray_destroy (array);
    return newArray;
}


uint64
cdk_darray_capacity (void* array)
{
    _CDKDArrayHeader* header = _cdk_darray_get_header (array);
    return header->capacity;
}


uint64
cdk_darray_length (void* array)
{
    _CDKDArrayHeader* header = _cdk_darray_get_header (array);
    return header->length;
}


void
cdk_darray_set_length (void* array, uint64 length)
{
    _CDKDArrayHeader* header = _cdk_darray_get_header (array);
    header->length = length;
}


void*
_cdk_darray_insert (void* array, const void* valuePtr)
{
    _CDKDArrayHeader* header = _cdk_darray_get_header (array);

    if (header->length >= header->capacity)
    {
        array = _cdk_darray_resize (array);
        header = _cdk_darray_get_header (array);
    }


    uint64 offset = header->length * header->dataSize;
    memcpy (array + offset, valuePtr, header->dataSize);
    header->length++;
    return array;
}

void*
_cdk_darray_insertat (void* array, uint64 index, const void* valuePtr) 
{
    return NULL;
}

void*
cdk_darray_remove (void* array, void* dest)
{
    _CDKDArrayHeader* header = _cdk_darray_get_header (array);
    if (header->length < 1)
    {
        cdk_log_error ("cdk_darray_remove called on empty darray");
        return array;
    }

    uint64 offset = (header->length - 1) * header->dataSize;
    if (dest)
        memcpy (dest, array + offset, header->dataSize); 

    memset (array + offset, 0, header->dataSize);

    header->length--;
    return array;
}


void*
cdk_darray_removeat (void* array, uint64 index, void* dest)
{   
    _CDKDArrayHeader* header = _cdk_darray_get_header (array);
    if (header->length < 1)
    {
        cdk_log_error ("cdk_darray_removeat called on empty darray");
        return array;
    }

    if (index >= header->length)
    {
        cdk_log_error ("index out of bounds");
        return array;
    }

    uint64 offset = index * header->dataSize;
    if (dest)
        memcpy (dest, array + offset, header->dataSize); 
    
    // trim array to left
    if (index < header->length - 1)
        memcpy (array + offset,
                array + offset + header->dataSize,
                header->dataSize * (header->length - (index + 1)));

    // delete data
    // memset (array + header->dataSize * (header->length - 1), 0, header->dataSize);


    header->length--;
    return array;
}

