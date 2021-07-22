/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#import "AudioUnitAudioPlayer.h"

#include "emapp/Constants.h"
#include "emapp/Error.h"
#include "emapp/StringUtils.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace macos {
namespace {

static const AudioObjectPropertyAddress kDefaultOutputDevicePropertyAddress = {
    kAudioHardwarePropertyDefaultOutputDevice,
    kAudioObjectPropertyScopeGlobal,
    kAudioObjectPropertyElementMaster,
};
static const AudioObjectPropertyAddress kDataSourcePropertyAddress = {
    kAudioDevicePropertyDataSource,
    kAudioObjectPropertyScopeOutput,
    kAudioObjectPropertyElementMaster,
};
static const uint32_t kDeviceDataSourceIDHeadphone = 'hdpn';
static const uint64_t kClockLatencyThresholdFPSCount = 10;

} /* namespace anonymous */

AudioUnitAudioPlayer::AudioUnitAudioPlayer(IEventPublisher *eventPublisher)
    : m_eventPublisher(eventPublisher)
    , m_offset(0)
{
}

AudioUnitAudioPlayer::~AudioUnitAudioPlayer()
{
}

bool
AudioUnitAudioPlayer::initialize(nanoem_frame_index_t /* duration */, nanoem_u32_t /* sampleRate */, Error &error)
{
    if (m_state.first == kInitialState) {
        AudioComponentDescription desc;
        Inline::clearZeroMemory(desc);
        desc.componentType = kAudioUnitType_Output;
        desc.componentSubType = kAudioUnitSubType_HALOutput;
        desc.componentManufacturer = kAudioUnitManufacturer_Apple;
        AudioComponent component = AudioComponentFindNext(nullptr, &desc);
        wrapCall(AudioComponentInstanceNew(component, &m_outputUnit), error);
        wrapCall(AudioUnitInitialize(m_outputUnit), error);
        initializeDefaultOutputDevice(error);
    }
    return !error.hasReason();
}

void
AudioUnitAudioPlayer::expandDuration(nanoem_frame_index_t frameIndex)
{
    if (isPlaying()) {
        AudioOutputUnitStop(m_outputUnit);
    }
    if (isLoaded()) {
        nanoem_rsize_t size = frameIndex * (m_nativeOutputDescription.mSampleRate / Constants::kHalfBaseFPS) *
            m_nativeOutputDescription.mBytesPerFrame;
        if (size > m_samples.size()) {
            m_samples.resize(size * 1.5f);
        }
    }
}

void
AudioUnitAudioPlayer::destroy()
{
    AudioObjectRemovePropertyListener(
        kAudioObjectSystemObject, &kDefaultOutputDevicePropertyAddress, handleDefaultOutputDeviceChange, this);
    if (m_outputUnit) {
        AudioUnitUninitialize(m_outputUnit);
        AudioComponentInstanceDispose(m_outputUnit);
        m_outputUnit = nullptr;
    }
    reset();
}

bool
AudioUnitAudioPlayer::loadAllLinearPCMSamples(const nanoem_u8_t *data, size_t size, Error &error)
{
    internalTransitStateStopped(error);
    wrapCall(AudioUnitUninitialize(m_outputUnit), error);
    internalLoadAllLinearPCMSamples(data, size, error);
    wrapCall(AudioUnitInitialize(m_outputUnit), error);
    m_offset = 0;
    m_loaded = true;
    m_linearPCMSamples.assign(data, data + size);
    return !error.hasReason();
}

void
AudioUnitAudioPlayer::playPart(double, double)
{
}

