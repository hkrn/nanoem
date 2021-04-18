/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_IMAGELOADER_H_
#define NANOEM_EMAPP_IMAGELOADER_H_

#include "emapp/IImageView.h"

#include "bimg/bimg.h"

namespace nanoem {

class Error;
class IDrawable;
class IFileReader;
class Project;
class URI;

enum {
    APNG_DISPOSE_OP_NONE = 0,
    APNG_DISPOSE_OP_BACKGROUND,
    APNG_DISPOSE_OP_PREVIOUS,
};
enum {
    APNG_BLEND_OP_SOURCE = 0,
    APNG_BLEND_OP_OVER,
};

struct APNGImage {
#pragma pack(push)
#pragma pack(1)
    struct Header {
        nanoem_u32_t m_width;
        nanoem_u32_t m_height;
        nanoem_u8_t m_depth;
        nanoem_u8_t m_colorType;
        nanoem_u8_t m_compression;
        nanoem_u8_t m_filterType;
        nanoem_u8_t m_interlaceType;
    };
    struct AnimationControl {
        nanoem_u32_t m_numFrames;
        nanoem_u32_t m_numPlayCount;
    };
    struct FrameControl {
        nanoem_u32_t m_sequenceNumber;
        nanoem_u32_t m_width;
        nanoem_u32_t m_height;
        nanoem_u32_t m_xoffset;
        nanoem_u32_t m_yoffset;
        nanoem_u16_t m_delayNum;
        nanoem_u16_t m_delayDen;
        nanoem_u8_t m_disposeOp;
        nanoem_u8_t m_blendOp;
    };
#pragma pack(pop)
    struct Frame {
        FrameControl m_control;
        ByteArrayList m_sequences;
        ByteArray m_composition;
        nanoem_f32_t m_seconds;
    };
    typedef tinystl::vector<Frame *, TinySTLAllocator> FrameSequenceList;

    ~APNGImage();
    nanoem_rsize_t seek(nanoem_f32_t seconds) const;

    Header m_header;
    AnimationControl m_control;
    FrameSequenceList m_frames;
};

class Image NANOEM_DECL_SEALED : public IImageView, private NonCopyable {
public:
    Image();
    ~Image();

    void create();
    void destroy();
    void setOriginData(const nanoem_u8_t *data, nanoem_rsize_t size);
    void setMipmapData(nanoem_rsize_t index, const nanoem_u8_t *data, nanoem_rsize_t size);
    void resizeMipmapData(nanoem_rsize_t value);

    sg_image handle() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setHandle(sg_image value);
    sg_image_desc description() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setDescription(const sg_image_desc &value);
    const ByteArray *originData() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    const ByteArray *mipmapData(nanoem_rsize_t index) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    const char *filenameConstString() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    String filename() const NANOEM_DECL_OVERRIDE;
    void setFilename(const String &value);

private:
    String m_filename;
    sg_image m_handle;
    sg_image_desc m_description;
    ByteArray m_originData;
    ByteArrayList m_mipmapData;
};

class ImageLoader NANOEM_DECL_SEALED : private NonCopyable {
public:
    enum Flags {
        kFlagsFallbackWhiteOpaque = 0x1,
        kFlagsEnableMipmap = 0x2,
        kFlagsEnableFlipY = 0x4,
        kFlagsFallbackBlackOpaque = 0x8,
    };

    static sg_pixel_format resolvePixelFormat(
        const bimg::ImageContainer *container, nanoem_u32_t &bytesPerPixel) NANOEM_DECL_NOEXCEPT;
    static bool generateMipmapImages(
        const bimg::ImageContainer *container, bool flip, ByteArrayList &mipmapPayloads, sg_image_desc &descRef);
    static APNGImage *decodeAnimatedPNG(IFileReader *reader, Error &error);
    static void copyImageDescrption(const sg_image_desc &desc, Image *image);
    static bool validateImageSize(const String &name, const sg_image_desc &desc, Error &error);
    static bool isScreenBMP(const char *path) NANOEM_DECL_NOEXCEPT;
    static void fill1x1PixelImage(const nanoem_u32_t *pixel, sg_image_desc &desc) NANOEM_DECL_NOEXCEPT;
    static void fill1x1WhitePixelImage(sg_image_desc &desc) NANOEM_DECL_NOEXCEPT;
    static void fill1x1BlackPixelImage(sg_image_desc &desc) NANOEM_DECL_NOEXCEPT;
    static void fill1x1TransparentPixelImage(sg_image_desc &desc) NANOEM_DECL_NOEXCEPT;
    static void flipImage(nanoem_u8_t *source, nanoem_u32_t width, nanoem_u32_t height, nanoem_u32_t bpp);

    ImageLoader(const Project *project);
    ~ImageLoader();

    IImageView *load(const URI &fileURI, IDrawable *drawable, sg_wrap wrap, nanoem_u32_t flags, Error &error);
    IImageView *decode(const ByteArray &bytes, const String &filename, IDrawable *drawable, sg_wrap wrap,
        nanoem_u32_t flags, Error &error);

private:
    struct ImmutableImageContainer {
        ImmutableImageContainer(const String &name, const nanoem_u8_t *dataPtr, const size_t dataSize,
            const Vector2UI16 &size, sg_wrap wrap, int anisotropy, nanoem_u32_t flags)
            : m_name(name)
            , m_dataPtr(dataPtr)
            , m_dataSize(dataSize)
            , m_size(size)
            , m_wrap(wrap)
            , m_anisotropy(anisotropy)
            , m_flags(flags)
        {
        }
        ImmutableImageContainer(const String &name, const ByteArray &bytes, const Vector2UI16 &size, sg_wrap wrap,
            int anisotropy, nanoem_u32_t flags)
            : m_name(name)
            , m_dataPtr(bytes.data())
            , m_dataSize(bytes.size())
            , m_size(size)
            , m_wrap(wrap)
            , m_anisotropy(anisotropy)
            , m_flags(flags)
        {
        }
        ImmutableImageContainer(const String &name, const bimg::ImageContainer *containerPtr, sg_wrap wrap,
            int anisotropy, nanoem_u32_t flags)
            : m_name(name)
            , m_dataPtr(static_cast<const nanoem_u8_t *>(containerPtr->m_data))
            , m_dataSize(containerPtr->m_size)
            , m_size(containerPtr->m_width, containerPtr->m_height)
            , m_wrap(wrap)
            , m_anisotropy(anisotropy)
            , m_flags(flags)
        {
        }
        ~ImmutableImageContainer()
        {
        }
        const String m_name;
        const nanoem_u8_t *m_dataPtr;
        const size_t m_dataSize;
        const Vector2UI16 m_size;
        const sg_wrap m_wrap;
        const int m_anisotropy;
        const nanoem_u32_t m_flags;
    };
    static IImageView *decodeImageContainer(
        const ImmutableImageContainer &textureData, IDrawable *drawable, Error &error);
    static void generateMipmapImagesRGBA32F(const bimg::ImageContainer *container, int numMips, bool flip,
        ByteArrayList &mipmapPayloads, sg_image_desc &descRef);
    static void generateMipmapImagesRGBA8(const bimg::ImageContainer *container, int numMips, bool flip,
        ByteArrayList &mipmapPayloads, sg_image_desc &descRef);
    static void ensureRGBA8ImageData(const bimg::ImageContainer *decodedImageContainer, bool needsRGBA8Conversion,
        ByteArray &decodedRGBA8, sg_image_desc &desc);

    const Project *m_project;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_IMAGELOADER_H_ */
