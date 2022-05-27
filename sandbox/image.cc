#include "emapp/Allocator.h"
#include "emapp/Error.h"
#include "emapp/FileUtils.h"
#include "emapp/ImageLoader.h"
#include "emapp/URI.h"
#include "emapp/private/CommonInclude.h"

#include "bx/commandline.h"

using namespace nanoem;

int
main(int argc, char *argv[])
{
    Allocator::initialize();
    const bx::CommandLine command(argc, argv);
    Error error;
    FileReaderScope scope(nullptr);
    if (scope.open(URI::createFromFilePath(command.findOption("path", "")), error)) {
        IFileReader *reader = scope.reader();
        if (image::APNG *apng = ImageLoader::decodeAPNG(reader, error)) {
            nanoem_delete(apng);
            return 0;
        }
        reader->seek(0, IFileReader::kSeekTypeBegin, error);
        if (image::DDS *dds = ImageLoader::decodeDDS(reader, error)) {
            nanoem_delete(dds);
            return 0;
        }
        reader->seek(0, IFileReader::kSeekTypeBegin, error);
        ByteArray bytes;
        bytes.resize(reader->size());
        FileUtils::read(reader, bytes, error);
        sg_image_desc desc;
        nanoem_u8_t *ptr;
        if (ImageLoader::decodeImageWithSTB(bytes.data(), bytes.size(), "", desc, &ptr, error)) {
            ImageLoader::releaseDecodedImageWithSTB(&ptr);
        }
        else if (image::PFM *pfm = ImageLoader::decodePFM(bytes, error)) {
            nanoem_delete(pfm);
        }
    }
    Allocator::destroy();
    return 0;
}
