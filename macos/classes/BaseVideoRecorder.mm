/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#import "BaseVideoRecorder.h"

#import "ErrorExtension.h"
#import <AVFoundation/AVFoundation.h>
#import <AppKit/AppKit.h>

#include "emapp/BaseApplicationService.h"
#include "emapp/IAudioPlayer.h"
#include "emapp/IEventPublisher.h"
#include "emapp/Project.h"
#include "emapp/StringUtils.h"
#include "emapp/internal/BlitPass.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace macos {
namespace {
const char *const kAudioCodecKeyAAC = "aac";
const char *const kAudioCodecKeyOpus = "opus";
const char *const kAudioCodecKeyMP3 = "mp3";
const char *const kAudioCodecKeyALAC = "alac";
const char *const kAudioCodecKeyPCM = "alac";
}

BaseVideoRecorder::BaseVideoRecorder(Project *projectPtr)
    : m_project(projectPtr)
    , m_audioCodec(kAudioFormatMPEG4AAC)
    , m_audioBitRate([[NSNumber alloc] initWithUnsignedInt:256000])
    , m_videoCodec(AVVideoCodecH264)
    , m_videoProfile(AVVideoProfileLevelH264HighAutoLevel)
    , m_videoFileType(AVFileTypeMPEG4)
    , m_pixelFormat(SG_PIXELFORMAT_RGBA8, kCVPixelFormatType_32BGRA)
    , m_lastAppendSample(Motion::kMaxFrameIndex, Motion::kMaxFrameIndex)
{
    Inline::clearZeroMemory(m_description);
    Inline::clearZeroMemory(m_receivePassDesc);
    m_description.pixel_format = SG_PIXELFORMAT_RGBA8;
    m_description.usage = SG_USAGE_IMMUTABLE;
    m_description.mag_filter = m_description.min_filter = SG_FILTER_LINEAR;
    m_description.wrap_u = m_description.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
    m_description.render_target = true;
    m_depthImage = { SG_INVALID_ID };
    m_receivePass = { SG_INVALID_ID };
    m_blitter = nanoem_new(internal::BlitPass(projectPtr, false));
}

BaseVideoRecorder::~BaseVideoRecorder() noexcept
{
    sg::destroy_image(m_colorImage);
    sg::destroy_image(m_depthImage);
    sg::destroy_image(m_receivePassDesc.color_attachments[0].image);
    sg::destroy_image(m_receivePassDesc.depth_stencil_attachment.image);
    sg::destroy_pass(m_videoFramePass);
    destroyBlitter(m_blitterMSAA);
    destroyBlitter(m_blitter);
    if (m_previousSurface) {
        IOSurfaceDecrementUseCount(m_previousSurface);
        m_previousSurface = nullptr;
    }
    if (m_previousBuffer) {
        CVPixelBufferRelease(m_previousBuffer);
        m_previousBuffer = nullptr;
    }
    if (m_pixelBufferPool) {
        CVPixelBufferPoolRelease(m_pixelBufferPool);
        m_pixelBufferPool = nullptr;
    }
}

bool
BaseVideoRecorder::isStarted() const noexcept
{
    return m_writer.status == AVAssetWriterStatusWriting;
}

