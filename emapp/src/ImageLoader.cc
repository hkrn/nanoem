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

/* for sscanf */
#include <stdio.h>

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
static const nanoem_u32_t kPNGImageSizeHardLimit = 16384;
static const nanoem_u32_t kPNGChunkTypeImageData = nanoem_fourcc('I', 'D', 'A', 'T');
static const nanoem_u32_t kPNGChunkTypeImageHeader = nanoem_fourcc('I', 'H', 'D', 'R');
static const nanoem_u32_t kPNGChunkTypeImageEnd = nanoem_fourcc('I', 'E', 'N', 'D');
static const nanoem_u32_t kPNGChunkTypeAnimationControl = nanoem_fourcc('a', 'c', 'T', 'L');
static const nanoem_u32_t kPNGChunkTypeFrameControl = nanoem_fourcc('f', 'c', 'T', 'L');
static const nanoem_u32_t kPNGChunkTypeFrameData = nanoem_fourcc('f', 'd', 'A', 'T');

static const nanoem_u32_t kDDSImageSizeHardLimit = 16384;
static const nanoem_u32_t kDDSImageDepthHardLimit = 2048;
static const nanoem_u32_t kDDSHeaderFlagTypeMipmap = 0x20000;
static const nanoem_u32_t kDDSHeaderFlagTypeVolume = 0x800000;
static const nanoem_u32_t kDDSFormatFlagTypeRGB = 0x40;
static const nanoem_u32_t kDDSFormatFlagTypeLuminance = 0x00020000;
static const nanoem_u32_t kDDSFormatFlagTypeFourCC = 0x4;
static const nanoem_u32_t kDDSCapsCubeMapPositiveX = 0x600;
static const nanoem_u32_t kDDSCapsCubeMapNegativeX = 0xa00;
static const nanoem_u32_t kDDSCapsCubeMapPositiveY = 0x1200;
static const nanoem_u32_t kDDSCapsCubeMapNegativeY = 0x2200;
static const nanoem_u32_t kDDSCapsCubeMapPositiveZ = 0x4200;
static const nanoem_u32_t kDDSCapsCubeMapNegativeZ = 0x8200;
static const nanoem_u32_t kDDSCapsCubeMapAllFaces = kDDSCapsCubeMapPositiveX | kDDSCapsCubeMapNegativeX |
    kDDSCapsCubeMapPositiveY | kDDSCapsCubeMapNegativeY | kDDSCapsCubeMapPositiveZ | kDDSCapsCubeMapNegativeZ;

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

/* https://docs.microsoft.com/en-us/windows/win32/api/dxgiformat/ne-dxgiformat-dxgi_format */
enum DXGI_FORMAT {
    DXGI_FORMAT_R32G32B32A32_FLOAT = 2,
    DXGI_FORMAT_R16G16B16A16_FLOAT = 10,
    DXGI_FORMAT_R16G16B16A16_UNORM = 11,
    DXGI_FORMAT_R16G16B16A16_SNORM = 13,
    DXGI_FORMAT_R32G32_FLOAT = 16,
    DXGI_FORMAT_R10G10B10A2_UNORM = 24,
    DXGI_FORMAT_R8G8B8A8_UNORM = 28,
    DXGI_FORMAT_R8G8B8A8_UNORM_SRGB = 29,
    DXGI_FORMAT_R8G8B8A8_SNORM = 31,
    DXGI_FORMAT_R16G16_FLOAT = 34,
    DXGI_FORMAT_R16G16_UNORM = 35,
    DXGI_FORMAT_R16G16_SNORM = 37,
    DXGI_FORMAT_R32_FLOAT = 41,
    DXGI_FORMAT_R8G8_UNORM = 49,
    DXGI_FORMAT_R8G8_SNORM = 51,
    DXGI_FORMAT_R16_FLOAT = 54,
    DXGI_FORMAT_R16_UNORM = 56,
    DXGI_FORMAT_R16_SNORM = 58,
    DXGI_FORMAT_R8_UNORM = 61,
    DXGI_FORMAT_R8_SNORM = 63,
    DXGI_FORMAT_A8_UNORM = 65,
    DXGI_FORMAT_BC1_UNORM = 71,
    DXGI_FORMAT_BC1_UNORM_SRGB = 72,
    DXGI_FORMAT_BC2_UNORM = 74,
    DXGI_FORMAT_BC2_UNORM_SRGB = 75,
    DXGI_FORMAT_BC3_UNORM = 77,
    DXGI_FORMAT_BC3_UNORM_SRGB = 78,
    DXGI_FORMAT_BC4_UNORM = 80,
    DXGI_FORMAT_BC4_SNORM = 81,
    DXGI_FORMAT_BC5_UNORM = 83,
    DXGI_FORMAT_BC5_SNORM = 84,
    DXGI_FORMAT_B5G6R5_UNORM = 85,
    DXGI_FORMAT_B5G5R5A1_UNORM = 86,
    DXGI_FORMAT_B8G8R8A8_UNORM = 87,
    DXGI_FORMAT_B8G8R8X8_UNORM = 88,
    DXGI_FORMAT_B8G8R8A8_UNORM_SRGB = 91,
    DXGI_FORMAT_B8G8R8X8_UNORM_SRGB = 93,
    DXGI_FORMAT_FORCE_UINT = 0xffffffff
};

/* https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_resource_dimension */
enum D3D11_RESOURCE_DIMENSION { D3D11_RESOURCE_DIMENSION_TEXTURE2D = 3, D3D11_RESOURCE_DIMENSION_TEXTURE3D = 4 };

/* https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_resource_misc_flag */
enum D3D11_RESOURCE_MISC_FLAG {
    D3D11_RESOURCE_MISC_TEXTURECUBE = 0x4L,
};

} /* namespace anonymous */

struct image::APNG::State {
    ByteArray m_dataChunk;
    CRC m_crc;
    APNG::Frame *m_frame;
    nanoem_u32_t m_expectedSequenceNumber;
    nanoem_f32_t m_seconds;
    State()
        : m_frame(nullptr)
        , m_expectedSequenceNumber(0)
        , m_seconds(0)
    {
    }
    ~State()
    {
        nanoem_delete(m_frame);
    }
};

