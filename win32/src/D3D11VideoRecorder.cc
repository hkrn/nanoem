/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "D3D11VideoRecorder.h"
#if WINVER >= _WIN32_WINNT_WIN7

#include <d3d11.h>
#include <dxgi.h>
#include <evr.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <stdio.h>

#include "COMInline.h"
#include "emapp/Error.h"
#include "emapp/IAudioPlayer.h"
#include "emapp/Project.h"
#include "emapp/StringUtils.h"
#include "emapp/internal/BlitPass.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace win32 {

const char *const D3D11VideoRecorder::kGUIDFormat =
    "%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX";
const char *const D3D11VideoRecorder::kLabelPrefixName = "@nanoem/win32/D3D11VideoRecorder";

GUID
D3D11VideoRecorder::fromString(const String &value)
{
    GUID guid;
    ::sscanf_s(value.c_str(), kGUIDFormat, &guid.Data1, &guid.Data2, &guid.Data3, &guid.Data4[0], &guid.Data4[1],
        &guid.Data4[2], &guid.Data4[3], &guid.Data4[4], &guid.Data4[5], &guid.Data4[6], &guid.Data4[7]);
    return guid;
}

String
D3D11VideoRecorder::toString(const GUID &value)
{
    char buffer[38];
    bx::snprintf(buffer, sizeof(buffer), kGUIDFormat, value.Data1, value.Data2, value.Data3, value.Data4[0],
        value.Data4[1], value.Data4[2], value.Data4[3], value.Data4[4], value.Data4[5], value.Data4[6], value.Data4[7]);
    return buffer;
}

D3D11VideoRecorder::D3D11VideoRecorder(Project *project, ID3D11Device *device)
    : m_project(project)
    , m_device(device)
{
    Inline::clearZeroMemory(m_colorImageDescription);
    Inline::clearZeroMemory(m_receiveMSAAPassDesc);
    m_colorImageDescription.pixel_format = SG_PIXELFORMAT_BGRA8;
    m_colorImageDescription.usage = SG_USAGE_IMMUTABLE;
    m_colorImageDescription.mag_filter = m_colorImageDescription.min_filter = SG_FILTER_LINEAR;
    m_colorImageDescription.wrap_u = m_colorImageDescription.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
    m_colorImageDescription.render_target = true;
    m_device->AddRef();
    m_blitter = nanoem_new(internal::BlitPass(project, false));
    m_mfplatDll = LoadLibraryA("mfplat.dll");
    if (m_mfplatDll) {
        _MFCreateDXGISurfaceBuffer =
            reinterpret_cast<pfn_MFCreateDXGISurfaceBuffer>(GetProcAddress(m_mfplatDll, "MFCreateDXGISurfaceBuffer"));
    }
}

D3D11VideoRecorder::~D3D11VideoRecorder()
{
    COMInline::safeRelease(m_writer);
    if (m_mfplatDll) {
        FreeLibrary(m_mfplatDll);
        m_mfplatDll = nullptr;
    }
    sg::destroy_image(m_colorImage);
    sg::destroy_image(m_depthImage);
    sg::destroy_image(m_receiveMSAAPassDesc.color_attachments[0].image);
    sg::destroy_image(m_receiveMSAAPassDesc.depth_stencil_attachment.image);
    sg::destroy_pass(m_receiveMSAAPass);
    sg::destroy_pass(m_videoFramePass);
    destroyBlitter(m_blitterMSAA);
    destroyBlitter(m_blitter);
    COMInline::safeRelease(m_device);
}

bool
D3D11VideoRecorder::isStarted() const noexcept
{
    return m_started;
}

