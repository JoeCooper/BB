#include "model.h"
#include <stdlib.h>

Model * model_read(FILE *restrict stream)
{
    Model *model = (Model *)malloc(sizeof(Model));
    fread(&model->hash, sizeof(uint32_t), 1, stream);
    fread(&model->fragment_count, sizeof(uint32_t), 1, stream);
    model->fragments = (Fragment *) malloc(model->fragment_count * sizeof(Fragment));
    for(int i = 0; i < model->fragment_count; i++)
    {
        Fragment *f = &model->fragments[i];
        fread(&f->length, sizeof(uint8_t), 1, stream);
        f->body = (uint8_t *) malloc(f->length);
        fread(f->body, sizeof(uint8_t), f->length, stream);
    }
    return model;
}

size_t model_write(Model *model, FILE *restrict stream)
{
    size_t written = 0;
    written += fwrite((void *)&model->hash, sizeof(uint32_t), 1, stream);
    written += fwrite((void *)&model->fragment_count, sizeof(uint32_t), 1, stream);
    for(size_t i = 0; i < model->fragment_count; i++) {
        Fragment *f = &model->fragments[i];
        written += fwrite(&f->length, sizeof(uint8_t), 1, stream);
        written += fwrite(f->body, sizeof(uint8_t), f->length, stream);
    }
    return written;
}

void model_free(Model *model)
{
    for(int i = 0; i < model->fragment_count; i++)
    {
        free(
            model->fragments[i].body);
    }
    free(model->fragments);
    free(model);
}