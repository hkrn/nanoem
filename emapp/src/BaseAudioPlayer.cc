/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/BaseAudioPlayer.h"

#include "emapp/Constants.h"
#include "emapp/Error.h"
#include "emapp/FileUtils.h"
#include "emapp/ListUtils.h"
#include "emapp/URI.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {

const size_t BaseAudioPlayer::kRIFFTagSize = 44;

static const nanoem_u8_t *
findLinearPCMSamplesPayload(const nanoem_u8_t *dataPtr, nanoem_rsize_t dataSize, nanoem_rsize_t &payloadSize)
{
    const nanoem_u8_t *newDataPtr = nullptr;
    BaseAudioPlayer::Chunk chunk = { 0, 0 };
    nanoem_rsize_t offset = 0;
    payloadSize = 0;
    while (chunk.m_size < dataSize && (offset + sizeof(chunk)) < dataSize) {
        chunk = *reinterpret_cast<const BaseAudioPlayer::Chunk *>(dataPtr + offset);
        if (chunk.m_id == nanoem_fourcc('d', 'a', 't', 'a')) {
            newDataPtr = dataPtr + offset + sizeof(chunk);
            if (chunk.m_size + offset + sizeof(chunk) <= dataSize) {
                payloadSize = chunk.m_size;
                break;
            }
            /* workaround for archived wav data older than 24.1 */
            else if (dataSize == chunk.m_size - 32) {
                payloadSize = dataSize;
                break;
            }
        }
        offset += chunk.m_size + sizeof(chunk);
    }
    return newDataPtr;
}

bool
BaseAudioPlayer::containsRIFFTag(const nanoem_u8_t *ptr, size_t size) NANOEM_DECL_NOEXCEPT
{
    static const nanoem_u32_t kRIFFTag = nanoem_fourcc('R', 'I', 'F', 'F');
    return size > 8 && memcmp(ptr, &kRIFFTag, sizeof(kRIFFTag)) == 0 &&
        *reinterpret_cast<const nanoem_u32_t *>(ptr + sizeof(kRIFFTag)) == size - sizeof(kRIFFTag);
}

void
BaseAudioPlayer::initializeDescription(nanoem_u8_t bits, nanoem_u16_t channels, nanoem_u32_t sampleRate,
    nanoem_rsize_t size, WAVDescription &output) NANOEM_DECL_NOEXCEPT
{
    Inline::clearZeroMemory(output);
    bits = glm::max(bits, nanoem_u8_t(8)) & ~0x7;
    channels = glm::max(channels, nanoem_u16_t(1));
    sampleRate = glm::max(sampleRate, nanoem_u32_t(1));
    const nanoem_u16_t bytesPerPakcet = nanoem_u16_t((bits / 8) * channels);
    output.m_header.m_riffChunk.m_id = nanoem_fourcc('R', 'I', 'F', 'F');
    output.m_header.m_riffChunk.m_size = Inline::saturateInt32U(size + sizeof(output) - sizeof(output.m_header.m_riffChunk.m_id));
    output.m_header.m_wave = nanoem_fourcc('W', 'A', 'V', 'E');
    output.m_formatChunk.m_id = nanoem_fourcc('f', 'm', 't', ' ');
    output.m_formatChunk.m_size = sizeof(output.m_formatData);
    output.m_formatData.m_audioFormat = 1;
    output.m_formatData.m_numChannels = channels;
    output.m_formatData.m_sampleRate = sampleRate;
    output.m_formatData.m_bytesPerSecond = bytesPerPakcet * sampleRate;
    output.m_formatData.m_bytesPerPacket = bytesPerPakcet;
    output.m_formatData.m_bitsPerSample = nanoem_u16_t(bits);
}

void
BaseAudioPlayer::initializeDescription(
    const IAudioPlayer *player, nanoem_rsize_t size, WAVDescription &output) NANOEM_DECL_NOEXCEPT
{
    return initializeDescription(player->bitsPerSample(), player->numChannels(), player->sampleRate(), size, output);
}

StringList
BaseAudioPlayer::loadableExtensions()
{
    static const String kLoadableModelExtensions[] = { String("wav"), String() };
    return StringList(
        &kLoadableModelExtensions[0], &kLoadableModelExtensions[BX_COUNTOF(kLoadableModelExtensions) - 1]);
}

StringSet
BaseAudioPlayer::loadableExtensionsSet()
{
    return ListUtils::toSetFromList<String>(loadableExtensions());
}

bool
BaseAudioPlayer::isLoadableExtension(const String &extension)
{
    return FileUtils::isLoadableExtension(extension, loadableExtensionsSet());
}

bool
BaseAudioPlayer::isLoadableExtension(const URI &fileURI)
{
    return isLoadableExtension(fileURI.pathExtension());
}

BaseAudioPlayer::BaseAudioPlayer()
    : m_volumeGain(0, 1, 1)
    , m_state(kInitialState, kInitialState)
    , m_finished(false)
    , m_loaded(false)
{
    Inline::clearZeroMemory(m_description);
    Inline::clearZeroMemory(m_currentRational);
    Inline::clearZeroMemory(m_lastRational);
    Inline::clearZeroMemory(m_durationRational);
    m_currentRational.m_denominator = 1;
}

