/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/sdk/Decoder.h"
#include "emapp/sdk/Encoder.h"

#import <AVFoundation/AVFoundation.h>
#import <AudioToolbox/AudioToolbox.h>

namespace {

using namespace nanoem::application::plugin;

struct AVFoundationEncoder {
    struct DataTransferProtocol {
        DataTransferProtocol(nanoem_frame_index_t frameIndex, const nanoem_u8_t *bytes, nanoem_u32_t size)
            : m_frameIndex(frameIndex)
            , m_bytes(nullptr)
            , m_size(size)
        {
            m_bytes = new nanoem_u8_t[size];
            memcpy(m_bytes, bytes, size);
        }
        ~DataTransferProtocol()
        {
        }
        void
        writeAudioFrame(AVAssetWriterInput *audioInput, CMAudioFormatDescriptionRef formatDescription,
            nanoem_frame_index_t duration, int fps) const
        {
            const CMTime &presentationTime = CMTimeMake(m_frameIndex, fps);
            nanoem_u32_t dataSize = m_size;
            OSStatus oss;
            if (void *ptr = malloc(dataSize)) {
                CMBlockBufferRef blockBuffer;
                memcpy(ptr, m_bytes, dataSize);
                oss = CMBlockBufferCreateWithMemoryBlock(kCFAllocatorDefault, ptr, dataSize, kCFAllocatorMalloc,
                    nullptr, 0, dataSize, kCMBlockBufferAssureMemoryNowFlag, &blockBuffer);
                if (oss == kCMBlockBufferNoErr) {
                    const AudioStreamBasicDescription *description =
                        CMAudioFormatDescriptionGetStreamBasicDescription(formatDescription);
                    CMItemCount numSamples = CMItemCount(dataSize / description->mBytesPerFrame);
                    CMSampleBufferRef sampleBuffer;
                    CMSampleTimingInfo info;
                    memset(&info, 0, sizeof(info));
                    info.decodeTimeStamp = kCMTimeInvalid;
                    info.presentationTimeStamp = presentationTime;
                    info.duration = CMTimeMake(duration, fps);
                    size_t sampleSizeArray = description->mBytesPerFrame;
                    oss = CMSampleBufferCreate(kCFAllocatorDefault, blockBuffer, YES, nullptr, nullptr,
                        formatDescription, numSamples, 1, &info, 1, &sampleSizeArray, &sampleBuffer);
                    if (oss == kCMBlockBufferNoErr) {
                        [audioInput appendSampleBuffer:sampleBuffer];
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
        void
        writeVideoFrame(AVAssetWriterInputPixelBufferAdaptor *pixelBufferAdaptor, nanoem_u32_t width,
            nanoem_u32_t height, int fps, bool yflip) const
        {
            CVReturn ret;
            const CMTime &presentationTime = CMTimeMake(m_frameIndex, fps);
            if (CVPixelBufferPoolRef pool = pixelBufferAdaptor.pixelBufferPool) {
                CVPixelBufferRef pixelBuffer;
                ret = CVPixelBufferPoolCreatePixelBuffer(kCFAllocatorDefault, pool, &pixelBuffer);
                if (ret != kCVReturnSuccess) {
                    return;
                }
                ret = CVPixelBufferLockBaseAddress(pixelBuffer, 0);
                if (ret != kCVReturnSuccess) {
                    CVPixelBufferRelease(pixelBuffer);
                    return;
                }
                if (nanoem_u8_t *pixels = static_cast<nanoem_u8_t *>(CVPixelBufferGetBaseAddress(pixelBuffer))) {
                    if (yflip) {
                        const nanoem_u32_t stride = width * 4;
                        const nanoem_u8_t *ptr = m_bytes + stride * height - stride;
                        for (nanoem_u32_t y = 0; y < height; ++y) {
                            memcpy(pixels + y * stride, ptr, stride);
                            ptr -= stride;
                        }
                    }
                    else {
                        memcpy(pixels, m_bytes, m_size);
                    }
                    for (size_t i = 0, size = m_size / sizeof(nanoem_u32_t); i < size; i++) {
                        nanoem_u32_t *ptr = reinterpret_cast<nanoem_u32_t *>(pixels) + i, v = *ptr;
                        *ptr = 0 | ((v & 0x000000ff) << 16) | (v & 0x0000ff00) | ((v & 0x00ff0000) >> 16) |
                            (v & 0xff000000);
                    }
                }
                ret = CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);
                if (ret != kCVReturnSuccess) {
                    CVPixelBufferRelease(pixelBuffer);
                    return;
                }
                [pixelBufferAdaptor appendPixelBuffer:pixelBuffer withPresentationTime:presentationTime];
                CVPixelBufferRelease(pixelBuffer);
            }
        }
        void
        destroy() const
        {
            delete[] m_bytes;
            m_bytes = nullptr;
        }
        nanoem_frame_index_t m_frameIndex;
        mutable nanoem_u8_t *m_bytes;
        nanoem_u32_t m_size;
    };

    static NSString *
    determineVideoFileType(NSURL *fileURL)
    {
        NSString *extension = [fileURL pathExtension];
        NSString *fileType;
        if ([extension isEqualToString:@"mp4"]) {
            fileType = AVFileTypeMPEG4;
        }
        else if ([extension isEqualToString:@"m4v"]) {
            fileType = AVFileTypeAppleM4V;
        }
        else if ([extension isEqualToString:@"mov"]) {
            fileType = AVFileTypeQuickTimeMovie;
        }
        else {
            fileType = AVFileTypeMPEG4;
        }
        return fileType;
    }

    AVFoundationEncoder()
        : m_writer(nil)
        , m_audioFrameQueue(nil)
        , m_videoFrameQueue(nil)
        , m_error(nil)
        , m_duration(nil)
        , m_fps(nil)
        , m_numFrequency(nil)
        , m_numChannels(nil)
        , m_numBits(nil)
        , m_width(nil)
        , m_height(nil)
        , m_format(nil)
        , m_yflip(nil)
        , m_audioEncoderQueue(nullptr)
        , m_videoEncoderQueue(nullptr)
        , m_audioQueueSema(nullptr)
        , m_videoQueueSema(nullptr)
        , m_destructionSema(nullptr)
    {
        m_audioFrameQueue = [[NSMutableArray alloc] init];
        m_videoFrameQueue = [[NSMutableArray alloc] init];
        m_audioEncoderQueue =
            dispatch_queue_create("com.github.nanoem.plugin.encoder.AVFoundation.AudioEncoderQueue", nullptr);
        m_videoEncoderQueue =
            dispatch_queue_create("com.github.nanoem.plugin.encoder.AVFoundation.VideoEncoderQueue", nullptr);
        m_audioQueueSema = dispatch_semaphore_create(0);
        m_videoQueueSema = dispatch_semaphore_create(0);
        m_audioCompletionSema = dispatch_semaphore_create(0);
        m_videoCompletionSema = dispatch_semaphore_create(0);
        m_destructionSema = dispatch_semaphore_create(0);
    }
    ~AVFoundationEncoder()
    {
    }

    int
    open(const char *filePath, nanoem_application_plugin_status_t *status)
    {
        NSError *error;
        NSURL *fileURL = [[NSURL alloc] initFileURLWithPath:[[NSString alloc] initWithUTF8String:filePath]];
        NSString *fileType = determineVideoFileType(fileURL);
        AVAssetWriter *writer = [[AVAssetWriter alloc] initWithURL:fileURL fileType:fileType error:&error];
        if (error == nil) {
            AudioChannelLayout channelLayout;
            memset(&channelLayout, 0, sizeof(channelLayout));
            AVAssetWriterInput *audioInput = nil;
            NSMutableDictionary *outputVideoSettings =
                [[NSMutableDictionary alloc] initWithObjectsAndKeys:m_width, AVVideoWidthKey, m_height,
                                             AVVideoHeightKey, AVVideoCodecH264, AVVideoCodecKey, nil];
            AVAssetWriterInput *videoInput = [[AVAssetWriterInput alloc] initWithMediaType:AVMediaTypeVideo
                                                                            outputSettings:outputVideoSettings];
            if (m_numChannels.integerValue > 0 && m_numFrequency.integerValue > 0) {
                channelLayout.mChannelLayoutTag = kAudioChannelLayoutTag_Stereo;
                NSMutableDictionary *outputAudioSettings = [[NSMutableDictionary alloc]
                    initWithObjectsAndKeys:[[NSNumber alloc] initWithUnsignedInt:kAudioFormatMPEG4AAC], AVFormatIDKey,
                    [[NSNumber alloc] initWithUnsignedInt:256000], AVEncoderBitRateKey, m_numFrequency, AVSampleRateKey,
                    m_numChannels, AVNumberOfChannelsKey,
                    [[NSData alloc] initWithBytes:&channelLayout length:sizeof(channelLayout)], AVChannelLayoutKey,
                    nil];
                @try {
                    audioInput = [[AVAssetWriterInput alloc] initWithMediaType:AVMediaTypeAudio
                                                                outputSettings:outputAudioSettings];
                    if ([writer canAddInput:audioInput]) {
                        [writer addInput:audioInput];
                    }
                } @catch (NSException *exception) {
                    NSDictionary *userInfo = [[NSDictionary alloc]
                        initWithObjectsAndKeys:exception.reason, NSLocalizedFailureReasonErrorKey, nil];
                    error = [[NSError alloc] initWithDomain:NSCocoaErrorDomain code:0 userInfo:userInfo];
                }
            }
            else {
                dispatch_semaphore_signal(m_audioCompletionSema);
            }
            if (error != nil) {
                m_error = error;
            }
            else if ([writer canAddInput:videoInput]) {
                [writer addInput:videoInput];
                NSNumber *stride = [[NSNumber alloc] initWithInt:m_width.intValue * 4];
                AudioStreamBasicDescription streamBasicDescription;
                memset(&streamBasicDescription, 0, sizeof(streamBasicDescription));
                streamBasicDescription.mSampleRate = [m_numFrequency floatValue];
                streamBasicDescription.mFormatID = kAudioFormatLinearPCM;
                streamBasicDescription.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
                streamBasicDescription.mFramesPerPacket = 1;
                streamBasicDescription.mChannelsPerFrame = [m_numChannels unsignedIntValue];
                streamBasicDescription.mBitsPerChannel = [m_numBits unsignedIntValue];
                streamBasicDescription.mBytesPerFrame =
                    (streamBasicDescription.mBitsPerChannel / 8) * streamBasicDescription.mChannelsPerFrame;
                streamBasicDescription.mBytesPerPacket =
                    streamBasicDescription.mBytesPerFrame * streamBasicDescription.mFramesPerPacket;
                __block CMAudioFormatDescriptionRef formatDescription;
                CMAudioFormatDescriptionCreate(kCFAllocatorDefault, &streamBasicDescription, sizeof(channelLayout),
                    &channelLayout, 0, nullptr, nullptr, &formatDescription);
                NSDictionary *adaptorOptions = [[NSDictionary alloc]
                    initWithObjectsAndKeys:[[NSNumber alloc] initWithUnsignedLongLong:kCVPixelFormatType_32BGRA],
                    kCVPixelBufferPixelFormatTypeKey, stride, kCVPixelBufferBytesPerRowAlignmentKey, m_height,
                    kCVPixelBufferHeightKey, m_width, kCVPixelBufferWidthKey, nil];
                __block AVAssetWriterInputPixelBufferAdaptor *pixelBufferAdaptor =
                    [[AVAssetWriterInputPixelBufferAdaptor alloc] initWithAssetWriterInput:videoInput
                                                               sourcePixelBufferAttributes:adaptorOptions];
                [[NSFileManager defaultManager] removeItemAtURL:fileURL error:nil];
                audioInput.expectsMediaDataInRealTime = NO;
                videoInput.expectsMediaDataInRealTime = NO;
                if ([writer startWriting]) {
                    m_writer = writer;
                    [writer startSessionAtSourceTime:kCMTimeZero];
                    [audioInput requestMediaDataWhenReadyOnQueue:m_audioEncoderQueue
                                                      usingBlock:^() {
                                                          writeAllAudioFramesFromQueue(audioInput, formatDescription);
                                                      }];
                    [videoInput requestMediaDataWhenReadyOnQueue:m_videoEncoderQueue
                                                      usingBlock:^() {
                                                          writeAllVideoFramesFromQueue(videoInput, pixelBufferAdaptor);
                                                      }];
                }
                else {
                    m_error = [writer error];
                }
            }
        }
        else {
            m_error = error;
        }
        if (status && m_error) {
            nanoem_application_plugin_status_assign_error(status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_REFER_REASON);
        }
        return error == nil;
    }
    void
    writeAllAudioFramesFromQueue(AVAssetWriterInput *audioInput, CMAudioFormatDescriptionRef formatDescription)
    {
        while (audioInput.readyForMoreMediaData) {
            NSUInteger count = 0;
            @synchronized(m_audioFrameQueue) {
                count = m_audioFrameQueue.count;
            }
            NSData *data = nil;
            if (count == 0) {
                break;
            }
            else if (count > 0) {
                @synchronized(m_audioFrameQueue) {
                    data = [m_audioFrameQueue objectAtIndex:0];
                    [m_audioFrameQueue removeObjectAtIndex:0];
                }
            }
            if (!data || data.length == 0) {
                finishEncoding(audioInput, formatDescription);
                break;
            }
            else if (const DataTransferProtocol *protocol = static_cast<const DataTransferProtocol *>(data.bytes)) {
                protocol->writeAudioFrame(audioInput, formatDescription, m_duration.longLongValue, m_fps.intValue);
                protocol->destroy();
            }
        }
    }
    void
    writeAllVideoFramesFromQueue(
        AVAssetWriterInput *videoInput, AVAssetWriterInputPixelBufferAdaptor *pixelBufferAdaptor)
    {
        while (videoInput.readyForMoreMediaData) {
            NSUInteger count = 0;
            @synchronized(m_videoFrameQueue) {
                count = m_videoFrameQueue.count;
            }
            NSData *data = nil;
            if (count == 0) {
                break;
            }
            else if (count > 0) {
                @synchronized(m_videoFrameQueue) {
                    data = [m_videoFrameQueue objectAtIndex:0];
                    [m_videoFrameQueue removeObjectAtIndex:0];
                }
            }
            if (!data || data.length == 0) {
                finishEncoding(videoInput, pixelBufferAdaptor);
                break;
            }
            else if (const DataTransferProtocol *protocol = static_cast<const DataTransferProtocol *>(data.bytes)) {
                protocol->writeVideoFrame(pixelBufferAdaptor, m_width.unsignedIntValue, m_height.unsignedIntValue,
                    m_fps.intValue, m_yflip.boolValue);
                protocol->destroy();
            }
        }
    }
    void
    finishEncoding(AVAssetWriterInput *audioInput, CMAudioFormatDescriptionRef formatDescription)
    {
        [audioInput markAsFinished];
        @synchronized(m_audioFrameQueue) {
            for (NSData *data in m_audioFrameQueue) {
                if (data.length > 0) {
                    if (const DataTransferProtocol *protocol = static_cast<const DataTransferProtocol *>(data.bytes)) {
                        protocol->destroy();
                    }
                }
            }
            [m_audioFrameQueue removeAllObjects];
        }
        CFRelease(formatDescription);
        dispatch_semaphore_signal(m_audioCompletionSema);
    }
    void
    finishEncoding(AVAssetWriterInput *videoInput, AVAssetWriterInputPixelBufferAdaptor *pixelBufferAdaptor)
    {
        [videoInput markAsFinished];
        @synchronized(m_videoFrameQueue) {
            for (NSData *data in m_videoFrameQueue) {
                if (data.length > 0) {
                    if (const DataTransferProtocol *protocol = static_cast<const DataTransferProtocol *>(data.bytes)) {
                        protocol->destroy();
                    }
                }
            }
            [m_videoFrameQueue removeAllObjects];
        }
        CVPixelBufferPoolRelease(pixelBufferAdaptor.pixelBufferPool);
        dispatch_semaphore_signal(m_videoCompletionSema);
    }
    void
    setOption(nanoem_u32_t key, const void *value, nanoem_u32_t size, nanoem_application_plugin_status_t *status)
    {
        switch (key) {
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_FPS: {
            if (Inline::validateArgument<int>(value, size, status)) {
                m_fps = [[NSNumber alloc] initWithUnsignedInt:*static_cast<const nanoem_u32_t *>(value)];
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_DURATION: {
            if (Inline::validateArgument<nanoem_frame_index_t>(value, size, status)) {
                m_duration = [[NSNumber alloc] initWithLong:*static_cast<const nanoem_frame_index_t *>(value)];
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_LOCATION: {
            /* unused */
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_AUDIO_NUM_FREQUENCY: {
            if (Inline::validateArgument<int>(value, size, status)) {
                m_numFrequency = [[NSNumber alloc] initWithInt:*static_cast<const int *>(value)];
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_AUDIO_NUM_CHANNELS: {
            if (Inline::validateArgument<int>(value, size, status)) {
                m_numChannels = [[NSNumber alloc] initWithInt:*static_cast<const int *>(value)];
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_AUDIO_NUM_BITS: {
            if (Inline::validateArgument<int>(value, size, status)) {
                m_numBits = [[NSNumber alloc] initWithInt:*static_cast<const int *>(value)];
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_WIDTH: {
            if (Inline::validateArgument<int>(value, size, status)) {
                m_width = [[NSNumber alloc] initWithUnsignedInt:*static_cast<const nanoem_u32_t *>(value)];
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_HEIGHT: {
            if (Inline::validateArgument<int>(value, size, status)) {
                m_height = [[NSNumber alloc] initWithUnsignedInt:*static_cast<const nanoem_u32_t *>(value)];
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_YFLIP: {
            if (Inline::validateArgument<int>(value, size, status)) {
                m_yflip = [[NSNumber alloc] initWithBool:*static_cast<const nanoem_u32_t *>(value) != 0];
            }
            break;
        }
        default:
            nanoem_application_plugin_status_assign_error(
                status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_UNKNOWN_OPTION);
            break;
        }
    }
    void
    encodeAudioFrame(nanoem_frame_index_t currentFrameIndex, const nanoem_u8_t *data, nanoem_u32_t size,
        nanoem_application_plugin_status_t *status)
    {
        DataTransferProtocol protocol(currentFrameIndex, data, size);
        NSData *bytes = [[NSData alloc] initWithBytes:&protocol length:sizeof(protocol)];
        @synchronized(m_audioFrameQueue) {
            [m_audioFrameQueue addObject:bytes];
        }
        dispatch_semaphore_signal(m_audioQueueSema);
        nanoem_application_plugin_status_assign_success(status);
    }
    void
    encodeVideoFrame(nanoem_frame_index_t currentFrameIndex, const nanoem_u8_t *data, nanoem_u32_t size,
        nanoem_application_plugin_status_t *status)
    {
        DataTransferProtocol protocol(currentFrameIndex, data, size);
        NSData *bytes = [[NSData alloc] initWithBytes:&protocol length:sizeof(protocol)];
        @synchronized(m_videoFrameQueue) {
            [m_videoFrameQueue addObject:bytes];
        }
        dispatch_semaphore_signal(m_videoQueueSema);
        nanoem_application_plugin_status_assign_success(status);
    }
    void
    interrupt(nanoem_application_plugin_status_t *status)
    {
        if (m_writer) {
            NSData *poison = [[NSData alloc] init];
            @synchronized(m_audioFrameQueue) {
                [m_audioFrameQueue insertObject:poison atIndex:0];
            }
            @synchronized(m_videoFrameQueue) {
                [m_videoFrameQueue insertObject:poison atIndex:0];
            }
            waitForCompletion();
        }
        else {
            dispatch_semaphore_signal(m_destructionSema);
        }
        nanoem_application_plugin_status_assign_success(status);
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

    int
    close(nanoem_application_plugin_status_t *status)
    {
        if (m_writer) {
            NSData *poison = [[NSData alloc] init];
            @synchronized(m_audioFrameQueue) {
                [m_audioFrameQueue addObject:poison];
            }
            @synchronized(m_videoFrameQueue) {
                [m_videoFrameQueue addObject:poison];
            }
            waitForCompletion();
        }
        else {
            dispatch_semaphore_signal(m_destructionSema);
        }
        nanoem_application_plugin_status_assign_success(status);
        return 1;
    }
    void
    waitForCompletion()
    {
        dispatch_semaphore_signal(m_audioQueueSema);
        dispatch_semaphore_signal(m_videoQueueSema);
        dispatch_semaphore_wait(m_audioCompletionSema, DISPATCH_TIME_FOREVER);
        dispatch_semaphore_wait(m_videoCompletionSema, DISPATCH_TIME_FOREVER);
        [m_writer finishWritingWithCompletionHandler:^() {
            dispatch_semaphore_signal(m_destructionSema);
        }];
    }
    void
    destroy()
    {
        if (m_writer) {
            dispatch_semaphore_wait(m_destructionSema, DISPATCH_TIME_FOREVER);
        }
    }

    AVAssetWriter *m_writer;
    NSMutableArray *m_audioFrameQueue;
    NSMutableArray *m_videoFrameQueue;
    NSError *m_error;
    NSNumber *m_duration;
    NSNumber *m_fps;
    NSNumber *m_numFrequency;
    NSNumber *m_numChannels;
    NSNumber *m_numBits;
    NSNumber *m_width;
    NSNumber *m_height;
    NSNumber *m_format;
    NSNumber *m_yflip;
    dispatch_queue_t m_audioEncoderQueue;
    dispatch_queue_t m_videoEncoderQueue;
    dispatch_semaphore_t m_audioQueueSema;
    dispatch_semaphore_t m_videoQueueSema;
    dispatch_semaphore_t m_audioCompletionSema;
    dispatch_semaphore_t m_videoCompletionSema;
    dispatch_semaphore_t m_destructionSema;
};

struct AVFoundationDecoder {
    AVFoundationDecoder()
        : m_audioFileRef(nullptr)
        , m_assetImageGenerator(nil)
        , m_videoTrack(nil)
        , m_error(nil)
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
    ~AVFoundationDecoder()
    {
        if (void *ptr = m_bufferList.mBuffers[0].mData) {
            free(ptr);
        }
    }

    int
    open(const char *filePath, nanoem_application_plugin_status_t *status)
    {
        NSError *error = nil;
        if (m_audioFileRef) {
            ExtAudioFileDispose(m_audioFileRef);
            m_audioFileRef = nullptr;
        }
        NSURL *audioFileURL = [[NSURL alloc] initFileURLWithPath:[[NSString alloc] initWithUTF8String:filePath]];
        OSStatus oss = ExtAudioFileOpenURL((__bridge CFURLRef) audioFileURL, &m_audioFileRef);
        if (oss == kAudioServicesNoError) {
            oss = ExtAudioFileSetProperty(
                m_audioFileRef, kExtAudioFileProperty_ClientDataFormat, sizeof(m_description), &m_description);
            if (oss != kAudioServicesNoError) {
                error = [[NSError alloc] initWithDomain:NSOSStatusErrorDomain code:oss userInfo:nil];
            }
        }
        else {
            error = [[NSError alloc] initWithDomain:NSOSStatusErrorDomain code:oss userInfo:nil];
        }
        if (m_videoTrack) {
            m_assetImageGenerator = nil;
            m_videoTrack = nil;
        }
        NSURL *videoFileURL = [[NSURL alloc] initFileURLWithPath:[[NSString alloc] initWithUTF8String:filePath]];
        AVAsset *asset = [AVAsset assetWithURL:videoFileURL];
        NSArray *tracks = [asset tracksWithMediaType:AVMediaTypeVideo];
        if (tracks.count > 0) {
            m_videoTrack = tracks.firstObject;
            m_assetImageGenerator = [[AVAssetImageGenerator alloc] initWithAsset:asset];
            m_assetImageGenerator.requestedTimeToleranceAfter = kCMTimeZero;
            m_assetImageGenerator.requestedTimeToleranceBefore = kCMTimeZero;
        }
        if (!m_audioFileRef && !m_videoTrack && error) {
            nanoem_application_plugin_status_assign_error(status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_REFER_REASON);
            m_error = error;
        }
        return m_error == nil;
    }
    void
    setOption(nanoem_u32_t key, const void *value, nanoem_u32_t size, nanoem_application_plugin_status_t *status)
    {
        switch (key) {
        case NANOEM_APPLICATION_PLUGIN_DECODER_OPTION_FPS: {
            if (Inline::validateArgument<nanoem_u32_t>(value, size, status)) {
                m_fps = [[NSNumber alloc] initWithUnsignedInt:*static_cast<const nanoem_u32_t *>(value)];
                AudioBuffer &audioBuffer = m_bufferList.mBuffers[0];
                if (audioBuffer.mData) {
                    free(audioBuffer.mData);
                }
                audioBuffer.mDataByteSize =
                    nanoem_u32_t((1.0f / m_fps.floatValue) * m_description.mSampleRate * m_description.mBytesPerFrame);
                audioBuffer.mData = malloc(audioBuffer.mDataByteSize);
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_DECODER_OPTION_AUDIO_LOCATION: {
            /* unused */
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_DECODER_OPTION_VIDEO_LOCATION: {
            /* unused */
            break;
        }
        default:
            nanoem_application_plugin_status_assign_error(
                status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_UNKNOWN_OPTION);
            break;
        }
    }
    void
    audioFormatValue(nanoem_u32_t key, void *value, nanoem_u32_t *size, nanoem_application_plugin_status_t *status)
    {
        switch (key) {
        case NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_NUM_BITS: {
            Inline::assignOption(m_description.mBitsPerChannel, value, size, status);
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_NUM_CHANNELS: {
            Inline::assignOption(m_description.mChannelsPerFrame, value, size, status);
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_FREQUENCY: {
            const uint32_t sampleRate(m_description.mSampleRate);
            Inline::assignOption(sampleRate, value, size, status);
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_DURATION: {
            SInt64 length = 0;
            UInt32 s = sizeof(length);
            ExtAudioFileGetProperty(m_audioFileRef, kExtAudioFileProperty_FileLengthFrames, &s, &length);
            nanoem_frame_index_t duration =
                nanoem_frame_index_t(length / m_description.mSampleRate) * m_fps.unsignedIntValue;
            Inline::assignOption(duration, value, size, status);
            break;
        }
        default:
            nanoem_application_plugin_status_assign_error(
                status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_UNKNOWN_OPTION);
            if (size) {
                *size = 0;
            }
            break;
        }
    }
    void
    videoFormatValue(nanoem_u32_t key, void *value, nanoem_u32_t *size, nanoem_application_plugin_status_t *status)
    {
        switch (key) {
        case NANOEM_APPLICATION_PLUGIN_DECODER_VIDEO_FORMAT_WIDTH: {
            Inline::assignOption(static_cast<nanoem_u32_t>(m_videoTrack.naturalSize.width), value, size, status);
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_DECODER_VIDEO_FORMAT_HEIGHT: {
            Inline::assignOption(static_cast<nanoem_u32_t>(m_videoTrack.naturalSize.height), value, size, status);
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_DECODER_VIDEO_FORMAT_STRIDE: {
            nanoem_u32_t stride = nanoem_u32_t(m_videoTrack.totalSampleDataLength / m_videoTrack.naturalSize.height);
            Inline::assignOption(stride, value, size, status);
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_DECODER_VIDEO_FORMAT_DURATION: {
            nanoem_frame_index_t duration = 0;
            if (m_videoTrack) {
                const CMTime time = m_videoTrack.timeRange.duration;
                duration = nanoem_frame_index_t((time.value / time.timescale) * m_fps.unsignedIntValue);
            }
            Inline::assignOption(duration, value, size, status);
            break;
        }
        default:
            nanoem_application_plugin_status_assign_error(
                status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_UNKNOWN_OPTION);
            if (size) {
                *size = 0;
            }
            break;
        }
    }
    void
    decodeAudioFrame(nanoem_frame_index_t currentFrameIndex, nanoem_u8_t **data, nanoem_u32_t *size,
        nanoem_application_plugin_status_t *status)
    {
        AudioBuffer &audioBuffer = m_bufferList.mBuffers[0];
        UInt32 numFrames = audioBuffer.mDataByteSize / m_description.mBytesPerFrame,
               dataByteSize = audioBuffer.mDataByteSize;
        OSStatus oss = ExtAudioFileSeek(
            m_audioFileRef, SInt64((m_description.mSampleRate / m_fps.doubleValue) * currentFrameIndex));
        if (oss == kAudioServicesNoError) {
            oss = ExtAudioFileRead(m_audioFileRef, &numFrames, &m_bufferList);
            if (oss == kAudioServicesNoError) {
                /* numFrames == 0 indicates EOF */
                if (numFrames > 0) {
                    nanoem_u8_t *ptr = *data = new nanoem_u8_t[dataByteSize];
                    memcpy(ptr, audioBuffer.mData, dataByteSize);
                    *size = dataByteSize;
                }
            }
            else {
                m_error = [[NSError alloc] initWithDomain:NSOSStatusErrorDomain code:oss userInfo:nil];
                *data = nullptr;
                *size = 0;
            }
        }
        else {
            m_error = [[NSError alloc] initWithDomain:NSOSStatusErrorDomain code:oss userInfo:nil];
            *data = nullptr;
            *size = 0;
        }
        if (m_error) {
            nanoem_application_plugin_status_assign_error(status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_REFER_REASON);
        }
        audioBuffer.mDataByteSize = dataByteSize;
    }
    void
    decodeVideoFrame(nanoem_frame_index_t currentFrameIndex, nanoem_u8_t **data, nanoem_u32_t *size,
        nanoem_application_plugin_status_t *status)
    {
        CMTime time = CMTimeMake(currentFrameIndex, [m_fps intValue]), actualTime;
        NSError *error;
        CGImageRef image = [m_assetImageGenerator copyCGImageAtTime:time actualTime:&actualTime error:&error];
        if (error == nil) {
            nanoem_u32_t width = CGImageGetWidth(image);
            nanoem_u32_t height = CGImageGetHeight(image);
            CGDataProviderRef provider = CGImageGetDataProvider(image);
            CFDataRef inputData = CGDataProviderCopyData(provider);
            nanoem_u32_t dataSize = *size = CFDataGetLength(inputData);
            nanoem_u32_t stride = dataSize / height;
            nanoem_u8_t *outputImageData = *data = new nanoem_u8_t[dataSize];
            const nanoem_u8_t *inputImageData = CFDataGetBytePtr(inputData);
            for (nanoem_u32_t y = 0; y < height; y++) {
                const nanoem_u8_t *constImageData = inputImageData + y * stride;
                nanoem_u8_t *imageData = outputImageData + y * stride;
                for (nanoem_u32_t x = 0; x < width; x++) {
                    imageData[0] = constImageData[3];
                    imageData[1] = constImageData[2];
                    imageData[2] = constImageData[1];
                    imageData[3] = constImageData[0];
                    imageData += 4;
                    constImageData += 4;
                }
            }
            CFRelease(inputData);
        }
        else {
            m_error = error;
        }
        if (m_error) {
            nanoem_application_plugin_status_assign_error(status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_REFER_REASON);
        }
        CGImageRelease(image);
    }
    void
    destroyAudioFrame(nanoem_frame_index_t /* currentFrameIndex */, nanoem_u8_t *data, nanoem_u32_t /* size */)
    {
        delete[] data;
    }
    void
    destroyVideoFrame(nanoem_frame_index_t /* currentFrameIndex */, nanoem_u8_t *data, nanoem_u32_t /* size */)
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
    int
    close(nanoem_application_plugin_status_t *status)
    {
        OSStatus oss = ExtAudioFileDispose(m_audioFileRef);
        m_audioFileRef = nullptr;
        if (status) {
            if (oss == kAudioServicesNoError) {
                nanoem_application_plugin_status_assign_success(status);
            }
            else {
                nanoem_application_plugin_status_assign_error(
                    status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_REFER_REASON);
            }
        }
        m_videoTrack = nil;
        m_assetImageGenerator = nil;
        return 1;
    }

    ExtAudioFileRef m_audioFileRef;
    AudioStreamBasicDescription m_description;
    AudioBufferList m_bufferList;
    AVAssetImageGenerator *m_assetImageGenerator;
    AVAssetTrack *m_videoTrack;
    NSError *m_error;
    NSNumber *m_fps;
};

} /* namespace anonymous */

struct nanoem_application_plugin_decoder_t : AVFoundationDecoder { };

struct nanoem_application_plugin_encoder_t : AVFoundationEncoder { };

nanoem_u32_t
nanoemApplicationPluginEncoderGetABIVersion()
{
    return NANOEM_APPLICATION_PLUGIN_ENCODER_ABI_VERSION;
}

void
nanoemApplicationPluginEncoderInitialize()
{
}

nanoem_application_plugin_encoder_t *APIENTRY
nanoemApplicationPluginEncoderCreate()
{
    nanoem_application_plugin_encoder_t *opaque = new nanoem_application_plugin_encoder_t();
    return opaque;
}

int APIENTRY
nanoemApplicationPluginEncoderOpen(
    nanoem_application_plugin_encoder_t *encoder, const char *filePath, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    int result = 0;
    if (nanoem_is_not_null(encoder)) {
        result = encoder->open(filePath, statusPtr);
    }
    else {
        nanoem_application_plugin_status_assign_error(statusPtr, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
    return result;
}

void APIENTRY
nanoemApplicationPluginEncoderSetOption(nanoem_application_plugin_encoder_t *encoder, nanoem_u32_t key,
    const void *value, nanoem_u32_t size, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    if (nanoem_is_not_null(encoder)) {
        encoder->setOption(key, value, size, statusPtr);
    }
    else {
        nanoem_application_plugin_status_assign_error(statusPtr, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
}

void APIENTRY
nanoemApplicationPluginEncoderEncodeAudioFrame(nanoem_application_plugin_encoder_t *encoder,
    nanoem_frame_index_t currentFrameIndex, const nanoem_u8_t *data, nanoem_u32_t size, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    if (nanoem_is_not_null(encoder)) {
        encoder->encodeAudioFrame(currentFrameIndex, data, size, statusPtr);
    }
    else {
        nanoem_application_plugin_status_assign_error(statusPtr, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
}

void APIENTRY
nanoemApplicationPluginEncoderEncodeVideoFrame(nanoem_application_plugin_encoder_t *encoder,
    nanoem_frame_index_t currentFrameIndex, const nanoem_u8_t *data, nanoem_u32_t size, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    if (nanoem_is_not_null(encoder)) {
        encoder->encodeVideoFrame(currentFrameIndex, data, size, statusPtr);
    }
    else {
        nanoem_application_plugin_status_assign_error(statusPtr, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
}

void APIENTRY
nanoemApplicationPluginEncoderInterrupt(nanoem_application_plugin_encoder_t *encoder, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    if (nanoem_is_not_null(encoder)) {
        encoder->interrupt(statusPtr);
    }
    else {
        nanoem_application_plugin_status_assign_error(statusPtr, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
}

const char *const *APIENTRY
nanoemApplicationPluginEncoderGetAllAvailableVideoFormatExtensions(
    const nanoem_application_plugin_encoder_t * /* encoder */, nanoem_u32_t *length)
{
    static const char *kFormatExtensions[] = { "mp4", "m4v" };
    *length = sizeof(kFormatExtensions) / sizeof(kFormatExtensions[0]);
    return kFormatExtensions;
}

const char *APIENTRY
nanoemApplicationPluginEncoderGetFailureReason(const nanoem_application_plugin_encoder_t *encoder)
{
    return nanoem_is_not_null(encoder) ? encoder->failureReason() : nullptr;
}

const char *APIENTRY
nanoemApplicationPluginEncoderGetRecoverySuggestion(const nanoem_application_plugin_encoder_t *encoder)
{
    return nanoem_is_not_null(encoder) ? encoder->recoverySuggestion() : nullptr;
}

int APIENTRY
nanoemApplicationPluginEncoderClose(nanoem_application_plugin_encoder_t *encoder, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    int result = 0;
    if (nanoem_is_not_null(encoder)) {
        result = encoder->close(statusPtr);
    }
    else {
        nanoem_application_plugin_status_assign_error(statusPtr, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
    return result;
}

void APIENTRY
nanoemApplicationPluginEncoderDestroy(nanoem_application_plugin_encoder_t *encoder)
{
    if (nanoem_is_not_null(encoder)) {
        encoder->destroy();
        delete encoder;
    }
}

void APIENTRY
nanoemApplicationPluginEncoderTerminate()
{
}

nanoem_u32_t
nanoemApplicationPluginDecoderGetABIVersion()
{
    return NANOEM_APPLICATION_PLUGIN_DECODER_ABI_VERSION;
}

void APIENTRY
nanoemApplicationPluginDecoderInitialize()
{
}

nanoem_application_plugin_decoder_t *APIENTRY
nanoemApplicationPluginDecoderCreate()
{
    return new nanoem_application_plugin_decoder_t();
}

int APIENTRY
nanoemApplicationPluginDecoderOpen(
    nanoem_application_plugin_decoder_t *decoder, const char *filePath, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    int result = 0;
    if (nanoem_is_not_null(decoder)) {
        result = decoder->open(filePath, statusPtr);
    }
    else {
        nanoem_application_plugin_status_assign_error(statusPtr, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
    return result;
}

void APIENTRY
nanoemApplicationPluginDecoderSetOption(nanoem_application_plugin_decoder_t *decoder, nanoem_u32_t key,
    const void *value, nanoem_u32_t size, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    if (nanoem_is_not_null(decoder)) {
        decoder->setOption(key, value, size, statusPtr);
    }
    else {
        nanoem_application_plugin_status_assign_error(statusPtr, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
}

void APIENTRY
nanoemApplicationPluginDecoderGetAudioFormatValue(nanoem_application_plugin_decoder_t *decoder, nanoem_u32_t key,
    void *value, nanoem_u32_t *size, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    if (nanoem_is_not_null(decoder)) {
        decoder->audioFormatValue(key, value, size, statusPtr);
    }
    else {
        nanoem_application_plugin_status_assign_error(statusPtr, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
}

void APIENTRY
nanoemApplicationPluginDecoderGetVideoFormatValue(nanoem_application_plugin_decoder_t *decoder, nanoem_u32_t key,
    void *value, nanoem_u32_t *size, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    if (nanoem_is_not_null(decoder)) {
        decoder->videoFormatValue(key, value, size, statusPtr);
    }
    else {
        nanoem_application_plugin_status_assign_error(statusPtr, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
}

void APIENTRY
nanoemApplicationPluginDecoderDecodeAudioFrame(nanoem_application_plugin_decoder_t *decoder,
    nanoem_frame_index_t currentFrameIndex, nanoem_u8_t **data, nanoem_u32_t *size, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    if (nanoem_is_not_null(decoder)) {
        decoder->decodeAudioFrame(currentFrameIndex, data, size, statusPtr);
    }
    else {
        nanoem_application_plugin_status_assign_error(statusPtr, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
}

void APIENTRY
nanoemApplicationPluginDecoderDecodeVideoFrame(nanoem_application_plugin_decoder_t *decoder,
    nanoem_frame_index_t currentFrameIndex, nanoem_u8_t **data, nanoem_u32_t *size, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    if (nanoem_is_not_null(decoder)) {
        decoder->decodeVideoFrame(currentFrameIndex, data, size, statusPtr);
    }
    else {
        nanoem_application_plugin_status_assign_error(statusPtr, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
}

void APIENTRY
nanoemApplicationPluginDecoderDestroyAudioFrame(nanoem_application_plugin_decoder_t *decoder,
    nanoem_frame_index_t currentFrameIndex, nanoem_u8_t *data, nanoem_u32_t size)
{
    if (nanoem_is_not_null(decoder)) {
        decoder->destroyAudioFrame(currentFrameIndex, data, size);
    }
}

void APIENTRY
nanoemApplicationPluginDecoderDestroyVideoFrame(nanoem_application_plugin_decoder_t *decoder,
    nanoem_frame_index_t currentFrameIndex, nanoem_u8_t *data, nanoem_u32_t size)
{
    if (nanoem_is_not_null(decoder)) {
        decoder->destroyVideoFrame(currentFrameIndex, data, size);
    }
}

const char *const *APIENTRY
nanoemApplicationPluginDecoderGetAllAvailableAudioFormatExtensions(
    nanoem_application_plugin_decoder_t * /* decoder */, nanoem_u32_t *length)
{
    static const char *kFormatExtensions[] = { "mp3", "m4a", "aac", "caf" };
    *length = sizeof(kFormatExtensions) / sizeof(kFormatExtensions[0]);
    return kFormatExtensions;
}

const char *const *APIENTRY
nanoemApplicationPluginDecoderGetAllAvailableVideoFormatExtensions(
    nanoem_application_plugin_decoder_t * /* decoder */, nanoem_u32_t *length)
{
    static const char *kFormatExtensions[] = { "mp4", "m4v", "mov" };
    *length = sizeof(kFormatExtensions) / sizeof(kFormatExtensions[0]);
    return kFormatExtensions;
}

const char *APIENTRY
nanoemApplicationPluginDecoderGetFailureReason(nanoem_application_plugin_decoder_t *decoder)
{
    return nanoem_is_not_null(decoder) ? decoder->failureReason() : nullptr;
}

const char *APIENTRY
nanoemApplicationPluginDecoderGetRecoverySuggestion(nanoem_application_plugin_decoder_t *decoder)
{
    return nanoem_is_not_null(decoder) ? decoder->recoverySuggestion() : nullptr;
}

int APIENTRY
nanoemApplicationPluginDecoderClose(nanoem_application_plugin_decoder_t *decoder, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    int result = 0;
    if (nanoem_is_not_null(decoder)) {
        result = decoder->close(statusPtr);
    }
    else {
        nanoem_application_plugin_status_assign_error(statusPtr, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
    return result;
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
