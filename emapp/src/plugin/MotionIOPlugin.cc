/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/plugin/MotionIOPlugin.h"

#include "emapp/Error.h"
#include "emapp/IAudioPlayer.h"
#include "emapp/IEventPublisher.h"
#include "emapp/Model.h"
#include "emapp/StringUtils.h"
#include "emapp/URI.h"
#include "emapp/private/CommonInclude.h"
#include "emapp/sdk/Motion.h"

#include "nanoem/ext/mutable.h"

namespace nanoem {
namespace plugin {

MotionIOPlugin::MotionIOPlugin(IEventPublisher *publisher)
    : BasePlugin(publisher)
    , m_motionIO(nullptr)
    , _motionIOInitialize(nullptr)
    , _motionIOCreate(nullptr)
    , _motionIOSetLanguage(nullptr)
    , _motionIOGetName(nullptr)
    , _motionIOGetDescription(nullptr)
    , _motionIOGetVersion(nullptr)
    , _motionIOCountAllFunctions(nullptr)
    , _motionIOGetFunctionName(nullptr)
    , _motionIOSetFunction(nullptr)
    , _motionIOSetAllNamedSelectedBoneKeyframes(nullptr)
    , _motionIOSetAllNamedSelectedMorphKeyframes(nullptr)
    , _motionIOSetAllSelectedAccessoryKeyframes(nullptr)
    , _motionIOSetAllSelectedCameraKeyframes(nullptr)
    , _motionIOSetAllSelectedLightKeyframes(nullptr)
    , _motionIOSetAllSelectedModelKeyframes(nullptr)
    , _motionIOSetAllSelectedSelfShadowKeyframes(nullptr)
    , _motionIOSetAudioDescription(nullptr)
    , _motionIOSetCameraDescription(nullptr)
    , _motionIOSetLightDescription(nullptr)
    , _motionIOSetInputMotionData(nullptr)
    , _motionIOSetInputActiveModelData(nullptr)
    , _motionIOSetInputAudioData(nullptr)
    , _motionIOExecute(nullptr)
    , _motionIOLoadUIWindowLayout(nullptr)
    , _motionIOGetOutputMotionDataSize(nullptr)
    , _motionIOGetOutputMotionData(nullptr)
    , _motionIOGetUIWindowLayoutDataSize(nullptr)
    , _motionIOGetUIWindowLayoutData(nullptr)
    , _motionIOSetUIComponentLayoutData(nullptr)
    , _motionIOGetFailureReason(nullptr)
    , _motionIOGetRecoverySuggestion(nullptr)
    , _motionIODestroy(nullptr)
    , _motionIOTerminate(nullptr)

{
}

MotionIOPlugin::~MotionIOPlugin() NANOEM_DECL_NOEXCEPT
{
}

bool
MotionIOPlugin::load(const URI &fileURI)
{
    bool succeeded = m_handle != nullptr;
    if (!succeeded) {
        bool valid = true;
        if (void *handle = bx::dlopen(fileURI.absolutePathConstString())) {
            PFN_nanoemApplicationPluginMotionIOGetABIVersion _motionIOGetABIVersion = nullptr;
            EMLOG_DEBUG("Loading motion I/O plugin path={} handle={}", fileURI.absolutePathConstString(), handle);
            Inline::resolveSymbol(
                handle, "nanoemApplicationPluginMotionIOGetABIVersion", _motionIOGetABIVersion, valid);
            Inline::resolveSymbol(handle, "nanoemApplicationPluginMotionIOInitialize", _motionIOInitialize, valid);
            Inline::resolveSymbol(handle, "nanoemApplicationPluginMotionIOCreate", _motionIOCreate, valid);
            Inline::resolveSymbol(handle, "nanoemApplicationPluginMotionIOGetVersion", _motionIOGetVersion, valid);
            Inline::resolveSymbol(
                handle, "nanoemApplicationPluginMotionIOCountAllFunctions", _motionIOCountAllFunctions, valid);
            Inline::resolveSymbol(
                handle, "nanoemApplicationPluginMotionIOGetFunctionName", _motionIOGetFunctionName, valid);
            Inline::resolveSymbol(handle, "nanoemApplicationPluginMotionIOSetFunction", _motionIOSetFunction, valid);
            Inline::resolveSymbol(
                handle, "nanoemApplicationPluginMotionIOSetInputMotionData", _motionIOSetInputMotionData, valid);
            Inline::resolveSymbol(handle, "nanoemApplicationPluginMotionIOExecute", _motionIOExecute, valid);
            Inline::resolveSymbol(
                handle, "nanoemApplicationPluginMotionIOLoadUIWindowLayout", _motionIOLoadUIWindowLayout, valid);
            Inline::resolveSymbol(handle, "nanoemApplicationPluginMotionIOGetOutputMotionDataSize",
                _motionIOGetOutputMotionDataSize, valid);
            Inline::resolveSymbol(
                handle, "nanoemApplicationPluginMotionIOGetOutputMotionData", _motionIOGetOutputMotionData, valid);
            Inline::resolveSymbol(
                handle, "nanoemApplicationPluginMotionIOGetFailureReason", _motionIOGetFailureReason, valid);
            Inline::resolveSymbol(
                handle, "nanoemApplicationPluginMotionIOGetRecoverySuggestion", _motionIOGetRecoverySuggestion, valid);
            Inline::resolveSymbol(handle, "nanoemApplicationPluginMotionIODestroy", _motionIODestroy, valid);
            Inline::resolveSymbol(handle, "nanoemApplicationPluginMotionIOTerminate", _motionIOTerminate, valid);
            if (valid &&
                isABICompatible(_motionIOGetABIVersion(), NANOEM_APPLICATION_PLUGIN_MOTION_ABI_VERSION_MAJOR)) {
                m_handle = handle;
                m_name = fileURI.lastPathComponent();
                Inline::resolveSymbol(
                    handle, "nanoemApplicationPluginMotionIOCreateWithLocation", _motionIOCreateWithLocation);
                Inline::resolveSymbol(handle, "nanoemApplicationPluginMotionIOSetAllNamedSelectedBoneKeyframes",
                    _motionIOSetAllNamedSelectedBoneKeyframes);
                Inline::resolveSymbol(handle, "nanoemApplicationPluginMotionIOSetAllNamedSelectedMorphKeyframes",
                    _motionIOSetAllNamedSelectedMorphKeyframes);
                Inline::resolveSymbol(handle, "nanoemApplicationPluginMotionIOSetAllSelectedAccessoryKeyframes",
                    _motionIOSetAllSelectedAccessoryKeyframes);
                Inline::resolveSymbol(handle, "nanoemApplicationPluginMotionIOSetAllSelectedCameraKeyframes",
                    _motionIOSetAllSelectedCameraKeyframes);
                Inline::resolveSymbol(handle, "nanoemApplicationPluginMotionIOSetAllSelectedLightKeyframes",
                    _motionIOSetAllSelectedLightKeyframes);
                Inline::resolveSymbol(handle, "nanoemApplicationPluginMotionIOSetAllSelectedModelKeyframes",
                    _motionIOSetAllSelectedModelKeyframes);
                Inline::resolveSymbol(handle, "nanoemApplicationPluginMotionIOSetAllSelectedSelfShadowKeyframes",
                    _motionIOSetAllSelectedSelfShadowKeyframes);
                Inline::resolveSymbol(handle, "nanoemApplicationPluginMotionIOSetLanguage", _motionIOSetLanguage);
                Inline::resolveSymbol(handle, "nanoemApplicationPluginMotionIOGetName", _motionIOGetName);
                Inline::resolveSymbol(handle, "nanoemApplicationPluginMotionIOGetDescription", _motionIOGetDescription);
                Inline::resolveSymbol(
                    handle, "nanoemApplicationPluginMotionIOSetInputActiveModelData", _motionIOSetInputActiveModelData);
                Inline::resolveSymbol(
                    handle, "nanoemApplicationPluginMotionIOSetInputAudioData", _motionIOSetInputAudioData);
                Inline::resolveSymbol(handle, "nanoemApplicationPluginMotionIOGetUIWindowLayoutDataSize",
                    _motionIOGetUIWindowLayoutDataSize);
                Inline::resolveSymbol(
                    handle, "nanoemApplicationPluginMotionIOGetUIWindowLayoutData", _motionIOGetUIWindowLayoutData);
                Inline::resolveSymbol(handle, "nanoemApplicationPluginMotionIOSetUIComponentLayoutData",
                    _motionIOSetUIComponentLayoutData);
                Inline::resolveSymbol(
                    handle, "nanoemApplicationPluginMotionIOSetAudioDescription", _motionIOSetAudioDescription);
                Inline::resolveSymbol(
                    handle, "nanoemApplicationPluginMotionIOSetCameraDescription", _motionIOSetCameraDescription);
                Inline::resolveSymbol(
                    handle, "nanoemApplicationPluginMotionIOSetLightDescription", _motionIOSetLightDescription);
                _motionIOInitialize();
                m_fileURI = fileURI;
                succeeded = true;
            }
            else {
                _motionIODestroy = nullptr;
                _motionIOTerminate = nullptr;
                bx::dlclose(handle);
            }
        }
    }
    return succeeded;
}

void
MotionIOPlugin::unload()
{
    if (_motionIOTerminate) {
        _motionIOTerminate();
        _motionIOTerminate = nullptr;
    }
    if (m_handle) {
        bx::dlclose(m_handle);
        m_handle = nullptr;
    }
}

bool
MotionIOPlugin::create()
{
    if (!m_motionIO) {
        m_motionIO = _motionIOCreateWithLocation ? _motionIOCreateWithLocation(m_fileURI.absolutePathConstString())
                                                 : _motionIOCreate();
    }
    return m_motionIO != nullptr;
}

void
MotionIOPlugin::destroy()
{
    if (_motionIODestroy) {
        _motionIODestroy(m_motionIO);
        m_motionIO = nullptr;
    }
}

void
MotionIOPlugin::setLanguage(int value)
{
    if (_motionIOSetLanguage) {
        _motionIOSetLanguage(m_motionIO, value);
    }
}

const char *
MotionIOPlugin::name() const NANOEM_DECL_NOEXCEPT
{
    const char *ptr = nullptr;
    if (_motionIOGetName) {
        ptr = _motionIOGetName(m_motionIO);
    }
    return StringUtils::length(ptr) > 0 ? ptr : m_name.c_str();
}

const char *
MotionIOPlugin::description() const NANOEM_DECL_NOEXCEPT
{
    const char *ptr = nullptr;
    if (_motionIOGetDescription) {
        ptr = _motionIOGetDescription(m_motionIO);
    }
    return ptr ? ptr : "";
}

const char *
MotionIOPlugin::version() const NANOEM_DECL_NOEXCEPT
{
    const char *ptr = _motionIOGetVersion(m_motionIO);
    return ptr ? ptr : "";
}

int
MotionIOPlugin::countAllFunctions() const NANOEM_DECL_NOEXCEPT
{
    return _motionIOCountAllFunctions(m_motionIO);
}

const char *
MotionIOPlugin::functionName(int value) const NANOEM_DECL_NOEXCEPT
{
    const char *ptr = _motionIOGetFunctionName(m_motionIO, value);
    return ptr ? ptr : "";
}

void
MotionIOPlugin::setFunction(int value, Error &error)
{
    int status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
    _motionIOSetFunction(m_motionIO, value, &status);
    handlePluginStatus(status, error);
}

void
MotionIOPlugin::setAllNamedSelectedBoneKeyframes(
    const char *name, const nanoem_frame_index_t *values, nanoem_rsize_t size, Error &error)
{
    if (_motionIOSetAllNamedSelectedBoneKeyframes) {
        int status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
        _motionIOSetAllNamedSelectedBoneKeyframes(m_motionIO, name, values, Inline::saturateInt32U(size), &status);
        handlePluginStatus(status, error);
    }
}

void
MotionIOPlugin::setAllNamedSelectedMorphKeyframes(
    const char *name, const nanoem_frame_index_t *values, nanoem_rsize_t size, Error &error)
{
    if (_motionIOSetAllNamedSelectedMorphKeyframes) {
        int status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
        _motionIOSetAllNamedSelectedMorphKeyframes(m_motionIO, name, values, Inline::saturateInt32U(size), &status);
        handlePluginStatus(status, error);
    }
}

void
MotionIOPlugin::setAllSelectedAccessoryKeyframes(const nanoem_frame_index_t *values, nanoem_rsize_t size, Error &error)
{
    if (_motionIOSetAllSelectedAccessoryKeyframes) {
        int status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
        _motionIOSetAllSelectedAccessoryKeyframes(m_motionIO, values, Inline::saturateInt32U(size), &status);
        handlePluginStatus(status, error);
    }
}

void
MotionIOPlugin::setAllSelectedCameraKeyframes(const nanoem_frame_index_t *values, nanoem_rsize_t size, Error &error)
{
    if (_motionIOSetAllSelectedCameraKeyframes) {
        int status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
        _motionIOSetAllSelectedCameraKeyframes(m_motionIO, values, Inline::saturateInt32U(size), &status);
        handlePluginStatus(status, error);
    }
}

void
MotionIOPlugin::setAllSelectedLightKeyframes(const nanoem_frame_index_t *values, nanoem_rsize_t size, Error &error)
{
    if (_motionIOSetAllSelectedLightKeyframes) {
        int status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
        _motionIOSetAllSelectedLightKeyframes(m_motionIO, values, Inline::saturateInt32U(size), &status);
        handlePluginStatus(status, error);
    }
}

void
MotionIOPlugin::setAllSelectedModelKeyframes(const nanoem_frame_index_t *values, nanoem_rsize_t size, Error &error)
{
    if (_motionIOSetAllSelectedModelKeyframes) {
        int status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
        _motionIOSetAllSelectedModelKeyframes(m_motionIO, values, Inline::saturateInt32U(size), &status);
        handlePluginStatus(status, error);
    }
}

void
MotionIOPlugin::setAllSelectedSelfShadowKeyframes(const nanoem_frame_index_t *values, nanoem_rsize_t size, Error &error)
{
    if (_motionIOSetAllSelectedSelfShadowKeyframes) {
        int status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
        _motionIOSetAllSelectedSelfShadowKeyframes(m_motionIO, values, Inline::saturateInt32U(size), &status);
        handlePluginStatus(status, error);
    }
}

void
MotionIOPlugin::setInputAudio(const IAudioPlayer *player, Error &error)
{
    if (_motionIOSetAudioDescription && _motionIOSetInputAudioData && player->isLoaded()) {
        const ByteArray *samplesPtr = player->linearPCMSamples();
        ByteArray description;
        int status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
        encodeAudioDescription(player, description);
        _motionIOSetAudioDescription(
            m_motionIO, description.data(), Inline::saturateInt32U(description.size()), &status);
        if (status == 0) {
            _motionIOSetInputAudioData(
                m_motionIO, samplesPtr->data(), Inline::saturateInt32U(samplesPtr->size()), &status);
            handlePluginStatus(status, error);
        }
    }
}

void
MotionIOPlugin::setInputCamera(const ICamera *camera, Error &error)
{
    if (_motionIOSetCameraDescription) {
        ByteArray description;
        int status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
        encodeCameraDescription(camera, description);
        _motionIOSetCameraDescription(
            m_motionIO, description.data(), Inline::saturateInt32U(description.size()), &status);
        handlePluginStatus(status, error);
    }
}

void
MotionIOPlugin::setInputLight(const ILight *light, Error &error)
{
    if (_motionIOSetLightDescription) {
        ByteArray description;
        int status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
        encodeLightDescription(light, description);
        _motionIOSetLightDescription(
            m_motionIO, description.data(), Inline::saturateInt32U(description.size()), &status);
        handlePluginStatus(status, error);
    }
}

void
MotionIOPlugin::setInputActiveModel(Model *activeModel, Error &error)
{
    ByteArray bytes;
    if (_motionIOSetInputActiveModelData && activeModel) {
        const nanoem_codec_type_t codec = nanoemModelGetCodecType(activeModel->data());
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoem_mutable_model_t *mutableModel = nanoemMutableModelCreateAsReference(activeModel->data(), &status);
        nanoemMutableModelSetCodecType(mutableModel, NANOEM_CODEC_TYPE_UTF8);
        if (activeModel->save(bytes, error)) {
            int status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
            _motionIOSetInputActiveModelData(m_motionIO, bytes.data(), Inline::saturateInt32U(bytes.size()), &status);
            handlePluginStatus(status, error);
        }
        nanoemMutableModelSetCodecType(mutableModel, codec);
        nanoemMutableModelDestroy(mutableModel);
    }
}

void
MotionIOPlugin::setInputMotionData(const ByteArray &bytes, Error &error)
{
    int status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
    _motionIOSetInputMotionData(m_motionIO, bytes.data(), Inline::saturateInt32U(bytes.size()), &status);
    handlePluginStatus(status, error);
}

bool
MotionIOPlugin::execute(Error &error)
{
    int status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
    _motionIOExecute(m_motionIO, &status);
    handlePluginStatus(status, error);
    return status == NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
}

void
MotionIOPlugin::getOutputData(ByteArray &bytes, Error &error)
{
    nanoem_u32_t size = 0;
    _motionIOGetOutputMotionDataSize(m_motionIO, &size);
    if (size > 0) {
        int status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
        bytes.resize(size);
        _motionIOGetOutputMotionData(m_motionIO, bytes.data(), size, &status);
        handlePluginStatus(status, error);
    }
}

void
MotionIOPlugin::getUIWindowLayout(ByteArray &bytes, Error &error)
{
    if (_motionIOLoadUIWindowLayout && _motionIOGetUIWindowLayoutDataSize && _motionIOGetUIWindowLayoutData) {
        int status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
        _motionIOLoadUIWindowLayout(m_motionIO, &status);
        if (status == NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS) {
            nanoem_u32_t size = 0;
            _motionIOGetUIWindowLayoutDataSize(m_motionIO, &size);
            if (size > 0) {
                bytes.resize(size);
                _motionIOGetUIWindowLayoutData(m_motionIO, bytes.data(), size, &status);
                handlePluginStatus(status, error);
            }
        }
        else {
            handlePluginStatus(status, error);
        }
    }
}

void
MotionIOPlugin::setUIComponentLayout(const char *id, const ByteArray &bytes, bool &reloadLayout, Error &error)
{
    if (_motionIOSetUIComponentLayoutData) {
        int status = 0, reload = 0;
        _motionIOSetUIComponentLayoutData(
            m_motionIO, id, bytes.data(), Inline::saturateInt32U(bytes.size()), &reload, &status);
        handlePluginStatus(status, error);
        reloadLayout = reload != 0;
    }
}

const char *
MotionIOPlugin::failureReason() const NANOEM_DECL_NOEXCEPT
{
    const char *ptr = _motionIOGetFailureReason(m_motionIO);
    return ptr ? ptr : "";
}

const char *
MotionIOPlugin::recoverySuggestion() const NANOEM_DECL_NOEXCEPT
{
    const char *ptr = _motionIOGetRecoverySuggestion(m_motionIO);
    return ptr ? ptr : "";
}

} /* namespace plugin */
} /* namespace nanoem */
