/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is licensed under MIT license. for more details, see LICENSE.txt.
 */

#include "nanomqo.h"

#include <stdint.h>
#include <stdio.h>

extern "C" int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    nanomqo_document_t *document = nanomqoDocumentCreate();
    nanomqo_buffer_t *buffer = nanomqoBufferCreate(data, size);
    nanomqoDocumentParse(document, buffer);
    nanomqoBufferDestroy(buffer);
    nanomqoDocumentDestroy(document);
    return 0;
}
