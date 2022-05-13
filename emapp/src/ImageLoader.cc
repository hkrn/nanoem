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

#include "bx/file.h"

extern "C" {
void *__stb_malloc(size_t size, const char *file, int line);
void *__stb_realloc(void *ptr, size_t size, const char *file, int line);
void __stb_free(void *ptr, const char *file, int line);
}

#define STBI_NO_STDIO
#define STBI_FAILURE_USERMSG
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

image::APNG::APNG()
{
}

image::APNG::~APNG() NANOEM_DECL_NOEXCEPT
{
    for (APNG::FrameSequenceList::const_iterator it = m_frames.begin(), end = m_frames.end(); it != end; ++it) {
        APNG::Frame *frame = *it;
        nanoem_delete(frame);
    }
    m_frames.clear();
}

bool
image::APNG::decode(ISeekableReader *reader, Error &error)
{
    ByteArray dataChunk;
    CRC crc;
    APNG::Frame *frame = nullptr;
    nanoem_u32_t expectedSequenceNumber = 0;
    nanoem_f32_t seconds = 0;
    bool loop = true;
    while (loop && !error.hasReason()) {
        nanoem_u32_t chunkLength, chunkType, chunkCRC, checksum = 0;
        if (FileUtils::readTyped(reader, chunkLength, error) != Inline::saturateInt32(chunkLength)) {
            loop = false;
            break;
        }
        chunkLength = bx::toBigEndian(chunkLength);
        if (FileUtils::readTyped(reader, chunkType, error) != Inline::saturateInt32(chunkType)) {
            loop = false;
            break;
        }
        switch (chunkType) {
        case kPNGChunkTypeAnimationControl: {
            if (FileUtils::readTyped(reader, m_control, error) != Inline::saturateInt32(sizeof(m_control))) {
                loop = false;
                break;
            }
            checksum = crc.checksumTyped(chunkType, m_control);
            m_control.m_numFrames = bx::toBigEndian(m_control.m_numFrames);
            m_control.m_numPlayCount = bx::toBigEndian(m_control.m_numPlayCount);
            break;
        }
        case kPNGChunkTypeFrameControl: {
            if (frame) {
                if (frame->m_sequences.empty()) {
                    error = Error("APNG: Empty image sequence", nullptr, Error::kDomainTypeApplication);
                }
                else {
                    m_frames.push_back(frame);
                }
            }
            if (!error.hasReason()) {
                APNG::FrameControl &frameControl = frame->m_control;
                if (FileUtils::readTyped(reader, frameControl, error) != Inline::saturateInt32(sizeof(frameControl))) {
                    loop = false;
                    break;
                }
                checksum = crc.checksumTyped(chunkType, frameControl);
                frame = nanoem_new(APNG::Frame);
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
                    error = Error("APNG: Invalid sequence number", nullptr, Error::kDomainTypeApplication);
                }
                else if (frameControl.m_width == 0 || frameControl.m_height == 0 ||
                    !(frameControl.m_xoffset + frameControl.m_width <= m_header.m_width) ||
                    !(frameControl.m_yoffset + frameControl.m_height <= m_header.m_height)) {
                    nanoem_delete_safe(frame);
                    error = Error("APNG: Invalid frame rectangle", nullptr, Error::kDomainTypeApplication);
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
                if (FileUtils::read(reader, dataChunk.data(), chunkLength, error) != Inline::saturateInt32(chunkLength)) {
                    loop = false;
                    break;
                }
                checksum = crc.checksum(chunkType, dataChunk.data(), chunkLength);
                sequenceNumber = bx::toBigEndian(*reinterpret_cast<const nanoem_u32_t *>(dataChunk.data()));
                if (expectedSequenceNumber != sequenceNumber) {
                    error = Error("APNG: Incorrect sequence number", nullptr, Error::kDomainTypeApplication);
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
            if (FileUtils::read(reader, dataChunk.data(), chunkLength, error) != Inline::saturateInt32(chunkLength)) {
                loop = false;
                break;
            }
            checksum = crc.checksum(chunkType, dataChunk.data(), chunkLength);
            if (frame) {
                frame->m_sequences.push_back(dataChunk);
            }
            break;
        }
        case kPNGChunkTypeImageEnd: {
            if (frame) {
                if (frame->m_sequences.empty()) {
                    error = Error("APNG: Empty image sequence", nullptr, Error::kDomainTypeApplication);
                }
                else {
                    m_frames.push_back(frame);
                }
            }
            checksum = crc.checksum(chunkType, nullptr, 0);
            loop = false;
            break;
        }
        case kPNGChunkTypeImageHeader: {
            if (FileUtils::readTyped(reader, m_header, error) != Inline::saturateInt32(sizeof(m_header))) {
                loop = false;
                break;
            }
            checksum = crc.checksumTyped(chunkType, m_header);
            m_header.m_width = bx::toBigEndian(m_header.m_width);
            m_header.m_height = bx::toBigEndian(m_header.m_height);
            break;
        }
        default:
            reader->seek(chunkLength, IFileReader::kSeekTypeCurrent, error);
            break;
        }
        FileUtils::readTyped(reader, chunkCRC, error);
        if (chunkCRC != checksum) {
            error = Error("APNG: CRC checksum not matched", nullptr, Error::kDomainTypeApplication);
            loop = false;
        }
    }
    if (!error.hasReason() && m_control.m_numFrames != m_frames.size()) {
        error = Error("APNG: Frame count doesn't equal", nullptr, Error::kDomainTypeApplication);
    }
    return !error.hasReason();
}

void
image::APNG::composite(Error &error)
{
    CRC crc;
    int numFrames = 0;
    ByteArray compositionImageData, buffer;
    MemoryWriter writer(&buffer);
    const nanoem_u32_t width = m_header.m_width, height = m_header.m_height;
    compositionImageData.resize(nanoem_rsize_t(4) * width * height);
    for (APNG::FrameSequenceList::const_iterator it = m_frames.begin(),
                                                      end = m_frames.end();
         it != end; ++it) {
        APNG::Frame *frame = *it;
        const APNG::FrameControl &frameControl = frame->m_control;
        switch (frameControl.m_disposeOp) {
        case APNG::kDisposeOpPrevious: {
            if (it > m_frames.begin()) {
                const APNG::Frame *previousFrame = *(it - 1);
                compositionImageData = previousFrame->m_composition;
            }
            break;
        }
        case APNG::kDisposeOpBackground: {
            memset(compositionImageData.data(), 0, compositionImageData.size());
            break;
        }
        case APNG::kDisposeOpNone:
        default:
            break;
        }
        for (ByteArrayList::const_iterator it2 = frame->m_sequences.begin(), end2 = frame->m_sequences.end();
             it2 != end2; ++it2) {
            const ByteArray &bytes = *it2;
            APNG::Header header(m_header);
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
            Error err;
            sg_image_desc desc;
            nanoem_u8_t *imageDataPtr = nullptr;
            if (ImageLoader::decodeImageWithSTB(buffer.data(), buffer.size(), desc, &imageDataPtr, err)) {
                if (frameControl.m_blendOp == APNG::kBlendOpSource) {
                    const nanoem_u32_t *imageDataPtr = static_cast<const nanoem_u32_t *>(imageDataPtr);
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
                else if (frameControl.m_blendOp == APNG::kBlendOpOver) {
                    const nanoem_u32_t xoffset = frameControl.m_xoffset, yoffset = frameControl.m_yoffset;
                    const nanoem_u8_t *imageDataPtr = static_cast<const nanoem_u8_t *>(imageDataPtr);
                    nanoem_u8_t *compositionImageDataPtr = compositionImageData.data();
                    // nanoem_u32_t i0, i1;
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
                            // i0 = Inline::saturateInt32U(srcOffset);
                            // i1 = Inline::saturateInt32U(dstOffset);
                        }
                    }
                }
                ImageLoader::releaseDecodedImageWithSTB(&imageDataPtr);
            }
            numFrames++;
        }
        frame->m_composition = compositionImageData;
    }
}

nanoem_rsize_t
image::APNG::findNearestOffset(nanoem_f32_t seconds) const NANOEM_DECL_NOEXCEPT
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

const ByteArray *
image::APNG::compositedFrameImage(nanoem_rsize_t offset) const NANOEM_DECL_NOEXCEPT
{
    const ByteArray *composition = nullptr;
    if (offset < m_frames.size()) {
        composition = &m_frames[offset]->m_composition;
    }
    return composition;
}

nanoem_u32_t
image::APNG::width() const NANOEM_DECL_NOEXCEPT
{
    return m_header.m_width;
}

nanoem_u32_t
image::APNG::height() const NANOEM_DECL_NOEXCEPT
{
    return m_header.m_height;
}

Image::Image()
    : m_fileExist(false)
{
    Inline::clearZeroMemory(m_description);
    m_handle = { SG_INVALID_ID };
}

Image::~Image() NANOEM_DECL_NOEXCEPT
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
    return index < SG_MAX_MIPMAPS ? &m_mipmapData[index] : nullptr;
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

image::APNG *
ImageLoader::decodeAPNG(ISeekableReader *reader, Error &error)
{
    nanoem_u64_t signature;
    FileUtils::readTyped(reader, signature, error);
    image::APNG *image = nullptr;
    if (signature == kPNGSignature) {
        image = nanoem_new(image::APNG);
        if (image->decode(reader, error)) {
            image->composite(error);
        }
        else {
            nanoem_delete_safe(image);
        }
    }
    return image;
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
        for (int i = 0; i < numMipmaps; i++) {
            const sg_range &innerSrc = desc.data.subimage[0][i + 1];
            const nanoem_u8_t *innerDataPtr = static_cast<const nanoem_u8_t *>(innerSrc.ptr);
            image->setMipmapData(i, innerDataPtr, innerSrc.size);
        }
    }
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

bool
ImageLoader::decodeImageWithSTB(const nanoem_u8_t *dataPtr, const size_t dataSize, sg_image_desc &desc, nanoem_u8_t **decodedImagePtr, Error &error)
{
    int width, height, components;
    bool result = false;
    if (stbi_uc *data =
            stbi_load_from_memory(dataPtr, Inline::saturateInt32(dataSize), &width, &height, &components, 4)) {
        desc.width = width;
        desc.height = height;
        desc.pixel_format = SG_PIXELFORMAT_RGBA8;
        desc.min_filter = SG_FILTER_LINEAR;
        desc.mag_filter = SG_FILTER_LINEAR;
        sg_range &content = desc.data.subimage[0][0];
        content.ptr = data;
        content.size = nanoem_rsize_t(4) * width * height;
        *decodedImagePtr = data;
        result = true;
    }
    else {
        error = Error(stbi_failure_reason(), 0, Error::kDomainTypeApplication);
    }
    return result;
}

void
ImageLoader::releaseDecodedImageWithSTB(nanoem_u8_t **decodedImagePtr)
{
    stbi_image_free(*decodedImagePtr);
    *decodedImagePtr = nullptr;
}

ImageLoader::ImageLoader(const Project *project)
    : m_project(project)
{
}

ImageLoader::~ImageLoader() NANOEM_DECL_NOEXCEPT
{
}

IImageView *
ImageLoader::load(const URI &fileURI, IDrawable *drawable, sg_wrap wrap, nanoem_u32_t flags, Error &error)
{
    nanoem_parameter_assert(!fileURI.isEmpty(), "must NOT be empty");
    IImageView *imageView = nullptr;
    const String filename(
        FileUtils::relativePath(fileURI.absolutePath(), drawable->fileURI().absolutePathByDeletingLastPathComponent()));
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
    sg_image_desc desc;
    IImageView *imageView = nullptr;
    nanoem_u8_t *decodedImagePtr = nullptr;
    Inline::clearZeroMemory(desc);
    if (decodeImageWithSTB(container.m_dataPtr, container.m_dataSize, desc, &decodedImagePtr, error)) {
        desc.max_anisotropy = container.m_anisotropy;
        desc.wrap_u = desc.wrap_v = container.m_wrap;
        imageView = drawable->uploadImage(container.m_name, desc);
        releaseDecodedImageWithSTB(&decodedImagePtr);
    }
    return imageView;
}

} /* namespace nanoem */
