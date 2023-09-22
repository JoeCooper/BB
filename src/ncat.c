#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_BUFFER 1024

void print_file(const char* filename, int times) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "ncat: %s: No such file or directory\n", filename);
        return;
    }

    char buffer[MAX_BUFFER];
    for (int i = 0; i < times; i++) {
        fseek(file, 0, SEEK_SET);
        while (fgets(buffer, MAX_BUFFER, file) != NULL) {
            printf("%s", buffer);
        }
    }

    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "ncat: missing file operand\n");
        return 1;
    }

    int times = 1;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0) {
            if (i + 1 < argc) {
                times = atoi(argv[++i]);
                if (times <= 0) {
                    fprintf(stderr, "ncat: invalid repeat count '%s'\n", argv[i]);
                    return 1;
                }
            } else {
                fprintf(stderr, "ncat: option requires an argument -- 'n'\n");
                return 1;
            }
        } else {
            print_file(argv[i], times);
        }
    }

    return 0;
}
