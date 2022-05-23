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
    struct State;
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

    nanoem_u32_t decodeAnimationControl(ISeekableReader *reader, State &state, Error &error);
    nanoem_u32_t decodeFrameControl(ISeekableReader *reader, State &state, Error &error);
    nanoem_u32_t decodeFrameData(ISeekableReader *reader, nanoem_u32_t chunkLength, State &state, Error &error);
    nanoem_u32_t decodeImageData(ISeekableReader *reader, nanoem_u32_t chunkLength, State &state, Error &error);
    nanoem_u32_t decodeImageEnd(State &state, Error &error);
    nanoem_u32_t decodeImageHeader(ISeekableReader *reader, State &state, Error &error);

    Header m_header;
    AnimationControl m_control;
    FrameSequenceList m_frames;
};

class DDS {
public:
    static const nanoem_u32_t kSignature;

    DDS();
    ~DDS() NANOEM_DECL_NOEXCEPT;

    bool decode(IReader *reader, Error &error);
    void setImageDescription(sg_image_desc &desc) const NANOEM_DECL_NOEXCEPT;

    const ByteArray *imageData(nanoem_rsize_t face, nanoem_rsize_t index) const NANOEM_DECL_NOEXCEPT;
    sg_pixel_format format() const NANOEM_DECL_NOEXCEPT;
    sg_image_type type() const NANOEM_DECL_NOEXCEPT;
    nanoem_u32_t width() const NANOEM_DECL_NOEXCEPT;
    nanoem_u32_t height() const NANOEM_DECL_NOEXCEPT;
    nanoem_u32_t depth() const NANOEM_DECL_NOEXCEPT;
    nanoem_u32_t mipmapCount() const NANOEM_DECL_NOEXCEPT;

private:
#pragma pack(push)
#pragma pack(1)
    struct PixelFormat {
        nanoem_u32_t m_size;
        nanoem_u32_t m_flags;
        nanoem_u32_t m_fourCC;
        nanoem_u32_t m_colorBitCount;
        nanoem_u32_t m_redBitMask;
        nanoem_u32_t m_greenBitMask;
        nanoem_u32_t m_blueBitMask;
        nanoem_u32_t m_alphaBitMask;
        bool testMask(nanoem_u32_t r, nanoem_u32_t g, nanoem_u32_t b, nanoem_u32_t a) const NANOEM_DECL_NOEXCEPT;
    };
    struct Header {
        nanoem_u32_t m_size;
        nanoem_u32_t m_flags;
        nanoem_u32_t m_height;
        nanoem_u32_t m_width;
        nanoem_u32_t m_stride;
        nanoem_u32_t m_depth;
        nanoem_u32_t m_mipmapCount;
        nanoem_u32_t m_reversed[11];
        PixelFormat m_pixelFormat;
        nanoem_u32_t m_caps[4];
        nanoem_u32_t m_reserved2;
    };
    struct ExtendedHeader {
        nanoem_u32_t m_dxgiFormat;
        nanoem_u32_t m_resourceDimension;
        nanoem_u32_t m_miscFlags;
        nanoem_u32_t m_arraySize;
        nanoem_u32_t m_miscFlags2;
    };
#pragma pack(pop)

    static sg_pixel_format findPixelFormat(const PixelFormat &value, const ExtendedHeader &extend) NANOEM_DECL_NOEXCEPT;
    static void getBCImageSize(nanoem_u32_t width, nanoem_u32_t height, nanoem_rsize_t blockSize,
        nanoem_rsize_t &numBytes) NANOEM_DECL_NOEXCEPT;
    static void getRawImageSize(nanoem_u32_t width, nanoem_u32_t height, nanoem_rsize_t pixelSize,
        nanoem_rsize_t &numBytes) NANOEM_DECL_NOEXCEPT;
    void getImageSize(nanoem_u32_t width, nanoem_u32_t height, nanoem_rsize_t &numBytes) const NANOEM_DECL_NOEXCEPT;
    bool decodeImage(IReader *reader, nanoem_u32_t faceIndex, nanoem_u32_t mipmapIndex, nanoem_u32_t width,
        nanoem_u32_t height, nanoem_rsize_t numBytes, ByteArray &bytes, Error &error);

    Header m_header;
    ExtendedHeader m_extendedHeader;
    ByteArray m_images[SG_CUBEFACE_NUM][SG_MAX_MIPMAPS];
    sg_pixel_format m_format;
};

class PFM {
public:
    PFM();
    ~PFM() NANOEM_DECL_NOEXCEPT;

    bool decode(const ByteArray &bytes, Error &error);
    void setImageDescription(sg_image_desc &desc) const NANOEM_DECL_NOEXCEPT;

    sg_pixel_format format() const NANOEM_DECL_NOEXCEPT;
    nanoem_u32_t width() const NANOEM_DECL_NOEXCEPT;
    nanoem_u32_t height() const NANOEM_DECL_NOEXCEPT;

private:
    ByteArray m_image;
    sg_pixel_format m_format;
    nanoem_u32_t m_width;
    nanoem_u32_t m_height;
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
    static image::DDS *decodeDDS(IReader *reader, Error &error);
    static image::PFM *decodePFM(const ByteArray &bytes, Error &error);
    static void copyImageDescrption(const sg_image_desc &desc, Image *image);
    static bool validateImageSize(const sg_image_desc &desc, const char *name, Error &error);
    static bool isScreenBMP(const char *path) NANOEM_DECL_NOEXCEPT;
    static void fill1x1PixelImage(const nanoem_u32_t *pixel, sg_image_desc &desc) NANOEM_DECL_NOEXCEPT;
    static void fill1x1WhitePixelImage(sg_image_desc &desc) NANOEM_DECL_NOEXCEPT;
    static void fill1x1BlackPixelImage(sg_image_desc &desc) NANOEM_DECL_NOEXCEPT;
    static void fill1x1TransparentPixelImage(sg_image_desc &desc) NANOEM_DECL_NOEXCEPT;
    static void flipImage(nanoem_u8_t *source, nanoem_u32_t width, nanoem_u32_t height, nanoem_u32_t bpp);
    static bool decodeImageWithSTB(const nanoem_u8_t *dataPtr, const size_t dataSize, const char *name,
        sg_image_desc &desc, nanoem_u8_t **decodedImagePtr, Error &error);
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
