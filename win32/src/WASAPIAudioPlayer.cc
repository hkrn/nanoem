/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "WASAPIAudioPlayer.h"

#include <Mferror.h>
#include <mfapi.h>
#include <mftransform.h>
#include <wmcodecdsp.h>

#include "COMInline.h"
#include "emapp/Constants.h"
#include "emapp/Error.h"
#include "emapp/IEventPublisher.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace win32 {
namespace {

static const DWORD kRenderAudioEventTimeoutMillis = 1000;
static const uint64_t kClockLatencyThresholdFPSCount = 15ull;

} /* namespace anonymous */

WASAPIAudioPlayer::WASAPIAudioPlayer(IEventPublisher *eventPublisher)
    : m_eventPublisher(eventPublisher)
    , m_audioSessionEventsHandler(this)
    , m_notificationClient(this)
    , m_offset(0)
    , m_numProceededPackets(0)
    , m_requestState(kRequestStateNone)
{
    Error error;
    m_eventHandle = CreateEventW(nullptr, false, false, L"WASAPIAudioPlayer::m_eventHandle");
    COMInline::wrapCall(
        CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_enumerator)),
        error);
    if (m_enumerator) {
        COMInline::wrapCall(m_enumerator->RegisterEndpointNotificationCallback(&m_notificationClient), error);
    }
    error.notify(eventPublisher);
}

WASAPIAudioPlayer::~WASAPIAudioPlayer()
{
    destroy();
    if (m_enumerator) {
        m_enumerator->UnregisterEndpointNotificationCallback(&m_notificationClient);
    }
    if (m_eventHandle) {
        CloseHandle(m_eventHandle);
        m_eventHandle = nullptr;
    }
    COMInline::safeRelease(m_enumerator);
}

bool
WASAPIAudioPlayer::initialize(nanoem_frame_index_t duration, nanoem_u32_t sampleRate, Error &error)
{
    if (m_state.first == kInitialState && m_enumerator) {
        IMMDevice *device = nullptr;
        COMInline::wrapCall(m_enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device), error);
        nanoem_u32_t actualSampleRate = sampleRate * kSmoothnessScaleFactor;
        m_linearPCMSamples.resize((static_cast<nanoem_rsize_t>(duration) + 1) * actualSampleRate);
        initializeDescription(8, 1, actualSampleRate, m_linearPCMSamples.size(), m_description);
        if (!error.hasReason() && initializeClient(device, error)) {
            createNullRenderThread();
        }
        COMInline::safeRelease(device);
        m_offset = 0;
        m_numProceededPackets = 0;
        m_currentRational.m_denominator = m_nativeOutputDescription.nSamplesPerSec;
        m_state.first = kStopState;
    }
    return !error.hasReason();
}

void
WASAPIAudioPlayer::expandDuration(nanoem_frame_index_t frameIndex)
{
    if (isPlaying()) {
        m_client->Stop();
    }
    if (isLoaded()) {
        const nanoem_rsize_t size = static_cast<nanoem_rsize_t>(frameIndex) *
            (m_nativeOutputDescription.nSamplesPerSec / Constants::kHalfBaseFPS) *
            m_nativeOutputDescription.nBlockAlign;
        if (size > m_resampledAudioSamples.size()) {
            m_resampledAudioSamples.resize(nanoem_rsize_t(size * 1.5));
        }
    }
}

void
WASAPIAudioPlayer::destroy()
{
    destroyClient();
    reset();
}

bool
WASAPIAudioPlayer::loadAllLinearPCMSamples(const nanoem_u8_t *data, size_t size, Error &error)
{
    IMMDevice *device;
    bool succeeded = false;
    internalTransitStateStopped(error);
    if (m_enumerator) {
        COMInline::wrapCall(m_enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device), error);
        if (initializeClient(device, error) && !error.hasReason()) {
            WAVEFORMATEX *mixFormat = nullptr;
            m_client->GetMixFormat(&mixFormat);
            resampleAllLinearPCMSamples(data, size, mixFormat, error);
            m_durationRational.m_numerator = m_resampledAudioSamples.size();
            m_durationRational.m_denominator =
                m_nativeOutputDescription.nBlockAlign * m_nativeOutputDescription.nSamplesPerSec;
            m_running = true;
            m_linearPCMSamples.assign(data, data + size);
            m_offset = 0;
            m_numProceededPackets = 0;
            m_loaded = succeeded = true;
            createAudioRenderThread();
        }
        COMInline::safeRelease(device);
    }
    return succeeded;
}

