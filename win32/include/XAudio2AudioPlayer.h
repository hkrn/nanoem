/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_WIN32_XAUDIO2AUDIOPLAYER_H_
#define NANOEM_EMAPP_WIN32_XAUDIO2AUDIOPLAYER_H_

#include "emapp/BaseAudioPlayer.h"

namespace nanoem {

class IEventPublisher;

namespace win32 {

class XAudio2AudioPlayer : public BaseAudioPlayer {
public:
    XAudio2AudioPlayer(IEventPublisher *eventPublisher);
    ~XAudio2AudioPlayer() override;

    bool initialize(nanoem_frame_index_t duration, nanoem_u32_t sampleRate, Error &error) override;
    void expandDuration(nanoem_frame_index_t frameIndex) override;
    void destroy() override;

    bool loadAllLinearPCMSamples(const nanoem_u8_t *data, size_t size, Error &error) override;
    void playPart(double start, double length) override;
    void update() override;
    void seek(const Rational &value) override;

private:
    void internalSetVolumeGain(float value, Error &error) override;
    void internalTransitStateStarted(Error &error) override;
    void internalTransitStatePaused(Error &error) override;
    void internalTransitStateResumed(Error &error) override;
    void internalTransitStateStopped(Error &error) override;

    IEventPublisher *m_eventPublisher = nullptr;
    volatile uint64_t m_offset = 0;
};

} /* namespace win32 */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_WIN32_XAUDIO2AUDIOPLAYER_H_ */
