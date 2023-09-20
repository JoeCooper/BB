#include <stdint.h>
#include <stdio.h>

typedef struct fragment_t {
    uint8_t length;
    uint8_t *body;
} Fragment;

typedef struct model_t {
    uint32_t hash;
    uint32_t fragment_count;
    Fragment *fragments;
} Model;

Model * model_read(FILE *restrict stream);
size_t model_write(Model *model, FILE *restrict stream);
void model_free(Model *model);
