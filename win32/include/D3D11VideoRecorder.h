/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_WIN32_D3D11VIDEORECORDER_H_
#define NANOEM_EMAPP_WIN32_D3D11VIDEORECORDER_H_

#include "emapp/IVideoRecorder.h"
#include "emapp/URI.h"

#if WINVER >= _WIN32_WINNT_WIN7

#include <codecapi.h>
#include <mfapi.h>

struct ID3D11Device;
struct ID3D11Texture2D;
struct IMFSinkWriter;

namespace nanoem {

class Accessory;
class Model;
class Project;

namespace internal {
class BlitPass;
}

namespace win32 {

class D3D11VideoRecorder : public IVideoRecorder {
public:
    D3D11VideoRecorder(Project *project, ID3D11Device *device);
    ~D3D11VideoRecorder() override;

    bool isStarted() const noexcept override;
    bool start(Error &error) override;
    bool capture(nanoem_frame_index_t frameIndex) override;
    bool finish(Error &error) override;
    void setAudioCodec(const String &value) override;
    void setVideoCodec(const String &value) override;
    void setVideoProfile(const String &value) override;
    void setVideoPixelFormat(sg_pixel_format value) override;
    void setViewportAspectRatioEnabled(bool value) override;
    void getAllAvailableAudioCodecs(StringPairList &value) const override;
    void getAllAvailableVideoCodecs(StringPairList &value) const override;
    void getAllAvailableVideoProfiles(StringPairList &value) const override;
    void getAllAvailableVideoPixelFormats(FormatPairList &value) const override;
    void getAllAvailableExtensions(StringList &value) const override;
    bool isCancelled() const noexcept override;
    void cancel() override;
    bool isConfigured() const noexcept override;
    void makeConfigured() override;
    void setSize(const Vector2UI16 &value) override;
    void setFileURI(const URI &value) override;
    sg_pixel_format pixelFormat() const noexcept override;
    nanoem_frame_index_t duration() const noexcept override;

private:
    typedef HRESULT(WINAPI *pfn_MFCreateDXGISurfaceBuffer)(REFIID, IUnknown *, UINT, BOOL, IMFMediaBuffer **);
    static const char *const kGUIDFormat;
    static const char *const kLabelPrefixName;
    static GUID fromString(const String &value);
    static String toString(const GUID &value);
    void setInputAudioStream(Error &error);
    void setOutputAudioStream(Error &error);
    void setInputVideoStream(Error &error);
    void setOutputVideoStream(Error &error);
    void blitPass(sg::PassBlock::IDrawQueue *drawQueue, sg_pass value);
    void updateDepthImage(int width, int height);
    void updateAllMSAAImages(int width, int height);
    void destroyBlitter(internal::BlitPass *&value);

    pfn_MFCreateDXGISurfaceBuffer _MFCreateDXGISurfaceBuffer = nullptr;
    Project *m_project = nullptr;
    ID3D11Device *m_device = nullptr;
    IMFSinkWriter *m_writer = nullptr;
    ID3D11Texture2D *m_texture = nullptr;
    internal::BlitPass *m_blitter = nullptr;
    internal::BlitPass *m_blitterMSAA = nullptr;
    HMODULE m_mfplatDll = nullptr;
    sg_image_desc m_colorImageDescription;
    sg_pass_desc m_receiveMSAAPassDesc;
    sg_image m_colorImage = { SG_INVALID_ID };
    sg_image m_depthImage = { SG_INVALID_ID };
    sg_pass m_receiveMSAAPass = { SG_INVALID_ID };
    sg_pass m_videoFramePass = { SG_INVALID_ID };
    URI m_fileURI;
    GUID m_audioCodec = MFAudioFormat_AAC;
    GUID m_videoCodec = MFVideoFormat_H264;
    eAVEncH264VProfile m_videoProfile = eAVEncH264VProfile_High;
    eAVEncH264VLevel m_videoLevel = eAVEncH264VLevel4_2;
    sg_pixel_format m_videoPixelFormat = SG_PIXELFORMAT_NONE;
    Accessory *m_lastActiveAccessory = nullptr;
    Model *m_lastActiveModel = nullptr;
    Vector4 m_blitRect = Vector4(0, 0, 1, 1);
    Vector2UI16 m_lastViewportSize;
    Vector2UI16 m_size;
    nanoem_u32_t m_fps = 0;
    nanoem_u32_t m_maxBitRate = 0;
    DWORD m_audioStreamIndex = MAXDWORD;
    DWORD m_videoStreamIndex = MAXDWORD;
    nanoem_frame_index_t m_lastFrameIndex = 0;
    nanoem_frame_index_t m_duration = 0;
    nanoem_f32_t m_lastDevicePixelViewportRatio = 1.0f;
    bool m_viewportAspectRatioEnabled = false;
    bool m_started = false;
    bool m_configured = false;
    bool m_cancelled = false;
};

} /* namespace win32 */
} /* namespace nanoem */

#endif /* WINVER >= _WIN32_WINNT_WIN7 */

#endif /* NANOEM_EMAPP_WIN32_D3D11VIDEORECORDER_H_ */
