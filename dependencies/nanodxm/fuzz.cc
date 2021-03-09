/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is licensed under MIT license. for more details, see LICENSE.txt.
 */

#include <stdint.h>
#include "nanodxm.h"

extern "C" int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    nanodxm_status_t status = NANODXM_STATUS_SUCCESS;
    nanodxm_document_t *document = nanodxmDocumentCreate();
    nanodxm_buffer_t *buffer = nanodxmBufferCreate(data, size);
    status = nanodxmDocumentParse(document, buffer);
    nanodxmBufferDestroy(buffer);
    nanodxmDocumentDestroy(document);
    return 0;
}

