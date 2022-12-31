/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/sdk/Decoder.h"
#include "emapp/sdk/Encoder.h"

#import <AudioToolbox/AudioToolbox.h>
#import <QTKit/QTKit.h>

#ifndef __bridge
#define __bridge
#endif

struct nanoem_application_plugin_encoder_t {
    nanoem_application_plugin_encoder_t()
        : m_movie(nil)
        , m_fileURL(nil)
        , m_error(nil)
        , m_frameAttributes(nil)
        , m_duration(nil)
        , m_fps(nil)
        , m_width(nil)
        , m_height(nil)
        , m_format(nil)
        , m_yflip(nil)
    {
    }
    ~nanoem_application_plugin_encoder_t()
    {
        [m_movie release];
        [m_fileURL release];
        [m_frameAttributes release];
        [m_error release];
        [m_duration release];
        [m_fps release];
        [m_width release];
        [m_height release];
        [m_format release];
        [m_yflip release];
    }

    nanoem_application_plugin_result_t
    open()
    {
        NSError *error = nil;
        m_movie = [[QTMovie alloc] initToWritableFile:[m_fileURL path] error:&error];
        if (error == nil) {
            NSNumber *quality = [NSNumber numberWithLong:codecMaxQuality];
            m_frameAttributes = [[NSDictionary alloc]
                initWithObjectsAndKeys:@"mp4v", QTAddImageCodecType, quality, QTAddImageCodecQuality, nil];
            [quality release];
        }
        else {
            m_error = [error retain];
        }
        return error == nil ? NANOEM_APPLICATION_PLUGIN_RESULT_OK : NANOEM_APPLICATION_PLUGIN_RESULT_ERROR;
    }
    nanoem_application_plugin_result_t
    setOption(nanoem_u32_t key, const void *value, size_t /* size */)
    {
        nanoem_application_plugin_result_t ret = NANOEM_APPLICATION_PLUGIN_RESULT_OK;
        switch (key) {
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_FPS:
            m_fps = [[NSNumber alloc] initWithUnsignedInt:*static_cast<const nanoem_u32_t *>(value)];
            break;
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_DURATION:
            m_duration = [[NSNumber alloc] initWithLong:*static_cast<const nanoem_frame_index_t *>(value)];
            break;
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_LOCATION: {
            NSString *path = [[NSString alloc] initWithUTF8String:static_cast<const char *>(value)];
            m_fileURL = [[NSURL alloc] initFileURLWithPath:path];
            [path release];
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_WIDTH: {
            m_width = [[NSNumber alloc] initWithUnsignedInt:*static_cast<const nanoem_u32_t *>(value)];
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_HEIGHT: {
            m_height = [[NSNumber alloc] initWithUnsignedInt:*static_cast<const nanoem_u32_t *>(value)];
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_YFLIP: {
            m_yflip = [[NSNumber alloc] initWithBool:*static_cast<const nanoem_u32_t *>(value) != 0];
            break;
        }
        default:
            ret = NANOEM_APPLICATION_PLUGIN_RESULT_ERROR;
            break;
        }
        return ret;
    }
    nanoem_application_plugin_result_t
    encodeAudioFrame(
        nanoem_frame_index_t /* current_frame_index */, const nanoem_u8_t * /* data */, nanoem_rsize_t /* size */)
    {
        return NANOEM_APPLICATION_PLUGIN_RESULT_OK;
    }
    nanoem_application_plugin_result_t
    encodeVideoFrame(nanoem_frame_index_t /* current_frame_index */, const nanoem_u8_t *data, nanoem_rsize_t size)
    {
        NSMutableData *bytes = [[NSMutableData alloc] initWithCapacity:size];
        const size_t stride = size / m_height.unsignedIntegerValue;
        if (m_yflip.boolValue) {
            const nanoem_u8_t *sourcePtr = data + stride * m_height.unsignedIntValue - stride;
            for (nanoem_u32_t y = 0; y < m_height.unsignedIntValue; ++y) {
                nanoem_u8_t *destPtr = static_cast<nanoem_u8_t *>(bytes.mutableBytes) + y * stride;
                memcpy(destPtr, sourcePtr, stride);
                for (nanoem_u32_t x = 0; x < m_width.unsignedIntValue; x++) {
                    const nanoem_u8_t *p = &sourcePtr[x * 4];
                    nanoem_u8_t *mp = &destPtr[x * 4];
                    mp[0] = p[2];
                    mp[2] = p[0];
                }
                sourcePtr -= stride;
            }
        }
        else {
            const nanoem_u8_t *sourcePtr = data;
            for (nanoem_u32_t y = 0; y < m_height.unsignedIntValue; ++y) {
                nanoem_u8_t *destPtr = static_cast<nanoem_u8_t *>(bytes.mutableBytes) + y * stride;
                memcpy(destPtr, sourcePtr, stride);
                for (nanoem_u32_t x = 0; x < m_width.unsignedIntValue; x++) {
                    const nanoem_u8_t *p = &sourcePtr[x * 4];
                    nanoem_u8_t *mp = &destPtr[x * 4];
                    mp[0] = p[2];
                    mp[2] = p[0];
                }
                sourcePtr += stride;
            }
        }
        uint8_t *bytesPtr = static_cast<uint8_t *>(bytes.mutableBytes), **bytesPtrPtr = &bytesPtr;
        NSBitmapImageRep *frameBitmap = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:bytesPtrPtr
                                                                                pixelsWide:m_width.integerValue
                                                                                pixelsHigh:m_height.integerValue
                                                                             bitsPerSample:8
                                                                           samplesPerPixel:4
                                                                                  hasAlpha:YES
                                                                                  isPlanar:NO
                                                                            colorSpaceName:NSCalibratedRGBColorSpace
                                                                               bytesPerRow:stride
                                                                              bitsPerPixel:0];
        NSImage *frameImage = [[NSImage alloc] init];
        [frameImage addRepresentation:frameBitmap];
        [frameBitmap release];
        [m_movie addImage:frameImage forDuration:QTMakeTime(1, m_fps.intValue) withAttributes:m_frameAttributes];
        nanoem_application_plugin_result_t ret = NANOEM_APPLICATION_PLUGIN_RESULT_OK;
        if (![m_movie updateMovieFile]) {
            ret = NANOEM_APPLICATION_PLUGIN_RESULT_ERROR;
        }
        [frameImage release];
        return ret;
    }
    nanoem_application_plugin_result_t
    interrupt()
    {
        return NANOEM_APPLICATION_PLUGIN_RESULT_OK;
    }
    const char *
    failureReason() const
    {
        return m_error.localizedFailureReason.UTF8String;
    }
    const char *
    recoverySuggestion() const
    {
        return m_error.localizedRecoverySuggestion.UTF8String;
    }
    nanoem_application_plugin_result_t
    close()
    {
        return NANOEM_APPLICATION_PLUGIN_RESULT_OK;
    }

    QTMovie *m_movie;
    NSURL *m_fileURL;
    NSError *m_error;
    NSDictionary *m_frameAttributes;
    NSNumber *m_duration;
    NSNumber *m_fps;
    NSNumber *m_width;
    NSNumber *m_height;
    NSNumber *m_format;
    NSNumber *m_yflip;
    bool m_disposable;
};

struct nanoem_application_plugin_decoder_t {
    nanoem_application_plugin_decoder_t()
        : m_audioFileRef(NULL)
        , m_movie(nil)
        , m_error(nil)
        , m_audioFileURL(nil)
        , m_videoFileURL(nil)
        , m_fps(nil)
    {
        memset(&m_description, 0, sizeof(m_description));
        memset(&m_bufferList, 0, sizeof(m_bufferList));
        m_description.mSampleRate = 44100;
        m_description.mFormatID = kAudioFormatLinearPCM;
        m_description.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
        m_description.mBitsPerChannel = 16;
        m_description.mChannelsPerFrame = 2;
        m_description.mFramesPerPacket = 1;
        m_description.mBytesPerFrame = (m_description.mBitsPerChannel / 8) * m_description.mChannelsPerFrame;
        m_description.mBytesPerPacket = m_description.mBytesPerFrame * m_description.mFramesPerPacket;
        m_description.mReserved = 0;
        m_bufferList.mNumberBuffers = 1;
        AudioBuffer &audioBuffer = m_bufferList.mBuffers[0];
        audioBuffer.mNumberChannels = m_description.mChannelsPerFrame;
    }
    ~nanoem_application_plugin_decoder_t()
    {
        if (void *ptr = m_bufferList.mBuffers[0].mData) {
            free(ptr);
        }
        [m_movie release];
        [m_error release];
        [m_audioFileURL release];
        [m_videoFileURL release];
        [m_fps release];
    }

    nanoem_application_plugin_result_t
    open()
    {
        nanoem_application_plugin_result_t ret = NANOEM_APPLICATION_PLUGIN_RESULT_OK;
        if (m_audioFileURL && !m_audioFileRef) {
            OSStatus status = ExtAudioFileOpenURL((__bridge CFURLRef) m_audioFileURL, &m_audioFileRef);
            if (status == kAudioServicesNoError) {
                status = ExtAudioFileSetProperty(
                    m_audioFileRef, kExtAudioFileProperty_ClientDataFormat, sizeof(m_description), &m_description);
                if (status != kAudioServicesNoError) {
                    m_error = [[NSError alloc] initWithDomain:NSOSStatusErrorDomain code:status userInfo:nil];
                    ret = NANOEM_APPLICATION_PLUGIN_RESULT_ERROR;
                }
            }
            else {
                m_error = [[NSError alloc] initWithDomain:NSOSStatusErrorDomain code:status userInfo:nil];
                ret = NANOEM_APPLICATION_PLUGIN_RESULT_ERROR;
            }
        }
        if (m_videoFileURL && !m_movie) {
            m_movie = [[QTMovie alloc] initWithURL:m_videoFileURL];
        }
        return ret;
    }
    nanoem_application_plugin_result_t
    setOption(nanoem_u32_t key, const void *value, nanoem_rsize_t /* size */)
    {
        nanoem_application_plugin_result_t ret = NANOEM_APPLICATION_PLUGIN_RESULT_OK;
        switch (key) {
        case NANOEM_APPLICATION_PLUGIN_DECODER_OPTION_FPS: {
            [m_fps release];
            m_fps = [[NSNumber alloc] initWithUnsignedInt:*static_cast<const nanoem_u32_t *>(value)];
            AudioBuffer &audioBuffer = m_bufferList.mBuffers[0];
            if (audioBuffer.mData) {
                free(audioBuffer.mData);
            }
            audioBuffer.mDataByteSize =
                nanoem_rsize_t((1.0f / m_fps.floatValue) * m_description.mSampleRate * m_description.mBytesPerFrame);
            audioBuffer.mData = malloc(audioBuffer.mDataByteSize);
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_DECODER_OPTION_AUDIO_LOCATION: {
            NSString *path = [[NSString alloc] initWithUTF8String:static_cast<const char *>(value)];
            ExtAudioFileDispose(m_audioFileRef);
            m_audioFileRef = NULL;
            [m_audioFileURL release];
            m_audioFileURL = [[NSURL alloc] initFileURLWithPath:path];
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_DECODER_OPTION_VIDEO_LOCATION: {
            NSString *path = [[NSString alloc] initWithUTF8String:static_cast<const char *>(value)];
            [m_movie release];
            m_movie = nil;
            [m_videoFileURL release];
            m_videoFileURL = [[NSURL alloc] initFileURLWithPath:path];
            break;
        }
        default:
            ret = NANOEM_APPLICATION_PLUGIN_RESULT_ERROR;
            break;
        }
        return ret;
    }
    nanoem_application_plugin_result_t
    audioFormatValue(nanoem_u32_t key, void *value, nanoem_rsize_t *size)
    {
        nanoem_application_plugin_result_t ret = NANOEM_APPLICATION_PLUGIN_RESULT_OK;
        switch (key) {
        case NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_NUM_BITS: {
            nanoem_u32_t bits = m_description.mBitsPerChannel;
            *static_cast<nanoem_u32_t *>(value) = bits;
            *size = sizeof(bits);
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_NUM_CHANNELS: {
            nanoem_u32_t channels = m_description.mChannelsPerFrame;
            *static_cast<nanoem_u32_t *>(value) = channels;
            *size = sizeof(channels);
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_FREQUENCY: {
            nanoem_u32_t frequency = nanoem_u32_t(m_description.mSampleRate);
            *static_cast<nanoem_u32_t *>(value) = frequency;
            *size = sizeof(frequency);
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_DURATION: {
            SInt64 length = 0;
            UInt32 s = sizeof(length);
            ExtAudioFileGetProperty(m_audioFileRef, kExtAudioFileProperty_FileLengthFrames, &s, &length);
            nanoem_frame_index_t duration =
                nanoem_frame_index_t(length / m_description.mSampleRate) * m_fps.unsignedIntValue;
            *static_cast<nanoem_frame_index_t *>(value) = duration;
            *size = sizeof(duration);
            break;
        }
        default:
            ret = NANOEM_APPLICATION_PLUGIN_RESULT_ERROR;
            *size = 0;
            break;
        }
        return ret;
    }
    nanoem_application_plugin_result_t
    videoFormatValue(nanoem_u32_t key, void *value, nanoem_rsize_t *size)
    {
        nanoem_application_plugin_result_t ret = NANOEM_APPLICATION_PLUGIN_RESULT_OK;
        switch (key) {
        case NANOEM_APPLICATION_PLUGIN_DECODER_VIDEO_FORMAT_WIDTH: {
            nanoem_u32_t width = nanoem_u32_t(m_movie.currentFrameImage.size.width);
            *static_cast<nanoem_u32_t *>(value) = width;
            *size = sizeof(width);
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_DECODER_VIDEO_FORMAT_HEIGHT: {
            nanoem_u32_t height = nanoem_u32_t(m_movie.currentFrameImage.size.height);
            *static_cast<nanoem_u32_t *>(value) = height;
            *size = sizeof(height);
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_DECODER_VIDEO_FORMAT_STRIDE: {
            nanoem_rsize_t stride = 0;
            *static_cast<nanoem_rsize_t *>(value) = stride;
            *size = sizeof(stride);
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_DECODER_VIDEO_FORMAT_DURATION: {
            nanoem_frame_index_t duration = 0;
            if (m_movie) {
                const QTTime &time = m_movie.duration;
                duration = nanoem_frame_index_t((time.timeValue / time.timeScale) * m_fps.unsignedIntValue);
            }
            *static_cast<nanoem_frame_index_t *>(value) = duration;
            *size = sizeof(duration);
            break;
        }
        default:
            ret = NANOEM_APPLICATION_PLUGIN_RESULT_ERROR;
            *size = 0;
            break;
        }
        return ret;
    }
    nanoem_application_plugin_result_t
    decodeAudioFrame(nanoem_frame_index_t current_frame_index, nanoem_u8_t **data, nanoem_rsize_t *size)
    {
        nanoem_application_plugin_result_t ret = NANOEM_APPLICATION_PLUGIN_RESULT_OK;
        AudioBuffer &audioBuffer = m_bufferList.mBuffers[0];
        UInt32 numFrames = audioBuffer.mDataByteSize / m_description.mBytesPerFrame,
               dataByteSize = audioBuffer.mDataByteSize;
        OSStatus status = ExtAudioFileSeek(
            m_audioFileRef, SInt64((m_description.mSampleRate / m_fps.doubleValue) * current_frame_index));
        if (status == kAudioServicesNoError) {
            status = ExtAudioFileRead(m_audioFileRef, &numFrames, &m_bufferList);
            if (status == kAudioServicesNoError && numFrames > 0) {
                nanoem_u8_t *ptr = *data = new nanoem_u8_t[dataByteSize];
                memcpy(ptr, audioBuffer.mData, dataByteSize);
                *size = dataByteSize;
            }
            else {
                m_error = [[NSError alloc] initWithDomain:NSOSStatusErrorDomain code:status userInfo:nil];
                ret = NANOEM_APPLICATION_PLUGIN_RESULT_ERROR;
                *data = NULL;
                *size = 0;
            }
        }
        else {
            m_error = [[NSError alloc] initWithDomain:NSOSStatusErrorDomain code:status userInfo:nil];
            ret = NANOEM_APPLICATION_PLUGIN_RESULT_ERROR;
            *data = NULL;
            *size = 0;
        }
        audioBuffer.mDataByteSize = dataByteSize;
        return ret;
    }
    nanoem_application_plugin_result_t
    decodeVideoFrame(nanoem_frame_index_t current_frame_index, nanoem_u8_t **data, nanoem_rsize_t *size)
    {
        NSDictionary *attributes =
            [NSDictionary dictionaryWithObjectsAndKeys:QTMovieFrameImageTypeNSImage, QTMovieFrameImageType, nil];
        NSError *error = nil;
        QTTime time = QTMakeTime(current_frame_index, m_fps.intValue);
        nanoem_application_plugin_result_t ret = NANOEM_APPLICATION_PLUGIN_RESULT_OK;
        void *ptr = [m_movie frameImageAtTime:time withAttributes:attributes error:&error];
        if (NSImage *nsImage = static_cast<NSImage *>(ptr)) {
            NSRect rect;
            CGImageRef image = [nsImage CGImageForProposedRect:&rect
                                                       context:[NSGraphicsContext currentContext]
                                                         hints:nil];
            nanoem_rsize_t width = CGImageGetWidth(image);
            nanoem_rsize_t height = CGImageGetHeight(image);
            CGDataProviderRef provider = CGImageGetDataProvider(image);
            CFDataRef inputData = CGDataProviderCopyData(provider);
            nanoem_rsize_t dataSize = *size = CFDataGetLength(inputData);
            nanoem_rsize_t stride = dataSize / height;
            nanoem_u8_t *outputImageData = *data = new nanoem_u8_t[dataSize];
            const nanoem_u8_t *inputImageData = CFDataGetBytePtr(inputData);
            for (nanoem_u32_t y = 0; y < height; y++) {
                const nanoem_u8_t *constImageData = inputImageData + y * stride;
                nanoem_u8_t *imageData = outputImageData + y * stride;
                for (nanoem_u32_t x = 0; x < width; x++) {
                    imageData[0] = constImageData[2];
                    imageData[1] = constImageData[1];
                    imageData[2] = constImageData[0];
                    imageData[3] = constImageData[3];
                    imageData += 4;
                    constImageData += 4;
                }
            }
            CFRelease(inputData);
        }
        else {
            m_error = [error retain];
            ret = NANOEM_APPLICATION_PLUGIN_RESULT_ERROR;
        }
        return ret;
    }
    void
    destroyAudioFrame(nanoem_frame_index_t /* current_frame_index */, nanoem_u8_t *data, nanoem_rsize_t /* size */)
    {
        delete[] data;
    }
    void
    destroyVideoFrame(nanoem_frame_index_t /* current_frame_index */, nanoem_u8_t *data, nanoem_rsize_t /* size */)
    {
        delete[] data;
    }
    const char *
    failureReason() const
    {
        return m_error.localizedFailureReason.UTF8String;
    }
    const char *
    recoverySuggestion() const
    {
        return m_error.localizedRecoverySuggestion.UTF8String;
    }
    nanoem_application_plugin_result_t
    close()
    {
        ExtAudioFileDispose(m_audioFileRef);
        m_audioFileRef = NULL;
        return NANOEM_APPLICATION_PLUGIN_RESULT_OK;
    }

    ExtAudioFileRef m_audioFileRef;
    AudioStreamBasicDescription m_description;
    AudioBufferList m_bufferList;
    QTMovie *m_movie;
    NSError *m_error;
    NSURL *m_audioFileURL;
    NSURL *m_videoFileURL;
    NSNumber *m_fps;
};

void
nanoemApplicationPluginEncoderInitialize(void)
{
}

nanoem_application_plugin_encoder_t *APIENTRY
nanoemApplicationPluginEncoderCreate()
{
    nanoem_application_plugin_encoder_t *opaque = new nanoem_application_plugin_encoder_t();
    return opaque;
}

nanoem_application_plugin_result_t APIENTRY
nanoemApplicationPluginEncoderOpen(nanoem_application_plugin_encoder_t *encoder)
{
    return nanoem_is_not_null(encoder) ? encoder->open() : NANOEM_APPLICATION_PLUGIN_RESULT_ERROR;
}

nanoem_application_plugin_result_t APIENTRY
nanoemApplicationPluginEncoderSetOption(
    nanoem_application_plugin_encoder_t *encoder, nanoem_u32_t key, const void *value, nanoem_rsize_t size)
{
    return nanoem_is_not_null(encoder) ? encoder->setOption(key, value, size) : NANOEM_APPLICATION_PLUGIN_RESULT_ERROR;
}

nanoem_application_plugin_result_t APIENTRY
nanoemApplicationPluginEncoderEncodeAudioFrame(nanoem_application_plugin_encoder_t *encoder,
    nanoem_frame_index_t current_frame_index, const nanoem_u8_t *data, nanoem_rsize_t size)
{
    return nanoem_is_not_null(encoder) ? encoder->encodeAudioFrame(current_frame_index, data, size)
                                       : NANOEM_APPLICATION_PLUGIN_RESULT_ERROR;
}

nanoem_application_plugin_result_t APIENTRY
nanoemApplicationPluginEncoderEncodeVideoFrame(nanoem_application_plugin_encoder_t *encoder,
    nanoem_frame_index_t current_frame_index, const nanoem_u8_t *data, nanoem_rsize_t size)
{
    return nanoem_is_not_null(encoder) ? encoder->encodeVideoFrame(current_frame_index, data, size)
                                       : NANOEM_APPLICATION_PLUGIN_RESULT_ERROR;
}

nanoem_application_plugin_result_t APIENTRY
nanoemApplicationPluginEncoderInterrupt(nanoem_application_plugin_encoder_t *encoder)
{
    return nanoem_is_not_null(encoder) ? encoder->interrupt() : NANOEM_APPLICATION_PLUGIN_RESULT_ERROR;
}

const char *const *APIENTRY
nanoemApplicationPluginEncoderGetAllAvailableVideoFormatExtensions(
    const nanoem_application_plugin_encoder_t * /* encoder */, nanoem_rsize_t *length)
{
    static const char *kFormatExtensions[] = { "mp4", "m4v", "mov" };
    *length = sizeof(kFormatExtensions) / sizeof(kFormatExtensions[0]);
    return kFormatExtensions;
}

const char *APIENTRY
nanoemApplicationPluginEncoderGetFailureReason(const nanoem_application_plugin_encoder_t *encoder)
{
    return nanoem_is_not_null(encoder) ? encoder->failureReason() : NULL;
}

const char *APIENTRY
nanoemApplicationPluginEncoderGetRecoverySuggestion(const nanoem_application_plugin_encoder_t *encoder)
{
    return nanoem_is_not_null(encoder) ? encoder->recoverySuggestion() : NULL;
}

nanoem_application_plugin_result_t APIENTRY
nanoemApplicationPluginEncoderClose(nanoem_application_plugin_encoder_t *encoder)
{
    return nanoem_is_not_null(encoder) ? encoder->close() : NANOEM_APPLICATION_PLUGIN_RESULT_ERROR;
}

void APIENTRY
nanoemApplicationPluginEncoderDestroy(nanoem_application_plugin_encoder_t *encoder)
{
    delete encoder;
}

void APIENTRY
nanoemApplicationPluginEncoderTerminate()
{
}

void APIENTRY
nanoemApplicationPluginDecoderInitialize(void)
{
}

nanoem_application_plugin_decoder_t *APIENTRY
nanoemApplicationPluginDecoderCreate()
{
    return new nanoem_application_plugin_decoder_t();
}

nanoem_application_plugin_result_t APIENTRY
nanoemApplicationPluginDecoderOpen(nanoem_application_plugin_decoder_t *decoder)
{
    return nanoem_is_not_null(decoder) ? decoder->open() : NANOEM_APPLICATION_PLUGIN_RESULT_ERROR;
}

nanoem_application_plugin_result_t APIENTRY
nanoemApplicationPluginDecoderSetOption(
    nanoem_application_plugin_decoder_t *decoder, nanoem_u32_t key, const void *value, nanoem_rsize_t size)
{
    return nanoem_is_not_null(decoder) ? decoder->setOption(key, value, size) : NANOEM_APPLICATION_PLUGIN_RESULT_ERROR;
}

nanoem_application_plugin_result_t APIENTRY
nanoemApplicationPluginDecoderGetAudioFormatValue(
    nanoem_application_plugin_decoder_t *decoder, nanoem_u32_t key, void *value, nanoem_rsize_t *size)
{
    return nanoem_is_not_null(decoder) ? decoder->audioFormatValue(key, value, size)
                                       : NANOEM_APPLICATION_PLUGIN_RESULT_ERROR;
}

nanoem_application_plugin_result_t APIENTRY
nanoemApplicationPluginDecoderGetVideoFormatValue(
    nanoem_application_plugin_decoder_t *decoder, nanoem_u32_t key, void *value, nanoem_rsize_t *size)
{
    return nanoem_is_not_null(decoder) ? decoder->videoFormatValue(key, value, size)
                                       : NANOEM_APPLICATION_PLUGIN_RESULT_ERROR;
}

nanoem_application_plugin_result_t APIENTRY
nanoemApplicationPluginDecoderDecodeAudioFrame(nanoem_application_plugin_decoder_t *decoder,
    nanoem_frame_index_t current_frame_index, nanoem_u8_t **data, nanoem_rsize_t *size)
{
    return nanoem_is_not_null(decoder) ? decoder->decodeAudioFrame(current_frame_index, data, size)
                                       : NANOEM_APPLICATION_PLUGIN_RESULT_ERROR;
}

nanoem_application_plugin_result_t APIENTRY
nanoemApplicationPluginDecoderDecodeVideoFrame(nanoem_application_plugin_decoder_t *decoder,
    nanoem_frame_index_t current_frame_index, nanoem_u8_t **data, nanoem_rsize_t *size)
{
    return nanoem_is_not_null(decoder) ? decoder->decodeVideoFrame(current_frame_index, data, size)
                                       : NANOEM_APPLICATION_PLUGIN_RESULT_ERROR;
}

void APIENTRY
nanoemApplicationPluginDecoderDestroyAudioFrame(nanoem_application_plugin_decoder_t *decoder,
    nanoem_frame_index_t current_frame_index, nanoem_u8_t *data, nanoem_rsize_t size)
{
    if (nanoem_is_not_null(decoder)) {
        decoder->destroyAudioFrame(current_frame_index, data, size);
    }
}

void APIENTRY
nanoemApplicationPluginDecoderDestroyVideoFrame(nanoem_application_plugin_decoder_t *decoder,
    nanoem_frame_index_t current_frame_index, nanoem_u8_t *data, nanoem_rsize_t size)
{
    if (nanoem_is_not_null(decoder)) {
        decoder->destroyVideoFrame(current_frame_index, data, size);
    }
}

const char *const *APIENTRY
nanoemApplicationPluginDecoderGetAllAvailableAudioFormatExtensions(
    nanoem_application_plugin_decoder_t * /* decoder */, nanoem_rsize_t *length)
{
    static const char *kFormatExtensions[] = { "mp3", "m4a", "aac", "caf" };
    *length = sizeof(kFormatExtensions) / sizeof(kFormatExtensions[0]);
    return kFormatExtensions;
}

const char *const *APIENTRY
nanoemApplicationPluginDecoderGetAllAvailableVideoFormatExtensions(
    nanoem_application_plugin_decoder_t * /* decoder */, nanoem_rsize_t *length)
{
    static const char *kFormatExtensions[] = { "mp4", "m4v", "mov" };
    *length = sizeof(kFormatExtensions) / sizeof(kFormatExtensions[0]);
    return kFormatExtensions;
}

const char *APIENTRY
nanoemApplicationPluginDecoderGetFailureReason(nanoem_application_plugin_decoder_t *decoder)
{
    return nanoem_is_not_null(decoder) ? decoder->failureReason() : NULL;
}

const char *APIENTRY
nanoemApplicationPluginDecoderGetRecoverySuggestion(nanoem_application_plugin_decoder_t *decoder)
{
    return nanoem_is_not_null(decoder) ? decoder->recoverySuggestion() : NULL;
}

nanoem_application_plugin_result_t APIENTRY
nanoemApplicationPluginDecoderClose(nanoem_application_plugin_decoder_t *decoder)
{
    return nanoem_is_not_null(decoder) ? decoder->close() : NANOEM_APPLICATION_PLUGIN_RESULT_ERROR;
}

void APIENTRY
nanoemApplicationPluginDecoderDestroy(nanoem_application_plugin_decoder_t *decoder)
{
    delete decoder;
}

void APIENTRY
nanoemApplicationPluginDecoderTerminate()
{
}
