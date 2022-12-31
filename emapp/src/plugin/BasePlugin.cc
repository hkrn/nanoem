/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/plugin/BasePlugin.h"

#include "emapp/Error.h"
#include "emapp/IAudioPlayer.h"
#include "emapp/ICamera.h"
#include "emapp/IEventPublisher.h"
#include "emapp/ILight.h"
#include "emapp/StringUtils.h"
#include "emapp/sdk/Common.h"

#include "../protoc/plugin.pb-c.h"

namespace nanoem {
namespace plugin {

BasePlugin::BasePlugin(IEventPublisher *publisher)
    : m_eventPublisher(publisher)
    , m_handle(0)
{
}

BasePlugin::~BasePlugin() NANOEM_DECL_NOEXCEPT
{
}

void
BasePlugin::encodeAudioDescription(const IAudioPlayer *player, ByteArray &bytes)
{
    Nanoem__Application__Plugin__AudioDescription desc = NANOEM__APPLICATION__PLUGIN__AUDIO_DESCRIPTION__INIT;
    desc.sample_rate = player->sampleRate();
    desc.volume_gain = player->volumeGain();
    desc.num_channels = player->numChannels();
    desc.bits_per_sample = player->bitsPerSample();
    Nanoem__Common__Rational current = NANOEM__COMMON__RATIONAL__INIT, duration = NANOEM__COMMON__RATIONAL__INIT;
    const IAudioPlayer::Rational &cr = player->currentRational();
    current.denominator = cr.m_denominator;
    current.numerator = cr.m_numerator;
    const IAudioPlayer::Rational &dr = player->durationRational();
    duration.denominator = dr.m_denominator;
    duration.numerator = dr.m_numerator;
    desc.current_offset = &current;
    desc.duration = &duration;
    bytes.resize(nanoem__application__plugin__audio_description__get_packed_size(&desc));
    nanoem__application__plugin__audio_description__pack(&desc, bytes.data());
}

void
BasePlugin::encodeCameraDescription(const ICamera *camera, ByteArray &bytes)
{
    Nanoem__Application__Plugin__CameraDescription desc = NANOEM__APPLICATION__PLUGIN__CAMERA_DESCRIPTION__INIT;
    Nanoem__Common__Vector4 lookAt, angle;
    encodeVector4(camera->lookAt(), &lookAt);
    encodeVector4(camera->angle(), &angle);
    desc.look_at = &lookAt;
    desc.angle = &angle;
    desc.fov = camera->fovRadians();
    desc.distance = camera->distance();
    desc.is_perspective = camera->isPerspective() ? 1 : 0;
    bytes.resize(nanoem__application__plugin__camera_description__get_packed_size(&desc));
    nanoem__application__plugin__camera_description__pack(&desc, bytes.data());
}

void
BasePlugin::encodeLightDescription(const ILight *light, ByteArray &bytes)
{
    Nanoem__Application__Plugin__LightDescription desc = NANOEM__APPLICATION__PLUGIN__LIGHT_DESCRIPTION__INIT;
    Nanoem__Common__Vector4 color, direction;
    encodeVector4(light->color(), &color);
    encodeVector4(light->direction(), &direction);
    desc.color = &color;
    desc.direction = &direction;
    bytes.resize(nanoem__application__plugin__light_description__get_packed_size(&desc));
    nanoem__application__plugin__light_description__pack(&desc, bytes.data());
}

void
BasePlugin::encodeVector4(const Vector3 &value, void *dest)
{
    Nanoem__Common__Vector4 *v = static_cast<Nanoem__Common__Vector4 *>(dest);
    nanoem__common__vector4__init(v);
    v->x = value.x;
    v->y = value.y;
    v->z = value.z;
    v->w = 1.0f;
}

bool
BasePlugin::isABICompatible(nanoem_u32_t version, nanoem_u32_t major) NANOEM_DECL_NOEXCEPT
{
    return version >= NANOEM_APPLICATION_PLUGIN_MAKE_ABI_VERSION(major, 0) &&
        version < NANOEM_APPLICATION_PLUGIN_MAKE_ABI_VERSION(major + 1, 0);
}

void
BasePlugin::handlePluginStatus(int result, Error &error)
{
    if (!error.hasReason()) {
        char reason[Error::kMaxReasonLength];
        switch (result) {
        case NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT: {
            StringUtils::format(reason, sizeof(reason), "The plugin %s tried passing nullptr", m_name.c_str());
            error = Error(reason, result, Error::kDomainTypePlugin);
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_REFER_REASON: {
            error = Error(failureReason(), recoverySuggestion(), Error::kDomainTypePlugin);
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_UNKNOWN_OPTION: {
            StringUtils::format(reason, sizeof(reason), "The plugin %s cannot recognize option", m_name.c_str());
            error = Error(reason, result, Error::kDomainTypePlugin);
            break;
        }
        default: {
            StringUtils::format(
                reason, sizeof(reason), "The plugin %s received unknown error %d", m_name.c_str(), result);
            error = Error(reason, result, Error::kDomainTypePlugin);
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS:
            break;
        }
    }
}

} /* namespace plugin */
} /* namespace nanoem */