bool
BaseVideoRecorder::start(Error &error)
{
    bool started = false;
    int width = m_outputSize.x, height = m_outputSize.y;
    if (!m_writer && !m_fileURI.isEmpty() && width > 0 && height > 0) {
        NSString *filePath = [[NSString alloc] initWithUTF8String:m_fileURI.absolutePathConstString()];
        NSURL *fileURL = [[NSURL alloc] initFileURLWithPath:filePath];
        [[NSFileManager defaultManager] removeItemAtURL:fileURL error:nil];
        NSError *err;
        m_writer = [[AVAssetWriter alloc] initWithURL:fileURL fileType:m_videoFileType error:&err];
        if (m_writer) {
            m_videoFPS = m_project->preferredMotionFPS();
            setupAudioInput(error);
            setupVideoInput(width, height, error);
            if (!error.hasReason()) {
                if ([m_writer startWriting]) {
                    m_lastActiveAccessory = m_project->activeAccessory();
                    m_lastActiveModel = m_project->activeModel();
                    m_lastFrameIndex = m_project->currentLocalFrameIndex();
                    m_lastPixelFormat = m_project->viewportPixelFormat();
                    m_lastPhysicsSimulationMode = m_project->physicsEngine()->simulationMode();
                    m_duration = m_project->duration();
                    m_project->setActiveAccessory(0);
                    m_project->setActiveModel(0);
                    m_project->setPhysicsSimulationMode(
                        m_lastPhysicsSimulationMode != PhysicsEngine::kSimulationModeDisable
                            ? PhysicsEngine::kSimulationModeEnableTracing
                            : PhysicsEngine::kSimulationModeDisable);
                    m_project->seek(0, true);
                    [m_writer startSessionAtSourceTime:kCMTimeZero];
                    m_lastViewportImageSize = m_project->logicalScaleUniformedViewportImageSize();
                    m_lastDevicePixelViewportRatio = m_project->viewportDevicePixelRatio();
                    m_project->setViewportCaptured(true);
                    m_project->setViewportDevicePixelRatio(1);
                    m_project->resizeUniformedViewportImage(m_outputSize);
                    m_project->setViewportPixelFormat(m_pixelFormat.first);
                    if (m_viewportAspectRatioEnabled) {
                        const Vector4 layout(m_project->logicalScaleUniformedViewportLayoutRect());
                        const nanoem_f32_t ratio = glm::max(width / layout.z, height / layout.w);
                        const Vector2 viewportImageSizeF(layout.z * ratio, layout.w * ratio);
                        m_project->resizeUniformedViewportLayout(
                            Vector4UI16(0, 0, viewportImageSizeF.x, viewportImageSizeF.y));
                        const Vector4 rect((width - viewportImageSizeF.x) / width,
                            (height - viewportImageSizeF.y) / height, (viewportImageSizeF.x / width),
                            (viewportImageSizeF.y / height));
                        m_blitRect = rect;
                    }
                    started = true;
                }
                else {
                    error = m_writer.error.nanoem;
                }
            }
        }
        else {
            error = err.nanoem;
        }
    }
    return started;
}

bool
BaseVideoRecorder::finish(Error &error)
{
    bool result = true;
    if (m_writer) {
        __block dispatch_semaphore_t sema = dispatch_semaphore_create(0);
        if (isStarted()) {
            [m_audioWriterInput markAsFinished];
            [m_videoWriterInput markAsFinished];
            [m_writer finishWritingWithCompletionHandler:^() {
                dispatch_semaphore_signal(sema);
            }];
            dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
            result = m_writer.status != AVAssetWriterStatusFailed;
            if (!result) {
                error = m_writer.error.nanoem;
            }
            m_project->setActiveAccessory(m_lastActiveAccessory);
            m_project->setActiveModel(m_lastActiveModel);
            m_project->setPhysicsSimulationMode(m_lastPhysicsSimulationMode);
            m_project->setViewportPixelFormat(m_lastPixelFormat);
            m_project->setViewportCaptured(false);
            m_project->setViewportDevicePixelRatio(m_lastDevicePixelViewportRatio);
            m_project->resizeUniformedViewportImage(m_lastViewportImageSize);
            m_project->seek(m_lastFrameIndex, true);
            m_project->restart(m_lastFrameIndex);
        }
        m_videoWriterInput = nil;
        m_writer = nil;
        IOSurfaceDecrementUseCount(m_previousSurface);
        m_previousSurface = nullptr;
        CVPixelBufferPoolRelease(m_pixelBufferPool);
        m_pixelBufferPool = nullptr;
    }
    return result;
}
void
BaseVideoRecorder::setAudioCodec(const String &value)
{
    if (StringUtils::equals(value.c_str(), kAudioCodecKeyAAC)) {
        m_audioCodec = kAudioFormatMPEG4AAC;
    }
    else if (StringUtils::equals(value.c_str(), kAudioCodecKeyOpus)) {
        m_audioCodec = kAudioFormatOpus;
    }
    else if (StringUtils::equals(value.c_str(), kAudioCodecKeyMP3)) {
        m_audioCodec = kAudioFormatMPEGLayer3;
    }
    else if (StringUtils::equals(value.c_str(), kAudioCodecKeyALAC)) {
        m_audioCodec = kAudioFormatAppleLossless;
    }
    else if (StringUtils::equals(value.c_str(), kAudioCodecKeyPCM)) {
        m_audioCodec = kAudioFormatLinearPCM;
    }
}

