#include "encoding.h"

#define INDEX_RANGE_START 128
#define INDEX_RANGE_END 251
#define SINGLE_BYTE_UPPER_BOUND (INDEX_RANGE_END - INDEX_RANGE_START)

void encoding_write(index_t index, FILE *restrict stream)
{
    const size_t count = encoding_get_byte_count(index);
    uint8_t header;
    if (count == 1)
    {
        // write the index if it's in range
        header = (uint8_t) index + INDEX_RANGE_START;
    }
    else {
        // write the number of bytes we will encode
        header = (uint8_t) INDEX_RANGE_END + (count - 1);
    }
    fwrite(&header, sizeof(uint8_t), 1, stream);
    if (count == 2)
    {
        uint8_t body = (uint8_t) index;
        fwrite(&body, sizeof(uint8_t), 1, stream);
    }
    else if(count == 3)
    {
        uint16_t body = (uint16_t) index;
        fwrite(&body, sizeof(uint16_t), 1, stream);
    }
    else
    {
        uint32_t body = (uint32_t) index;
        fwrite(&body, sizeof(uint32_t), 1, stream);
    }
}

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
    return 5;
}