void
WASAPIAudioPlayer::playPart(double, double)
{
}

void
WASAPIAudioPlayer::update()
{
    const nanoem_u64_t audioSampleOffset = static_cast<nanoem_u64_t>(m_offset / m_nativeOutputDescription.nBlockAlign),
                       clockSampleOffset =
                           static_cast<nanoem_u64_t>(m_clock.seconds() * m_nativeOutputDescription.nSamplesPerSec),
                       clockSampleLatencyThreshold =
                           (m_nativeOutputDescription.nSamplesPerSec / 60) * kClockLatencyThresholdFPSCount,
                       clockSampleLatency =
                           (clockSampleOffset > audioSampleOffset ? clockSampleOffset - audioSampleOffset
                                                                  : audioSampleOffset - clockSampleOffset) *
        (audioSampleOffset > 0);
    m_lastRational = m_currentRational;
    if (m_client) {
        nanoem_u64_t value;
        if (clockSampleLatency < clockSampleLatencyThreshold) {
            value = clockSampleOffset;
        }
        else {
            value = audioSampleOffset + clockSampleLatencyThreshold;
        }
        m_currentRational.m_numerator = value;
    }
    else {
        const nanoem_u64_t offset = static_cast<nanoem_u64_t>(m_clock.seconds() * m_currentRational.m_denominator);
        m_currentRational.m_numerator = offset;
    }
    const RequestState state = m_requestState;
    if (state > kRequestStateNone && m_enumerator) {
        Error error;
        IMMDevice *device = nullptr;
        pause();
        COMInline::wrapCall(m_enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device), error);
        if (initializeClient(device, error)) {
            m_running = true;
            isLoaded() ? createAudioRenderThread() : createNullRenderThread();
            if (state > kRequestStatePause) {
                resume();
            }
        }
        error.notify(m_eventPublisher);
        m_requestState = kRequestStateNone;
    }
}

void
WASAPIAudioPlayer::seek(const IAudioPlayer::Rational &value)
{
    Error error;
    const nanoem_u64_t numProceededPackets = value.m_numerator,
                       offset = numProceededPackets * static_cast<nanoem_i64_t>(m_nativeOutputDescription.nBlockAlign);
    m_clock.seek(value);
    m_numProceededPackets = numProceededPackets;
    m_offset = offset;
    offset > 0 ? internalTransitStatePaused(error) : internalTransitStateStopped(error);
    error.notify(m_eventPublisher);
}

bool
WASAPIAudioPlayer::initializeClient(IMMDevice *device, Error &error)
{
    const REFERENCE_TIME bufferDuration = static_cast<REFERENCE_TIME>(kNanoSecondsUnit / 10); /* 100msecs */
    const nanoem_u32_t flags = 0 | AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM |
        AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY;
    IAudioClient *client = nullptr;
    bool succeeded = false;
    COMInline::wrapCall(
        device->Activate(__uuidof(IAudioClient), CLSCTX_INPROC_SERVER, nullptr, reinterpret_cast<void **>(&client)),
        error);
    if (client) {
        WAVEFORMATEX *mixedFormat = nullptr;
        client->GetMixFormat(&mixedFormat);
        COMInline::wrapCall(
            client->Initialize(AUDCLNT_SHAREMODE_SHARED, flags, bufferDuration, 0, mixedFormat, nullptr), error);
        COMInline::wrapCall(client->GetBufferSize(&m_numBufferPackets), error);
        COMInline::wrapCall(client->SetEventHandle(m_eventHandle), error);
        succeeded = !error.hasReason();
        if (succeeded) {
            destroyClient();
            COMInline::wrapCall(client->GetService(IID_PPV_ARGS(&m_renderer)), error);
            COMInline::wrapCall(client->GetService(IID_PPV_ARGS(&m_volume)), error);
            COMInline::wrapCall(client->GetService(IID_PPV_ARGS(&m_control)), error);
            m_control->RegisterAudioSessionNotification(&m_audioSessionEventsHandler);
            m_nativeOutputDescription = *mixedFormat;
            m_client = client;
        }
        else {
            COMInline::safeRelease(client);
        }
    }
    return succeeded;
}

