#include <stdio.h>
#include <stdlib.h>
#include "model.h"
#include "hashtable.h"
#include "encoding.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <model_filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    size_t longest_fragment_length = 0;
    
    HashTable *table;
    {
        FILE *file = fopen(argv[1], "r");
        if (file == NULL) {
            fprintf(stderr, "Error opening file: %s\n", argv[1]);
            exit(EXIT_FAILURE);
        }
        Model *model = model_read(file);
        fclose(file);
        {
            const int initial_table_size = 128; // this could be smarter
            table = hash_table_new(initial_table_size);
        }
        for(int i = 0; i < model->fragment_count; i++)
        {
            Fragment fragment = model->fragments[i];
            HashKey key;
            key.length = fragment.length;
            key.body = fragment.body; // this will be duplicated when added to the table
            hash_table_put(table, key, i);

            longest_fragment_length =
                longest_fragment_length > key.length
                ? longest_fragment_length
                : key.length;
        }

        // write the model hash first
        fwrite(&model->hash, sizeof(uint32_t), 1, stdout);

        model_free(model);
    }

    const size_t buffer_size = 128;
    uint8_t buffer[buffer_size];

    HashKey workingKey;
    workingKey.length = 0;
    workingKey.body = malloc(buffer_size * 2);

    HashKey workingLiteral;
    workingLiteral.length = 0;
    workingLiteral.body = malloc(buffer_size);

    size_t bytes_read;
    while((bytes_read = fread((void *)buffer, sizeof(uint8_t), buffer_size, stdin)) > 0)
    {
        memcpy(
            (void *)&workingKey.body[
                workingKey.length],
            buffer,
            bytes_read);
        workingKey.length += bytes_read;

        bool end_of_stream = bytes_read < buffer_size;

        while (workingKey.length > 0 && ((workingKey.length > longest_fragment_length) || end_of_stream))
        {
            size_t crop_length = 1;
            bool crop_to_literal = true;

            size_t max_possible_eval =
                longest_fragment_length > workingKey.length
                ? workingKey.length
                : longest_fragment_length;

            HashKey tentativeKey;
            tentativeKey.body = workingKey.body;

            for(
                tentativeKey.length = max_possible_eval;
                tentativeKey.length > 0;
                tentativeKey.length--)
            {
                const bool found_in_table = hash_table_contains(table, tentativeKey);
                if(found_in_table)
                {
                    // flush the working literal
                    if (workingLiteral.length > 0)
                    {
                        uint8_t literal_header = (uint8_t) workingLiteral.length;
                        fwrite(&literal_header, sizeof(uint8_t), 1, stdout);
                        fwrite(workingLiteral.body, sizeof(uint8_t), workingLiteral.length, stdout);
                        workingLiteral.length = 0;
                    }

                    const index_t index = hash_table_get(table, tentativeKey);
                    encoding_write(index, stdout);

                    crop_length = tentativeKey.length;
                    crop_to_literal = false;
                    break;
                }
            }

            if (crop_to_literal) {
                memcpy(
                    (void *)&workingLiteral.body[
                        workingLiteral.length],
                    workingKey.body,
                    crop_length);
                workingLiteral.length += crop_length;
            }

            memcpy(
                workingKey.body,
                (void *)&workingKey.body[
                    crop_length],
                workingKey.length - crop_length);

            workingKey.length -= crop_length;

            if (workingLiteral.length >= 127)
            {
                uint8_t literal_header = (uint8_t) workingLiteral.length;
                fwrite(&literal_header, sizeof(uint8_t), 1, stdout);
                fwrite(workingLiteral.body, sizeof(uint8_t), workingLiteral.length, stdout);
                workingLiteral.length = 0;
            }
        }
    }

    if(workingLiteral.length > 0) {
        uint8_t literal_header = (uint8_t) workingLiteral.length;
        fwrite(&literal_header, sizeof(uint8_t), 1, stdout);
        fwrite(workingLiteral.body, sizeof(uint8_t), workingLiteral.length, stdout);
    }

    return EXIT_SUCCESS;
}