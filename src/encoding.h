#include <stdint.h>
#include <stdio.h>

#define MAX_INDEX 0xffffffff
typedef uint32_t index_t;
size_t encoding_get_byte_count(index_t index);
void encoding_write(index_t index, FILE *restrict stream);
index_t encoding_read(FILE *restrict stream);
