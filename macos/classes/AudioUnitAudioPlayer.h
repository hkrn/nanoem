/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/BaseAudioPlayer.h"

#import <AudioToolbox/AudioToolbox.h>
#import <Foundation/Foundation.h>

#include <atomic>

namespace nanoem {

class IEventPublisher;

namespace macos {

class AudioUnitAudioPlayer : public BaseAudioPlayer {
public:
    AudioUnitAudioPlayer(IEventPublisher *eventPublisher);
    ~AudioUnitAudioPlayer() override;

    bool initialize(nanoem_frame_index_t duration, nanoem_u32_t sampleRate, Error &error) override;
    void expandDuration(nanoem_frame_index_t frameIndex) override;
    void destroy() override;

    bool loadAllLinearPCMSamples(const nanoem_u8_t *data, size_t size, Error &error) override;
    void playPart(double start, double length) override;
    void update() override;
    void seek(const Rational &value) override;

private:
    static void copyDescription(const WAVDescription &source, AudioStreamBasicDescription &dest);
    static void wrapCall(OSStatus err, Error &error);
    static OSStatus renderAudioBuffer(
        void *inRefCon, AudioUnitRenderActionFlags *, const AudioTimeStamp *, UInt32, UInt32, AudioBufferList *ioData);
    static OSStatus renderNullBuffer(
        void *inRefCon, AudioUnitRenderActionFlags *, const AudioTimeStamp *, UInt32, UInt32, AudioBufferList *ioData);
    static OSStatus handleDefaultOutputDeviceChange(AudioObjectID inObjectID, UInt32 inNumberAddresses,
        const AudioObjectPropertyAddress *inAddresses, void *inClientData);

    void internalSetVolumeGain(float value, Error &error) override;
    void internalTransitStateStarted(Error &error) override;
    void internalTransitStatePaused(Error &error) override;
    void internalTransitStateResumed(Error &error) override;
    void internalTransitStateStopped(Error &error) override;
    void initializeDefaultOutputDevice(Error &error);
    void internalLoadAllLinearPCMSamples(const nanoem_u8_t *data, size_t size, Error &error);
    void convertAllLinearPCMSamplesComplex(const nanoem_u8_t *data, size_t size, Error &error);
    void convertAllLinearPCMSamplesSimple(const nanoem_u8_t *data, size_t size, Error &error);

    template <typename T>
    static inline void
    getObjectProperty(AudioObjectID id, const AudioObjectPropertyAddress *address, T &data, Error &error)
    {
        uint32_t size = sizeof(data);
        wrapCall(AudioObjectGetPropertyData(id, address, 0, nullptr, &size, &data), error);
        nanoem_assert(size == sizeof(data), "must be equal to caller size");
    }
    template <typename T>
    inline void
    getUnitProperty(AudioUnitPropertyID id, AudioUnitScope scope, T &data, Error &error)
    {
        uint32_t size = sizeof(data);
        wrapCall(AudioUnitGetProperty(m_outputUnit, id, scope, 0, &data, &size), error);
        nanoem_assert(size == sizeof(data), "must be equal to caller size");
    }
    template <typename T>
    inline void
    setUnitProperty(AudioUnitPropertyID id, AudioUnitScope scope, const T &data, Error &error)
    {
        wrapCall(AudioUnitSetProperty(m_outputUnit, id, scope, 0, &data, sizeof(data)), error);
    }

    AudioDeviceID m_defaultOutputDeviceID = kAudioDeviceUnknown;
    AudioUnit m_outputUnit = nullptr;
    AudioStreamBasicDescription m_nativeOutputDescription = {};
    IEventPublisher *m_eventPublisher = nullptr;
    ByteArray m_samples;
    Clock m_clock;
    std::atomic<uint64_t> m_offset;
};

} /* namespace macos */
} /* namespace nanoem */
