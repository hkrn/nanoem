/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/ImageLoader.h"

#include "emapp/EnumUtils.h"
#include "emapp/Error.h"
#include "emapp/FileUtils.h"
#include "emapp/IDrawable.h"
#include "emapp/Project.h"
#include "emapp/StringUtils.h"
#include "emapp/URI.h"
#include "emapp/private/CommonInclude.h"

#include "bimg/decode.h"
#include "bx/file.h"

extern "C" {
void *__stb_malloc(size_t size, const char *file, int line);
void *__stb_realloc(void *ptr, size_t size, const char *file, int line);
void __stb_free(void *ptr, const char *file, int line);
}

#define STBI_NO_STDIO
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#ifndef NDEBUG
#define STBI_MALLOC(sz) __stb_malloc((sz), __FILE__, __LINE__)
#define STBI_REALLOC(p, newsz) __stb_realloc((p), (newsz), __FILE__, __LINE__)
#define STBI_FREE(p) __stb_free((p), __FILE__, __LINE__)
#else /* NDEBUG */
#define STBI_MALLOC(sz) __stb_malloc((sz), nullptr, 0)
#define STBI_REALLOC(p, newsz) __stb_realloc((p), (newsz), nullptr, 0)
#define STBI_FREE(p) __stb_free((p), nullptr, 0)
#endif /* NDEBUG */
#include "stb/stb_image.h"