bool
BaseAudioPlayer::load(const ByteArray &bytes, Error &error)
{
    const WAVDescription *descPtr;
    bool succeeded = false;
    if (bytes.size() >= sizeof(*descPtr)) {
        descPtr = reinterpret_cast<const WAVDescription *>(bytes.data());
        const nanoem_u8_t *bytesPtr = bytes.data() + sizeof(WAVHeader);
        const size_t bytesSize = bytes.size() - sizeof(WAVHeader);
        nanoem_rsize_t samplesSize;
        if (const nanoem_u8_t *samplesPtr = findLinearPCMSamplesPayload(bytesPtr, bytesSize, samplesSize)) {
            const Format &format = descPtr->m_formatData;
            initializeDescription(
                format.m_bitsPerSample, format.m_numChannels, format.m_sampleRate, bytesSize, m_description);
            succeeded = loadAllLinearPCMSamples(samplesPtr, samplesSize, error);
        }
    }
    return succeeded;
}

bool
BaseAudioPlayer::load(const ByteArray &bytes, const WAVDescription &desc, Error &error)
{
    m_description = desc;
    return loadAllLinearPCMSamples(bytes.data(), bytes.size(), error);
}

void
BaseAudioPlayer::play()
{
    if (m_state.first > kInitialState && m_state.first < kPlayState) {
        Error error;
        m_finished = false;
        internalTransitStateStarted(error);
        setState(kPlayState);
    }
}

void
BaseAudioPlayer::pause()
{
    if (isPlaying()) {
        Error error;
        internalTransitStatePaused(error);
        if (m_state.first > kInitialState) {
            setState(kPauseState);
        }
    }
}

void
BaseAudioPlayer::resume()
{
    Error error;
    internalTransitStateResumed(error);
    setState(kPlayState);
}

void
BaseAudioPlayer::suspend()
{
    setState(kSuspendState);
}

void
BaseAudioPlayer::stop()
{
    if (isPlaying() || isPaused()) {
        Error error;
        internalTransitStateStopped(error);
        if (m_state.first > kInitialState) {
            setState(kStopState);
        }
    }
}

const ByteArray *
BaseAudioPlayer::linearPCMSamples() const NANOEM_DECL_NOEXCEPT
{
    return &m_linearPCMSamples;
}

URI
BaseAudioPlayer::fileURI() const
{
    return m_fileURI;
}

void
BaseAudioPlayer::setFileURI(const URI &value)
{
    m_fileURI = value;
}

Vector2
BaseAudioPlayer::volumeGainRange() const NANOEM_DECL_NOEXCEPT
{
    return m_volumeGain;
}

BaseAudioPlayer::Rational
BaseAudioPlayer::currentRational() const NANOEM_DECL_NOEXCEPT
{
    return m_currentRational;
}

BaseAudioPlayer::Rational
BaseAudioPlayer::lastRational() const NANOEM_DECL_NOEXCEPT
{
    return m_lastRational;
}

BaseAudioPlayer::Rational
BaseAudioPlayer::durationRational() const NANOEM_DECL_NOEXCEPT
{
    return m_durationRational;
}

nanoem_u32_t
BaseAudioPlayer::bitsPerSample() const NANOEM_DECL_NOEXCEPT
{
    return glm::max(m_description.m_formatData.m_bitsPerSample >> 3, 1) << 3;
}

nanoem_u32_t
BaseAudioPlayer::numChannels() const NANOEM_DECL_NOEXCEPT
{
    return glm::max(static_cast<nanoem_u32_t>(m_description.m_formatData.m_numChannels), 1u);
}

nanoem_u32_t
BaseAudioPlayer::sampleRate() const NANOEM_DECL_NOEXCEPT
{
    return glm::max(m_description.m_formatData.m_sampleRate, 1u);
}

bool
BaseAudioPlayer::isLoaded() const NANOEM_DECL_NOEXCEPT
{
    return m_loaded;
}

bool
BaseAudioPlayer::isFinished() const NANOEM_DECL_NOEXCEPT
{
    return m_finished;
}

bool
BaseAudioPlayer::isPlaying() const NANOEM_DECL_NOEXCEPT
{
    return m_state.first == kPlayState;
}

bool
BaseAudioPlayer::isPaused() const NANOEM_DECL_NOEXCEPT
{
    return m_state.first == kPauseState;
}

bool
BaseAudioPlayer::isStopped() const NANOEM_DECL_NOEXCEPT
{
    return m_state.first == kStopState;
}

bool
BaseAudioPlayer::wasPlaying() const NANOEM_DECL_NOEXCEPT
{
    return m_state.first != kSuspendState && m_state.second == kPlayState;
}

nanoem_f32_t
BaseAudioPlayer::volumeGain() const NANOEM_DECL_NOEXCEPT
{
    return m_volumeGain.z;
}

void
BaseAudioPlayer::setVolumeGain(nanoem_f32_t value)
{
    if (value != m_volumeGain.z) {
        Error error;
        value = glm::clamp(value, m_volumeGain.x, m_volumeGain.y);
        internalSetVolumeGain(value, error);
        m_volumeGain.z = value;
    }
}

void
BaseAudioPlayer::reset()
{
    Inline::clearZeroMemory(m_description);
    Inline::clearZeroMemory(m_currentRational);
    Inline::clearZeroMemory(m_lastRational);
    Inline::clearZeroMemory(m_durationRational);
    setState(kInitialState);
    m_linearPCMSamples.clear();
    m_currentRational.m_denominator = 1;
    m_loaded = false;
    m_finished = false;
}

void
BaseAudioPlayer::setState(State value)
{
    if (m_state.first > kInitialState) {
        m_state.second = m_state.first;
        m_state.first = value;
    }
}

} /* namespace nanoem */