bool
D3D11VideoRecorder::start(Error &error)
{
    MutableWideString ws;
    StringUtils::getWideCharString(m_fileURI.absolutePathConstString(), ws);
    IMFAttributes *attributes = nullptr;
    MFCreateAttributes(&attributes, 1);
    attributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE);
    COMInline::wrapCall(MFCreateSinkWriterFromURL(ws.data(), nullptr, attributes, &m_writer), error);
    if (!error.hasReason()) {
        m_fps = m_project->preferredMotionFPS();
        setOutputAudioStream(error);
        setOutputVideoStream(error);
        setInputAudioStream(error);
        setInputVideoStream(error);
        if (!error.hasReason()) {
            COMInline::wrapCall(m_writer->BeginWriting(), error);
            if (!error.hasReason()) {
                m_lastActiveAccessory = m_project->activeAccessory();
                m_lastActiveModel = m_project->activeModel();
                m_lastFrameIndex = m_project->currentLocalFrameIndex();
                m_duration = m_project->duration();
                m_project->setActiveAccessory(nullptr);
                m_project->setActiveModel(nullptr);
                m_project->seek(0, true);
                m_lastViewportSize = m_project->logicalScaleUniformedViewportImageSize();
                m_lastDevicePixelViewportRatio = m_project->viewportDevicePixelRatio();
                m_project->setViewportDevicePixelRatio(1);
                m_project->setViewportImageSize(m_size);
                m_project->setViewportCaptured(true);
                if (m_viewportAspectRatioEnabled) {
                    const int width = m_size.x, height = m_size.y;
                    const Vector4 layout(m_project->logicalScaleUniformedViewportLayoutRect());
                    const nanoem_f32_t ratio = glm::max(width / layout.z, height / layout.w);
                    const Vector2 viewportImageSizeF(layout.z * ratio, layout.w * ratio);
                    m_project->resizeUniformedViewportLayout(
                        Vector4UI16(0, 0, viewportImageSizeF.x, viewportImageSizeF.y));
                    const Vector4 rect((width - viewportImageSizeF.x) / width, (height - viewportImageSizeF.y) / height,
                        (viewportImageSizeF.x / width), (viewportImageSizeF.y / height));
                    m_blitRect = rect;
                }
                m_started = true;
            }
        }
    }
    return m_started;
}

bool
D3D11VideoRecorder::capture(nanoem_frame_index_t frameIndex)
{
    SG_PUSH_GROUPF("D3D11VideoRecorder::capture(frameIndex=%d, width=%d, height=%d)", frameIndex, m_size.x, m_size.y);
    const nanoem_u16_t width = m_size.x, height = m_size.y;
    Error error;
    D3D11_TEXTURE2D_DESC desc;
    Inline::clearZeroMemory(desc);
    desc.Width = width;
    desc.Height = height;
    desc.ArraySize = desc.MipLevels = desc.SampleDesc.Count = 1;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.Usage = D3D11_USAGE_DEFAULT;
    COMInline::safeRelease(m_texture);
    COMInline::wrapCall(m_device->CreateTexture2D(&desc, nullptr, &m_texture), error);
    if (!error.hasReason() && m_texture) {
        sg_image_desc colorImageDescription;
        Inline::clearZeroMemory(colorImageDescription);
        colorImageDescription.width = width;
        colorImageDescription.height = height;
        colorImageDescription.pixel_format = m_videoPixelFormat;
        colorImageDescription.mag_filter = colorImageDescription.min_filter = SG_FILTER_NEAREST;
        colorImageDescription.wrap_u = colorImageDescription.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
        colorImageDescription.d3d11_texture = m_texture;
        colorImageDescription.render_target = true;
        updateDepthImage(width, height);
        char label[Inline::kMarkerStringLength];
        if (Inline::isDebugLabelEnabled()) {
            StringUtils::format(label, sizeof(label), "%s/Color/%d", kLabelPrefixName, frameIndex);
            colorImageDescription.label = label;
        }
        sg::destroy_image(m_colorImage);
        m_colorImage = sg::make_image(&colorImageDescription);
        sg_pass_desc pd;
        Inline::clearZeroMemory(pd);
        pd.color_attachments[0].image = m_colorImage;
        pd.depth_stencil_attachment.image = m_depthImage;
        if (Inline::isDebugLabelEnabled()) {
            StringUtils::format(label, sizeof(label), "%s/Pass/%d", kLabelPrefixName, frameIndex);
            pd.label = label;
        }
        sg::destroy_pass(m_videoFramePass);
        m_videoFramePass = sg::make_pass(&pd);
        m_project->setRenderPassName(m_videoFramePass, label);
        sg::PassBlock::IDrawQueue *drawQueue = m_project->sharedBatchDrawQueue();
        blitPass(drawQueue, m_videoFramePass);
        sg_pass_action pa = {};
        sg::PassBlock pb(drawQueue, m_videoFramePass, pa);
        struct CallbackArgument {
            D3D11VideoRecorder *m_self;
            nanoem_frame_index_t m_frameIndex;
        };
        CallbackArgument *argument = nanoem_new(CallbackArgument);
        argument->m_self = this;
        argument->m_frameIndex = frameIndex;
        pb.registerCallback(
            [](sg_pass, void *opaque) {
                auto args = static_cast<CallbackArgument *>(opaque);
                D3D11VideoRecorder *self = args->m_self;
                Error error;
                IMFMediaBuffer *buffer = nullptr;
                COMInline::wrapCall(
                    self->_MFCreateDXGISurfaceBuffer(__uuidof(ID3D11Texture2D), self->m_texture, 0, false, &buffer),
                    error);
                /*
                   see https://stackoverflow.com/questions/47930340/sinkwriter-writesample-fails-with-e-invalidarg
                  */
                IMF2DBuffer2 *buffer2D = nullptr;
                COMInline::wrapCall(buffer->QueryInterface(IID_PPV_ARGS(&buffer2D)), error);
                DWORD length;
                COMInline::wrapCall(buffer2D->GetContiguousLength(&length), error);
                COMInline::wrapCall(buffer->SetCurrentLength(length), error);
                IMFSample *sample = nullptr;
                COMInline::wrapCall(MFCreateVideoSampleFromSurface(nullptr, &sample), error);
                if (!error.hasReason() && sample) {
                    static const uint64_t kNanoSecondsUnit = 10000000;
                    const uint64_t unit = (1.0 / self->m_fps) * kNanoSecondsUnit;
                    sample->SetSampleTime(args->m_frameIndex * unit);
                    sample->SetSampleDuration(unit);
                    sample->AddBuffer(buffer);
                    COMInline::wrapCall(self->m_writer->WriteSample(self->m_videoStreamIndex, sample), error);
                    COMInline::safeRelease(sample);
                }
                COMInline::safeRelease(buffer);
                COMInline::safeRelease(buffer2D);
                nanoem_delete(args);
            },
            argument);
    }
    SG_POP_GROUP();
    return !error.hasReason();
}

