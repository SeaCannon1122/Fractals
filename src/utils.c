#include "headers.h"

int string_length(char* str) {
    int len = 0;
    for (; str[len] != '\0'; len++);
    return (len + 1);
}

char* parse_file(const char* filename) {

    FILE* file = fopen(filename, "rb");
    if (file == NULL) { 
        printf("couldnt fint file %s\n", filename);
        return NULL; 
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    char* buffer = (char*) malloc((fileSize + 1) * sizeof(char));
    if (buffer == NULL) {
        fclose(file);
        return NULL;
    }

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead != fileSize) {
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[fileSize] = '\0';

    fclose(file);

    return buffer;
}
