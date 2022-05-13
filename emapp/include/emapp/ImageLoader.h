/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_IMAGELOADER_H_
#define NANOEM_EMAPP_IMAGELOADER_H_

#include "emapp/IImageView.h"

namespace nanoem {

class Error;
class IDrawable;
class IReader;
class ISeekableReader;
class Project;
class URI;

namespace image {

class APNG {
public:
    APNG();
    ~APNG() NANOEM_DECL_NOEXCEPT;

    bool decode(ISeekableReader *reader, Error &error);
    void composite(Error &error);
    nanoem_rsize_t findNearestOffset(nanoem_f32_t seconds) const NANOEM_DECL_NOEXCEPT;

    const ByteArray *compositedFrameImage(nanoem_rsize_t offset) const NANOEM_DECL_NOEXCEPT;
    nanoem_u32_t width() const NANOEM_DECL_NOEXCEPT;
    nanoem_u32_t height() const NANOEM_DECL_NOEXCEPT;

private:
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

    enum DisposeOp {
        kDisposeOpNone = 0,
        kDisposeOpBackground,
        kDisposeOpPrevious,
    };
    enum BlendOp {
        kBlendOpSource = 0,
        kBlendOpOver,
    };

    Header m_header;
    AnimationControl m_control;
    FrameSequenceList m_frames;
};

} /* namespace image */

class Image NANOEM_DECL_SEALED : public IImageView, private NonCopyable {
public:
    Image();
    ~Image() NANOEM_DECL_NOEXCEPT;

    void create();
    void destroy();
    void setOriginData(const nanoem_u8_t *data, nanoem_rsize_t size);
    void setMipmapData(nanoem_rsize_t index, const nanoem_u8_t *data, nanoem_rsize_t size);
    void setLabel(const String &value);

    sg_image handle() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setHandle(sg_image value);
    sg_image_desc description() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setDescription(const sg_image_desc &value);
    const ByteArray *originData() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    const ByteArray *mipmapData(nanoem_rsize_t index) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    const char *filenameConstString() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    String filename() const NANOEM_DECL_OVERRIDE;
    void setFilename(const String &value);
    bool isFileExist() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setFileExist(bool value);

private:
    String m_filename;
    String m_label;
    sg_image m_handle;
    sg_image_desc m_description;
    ByteArray m_originData;
    ByteArray m_mipmapData[SG_MAX_MIPMAPS];
    bool m_fileExist;
};

class ImageLoader NANOEM_DECL_SEALED : private NonCopyable {
public:
    enum Flags {
        kFlagsFallbackWhiteOpaque = 0x1,
        kFlagsEnableMipmap = 0x2,
        kFlagsEnableFlipY = 0x4,
        kFlagsFallbackBlackOpaque = 0x8,
    };

    static image::APNG *decodeAPNG(ISeekableReader *reader, Error &error);
    static void copyImageDescrption(const sg_image_desc &desc, Image *image);
    static bool validateImageSize(const String &name, const sg_image_desc &desc, Error &error);
    static bool isScreenBMP(const char *path) NANOEM_DECL_NOEXCEPT;
    static void fill1x1PixelImage(const nanoem_u32_t *pixel, sg_image_desc &desc) NANOEM_DECL_NOEXCEPT;
    static void fill1x1WhitePixelImage(sg_image_desc &desc) NANOEM_DECL_NOEXCEPT;
    static void fill1x1BlackPixelImage(sg_image_desc &desc) NANOEM_DECL_NOEXCEPT;
    static void fill1x1TransparentPixelImage(sg_image_desc &desc) NANOEM_DECL_NOEXCEPT;
    static void flipImage(nanoem_u8_t *source, nanoem_u32_t width, nanoem_u32_t height, nanoem_u32_t bpp);
    static bool decodeImageWithSTB(
        const nanoem_u8_t *dataPtr, const size_t dataSize, sg_image_desc &desc, nanoem_u8_t **decodedImagePtr, Error &error);
    static void releaseDecodedImageWithSTB(nanoem_u8_t **decodedImagePtr);

    ImageLoader(const Project *project);
    ~ImageLoader() NANOEM_DECL_NOEXCEPT;

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
        ~ImmutableImageContainer() NANOEM_DECL_NOEXCEPT
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

    IImageView *decodeImageContainer(const ImmutableImageContainer &container, IDrawable *drawable, Error &error);

    const Project *m_project;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_IMAGELOADER_H_ */