void
BaseVideoRecorder::setVideoCodec(const String &value)
{
    m_videoCodec = [[NSString alloc] initWithUTF8String:value.c_str()];
}

void
BaseVideoRecorder::setVideoProfile(const String &value)
{
    m_videoProfile =
        [m_videoCodec isEqualToString:AVVideoCodecH264] ? [[NSString alloc] initWithUTF8String:value.c_str()] : nil;
}

void
BaseVideoRecorder::setVideoPixelFormat(sg_pixel_format value)
{
    m_pixelFormat.first = value;
    switch (value) {
    case SG_PIXELFORMAT_RGB10A2: {
        m_pixelFormat.second = kCVPixelFormatType_30RGB;
        break;
    }
    case SG_PIXELFORMAT_RGBA16F: {
        m_pixelFormat.second = kCVPixelFormatType_64RGBAHalf;
        break;
    }
    case SG_PIXELFORMAT_RGBA32F: {
        m_pixelFormat.second = kCVPixelFormatType_128RGBAFloat;
        break;
    }
    case SG_PIXELFORMAT_RGBA8:
    default:
        m_pixelFormat.first = SG_PIXELFORMAT_RGBA8;
        m_pixelFormat.second = kCVPixelFormatType_32BGRA;
        break;
    }
    if (m_compatibleWithMetal && m_pixelFormat.first == SG_PIXELFORMAT_RGBA8) {
        m_pixelFormat.first = SG_PIXELFORMAT_BGRA8;
    }
}

void
BaseVideoRecorder::setViewportAspectRatioEnabled(bool value)
{
    m_viewportAspectRatioEnabled = value;
}

void
BaseVideoRecorder::getAllAvailableAudioCodecs(StringPairList &value) const
{
    value.clear();
    value.push_back(StringPair(kAudioCodecKeyAAC, "AAC"));
    if ([m_videoFileType isEqualToString:AVFileTypeQuickTimeMovie]) {
        value.push_back(StringPair(kAudioCodecKeyALAC, "Apple Lossless"));
        value.push_back(StringPair(kAudioCodecKeyPCM, "Linear PCM"));
    }
}

void
BaseVideoRecorder::getAllAvailableVideoCodecs(StringPairList &value) const
{
    value.clear();
    value.push_back(StringPair(AVVideoCodecH264.UTF8String, "H.264/AVC"));
    if (@available(macOS 10.13, *)) {
        value.push_back(StringPair(AVVideoCodecTypeHEVC.UTF8String, "H.265/HEVC"));
    }
    if ([m_videoFileType isEqualToString:AVFileTypeQuickTimeMovie]) {
        value.push_back(StringPair(AVVideoCodecAppleProRes422.UTF8String, "Apple ProRes 422"));
        value.push_back(StringPair(AVVideoCodecAppleProRes4444.UTF8String, "Apple ProRes 4444"));
    }
}

void
BaseVideoRecorder::getAllAvailableVideoProfiles(StringPairList &value) const
{
    value.clear();
    value.push_back(StringPair(AVVideoProfileLevelH264HighAutoLevel.UTF8String, "H.264 High Profile"));
    value.push_back(StringPair(AVVideoProfileLevelH264MainAutoLevel.UTF8String, "H.264 Main Profile"));
    value.push_back(StringPair(AVVideoProfileLevelH264BaselineAutoLevel.UTF8String, "H.264 Baseline Profile"));
}

