/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is licensed under MIT license. for more details, see LICENSE.txt.
 */

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#endif /* _WIN32 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nanodxm.h"

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

#define assert_equal_float(a, b) \
    for (;;) {                   \
        union __float2int_t {    \
            int i;               \
            float f;             \
        } v;                     \
        v.f = (b);               \
        assert_equal((a), v.i);  \
        break;                   \
    }

static void
nanodxmTestNull(void)
{
}

static void
nanodxmLoadFile(const char *path)
{
    FILE *fp;
    long size;
    nanodxm_uint8_t *bytes;
    nanodxm_buffer_t *buffer;
    nanodxm_document_t *document;
    nanodxm_rsize_t read_size, nvertices, nmaterials, nfaces;
    nanodxm_status_t rc;
    fp = fopen(path, "rb");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        bytes = (nanodxm_uint8_t *) malloc(size);
        read_size = fread(bytes, 1, size, fp);
        fclose(fp);
        buffer = nanodxmBufferCreate(bytes, size);
        document = nanodxmDocumentCreate();
        rc = nanodxmDocumentParse(document, buffer);
        if (rc == NANODXM_STATUS_SUCCESS) {
            nanodxmDocumentGetVertices(document, &nvertices);
            nanodxmDocumentGetFaceMaterialIndices(document, &nmaterials);
            nanodxmDocumentGetVertexFaces(document, &nfaces);
            fprintf(stderr, "test: %s\nsize = %ld nvertices = %ld, nmaterials = %ld, nfaces = %ld\n",
                    path, read_size, nvertices, nmaterials, nfaces);
        }
        else {
            fprintf(stderr, "test: %s\n", path);
            switch (rc) {
            case NANODXM_STATUS_SUCCESS:
                fprintf(stderr, "status = NANODXM_STATUS_SUCCESS\n");
                break;
            case NANODXM_STATUS_ERROR_NULL_BUFFER:
                fprintf(stderr, "status = NANODXM_STATUS_ERROR_NULL_BUFFER\n");
                break;
            case NANODXM_STATUS_ERROR_NULL_DOCUMENT:
                fprintf(stderr, "status = NANODXM_STATUS_ERROR_NULL_DOCUMENT\n");
                break;
            case NANODXM_STATUS_ERROR_NULL_SCENE:
                fprintf(stderr, "status = NANODXM_STATUS_ERROR_NULL_SCENE\n");
                break;
            case NANODXM_STATUS_ERROR_NULL_MATERIAL:
                fprintf(stderr, "status = NANODXM_STATUS_ERROR_NULL_MATERIAL\n");
                break;
            case NANODXM_STATUS_ERROR_NULL_OBJECT:
                fprintf(stderr, "status = NANODXM_STATUS_ERROR_NULL_OBJECT\n");
                break;
            case NANODXM_STATUS_ERROR_NULL_VERTEX:
                fprintf(stderr, "status = NANODXM_STATUS_ERROR_NULL_VERTEX\n");
                break;
            case NANODXM_STATUS_ERROR_NULL_FACE:
                fprintf(stderr, "status = NANODXM_STATUS_ERROR_NULL_FACE\n");
                break;
            case NANODXM_STATUS_ERROR_INVALID_SIGNATURE:
                fprintf(stderr, "status = NANODXM_STATUS_ERROR_INVALID_SIGNATURE\n");
                break;
            case NANODXM_STATUS_ERROR_INVALID_VERSION:
                fprintf(stderr, "status = NANODXM_STATUS_ERROR_INVALID_VERSION\n");
                break;
            case NANODXM_STATUS_ERROR_INVALID_DATA_TYPE:
                fprintf(stderr, "status = NANODXM_STATUS_ERROR_INVALID_DATA_TYPE\n");
                break;
            case NANODXM_STATUS_ERROR_INVALID_FLOAT_SIZE:
                fprintf(stderr, "status = NANODXM_STATUS_ERROR_INVALID_FLOAT_SIZE\n");
                break;
            case NANODXM_STATUS_ERROR_INVALID_TOKEN:
                fprintf(stderr, "status = NANODXM_STATUS_ERROR_INVALID_TOKEN\n");
                break;
            case NANODXM_STATUS_ERROR_INVALID_EOF:
                fprintf(stderr, "status = NANODXM_STATUS_ERROR_INVALID_EOF\n");
                break;
            case NANODXM_STATUS_ERROR_NOT_SUPPORTED_DATA_TYPE:
                fprintf(stderr, "status = NANODXM_STATUS_ERROR_NOT_SUPPORTED_DATA_TYPE\n");
                break;
            case NANODXM_STATUS_ERROR_NOT_SUPPORTED_TOKEN:
                fprintf(stderr, "status = NANODXM_STATUS_ERROR_NOT_SUPPORTED_TOKEN\n");
                break;
            case NANODXM_STATUS_ERROR_UNKNOWN_CHUNK_TYPE:
                fprintf(stderr, "status = NANODXM_STATUS_ERROR_UNKNOWN_CHUNK_TYPE\n");
                break;
            default:
                break;
            }
        }
        nanodxmDocumentDestroy(document);
        nanodxmBufferDestroy(buffer);
        free(bytes);
    }
}

static void
nanodxmTestDynamicModelFixtures(FILE *fp)
{
    char path[1024], *p;
    while (fgets(path, sizeof(path), fp) != NULL) {
        p = memchr(path, '\n', sizeof(path));
        if (p) {
            *p = '\0';
        }
        nanodxmLoadFile(path);
    }
    fclose(fp);
}

static void *
nanodxmTestMalloc(void *opaque, size_t size, const char *file, int line)
{
    nanodxm_mark_unused(opaque);
    nanodxm_mark_unused(file);
    nanodxm_mark_unused(line);
    return malloc(size);
}

static void *
nanodxmTestCalloc(void *opaque, size_t length, size_t size, const char *file, int line)
{
    nanodxm_mark_unused(opaque);
    nanodxm_mark_unused(file);
    nanodxm_mark_unused(line);
    return calloc(length, size);
}

static void *
nanodxmTestRealloc(void *opaque, void *ptr, size_t size, const char *file, int line)
{
    nanodxm_mark_unused(opaque);
    nanodxm_mark_unused(file);
    nanodxm_mark_unused(line);
    return realloc(ptr, size);
}

static void
nanodxmTestFree(void *opaque, void *ptr, const char *file, int line)
{
    nanodxm_mark_unused(opaque);
    nanodxm_mark_unused(file);
    nanodxm_mark_unused(line);
    free(ptr);
}

int
main(int argc, char *argv[])
{
    nanodxm_global_allocator_t allocator = {
        NULL,
        nanodxmTestMalloc,
        nanodxmTestCalloc,
        nanodxmTestRealloc,
        nanodxmTestFree
    };
    nanodxmGlobalSetCustomAllocator(&allocator);
    char *value = getenv("NANODXM_FIXTURES_PATH");
    if (value) {
        FILE *model_fp = fopen(value, "r");
        if (model_fp) {
            nanodxmTestDynamicModelFixtures(model_fp);
            fclose(model_fp);
        }
    }
    else if (argc > 1) {
        nanodxmTestNull();
        for (int i = 1; i < argc; i++) {
            const char *filename = argv[i];
            fprintf(stderr, "test: %s\n", filename);
            nanodxmLoadFile(filename);
        }
    }
    return 0;
}
