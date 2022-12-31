/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#if defined(NANOEM_HAS_SOUNDIO)
#pragma once
#ifndef NANOEM_EMAPP_GLFW_SOUNDIOAUDIOPLAYER_H_
#define NANOEM_EMAPP_GLFW_SOUNDIOAUDIOPLAYER_H_

#include "soundio/soundio.h"
#include <atomic>
#include <functional>

#include "emapp/BaseAudioPlayer.h"
#include "emapp/Error.h"

namespace nanoem {
namespace glfw {

class SoundIOAudioPlayer : public BaseAudioPlayer {
public:
    static const uint64_t kClockLatencyThresholdFPSCount;
    static const uint32_t kSmoothnessScaleFactor = 16;

    SoundIOAudioPlayer();
    ~SoundIOAudioPlayer();

    bool initialize(nanoem_frame_index_t /* duration */, nanoem_u32_t sampleRate, Error &error) override;
    void expandDuration(nanoem_frame_index_t /* frameIndex */) override;
    void destroy() override;
    bool loadAllLinearPCMSamples(const nanoem_u8_t *data, size_t size, Error &error) override;
    void playPart(double /* start */, double /* length */) override;
    void update() override;
    void seek(const Rational &value) override;

private:
    using WriteSampleCallback = std::function<void(char *, nanoem_rsize_t)>;
    void internalSetVolumeGain(float value, Error &error) override;
    void internalTransitStateStarted(Error &error) override;
    void internalTransitStatePaused(Error &error) override;
    void internalTransitStateResumed(Error &error) override;
    void internalTransitStateStopped(Error &error) override;

    void writeAllSamples(struct SoundIoOutStream *stream, int framesLeft, WriteSampleCallback callback);
    void wrapCall(int code, Error &error);
    bool createSoundIOContext(Error &error);
    void destroySoundIOContext();

    struct SoundIo *m_context;
    struct SoundIoDevice *m_device;
    struct SoundIoOutStream *m_stream;
    Clock m_clock;
    std::atomic<uint64_t> m_offset;
};

} /* namespace glfw */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_GLFW_SOUNDIOAUDIOPLAYER_H_ */
#endif /* NANOEM_HAS_SOUNDIO */