bool
D3D11VideoRecorder::finish(Error &error)
{
    if (m_writer) {
        COMInline::wrapCall(m_writer->Finalize(), error);
        m_project->setActiveAccessory(m_lastActiveAccessory);
        m_project->setActiveModel(m_lastActiveModel);
        m_project->setViewportCaptured(false);
        m_project->setViewportDevicePixelRatio(m_lastDevicePixelViewportRatio);
        m_project->resizeUniformedViewportImage(m_lastViewportSize);
        m_project->seek(m_lastFrameIndex, true);
        m_project->restart(m_lastFrameIndex);
    }
    return !error.hasReason();
}

void
D3D11VideoRecorder::setAudioCodec(const String &value)
{
    m_audioCodec = fromString(value);
}

void
D3D11VideoRecorder::setVideoCodec(const String &value)
{
    m_videoCodec = fromString(value);
}

void
D3D11VideoRecorder::setVideoProfile(const String &value)
{
    /*
       bitrate from
       http://up-cat.net/x264%2528vbv%252Dmaxrate%252Cvbv%252Dbufsize%252Cprofile%252Clevel%2529%252C%2BH%252E264%2528Profile%252FLevel%2529.html
     */
    if (StringUtils::equals(value.c_str(), "high")) {
        m_videoProfile = eAVEncH264VProfile_High;
        m_videoLevel = eAVEncH264VLevel4_2;
        m_maxBitRate = 40000000; /* 40Mbps */
    }
    else if (StringUtils::equals(value.c_str(), "main")) {
        m_videoProfile = eAVEncH264VProfile_Main;
        m_videoLevel = eAVEncH264VLevel4_2;
        m_maxBitRate = 40000000; /* 40Mbps */
    }
    else if (StringUtils::equals(value.c_str(), "base")) {
        m_videoProfile = eAVEncH264VProfile_Base;
        m_videoLevel = eAVEncH264VLevel3_2;
        m_maxBitRate = 16000000; /* 16Mbps */
    }
}

