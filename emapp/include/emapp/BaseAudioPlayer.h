/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_BASEAUDIOPLAYER_H_
#define NANOEM_EMAPP_BASEAUDIOPLAYER_H_

#include "emapp/IAudioPlayer.h"
#include "emapp/URI.h"

#include "sokol/sokol_time.h"

namespace nanoem {

class BaseAudioPlayer : public IAudioPlayer {
public:
    struct Clock {
        Clock() NANOEM_DECL_NOEXCEPT : m_ticks(0), m_delta(0)
        {
        }
        inline nanoem_f64_t
        seconds() const NANOEM_DECL_NOEXCEPT
        {
            return stm_sec(stm_since(m_ticks));
        }
        inline bool
        isPaused() const NANOEM_DECL_NOEXCEPT
        {
            return m_ticks == 0 || m_delta > 0;
        }
        inline void
        seek(const Rational &value) NANOEM_DECL_NOEXCEPT
        {
            m_ticks = stm_now() + nanoem_u64_t(value.subdivide() * 1000000000ull);
            m_delta = 0;
        }
        inline void
        start() NANOEM_DECL_NOEXCEPT
        {
            m_ticks = stm_now();
        }
        inline void
        pause() NANOEM_DECL_NOEXCEPT
        {
            if (m_ticks > 0) {
                m_delta = stm_since(m_ticks);
                m_ticks = 0;
            }
        }
        inline void
        resume() NANOEM_DECL_NOEXCEPT
        {
            if (m_delta > 0) {
                m_ticks = stm_since(m_delta);
                m_delta = 0;
            }
        }
        inline void
        stop() NANOEM_DECL_NOEXCEPT
        {
            m_ticks = m_delta = 0;
        }
        nanoem_u64_t m_ticks;
        nanoem_u64_t m_delta;
    };

    static const size_t kRIFFTagSize;
    static bool containsRIFFTag(const nanoem_u8_t *ptr, size_t size) NANOEM_DECL_NOEXCEPT;
    static void initializeDescription(nanoem_u8_t bits, nanoem_u16_t channels, nanoem_u32_t frequency,
        nanoem_rsize_t size, WAVDescription &output) NANOEM_DECL_NOEXCEPT;
    static void initializeDescription(
        const IAudioPlayer *player, nanoem_rsize_t size, WAVDescription &output) NANOEM_DECL_NOEXCEPT;
    static StringList loadableExtensions();
    static StringSet loadableExtensionsSet();
    static bool isLoadableExtension(const String &extension);
    static bool isLoadableExtension(const URI &fileURI);

    BaseAudioPlayer();

    bool load(const ByteArray &bytes, Error &error) NANOEM_DECL_OVERRIDE;
    bool load(const ByteArray &bytes, const WAVDescription &desc, Error &error) NANOEM_DECL_OVERRIDE;
    void play() NANOEM_DECL_OVERRIDE;
    void pause() NANOEM_DECL_OVERRIDE;
    void resume() NANOEM_DECL_OVERRIDE;
    void suspend() NANOEM_DECL_OVERRIDE;
    void stop() NANOEM_DECL_OVERRIDE;

    const ByteArray *linearPCMSamples() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    URI fileURI() const NANOEM_DECL_OVERRIDE;
    void setFileURI(const URI &value) NANOEM_DECL_OVERRIDE;
    Vector2 volumeGainRange() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    Rational currentRational() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    Rational lastRational() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    Rational durationRational() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

    nanoem_u32_t bitsPerSample() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    nanoem_u32_t numChannels() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    nanoem_u32_t sampleRate() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool isLoaded() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool isFinished() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool isPlaying() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool isPaused() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool isStopped() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool wasPlaying() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    nanoem_f32_t volumeGain() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setVolumeGain(nanoem_f32_t value) NANOEM_DECL_OVERRIDE;

protected:
    enum State { kInitialState, kStopState, kSuspendState, kPauseState, kPlayState, kMaxPlayingState };
    virtual void internalSetVolumeGain(nanoem_f32_t value, Error &error) = 0;
    virtual void internalTransitStateStarted(Error &error) = 0;
    virtual void internalTransitStatePaused(Error &error) = 0;
    virtual void internalTransitStateResumed(Error &error) = 0;
    virtual void internalTransitStateStopped(Error &error) = 0;
    void reset();
    void setState(State value);

    ByteArray m_linearPCMSamples;
    URI m_fileURI;
    Vector3 m_volumeGain;
    tinystl::pair<State, State> m_state;
    Rational m_currentRational;
    Rational m_lastRational;
    Rational m_durationRational;
    WAVDescription m_description;
    bool m_finished;
    bool m_loaded;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_BASEAUDIOPLAYER_H_ */
