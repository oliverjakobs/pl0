#include "common.h"

#include "lexer.h"

void error(const char* fmt, ...) {
    fprintf(stderr, "pl0c: error: ");

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fputc('\n', stderr);

    exit(1);
}

void errorAt(int line, const char* fmt, ...) {
    fprintf(stderr, "pl0c: error: line %lu: ", line);

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fputc('\n', stderr);

    exit(1);
}

char* readFile(const char* path) {
    char* ext = strrchr(path, '.');
    if (!ext || strncmp(ext, ".pl0", 4) != 0) error(0, "file must end in '.pl0'");

    FILE* file = fopen(path, "rb");
    if (!file) error(0, "couldn't open %s", path);

    /* find file size */
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    char* buffer = malloc(size + 1);
    if (!buffer) error(0, "malloc failed");

    if (fread(buffer, size, 1, file) != 1)
        error(0, "couldn't read %s", path);

    /* zero terminate buffer */
    buffer[size] = '\0';

    fclose(file);
    return buffer;
}