void
D3D11VideoRecorder::setVideoPixelFormat(sg_pixel_format value)
{
    if (value == SG_PIXELFORMAT_RGBA8) {
        /* override */
        value = SG_PIXELFORMAT_BGRA8;
    }
    m_videoPixelFormat = value;
}

void
D3D11VideoRecorder::setViewportAspectRatioEnabled(bool value)
{
    m_viewportAspectRatioEnabled = value;
}

void
D3D11VideoRecorder::getAllAvailableAudioCodecs(StringPairList &value) const
{
    value.clear();
    value.push_back(StringPair(toString(MFAudioFormat_AAC), "AAC"));
#if 0
    value.push_back(StringPair(toString(MFAudioFormat_FLAC), "FLAC"));
    value.push_back(StringPair(toString(MFAudioFormat_ALAC), "Apple Lossless"));
    value.push_back(StringPair(toString(MFAudioFormat_Opus), "Opus"));
#endif
}

void
D3D11VideoRecorder::getAllAvailableVideoCodecs(StringPairList &value) const
{
    value.clear();
    value.push_back(StringPair(toString(MFVideoFormat_H264), "H.264/AVC"));
    value.push_back(StringPair(toString(MFVideoFormat_H265), "H.265/HEVC"));
    value.push_back(StringPair(toString(MFVideoFormat_WMV3), "WMV3"));
    value.push_back(StringPair(toString(MFVideoFormat_VP80), "VP8"));
    value.push_back(StringPair(toString(MFVideoFormat_VP90), "VP9"));
    value.push_back(StringPair(toString(MFVideoFormat_VP10), "VP10"));
    value.push_back(StringPair(toString(MFVideoFormat_AV1), "AV1"));
}

void
D3D11VideoRecorder::getAllAvailableVideoProfiles(StringPairList &value) const
{
    value.clear();
    value.push_back(StringPair("base", "H.264 Base Profile"));
    value.push_back(StringPair("main", "H.264 Main Profile"));
    value.push_back(StringPair("high", "H.264 High Profile"));
}

void
D3D11VideoRecorder::getAllAvailableVideoPixelFormats(FormatPairList &value) const
{
    value.clear();
    value.push_back(tinystl::make_pair(SG_PIXELFORMAT_BGRA8, String("RGBA 8bits")));
}

void
D3D11VideoRecorder::getAllAvailableExtensions(StringList &value) const
{
    value.clear();
    value.push_back("mp4");
}

bool
D3D11VideoRecorder::isCancelled() const noexcept
{
    return m_cancelled;
}

void
D3D11VideoRecorder::cancel()
{
    m_cancelled = true;
}

bool
D3D11VideoRecorder::isConfigured() const noexcept
{
    return m_configured;
}

void
D3D11VideoRecorder::makeConfigured()
{
    m_configured = true;
}

void
D3D11VideoRecorder::setSize(const Vector2UI16 &value)
{
    m_size = value;
}

void
D3D11VideoRecorder::setFileURI(const URI &value)
{
    m_fileURI = value;
}

sg_pixel_format
D3D11VideoRecorder::pixelFormat() const noexcept
{
    return m_videoPixelFormat;
}

nanoem_frame_index_t
D3D11VideoRecorder::duration() const noexcept
{
    return m_duration;
}

void
D3D11VideoRecorder::setInputAudioStream(Error &error)
{
    const IAudioPlayer *audio = m_project->audioPlayer();
    if (audio->isLoaded()) {
        IMFMediaType *inputType = nullptr;
        const nanoem_u32_t bitsPerSample = audio->bitsPerSample(), numChannels = audio->numChannels(),
                           sampleRate = audio->sampleRate();
        COMInline::wrapCall(MFCreateMediaType(&inputType), error);
        COMInline::wrapCall(inputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio), error);
        COMInline::wrapCall(inputType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM), error);
        COMInline::wrapCall(inputType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, numChannels), error);
        COMInline::wrapCall(inputType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, bitsPerSample), error);
        COMInline::wrapCall(inputType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, sampleRate), error);
        COMInline::wrapCall(inputType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE), error);
        COMInline::wrapCall(m_writer->SetInputMediaType(m_audioStreamIndex, inputType, nullptr), error);
        COMInline::safeRelease(inputType);
    }
}

