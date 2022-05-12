/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_WIN32_WASAPIAUDIOPLAYER_H_
#define NANOEM_EMAPP_WIN32_WASAPIAUDIOPLAYER_H_

#include "emapp/BaseAudioPlayer.h"

#include <atomic>

#include <audiopolicy.h>
#include <mmdeviceapi.h>

namespace nanoem {

class IEventPublisher;

namespace win32 {

class WASAPIAudioPlayer : public BaseAudioPlayer {
public:
    WASAPIAudioPlayer(IEventPublisher *eventPublisher);
    ~WASAPIAudioPlayer() override;

    bool initialize(nanoem_frame_index_t duration, nanoem_u32_t sampleRate, Error &error) override;
    void expandDuration(nanoem_frame_index_t frameIndex) override;
    void destroy() override;

    bool loadAllLinearPCMSamples(const nanoem_u8_t *data, size_t size, Error &error) override;
    void playPart(double start, double length) override;
    void update() override;
    void seek(const Rational &value) override;

private:
    static const uint64_t kNanoSecondsUnit = 10000000;
    static const uint32_t kSmoothnessScaleFactor = 16;

    struct AudioSessionEventsHandler : IAudioSessionEvents {
        AudioSessionEventsHandler(WASAPIAudioPlayer *parent);
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) override;
        ULONG STDMETHODCALLTYPE AddRef() override;
        ULONG STDMETHODCALLTYPE Release() override;
        HRESULT STDMETHODCALLTYPE OnDisplayNameChanged(LPCWSTR newDisplayName, LPCGUID eventContext) override;
        HRESULT STDMETHODCALLTYPE OnIconPathChanged(LPCWSTR newIconPath, LPCGUID eventContext) override;
        HRESULT STDMETHODCALLTYPE OnSimpleVolumeChanged(float newVolume, BOOL NewMute, LPCGUID eventContext) override;
        HRESULT STDMETHODCALLTYPE OnChannelVolumeChanged(
            DWORD channelCount, float NewChannelVolumeArray[], DWORD ChangedChannel, LPCGUID eventContext) override;
        HRESULT STDMETHODCALLTYPE OnGroupingParamChanged(LPCGUID newGroupingParam, LPCGUID eventContext) override;
        HRESULT STDMETHODCALLTYPE OnStateChanged(AudioSessionState newState) override;
        HRESULT STDMETHODCALLTYPE OnSessionDisconnected(AudioSessionDisconnectReason disconnectReason) override;
        WASAPIAudioPlayer *m_parent;
    };
    struct NotificationClient : IMMNotificationClient {
        NotificationClient(WASAPIAudioPlayer *parent);
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) override;
        ULONG STDMETHODCALLTYPE AddRef() override;
        ULONG STDMETHODCALLTYPE Release() override;
        HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState) override;
        HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId) override;
        HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId) override;
        HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(
            EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId) override;
        HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key) override;
        WASAPIAudioPlayer *m_parent;
    };
    enum RequestState {
        kRequestStateNone,
        kRequestStatePause,
        kRequestStateRestart,
    };

    void internalSetVolumeGain(float value, Error &error) override;
    void internalTransitStateStarted(Error &error) override;
    void internalTransitStatePaused(Error &error) override;
    void internalTransitStateResumed(Error &error) override;
    void internalTransitStateStopped(Error &error) override;
    bool initializeClient(IMMDevice *device, Error &error);
    void destroyClient();
    void resampleAllLinearPCMSamples(const nanoem_u8_t *data, size_t size, const WAVEFORMATEX *format, Error &error);
    void createAudioRenderThread();
    void createNullRenderThread();
    void renderAudioBuffer(nanoem_rsize_t bytesPerPacket, uint32_t numAvailablePackets, bool enableOffset);
    void renderNullBuffer(nanoem_rsize_t bytesPerPacket, uint32_t numAvailablePackets, bool enableOffset);
    void destroyRenderThread();

    IEventPublisher *m_eventPublisher = nullptr;
    IMMDeviceEnumerator *m_enumerator = nullptr;
    IAudioClient *m_client = nullptr;
    IAudioRenderClient *m_renderer = nullptr;
    IAudioStreamVolume *m_volume = nullptr;
    IAudioSessionControl *m_control = nullptr;
    HANDLE m_eventHandle = nullptr;
    HANDLE m_threadHandle = nullptr;
    AudioSessionEventsHandler m_audioSessionEventsHandler;
    NotificationClient m_notificationClient;
    Clock m_clock;
    ByteArray m_resampledAudioSamples;
    WAVEFORMATEX m_nativeOutputDescription = {};
    std::atomic<uint64_t> m_offset;
    std::atomic<uint64_t> m_numProceededPackets;
    std::atomic<RequestState> m_requestState;
    uint32_t m_numBufferPackets = 0;
    volatile bool m_running = true;
};

} /* namespace win32 */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_WIN32_WASAPIAUDIOPLAYER_H_ */
