#include <stdio.h>
#include <stdlib.h>
#include "model.h"
#include "encoding.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <model_filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    Model *model;

    {
        FILE *file = fopen(argv[1], "r");
        if (file == NULL) {
            fprintf(stderr, "Error opening file: %s\n", argv[1]);
            exit(EXIT_FAILURE);
        }
        model = model_read(file);
        fclose(file);
    }

    uint32_t saved_model_hash;
    fread((void *) &saved_model_hash, sizeof(uint32_t), 1, stdin);

    if (saved_model_hash != model->hash) {
        fprintf(stderr, "This file does not match this model. input: %u\n; model: %u", saved_model_hash, model->hash);
        exit(EXIT_FAILURE);
    }

    uint8_t token;
    uint8_t *literal_buffer = (uint8_t *) malloc(128);

    size_t bytes_read;
    while((bytes_read = fread((void *) &token, sizeof(uint8_t), 1, stdin)) > 0)
    {
        if(token < 128) {
            size_t derp = fread(literal_buffer, sizeof(uint8_t), token, stdin);
            fwrite(literal_buffer, sizeof(uint8_t), derp, stdout);
        }
        else if (token < 252) {
            index_t index = (index_t) token - 128;
            Fragment *f = &model->fragments[index];
            fwrite(f->body, sizeof(uint8_t), f->length, stdout);
        }
        else {
            uint8_t byteCount = token - 252 + 1;
            index_t index;
            if (byteCount == 1)
            {
                uint8_t derp;
                fread((void *) &derp, sizeof(uint8_t), 1, stdin);
                index = (index_t) derp;
            }
            else if (byteCount == 2)
            {
                uint16_t derp;
                fread((void *) &derp, sizeof(uint16_t), 1, stdin);
                index = (index_t) derp;
            }
            else if (byteCount == 4)
            {
                uint32_t derp;
                fread((void *) &derp, sizeof(uint32_t), 1, stdin);
                index = (index_t) derp;
            }
            else
            {
                fprintf(stderr, "Three-byte sequences not supported!\n");
                exit(EXIT_FAILURE);
            }
            Fragment *f = &model->fragments[index];
            fwrite(f->body, sizeof(uint8_t), f->length, stdout);
        }
    }

    model_free(model);

    return EXIT_SUCCESS;
}