void
WASAPIAudioPlayer::destroyClient()
{
    if (m_client) {
        m_client->Stop();
    }
    if (m_control) {
        m_control->UnregisterAudioSessionNotification(&m_audioSessionEventsHandler);
    }
    destroyRenderThread();
    COMInline::safeRelease(m_control);
    COMInline::safeRelease(m_renderer);
    COMInline::safeRelease(m_volume);
    COMInline::safeRelease(m_client);
}

void
WASAPIAudioPlayer::resampleAllLinearPCMSamples(
    const nanoem_u8_t *data, size_t size, const WAVEFORMATEX *format, Error &error)
{
    IMFTransform *transform = nullptr;
    IUnknown *resamplerSink = nullptr;
    CoCreateInstance(CLSID_CResamplerMediaObject, nullptr, CLSCTX_INPROC_SERVER, IID_IUnknown,
        reinterpret_cast<void **>(&resamplerSink));
    resamplerSink->QueryInterface(IID_PPV_ARGS(&transform));
    IWMResamplerProps *props;
    transform->QueryInterface(IID_PPV_ARGS(&props));
    props->SetHalfFilterLength(60);
    IMFMediaType *inputType = nullptr;
    WAVEFORMATEXTENSIBLE inputFormatEX = {};
    auto &inputFormat = inputFormatEX.Format;
    inputFormatEX.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
    inputFormat.cbSize = 22;
    inputFormat.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    inputFormat.nChannels = m_description.m_formatData.m_numChannels;
    inputFormat.nSamplesPerSec = m_description.m_formatData.m_sampleRate;
    inputFormat.nBlockAlign = m_description.m_formatData.m_bytesPerPacket;
    inputFormat.nAvgBytesPerSec = m_description.m_formatData.m_bytesPerSecond;
    inputFormat.wBitsPerSample = m_description.m_formatData.m_bitsPerSample;
    MFCreateMediaType(&inputType);
    MFInitMediaTypeFromWaveFormatEx(inputType, &inputFormat, sizeof(inputFormatEX));
    transform->SetInputType(0, inputType, 0);
    IMFMediaType *outputType = nullptr;
    MFCreateMediaType(&outputType);
    MFInitMediaTypeFromWaveFormatEx(outputType, format, sizeof(inputFormatEX));
    transform->SetOutputType(0, outputType, 0);
    inputType->Release();
    outputType->Release();
    transform->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, NULL);
    transform->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, NULL);
    transform->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, NULL);
    const nanoem_rsize_t bufferLength = m_description.m_formatData.m_bytesPerSecond;
    const DWORD bufferLengthI32 = Inline::saturateInt32U(bufferLength);
    nanoem_rsize_t offset = 0;
    DWORD status = 0;
    HRESULT rc = S_OK;
    while (size - offset > 0) {
        IMFMediaBuffer *inputBuffer = nullptr;
        MFCreateMemoryBuffer(bufferLengthI32, &inputBuffer);
        BYTE *bytesPtr = nullptr;
        const nanoem_rsize_t copySize = glm::min(bufferLength, size - offset);
        inputBuffer->Lock(&bytesPtr, nullptr, nullptr);
        memcpy(bytesPtr, data + offset, copySize);
        offset += copySize;
        inputBuffer->Unlock();
        inputBuffer->SetCurrentLength(bufferLengthI32);
        IMFSample *inputSample = nullptr;
        MFCreateSample(&inputSample);
        inputSample->AddBuffer(inputBuffer);
        transform->ProcessInput(0, inputSample, 0);
        inputBuffer->Release();
        inputSample->Release();
        do {
            MFT_OUTPUT_DATA_BUFFER output = {};
            IMFMediaBuffer *outputBuffer = nullptr;
            MFCreateMemoryBuffer(format->nAvgBytesPerSec, &outputBuffer);
            MFCreateSample(&output.pSample);
            output.pSample->AddBuffer(outputBuffer);
            rc = transform->ProcessOutput(0, 1, &output, &status);
            if (rc != S_OK) {
                outputBuffer->Release();
                output.pSample->Release();
                break;
            }
            BYTE *bytesPtr = nullptr;
            DWORD bytesLength = 0;
            outputBuffer->Lock(&bytesPtr, nullptr, &bytesLength);
            m_resampledAudioSamples.insert(m_resampledAudioSamples.end(), bytesPtr, bytesPtr + bytesLength);
            outputBuffer->Unlock();
            outputBuffer->Release();
            output.pSample->Release();
        } while (true);
    }
    transform->ProcessMessage(MFT_MESSAGE_NOTIFY_END_OF_STREAM, NULL);
    transform->ProcessMessage(MFT_MESSAGE_COMMAND_DRAIN, NULL);
    transform->ProcessMessage(MFT_MESSAGE_NOTIFY_END_STREAMING, NULL);
    transform->Release();
}

