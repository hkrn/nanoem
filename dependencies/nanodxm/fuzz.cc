/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is licensed under MIT license. for more details, see LICENSE.txt.
 */

#include "nanodxm.h"

#include <stdint.h>

extern "C" int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    nanodxm_document_t *document = nanodxmDocumentCreate();
    nanodxm_buffer_t *buffer = nanodxmBufferCreate(data, size);
    nanodxmDocumentParse(document, buffer);
    nanodxmBufferDestroy(buffer);
    nanodxmDocumentDestroy(document);
    return 0;
}
