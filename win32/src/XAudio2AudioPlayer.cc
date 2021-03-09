/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "XAudio2AudioPlayer.h"

#include "COMInline.h"
#include "emapp/Constants.h"
#include "emapp/Error.h"
#include "emapp/IEventPublisher.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace win32 {

XAudio2AudioPlayer::XAudio2AudioPlayer(IEventPublisher *eventPublisher)
    : m_eventPublisher(eventPublisher)
{
}

XAudio2AudioPlayer::~XAudio2AudioPlayer()
{
}

bool
XAudio2AudioPlayer::initialize(nanoem_frame_index_t duration, nanoem_u32_t sampleRate, Error &error)
{
    if (m_state.first == kInitialState) {
    }
    return false;
}

void
XAudio2AudioPlayer::expandDuration(nanoem_frame_index_t frameIndex)
{
    Error error;
    if (isPlaying()) {
        internalTransitStateStopped(error);
    }
    if (isLoaded()) {
        const Format &format = m_description.m_formatData;
        const nanoem_rsize_t size = frameIndex *
            static_cast<nanoem_rsize_t>(format.m_sampleRate / Constants::kHalfBaseFPS) *
            static_cast<nanoem_rsize_t>(format.m_bytesPerPacket);
        m_linearPCMSamples.resize(nanoem_rsize_t(size * 1.5));
    }
    error.notify(m_eventPublisher);
}

void
XAudio2AudioPlayer::destroy()
{
    setState(kInitialState);
    m_finished = false;
}

bool
XAudio2AudioPlayer::loadAllLinearPCMSamples(const nanoem_u8_t *data, size_t size, Error &error)
{
    bool succeeded = false;
    internalTransitStateStopped(error);
    return succeeded;
}

void
XAudio2AudioPlayer::playPart(double, double)
{
}

void
XAudio2AudioPlayer::update()
{
    const nanoem_u64_t offset = m_offset / m_description.m_formatData.m_bytesPerPacket;
    m_lastRational = m_currentRational;
    m_currentRational.m_numerator = offset;
}

void
XAudio2AudioPlayer::seek(const IAudioPlayer::Rational &value)
{
    Error error;
    value.m_numerator > 0 ? internalTransitStatePaused(error) : internalTransitStateStopped(error);
    const Format &format = m_description.m_formatData;
    const nanoem_u32_t denominator = format.m_sampleRate / Constants::kHalfBaseFPS;
    m_offset = static_cast<nanoem_u64_t>(format.m_bytesPerPacket);
    m_finished = false;
    error.notify(m_eventPublisher);
}

void
XAudio2AudioPlayer::internalSetVolumeGain(float value, Error &error)
{
    BX_UNUSED_2(value, error);
}

void
XAudio2AudioPlayer::internalTransitStateStarted(Error &error)
{
    BX_UNUSED_1(error);
}

void
XAudio2AudioPlayer::internalTransitStatePaused(Error &error)
{
    BX_UNUSED_1(error);
}

void
XAudio2AudioPlayer::internalTransitStateResumed(Error &error)
{
    BX_UNUSED_1(error);
}

void
XAudio2AudioPlayer::internalTransitStateStopped(Error &error)
{
    BX_UNUSED_1(error);
}

} /* namespace win32 */
} /* namespace nanoem */