void
WASAPIAudioPlayer::createAudioRenderThread()
{
    auto callback = [](LPVOID userData) -> DWORD {
        auto self = static_cast<WASAPIAudioPlayer *>(userData);
        if (!FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED))) {
            const nanoem_rsize_t bytesPerPacket = self->m_nativeOutputDescription.nBlockAlign;
            const nanoem_u32_t numBufferPackets = self->m_numBufferPackets;
            while (self->m_running) {
                WaitForSingleObject(self->m_eventHandle, kRenderAudioEventTimeoutMillis);
                uint32_t numPaddingPackets = 0;
                if (!FAILED(self->m_client->GetCurrentPadding(&numPaddingPackets)) &&
                    numPaddingPackets < numBufferPackets) {
                    const uint32_t numAvailablePackets = numBufferPackets - numPaddingPackets;
                    self->renderAudioBuffer(bytesPerPacket, numAvailablePackets, true);
                }
            }
            CoUninitialize();
        }
        return 0;
    };
    m_threadHandle = CreateThread(nullptr, 0, callback, this, 0, nullptr);
}

void
WASAPIAudioPlayer::createNullRenderThread()
{
    auto callback = [](LPVOID userData) -> DWORD {
        auto self = static_cast<WASAPIAudioPlayer *>(userData);
        if (!FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED))) {
            const nanoem_rsize_t bytesPerPacket = self->m_nativeOutputDescription.nBlockAlign;
            const nanoem_u32_t numBufferPackets = self->m_numBufferPackets;
            while (self->m_running) {
                WaitForSingleObject(self->m_eventHandle, kRenderAudioEventTimeoutMillis);
                uint32_t numPaddingPackets = 0;
                if (!FAILED(self->m_client->GetCurrentPadding(&numPaddingPackets)) &&
                    numPaddingPackets < numBufferPackets) {
                    const uint32_t numAvailablePackets = numBufferPackets - numPaddingPackets;
                    self->renderNullBuffer(bytesPerPacket, numAvailablePackets, true);
                }
            }
            CoUninitialize();
        }
        return 0;
    };
    m_threadHandle = CreateThread(nullptr, 0, callback, this, 0, nullptr);
}