image::APNG::APNG()
{
    Inline::clearZeroMemory(m_header);
    Inline::clearZeroMemory(m_control);
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
    State state;
    char message[Error::kMaxReasonLength];
    bool hasAnimationControl = false, hasImageData = false, loop = true;
    while (loop && !error.hasReason()) {
        nanoem_u32_t chunkLength, chunkType, chunkCRC, checksum = 0;
        if (FileUtils::readTyped(reader, chunkLength, error) != Inline::saturateInt32(sizeof(chunkLength))) {
            error = Error("APNG: Corrupted chunk length", 0, Error::kDomainTypeApplication);
            loop = false;
            break;
        }
        chunkLength = bx::toBigEndian(chunkLength);
        if (chunkLength > Inline::saturateInt32U(reader->size())) {
            StringUtils::format(
                message, sizeof(message), "Too large chunk to read (%u > %lu)", chunkLength, reader->size());
            error = Error(message, 0, Error::kDomainTypeApplication);
            loop = false;
            break;
        }
        if (FileUtils::readTyped(reader, chunkType, error) != Inline::saturateInt32(sizeof(chunkType))) {
            error = Error("APNG: Corrupted chunk type", 0, Error::kDomainTypeApplication);
            loop = false;
            break;
        }
        switch (chunkType) {
        case kPNGChunkTypeAnimationControl: {
            checksum = decodeAnimationControl(reader, state, error);
            hasAnimationControl = true;
            break;
        }
        case kPNGChunkTypeFrameControl: {
            checksum = decodeFrameControl(reader, state, error);
            break;
        }
        case kPNGChunkTypeFrameData: {
            checksum = decodeFrameData(reader, chunkLength, state, error);
            break;
        }
        case kPNGChunkTypeImageData: {
            checksum = decodeImageData(reader, chunkLength, state, error);
            hasImageData = true;
            break;
        }
        case kPNGChunkTypeImageEnd: {
            checksum = decodeImageEnd(state, error);
            loop = false;
            break;
        }
        case kPNGChunkTypeImageHeader: {
            checksum = decodeImageHeader(reader, state, error);
            break;
        }
        default:
            /* skip and ignore the unknown chunk */
            state.m_dataChunk.resize(chunkLength);
            if (reader->read(state.m_dataChunk.data(), chunkLength, error) != Inline::saturateInt32(chunkLength)) {
                loop = false;
                break;
            }
            checksum = state.m_crc.checksum(chunkType, state.m_dataChunk.data(), chunkLength);
            break;
        }
        if (checksum == 0) {
            loop = false;
            break;
        }
        FileUtils::readTyped(reader, chunkCRC, error);
        if (chunkCRC != checksum) {
            StringUtils::format(message, sizeof(message),
                "APNG: CRC chunk checksum (%c%c%c%c: 0x%x != 0x%x) not matched", chunkType & 0xff,
                chunkType >> 8 & 0xff, chunkType >> 16 & 0xff, chunkType >> 24 & 0xff, chunkCRC, checksum);
            error = Error(message, 0, Error::kDomainTypeApplication);
            loop = false;
            break;
        }
    }
    const nanoem_u32_t numFrames = Inline::saturateInt32U(m_frames.size());
    if (!error.hasReason() && hasAnimationControl) {
        if (!hasImageData && m_control.m_numFrames == 0) {
            error = Error("APNG: Empty animation control", 0, Error::kDomainTypeApplication);
        }
        else if (m_control.m_numFrames != numFrames) {
            StringUtils::format(message, sizeof(message), "APNG: Frame count (%u != %u) doesn't match",
                m_control.m_numFrames, numFrames);
            error = Error(message, 0, Error::kDomainTypeApplication);
        }
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
    for (APNG::FrameSequenceList::const_iterator it = m_frames.begin(), end = m_frames.end(); it != end; ++it) {
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
            FileUtils::writeTyped(&writer, crc.checksum(kPNGChunkTypeImageData, bytes.data(), bytes.size()), error);
            FileUtils::writeTyped(&writer, 0, error);
            FileUtils::writeTyped(&writer, kPNGChunkTypeImageEnd, error);
            FileUtils::writeTyped(&writer, crc.checksum(kPNGChunkTypeImageEnd, nullptr, 0), error);
            Error err;
            sg_image_desc desc;
            nanoem_u8_t *decodedImageDataPtr = nullptr;
            Inline::clearZeroMemory(desc);
            if (ImageLoader::decodeImageWithSTB(buffer.data(), buffer.size(), "", desc, &decodedImageDataPtr, err)) {
                desc.mag_filter = desc.min_filter = SG_FILTER_LINEAR;
                if (frameControl.m_blendOp == APNG::kBlendOpSource) {
                    const nanoem_u32_t *imageDataPtr = reinterpret_cast<const nanoem_u32_t *>(decodedImageDataPtr);
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
                    const nanoem_u8_t *imageDataPtr = static_cast<const nanoem_u8_t *>(decodedImageDataPtr);
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
                ImageLoader::releaseDecodedImageWithSTB(&decodedImageDataPtr);
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

nanoem_u32_t
image::APNG::decodeAnimationControl(ISeekableReader *reader, State &state, Error &error)
{
    if (FileUtils::readTyped(reader, m_control, error) != Inline::saturateInt32(sizeof(m_control))) {
        return 0;
    }
    nanoem_u32_t checksum = state.m_crc.checksumTyped(kPNGChunkTypeAnimationControl, m_control);
    m_control.m_numFrames = bx::toBigEndian(m_control.m_numFrames);
    m_control.m_numPlayCount = bx::toBigEndian(m_control.m_numPlayCount);
    if (m_control.m_numFrames >= 0x80000000u) {
        m_control.m_numFrames -= 0x80000000u;
    }
    return checksum;
}

nanoem_u32_t
image::APNG::decodeFrameControl(ISeekableReader *reader, State &state, Error &error)
{
    char message[Error::kMaxReasonLength];
    if (state.m_frame) {
        if (state.m_frame->m_sequences.empty()) {
            error = Error("APNG: Empty image sequence", 0, Error::kDomainTypeApplication);
            return 0;
        }
        else {
            m_frames.push_back(state.m_frame);
            state.m_frame = nullptr;
        }
    }
    state.m_frame = nanoem_new(APNG::Frame);
    APNG::FrameControl &frameControl = state.m_frame->m_control;
    if (FileUtils::readTyped(reader, frameControl, error) != Inline::saturateInt32(sizeof(frameControl))) {
        error = Error("APNG: Corrupted frame control", 0, Error::kDomainTypeApplication);
        nanoem_delete_safe(state.m_frame);
        return 0;
    }
    nanoem_u32_t checksum = state.m_crc.checksumTyped(kPNGChunkTypeFrameControl, frameControl);
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
    if (state.m_expectedSequenceNumber != frameControl.m_sequenceNumber) {
        StringUtils::format(message, sizeof(message), "APNG: Incorrect sequence number (%u != %u) doesn't match",
            state.m_expectedSequenceNumber, frameControl.m_sequenceNumber);
        error = Error(message, 0, Error::kDomainTypeApplication);
        nanoem_delete_safe(state.m_frame);
        return 0;
    }
    else if (frameControl.m_width == 0 || frameControl.m_height == 0 ||
        !(frameControl.m_xoffset + frameControl.m_width <= m_header.m_width) ||
        !(frameControl.m_yoffset + frameControl.m_height <= m_header.m_height)) {
        error = Error("APNG: Invalid frame rectangle", 0, Error::kDomainTypeApplication);
        nanoem_delete_safe(state.m_frame);
        return 0;
    }
    state.m_frame->m_seconds = state.m_seconds;
    state.m_seconds += frameControl.m_delayNum / nanoem_f32_t(frameControl.m_delayDen);
    state.m_expectedSequenceNumber++;
    return checksum;
}

nanoem_u32_t
image::APNG::decodeFrameData(ISeekableReader *reader, nanoem_u32_t chunkLength, State &state, Error &error)
{
    char message[Error::kMaxReasonLength];
    state.m_dataChunk.clear();
    nanoem_u32_t sequenceNumber, checksum = 0;
    if (chunkLength >= sizeof(sequenceNumber)) {
        state.m_dataChunk.resize(chunkLength);
        if (FileUtils::read(reader, state.m_dataChunk.data(), chunkLength, error) !=
            Inline::saturateInt32(chunkLength)) {
            error = Error("APNG: Corrupted frame data", 0, Error::kDomainTypeApplication);
            return 0;
        }
        checksum = state.m_crc.checksum(kPNGChunkTypeFrameData, state.m_dataChunk.data(), chunkLength);
        sequenceNumber = bx::toBigEndian(*reinterpret_cast<const nanoem_u32_t *>(state.m_dataChunk.data()));
        if (state.m_expectedSequenceNumber != sequenceNumber) {
            StringUtils::format(message, sizeof(message), "APNG: Incorrect sequence number (%u != %u) doesn't match",
                state.m_expectedSequenceNumber, sequenceNumber);
            error = Error(message, 0, Error::kDomainTypeApplication);
        }
        else if (state.m_frame) {
            state.m_dataChunk.erase(state.m_dataChunk.begin(), state.m_dataChunk.begin() + sizeof(sequenceNumber));
            state.m_frame->m_sequences.push_back(state.m_dataChunk);
        }
    }
    state.m_expectedSequenceNumber++;
    return checksum;
}

nanoem_u32_t
image::APNG::decodeImageData(ISeekableReader *reader, nanoem_u32_t chunkLength, State &state, Error &error)
{
    state.m_dataChunk.resize(chunkLength);
    if (FileUtils::read(reader, state.m_dataChunk.data(), chunkLength, error) != Inline::saturateInt32(chunkLength)) {
        error = Error("APNG: Corrupted image data", 0, Error::kDomainTypeApplication);
        return 0;
    }
    nanoem_u32_t checksum = state.m_crc.checksum(kPNGChunkTypeImageData, state.m_dataChunk.data(), chunkLength);
    if (state.m_frame) {
        state.m_frame->m_sequences.push_back(state.m_dataChunk);
    }
    return checksum;
}

nanoem_u32_t
image::APNG::decodeImageEnd(State &state, Error &error)
{
    if (state.m_frame) {
        if (state.m_frame->m_sequences.empty()) {
            error = Error("APNG: Empty image sequence", 0, Error::kDomainTypeApplication);
            return 0;
        }
        else {
            m_frames.push_back(state.m_frame);
            state.m_frame = nullptr;
        }
    }
    return state.m_crc.checksum(kPNGChunkTypeImageEnd, nullptr, 0);
}

nanoem_u32_t
image::APNG::decodeImageHeader(ISeekableReader *reader, State &state, Error &error)
{
    if (FileUtils::readTyped(reader, m_header, error) != Inline::saturateInt32(sizeof(m_header))) {
        error = Error("APNG: Corrupted image header", 0, Error::kDomainTypeApplication);
        return 0;
    }
    nanoem_u32_t checksum = state.m_crc.checksumTyped(kPNGChunkTypeImageHeader, m_header);
    m_header.m_width = bx::toBigEndian(m_header.m_width);
    m_header.m_height = bx::toBigEndian(m_header.m_height);
    if (m_header.m_width == 0 || m_header.m_height == 0) {
        error = Error("APNG: Empty width or height found", 0, Error::kDomainTypeApplication);
        return 0;
    }
    if (m_header.m_width > kPNGImageSizeHardLimit || m_header.m_height > kPNGImageSizeHardLimit) {
        error = Error("APNG: Exceeded hard limit size of image", 0, Error::kDomainTypeApplication);
        return 0;
    }
    return checksum;
}

const nanoem_u32_t image::DDS::kSignature = nanoem_fourcc('D', 'D', 'S', ' ');

image::DDS::DDS()
    : m_format(SG_PIXELFORMAT_NONE)
{
    Inline::clearZeroMemory(m_header);
    Inline::clearZeroMemory(m_extendedHeader);
}

image::DDS::~DDS() NANOEM_DECL_NOEXCEPT
{
}

bool
image::DDS::decode(IReader *reader, Error &error)
{
    if (FileUtils::readTyped(reader, m_header, error) != Inline::saturateInt32(sizeof(m_header))) {
        return false;
    }
    if (m_header.m_size != Inline::saturateInt32U(sizeof(m_header)) ||
        m_header.m_pixelFormat.m_size != Inline::saturateInt32U(sizeof(m_header.m_pixelFormat))) {
        error = Error("DDS: Unmatched header or format size", 0, Error::kDomainTypeApplication);
        return false;
    }
    if (m_header.m_width == 0 || m_header.m_height == 0) {
        error = Error("DDS: Empty width or height found", 0, Error::kDomainTypeApplication);
        return false;
    }
    if (m_header.m_width > kDDSImageSizeHardLimit || m_header.m_height > kDDSImageSizeHardLimit) {
        error = Error("DDS: Exceeded hard limit size of image", 0, Error::kDomainTypeApplication);
        return false;
    }
    if (m_header.m_depth > kDDSImageDepthHardLimit) {
        error = Error("DDS: Exceeded hard limit depth of image", 0, Error::kDomainTypeApplication);
        return false;
    }
    if (EnumUtils::isEnabled(kDDSFormatFlagTypeFourCC, m_header.m_pixelFormat.m_flags) &&
        m_header.m_pixelFormat.m_fourCC == nanoem_fourcc('D', 'X', '1', '0')) {
        if (FileUtils::readTyped(reader, m_extendedHeader, error) != Inline::saturateInt32(sizeof(m_extendedHeader))) {
            return false;
        }
    }
    const sg_pixel_format format = findPixelFormat(m_header.m_pixelFormat, m_extendedHeader);
    if (format != SG_PIXELFORMAT_NONE) {
        m_format = format;
    }
    else {
        const PixelFormat &pf = m_header.m_pixelFormat;
        char message[Error::kMaxReasonLength];
        StringUtils::format(message, sizeof(message),
            "DDS: Unsupported texture format: DXGI=%d colorBits=%u fourCC=%c%c%c%c:%u", m_extendedHeader.m_dxgiFormat,
            pf.m_colorBitCount, pf.m_fourCC & 0xff, (pf.m_fourCC >> 8) & 0xff, (pf.m_fourCC >> 16) & 0xff,
            (pf.m_fourCC >> 24) & 0xff, pf.m_fourCC);
        error = Error(message, 0, Error::kDomainTypeApplication);
        return false;
    }
    const nanoem_u32_t numMipmaps = EnumUtils::isEnabled(kDDSHeaderFlagTypeMipmap, m_header.m_flags)
        ? glm::clamp(m_header.m_mipmapCount, 1u, nanoem_u32_t(SG_MAX_MIPMAPS))
        : 1u,
                       numDepths = glm::clamp(m_header.m_depth, 1u, nanoem_u32_t(SG_MAX_TEXTUREARRAY_LAYERS));
    nanoem_rsize_t numBytes = 0;
    nanoem_u32_t mipmapWidth, mipmapHeight;
    m_header.m_mipmapCount = numMipmaps;
    m_header.m_depth = numDepths;
    switch (type()) {
    case SG_IMAGETYPE_CUBE: {
        for (nanoem_rsize_t i = 0; i < SG_CUBEFACE_NUM; i++) {
            mipmapWidth = width();
            mipmapHeight = height();
            for (nanoem_rsize_t j = 0; j < numMipmaps; j++) {
                ByteArray &image = m_images[i][j];
                getImageSize(mipmapWidth, mipmapHeight, numBytes);
                if (!decodeImage(reader, i, j, mipmapWidth, mipmapHeight, numBytes, image, error)) {
                    i = SG_MAX_MIPMAPS;
                    break;
                }
                mipmapWidth = glm::max(mipmapWidth >> 1, 1u);
                mipmapHeight = glm::max(mipmapHeight >> 1, 1u);
            }
        }
        break;
    }
    case SG_IMAGETYPE_3D:
    case SG_IMAGETYPE_2D: {
        mipmapWidth = width();
        mipmapHeight = height();
        for (nanoem_rsize_t i = 0; i < numMipmaps; i++) {
            ByteArray &image = m_images[0][i];
            getImageSize(mipmapWidth, mipmapHeight, numBytes);
            if (!decodeImage(reader, 0, i, mipmapWidth, mipmapHeight, numBytes, image, error)) {
                break;
            }
            mipmapWidth = glm::max(mipmapWidth >> 1, 1u);
            mipmapHeight = glm::max(mipmapHeight >> 1, 1u);
        }
        break;
    }
    default:
        break;
    }
    return !error.hasReason();
}

void
image::DDS::setImageDescription(sg_image_desc &desc) const NANOEM_DECL_NOEXCEPT
{
    desc.width = width();
    desc.height = height();
    desc.num_slices = depth();
    desc.num_mipmaps = mipmapCount();
    desc.pixel_format = format();
    desc.type = type();
    for (nanoem_rsize_t i = 0; i < SG_CUBEFACE_NUM; i++) {
        for (nanoem_rsize_t j = 0; j < SG_MAX_MIPMAPS; j++) {
            const ByteArray *bytes = imageData(i, j);
            if (!bytes->empty()) {
                sg_range &content = desc.data.subimage[i][j];
                content.ptr = bytes->data();
                content.size = bytes->size();
            }
        }
    }
}

const ByteArray *
image::DDS::imageData(nanoem_rsize_t face, nanoem_rsize_t index) const NANOEM_DECL_NOEXCEPT
{
    const ByteArray *data = nullptr;
    if (face < SG_CUBEFACE_NUM && index < SG_MAX_MIPMAPS) {
        data = &m_images[face][index];
    }
    return data;
}

sg_pixel_format
image::DDS::format() const NANOEM_DECL_NOEXCEPT
{
    return m_format;
}

sg_image_type
image::DDS::type() const NANOEM_DECL_NOEXCEPT
{
    sg_image_type type = SG_IMAGETYPE_2D;
    if (m_extendedHeader.m_resourceDimension != 0) {
        if (m_extendedHeader.m_resourceDimension == D3D11_RESOURCE_DIMENSION_TEXTURE3D) {
            type = SG_IMAGETYPE_3D;
        }
        else if (m_extendedHeader.m_resourceDimension == D3D11_RESOURCE_DIMENSION_TEXTURE2D &&
            EnumUtils::isEnabled(D3D11_RESOURCE_MISC_TEXTURECUBE, m_extendedHeader.m_miscFlags)) {
            type = SG_IMAGETYPE_CUBE;
        }
    }
    else if (EnumUtils::isEnabled(kDDSHeaderFlagTypeVolume, m_header.m_flags)) {
        type = SG_IMAGETYPE_3D;
    }
    else if (EnumUtils::isEnabled(kDDSCapsCubeMapAllFaces, m_header.m_caps[1])) {
        type = SG_IMAGETYPE_CUBE;
    }
    return type;
}

nanoem_u32_t
image::DDS::width() const NANOEM_DECL_NOEXCEPT
{
    return m_header.m_width;
}

nanoem_u32_t
image::DDS::height() const NANOEM_DECL_NOEXCEPT
{
    return m_header.m_height;
}

nanoem_u32_t
image::DDS::depth() const NANOEM_DECL_NOEXCEPT
{
    return m_header.m_depth;
}

nanoem_u32_t
image::DDS::mipmapCount() const NANOEM_DECL_NOEXCEPT
{
    return m_header.m_mipmapCount;
}

bool
image::DDS::PixelFormat::testMask(
    nanoem_u32_t r, nanoem_u32_t g, nanoem_u32_t b, nanoem_u32_t a) const NANOEM_DECL_NOEXCEPT
{
    return m_redBitMask == r && m_greenBitMask == g && m_blueBitMask == b && m_alphaBitMask == a;
}

sg_pixel_format
image::DDS::findPixelFormat(const PixelFormat &value, const ExtendedHeader &extend) NANOEM_DECL_NOEXCEPT
{
    sg_pixel_format format = SG_PIXELFORMAT_NONE;
    if (extend.m_dxgiFormat != 0) {
        switch (extend.m_dxgiFormat) {
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: {
            format = SG_PIXELFORMAT_RGBA8;
            break;
        }
        case DXGI_FORMAT_R8G8B8A8_SNORM: {
            format = SG_PIXELFORMAT_RGBA8SN;
            break;
        }
        case DXGI_FORMAT_B8G8R8A8_UNORM:
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: {
            format = SG_PIXELFORMAT_BGRA8;
            break;
        }
        case DXGI_FORMAT_R10G10B10A2_UNORM: {
            format = SG_PIXELFORMAT_RGB10A2;
            break;
        }
        case DXGI_FORMAT_R32G32B32A32_FLOAT: {
            format = SG_PIXELFORMAT_RGBA32F;
            break;
        }
        case DXGI_FORMAT_R16G16B16A16_FLOAT: {
            format = SG_PIXELFORMAT_RGBA16F;
            break;
        }
        case DXGI_FORMAT_R16G16B16A16_UNORM: {
            format = SG_PIXELFORMAT_RGBA16;
            break;
        }
        case DXGI_FORMAT_R16G16B16A16_SNORM: {
            format = SG_PIXELFORMAT_RGBA16SN;
            break;
        }
        case DXGI_FORMAT_R32G32_FLOAT: {
            format = SG_PIXELFORMAT_RG32F;
            break;
        }
        case DXGI_FORMAT_R16G16_FLOAT: {
            format = SG_PIXELFORMAT_RG16F;
            break;
        }
        case DXGI_FORMAT_R16G16_UNORM: {
            format = SG_PIXELFORMAT_RG16;
            break;
        }
        case DXGI_FORMAT_R16G16_SNORM: {
            format = SG_PIXELFORMAT_RG16SN;
            break;
        }
        case DXGI_FORMAT_R8G8_UNORM: {
            format = SG_PIXELFORMAT_RG8;
            break;
        }
        case DXGI_FORMAT_R8G8_SNORM: {
            format = SG_PIXELFORMAT_RG8SN;
            break;
        }
        case DXGI_FORMAT_R32_FLOAT: {
            format = SG_PIXELFORMAT_R32F;
            break;
        }
        case DXGI_FORMAT_R16_FLOAT: {
            format = SG_PIXELFORMAT_R16F;
            break;
        }
        case DXGI_FORMAT_R8_UNORM:
        case DXGI_FORMAT_A8_UNORM: {
            format = SG_PIXELFORMAT_R8;
            break;
        }
        case DXGI_FORMAT_R8_SNORM: {
            format = SG_PIXELFORMAT_R8SN;
            break;
        }
        case DXGI_FORMAT_BC1_UNORM:
        case DXGI_FORMAT_BC1_UNORM_SRGB: {
            format = SG_PIXELFORMAT_BC1_RGBA;
            break;
        }
        case DXGI_FORMAT_BC2_UNORM:
        case DXGI_FORMAT_BC2_UNORM_SRGB: {
            format = SG_PIXELFORMAT_BC2_RGBA;
            break;
        }
        case DXGI_FORMAT_BC3_UNORM:
        case DXGI_FORMAT_BC3_UNORM_SRGB: {
            format = SG_PIXELFORMAT_BC3_RGBA;
            break;
        }
        case DXGI_FORMAT_BC4_UNORM: {
            format = SG_PIXELFORMAT_BC4_R;
            break;
        }
        case DXGI_FORMAT_BC4_SNORM: {
            format = SG_PIXELFORMAT_BC4_RSN;
            break;
        }
        case DXGI_FORMAT_BC5_UNORM: {
            format = SG_PIXELFORMAT_BC5_RG;
            break;
        }
        case DXGI_FORMAT_BC5_SNORM: {
            format = SG_PIXELFORMAT_BC5_RGSN;
            break;
        }
        default:
            break;
        }
    }
    else if (EnumUtils::isEnabled(kDDSFormatFlagTypeRGB, value.m_flags)) {
        switch (value.m_colorBitCount) {
        case 32: {
            if (value.testMask(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000)) {
                format = SG_PIXELFORMAT_RGBA8;
            }
            else if (value.testMask(0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000) ||
                value.testMask(0x00ff0000, 0x0000ff00, 0x000000ff, 0)) {
                format = SG_PIXELFORMAT_BGRA8;
            }
            else if (value.testMask(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000)) {
                format = SG_PIXELFORMAT_RGB10A2;
            }
            else if (value.testMask(0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000) ||
                value.testMask(0x00ff0000, 0x0000ff00, 0x000000ff, 0)) {
                format = SG_PIXELFORMAT_RG16;
            }
            else if (value.testMask(0xffffffff, 0, 0, 0)) {
                format = SG_PIXELFORMAT_R32F;
            }
            break;
        }
        case 24: {
            format = SG_PIXELFORMAT_RGBA8;
            break;
        }
        case 16: {
            if (value.testMask(0xff00, 0xff, 0, 0)) {
                format = SG_PIXELFORMAT_RGBA8;
            }
            else if (value.testMask(0xffff, 0, 0, 0)) {
                format = SG_PIXELFORMAT_R16;
            }
            break;
        }
        case 8: {
            if (value.testMask(0xff, 0, 0, 0)) {
                format = SG_PIXELFORMAT_R8;
            }
            break;
        }
        default:
            break;
        }
    }
    else if (EnumUtils::isEnabled(kDDSFormatFlagTypeLuminance, value.m_flags)) {
        switch (value.m_colorBitCount) {
        case 16: {
            if (value.testMask(0xffff, 0, 0, 0)) {
                format = SG_PIXELFORMAT_R16;
            }
            break;
        }
        case 8: {
            if (value.testMask(0xff, 0, 0, 0)) {
                format = SG_PIXELFORMAT_R8;
            }
            break;
        }
        default:
            break;
        }
    }
    else if (EnumUtils::isEnabled(kDDSFormatFlagTypeFourCC, value.m_flags)) {
        switch (value.m_fourCC) {
        case nanoem_fourcc('D', 'X', 'T', '1'): {
            format = SG_PIXELFORMAT_BC1_RGBA;
            break;
        }
        case nanoem_fourcc('D', 'X', 'T', '2'):
        case nanoem_fourcc('D', 'X', 'T', '3'): {
            format = SG_PIXELFORMAT_BC2_RGBA;
            break;
        }
        case nanoem_fourcc('D', 'X', 'T', '4'):
        case nanoem_fourcc('D', 'X', 'T', '5'): {
            format = SG_PIXELFORMAT_BC3_RGBA;
            break;
        }
        case nanoem_fourcc('B', 'C', '4', 'U'): {
            format = SG_PIXELFORMAT_BC4_R;
            break;
        }
        case nanoem_fourcc('B', 'C', '4', 'S'): {
            format = SG_PIXELFORMAT_BC4_RSN;
            break;
        }
        case nanoem_fourcc('B', 'C', '5', 'U'): {
            format = SG_PIXELFORMAT_BC5_RG;
            break;
        }
        case nanoem_fourcc('B', 'C', '5', 'S'): {
            format = SG_PIXELFORMAT_BC5_RGSN;
            break;
        }
        case 36: {
            format = SG_PIXELFORMAT_RGBA16;
            break;
        }
        case 111: {
            format = SG_PIXELFORMAT_R16F;
            break;
        }
        case 112: {
            format = SG_PIXELFORMAT_RG16F;
            break;
        }
        case 113: {
            format = SG_PIXELFORMAT_RGBA16F;
            break;
        }
        case 114: {
            format = SG_PIXELFORMAT_R32F;
            break;
        }
        case 115: {
            format = SG_PIXELFORMAT_RG32F;
            break;
        }
        case 116: {
            format = SG_PIXELFORMAT_RGBA32F;
            break;
        }
        default:
            break;
        }
    }
    return format;
}

void
image::DDS::getBCImageSize(
    nanoem_u32_t width, nanoem_u32_t height, nanoem_rsize_t blockSize, nanoem_rsize_t &numBytes) NANOEM_DECL_NOEXCEPT
{
    static const nanoem_rsize_t kMinimumBlockSize = 1u;
    nanoem_rsize_t numBlocksWidth = 0, numBlocksHeight = 0;
    if (width > 0) {
        numBlocksWidth = glm::max(kMinimumBlockSize, (nanoem_rsize_t(width) + 3u) / 4u);
    }
    if (height > 0) {
        numBlocksHeight = glm::max(kMinimumBlockSize, (nanoem_rsize_t(height) + 3u) / 4u);
    }
    const nanoem_rsize_t rowBytes = numBlocksWidth * blockSize, numRows = numBlocksHeight;
    numBytes = rowBytes * numRows;
}

void
image::DDS::getRawImageSize(
    nanoem_u32_t width, nanoem_u32_t height, nanoem_rsize_t pixelSize, nanoem_rsize_t &numBytes) NANOEM_DECL_NOEXCEPT
{
    const nanoem_rsize_t rowBytes = width * pixelSize, numRows = height;
    numBytes = rowBytes * numRows;
}

void
image::DDS::getImageSize(nanoem_u32_t width, nanoem_u32_t height, nanoem_rsize_t &numBytes) const NANOEM_DECL_NOEXCEPT
{
    switch (m_format) {
    case SG_PIXELFORMAT_RGBA8:
    case SG_PIXELFORMAT_BGRA8:
    case SG_PIXELFORMAT_RGBA8SN: {
        if (m_header.m_pixelFormat.m_colorBitCount == 24) {
            getRawImageSize(width, height, 3, numBytes);
        }
        else {
            getRawImageSize(width, height, 4, numBytes);
        }
        break;
    }
    case SG_PIXELFORMAT_R16:
    case SG_PIXELFORMAT_R16SN:
    case SG_PIXELFORMAT_RG8:
    case SG_PIXELFORMAT_RG8SN: {
        getRawImageSize(width, height, 2, numBytes);
        break;
    }
    case SG_PIXELFORMAT_R8:
    case SG_PIXELFORMAT_R8SN: {
        getRawImageSize(width, height, 1, numBytes);
        break;
    }
    case SG_PIXELFORMAT_BC1_RGBA: {
        getBCImageSize(width, height, 8, numBytes);
        break;
    }
    case SG_PIXELFORMAT_BC2_RGBA: {
        getBCImageSize(width, height, 16, numBytes);
        break;
    }
    case SG_PIXELFORMAT_BC3_RGBA: {
        getBCImageSize(width, height, 8, numBytes);
        break;
    }
    case SG_PIXELFORMAT_BC4_R: {
        getBCImageSize(width, height, 8, numBytes);
        break;
    }
    case SG_PIXELFORMAT_BC4_RSN: {
        getBCImageSize(width, height, 8, numBytes);
        break;
    }
    case SG_PIXELFORMAT_BC5_RG: {
        getBCImageSize(width, height, 16, numBytes);
        break;
    }
    case SG_PIXELFORMAT_BC5_RGSN: {
        getBCImageSize(width, height, 16, numBytes);
        break;
    }
    case SG_PIXELFORMAT_RGBA32F: {
        getRawImageSize(width, height, 16, numBytes);
        break;
    }
    case SG_PIXELFORMAT_RGBA16:
    case SG_PIXELFORMAT_RGBA16SN: {
        getRawImageSize(width, height, 8, numBytes);
        break;
    }
    case SG_PIXELFORMAT_RG32F:
    case SG_PIXELFORMAT_RGBA16F: {
        getRawImageSize(width, height, 8, numBytes);
        break;
    }
    case SG_PIXELFORMAT_R32F:
    case SG_PIXELFORMAT_RG16F:
    case SG_PIXELFORMAT_RGB10A2: {
        getRawImageSize(width, height, 4, numBytes);
        break;
    }
    case SG_PIXELFORMAT_R16F: {
        getRawImageSize(width, height, 2, numBytes);
        break;
    }
    default:
        break;
    }
}

bool
image::DDS::decodeImage(IReader *reader, nanoem_u32_t faceIndex, nanoem_u32_t mipmapIndex, nanoem_u32_t width,
    nanoem_u32_t height, nanoem_rsize_t numBytes, ByteArray &bytes, Error &error)
{
    char message[Error::kMaxReasonLength];
    if (numBytes <= reader->size()) {
        const int intNumBytes = Inline::saturateInt32(numBytes);
        bytes.resize(numBytes);
        int readSize = reader->read(bytes.data(), intNumBytes, error);
        if (readSize != intNumBytes) {
            StringUtils::format(message, sizeof(message),
                "DDS: Unexpected data EOF (faceIndex=%u, mipmapIndex=%u, width=%u, height=%u, type=%d, format=%u, "
                "expected=%d, actual=%d)",
                faceIndex, mipmapIndex, width, height, type(), m_format, intNumBytes, readSize);
            error = Error(message, 0, Error::kDomainTypeApplication);
        }
        if (m_format == SG_PIXELFORMAT_RGBA8 && m_header.m_pixelFormat.m_colorBitCount == 24) {
            const nanoem_rsize_t numPixels = width * height;
            ByteArray newPixels(numPixels * 4);
            for (nanoem_rsize_t i = 0; i < numPixels; i++) {
                const nanoem_u8_t *source = bytes.data() + i * 3;
                nanoem_u8_t *dest = newPixels.data() + i * 4;
                dest[0] = source[0];
                dest[1] = source[1];
                dest[2] = source[2];
                dest[3] = 0xff;
            }
            bytes = newPixels;
        }
    }
    else {
        StringUtils::format(
            message, sizeof(message), "DDS: Too large image data to read (%lu < %lu)", numBytes, reader->size());
        error = Error(message, 0, Error::kDomainTypeApplication);
    }
    return !error.hasReason();
}

image::PFM::PFM()
    : m_format(SG_PIXELFORMAT_NONE)
    , m_width(0)
    , m_height(0)
{
}

image::PFM::~PFM() NANOEM_DECL_NOEXCEPT
{
}

bool
image::PFM::decode(const ByteArray &bytes, Error &error)
{
    char header[128], message[Error::kMaxReasonLength];
    nanoem_rsize_t length = glm::min(bytes.size(), sizeof(header) - 1);
    nanoem_u32_t width, height;
    nanoem_f32_t byteOrderIndicator;
    int offset;
    ::memcpy(header, bytes.data(), length);
    header[length] = 0;
    if (::sscanf(header, "PF\n%u %u\n%f\n%n", &width, &height, &byteOrderIndicator, &offset) == 3 && width > 0 &&
        height > 0 && bytes.size() >= Inline::roundInt32(offset)) {
        const nanoem_rsize_t channelSize = static_cast<nanoem_rsize_t>(width) * height * sizeof(nanoem_f32_t),
                             actualDataSize = bytes.size() - offset;
        if (channelSize * 3 == actualDataSize) {
            m_image.resize(channelSize * 4);
            const nanoem_f32_t *source = reinterpret_cast<const nanoem_f32_t *>(bytes.data() + offset);
            nanoem_f32_t *dest = reinterpret_cast<nanoem_f32_t *>(m_image.data());
            for (size_t i = 0, size = channelSize / sizeof(nanoem_f32_t); i < size; i++) {
                const nanoem_rsize_t sourceOffset = i * 3, destOffset = i * 4;
                dest[destOffset + 0] = source[sourceOffset + 0];
                dest[destOffset + 1] = source[sourceOffset + 1];
                dest[destOffset + 2] = source[sourceOffset + 2];
                dest[destOffset + 3] = 1.0f;
            }
            m_format = SG_PIXELFORMAT_RGBA32F;
            m_width = width;
            m_height = height;
        }
        else {
            StringUtils::format(message, sizeof(message), "PFM: Invalid size of RGB32F image data (%ux%u: %lu != %lu)",
                width, height, channelSize * 3, actualDataSize);
            error = Error(message, 0, Error::kDomainTypeApplication);
        }
    }
    else if (::sscanf(header, "Pf\n%u %u\n%f\n%n", &width, &height, &byteOrderIndicator, &offset) == 3 && width > 0 &&
        height > 0 && bytes.size() >= Inline::roundInt32(offset)) {
        const nanoem_rsize_t channelSize = static_cast<nanoem_rsize_t>(width) * height * sizeof(nanoem_f32_t),
                             actualDataSize = bytes.size() - offset;
        if (channelSize == actualDataSize) {
            const nanoem_u8_t *dataPtr = bytes.data() + offset;
            m_image.assign(dataPtr, dataPtr + actualDataSize);
            m_format = SG_PIXELFORMAT_R32F;
            m_width = width;
            m_height = height;
        }
        else {
            StringUtils::format(message, sizeof(message), "PFM: Invalid size of R32F image data (%ux%u: %lu != %lu)",
                width, height, channelSize, actualDataSize);
            error = Error(message, 0, Error::kDomainTypeApplication);
        }
    }
    else {
        StringUtils::format(
            message, sizeof(message), "PFM: Invalid header data (%ux%u: offset=%u)", width, height, offset);
        error = Error(message, 0, Error::kDomainTypeApplication);
    }
    return !error.hasReason();
}

void
image::PFM::setImageDescription(sg_image_desc &desc) const NANOEM_DECL_NOEXCEPT
{
    desc.type = SG_IMAGETYPE_2D;
    desc.width = width();
    desc.height = height();
    desc.pixel_format = format();
    sg_range &content = desc.data.subimage[0][0];
    content.ptr = m_image.data();
    content.size = m_image.size();
}

sg_pixel_format
image::PFM::format() const NANOEM_DECL_NOEXCEPT
{
    return m_format;
}

nanoem_u32_t
image::PFM::width() const NANOEM_DECL_NOEXCEPT
{
    return m_width;
}

nanoem_u32_t
image::PFM::height() const NANOEM_DECL_NOEXCEPT
{
    return m_height;
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

image::DDS *
ImageLoader::decodeDDS(IReader *reader, Error &error)
{
    nanoem_u32_t signature;
    FileUtils::readTyped(reader, signature, error);
    image::DDS *image = nullptr;
    if (signature == image::DDS::kSignature) {
        image = nanoem_new(image::DDS);
        if (!image->decode(reader, error)) {
            nanoem_delete_safe(image);
        }
    }
    return image;
}

image::PFM *
ImageLoader::decodePFM(const ByteArray &bytes, Error &error)
{
    image::PFM *image = nullptr;
    image = nanoem_new(image::PFM);
    if (!image->decode(bytes, error)) {
        nanoem_delete_safe(image);
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
ImageLoader::validateImageSize(const sg_image_desc &desc, const char *name, Error &error)
{
    const int width = Inline::roundInt32(desc.width), height = Inline::roundInt32(desc.height),
              depth = Inline::roundInt32(desc.num_slices);
    char message[Error::kMaxReasonLength], recoverySuggestion[Error::kMaxRecoverySuggestionLength];
    Error localError;
    sg_limits limits;
    if (sg::query_limits) {
        limits = sg::query_limits();
    }
    else {
        Inline::clearZeroMemory(limits);
        limits.max_image_size_2d = 16384;
        limits.max_image_size_3d = 2048;
        limits.max_image_size_cube = 2048;
    }
    if (desc.type == SG_IMAGETYPE_CUBE && limits.max_image_size_cube > 0 &&
        (width >= limits.max_image_size_cube || height >= limits.max_image_size_cube)) {
        StringUtils::format(message, sizeof(message), "The texture %s exceeds limit (%dx%d) > (%dx%d)", name,
            desc.width, desc.height, limits.max_image_size_cube, limits.max_image_size_cube);
        StringUtils::format(recoverySuggestion, sizeof(recoverySuggestion),
            "Resize the texture %s equal or less than (%dx%d)", name, limits.max_image_size_cube,
            limits.max_image_size_cube);
        localError = Error(message, recoverySuggestion, Error::kDomainTypeApplication);
    }
    else if (desc.type == SG_IMAGETYPE_3D && limits.max_image_size_3d > 0 &&
        (width >= limits.max_image_size_3d || height >= limits.max_image_size_3d ||
            depth >= limits.max_image_size_3d)) {
        StringUtils::format(message, sizeof(message), "The texture %s exceeds limit (%dx%dx%d) > (%dx%dx%d)", name,
            desc.width, desc.height, desc.num_slices, limits.max_image_size_3d, limits.max_image_size_3d,
            limits.max_image_size_3d);
        StringUtils::format(recoverySuggestion, sizeof(recoverySuggestion),
            "Resize the texture %s equal or less than (%dx%dx%d)", name, limits.max_image_size_3d,
            limits.max_image_size_3d, limits.max_image_size_3d);
        localError = Error(message, recoverySuggestion, Error::kDomainTypeApplication);
    }
    else if (limits.max_image_size_2d > 0 &&
        (width >= limits.max_image_size_2d || height >= limits.max_image_size_2d)) {
        StringUtils::format(message, sizeof(message), "The texture %s exceeds limit (%dx%d) > (%dx%d)", name,
            desc.width, desc.height, limits.max_image_size_2d, limits.max_image_size_2d);
        StringUtils::format(recoverySuggestion, sizeof(recoverySuggestion),
            "Resize the texture %s equal or less than (%dx%d)", name, limits.max_image_size_2d,
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
ImageLoader::decodeImageWithSTB(const nanoem_u8_t *dataPtr, const size_t dataSize, const char *name,
    sg_image_desc &desc, nanoem_u8_t **decodedImagePtr, Error &error)
{
    int width, height, components, size = Inline::saturateInt32(dataSize);
    bool result = false;
    if (stbi_info_from_memory(dataPtr, size, &width, &height, &components)) {
        desc.width = width;
        desc.height = height;
        desc.type = SG_IMAGETYPE_2D;
        if (validateImageSize(desc, name, error)) {
            if (stbi_uc *data = stbi_load_from_memory(dataPtr, size, &width, &height, &components, 4)) {
                desc.pixel_format = SG_PIXELFORMAT_RGBA8;
                sg_range &content = desc.data.subimage[0][0];
                content.ptr = data;
                content.size = nanoem_rsize_t(4) * width * height;
                *decodedImagePtr = data;
                result = true;
            }
            else {
                error = Error(stbi_failure_reason(), 0, Error::kDomainTypeApplication);
            }
        }
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
    if (decodeImageWithSTB(
            container.m_dataPtr, container.m_dataSize, container.m_name.c_str(), desc, &decodedImagePtr, error)) {
        desc.mag_filter = desc.min_filter = SG_FILTER_LINEAR;
        desc.max_anisotropy = container.m_anisotropy;
        desc.wrap_u = desc.wrap_v = container.m_wrap;
        imageView = drawable->uploadImage(container.m_name, desc);
        releaseDecodedImageWithSTB(&decodedImagePtr);
    }
    else if (container.m_dataSize >= sizeof(image::DDS::kSignature) &&
        *reinterpret_cast<const nanoem_u32_t *>(container.m_dataPtr) == image::DDS::kSignature) {
        const ByteArray bytes(container.m_dataPtr, container.m_dataPtr + container.m_dataSize);
        MemoryReader reader(&bytes);
        Error innerError;
        if (image::DDS *dds = decodeDDS(&reader, innerError)) {
            dds->setImageDescription(desc);
            desc.mag_filter = desc.min_filter = SG_FILTER_LINEAR;
            desc.max_anisotropy = container.m_anisotropy;
            desc.wrap_u = desc.wrap_v = container.m_wrap;
            imageView = drawable->uploadImage(container.m_name, desc);
            nanoem_delete(dds);
        }
        else if (innerError.hasReason()) {
            error = innerError;
        }
    }
    return imageView;
}

} /* namespace nanoem */
