#include "./common.h"

static void
save_and_load(nanoem_unicode_string_factory_t *factory, nanoem_document_t *input)
{
#if 0
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_document_t *mutable_document = nanoemMutableDocumentCreateAsReference(input);
    nanoem_mutable_buffer_t *mutable_buffer = nanoemMutableBufferCreate();
    nanoemMutableDocumentSaveToBuffer(mutable_document, mutable_buffer, &status);
    nanoem_document_t *document = nanoemDocumentCreate(factory);
    nanoem_buffer_t *buffer = nanoemMutableBufferCreateBufferObject(mutable_buffer);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemDocumentLoadFromBuffer(document, buffer, 0, &status);
    nanoemBufferDestroy(buffer);
    nanoemDocumentDestroy(document);
    nanoemMutableBufferDestroy(mutable_buffer);
    nanoemMutableDocumentDestroy(mutable_document);
#endif
}

extern "C" int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_unicode_string_factory_t *factory = nanoemUnicodeStringFactoryCreateEXT(&status);
    nanoem_document_t *document = nanoemDocumentCreate(factory, &status);
    nanoem_buffer_t *buffer = nanoemBufferCreate(data, size, &status);
    nanoemDocumentLoadFromBuffer(document, buffer, &status);
    // save_and_load(factory, document);
    nanoemBufferDestroy(buffer);
    nanoemDocumentDestroy(document);
    nanoemUnicodeStringFactoryDestroyEXT(factory);
    return 0;
}