void
BaseVideoRecorder::getAllAvailableVideoPixelFormats(FormatPairList &value) const
{
    value.clear();
    value.push_back(tinystl::make_pair(SG_PIXELFORMAT_RGBA8, String("RGBA/8bits")));
#ifndef NDEBUG
    value.push_back(tinystl::make_pair(SG_PIXELFORMAT_RGB10A2, String("RGB/10bits")));
    value.push_back(tinystl::make_pair(SG_PIXELFORMAT_RGBA16F, String("RGBA/16bits")));
#endif
}

void
BaseVideoRecorder::getAllAvailableExtensions(StringList &value) const
{
    value.clear();
    value.push_back("mp4");
    value.push_back("mov");
}

bool
BaseVideoRecorder::isCancelled() const noexcept
{
    return m_cancelled;
}

void
BaseVideoRecorder::cancel()
{
    m_cancelled = true;
}

bool
BaseVideoRecorder::isConfigured() const noexcept
{
    return m_configured;
}

void
BaseVideoRecorder::makeConfigured()
{
    m_configured = true;
}

void
BaseVideoRecorder::setFileURI(const URI &value)
{
    m_fileURI = value;
    if (StringUtils::equals(value.pathExtension().c_str(), "mov")) {
        m_videoFileType = AVFileTypeQuickTimeMovie;
    }
}

void
BaseVideoRecorder::setSize(const Vector2UI16 &value)
{
    m_outputSize = value;
}

sg_pixel_format
BaseVideoRecorder::pixelFormat() const noexcept
{
    return m_pixelFormat.first;
}

nanoem_frame_index_t
BaseVideoRecorder::duration() const noexcept
{
    return m_duration;
}

OSType
BaseVideoRecorder::internalPixelFormat() const noexcept
{
    return m_pixelFormat.second;
}

Project *
BaseVideoRecorder::project()
{
    return m_project;
}

internal::BlitPass *
BaseVideoRecorder::blitter()
{
    return m_blitter;
}

void
BaseVideoRecorder::acquirePixelBuffer(CVPixelBufferRef *pixelBufferPtr, IOSurfaceRef *surfacePtr)
{
    CVPixelBufferRef pixelBuffer = nullptr;
    CVPixelBufferPoolCreatePixelBuffer(kCFAllocatorDefault, m_pixelBufferPool, &pixelBuffer);
    if (pixelBuffer) {
        IOSurfaceRef surface = *surfacePtr = CVPixelBufferGetIOSurface(pixelBuffer);
        if (surface != m_previousSurface) {
            if (m_previousSurface) {
                IOSurfaceDecrementUseCount(m_previousSurface);
            }
            m_previousSurface = surface;
            if (surface) {
                IOSurfaceIncrementUseCount(surface);
            }
        }
        if (pixelBuffer != m_previousBuffer) {
            if (m_previousBuffer) {
                CVPixelBufferRelease(m_previousBuffer);
            }
            m_previousBuffer = pixelBuffer;
            *pixelBufferPtr = pixelBuffer;
        }
    }
}

bool
BaseVideoRecorder::appendPixelBuffer(nanoem_frame_index_t frameIndex, CVPixelBufferRef pixelBuffer)
{
    bool appended = m_lastAppendSample.second == frameIndex;
    if (!appended && m_videoWriterInput.readyForMoreMediaData) {
        CMVideoFormatDescriptionRef videoFormatDescription;
        OSStatus status =
            CMVideoFormatDescriptionCreateForImageBuffer(kCFAllocatorDefault, pixelBuffer, &videoFormatDescription);
        if (status == kCMBlockBufferNoErr) {
            CMSampleTimingInfo sampleTimingInfo = kCMTimingInfoInvalid;
            sampleTimingInfo.presentationTimeStamp = CMTimeMake(frameIndex, m_videoFPS);
            CMSampleBufferRef sampleBuffer;
            status = CMSampleBufferCreateForImageBuffer(kCFAllocatorDefault, pixelBuffer, YES, nullptr, nullptr,
                videoFormatDescription, &sampleTimingInfo, &sampleBuffer);
            if (status == kCMBlockBufferNoErr) {
                appended = [m_videoWriterInput appendSampleBuffer:sampleBuffer];
                if (appended) {
                    m_lastAppendSample.second = frameIndex;
                }
                CFRelease(sampleBuffer);
            }
            CFRelease(videoFormatDescription);
        }
    }
    return appended;
}

