/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_IVIDEORECORDER_H_
#define NANOEM_EMAPP_IVIDEORECORDER_H_

#include "emapp/Forward.h"

namespace nanoem {

class Error;
class URI;

class IVideoRecorder {
public:
    typedef tinystl::pair<sg_pixel_format, String> FormatPair;
    typedef tinystl::vector<FormatPair, TinySTLAllocator> FormatPairList;

    virtual ~IVideoRecorder() NANOEM_DECL_NOEXCEPT
    {
    }

    virtual bool isStarted() const NANOEM_DECL_NOEXCEPT = 0;
    virtual bool start(Error &error) = 0;
    virtual bool capture(nanoem_frame_index_t frameIndex) = 0;
    virtual bool finish(Error &error) = 0;

    virtual void setAudioCodec(const String &value) = 0;
    virtual void setVideoCodec(const String &value) = 0;
    virtual void setVideoProfile(const String &value) = 0;
    virtual void setVideoPixelFormat(sg_pixel_format value) = 0;
    virtual void setViewportAspectRatioEnabled(bool value) = 0;
    virtual void getAllAvailableAudioCodecs(StringPairList &value) const = 0;
    virtual void getAllAvailableVideoCodecs(StringPairList &value) const = 0;
    virtual void getAllAvailableVideoProfiles(StringPairList &value) const = 0;
    virtual void getAllAvailableVideoPixelFormats(FormatPairList &value) const = 0;
    virtual void getAllAvailableExtensions(StringList &value) const = 0;
    virtual bool isCancelled() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void cancel() = 0;
    virtual bool isConfigured() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void makeConfigured() = 0;

    virtual void setSize(const Vector2UI16 &value) = 0;
    virtual void setFileURI(const URI &value) = 0;
    virtual sg_pixel_format pixelFormat() const NANOEM_DECL_NOEXCEPT = 0;
    virtual nanoem_frame_index_t duration() const NANOEM_DECL_NOEXCEPT = 0;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_IVIDEORECORDER_H_ */