void
D3D11VideoRecorder::setOutputAudioStream(Error &error)
{
    const IAudioPlayer *audio = m_project->audioPlayer();
    if (audio->isLoaded()) {
        IMFMediaType *outputType = nullptr;
        const nanoem_u32_t bitsPerSample = audio->bitsPerSample(), numChannels = audio->numChannels(),
                           sampleRate = audio->sampleRate();
        COMInline::wrapCall(MFCreateMediaType(&outputType), error);
        COMInline::wrapCall(outputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio), error);
        COMInline::wrapCall(outputType->SetGUID(MF_MT_SUBTYPE, m_audioCodec), error);
        COMInline::wrapCall(outputType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, numChannels), error);
        COMInline::wrapCall(outputType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, bitsPerSample), error);
        COMInline::wrapCall(outputType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, sampleRate), error);
        COMInline::wrapCall(m_writer->AddStream(outputType, &m_audioStreamIndex), error);
        COMInline::safeRelease(outputType);
    }
}

void
D3D11VideoRecorder::setInputVideoStream(Error &error)
{
    IMFMediaType *inputType = nullptr;
    COMInline::wrapCall(MFCreateMediaType(&inputType), error);
    COMInline::wrapCall(inputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video), error);
    COMInline::wrapCall(inputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_ARGB32), error);
    COMInline::wrapCall(inputType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE), error);
    COMInline::wrapCall(inputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive), error);
    COMInline::wrapCall(MFSetAttributeSize(inputType, MF_MT_FRAME_SIZE, m_size.x, m_size.y), error);
    COMInline::wrapCall(MFSetAttributeRatio(inputType, MF_MT_FRAME_RATE, m_fps, 1), error);
    COMInline::wrapCall(MFSetAttributeRatio(inputType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1), error);
    COMInline::wrapCall(m_writer->SetInputMediaType(m_videoStreamIndex, inputType, nullptr), error);
    COMInline::safeRelease(inputType);
}

void
D3D11VideoRecorder::setOutputVideoStream(Error &error)
{
    IMFMediaType *outputType = nullptr;
    COMInline::wrapCall(MFCreateMediaType(&outputType), error);
    COMInline::wrapCall(outputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video), error);
    COMInline::wrapCall(outputType->SetGUID(MF_MT_SUBTYPE, m_videoCodec), error);
    COMInline::wrapCall(outputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive), error);
    COMInline::wrapCall(outputType->SetUINT32(MF_MT_VIDEO_PROFILE, m_videoProfile), error);
    COMInline::wrapCall(outputType->SetUINT32(MF_MT_VIDEO_LEVEL, m_videoLevel), error);
    COMInline::wrapCall(outputType->SetUINT32(MF_MT_AVG_BITRATE, m_maxBitRate), error);
    COMInline::wrapCall(MFSetAttributeSize(outputType, MF_MT_FRAME_SIZE, m_size.x, m_size.y), error);
    COMInline::wrapCall(MFSetAttributeRatio(outputType, MF_MT_FRAME_RATE, m_fps, 1), error);
    COMInline::wrapCall(MFSetAttributeRatio(outputType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1), error);
    COMInline::wrapCall(m_writer->AddStream(outputType, &m_videoStreamIndex), error);
    COMInline::safeRelease(outputType);
}

void
D3D11VideoRecorder::blitPass(sg::PassBlock::IDrawQueue *drawQueue, sg_pass value)
{
    static const glm::vec4 kRect(0, 0, 1, 1);
    const sg_image viewportImage = m_project->viewportPrimaryImage();
    const int sampleCount = m_project->sampleCount();
    PixelFormat format;
    format.setColorPixelFormat(m_colorImageDescription.pixel_format, 0);
    if (sampleCount > 1) {
        m_blitterMSAA->blit(drawQueue, tinystl::make_pair(m_receiveMSAAPass, kLabelPrefixName),
            tinystl::make_pair(viewportImage, Project::kViewportPrimaryName), kRect, format);
        m_blitter->blit(drawQueue, tinystl::make_pair(value, m_project->findRenderPassName(value)),
            tinystl::make_pair(m_receiveMSAAPassDesc.color_attachments[0].image, kLabelPrefixName), kRect, format);
    }
    else {
        m_blitter->blit(drawQueue, tinystl::make_pair(value, kLabelPrefixName),
            tinystl::make_pair(viewportImage, Project::kViewportPrimaryName), kRect, format);
    }
}