bool
BaseVideoRecorder::appendAudioSampleBuffer(nanoem_frame_index_t frameIndex)
{
    bool appended = m_lastAppendSample.first == frameIndex || m_audioWriterInput == nil;
    if (!appended && m_audioWriterInput.readyForMoreMediaData) {
        const IAudioPlayer *audio = m_project->audioPlayer();
        const size_t sampleBufferSize =
            size_t(audio->numChannels() * audio->sampleRate() * (audio->bitsPerSample() / 8) * (1.0f / m_videoFPS));
        const size_t offset = size_t(frameIndex * sampleBufferSize);
        const ByteArray *samplesPtr = audio->linearPCMSamples();
        /* use malloc for kCFAllocatorMalloc */
        if (void *slice = ::malloc(sampleBufferSize)) {
            const size_t rest = samplesPtr->size() >= offset ? samplesPtr->size() - offset : 0;
            memcpy(slice, samplesPtr->data() + offset, glm::min(sampleBufferSize, rest));
            CMBlockBufferRef blockBuffer;
            OSStatus status = CMBlockBufferCreateWithMemoryBlock(kCFAllocatorDefault, slice, sampleBufferSize,
                kCFAllocatorMalloc, nullptr, 0, sampleBufferSize, kCMBlockBufferAssureMemoryNowFlag, &blockBuffer);
            if (status == kCMBlockBufferNoErr) {
                const AudioStreamBasicDescription *description =
                    CMAudioFormatDescriptionGetStreamBasicDescription(m_formatDescription);
                CMItemCount numSamples = CMItemCount(sampleBufferSize / description->mBytesPerFrame);
                CMSampleTimingInfo sampleTimingInfo = kCMTimingInfoInvalid;
                sampleTimingInfo.presentationTimeStamp = CMTimeMake(frameIndex, m_videoFPS);
                size_t sampleSizeArray = description->mBytesPerFrame;
                CMSampleBufferRef sampleBuffer;
                status = CMSampleBufferCreate(kCFAllocatorDefault, blockBuffer, YES, nullptr, nullptr,
                    m_formatDescription, numSamples, 1, &sampleTimingInfo, 1, &sampleSizeArray, &sampleBuffer);
                if (status == kCMBlockBufferNoErr) {
                    appended = [m_audioWriterInput appendSampleBuffer:sampleBuffer];
                    if (appended) {
                        m_lastAppendSample.first = frameIndex;
                    }
                }
                if (sampleBuffer) {
                    CFRelease(sampleBuffer);
                }
            }
            if (blockBuffer) {
                CFRelease(blockBuffer);
            }
        }
    }
    return appended;
}

void
BaseVideoRecorder::blitPass(sg::PassBlock::IDrawQueue *drawQueue, sg_pass value)
{
    static const char *kSourceImageName = "IOSurfaceRef";
    PixelFormat format;
    format.setColorPixelFormat(pixelFormat(), 0);
    sg_image viewportImage = m_project->viewportPrimaryImage();
    uint32_t sampleCount = m_project->sampleCount();
    if (sampleCount > 1) {
        m_blitterMSAA->blit(drawQueue, tinystl::make_pair(m_receivePass, kSourceImageName),
            tinystl::make_pair(viewportImage, kSourceImageName), m_blitRect, format);
        m_blitter->blit(drawQueue, tinystl::make_pair(value, m_project->findRenderPassName(value)),
            tinystl::make_pair(m_receivePassDesc.color_attachments[0].image, kSourceImageName), m_blitRect, format);
    }
    else {
        m_blitter->blit(drawQueue, tinystl::make_pair(value, m_project->findRenderPassName(value)),
            tinystl::make_pair(viewportImage, kSourceImageName), m_blitRect, format);
    }
}