void
WASAPIAudioPlayer::renderAudioBuffer(nanoem_rsize_t bytesPerPacket, uint32_t numAvailablePackets, bool enableOffset)
{
    const nanoem_rsize_t size =
        glm::min(numAvailablePackets * bytesPerPacket, nanoem_rsize_t(m_resampledAudioSamples.size() - m_offset));
    Error error;
    BYTE *buffer = nullptr;
    HRESULT result = m_renderer->GetBuffer(numAvailablePackets, &buffer);
    if (result == S_OK) {
        uint64_t numProceededPackets = m_numProceededPackets;
        if (buffer) {
            memcpy(buffer, m_resampledAudioSamples.data() + numProceededPackets * bytesPerPacket, size);
        }
        COMInline::wrapCall(m_renderer->ReleaseBuffer(numAvailablePackets, 0), error);
        if (enableOffset) {
            numProceededPackets += numAvailablePackets;
            m_offset = numProceededPackets * bytesPerPacket;
            m_numProceededPackets = numProceededPackets;
        }
    }
    else if (result != AUDCLNT_E_BUFFER_TOO_LARGE) {
        COMInline::wrapCall(result, error);
        m_running = false;
    }
    error.notify(m_eventPublisher);
}

void
WASAPIAudioPlayer::renderNullBuffer(nanoem_rsize_t bytesPerPacket, uint32_t numAvailablePackets, bool enableOffset)
{
    Error error;
    BYTE *buffer = nullptr;
    HRESULT result = m_renderer->GetBuffer(numAvailablePackets, &buffer);
    if (result == S_OK) {
        uint64_t numProceededPackets = m_numProceededPackets;
        if (buffer) {
            memset(buffer, 0, numAvailablePackets * bytesPerPacket);
        }
        m_renderer->ReleaseBuffer(numAvailablePackets, AUDCLNT_BUFFERFLAGS_SILENT);
        if (enableOffset) {
            numProceededPackets += numAvailablePackets;
            m_offset = numProceededPackets * bytesPerPacket;
            m_numProceededPackets = numProceededPackets;
        }
    }
    else if (result != AUDCLNT_E_BUFFER_TOO_LARGE) {
        COMInline::wrapCall(result, error);
        m_running = false;
    }
    error.notify(m_eventPublisher);
}

WASAPIAudioPlayer::AudioSessionEventsHandler::AudioSessionEventsHandler(WASAPIAudioPlayer *parent)
    : m_parent(parent)
{
}

