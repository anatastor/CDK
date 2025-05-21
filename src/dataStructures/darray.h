
#pragma once

#include "def.h"


typedef struct _CDKDArrayHeader
{
    uint64 capacity;
    uint64 length;
    uint64 dataSize;
} _CDKDArrayHeader;


void* _cdk_darray_create (uint64 capacity, uint64 dataSize);

void* cdk_darray_destroy (void* array);

// uint64 _cdk_darry_get_field (void* array, uint64 field);
// void _cdk_darray_set_field (void* array, uint64 field, uint64 value);

void* _cdk_darray_resize (void* array);


uint64 cdk_darray_capacity (void* array);
uint64 cdk_darray_length (void* array);

void* _cdk_darray_insert (void* array, const void* valuePtr);
void* _cdk_darray_insertat (void* array, uint64 index, const void* valuePtr);

void* cdk_darray_remove (void* array, void* dest);
void* cdk_darray_removeat (void* array, uint64 index, void* dest);


#define CDK_DARRAY_DEFAULT_CAPACITY    1
#define CDK_DARRAY_RESIZE_FACTOR       2


#define cdk_darray_create(type) \
    _cdk_darray_create(CDK_DARRAY_DEFAULT_CAPACITY, sizeof (type))

#define cdk_darray_reserve(type, capacity) \
    _cdk_darray_create(capacity, sizeof (type))

#define cdk_darray_insert(array, value)         \
({                                              \
    typeof(value) tmp = value;                  \
    array = _cdk_darray_insert (array, &tmp);   \
})

#define cdk_darray_insertat(array, index, value)        \
{                                                       \
    typeof(value) temp = value;                         \
    array = _cdk_darray_insertat (array, index, &tmp);  \
}

