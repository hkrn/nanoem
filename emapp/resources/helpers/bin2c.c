#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "lz4/lib/lz4hc.h"

static void
dump(const char *symbol, const unsigned char *bytes, size_t size, size_t inflated_size, int as_string, FILE *output)
{
    size_t i, new_size = 0;
    int column = 0;
    unsigned char byte;
    if (inflated_size > 0) {
        fprintf(output, "static const unsigned int %s_inflated_size = %" PRIu32 ";\n", symbol, inflated_size);
        fprintf(output, "static const unsigned int %s_deflated_size = %" PRIu32 ";\n", symbol, size);
    }
    else {
        if (as_string) {
            for (i = 0; i < size; i++) {
                byte = bytes[i];
                if (byte != '\r') {
                    new_size++;
                }
            }
            new_size += 1;
        }
        else {
            new_size = size;
        }
        fprintf(output, "static const unsigned int %s_size = %" PRIu32 ";\n", symbol, new_size);
    }
    fprintf(output, "static const unsigned char %s_data[] =\n{", symbol);
    for (i = 0; i < size; i++) {
        byte = bytes[i];
        if (as_string && byte == '\r') {
            continue;
        }
        else if ((column++ % 10) == 0) {
            fprintf(output, "\n    0x%02x, ", byte);
        }
        else {
            fprintf(output, "0x%02x, ", byte);
        }
    }
    if (as_string) {
        fprintf(output, "\n    0x0,\n");
    }
    fprintf(output, "\n};\n");
}

static int
binary_to_c(const char *filename, const char *symbol, int as_string, FILE *output)
{
    unsigned char *bytes;
    size_t size;
    int exit_code = 0;
    FILE *fp = fopen(filename, "rb");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        size = (size_t) ftell(fp);
        fseek(fp, 0, SEEK_SET);
        bytes = (unsigned char *) malloc(size);
        fread(bytes, size, 1, fp);
        fclose(fp);
        dump(symbol, bytes, size, 0, as_string, output);
        free(bytes);
        exit_code = 0;
    }
    else {
        fprintf(stderr, "%s is not found\n", filename);
    }
    return exit_code;
}

static int
binary_to_c_compressed(const char *filename, const char *symbol, FILE *output)
{
    char *input_bytes, *output_bytes;
    size_t input_size, output_size, capacity;
    int exit_code = -1;
    FILE *fp = fopen(filename, "rb");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        input_size = (size_t) ftell(fp);
        fseek(fp, 0, SEEK_SET);
        capacity = LZ4_compressBound(input_size);
        input_bytes = (char *) malloc(input_size);
        output_bytes = (char *) malloc(capacity);
        fread(input_bytes, input_size, 1, fp);
        fclose(fp);
        output_size = LZ4_compress_HC(input_bytes, output_bytes, input_size, capacity, LZ4HC_CLEVEL_MAX);
        free(input_bytes);
        dump(symbol, (unsigned char *) output_bytes, output_size, input_size, 0, output);
        free(output_bytes);
        exit_code = 0;
    }
    else {
        fprintf(stderr, "%s is not found\n", filename);
    }
    return exit_code;
}

int
main(int argc, char *argv[])
{
    static const char compressed_key[] = "--compressed",
            input_key[] = "--input",
            output_key[] = "--output",
            symbol_key[] = "--symbol",
            as_string_key[] = "--as-string";
    const char *input = 0, *symbol = 0, *value;
    FILE *output = stdout;
    int compressed = 0, as_string = 0, exit_code = -1;
    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i], *ptr = 0;
        if ((ptr = strchr(arg, '=')) != 0) {
            value = ptr + 1;
            if (strncmp(arg, compressed_key, sizeof(compressed_key) - 1) == 0) {
                compressed = strcmp(value, "true") == 0;
            }
            else if (strncmp(arg, input_key, sizeof(input_key) - 1) == 0) {
                input = value;
            }
            else if (strncmp(arg, output_key, sizeof(output_key) - 1) == 0) {
                FILE *f = fopen(value, "wb");
                if (f != NULL) {
                    output = f;
                }
            }
            else if (strncmp(arg, symbol_key, sizeof(symbol_key) - 1) == 0) {
                symbol = value;
            }
            else if (strncmp(arg, as_string_key, sizeof(as_string_key) - 1) == 0) {
                as_string = strcmp(value, "true") == 0;
            }
        }
    }
    if (compressed && input && symbol) {
        exit_code = binary_to_c_compressed(input, symbol, output);
    }
    else if (input && symbol) {
        exit_code = binary_to_c(input, symbol, as_string, output);
    }
    if (output != stdout) {
        fclose(output);
    }
    return exit_code;
}