HRESULT STDMETHODCALLTYPE
WASAPIAudioPlayer::AudioSessionEventsHandler::QueryInterface(REFIID riid, void **ppvObject)
{
    if (riid == IID_IUnknown || riid == __uuidof(IAudioSessionEvents)) {
        *ppvObject = this;
        return S_OK;
    }
    *ppvObject = nullptr;
    return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE
WASAPIAudioPlayer::AudioSessionEventsHandler::AddRef()
{
    return 1;
}

ULONG STDMETHODCALLTYPE
WASAPIAudioPlayer::AudioSessionEventsHandler::Release()
{
    return 1;
}

HRESULT STDMETHODCALLTYPE
WASAPIAudioPlayer::AudioSessionEventsHandler::OnDisplayNameChanged(LPCWSTR newDisplayName, LPCGUID eventContext)
{
    BX_UNUSED_2(newDisplayName, eventContext);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE
WASAPIAudioPlayer::AudioSessionEventsHandler::OnIconPathChanged(LPCWSTR newIconPath, LPCGUID eventContext)
{
    BX_UNUSED_2(newIconPath, eventContext);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE
WASAPIAudioPlayer::AudioSessionEventsHandler::OnSimpleVolumeChanged(float newVolume, BOOL newMute, LPCGUID eventContext)
{
    BX_UNUSED_3(newVolume, newMute, eventContext);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE
WASAPIAudioPlayer::AudioSessionEventsHandler::OnChannelVolumeChanged(
    DWORD channelCount, float newChannelVolumeArray[], DWORD ChangedChannel, LPCGUID eventContext)
{
    BX_UNUSED_4(channelCount, newChannelVolumeArray, ChangedChannel, eventContext);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE
WASAPIAudioPlayer::AudioSessionEventsHandler::OnGroupingParamChanged(LPCGUID newGroupingParam, LPCGUID eventContext)
{
    BX_UNUSED_2(newGroupingParam, eventContext);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE
WASAPIAudioPlayer::AudioSessionEventsHandler::OnStateChanged(AudioSessionState newState)
{
    BX_UNUSED_1(newState);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE
WASAPIAudioPlayer::AudioSessionEventsHandler::OnSessionDisconnected(AudioSessionDisconnectReason disconnectReason)
{
    BX_UNUSED_1(disconnectReason);
    m_parent->m_requestState = kRequestStatePause;
    return S_OK;
}

WASAPIAudioPlayer::NotificationClient::NotificationClient(WASAPIAudioPlayer *parent)
    : m_parent(parent)
{
}

HRESULT STDMETHODCALLTYPE
WASAPIAudioPlayer::NotificationClient::QueryInterface(REFIID riid, void **ppvObject)
{
    if (riid == IID_IUnknown || riid == __uuidof(IMMNotificationClient)) {
        *ppvObject = this;
        return S_OK;
    }
    *ppvObject = nullptr;
    return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE
WASAPIAudioPlayer::NotificationClient::AddRef()
{
    return 1;
}

ULONG STDMETHODCALLTYPE
WASAPIAudioPlayer::NotificationClient::Release()
{
    return 1;
}

HRESULT STDMETHODCALLTYPE
WASAPIAudioPlayer::NotificationClient::OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState)
{
    BX_UNUSED_1(pwstrDeviceId);
    if (dwNewState == DEVICE_STATE_UNPLUGGED) {
        m_parent->m_requestState = kRequestStatePause;
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE
WASAPIAudioPlayer::NotificationClient::OnDeviceAdded(LPCWSTR pwstrDeviceId)
{
    BX_UNUSED_1(pwstrDeviceId);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE
WASAPIAudioPlayer::NotificationClient::OnDeviceRemoved(LPCWSTR pwstrDeviceId)
{
    BX_UNUSED_1(pwstrDeviceId);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE
WASAPIAudioPlayer::NotificationClient::OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId)
{
    if (flow == eRender && role == eConsole) {
        RequestState expected = kRequestStateNone;
        m_parent->m_requestState.compare_exchange_strong(expected, kRequestStateRestart);
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE
WASAPIAudioPlayer::NotificationClient::OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key)
{
    BX_UNUSED_2(pwstrDeviceId, key);
    return S_OK;
}

void
WASAPIAudioPlayer::internalSetVolumeGain(float value, Error &error)
{
    if (m_volume) {
        uint32_t numChannels;
        COMInline::wrapCall(m_volume->GetChannelCount(&numChannels), error);
        for (uint32_t i = 0; i < numChannels; i++) {
            COMInline::wrapCall(m_volume->SetChannelVolume(i, value), error);
        }
    }
}

void
WASAPIAudioPlayer::internalTransitStateStarted(Error &error)
{
    if (m_client) {
        COMInline::wrapCall(m_client->Start(), error);
    }
    m_clock.start();
}

void
WASAPIAudioPlayer::internalTransitStatePaused(Error &error)
{
    if (m_client) {
        COMInline::wrapCall(m_client->Stop(), error);
        COMInline::wrapCall(m_client->Reset(), error);
    }
    m_clock.pause();
}

void
WASAPIAudioPlayer::internalTransitStateResumed(Error &error)
{
    if (m_client) {
        COMInline::wrapCall(m_client->Start(), error);
    }
    m_clock.resume();
}

void
WASAPIAudioPlayer::internalTransitStateStopped(Error &error)
{
    internalTransitStatePaused(error);
    m_clock.stop();
    m_offset = 0;
    m_numProceededPackets = 0;
}

void
WASAPIAudioPlayer::destroyRenderThread()
{
    if (m_threadHandle) {
        Error error;
        m_running = false;
        SetEvent(m_eventHandle);
        WaitForSingleObject(m_threadHandle, INFINITE);
        CloseHandle(m_threadHandle);
        m_threadHandle = nullptr;
    }
}

} /* namespace win32 */
} /* namespace nanoem */
