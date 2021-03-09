/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#ifndef NANOEM_EMAPP_MACOS_BASEVIDEORECORDER_H_
#define NANOEM_EMAPP_MACOS_BASEVIDEORECORDER_H_

#import "emapp/IVideoRecorder.h"

#import <CoreMedia/CoreMedia.h>
#import <CoreVideo/CoreVideo.h>
#import <IOSurface/IOSurface.h>

#include "emapp/Project.h"

@class AVAssetWriter;
@class AVAssetWriterInput;
@class AVAssetWriterInputPixelBufferAdaptor;

namespace nanoem {

class Accessory;
class Model;
class Project;

namespace internal {
class BlitPass;
}

namespace macos {

class BaseVideoRecorder : public IVideoRecorder, private NonCopyable {
public:
    BaseVideoRecorder(Project *projectPtr);
    ~BaseVideoRecorder() noexcept;

    bool isStarted() const noexcept override;
    bool start(Error &error) override;
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

    void setFileURI(const URI &value) override;
    void setSize(const Vector2UI16 &value) override;
    sg_pixel_format pixelFormat() const noexcept override;
    nanoem_frame_index_t duration() const noexcept override;

protected:
    OSType internalPixelFormat() const noexcept;
    Project *project();
    internal::BlitPass *blitter();
    void acquirePixelBuffer(CVPixelBufferRef *pixelBufferPtr, IOSurfaceRef *surfacePtr);
    bool appendPixelBuffer(nanoem_frame_index_t frameIndex, CVPixelBufferRef pixelBuffer);
    bool appendAudioSampleBuffer(nanoem_frame_index_t frameIndex);
    void blitPass(sg::PassBlock::IDrawQueue *drawQueue, sg_pass value);
    void updateAllMSAAImages(int width, int height);
    void enableOpenGLCompatibility();
    void enableMetalCompatibility();

    sg_image_desc m_description;
    sg_pass m_videoFramePass = { SG_INVALID_ID };
    sg_image m_colorImage = { SG_INVALID_ID };
    sg_image m_depthImage = { SG_INVALID_ID };

private:
    void setupAudioInput(Error &error);
    void setupVideoInput(int width, int height, Error &error);
    void destroyBlitter(internal::BlitPass *&value);

    AVAssetWriter *m_writer = nil;
    AVAssetWriterInput *m_audioWriterInput = nil;
    AVAssetWriterInput *m_videoWriterInput = nil;
    Project *m_project = nullptr;
    internal::BlitPass *m_blitter = nullptr;
    URI m_fileURI;
    Vector2UI16 m_lastViewportImageSize;
    Vector2UI16 m_outputSize;
    NSNumber *m_audioBits = nil;
    AudioFormatID m_audioCodec = 0;
    NSNumber *m_audioChannels = nil;
    NSNumber *m_audioDuration = nil;
    NSNumber *m_audioFrequency = nil;
    NSNumber *m_audioBitRate = nil;
    NSString *m_videoCodec = nil;
    NSString *m_videoProfile = nil;
    NSString *m_videoFileType = nil;
    tinystl::pair<sg_pixel_format, OSType> m_pixelFormat;
    tinystl::pair<nanoem_frame_index_t, nanoem_frame_index_t> m_lastAppendSample;
    Accessory *m_lastActiveAccessory = nullptr;
    Model *m_lastActiveModel = nullptr;
    PhysicsEngine::SimulationModeType m_lastPhysicsSimulationMode = PhysicsEngine::kSimulationModeDisable;
    internal::BlitPass *m_blitterMSAA = nullptr;
    CMAudioFormatDescriptionRef m_formatDescription = nullptr;
    CVPixelBufferPoolRef m_pixelBufferPool = nullptr;
    CVPixelBufferRef m_previousBuffer = nullptr;
    IOSurfaceRef m_previousSurface = nullptr;
    Vector4 m_blitRect = Vector4(0, 0, 1, 1);
    sg_pass_desc m_receivePassDesc;
    sg_pass m_receivePass;
    sg_pixel_format m_lastPixelFormat = SG_PIXELFORMAT_NONE;
    nanoem_frame_index_t m_lastFrameIndex = 0;
    nanoem_frame_index_t m_duration = 0;
    nanoem_f32_t m_lastDevicePixelViewportRatio = 1.0f;
    uint32_t m_videoFPS = 0;
    bool m_viewportAspectRatioEnabled = false;
    bool m_compatibleWithOpenGL = false;
    bool m_compatibleWithMetal = false;
    bool m_configured = false;
    bool m_cancelled = false;
};

} /* namespace macos */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_MACOS_BASEVIDEORECORDER_H_ */