void
BaseVideoRecorder::updateAllMSAAImages(int width, int height)
{
    if (m_project->sampleCount() > 1) {
        sg::destroy_image(m_receivePassDesc.color_attachments[0].image);
        sg::destroy_image(m_receivePassDesc.depth_stencil_attachment.image);
        sg_image_desc desc;
        Inline::clearZeroMemory(desc);
        desc.pixel_format = m_pixelFormat.first;
        desc.usage = SG_USAGE_IMMUTABLE;
        desc.width = width;
        desc.height = height;
        desc.mag_filter = desc.min_filter = SG_FILTER_LINEAR;
        desc.wrap_u = desc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
        desc.render_target = true;
        desc.sample_count = 1;
        m_receivePassDesc.color_attachments[0].image = sg::make_image(&desc);
        desc.pixel_format = SG_PIXELFORMAT_DEPTH_STENCIL;
        m_receivePassDesc.depth_stencil_attachment.image = sg::make_image(&desc);
        sg::destroy_pass(m_receivePass);
        m_receivePass = sg::make_pass(&m_receivePassDesc);
        m_project->setRenderPassName(m_videoFramePass, "@nanoem/BaseVideoRecorder/ReceiveMSAAPass");
        if (!m_blitterMSAA) {
            bool flipY = !sg::query_features().origin_top_left;
            m_blitterMSAA = nanoem_new(internal::BlitPass(m_project, flipY));
        }
    }
}

void
BaseVideoRecorder::enableOpenGLCompatibility()
{
    m_compatibleWithOpenGL = true;
}

void
BaseVideoRecorder::enableMetalCompatibility()
{
    m_compatibleWithMetal = true;
    m_pixelFormat.first = SG_PIXELFORMAT_BGRA8;
}

void
BaseVideoRecorder::setupAudioInput(Error &error)
{
    IAudioPlayer *audio = m_project->audioPlayer();
    if (audio->isLoaded()) {
        m_audioBits = [[NSNumber alloc] initWithUnsignedInt:audio->bitsPerSample()];
        m_audioChannels = [[NSNumber alloc] initWithUnsignedInt:audio->numChannels()];
        m_audioDuration = [[NSNumber alloc]
            initWithUnsignedInt:static_cast<nanoem_u32_t>(audio->durationRational().subdivide() * m_videoFPS)];
        m_audioFrequency = [[NSNumber alloc] initWithUnsignedInt:audio->sampleRate()];
        AudioChannelLayout channelLayout;
        Inline::clearZeroMemory(channelLayout);
        channelLayout.mChannelLayoutTag = kAudioChannelLayoutTag_Stereo;
        NSDictionary *initialOutputAudioSettings = @{
            AVFormatIDKey : @(m_audioCodec),
            AVSampleRateKey : m_audioFrequency,
            AVNumberOfChannelsKey : m_audioChannels,
            AVChannelLayoutKey : [[NSData alloc] initWithBytes:&channelLayout length:sizeof(channelLayout)]
        };
        NSMutableDictionary *outputAudioSettings =
            [[NSMutableDictionary alloc] initWithDictionary:initialOutputAudioSettings];
        if (m_audioCodec == kAudioFormatLinearPCM) {
            [outputAudioSettings setObject:m_audioBits forKey:AVLinearPCMBitDepthKey];
            [outputAudioSettings setObject:@NO forKey:AVLinearPCMIsBigEndianKey];
            [outputAudioSettings setObject:@NO forKey:AVLinearPCMIsNonInterleavedKey];
            [outputAudioSettings setObject:@NO forKey:AVLinearPCMIsFloatKey];
        }
        else if (m_audioCodec == kAudioFormatAppleLossless) {
            [outputAudioSettings setObject:m_audioBits forKey:AVEncoderBitDepthHintKey];
        }
        else {
            [outputAudioSettings setObject:m_audioBitRate forKey:AVEncoderBitRateKey];
        }
        @try {
            m_audioWriterInput = [[AVAssetWriterInput alloc] initWithMediaType:AVMediaTypeAudio
                                                                outputSettings:outputAudioSettings];
            m_audioWriterInput.expectsMediaDataInRealTime = YES;
            [m_writer addInput:m_audioWriterInput];
            AudioStreamBasicDescription streamBasicDescription = {};
            streamBasicDescription.mSampleRate = m_audioFrequency.floatValue;
            streamBasicDescription.mFormatID = kAudioFormatLinearPCM;
            streamBasicDescription.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
            streamBasicDescription.mFramesPerPacket = 1;
            streamBasicDescription.mChannelsPerFrame = m_audioChannels.unsignedIntValue;
            streamBasicDescription.mBitsPerChannel = m_audioBits.unsignedIntValue;
            streamBasicDescription.mBytesPerFrame =
                (streamBasicDescription.mBitsPerChannel / 8) * streamBasicDescription.mChannelsPerFrame;
            streamBasicDescription.mBytesPerPacket =
                streamBasicDescription.mBytesPerFrame * streamBasicDescription.mFramesPerPacket;
            CMAudioFormatDescriptionCreate(kCFAllocatorDefault, &streamBasicDescription, sizeof(channelLayout),
                &channelLayout, 0, nullptr, nullptr, &m_formatDescription);
        } @catch (NSException *exception) {
            error = Error(exception.reason.UTF8String, nullptr, Error::kDomainTypeOS);
        }
    }
}