void
AudioUnitAudioPlayer::update()
{
    const nanoem_u64_t audioSampleOffset =
                           static_cast<nanoem_u64_t>(m_offset / m_nativeOutputDescription.mBytesPerFrame),
                       clockSampleOffset =
                           static_cast<nanoem_u64_t>(m_clock.seconds() * m_nativeOutputDescription.mSampleRate),
                       clockSampleLatencyThreshold =
                           (m_nativeOutputDescription.mSampleRate / 60) * kClockLatencyThresholdFPSCount,
                       clockSampleLatency =
                           (clockSampleOffset > audioSampleOffset ? clockSampleOffset - audioSampleOffset
                                                                  : audioSampleOffset - clockSampleOffset) *
        (audioSampleOffset > 0);
    m_lastRational = m_currentRational;
    if (clockSampleLatency < clockSampleLatencyThreshold) {
        m_currentRational.m_numerator = clockSampleOffset;
    }
    else {
        m_currentRational.m_numerator = audioSampleOffset + clockSampleLatencyThreshold;
    }
}

void
AudioUnitAudioPlayer::seek(const IAudioPlayer::Rational &value)
{
    Error error;
    const nanoem_f64_t seconds = value.m_numerator / static_cast<nanoem_f64_t>(value.m_denominator);
    m_clock.seek(value);
    m_offset = seconds * m_nativeOutputDescription.mSampleRate * m_nativeOutputDescription.mBytesPerFrame;
    m_finished = false;
    value.m_numerator > 0 ? internalTransitStatePaused(error) : internalTransitStateStopped(error);
    error.notify(m_eventPublisher);
}

void
AudioUnitAudioPlayer::copyDescription(const WAVDescription &source, AudioStreamBasicDescription &dest)
{
    Inline::clearZeroMemory(dest);
    const Format &format = source.m_formatData;
    dest.mBitsPerChannel = format.m_bitsPerSample;
    dest.mBytesPerFrame = dest.mBytesPerPacket = format.m_bytesPerPacket;
    dest.mChannelsPerFrame = format.m_numChannels;
    dest.mFormatFlags = kAudioFormatFlagIsPacked | kAudioFormatFlagIsSignedInteger;
    dest.mFormatID = kAudioFormatLinearPCM;
    dest.mFramesPerPacket = 1;
    dest.mSampleRate = format.m_sampleRate;
}

void
AudioUnitAudioPlayer::wrapCall(OSStatus err, Error &error)
{
    if (err != noErr && !error.hasReason()) {
        char message[Error::kMaxReasonLength];
        StringUtils::format(message, sizeof(message), "OSStatus returns %d", err);
        error = Error(message, err, Error::kDomainTypeOS);
    }
}

OSStatus
AudioUnitAudioPlayer::renderAudioBuffer(
    void *inRefCon, AudioUnitRenderActionFlags *, const AudioTimeStamp *, UInt32, UInt32, AudioBufferList *ioData)
{
    auto self = static_cast<AudioUnitAudioPlayer *>(inRefCon);
    const ByteArray &samples = self->m_samples;
    const AudioBuffer &buffer = ioData->mBuffers[0];
    const nanoem_u8_t *samplesPtr = samples.data();
    const nanoem_rsize_t offset = self->m_offset, size = buffer.mDataByteSize,
                         rest = samples.size() >= offset ? samples.size() - offset : 0,
                         length = glm::min(size, rest) / sizeof(nanoem_f32_t);
    auto destDataPtr = reinterpret_cast<nanoem_f32_t *>(buffer.mData);
    auto sourceBasePtr = reinterpret_cast<const nanoem_f32_t *>(samplesPtr + offset);
    memset(destDataPtr, 0, size);
    for (nanoem_rsize_t i = 0; i < length; i++) {
        nanoem_f32_t value = *(sourceBasePtr + i);
        if (glm::isnan(value) || glm::isinf(value)) {
            value = 0.0f;
        }
        destDataPtr[i] = glm::clamp(value, -1.0f, 1.0f);
    }
    self->m_offset += size;
    return noErr;
}

OSStatus
AudioUnitAudioPlayer::renderNullBuffer(
    void *inRefCon, AudioUnitRenderActionFlags *, const AudioTimeStamp *, UInt32, UInt32, AudioBufferList *ioData)
{
    auto self = static_cast<AudioUnitAudioPlayer *>(inRefCon);
    const AudioBuffer &buffer = ioData->mBuffers[0];
    const size_t size = buffer.mDataByteSize;
    memset(buffer.mData, 0, size);
    self->m_offset += size;
    return noErr;
}