namespace nanoem {
namespace {

static const nanoem_u64_t kPNGSignature = 0x0a1a0a0d474e5089;
static const nanoem_u32_t kPNGChunkTypeImageData = nanoem_fourcc('I', 'D', 'A', 'T');
static const nanoem_u32_t kPNGChunkTypeImageHeader = nanoem_fourcc('I', 'H', 'D', 'R');
static const nanoem_u32_t kPNGChunkTypeImageEnd = nanoem_fourcc('I', 'E', 'N', 'D');
static const nanoem_u32_t kPNGChunkTypeAnimationControl = nanoem_fourcc('a', 'c', 'T', 'L');
static const nanoem_u32_t kPNGChunkTypeFrameControl = nanoem_fourcc('f', 'c', 'T', 'L');
static const nanoem_u32_t kPNGChunkTypeFrameData = nanoem_fourcc('f', 'd', 'A', 'T');

struct CRC {
    CRC()
    {
        for (nanoem_u32_t i = 0; i < 256; i++) {
            nanoem_u32_t c = i;
            for (int j = 0; j < 8; j++) {
                c = (c & 1) ? (0xEDB88320 ^ (c >> 1)) : (c >> 1);
            }
            m_table[i] = c;
        }
    }
    nanoem_u32_t
    checksum(nanoem_u32_t chunkType, const void *data, nanoem_rsize_t size) const NANOEM_DECL_NOEXCEPT
    {
        nanoem_u32_t c = 0xFFFFFFFF;
        const nanoem_u8_t *chunkTypePtr = reinterpret_cast<const nanoem_u8_t *>(&chunkType);
        for (nanoem_rsize_t i = 0; i < 4; i++) {
            c = m_table[(c ^ chunkTypePtr[i]) & 0xFF] ^ (c >> 8);
        }
        const nanoem_u8_t *dataPtr = reinterpret_cast<const nanoem_u8_t *>(data);
        for (nanoem_rsize_t i = 0; i < size; i++) {
            c = m_table[(c ^ dataPtr[i]) & 0xFF] ^ (c >> 8);
        }
        return bx::toBigEndian(c ^ 0xFFFFFFFF);
    }
    template <typename T>
    inline nanoem_u32_t
    checksumTyped(nanoem_u32_t chunkType, const T &data) const NANOEM_DECL_NOEXCEPT
    {
        return checksum(chunkType, &data, sizeof(data));
    }
    nanoem_u32_t m_table[256];
};

} /* namespace anonymous */

Image::Image()
    : m_fileExist(false)
{
    Inline::clearZeroMemory(m_description);
    m_handle = { SG_INVALID_ID };
}

Image::~Image()
{
}

void
Image::create()
{
    m_handle = sg::make_image(&m_description);
    if (!m_label.empty()) {
        SG_LABEL_IMAGE(m_handle, m_label.c_str());
    }
}

void
Image::destroy()
{
    sg::destroy_image(m_handle);
    m_handle = { SG_INVALID_ID };
}

void
Image::setOriginData(const nanoem_u8_t *data, nanoem_rsize_t size)
{
    m_originData.assign(data, data + size);
    sg_range &dst = m_description.data.subimage[0][0];
    dst.ptr = m_originData.data();
    dst.size = size;
}

void
Image::setMipmapData(nanoem_rsize_t index, const nanoem_u8_t *data, nanoem_rsize_t size)
{
    m_mipmapData[index].assign(data, data + size);
    sg_range &innerDst = m_description.data.subimage[0][index + 1];
    innerDst.ptr = m_mipmapData[index].data();
    innerDst.size = size;
}

void
Image::resizeMipmapData(nanoem_rsize_t value)
{
    m_mipmapData.resize(value);
}

void
Image::setLabel(const String &value)
{
    m_label = value;
    m_description.label = m_label.c_str();
}

sg_image
Image::handle() const NANOEM_DECL_NOEXCEPT
{
    return m_handle;
}

void
Image::setHandle(sg_image value)
{
    m_handle = value;
}

sg_image_desc
Image::description() const NANOEM_DECL_NOEXCEPT
{
    return m_description;
}

void
Image::setDescription(const sg_image_desc &value)
{
    m_description = value;
}

const ByteArray *
Image::originData() const NANOEM_DECL_NOEXCEPT
{
    return &m_originData;
}

const ByteArray *
Image::mipmapData(nanoem_rsize_t index) const NANOEM_DECL_NOEXCEPT
{
    return index < m_mipmapData.size() ? &m_mipmapData[index] : nullptr;
}

const char *
Image::filenameConstString() const NANOEM_DECL_NOEXCEPT
{
    return m_filename.c_str();
}

String
Image::filename() const
{
    return m_filename;
}

void
Image::setFilename(const String &value)
{
    m_filename = value;
}

bool
Image::isFileExist() const NANOEM_DECL_NOEXCEPT
{
    return m_fileExist;
}

void
Image::setFileExist(bool value)
{
    m_fileExist = value;
}

sg_pixel_format
ImageLoader::resolvePixelFormat(const bimg::ImageContainer *container, nanoem_u32_t &bytesPerPixel) NANOEM_DECL_NOEXCEPT
{
    sg_pixel_format value;
    switch (container->m_format) {
    case bimg::TextureFormat::BC1: {
        value = SG_PIXELFORMAT_BC1_RGBA;
        bytesPerPixel = 0;
        break;
    }
    case bimg::TextureFormat::BC2: {
        value = SG_PIXELFORMAT_BC2_RGBA;
        bytesPerPixel = 0;
        break;
    }
    case bimg::TextureFormat::BC3: {
        value = SG_PIXELFORMAT_BC3_RGBA;
        bytesPerPixel = 0;
        break;
    }
    case bimg::TextureFormat::ETC2: {
        value = SG_PIXELFORMAT_ETC2_RGB8;
        bytesPerPixel = 0;
        break;
    }
    case bimg::TextureFormat::A8:
    case bimg::TextureFormat::R8: {
        value = SG_PIXELFORMAT_R8;
        bytesPerPixel = 1;
        break;
    }
    case bimg::TextureFormat::RG8: {
        value = SG_PIXELFORMAT_RG8;
        bytesPerPixel = 2;
        break;
    }
    case bimg::TextureFormat::RGB10A2: {
        value = SG_PIXELFORMAT_RGB10A2;
        bytesPerPixel = 4;
        break;
    }
    case bimg::TextureFormat::R16: {
        value = SG_PIXELFORMAT_R16;
        bytesPerPixel = 2;
        break;
    }
    case bimg::TextureFormat::R16S: {
        value = SG_PIXELFORMAT_R16SI;
        bytesPerPixel = 2;
        break;
    }
    case bimg::TextureFormat::R16U: {
        value = SG_PIXELFORMAT_R16UI;
        bytesPerPixel = 2;
        break;
    }
    case bimg::TextureFormat::R16F: {
        value = SG_PIXELFORMAT_R16F;
        bytesPerPixel = 2;
        break;
    }
    case bimg::TextureFormat::RG16S: {
        value = SG_PIXELFORMAT_RG16SI;
        bytesPerPixel = 4;
        break;
    }
    case bimg::TextureFormat::RG16U: {
        value = SG_PIXELFORMAT_RG16UI;
        bytesPerPixel = 4;
        break;
    }
    case bimg::TextureFormat::RG16F: {
        value = SG_PIXELFORMAT_RG16F;
        bytesPerPixel = 4;
        break;
    }
    case bimg::TextureFormat::R32I: {
        value = SG_PIXELFORMAT_R32SI;
        bytesPerPixel = 4;
        break;
    }
    case bimg::TextureFormat::R32U: {
        value = SG_PIXELFORMAT_R32UI;
        bytesPerPixel = 4;
        break;
    }
    case bimg::TextureFormat::R32F: {
        value = SG_PIXELFORMAT_R32F;
        bytesPerPixel = 4;
        break;
    }
    case bimg::TextureFormat::RG32I: {
        value = SG_PIXELFORMAT_RG32SI;
        bytesPerPixel = 8;
        break;
    }
    case bimg::TextureFormat::RG32U: {
        value = SG_PIXELFORMAT_RG32UI;
        bytesPerPixel = 8;
        break;
    }
    case bimg::TextureFormat::RG32F: {
        value = SG_PIXELFORMAT_RG32F;
        bytesPerPixel = 8;
        break;
    }
    case bimg::TextureFormat::RGBA16: {
        value = SG_PIXELFORMAT_RGBA16;
        bytesPerPixel = 8;
        break;
    }
    case bimg::TextureFormat::RGBA16I: {
        value = SG_PIXELFORMAT_RGBA16SI;
        bytesPerPixel = 8;
        break;
    }
    case bimg::TextureFormat::RGBA16U: {
        value = SG_PIXELFORMAT_RGBA16UI;
        bytesPerPixel = 8;
        break;
    }
    case bimg::TextureFormat::RGBA16F: {
        value = SG_PIXELFORMAT_RGBA16F;
        bytesPerPixel = 8;
        break;
    }
    case bimg::TextureFormat::RGBA32I: {
        value = SG_PIXELFORMAT_RGBA32SI;
        bytesPerPixel = 16;
        break;
    }
    case bimg::TextureFormat::RGBA32U: {
        value = SG_PIXELFORMAT_RGBA32UI;
        bytesPerPixel = 16;
        break;
    }
    case bimg::TextureFormat::RGBA32F: {
        value = SG_PIXELFORMAT_RGBA32F;
        bytesPerPixel = 16;
        break;
    }
    case bimg::TextureFormat::RGBA8: {
        value = SG_PIXELFORMAT_RGBA8;
        bytesPerPixel = 4;
        break;
    }
    case bimg::TextureFormat::BGRA8: {
        value = SG_PIXELFORMAT_BGRA8;
        bytesPerPixel = 4;
        break;
    }
    default:
        value = SG_PIXELFORMAT_NONE;
        bytesPerPixel = 0;
        break;
    }
    return value;
}

APNGImage *
ImageLoader::decodeAnimatedPNG(IFileReader *reader, Error &error)
{
    nanoem_u64_t signature;
    FileUtils::readTyped(reader, signature, error);
    APNGImage *animation = nullptr;
    if (signature == kPNGSignature) {
        ByteArray dataChunk;
        APNGImage::Frame *frame = nullptr;
        CRC crc;
        nanoem_u32_t expectedSequenceNumber = 0;
        nanoem_f32_t seconds = 0;
        bool loop = true;
        animation = nanoem_new(APNGImage);
        while (loop && !error.hasReason()) {
            nanoem_u32_t chunkLength, chunkType, chunkCRC, checksum = 0;
            FileUtils::readTyped(reader, chunkLength, error);
            chunkLength = bx::toBigEndian(chunkLength);
            FileUtils::readTyped(reader, chunkType, error);
            switch (chunkType) {
            case kPNGChunkTypeAnimationControl: {
                APNGImage::AnimationControl &animationControl = animation->m_control;
                FileUtils::readTyped(reader, animationControl, error);
                checksum = crc.checksumTyped(chunkType, animationControl);
                animationControl.m_numFrames = bx::toBigEndian(animationControl.m_numFrames);
                animationControl.m_numPlayCount = bx::toBigEndian(animationControl.m_numPlayCount);
                break;
            }
            case kPNGChunkTypeFrameControl: {
                if (frame) {
                    if (frame->m_sequences.empty()) {
                        error = Error("Empty image sequence", nullptr, Error::kDomainTypeApplication);
                    }
                    else {
                        animation->m_frames.push_back(frame);
                    }
                }
                if (!error.hasReason()) {
                    frame = nanoem_new(APNGImage::Frame);
                    APNGImage::FrameControl &frameControl = frame->m_control;
                    FileUtils::readTyped(reader, frameControl, error);
                    checksum = crc.checksumTyped(chunkType, frameControl);
                    frameControl.m_sequenceNumber = bx::toBigEndian(frameControl.m_sequenceNumber);
                    frameControl.m_width = bx::toBigEndian(frameControl.m_width);
                    frameControl.m_height = bx::toBigEndian(frameControl.m_height);
                    frameControl.m_xoffset = bx::toBigEndian(frameControl.m_xoffset);
                    frameControl.m_yoffset = bx::toBigEndian(frameControl.m_yoffset);
                    frameControl.m_delayDen = bx::toBigEndian(frameControl.m_delayDen);
                    frameControl.m_delayNum = bx::toBigEndian(frameControl.m_delayNum);
                    if (frameControl.m_delayDen == 0) {
                        frameControl.m_delayDen = 100;
                    }
                    if (expectedSequenceNumber != frameControl.m_sequenceNumber) {
                        nanoem_delete_safe(frame);
                        error = Error("Invalid sequence number", nullptr, Error::kDomainTypeApplication);
                    }
                    else if (frameControl.m_width == 0 || frameControl.m_height == 0 ||
                        !(frameControl.m_xoffset + frameControl.m_width <= animation->m_header.m_width) ||
                        !(frameControl.m_yoffset + frameControl.m_height <= animation->m_header.m_height)) {
                        nanoem_delete_safe(frame);
                        error = Error("Invalid frame rectangle", nullptr, Error::kDomainTypeApplication);
                    }
                    frame->m_seconds = seconds;
                    seconds += frameControl.m_delayNum / nanoem_f32_t(frameControl.m_delayDen);
                    expectedSequenceNumber++;
                }
                break;
            }
            case kPNGChunkTypeFrameData: {
                dataChunk.clear();
                nanoem_u32_t sequenceNumber;
                if (chunkLength >= sizeof(sequenceNumber)) {
                    dataChunk.resize(chunkLength);
                    nanoem_u8_t *dataChunkPtr = dataChunk.data();
                    FileUtils::read(reader, dataChunkPtr, chunkLength, error);
                    checksum = crc.checksum(chunkType, dataChunkPtr, chunkLength);
                    sequenceNumber = bx::toBigEndian(*reinterpret_cast<const nanoem_u32_t *>(dataChunkPtr));
                    if (expectedSequenceNumber != sequenceNumber) {
                        error = Error("incorrect sequence number", nullptr, Error::kDomainTypeApplication);
                    }
                    else if (frame) {
                        dataChunk.erase(dataChunk.begin(), dataChunk.begin() + sizeof(sequenceNumber));
                        frame->m_sequences.push_back(dataChunk);
                    }
                }
                expectedSequenceNumber++;
                break;
            }
            case kPNGChunkTypeImageData: {
                dataChunk.resize(chunkLength);
                FileUtils::read(reader, dataChunk.data(), chunkLength, error);
                checksum = crc.checksum(chunkType, dataChunk.data(), chunkLength);
                if (frame) {
                    frame->m_sequences.push_back(dataChunk);
                }
                break;
            }
            case kPNGChunkTypeImageEnd: {
                if (frame) {
                    if (frame->m_sequences.empty()) {
                        error = Error("Empty image sequence", nullptr, Error::kDomainTypeApplication);
                    }
                    else {
                        animation->m_frames.push_back(frame);
                    }
                }
                checksum = crc.checksum(chunkType, nullptr, 0);
                loop = false;
                break;
            }
            case kPNGChunkTypeImageHeader: {
                APNGImage::Header &header = animation->m_header;
                FileUtils::readTyped(reader, header, error);
                checksum = crc.checksumTyped(chunkType, header);
                header.m_width = bx::toBigEndian(header.m_width);
                header.m_height = bx::toBigEndian(header.m_height);
                break;
            }
            default:
                reader->seek(chunkLength, IFileReader::kSeekTypeCurrent, error);
                break;
            }
            FileUtils::readTyped(reader, chunkCRC, error);
            if (chunkCRC != checksum) {
                error = Error("CRC checksum not matched", nullptr, Error::kDomainTypeApplication);
            }
        }
        if (!error.hasReason() && animation->m_control.m_numFrames != animation->m_frames.size()) {
            error = Error("frame count doesn't equal", nullptr, Error::kDomainTypeApplication);
        }
        if (!error.hasReason()) {
            int numFrames = 0;
            ByteArray compositionImageData, buffer;
            MemoryWriter writer(&buffer);
            const nanoem_u32_t width = animation->m_header.m_width, height = animation->m_header.m_height;
            compositionImageData.resize(nanoem_rsize_t(4) * width * height);
            for (APNGImage::FrameSequenceList::const_iterator it = animation->m_frames.begin(),
                                                              end = animation->m_frames.end();
                 it != end; ++it) {
                APNGImage::Frame *frame = *it;
                const APNGImage::FrameControl &frameControl = frame->m_control;
                switch (frameControl.m_disposeOp) {
                case APNG_DISPOSE_OP_PREVIOUS: {
                    if (it > animation->m_frames.begin()) {
                        const APNGImage::Frame *previousFrame = *(it - 1);
                        compositionImageData = previousFrame->m_composition;
                    }
                    break;
                }
                case APNG_DISPOSE_OP_BACKGROUND: {
                    memset(compositionImageData.data(), 0, compositionImageData.size());
                    break;
                }
                case APNG_DISPOSE_OP_NONE:
                default:
                    break;
                }
                for (ByteArrayList::const_iterator it2 = frame->m_sequences.begin(), end2 = frame->m_sequences.end();
                     it2 != end2; ++it2) {
                    const ByteArray &bytes = *it2;
                    APNGImage::Header header(animation->m_header);
                    header.m_width = bx::toBigEndian(frameControl.m_width);
                    header.m_height = bx::toBigEndian(frameControl.m_height);
                    writer.clear();
                    FileUtils::writeTyped(&writer, kPNGSignature, error);
                    FileUtils::writeTyped(&writer, bx::toBigEndian(Inline::saturateInt32U(sizeof(header))), error);
                    FileUtils::writeTyped(&writer, kPNGChunkTypeImageHeader, error);
                    FileUtils::writeTyped(&writer, header, error);
                    FileUtils::writeTyped(&writer, crc.checksumTyped(kPNGChunkTypeImageHeader, header), error);
                    FileUtils::writeTyped(&writer, bx::toBigEndian(Inline::saturateInt32U(bytes.size())), error);
                    FileUtils::writeTyped(&writer, kPNGChunkTypeImageData, error);
                    FileUtils::write(&writer, bytes, error);
                    FileUtils::writeTyped(
                        &writer, crc.checksum(kPNGChunkTypeImageData, bytes.data(), bytes.size()), error);
                    FileUtils::writeTyped(&writer, 0, error);
                    FileUtils::writeTyped(&writer, kPNGChunkTypeImageEnd, error);
                    FileUtils::writeTyped(&writer, crc.checksum(kPNGChunkTypeImageEnd, nullptr, 0), error);
                    bx::Error err;
                    if (bimg::ImageContainer *container = bimg::imageParse(g_bimg_allocator, buffer.data(),
                            Inline::saturateInt32U(buffer.size()), bimg::TextureFormat::RGBA8, &err)) {
                        if (frameControl.m_blendOp == APNG_BLEND_OP_SOURCE) {
                            const nanoem_u32_t *imageDataPtr = static_cast<const nanoem_u32_t *>(container->m_data);
                            nanoem_u32_t *compositionImageDataPtr =
                                             reinterpret_cast<nanoem_u32_t *>(compositionImageData.data()),
                                         xoffset = frameControl.m_xoffset, yoffset = frameControl.m_yoffset;
                            for (nanoem_u32_t y = 0, h = frameControl.m_height; y < h; y++) {
                                for (nanoem_u32_t x = 0, w = frameControl.m_width; x < w; x++) {
                                    const nanoem_rsize_t srcOffset = y * frameControl.m_width + x,
                                                         dstOffset = ((y + yoffset) * width) + (x + xoffset);
                                    compositionImageDataPtr[dstOffset] = imageDataPtr[srcOffset];
                                }
                            }
                        }
                        else if (frameControl.m_blendOp == APNG_BLEND_OP_OVER) {
                            const nanoem_u32_t xoffset = frameControl.m_xoffset, yoffset = frameControl.m_yoffset;
                            const nanoem_u8_t *imageDataPtr = static_cast<const nanoem_u8_t *>(container->m_data);
                            nanoem_u8_t *compositionImageDataPtr = compositionImageData.data();
                            nanoem_u32_t i0, i1;
                            for (nanoem_u32_t y = 0, h = frameControl.m_height; y < h; y++) {
                                for (nanoem_u32_t x = 0, w = frameControl.m_width; x < w; x++) {
                                    const nanoem_rsize_t srcOffset = (y * frameControl.m_width + x) * 4,
                                                         dstOffset = (((y + yoffset) * width) + (x + xoffset)) * 4;
                                    const nanoem_u8_t a = imageDataPtr[srcOffset + 3];
                                    float alpha = a / 255.0f;
                                    for (int i = 0; i < 3; i++) {
                                        const nanoem_u8_t fg = alpha * imageDataPtr[srcOffset + i];
                                        const nanoem_u8_t bg = (1.0f - alpha) * compositionImageDataPtr[dstOffset + i];
                                        compositionImageDataPtr[dstOffset + i] = fg + bg;
                                    }
                                    if (compositionImageDataPtr[dstOffset + 3] == 0) {
                                        compositionImageDataPtr[dstOffset + 3] = a;
                                    }
                                    i0 = Inline::saturateInt32U(srcOffset);
                                    i1 = Inline::saturateInt32U(dstOffset);
                                }
                            }
                        }
                        bimg::imageFree(container);
                    }
                    numFrames++;
                }
                frame->m_composition = compositionImageData;
            }
        }
        else {
            nanoem_delete_safe(animation);
        }
    }
    return animation;
}

void
ImageLoader::copyImageDescrption(const sg_image_desc &desc, Image *image)
{
    image->setDescription(desc);
    const sg_range &src = desc.data.subimage[0][0];
    const nanoem_u8_t *dataPtr = static_cast<const nanoem_u8_t *>(src.ptr);
    image->setOriginData(dataPtr, src.size);
    if (desc.num_mipmaps > 1) {
        const int numMipmaps = desc.num_mipmaps - 1;
        image->resizeMipmapData(numMipmaps);
        for (int i = 0; i < numMipmaps; i++) {
            const sg_range &innerSrc = desc.data.subimage[0][i + 1];
            const nanoem_u8_t *innerDataPtr = static_cast<const nanoem_u8_t *>(innerSrc.ptr);
            image->setMipmapData(i, innerDataPtr, innerSrc.size);
        }
    }
}

bool
ImageLoader::generateMipmapImages(
    const bimg::ImageContainer *container, bool flip, ByteArrayList &mipmapPayloads, sg_image_desc &descRef)
{
    bool result = false;
    if (container->m_numMips > 1) {
        const nanoem_u16_t numSides = container->m_numLayers * (container->m_cubeMap ? 6 : 1);
        const nanoem_u8_t numMips = glm::min(nanoem_u8_t(SG_MAX_MIPMAPS), container->m_numMips);
        for (nanoem_u16_t i = 0; i < numSides; i++) {
            sg_range *ptr = descRef.data.subimage[i];
            for (nanoem_u8_t j = 0; j < numMips; j++) {
                bimg::ImageMip mip;
                if (bimg::imageGetRawData(*container, i, j, container->m_data, container->m_size, mip)) {
                    sg_range &content = ptr[j + 1];
                    content.ptr = mip.m_data;
                    content.size = mip.m_size;
                }
            }
        }
        result = true;
    }
    else if (descRef.num_mipmaps > 1) {
        const int numMips = glm::min(int(SG_MAX_MIPMAPS), descRef.num_mipmaps);
        mipmapPayloads.resize(numMips + 1);
        if (descRef.pixel_format == SG_PIXELFORMAT_RGBA32F) {
            generateMipmapImagesRGBA32F(container, numMips, flip, mipmapPayloads, descRef);
        }
        else if (descRef.pixel_format == SG_PIXELFORMAT_RGBA8) {
            generateMipmapImagesRGBA8(container, numMips, flip, mipmapPayloads, descRef);
        }
        else {
            const nanoem_u8_t *dataPtr = static_cast<const nanoem_u8_t *>(container->m_data);
            ByteArray &bytesRef = mipmapPayloads[0];
            sg_range &content = descRef.data.subimage[0][0];
            bytesRef.assign(dataPtr, dataPtr + container->m_size);
            content.ptr = bytesRef.data();
            content.size = bytesRef.size();
            descRef.num_mipmaps = 1;
            switch (descRef.min_filter) {
            case SG_FILTER_LINEAR_MIPMAP_LINEAR:
            case SG_FILTER_LINEAR_MIPMAP_NEAREST:
                descRef.min_filter = SG_FILTER_LINEAR;
                break;
            case SG_FILTER_NEAREST_MIPMAP_LINEAR:
            case SG_FILTER_NEAREST_MIPMAP_NEAREST:
                descRef.min_filter = SG_FILTER_LINEAR;
            default:
                break;
            }
        }
        result = true;
    }
    return result;
}

bool
ImageLoader::validateImageSize(const String &name, const sg_image_desc &desc, Error &error)
{
    const sg_limits limits = sg::query_limits();
    const int width = Inline::roundInt32(desc.width), height = Inline::roundInt32(desc.height),
              depth = Inline::roundInt32(desc.num_slices);
    char message[Error::kMaxReasonLength], recoverySuggestion[Error::kMaxRecoverySuggestionLength];
    Error localError;
    if (desc.type == SG_IMAGETYPE_CUBE && limits.max_image_size_cube > 0 &&
        (width >= limits.max_image_size_cube || height >= limits.max_image_size_cube)) {
        StringUtils::format(message, sizeof(message), "The texture %s exceeds limit (%dx%d) > (%dx%d)", name.c_str(),
            desc.width, desc.height, limits.max_image_size_cube, limits.max_image_size_cube);
        StringUtils::format(recoverySuggestion, sizeof(recoverySuggestion),
            "Resize the texture %s equal or less than (%dx%d)", name.c_str(), limits.max_image_size_cube,
            limits.max_image_size_cube);
        localError = Error(message, recoverySuggestion, Error::kDomainTypeApplication);
    }
    else if (desc.type == SG_IMAGETYPE_3D && limits.max_image_size_3d > 0 &&
        (width >= limits.max_image_size_3d || height >= limits.max_image_size_3d ||
            depth >= limits.max_image_size_3d)) {
        StringUtils::format(message, sizeof(message), "The texture %s exceeds limit (%dx%dx%d) > (%dx%dx%d)",
            name.c_str(), desc.width, desc.height, desc.num_slices, limits.max_image_size_3d, limits.max_image_size_3d,
            limits.max_image_size_3d);
        StringUtils::format(recoverySuggestion, sizeof(recoverySuggestion),
            "Resize the texture %s equal or less than (%dx%dx%d)", name.c_str(), limits.max_image_size_3d,
            limits.max_image_size_3d, limits.max_image_size_3d);
        localError = Error(message, recoverySuggestion, Error::kDomainTypeApplication);
    }
    else if (limits.max_image_size_2d > 0 &&
        (width >= limits.max_image_size_2d || height >= limits.max_image_size_2d)) {
        StringUtils::format(message, sizeof(message), "The texture %s exceeds limit (%dx%d) > (%dx%d)", name.c_str(),
            desc.width, desc.height, limits.max_image_size_2d, limits.max_image_size_2d);
        StringUtils::format(recoverySuggestion, sizeof(recoverySuggestion),
            "Resize the texture %s equal or less than (%dx%d)", name.c_str(), limits.max_image_size_2d,
            limits.max_image_size_2d);
        localError = Error(message, recoverySuggestion, Error::kDomainTypeApplication);
    }
    bool hitError = localError.hasReason();
    if (hitError) {
        error = localError;
    }
    return !hitError;
}

bool
ImageLoader::isScreenBMP(const char *path) NANOEM_DECL_NOEXCEPT
{
    static const char kScreenBmp[] = "screen.bmp";
    return path && *path == 's' && StringUtils::equals(path, kScreenBmp);
}

void
ImageLoader::fill1x1PixelImage(const nanoem_u32_t *pixel, sg_image_desc &desc) NANOEM_DECL_NOEXCEPT
{
    Inline::clearZeroMemory(desc);
    desc.width = desc.height = desc.sample_count = 1;
    desc.type = SG_IMAGETYPE_2D;
    desc.mag_filter = desc.min_filter = SG_FILTER_NEAREST;
    desc.wrap_u = desc.wrap_v = SG_WRAP_REPEAT;
    desc.pixel_format = SG_PIXELFORMAT_RGBA8;
    desc.data.subimage[0][0].ptr = pixel;
    desc.data.subimage[0][0].size = sizeof(*pixel);
}

void
ImageLoader::fill1x1WhitePixelImage(sg_image_desc &desc) NANOEM_DECL_NOEXCEPT
{
    static const nanoem_u32_t kPixel = 0xffffffff;
    fill1x1PixelImage(&kPixel, desc);
}

void
ImageLoader::fill1x1BlackPixelImage(sg_image_desc &desc) NANOEM_DECL_NOEXCEPT
{
    static const nanoem_u32_t kPixel = 0xff000000;
    fill1x1PixelImage(&kPixel, desc);
}

void
ImageLoader::fill1x1TransparentPixelImage(sg_image_desc &desc) NANOEM_DECL_NOEXCEPT
{
    static const nanoem_u32_t kPixel = 0;
    fill1x1PixelImage(&kPixel, desc);
}

void
ImageLoader::flipImage(nanoem_u8_t *source, nanoem_u32_t width, nanoem_u32_t height, nanoem_u32_t bytesPerPixel)
{
    const nanoem_u32_t stride = width * bytesPerPixel, rows = height / 2;
    ByteArray buffer(stride);
    for (nanoem_u32_t i = 0; i < rows; i++) {
        const nanoem_u32_t sourceOffset = i * stride;
        const nanoem_u32_t targetOffset = (height - i - 1) * stride;
        memcpy(buffer.data(), source + sourceOffset, stride);
        memcpy(source + sourceOffset, source + targetOffset, stride);
        memcpy(source + targetOffset, buffer.data(), stride);
    }
}

APNGImage::~APNGImage()
{
    for (APNGImage::FrameSequenceList::const_iterator it = m_frames.begin(), end = m_frames.end(); it != end; ++it) {
        APNGImage::Frame *frame = *it;
        nanoem_delete(frame);
    }
    m_frames.clear();
}

nanoem_rsize_t
APNGImage::seek(nanoem_f32_t seconds) const
{
    nanoem_rsize_t offset = 0;
    if (!m_frames.empty()) {
        const nanoem_f32_t duration = m_frames.back()->m_seconds, value = fmod(seconds, duration);
        for (nanoem_rsize_t i = 0, numFrames = m_frames.size() - 1; i < numFrames; i++) {
            const nanoem_f32_t v0 = m_frames[i]->m_seconds;
            const nanoem_f32_t v1 = m_frames[i + 1]->m_seconds;
            if (v0 < value && v1 > value) {
                offset = i;
                break;
            }
        }
    }
    return offset;
}

ImageLoader::ImageLoader(const Project *project)
    : m_project(project)
{
}

ImageLoader::~ImageLoader()
{
}

IImageView *
ImageLoader::load(const URI &fileURI, IDrawable *drawable, sg_wrap wrap, nanoem_u32_t flags, Error &error)
{
    nanoem_parameter_assert(!fileURI.isEmpty(), "must NOT be empty");
    IImageView *imageView = nullptr;
    const String filename(FileUtils::relativePath(
            fileURI.absolutePath(), drawable->fileURI().absolutePathByDeletingLastPathComponent()));
    const char lastChr = filename.empty() ? 0 : *(filename.c_str() + filename.size() - 1);
    if (lastChr != '/' && FileUtils::exists(fileURI)) {
        FileReaderScope scope(nullptr);
        if (scope.open(fileURI, error)) {
            ByteArray bytes;
            FileUtils::read(scope, bytes, error);
            if (!error.hasReason()) {
                if (!m_project->isMipmapEnabled()) {
                    flags &= ~ImageLoader::kFlagsEnableMipmap;
                }
                const ImmutableImageContainer container(
                    filename, bytes, Vector2UI16(), wrap, m_project->maxAnisotropyValue(), flags);
                imageView = decodeImageContainer(container, drawable, error);
            }
        }
    }
    return imageView;
}

IImageView *
ImageLoader::decode(
    const ByteArray &bytes, const String &filename, IDrawable *drawable, sg_wrap wrap, nanoem_u32_t flags, Error &error)
{
    const ImageLoader::ImmutableImageContainer container(
        filename, bytes, Vector2UI16(), wrap, m_project->maxAnisotropyValue(), flags);
    return decodeImageContainer(container, drawable, error);
}

IImageView *
ImageLoader::decodeImageContainer(const ImmutableImageContainer &container, IDrawable *drawable, Error &error)
{
    bx::Error err;
    sg_image_desc desc;
    IImageView *imageView = nullptr;
    int width, height, components;
    Inline::clearZeroMemory(desc);
    if (stbi_uc *data = stbi_load_from_memory(
            container.m_dataPtr, Inline::saturateInt32(container.m_dataSize), &width, &height, &components, 4)) {
        desc.width = width;
        desc.height = height;
        desc.pixel_format = SG_PIXELFORMAT_RGBA8;
        desc.min_filter = SG_FILTER_LINEAR;
        desc.mag_filter = SG_FILTER_LINEAR;
        desc.max_anisotropy = container.m_anisotropy;
        desc.wrap_u = desc.wrap_v = container.m_wrap;
        sg_range &content = desc.data.subimage[0][0];
        content.ptr = data;
        content.size = nanoem_rsize_t(4) * width * height;
        textureHandle = drawable->uploadImage(container.m_name, desc);
        stbi_image_free(data);
    }
    else if (bimg::ImageContainer *decodedImageContainer = bimg::imageParse(g_bimg_allocator, container.m_dataPtr,
                 Inline::saturateInt32U(container.m_dataSize), bimg::TextureFormat::Count, &err)) {
        const nanoem_u32_t widthU = decodedImageContainer->m_width, heightU = decodedImageContainer->m_height;
        nanoem_u32_t bytesPerPixel;
        desc.width = Inline::saturateInt32(widthU);
        desc.height = Inline::saturateInt32(heightU);
        desc.pixel_format = resolvePixelFormat(decodedImageContainer, bytesPerPixel);
        if (validateImageSize(container.m_name, desc, error)) {
            const bimg::TextureFormat::Enum imageFormat = decodedImageContainer->m_format;
            const bool needsRGBA8Conversion =
                imageFormat != bimg::TextureFormat::RGBA8 && imageFormat != bimg::TextureFormat::RGBA16;
            ByteArrayList mipmapPayloads;
            ByteArray decodedRGBA8;
            if (EnumUtils::isEnabled(container.m_flags, kFlagsEnableMipmap)) {
                const Vector2 size(desc.width, desc.height);
                desc.num_mipmaps = glm::min(int(glm::log2(glm::max(size.x, size.y))) + 1, int(SG_MAX_MIPMAPS));
                desc.min_filter = SG_FILTER_LINEAR_MIPMAP_LINEAR;
                if (needsRGBA8Conversion) {
                    desc.pixel_format = SG_PIXELFORMAT_RGBA8;
                }
                const bool flip = EnumUtils::isEnabled(container.m_flags, kFlagsEnableFlipY);
                if (!generateMipmapImages(decodedImageContainer, flip, mipmapPayloads, desc)) {
                    ensureRGBA8ImageData(decodedImageContainer, needsRGBA8Conversion, decodedRGBA8, desc);
                }
            }
            else {
                ensureRGBA8ImageData(decodedImageContainer, needsRGBA8Conversion, decodedRGBA8, desc);
                if (EnumUtils::isEnabled(container.m_flags, kFlagsEnableFlipY)) {
                    flipImage(decodedRGBA8.data(), widthU, heightU, bytesPerPixel);
                }
                desc.min_filter = SG_FILTER_LINEAR;
            }
            bimg::imageFree(decodedImageContainer);
            desc.mag_filter = SG_FILTER_LINEAR;
            desc.max_anisotropy = container.m_anisotropy;
            desc.wrap_u = desc.wrap_v = container.m_wrap;
            imageView = drawable->uploadImage(container.m_name, desc);
        }
    }
    else {
        error = Error(err.getMessage().getPtr(), err.get().code, Error::kDomainTypeOS);
    }
    return imageView;
}

void
ImageLoader::generateMipmapImagesRGBA32F(const bimg::ImageContainer *container, int numMips, bool flip,
    ByteArrayList &mipmapPayloads, sg_image_desc &descRef)
{
    const bimg::ImageBlockInfo &info = bimg::getBlockInfo(container->m_format);
    const nanoem_u8_t *dataPtr = static_cast<const nanoem_u8_t *>(container->m_data), blockWidth = info.blockWidth;
    nanoem_u32_t width = descRef.width, height = descRef.height;
    ByteArray decodedBuffer(dataPtr, dataPtr + container->m_size), &bytesRef = mipmapPayloads[0];
    sg_range &content = descRef.data.subimage[0][0];
    if (flip) {
        flipImage(decodedBuffer.data(), width, height, info.bitsPerPixel / 8);
    }
    bytesRef = decodedBuffer;
    content.ptr = bytesRef.data();
    content.size = bytesRef.size();
    for (int i = 0; i < numMips; i++) {
        nanoem_u32_t widthHalf = glm::max(width >> 1, 1u);
        nanoem_u32_t heightHalf = glm::max(height >> 1, 1u);
        const nanoem_u32_t stride = bx::strideAlign(widthHalf, blockWidth) * 16;
        sg_range &content = descRef.data.subimage[0][i + 1];
        bimg::imageRgba32fDownsample2x2(decodedBuffer.data(), width, height, 1, stride, decodedBuffer.data());
        ByteArray &mipmapBytesRef = mipmapPayloads[i + 1];
        mipmapBytesRef.assign(
            decodedBuffer.data(), decodedBuffer.data() + heightHalf * static_cast<nanoem_rsize_t>(stride));
        content.ptr = mipmapBytesRef.data();
        content.size = mipmapBytesRef.size();
        width = widthHalf;
        height = heightHalf;
    }
}

void
ImageLoader::generateMipmapImagesRGBA8(const bimg::ImageContainer *container, int numMips, bool flip,
    ByteArrayList &mipmapPayloads, sg_image_desc &descRef)
{
    const bimg::ImageBlockInfo &info = bimg::getBlockInfo(container->m_format);
    const nanoem_u8_t *dataPtr = static_cast<const nanoem_u8_t *>(container->m_data), blockWidth = info.blockWidth;
    nanoem_u32_t width = descRef.width, height = descRef.height;
    nanoem_u32_t imageSize =
        bimg::imageGetSize(nullptr, width, height, container->m_depth, false, false, 1, bimg::TextureFormat::RGBA8);
    ByteArray decodedBuffer(imageSize), &bytesRef = mipmapPayloads[0];
    if (container->m_format != bimg::TextureFormat::RGBA8) {
        bimg::imageDecodeToRgba8(
            g_bimg_allocator, decodedBuffer.data(), container->m_data, width, height, width * 4, container->m_format);
    }
    else {
        decodedBuffer.assign(dataPtr, dataPtr + container->m_size);
    }
    if (flip) {
        flipImage(decodedBuffer.data(), width, height, info.bitsPerPixel / 8);
    }
    bytesRef = decodedBuffer;
    sg_range &data = descRef.data.subimage[0][0];
    data.ptr = bytesRef.data();
    data.size = bytesRef.size();
    for (int i = 0; i < numMips; i++) {
        nanoem_u32_t widthHalf = glm::max(width >> 1, 1u);
        nanoem_u32_t heightHalf = glm::max(height >> 1, 1u);
        const nanoem_u32_t stride = bx::strideAlign(widthHalf, blockWidth) * 4;
        sg_range &innerData = descRef.data.subimage[0][i + 1];
        bimg::imageRgba8Downsample2x2(decodedBuffer.data(), width, height, 1, width * 4, stride, decodedBuffer.data());
        ByteArray &mipmapBytesRef = mipmapPayloads[i + 1];
        mipmapBytesRef.assign(
            decodedBuffer.data(), decodedBuffer.data() + heightHalf * static_cast<nanoem_rsize_t>(stride));
        innerData.ptr = mipmapBytesRef.data();
        innerData.size = mipmapBytesRef.size();
        width = widthHalf;
        height = heightHalf;
    }
}

void
ImageLoader::ensureRGBA8ImageData(const bimg::ImageContainer *decodedImageContainer, bool needsRGBA8Conversion,
    ByteArray &decodedRGBA8, sg_image_desc &desc)
{
    const nanoem_u32_t width = decodedImageContainer->m_width, height = decodedImageContainer->m_height;
    if (needsRGBA8Conversion) {
        bimg::TextureInfo info;
        const nanoem_u32_t dataSize = bimg::imageGetSize(
            &info, width, decodedImageContainer->m_height, 1, false, false, 1, bimg::TextureFormat::RGBA8);
        const nanoem_u32_t stride = width * (info.bitsPerPixel / 8);
        decodedRGBA8.resize(dataSize);
        bimg::imageDecodeToRgba8(g_bimg_allocator, decodedRGBA8.data(), decodedImageContainer->m_data, width, height,
            stride, decodedImageContainer->m_format);
        desc.pixel_format = SG_PIXELFORMAT_RGBA8;
    }
    else {
        const nanoem_u8_t *dataPtr = static_cast<const nanoem_u8_t *>(decodedImageContainer->m_data);
        decodedRGBA8.assign(dataPtr, dataPtr + decodedImageContainer->m_size);
    }
    sg_range &content = desc.data.subimage[0][0];
    content.ptr = decodedRGBA8.data();
    content.size = decodedRGBA8.size();
}

} /* namespace nanoem */
