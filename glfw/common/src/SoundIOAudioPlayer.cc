/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#if defined(NANOEM_HAS_SOUNDIO)

#include "SoundIOAudioPlayer.h"

namespace nanoem {
namespace glfw {

const uint64_t SoundIOAudioPlayer::kClockLatencyThresholdFPSCount = 10;

SoundIOAudioPlayer::SoundIOAudioPlayer()
    : BaseAudioPlayer()
    , m_context(nullptr)
    , m_device(nullptr)
    , m_stream(nullptr)
    , m_offset(0)
{
}

SoundIOAudioPlayer::~SoundIOAudioPlayer()
{
    destroySoundIOContext();
}

bool
SoundIOAudioPlayer::initialize(nanoem_frame_index_t, nanoem_u32_t sampleRate, Error &error)
{
    nanoem_u32_t actualSampleRate = sampleRate * kSmoothnessScaleFactor;
    initializeDescription(8, 1, actualSampleRate, m_linearPCMSamples.size(), m_description);
    m_currentRational.m_denominator = m_description.m_formatData.m_sampleRate;
    return !error.hasReason();
}

void
SoundIOAudioPlayer::expandDuration(nanoem_frame_index_t)
{
    if (isPlaying()) {
        Error error;
        internalTransitStateStopped(error);
    }
}

void
SoundIOAudioPlayer::destroy()
{
    destroySoundIOContext();
    reset();
}

bool
SoundIOAudioPlayer::loadAllLinearPCMSamples(const nanoem_u8_t *data, size_t size, Error &error)
{
    bool succeeded = false;
    internalTransitStateStopped(error);
    destroySoundIOContext();
    if (createSoundIOContext(error)) {
        m_loaded = true;
        m_linearPCMSamples.assign(data, data + size);
        m_currentRational.m_denominator = m_description.m_formatData.m_sampleRate;
        m_durationRational.m_numerator = size;
        m_durationRational.m_denominator = m_description.m_formatData.m_bytesPerSecond;
        m_offset = 0;
        m_loaded = succeeded = true;
    }
    return succeeded;
}

void
SoundIOAudioPlayer::playPart(double, double)
{
}

void
SoundIOAudioPlayer::update()
{
    const Format &format = m_description.m_formatData;
    const nanoem_u64_t audioSampleOffset = m_offset,
                       clockSampleOffset = static_cast<nanoem_u64_t>(m_clock.seconds() * format.m_sampleRate),
                       clockSampleLatencyThreshold = format.m_sampleRate / kClockLatencyThresholdFPSCount,
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
    if (m_context) {
        soundio_flush_events(m_context);
    }
}

void
SoundIOAudioPlayer::seek(const IAudioPlayer::Rational &value)
{
    Error error;
    const nanoem_f64_t seconds = value.m_numerator / static_cast<nanoem_f64_t>(value.m_denominator);
    m_clock.seek(value);
    m_offset = seconds * m_description.m_formatData.m_sampleRate;
    m_finished = false;
    value.m_numerator > 0 ? internalTransitStatePaused(error) : internalTransitStateStopped(error);
}

void
SoundIOAudioPlayer::internalSetVolumeGain(float value, Error &error)
{
    if (m_stream) {
        wrapCall(soundio_outstream_set_volume(m_stream, value), error);
    }
}

void
SoundIOAudioPlayer::internalTransitStateStarted(Error &error)
{
    if (m_stream) {
        wrapCall(soundio_outstream_start(m_stream), error);
    }
    m_clock.start();
}

void
SoundIOAudioPlayer::internalTransitStatePaused(Error &error)
{
    if (m_stream) {
        wrapCall(soundio_outstream_pause(m_stream, true), error);
    }
    m_clock.pause();
}

void
SoundIOAudioPlayer::internalTransitStateResumed(Error &error)
{
    if (m_stream) {
        wrapCall(soundio_outstream_pause(m_stream, false), error);
    }
    m_clock.resume();
}

void
SoundIOAudioPlayer::internalTransitStateStopped(Error &error)
{
    if (m_stream) {
        wrapCall(soundio_outstream_pause(m_stream, true), error);
        m_offset = 0;
    }
    m_clock.stop();
}

void
SoundIOAudioPlayer::writeAllSamples(SoundIoOutStream *stream, int samplesLeft, WriteSampleCallback callback)
{
    struct SoundIoChannelArea *areas = nullptr;
    while (samplesLeft > 0) {
        int sampleCount = samplesLeft;
        if (soundio_outstream_begin_write(stream, &areas, &sampleCount) != SoundIoErrorNone) {
            break;
        }
        const struct SoundIoChannelLayout &layout = stream->layout;
        const int channels = layout.channel_count;
        const nanoem_u64_t sampleOffset = m_offset;
        for (int i = 0; i < sampleCount; i++) {
            const int localOffset = channels * (sampleOffset + i);
            for (int j = 0; j < channels; j++) {
                auto &area = areas[j];
                auto ptr = area.ptr + area.step * i;
                callback(ptr, localOffset);
            }
        }
        if (soundio_outstream_end_write(stream) != SoundIoErrorNone) {
            break;
        }
        m_offset += sampleCount;
        samplesLeft -= sampleCount;
    }
}

void
SoundIOAudioPlayer::wrapCall(int code, Error &error)
{
    if (code != 0) {
        error = Error(soundio_strerror(code), code, Error::kDomainTypeOS);
    }
}

bool
SoundIOAudioPlayer::createSoundIOContext(Error &error)
{
    m_context = soundio_create();
    wrapCall(soundio_connect(m_context), error);
    soundio_flush_events(m_context);
    int deviceIndex = soundio_default_output_device_index(m_context);
    if (deviceIndex >= 0) {
        m_device = soundio_get_output_device(m_context, deviceIndex);
        m_stream = soundio_outstream_create(m_device);
        m_stream->sample_rate = m_description.m_formatData.m_sampleRate;
        m_stream->userdata = this;
        m_stream->format = SoundIoFormatS16NE;
        if (m_description.m_formatData.m_bitsPerSample == 16) {
            m_stream->write_callback = [](struct SoundIoOutStream *stream, int frameCountMin, int frameCountMax) {
                BX_UNUSED_1(frameCountMin);
                auto self = static_cast<SoundIOAudioPlayer *>(stream->userdata);
                self->writeAllSamples(stream, frameCountMax, [self](void *ptr, nanoem_rsize_t offset) {
                    auto dest = static_cast<nanoem_i16_t *>(ptr);
                    auto src = reinterpret_cast<const nanoem_i16_t *>(self->m_linearPCMSamples.data()) + offset;
                    *dest = *src;
                });
            };
        }
        else if (m_description.m_formatData.m_bitsPerSample == 8) {
            m_stream->write_callback = [](struct SoundIoOutStream *stream, int frameCountMin, int frameCountMax) {
                BX_UNUSED_1(frameCountMin);
                auto self = static_cast<SoundIOAudioPlayer *>(stream->userdata);
                self->writeAllSamples(stream, frameCountMax, [self](void *ptr, nanoem_rsize_t offset) {
                    auto dest = static_cast<nanoem_i16_t *>(ptr);
                    auto src = self->m_linearPCMSamples.data() + offset;
                    *dest = *src;
                });
            };
        }
        wrapCall(soundio_outstream_open(m_stream), error);
        m_state.first = kStopState;
    }
    else {
        wrapCall(SoundIoErrorInitAudioBackend, error);
    }
    return !error.hasReason();
}

void
SoundIOAudioPlayer::destroySoundIOContext()
{
    if (m_stream) {
        soundio_outstream_destroy(m_stream);
        m_stream = nullptr;
    }
    if (m_device) {
        soundio_device_unref(m_device);
        m_device = nullptr;
    }
    if (m_context) {
        soundio_destroy(m_context);
        m_context = nullptr;
    }
}

} /* namespace glfw */
} /* namespace nanoem */

#endif /* NANOEM_HAS_SOUNDIO */
