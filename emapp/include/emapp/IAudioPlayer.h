/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_IAUDIO_H_
#define NANOEM_EMAPP_IAUDIO_H_

#include "emapp/Forward.h"

namespace nanoem {

class Error;
class URI;

class IAudioPlayer : private NonCopyable {
public:
    struct Chunk {
        nanoem_u32_t m_id;
        nanoem_u32_t m_size;
    };
    struct Format {
        nanoem_u16_t m_audioFormat;
        nanoem_u16_t m_numChannels;
        nanoem_u32_t m_sampleRate;
        nanoem_u32_t m_bytesPerSecond;
        nanoem_u16_t m_bytesPerPacket;
        nanoem_u16_t m_bitsPerSample;
    };
    struct WAVHeader {
        Chunk m_riffChunk;
        nanoem_u32_t m_wave;
    };
    struct WAVDescription {
        WAVHeader m_header;
        Chunk m_formatChunk;
        Format m_formatData;
    };
    struct Rational {
        inline nanoem_f64_t
        subdivide() const
        {
            return m_numerator / static_cast<nanoem_f64_t>(glm::max(m_denominator, 1u));
        }
        nanoem_u64_t m_numerator;
        nanoem_u32_t m_denominator;
    };
    virtual ~IAudioPlayer() NANOEM_DECL_NOEXCEPT
    {
    }

    virtual bool initialize(nanoem_frame_index_t duration, nanoem_u32_t frequency, Error &error) = 0;
    virtual void expandDuration(nanoem_frame_index_t frameIndex) = 0;
    virtual void destroy() = 0;

    virtual bool load(const ByteArray &bytes, Error &error) = 0;
    virtual bool load(const ByteArray &bytes, const WAVDescription &desc, Error &error) = 0;
    virtual bool loadAllLinearPCMSamples(const nanoem_u8_t *data, size_t size, Error &error) = 0;
    virtual void play() = 0;
    virtual void playPart(nanoem_f64_t start, nanoem_f64_t length) = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual void suspend() = 0;
    virtual void stop() = 0;
    virtual void update() = 0;
    virtual void seek(const Rational &value) = 0;

    virtual const ByteArray *linearPCMSamples() const = 0;
    virtual URI fileURI() const = 0;
    virtual void setFileURI(const URI &value) = 0;
    virtual Vector2 volumeGainRange() const NANOEM_DECL_NOEXCEPT = 0;
    virtual Rational currentRational() const NANOEM_DECL_NOEXCEPT = 0;
    virtual Rational lastRational() const NANOEM_DECL_NOEXCEPT = 0;
    virtual Rational durationRational() const NANOEM_DECL_NOEXCEPT = 0;
    virtual nanoem_u32_t bitsPerSample() const NANOEM_DECL_NOEXCEPT = 0;
    virtual nanoem_u32_t numChannels() const NANOEM_DECL_NOEXCEPT = 0;
    virtual nanoem_u32_t sampleRate() const NANOEM_DECL_NOEXCEPT = 0;

    virtual bool isLoaded() const NANOEM_DECL_NOEXCEPT = 0;
    virtual bool isFinished() const NANOEM_DECL_NOEXCEPT = 0;
    virtual bool isPlaying() const NANOEM_DECL_NOEXCEPT = 0;
    virtual bool isPaused() const NANOEM_DECL_NOEXCEPT = 0;
    virtual bool isStopped() const NANOEM_DECL_NOEXCEPT = 0;
    virtual bool wasPlaying() const NANOEM_DECL_NOEXCEPT = 0;
    virtual nanoem_f32_t volumeGain() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setVolumeGain(nanoem_f32_t value) = 0;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_IAUDIO_H_ */