void
BaseVideoRecorder::setupVideoInput(int width, int height, Error &error)
{
    NSNumber *widthValue = [NSNumber numberWithInt:width];
    NSNumber *heightValue = [NSNumber numberWithInt:height];
    NSDictionary *initialOutputVideoSettings = @{
        (NSString *) kCVPixelBufferWidthKey : widthValue,
        (NSString *) kCVPixelBufferHeightKey : heightValue,
        (NSString *) kCVPixelBufferPixelFormatTypeKey : @(m_pixelFormat.second),
        (NSString *) kCVPixelBufferOpenGLCompatibilityKey : @(m_compatibleWithOpenGL),
        (NSString *) kCVPixelBufferIOSurfacePropertiesKey : @ {},
    };
    NSMutableDictionary *pixelBufferAttributes =
        [[NSMutableDictionary alloc] initWithDictionary:initialOutputVideoSettings];
    if (@available(macOS 10.11, *)) {
        [pixelBufferAttributes setObject:@(m_compatibleWithMetal)
                                  forKey:(NSString *) kCVPixelBufferMetalCompatibilityKey];
    }
    CVPixelBufferPoolCreate(
        kCFAllocatorDefault, nullptr, (__bridge CFDictionaryRef) pixelBufferAttributes, &m_pixelBufferPool);
    @try {
        NSDictionary *outputVideoSettings;
        if (m_videoProfile) {
            outputVideoSettings = @{
                AVVideoWidthKey : widthValue,
                AVVideoHeightKey : heightValue,
                AVVideoCodecKey : m_videoCodec,
                AVVideoCompressionPropertiesKey : @ { AVVideoProfileLevelKey : m_videoProfile },
            };
        }
        else {
            outputVideoSettings = @ {
                AVVideoWidthKey : widthValue,
                AVVideoHeightKey : heightValue,
                AVVideoCodecKey : m_videoCodec,
            };
        }
        m_videoWriterInput = [[AVAssetWriterInput alloc] initWithMediaType:AVMediaTypeVideo
                                                            outputSettings:outputVideoSettings];
        m_videoWriterInput.expectsMediaDataInRealTime = YES;
        [m_writer addInput:m_videoWriterInput];
    } @catch (NSException *exception) {
        error = Error(exception.reason.UTF8String, nullptr, Error::kDomainTypeOS);
    }
}

void
BaseVideoRecorder::destroyBlitter(internal::BlitPass *&value)
{
    if (value) {
        value->destroy();
        nanoem_delete(value);
        value = nullptr;
    }
}

} /* namespace macos */
} /* namespace nanoem */
