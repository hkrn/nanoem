/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "ApplicationService.h"

#include "emapp/IProjectHolder.h"
#include "emapp/StringUtils.h"
#include "sokol/sokol_app.h"

#include "emapp/BaseAudioPlayer.h"
#include "emapp/Error.h"

#include "sokol/sokol_audio.h"
#include <atomic>

namespace nanoem {
namespace sapp {
namespace {

static const uint64_t kClockLatencyThresholdFPSCount = 10;

struct AudioPlayer : BaseAudioPlayer {
    static const uint32_t kSmoothnessScaleFactor = 16;

    AudioPlayer()
        : BaseAudioPlayer()
        , m_offset(0)
    {
    }

    bool
    initialize(nanoem_frame_index_t /* duration */, nanoem_u32_t sampleRate, Error & /* error */) NANOEM_DECL_OVERRIDE
    {
        nanoem_u32_t actualSampleRate = sampleRate * kSmoothnessScaleFactor;
        initializeDescription(8, 1, actualSampleRate, m_linearPCMSamples.size(), m_description);
        m_currentRational.m_denominator = m_description.m_formatData.m_sampleRate;
        m_state.first = kStopState;
        return true;
    }
    void expandDuration(nanoem_frame_index_t /* frameIndex */) NANOEM_DECL_OVERRIDE
    {
        if (isPlaying()) {
            Error error;
            internalTransitStateStopped(error);
        }
    }
    void
    destroy() NANOEM_DECL_OVERRIDE
    {
        saudio_shutdown();
    }
    bool
    loadAllLinearPCMSamples(const nanoem_u8_t *data, size_t size, Error & /* error */) NANOEM_DECL_OVERRIDE
    {
        m_loaded = true;
        m_linearPCMSamples.assign(data, data + size);
        return true;
    }
    void
    playPart(double /* start */, double /* length */) NANOEM_DECL_OVERRIDE
    {
    }
    void
    update() NANOEM_DECL_OVERRIDE
    {
        const Format &format = m_description.m_formatData;
        const nanoem_u64_t audioSampleOffset = static_cast<nanoem_u64_t>(m_offset / format.m_bytesPerSecond),
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
    }
    void
    seek(const Rational &value) NANOEM_DECL_OVERRIDE
    {
        Error error;
        value.m_numerator > 0 ? internalTransitStatePaused(error) : internalTransitStateStopped(error);
        m_finished = false;
    }
    void
    internalSetVolumeGain(float /* value */, Error & /* error */) NANOEM_DECL_OVERRIDE
    {
    }
    void
    internalTransitStateStarted(Error & /* error */) NANOEM_DECL_OVERRIDE
    {
        startAudioPlayer();
        m_clock.start();
    }
    void
    internalTransitStatePaused(Error & /* error */) NANOEM_DECL_OVERRIDE
    {
        saudio_shutdown();
        m_clock.pause();
    }
    void
    internalTransitStateResumed(Error & /* error */) NANOEM_DECL_OVERRIDE
    {
        startAudioPlayer();
        m_clock.resume();
    }
    void
    internalTransitStateStopped(Error & /* error */) NANOEM_DECL_OVERRIDE
    {
        saudio_shutdown();
        m_clock.stop();
    }

    void
    startAudioPlayer()
    {
        if (!m_linearPCMSamples.empty()) {
            const Format &format = m_description.m_formatData;
            saudio_desc desc = {};
            desc.stream_userdata_cb = [](float *buffer, int numFrames, int numChannels, void *userData) {
                auto self = static_cast<AudioPlayer *>(userData);
                const Format &format = self->m_description.m_formatData;
                const uint64_t offset = self->m_offset;
                const int numSamples = numFrames * numChannels, numBytes = numSamples * (format.m_bitsPerSample / 8);
                if (offset + numBytes <= self->m_linearPCMSamples.size()) {
                    if (format.m_bitsPerSample == 16) {
                        auto data = reinterpret_cast<const uint16_t *>(&self->m_linearPCMSamples[offset]);
                        for (int i = 0; i < numSamples; i++) {
                            buffer[i] = (data[i] / 32767.0f);
                        }
                    }
                    else if (format.m_bitsPerSample == 8) {
                        auto data = reinterpret_cast<const int8_t *>(&self->m_linearPCMSamples[offset]);
                        for (int i = 0; i < numSamples; i++) {
                            buffer[i] = (data[i] / 127.0f);
                        }
                    }
                    self->m_offset += numBytes;
                }
            };
            desc.user_data = this;
            desc.num_channels = format.m_numChannels;
            desc.sample_rate = format.m_sampleRate;
            saudio_setup(&desc);
        }
    }

    Clock m_clock;
    std::atomic<uint64_t> m_offset;
};

} /* namespace anonymous */

ApplicationService::ApplicationService(const JSON_Value *config, ApplicationClient::Bridge *bridge)
    : BaseApplicationService(config)
    , m_menubarApplciationClient(bridge)
    , m_bridge(bridge)
{
}

ApplicationService::~ApplicationService() noexcept
{
}

BaseApplicationClient *
ApplicationService::menubarApplicationClient()
{
    return &m_menubarApplciationClient;
}

void
ApplicationService::draw()
{
    drawDefaultPass();
}

void
ApplicationService::consumeDispatchAllCommandMessages()
{
    Project *project = projectHolder()->currentProject();
    ByteArrayList &commands = m_bridge->m_commands;
    for (ByteArrayList::const_iterator it = commands.begin(), end = commands.end(); it != end; ++it) {
        dispatchCommandMessage(it->data(), it->size(), project, false);
    }
    commands.clear();
}

void
ApplicationService::dispatchAllEventMessages()
{
    m_menubarApplciationClient.dispatchAllEventMessages();
}

bool
ApplicationService::isRendererAvailable(const char *value) const noexcept
{
    return StringUtils::equals(value, kRendererOpenGL);
}

void
ApplicationService::handleSetupGraphicsEngine(sg_desc &desc)
{
    sg_context_desc &context = desc.context;
    context.color_format = static_cast<sg_pixel_format>(sapp_color_format());
    context.depth_format = static_cast<sg_pixel_format>(sapp_depth_format());
    context.sample_count = sapp_sample_count();
    context.gl.force_gles2 = sapp_gles2();
    context.metal.device = sapp_metal_get_device();
    context.metal.renderpass_descriptor_cb = sapp_metal_get_renderpass_descriptor;
    context.metal.drawable_cb = sapp_metal_get_drawable;
    context.d3d11.device = sapp_d3d11_get_device();
    context.d3d11.device_context = sapp_d3d11_get_device_context();
    context.d3d11.render_target_view_cb = sapp_d3d11_get_render_target_view;
    context.d3d11.depth_stencil_view_cb = sapp_d3d11_get_depth_stencil_view;
    context.wgpu.device = sapp_wgpu_get_device();
    context.wgpu.render_view_cb = sapp_wgpu_get_render_view;
    context.wgpu.resolve_view_cb = sapp_wgpu_get_resolve_view;
    context.wgpu.depth_stencil_view_cb = sapp_wgpu_get_depth_stencil_view;
}

void
ApplicationService::sendEventMessage(const Nanoem__Application__Event *event)
{
    ByteArray bytes(sizeofEventMessage(event));
    packEventMessage(event, bytes.data());
    m_bridge->m_events.push_back(bytes);
}

} /* namespace sapp */
} /* namespace nanoem */
