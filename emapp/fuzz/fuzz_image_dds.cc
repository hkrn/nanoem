#include "emapp/Allocator.h"
#include "emapp/Error.h"
#include "emapp/FileUtils.h"
#include "emapp/ImageLoader.h"
#include "emapp/private/CommonInclude.h"

using namespace nanoem;

extern "C" int
LLVMFuzzerInitialize(int * /* argc */, char *** /* argv */)
{
    Allocator::initialize();
    return 0;
}

extern "C" int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    ByteArray bytes(data, data + size);
    MemoryReader reader(&bytes);
    Error error;
    image::DDS *dds = ImageLoader::decodeDDS(&reader, error);
    nanoem_delete(dds);
    return 0;
}