OSStatus
AudioUnitAudioPlayer::handleDefaultOutputDeviceChange(AudioObjectID /* inObjectID */, UInt32 /* inNumberAddresses */,
    const AudioObjectPropertyAddress * /* inAddresses */, void *inClientData)
{
    Error error;
    auto self = static_cast<AudioUnitAudioPlayer *>(inClientData);
    bool playing = self->isPlaying();
    if (playing) {
        self->pause();
    }
    self->initializeDefaultOutputDevice(error);
    uint32_t dataSourceID = 0;
    getObjectProperty(self->m_defaultOutputDeviceID, &kDataSourcePropertyAddress, dataSourceID, error);
    error.notify(self->m_eventPublisher);
    if (playing && dataSourceID == kDeviceDataSourceIDHeadphone) {
        self->resume();
    }
    return noErr;
}

void
AudioUnitAudioPlayer::internalSetVolumeGain(float value, Error &error)
{
    wrapCall(AudioUnitSetParameter(m_outputUnit, kHALOutputParam_Volume, kAudioUnitScope_Input, 0, value, 0), error);
}

void
AudioUnitAudioPlayer::internalTransitStateStarted(Error &error)
{
    wrapCall(AudioOutputUnitStart(m_outputUnit), error);
    m_clock.start();
}

void
AudioUnitAudioPlayer::internalTransitStatePaused(Error &error)
{
    wrapCall(AudioOutputUnitStop(m_outputUnit), error);
    m_clock.pause();
}

void
AudioUnitAudioPlayer::internalTransitStateResumed(Error &error)
{
    wrapCall(AudioOutputUnitStart(m_outputUnit), error);
    m_clock.resume();
}

void
AudioUnitAudioPlayer::internalTransitStateStopped(Error &error)
{
    wrapCall(AudioOutputUnitStop(m_outputUnit), error);
    m_clock.stop();
    m_offset = 0;
}

void
AudioUnitAudioPlayer::initializeDefaultOutputDevice(Error &error)
{
    wrapCall(AudioUnitUninitialize(m_outputUnit), error);
    getObjectProperty(kAudioObjectSystemObject, &kDefaultOutputDevicePropertyAddress, m_defaultOutputDeviceID, error);
    AudioObjectAddPropertyListener(
        kAudioObjectSystemObject, &kDefaultOutputDevicePropertyAddress, handleDefaultOutputDeviceChange, this);
    setUnitProperty(kAudioOutputUnitProperty_CurrentDevice, kAudioUnitScope_Input, m_defaultOutputDeviceID, error);
    getUnitProperty(kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, m_nativeOutputDescription, error);
    setUnitProperty(kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, m_nativeOutputDescription, error);
    m_currentRational.m_denominator = m_nativeOutputDescription.mSampleRate;
    if (m_loaded) {
        internalLoadAllLinearPCMSamples(m_linearPCMSamples.data(), m_linearPCMSamples.size(), error);
        internalSetVolumeGain(m_volumeGain.z, error);
    }
    else {
        m_state.first = kStopState;
        AURenderCallbackStruct callback;
        callback.inputProc = renderNullBuffer;
        callback.inputProcRefCon = this;
        setUnitProperty(kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, callback, error);
    }
    wrapCall(AudioUnitInitialize(m_outputUnit), error);
}

void
AudioUnitAudioPlayer::internalLoadAllLinearPCMSamples(const nanoem_u8_t *data, size_t size, Error &error)
{
    if (m_description.m_formatData.m_sampleRate != nanoem_u32_t(m_nativeOutputDescription.mSampleRate)) {
        convertAllLinearPCMSamplesComplex(data, size, error);
    }
    else {
        convertAllLinearPCMSamplesSimple(data, size, error);
    }
    AURenderCallbackStruct callback;
    callback.inputProc = renderAudioBuffer;
    callback.inputProcRefCon = this;
    setUnitProperty(kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, callback, error);
}