void
D3D11VideoRecorder::updateDepthImage(int width, int height)
{
    if (width > 0 && height > 0 &&
        (m_colorImageDescription.width != width || m_colorImageDescription.height != height)) {
        SG_PUSH_GROUP("D3D11VideoRecorderr::updateDepthImage()");
        m_colorImageDescription.width = width;
        m_colorImageDescription.height = height;
        sg_image_desc depthImageDescription(m_colorImageDescription);
        depthImageDescription.pixel_format = SG_PIXELFORMAT_DEPTH_STENCIL;
        char label[Inline::kMarkerStringLength];
        if (Inline::isDebugLabelEnabled()) {
            StringUtils::format(label, sizeof(label), "%s/Depth", kLabelPrefixName);
            depthImageDescription.label = label;
        }
        sg::destroy_image(m_depthImage);
        m_depthImage = sg::make_image(&depthImageDescription);
        SG_LABEL_IMAGE(m_depthImage, label);
        updateAllMSAAImages(width, height);
        SG_POP_GROUP();
    }
}

void
D3D11VideoRecorder::updateAllMSAAImages(int width, int height)
{
    if (m_project->sampleCount() > 1) {
        SG_PUSH_GROUPF("D3D11VideoRecorderr::updateAllMSAAImages(width=%d, height=%d)", width, height);
        sg_image_desc receiveMSAAImageDescription;
        Inline::clearZeroMemory(receiveMSAAImageDescription);
        receiveMSAAImageDescription.pixel_format = m_videoPixelFormat;
        receiveMSAAImageDescription.usage = SG_USAGE_IMMUTABLE;
        receiveMSAAImageDescription.width = width;
        receiveMSAAImageDescription.height = height;
        receiveMSAAImageDescription.mag_filter = receiveMSAAImageDescription.min_filter = SG_FILTER_LINEAR;
        receiveMSAAImageDescription.wrap_u = receiveMSAAImageDescription.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
        receiveMSAAImageDescription.render_target = true;
        receiveMSAAImageDescription.sample_count = 1;
        char label[Inline::kMarkerStringLength];
        if (Inline::isDebugLabelEnabled()) {
            StringUtils::format(label, sizeof(label), "%s/MSAA/Color", kLabelPrefixName);
            receiveMSAAImageDescription.label = label;
        }
        sg::destroy_image(m_receiveMSAAPassDesc.color_attachments[0].image);
        m_receiveMSAAPassDesc.color_attachments[0].image = sg::make_image(&receiveMSAAImageDescription);
        SG_LABEL_IMAGE(m_receiveMSAAPassDesc.color_attachments[0].image, label);
        receiveMSAAImageDescription.pixel_format = SG_PIXELFORMAT_DEPTH_STENCIL;
        if (Inline::isDebugLabelEnabled()) {
            StringUtils::format(label, sizeof(label), "%s/MSAA/Depth", kLabelPrefixName);
            receiveMSAAImageDescription.label = label;
        }
        sg::destroy_image(m_receiveMSAAPassDesc.depth_stencil_attachment.image);
        m_receiveMSAAPassDesc.depth_stencil_attachment.image = sg::make_image(&receiveMSAAImageDescription);
        SG_LABEL_IMAGE(m_receiveMSAAPassDesc.depth_stencil_attachment.image, label);
        sg::destroy_pass(m_receiveMSAAPass);
        m_receiveMSAAPass = sg::make_pass(&m_receiveMSAAPassDesc);
        if (!m_blitterMSAA) {
            const bool topLeft = sg::query_features().origin_top_left;
            m_blitterMSAA = nanoem_new(internal::BlitPass(m_project, !topLeft));
        }
        SG_POP_GROUP();
    }
}

void
D3D11VideoRecorder::destroyBlitter(internal::BlitPass *&value)
{
    if (value) {
        value->destroy();
        nanoem_delete(value);
        value = nullptr;
    }
}

} /* namespace win32 */
} /* namespace nanoem */

#endif /* WINVER >= _WIN32_WINNT_WIN7 */
