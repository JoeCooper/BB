#include "encoding.h"

#define INDEX_RANGE_START 128
#define INDEX_RANGE_END 251
#define SINGLE_BYTE_UPPER_BOUND (INDEX_RANGE_END - INDEX_RANGE_START)

size_t encoding_get_byte_count(index_t index)
{

    if (index < SINGLE_BYTE_UPPER_BOUND)
    {
        return 1;
    }
    if(index < 256)
    {
        return 2;
    }
    if(index < 65536)
    {
        return 3;
    }
    return 4;
}