void
AudioUnitAudioPlayer::convertAllLinearPCMSamplesComplex(const nanoem_u8_t *data, size_t size, Error &error)
{
    struct Input {
        Input(const nanoem_u8_t *data, size_t size, const WAVDescription &desc)
        {
            copyDescription(desc, m_desc);
            m_mutaleBytes.assign(data, data + size);
            m_audioBuffer.mNumberChannels = m_desc.mChannelsPerFrame;
            m_audioBuffer.mData = m_mutaleBytes.data();
            m_audioBuffer.mDataByteSize = size;
        }
        AudioBuffer m_audioBuffer;
        ByteArray m_mutaleBytes;
        AudioStreamBasicDescription m_desc;
    };
    AudioConverterRef converter;
    const nanoem_rsize_t bytesPerSample = m_description.m_formatData.m_bytesPerPacket, mod = size % bytesPerSample,
                         inputAudioSize = size - mod,
                         outputAudioSize = m_nativeOutputDescription.mBytesPerPacket *
        (inputAudioSize / bytesPerSample) *
        (m_nativeOutputDescription.mSampleRate / m_description.m_formatData.m_sampleRate);
    Input input(data, inputAudioSize, m_description);
    AudioBufferList outputBuffers;
    outputBuffers.mNumberBuffers = 1;
    auto &outputAudioBuffer = outputBuffers.mBuffers[0];
    m_samples.resize(outputAudioSize);
    outputAudioBuffer.mData = m_samples.data();
    outputAudioBuffer.mDataByteSize = outputAudioSize;
    outputAudioBuffer.mNumberChannels = m_nativeOutputDescription.mChannelsPerFrame;
    uint32_t outputNumPackets = outputAudioSize / m_nativeOutputDescription.mBytesPerPacket;
    wrapCall(AudioConverterNew(&input.m_desc, &m_nativeOutputDescription, &converter), error);
    wrapCall(AudioConverterFillComplexBuffer(
                 converter,
                 [](AudioConverterRef inAudioConverter, UInt32 *ioNumberDataPackets, AudioBufferList *ioData,
                     AudioStreamPacketDescription **outDataPacketDescription, void *inUserData) -> OSStatus {
                     BX_UNUSED_2(inAudioConverter, outDataPacketDescription);
                     Input *input = static_cast<Input *>(inUserData);
                     ioData->mNumberBuffers = 1;
                     ioData->mBuffers[0] = input->m_audioBuffer;
                     *ioNumberDataPackets = ioData->mBuffers[0].mDataByteSize / input->m_desc.mBytesPerPacket;
                     return noErr;
                 },
                 &input, &outputNumPackets, &outputBuffers, nullptr),
        error);
    wrapCall(AudioConverterDispose(converter), error);
    m_durationRational.m_numerator = outputAudioSize;
    m_durationRational.m_denominator =
        m_nativeOutputDescription.mBytesPerPacket * m_nativeOutputDescription.mSampleRate;
}

void
AudioUnitAudioPlayer::convertAllLinearPCMSamplesSimple(const nanoem_u8_t *data, size_t size, Error &error)
{
    AudioConverterRef converter;
    const nanoem_rsize_t bytesPerSample = m_description.m_formatData.m_bytesPerPacket, mod = size % bytesPerSample,
                         inputAudioSize = size - mod;
    AudioStreamBasicDescription inputDescription;
    copyDescription(m_description, inputDescription);
    wrapCall(AudioConverterNew(&inputDescription, &m_nativeOutputDescription, &converter), error);
    uint32_t outputAudioSize = m_nativeOutputDescription.mBytesPerPacket * (inputAudioSize / bytesPerSample);
    m_samples.resize(outputAudioSize);
    wrapCall(AudioConverterConvertBuffer(converter, inputAudioSize, data, &outputAudioSize, m_samples.data()), error);
    wrapCall(AudioConverterDispose(converter), error);
    m_durationRational.m_numerator = outputAudioSize;
    m_durationRational.m_denominator =
        m_nativeOutputDescription.mBytesPerPacket * m_nativeOutputDescription.mSampleRate;
}

} /* namespace macos */
} /* namespace nanoem */
