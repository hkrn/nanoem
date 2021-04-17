/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/BaseApplicationService.h"

#include "emapp/Accessory.h"
#include "emapp/AccessoryProgramBundle.h"
#include "emapp/ApplicationMenuBuilder.h"
#include "emapp/ApplicationPreference.h"
#include "emapp/BaseAudioPlayer.h"
#include "emapp/CommandRegistrator.h"
#include "emapp/DebugUtils.h"
#include "emapp/DefaultFileManager.h"
#include "emapp/Effect.h"
#include "emapp/EnumUtils.h"
#include "emapp/Error.h"
#include "emapp/FileUtils.h"
#include "emapp/Grid.h"
#include "emapp/IAudioPlayer.h"
#include "emapp/IBackgroundVideoRenderer.h"
#include "emapp/ICamera.h"
#include "emapp/IDebugCapture.h"
#include "emapp/IEventPublisher.h"
#include "emapp/IFileManager.h"
#include "emapp/ILight.h"
#include "emapp/IModelObjectSelection.h"
#include "emapp/IMotionKeyframeSelection.h"
#include "emapp/IPrimitive2D.h"
#include "emapp/ITranslator.h"
#include "emapp/IVideoRecorder.h"
#include "emapp/ImageLoader.h"
#include "emapp/ModalDialogFactory.h"
#include "emapp/Model.h"
#include "emapp/ModelProgramBundle.h"
#include "emapp/Progress.h"
#include "emapp/Project.h"
#include "emapp/ResourceBundle.h"
#include "emapp/ShadowCamera.h"
#include "emapp/StateController.h"
#include "emapp/StringUtils.h"
#include "emapp/internal/AccessoryValueState.h"
#include "emapp/internal/ApplicationUtils.h"
#include "emapp/internal/BoneValueState.h"
#include "emapp/internal/CameraValueState.h"
#include "emapp/internal/CapturingPassState.h"
#include "emapp/internal/ImGuiWindow.h"
#include "emapp/internal/LightValueState.h"
#include "emapp/model/Morph.h"
#include "emapp/private/CommonInclude.h"

#include "emapp/command/AddAccessoryKeyframeCommand.h"
#include "emapp/command/AddBoneKeyframeCommand.h"
#include "emapp/command/AddCameraKeyframeCommand.h"
#include "emapp/command/AddLightKeyframeCommand.h"
#include "emapp/command/AddModelKeyframeCommand.h"
#include "emapp/command/AddMorphKeyframeCommand.h"
#include "emapp/command/AddSelfShadowKeyframeCommand.h"
#include "emapp/command/BatchUndoCommandListCommand.h"
#include "emapp/command/InsertEmptyTimelineFrameCommand.h"
#include "emapp/command/MotionSnapshotCommand.h"
#include "emapp/command/RemoveAccessoryKeyframeCommand.h"
#include "emapp/command/RemoveBoneKeyframeCommand.h"
#include "emapp/command/RemoveCameraKeyframeCommand.h"
#include "emapp/command/RemoveLightKeyframeCommand.h"
#include "emapp/command/RemoveModelKeyframeCommand.h"
#include "emapp/command/RemoveMorphKeyframeCommand.h"
#include "emapp/command/RemoveSelfShadowKeyframeCommand.h"
#include "emapp/command/RemoveTimelineFrameCommand.h"
#include "emapp/command/TransformBoneCommand.h"
#include "emapp/command/TransformMorphCommand.h"
#include "emapp/command/UpdateAccessoryCommand.h"
#include "emapp/command/UpdateCameraCommand.h"
#include "emapp/command/UpdateLightCommand.h"
#include "protoc/application.pb-c.h"

#include "bx/commandline.h"
#include "bx/handlealloc.h"
#include "sokol/sokol_time.h"
#include "undo/undo.h"

namespace nanoem {

extern bx::AllocatorI *g_sokol_allocator;

struct sentry_options_t;
struct sentry_transport_t;

typedef sentry_transport_t *(APIENTRY *PFN_sentry_transport_new)(void (*)(sentry_envelope_t *, void *));
typedef void(APIENTRY *PFN_sentry_transport_set_state)(sentry_transport_t *, void *);
typedef void(APIENTRY *PFN_sentry_transport_set_free_func)(sentry_transport_t *, void (*)(void *));
typedef void(APIENTRY *PFN_sentry_transport_set_startup_func)(
    sentry_transport_t *, int (*)(const sentry_options_t *, void *));
typedef void(APIENTRY *PFN_sentry_transport_set_shutdown_func)(sentry_transport_t *, int (*)(uint64_t, void *));
typedef void(APIENTRY *PFN_sentry_transport_free)(sentry_transport_t *);

static PFN_sentry_transport_new sentry_transport_new;
static PFN_sentry_transport_set_state sentry_transport_set_state;
static PFN_sentry_transport_set_free_func sentry_transport_set_free_func;
static PFN_sentry_transport_set_startup_func sentry_transport_set_startup_func;
static PFN_sentry_transport_set_shutdown_func sentry_transport_set_shutdown_func;
static PFN_sentry_transport_free sentry_transport_free;

typedef sentry_options_t *(APIENTRY *PFN_sentry_options_new)();
typedef void(APIENTRY *PFN_sentry_init)(sentry_options_t *);
typedef void(APIENTRY *PFN_sentry_options_set_debug)(sentry_options_t *, int);
typedef void(APIENTRY *PFN_sentry_options_set_dsn)(sentry_options_t *, const char *);
typedef void(APIENTRY *PFN_sentry_options_set_release)(sentry_options_t *, const char *);
typedef void(APIENTRY *PFN_sentry_options_set_handler_path)(sentry_options_t *, const char *);
typedef void(APIENTRY *PFN_sentry_options_set_database_path)(sentry_options_t *, const char *);
typedef void(APIENTRY *PFN_sentry_options_set_transport)(sentry_options_t *, sentry_transport_t *);
typedef void(APIENTRY *PFN_sentry_set_user)(sentry_value_t);
typedef void(APIENTRY *PFN_sentry_shutdown)();
typedef const char *(APIENTRY *PFN_sentry_value_as_string)(sentry_value_t);
typedef void(APIENTRY *PFN_sentry_value_decref)(sentry_value_t);

static PFN_sentry_init sentry_init = nullptr;
static PFN_sentry_options_new sentry_options_new = nullptr;
static PFN_sentry_options_set_debug sentry_options_set_debug = nullptr;
static PFN_sentry_options_set_dsn sentry_options_set_dsn = nullptr;
static PFN_sentry_options_set_release sentry_options_set_release = nullptr;
static PFN_sentry_options_set_handler_path sentry_options_set_handler_path = nullptr;
static PFN_sentry_options_set_database_path sentry_options_set_database_path = nullptr;
static PFN_sentry_options_set_transport sentry_options_set_transport = nullptr;
static PFN_sentry_set_user sentry_set_user = nullptr;
static PFN_sentry_shutdown sentry_shutdown = nullptr;
static PFN_sentry_value_as_string sentry_value_as_string = nullptr;
static PFN_sentry_value_decref sentry_value_decref = nullptr;

PFN_sentry_value_new_breadcrumb sentry_value_new_breadcrumb = nullptr;
PFN_sentry_value_new_int32 sentry_value_new_int32 = nullptr;
PFN_sentry_value_new_double sentry_value_new_double = nullptr;
PFN_sentry_value_new_string sentry_value_new_string = nullptr;
PFN_sentry_value_new_object sentry_value_new_object = nullptr;
PFN_sentry_value_set_by_key sentry_value_set_by_key = nullptr;
PFN_sentry_add_breadcrumb sentry_add_breadcrumb = nullptr;
PFN_sentry_set_extra sentry_set_extra = nullptr;
PFN_sentry_set_tag sentry_set_tag = nullptr;

PFN_sentry_envelope_free sentry_envelope_free = nullptr;
PFN_sentry_envelope_serialize sentry_envelope_serialize = nullptr;
PFN_sentry_free sentry_free = nullptr;

namespace {

static sentry_value_t
defaultMaskStringFunc(const char *value)
{
    return sentry_value_new_string(value);
}

static bool g_initialized = false;
static bool g_sentryAvailable = false;
static BaseApplicationService::SentryDescription::PFN_mask_string_function g_sentryMaskStringProc =
    defaultMaskStringFunc;

struct TimeBasedAudioPlayer : BaseAudioPlayer {
    static const nanoem_u32_t kSmoothnessScaleFactor = 16;

    TimeBasedAudioPlayer()
        : BaseAudioPlayer()
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
    }
    void
    destroy() NANOEM_DECL_OVERRIDE
    {
    }
    bool
    loadAllLinearPCMSamples(const nanoem_u8_t * /* data */, size_t /* size */, Error & /* error */) NANOEM_DECL_OVERRIDE
    {
        return false;
    }
    void playPart(nanoem_f64_t /* start */, nanoem_f64_t /* length */) NANOEM_DECL_OVERRIDE
    {
    }
    void
    update() NANOEM_DECL_OVERRIDE
    {
        const nanoem_u64_t offset = static_cast<nanoem_u64_t>(m_clock.seconds() * m_currentRational.m_denominator);
        m_lastRational = m_currentRational;
        m_currentRational.m_numerator = offset;
    }
    void
    seek(const Rational &value) NANOEM_DECL_OVERRIDE
    {
        Error error;
        value.m_numerator > 0 ? internalTransitStatePaused(error) : internalTransitStateStopped(error);
        m_finished = false;
    }
    void
    internalSetVolumeGain(nanoem_f32_t /* value */, Error & /* error */) NANOEM_DECL_OVERRIDE
    {
    }
    void
    internalTransitStateStarted(Error & /* error */) NANOEM_DECL_OVERRIDE
    {
        m_clock.start();
    }
    void
    internalTransitStatePaused(Error & /* error */) NANOEM_DECL_OVERRIDE
    {
        m_clock.pause();
    }
    void
    internalTransitStateResumed(Error & /* error */) NANOEM_DECL_OVERRIDE
    {
        m_clock.resume();
    }
    void
    internalTransitStateStopped(Error & /* error */) NANOEM_DECL_OVERRIDE
    {
        m_clock.stop();
    }

    Clock m_clock;
};

struct StubBackgroundVideoRenderer : IBackgroundVideoRenderer {
    bool
    load(const URI & /* fileURI */, Error & /* error */) NANOEM_DECL_OVERRIDE
    {
        return true;
    }
    void
    draw(const Vector4 & /* rect */, nanoem_f32_t /* scaleFactor */, Project * /* project */) NANOEM_DECL_OVERRIDE
    {
    }
    void seek(nanoem_f64_t /* seconds */) NANOEM_DECL_OVERRIDE
    {
    }
    void
    flush() NANOEM_DECL_OVERRIDE
    {
    }
    void
    destroy() NANOEM_DECL_OVERRIDE
    {
    }
    URI
    fileURI() const NANOEM_DECL_NOEXCEPT_OVERRIDE
    {
        return URI();
    }
};

struct StubRendererCapability : Project::IRendererCapability {
    nanoem_u32_t
    suggestedSampleLevel(nanoem_u32_t value) const NANOEM_DECL_NOEXCEPT
    {
        return value;
    }
    bool supportsSampleLevel(nanoem_u32_t /* value */) const NANOEM_DECL_NOEXCEPT
    {
        return true;
    }
};

} /* namespace anonymous */

const char *const BaseApplicationService::kOrganizationDomain = "com.github.nanoem";
const char *const BaseApplicationService::kRendererOpenGL = "OpenGL";
const char *const BaseApplicationService::kRendererDirectX = "DirectX";
const char *const BaseApplicationService::kRendererMetal = "Metal";

class BaseApplicationService::EventPublisher : public IEventPublisher {
public:
    EventPublisher(BaseApplicationService *service);
    ~EventPublisher() NANOEM_DECL_NOEXCEPT;

    void publishTrackingEvent(
        const char *screen, const char *action, const char *category, const char *label) NANOEM_DECL_OVERRIDE;
    void publishUndoEvent(bool canUndo, bool canRedo) NANOEM_DECL_OVERRIDE;
    void publishRedoEvent(bool canRedo, bool canUndo) NANOEM_DECL_OVERRIDE;
    void publishUndoChangeEvent() NANOEM_DECL_OVERRIDE;
    void publishAddModelEvent(const Model *model) NANOEM_DECL_OVERRIDE;
    void publishSetActiveModelEvent(const Model *model) NANOEM_DECL_OVERRIDE;
    void publishSetActiveBoneEvent(const Model *model, const char *boneName) NANOEM_DECL_OVERRIDE;
    void publishSetActiveMorphEvent(const Model *model, const char *morphName) NANOEM_DECL_OVERRIDE;
    void publishRemoveModelEvent(const Model *model) NANOEM_DECL_OVERRIDE;
    void publishAddAccessoryEvent(const Accessory *accessory) NANOEM_DECL_OVERRIDE;
    void publishSetActiveAccessoryEvent(const Accessory *accessory) NANOEM_DECL_OVERRIDE;
    void publishRemoveAccessoryEvent(const Accessory *accessory) NANOEM_DECL_OVERRIDE;
    void publishAddMotionEvent(const Motion *motion) NANOEM_DECL_OVERRIDE;
    void publishRemoveMotionEvent(const Motion *motion) NANOEM_DECL_OVERRIDE;
    void publishPlayEvent(nanoem_frame_index_t duration, nanoem_frame_index_t localFrameIndex) NANOEM_DECL_OVERRIDE;
    void publishPauseEvent(nanoem_frame_index_t duration, nanoem_frame_index_t localFrameIndex) NANOEM_DECL_OVERRIDE;
    void publishResumeEvent(nanoem_frame_index_t duration, nanoem_frame_index_t localFrameIndex) NANOEM_DECL_OVERRIDE;
    void publishStopEvent(nanoem_frame_index_t duration, nanoem_frame_index_t localFrameIndex) NANOEM_DECL_OVERRIDE;
    void publishSeekEvent(nanoem_frame_index_t duration, nanoem_frame_index_t frameIndexTo,
        nanoem_frame_index_t frameIndexFrom) NANOEM_DECL_OVERRIDE;
    void publishUpdateDurationEvent(
        nanoem_frame_index_t currentDuration, nanoem_frame_index_t lastDuration) NANOEM_DECL_OVERRIDE;
    void publishToggleProjectEffectEnabledEvent(bool value) NANOEM_DECL_OVERRIDE;
    void publishToggleProjectGroundShadowEnabledEvent(bool value) NANOEM_DECL_OVERRIDE;
    void publishToggleProjectVertexShaderSkinningEnabledEvent(bool value) NANOEM_DECL_OVERRIDE;
    void publishToggleProjectComputeShaderSkinningEnabledEvent(bool value) NANOEM_DECL_OVERRIDE;
    void publishSetProjectSampleLevelEvent(nanoem_u32_t value) NANOEM_DECL_OVERRIDE;
    void publishToggleGridEnabledEvent(bool value) NANOEM_DECL_OVERRIDE;
    void publishSetGridCellEvent(const Vector2 &value) NANOEM_DECL_OVERRIDE;
    void publishSetGridSizeEvent(const Vector2 &value) NANOEM_DECL_OVERRIDE;
    void publishSetPreferredMotionFPSEvent(nanoem_u32_t value, bool unlimited) NANOEM_DECL_OVERRIDE;
    void publishSetPreferredEditingFPSEvent(nanoem_u32_t value) NANOEM_DECL_OVERRIDE;
    void publishSetPhysicsSimulationModeEvent(nanoem_u32_t value) NANOEM_DECL_OVERRIDE;
    void publishSetPhysicsSimulationEngineDebugFlagEvent(nanoem_u32_t value) NANOEM_DECL_OVERRIDE;
    void publishToggleShadowMapEnabledEvent(bool value) NANOEM_DECL_OVERRIDE;
    void publishSetShadowMapModeEvent(nanoem_u32_t value) NANOEM_DECL_OVERRIDE;
    void publishSetShadowMapDistanceEvent(nanoem_f32_t value) NANOEM_DECL_OVERRIDE;
    void publishToggleActiveModelAddBlendEnabledEvent(bool value) NANOEM_DECL_OVERRIDE;
    void publishToggleActiveModelShadowMapEnabledEvent(bool value) NANOEM_DECL_OVERRIDE;
    void publishToggleActiveModelVisibleEvent(bool value) NANOEM_DECL_OVERRIDE;
    void publishToggleActiveModelComputeShaderSkinningEnabledEvent(bool value) NANOEM_DECL_OVERRIDE;
    void publishToggleActiveModelShowAllBonesEvent(bool value) NANOEM_DECL_OVERRIDE;
    void publishToggleActiveModelShowAllRigidBodiesEvent(bool value) NANOEM_DECL_OVERRIDE;
    void publishToggleActiveModelShowAllVertexFacesEvent(bool value) NANOEM_DECL_OVERRIDE;
    void publishToggleActiveModelShowAllVertexPointsEvent(bool value) NANOEM_DECL_OVERRIDE;
    void publishToggleActiveModelVertexShaderSkinningEnabledEvent(bool value) NANOEM_DECL_OVERRIDE;
    void publishConsumePassEvent(nanoem_u64_t globalFrameIndex) NANOEM_DECL_OVERRIDE;
    void publishAddModalDialogEvent() NANOEM_DECL_OVERRIDE;
    void publishClearModalDialogEvent() NANOEM_DECL_OVERRIDE;
    void publishSetLanguageEvent(nanoem_u32_t value) NANOEM_DECL_OVERRIDE;
    void publishQueryOpenSingleFileDialogEvent(
        nanoem_u32_t value, const StringList &allowedExtensions) NANOEM_DECL_OVERRIDE;
    void publishQueryOpenMultipleFilesDialogEvent(
        nanoem_u32_t value, const StringList &allowedExtensions) NANOEM_DECL_OVERRIDE;
    void publishQuerySaveFileDialogEvent(nanoem_u32_t value, const StringList &allowedExtensions) NANOEM_DECL_OVERRIDE;
    void publishToggleProjectPlayingWithLoopEnabledEvent(bool value) NANOEM_DECL_OVERRIDE;
    void publishToggleActiveAccessoryAddBlendEnabledEvent(bool value) NANOEM_DECL_OVERRIDE;
    void publishToggleActiveAccessoryGroundShadowEnabledEvent(bool value) NANOEM_DECL_OVERRIDE;
    void publishToggleActiveAccessoryVisibleEvent(bool value) NANOEM_DECL_OVERRIDE;
    void publishToggleModelEditingEnabledEvent(bool value) NANOEM_DECL_OVERRIDE;
    void publishUpdateProgressEvent(
        nanoem_u32_t value, nanoem_u32_t total, nanoem_u32_t type, const char *text) NANOEM_DECL_OVERRIDE;
    void publishStartProgressEvent(const char *title, const char *text, nanoem_u32_t total) NANOEM_DECL_OVERRIDE;
    void publishStopProgressEvent() NANOEM_DECL_OVERRIDE;
    void publishSetupProjectEvent(const Vector2 &windowSize, nanoem_f32_t windowDevicePixelRatio,
        nanoem_f32_t viewportDevicePixelRatio) NANOEM_DECL_OVERRIDE;
    void publishSetEditingModeEvent(nanoem_u32_t value) NANOEM_DECL_OVERRIDE;
    void publishCompleteExportingImageConfigurationEvent(const StringList &availableExtensions) NANOEM_DECL_OVERRIDE;
    void publishCompleteExportingVideoConfigurationEvent(const StringList &availableExtensions) NANOEM_DECL_OVERRIDE;
    void publishCanCopyEvent(bool value) NANOEM_DECL_OVERRIDE;
    void publishCanPasteEvent(bool value) NANOEM_DECL_OVERRIDE;
    void publishSetWindowDevicePixelRatioEvent(nanoem_f32_t value) NANOEM_DECL_OVERRIDE;
    void publishSetViewportDevicePixelRatioEvent(nanoem_f32_t value) NANOEM_DECL_OVERRIDE;
    void publishDisableCursorEvent(const Vector2 &value) NANOEM_DECL_OVERRIDE;
    void publishEnableCursorEvent(const Vector2 &value) NANOEM_DECL_OVERRIDE;
    void publishQuitApplicationEvent() NANOEM_DECL_OVERRIDE;
    void publishErrorEvent(const Error &error) NANOEM_DECL_OVERRIDE;

private:
    void sendEventMessage(const Nanoem__Application__Event &event);

    BaseApplicationService *m_service;
};

BaseApplicationService::EventPublisher::EventPublisher(BaseApplicationService *service)
    : m_service(service)
{
}

BaseApplicationService::EventPublisher::~EventPublisher() NANOEM_DECL_NOEXCEPT
{
}

void
BaseApplicationService::EventPublisher::publishTrackingEvent(
    const char *screen, const char *action, const char *category, const char *label)
{
    Nanoem__Application__TrackEvent base = NANOEM__APPLICATION__TRACK_EVENT__INIT;
    MutableString screenString, actionString, categoryStirng, labelString;
    if (screen) {
        StringUtils::copyString(screen, screenString);
    }
    if (action) {
        StringUtils::copyString(action, actionString);
    }
    if (category) {
        StringUtils::copyString(category, categoryStirng);
    }
    if (label) {
        StringUtils::copyString(label, labelString);
    }
    base.screen = screenString.data();
    base.action = actionString.data();
    base.category = categoryStirng.data();
    base.label = labelString.data();
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_TRACK;
    event.track = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishUndoEvent(bool canUndo, bool canRedo)
{
    Nanoem__Application__UndoEvent base = NANOEM__APPLICATION__UNDO_EVENT__INIT;
    base.can_undo = canUndo ? 1 : 0;
    base.can_redo = canRedo ? 1 : 0;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_UNDO;
    event.undo = &base;
    sendEventMessage(event);
    if (g_sentryAvailable) {
        sentry_value_t breadcrumb = sentry_value_new_breadcrumb(nullptr, nullptr);
        sentry_value_set_by_key(breadcrumb, "category", sentry_value_new_string("undo"));
        sentry_add_breadcrumb(breadcrumb);
    }
}

void
BaseApplicationService::EventPublisher::publishRedoEvent(bool canRedo, bool canUndo)
{
    Nanoem__Application__RedoEvent base = NANOEM__APPLICATION__REDO_EVENT__INIT;
    base.can_undo = canUndo ? 1 : 0;
    base.can_redo = canRedo ? 1 : 0;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_REDO;
    event.redo = &base;
    sendEventMessage(event);
    if (g_sentryAvailable) {
        sentry_value_t breadcrumb = sentry_value_new_breadcrumb(nullptr, nullptr);
        sentry_value_set_by_key(breadcrumb, "category", sentry_value_new_string("redo"));
        sentry_add_breadcrumb(breadcrumb);
    }
}

void
BaseApplicationService::EventPublisher::publishUndoChangeEvent()
{
    Nanoem__Application__UndoChangeEvent base = NANOEM__APPLICATION__UNDO_CHANGE_EVENT__INIT;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_UNDO_CHANGE;
    event.undo_change = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishAddModelEvent(const Model *model)
{
    Nanoem__Application__AddModelEvent base = NANOEM__APPLICATION__ADD_MODEL_EVENT__INIT;
    MutableString name;
    if (model) {
        StringUtils::copyString(model->canonicalName(), name);
        base.model_handle = model->handle();
        base.name = name.data();
    }
    else {
        base.model_handle = bx::kInvalidHandle;
    }
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_ADD_MODEL;
    event.add_model = &base;
    sendEventMessage(event);
    if (g_sentryAvailable) {
        sentry_value_t breadcrumb = sentry_value_new_breadcrumb(nullptr, nullptr);
        sentry_value_t data = sentry_value_new_object();
        sentry_value_set_by_key(data, "handle", sentry_value_new_int32(base.model_handle));
        sentry_value_set_by_key(data, "name", sentry_value_new_string(base.name));
        sentry_value_set_by_key(breadcrumb, "category", sentry_value_new_string("model.add"));
        sentry_value_set_by_key(breadcrumb, "data", data);
        sentry_add_breadcrumb(breadcrumb);
    }
}

void
BaseApplicationService::EventPublisher::publishSetActiveModelEvent(const Model *model)
{
    Nanoem__Application__SetActiveModelEvent base = NANOEM__APPLICATION__SET_ACTIVE_MODEL_EVENT__INIT;
    MutableString name;
    if (model) {
        StringUtils::copyString(model->name(), name);
        base.name = name.data();
        base.model_handle = model->handle();
    }
    else {
        base.model_handle = bx::kInvalidHandle;
    }
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_SET_ACTIVE_MODEL;
    event.set_active_model = &base;
    sendEventMessage(event);
    if (g_sentryAvailable) {
        sentry_value_t breadcrumb = sentry_value_new_breadcrumb(nullptr, nullptr);
        sentry_value_t data = sentry_value_new_object();
        sentry_value_set_by_key(data, "handle", sentry_value_new_int32(base.model_handle));
        sentry_value_set_by_key(breadcrumb, "category", sentry_value_new_string("model.active"));
        sentry_value_set_by_key(breadcrumb, "data", data);
        sentry_add_breadcrumb(breadcrumb);
    }
}

void
BaseApplicationService::EventPublisher::publishSetActiveBoneEvent(const Model *model, const char *boneName)
{
    Nanoem__Application__SetActiveBoneEvent base = NANOEM__APPLICATION__SET_ACTIVE_BONE_EVENT__INIT;
    MutableString modelName, name;
    if (model) {
        StringUtils::copyString(model->name(), modelName);
        base.model_name = modelName.data();
        base.model_handle = model->handle();
    }
    else {
        base.model_handle = bx::kInvalidHandle;
    }
    if (boneName) {
        StringUtils::copyString(boneName, name);
        base.bone_name = name.data();
    }
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_SET_ACTIVE_BONE;
    event.set_active_bone = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishSetActiveMorphEvent(const Model *model, const char *morphName)
{
    Nanoem__Application__SetActiveMorphEvent base = NANOEM__APPLICATION__SET_ACTIVE_MORPH_EVENT__INIT;
    MutableString modelName, name;
    if (model) {
        StringUtils::copyString(model->name(), modelName);
        base.model_name = modelName.data();
        base.model_handle = model->handle();
    }
    else {
        base.model_handle = bx::kInvalidHandle;
    }
    if (morphName) {
        StringUtils::copyString(morphName, name);
        base.morph_name = name.data();
    }
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_SET_ACTIVE_MORPH;
    event.set_active_morph = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishRemoveModelEvent(const Model *model)
{
    Nanoem__Application__RemoveModelEvent base = NANOEM__APPLICATION__REMOVE_MODEL_EVENT__INIT;
    MutableString name;
    if (model) {
        StringUtils::copyString(model->name(), name);
        base.name = name.data();
        base.model_handle = model->handle();
    }
    else {
        base.model_handle = bx::kInvalidHandle;
    }
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_REMOVE_MODEL;
    event.remove_model = &base;
    sendEventMessage(event);
    if (g_sentryAvailable) {
        sentry_value_t breadcrumb = sentry_value_new_breadcrumb(nullptr, nullptr);
        sentry_value_t data = sentry_value_new_object();
        sentry_value_set_by_key(data, "handle", sentry_value_new_int32(base.model_handle));
        sentry_value_set_by_key(breadcrumb, "category", sentry_value_new_string("model.remove"));
        sentry_value_set_by_key(breadcrumb, "data", data);
        sentry_add_breadcrumb(breadcrumb);
    }
}

void
BaseApplicationService::EventPublisher::publishAddAccessoryEvent(const Accessory *accessory)
{
    Nanoem__Application__AddAccessoryEvent base = NANOEM__APPLICATION__ADD_ACCESSORY_EVENT__INIT;
    MutableString name;
    if (accessory) {
        StringUtils::copyString(accessory->canonicalName(), name);
        base.name = name.data();
        base.accessory_handle = accessory->handle();
    }
    else {
        base.accessory_handle = bx::kInvalidHandle;
    }
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_ADD_ACCESSORY;
    event.add_accessory = &base;
    sendEventMessage(event);
    if (g_sentryAvailable) {
        sentry_value_t breadcrumb = sentry_value_new_breadcrumb(nullptr, nullptr);
        sentry_value_t data = sentry_value_new_object();
        sentry_value_set_by_key(data, "handle", sentry_value_new_int32(base.accessory_handle));
        sentry_value_set_by_key(data, "name", sentry_value_new_string(base.name));
        sentry_value_set_by_key(breadcrumb, "category", sentry_value_new_string("accessory.add"));
        sentry_value_set_by_key(breadcrumb, "data", data);
        sentry_add_breadcrumb(breadcrumb);
    }
}

void
BaseApplicationService::EventPublisher::publishSetActiveAccessoryEvent(const Accessory *accessory)
{
    Nanoem__Application__SetActiveAccessoryEvent base = NANOEM__APPLICATION__SET_ACTIVE_ACCESSORY_EVENT__INIT;
    MutableString name;
    if (accessory) {
        StringUtils::copyString(accessory->name(), name);
        base.name = name.data();
        base.accessory_handle = accessory->handle();
    }
    else {
        base.accessory_handle = bx::kInvalidHandle;
    }
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_SET_ACTIVE_ACCESSORY;
    event.set_active_accessory = &base;
    sendEventMessage(event);
    if (g_sentryAvailable) {
        sentry_value_t breadcrumb = sentry_value_new_breadcrumb(nullptr, nullptr);
        sentry_value_t data = sentry_value_new_object();
        sentry_value_set_by_key(data, "handle", sentry_value_new_int32(base.accessory_handle));
        sentry_value_set_by_key(breadcrumb, "category", sentry_value_new_string("accessory.active"));
        sentry_value_set_by_key(breadcrumb, "data", data);
        sentry_add_breadcrumb(breadcrumb);
    }
}

void
BaseApplicationService::EventPublisher::publishRemoveAccessoryEvent(const Accessory *accessory)
{
    Nanoem__Application__RemoveAccessoryEvent base = NANOEM__APPLICATION__REMOVE_ACCESSORY_EVENT__INIT;
    MutableString name;
    if (accessory) {
        StringUtils::copyString(accessory->name(), name);
        base.name = name.data();
        base.accessory_handle = accessory->handle();
    }
    else {
        base.accessory_handle = bx::kInvalidHandle;
    }
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_REMOVE_ACCESSORY;
    event.remove_accessory = &base;
    sendEventMessage(event);
    if (g_sentryAvailable) {
        sentry_value_t breadcrumb = sentry_value_new_breadcrumb(nullptr, nullptr);
        sentry_value_t data = sentry_value_new_object();
        sentry_value_set_by_key(data, "handle", sentry_value_new_int32(base.accessory_handle));
        sentry_value_set_by_key(breadcrumb, "category", sentry_value_new_string("accessory.remove"));
        sentry_value_set_by_key(breadcrumb, "data", data);
        sentry_add_breadcrumb(breadcrumb);
    }
}

void
BaseApplicationService::EventPublisher::publishAddMotionEvent(const Motion *motion)
{
    Nanoem__Application__AddMotionEvent base = NANOEM__APPLICATION__ADD_MOTION_EVENT__INIT;
    base.motion_handle = motion ? motion->handle() : bx::kInvalidHandle;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_ADD_MOTION;
    event.add_motion = &base;
    sendEventMessage(event);
    if (g_sentryAvailable) {
        sentry_value_t breadcrumb = sentry_value_new_breadcrumb(nullptr, nullptr);
        sentry_value_t data = sentry_value_new_object();
        sentry_value_set_by_key(data, "handle", sentry_value_new_int32(base.motion_handle));
        sentry_value_set_by_key(breadcrumb, "category", sentry_value_new_string("motion.add"));
        sentry_value_set_by_key(breadcrumb, "data", data);
        sentry_add_breadcrumb(breadcrumb);
    }
}

void
BaseApplicationService::EventPublisher::publishRemoveMotionEvent(const Motion *motion)
{
    Nanoem__Application__RemoveMotionEvent base = NANOEM__APPLICATION__REMOVE_MOTION_EVENT__INIT;
    base.motion_handle = motion ? motion->handle() : bx::kInvalidHandle;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_REMOVE_MOTION;
    event.remove_motion = &base;
    sendEventMessage(event);
    if (g_sentryAvailable) {
        sentry_value_t breadcrumb = sentry_value_new_breadcrumb(nullptr, nullptr);
        sentry_value_t data = sentry_value_new_object();
        sentry_value_set_by_key(data, "handle", sentry_value_new_int32(base.motion_handle));
        sentry_value_set_by_key(breadcrumb, "category", sentry_value_new_string("motion.remove"));
        sentry_value_set_by_key(breadcrumb, "data", data);
        sentry_add_breadcrumb(breadcrumb);
    }
}

void
BaseApplicationService::EventPublisher::publishPlayEvent(
    nanoem_frame_index_t duration, nanoem_frame_index_t localFrameIndex)
{
    Nanoem__Application__PlayEvent base = NANOEM__APPLICATION__PLAY_EVENT__INIT;
    base.duration = duration;
    base.local_frame_index = localFrameIndex;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_PLAY;
    event.play = &base;
    sendEventMessage(event);
    if (g_sentryAvailable) {
        sentry_value_t breadcrumb = sentry_value_new_breadcrumb(nullptr, nullptr);
        sentry_value_t data = sentry_value_new_object();
        sentry_value_set_by_key(data, "duration", sentry_value_new_int32(duration));
        sentry_value_set_by_key(data, "index", sentry_value_new_int32(localFrameIndex));
        sentry_value_set_by_key(breadcrumb, "category", sentry_value_new_string("play"));
        sentry_value_set_by_key(breadcrumb, "data", data);
        sentry_add_breadcrumb(breadcrumb);
    }
}

void
BaseApplicationService::EventPublisher::publishPauseEvent(
    nanoem_frame_index_t duration, nanoem_frame_index_t localFrameIndex)
{
    Nanoem__Application__PauseEvent base = NANOEM__APPLICATION__PAUSE_EVENT__INIT;
    base.duration = duration;
    base.local_frame_index = localFrameIndex;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_PAUSE;
    event.pause = &base;
    sendEventMessage(event);
    if (g_sentryAvailable) {
        sentry_value_t breadcrumb = sentry_value_new_breadcrumb(nullptr, nullptr);
        sentry_value_t data = sentry_value_new_object();
        sentry_value_set_by_key(data, "duration", sentry_value_new_int32(duration));
        sentry_value_set_by_key(data, "index", sentry_value_new_int32(localFrameIndex));
        sentry_value_set_by_key(breadcrumb, "category", sentry_value_new_string("pause"));
        sentry_value_set_by_key(breadcrumb, "data", data);
        sentry_add_breadcrumb(breadcrumb);
    }
}

void
BaseApplicationService::EventPublisher::publishResumeEvent(
    nanoem_frame_index_t duration, nanoem_frame_index_t localFrameIndex)
{
    Nanoem__Application__ResumeEvent base = NANOEM__APPLICATION__RESUME_EVENT__INIT;
    base.duration = duration;
    base.local_frame_index = localFrameIndex;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_RESUME;
    event.resume = &base;
    sendEventMessage(event);
    if (g_sentryAvailable) {
        sentry_value_t breadcrumb = sentry_value_new_breadcrumb(nullptr, nullptr);
        sentry_value_t data = sentry_value_new_object();
        sentry_value_set_by_key(data, "duration", sentry_value_new_int32(duration));
        sentry_value_set_by_key(data, "index", sentry_value_new_int32(localFrameIndex));
        sentry_value_set_by_key(breadcrumb, "category", sentry_value_new_string("resume"));
        sentry_value_set_by_key(breadcrumb, "data", data);
        sentry_add_breadcrumb(breadcrumb);
    }
}

void
BaseApplicationService::EventPublisher::publishStopEvent(
    nanoem_frame_index_t duration, nanoem_frame_index_t localFrameIndex)
{
    Nanoem__Application__StopEvent base = NANOEM__APPLICATION__STOP_EVENT__INIT;
    base.duration = duration;
    base.local_frame_index = localFrameIndex;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_STOP;
    event.stop = &base;
    sendEventMessage(event);
    if (g_sentryAvailable) {
        sentry_value_t breadcrumb = sentry_value_new_breadcrumb(nullptr, nullptr);
        sentry_value_t data = sentry_value_new_object();
        sentry_value_set_by_key(data, "duration", sentry_value_new_int32(duration));
        sentry_value_set_by_key(data, "index", sentry_value_new_int32(localFrameIndex));
        sentry_value_set_by_key(breadcrumb, "category", sentry_value_new_string("stop"));
        sentry_value_set_by_key(breadcrumb, "data", data);
        sentry_add_breadcrumb(breadcrumb);
    }
}

void
BaseApplicationService::EventPublisher::publishSeekEvent(
    nanoem_frame_index_t duration, nanoem_frame_index_t frameIndexTo, nanoem_frame_index_t frameIndexFrom)
{
    Nanoem__Application__SeekEvent base = NANOEM__APPLICATION__SEEK_EVENT__INIT;
    base.duration = duration;
    base.local_frame_index_from = frameIndexFrom;
    base.local_frame_index_to = frameIndexTo;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_SEEK;
    event.seek = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishUpdateDurationEvent(
    nanoem_frame_index_t currentDuration, nanoem_frame_index_t lastDuration)
{
    Nanoem__Application__UpdateDurationEvent base = NANOEM__APPLICATION__UPDATE_DURATION_EVENT__INIT;
    base.current_duration = currentDuration;
    base.last_duration = lastDuration;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_UPDATE_DURATION;
    event.update_duration = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishToggleProjectEffectEnabledEvent(bool value)
{
    Nanoem__Application__ToggleProjectEffectEnabledEvent base =
        NANOEM__APPLICATION__TOGGLE_PROJECT_EFFECT_ENABLED_EVENT__INIT;
    base.value = value;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_PROJECT_EFFECT_ENABLED;
    event.toggle_project_effect_enabled = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishToggleProjectGroundShadowEnabledEvent(bool value)
{
    Nanoem__Application__ToggleProjectGroundShadowEnabledEvent base =
        NANOEM__APPLICATION__TOGGLE_PROJECT_GROUND_SHADOW_ENABLED_EVENT__INIT;
    base.value = value;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_PROJECT_GROUND_SHADOW_ENABLED;
    event.toggle_project_ground_shadow_enabled = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishToggleProjectVertexShaderSkinningEnabledEvent(bool value)
{
    Nanoem__Application__ToggleProjectVertexShaderSkinningEnabledEvent base =
        NANOEM__APPLICATION__TOGGLE_PROJECT_VERTEX_SHADER_SKINNING_ENABLED_EVENT__INIT;
    base.value = value;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_PROJECT_VERTEX_SHADER_SKINNING_ENABLED;
    event.toggle_project_vertex_shader_skinning_enabled = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishToggleProjectComputeShaderSkinningEnabledEvent(bool value)
{
    Nanoem__Application__ToggleProjectComputeShaderSkinningEnabledEvent base =
        NANOEM__APPLICATION__TOGGLE_PROJECT_COMPUTE_SHADER_SKINNING_ENABLED_EVENT__INIT;
    base.value = value;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_PROJECT_COMPUTE_SHADER_SKINNING_ENABLED;
    event.toggle_project_compute_shader_skinning_enabled = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishSetProjectSampleLevelEvent(nanoem_u32_t value)
{
    Nanoem__Application__SetProjectSampleLevelEvent base = NANOEM__APPLICATION__SET_PROJECT_SAMPLE_LEVEL_EVENT__INIT;
    base.value = value;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_SET_PROJECT_SAMPLE_LEVEL;
    event.set_project_sample_level = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishToggleGridEnabledEvent(bool value)
{
    Nanoem__Application__ToggleGridEnabledEvent base = NANOEM__APPLICATION__TOGGLE_GRID_ENABLED_EVENT__INIT;
    base.value = value;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_GRID_ENABLED;
    event.toggle_grid_enabled = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishSetGridCellEvent(const Vector2 &value)
{
    Nanoem__Application__SetGridCellEvent base = NANOEM__APPLICATION__SET_GRID_CELL_EVENT__INIT;
    base.x = value.x;
    base.y = value.y;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_SET_GRID_CELL;
    event.set_grid_cell = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishSetGridSizeEvent(const Vector2 &value)
{
    Nanoem__Application__SetGridSizeEvent base = NANOEM__APPLICATION__SET_GRID_SIZE_EVENT__INIT;
    base.x = value.x;
    base.y = value.y;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_SET_GRID_SIZE;
    event.set_grid_size = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishSetPreferredMotionFPSEvent(nanoem_u32_t value, bool unlimited)
{
    Nanoem__Application__SetPreferredMotionFPSEvent base = NANOEM__APPLICATION__SET_PREFERRED_MOTION_FPSEVENT__INIT;
    base.value = value;
    base.unlimited = unlimited;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_SET_PREFERRED_MOTION_FPS;
    event.set_preferred_motion_fps = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishSetPreferredEditingFPSEvent(nanoem_u32_t value)
{
    Nanoem__Application__SetPreferredEditingFPSEvent base = NANOEM__APPLICATION__SET_PREFERRED_EDITING_FPSEVENT__INIT;
    base.value = value;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_SET_PREFERRED_EDITING_FPS;
    event.set_preferred_editing_fps = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishSetPhysicsSimulationModeEvent(nanoem_u32_t value)
{
    Nanoem__Application__SetPhysicsSimulationModeEvent base =
        NANOEM__APPLICATION__SET_PHYSICS_SIMULATION_MODE_EVENT__INIT;
    base.value = value;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_SET_PHYSICS_SIMULATION_MODE;
    event.set_physics_simulation_mode = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishSetPhysicsSimulationEngineDebugFlagEvent(nanoem_u32_t value)
{
    Nanoem__Application__SetPhysicsSimulationEngineDebugFlagEvent base =
        NANOEM__APPLICATION__SET_PHYSICS_SIMULATION_ENGINE_DEBUG_FLAG_EVENT__INIT;
    base.value = value;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_SET_PHYSICS_SIMULATION_ENGINE_DEBUG_FLAG;
    event.set_physics_simulation_engine_debug_flag = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishToggleShadowMapEnabledEvent(bool value)
{
    Nanoem__Application__ToggleShadowMapEnabledEvent base = NANOEM__APPLICATION__TOGGLE_SHADOW_MAP_ENABLED_EVENT__INIT;
    base.value = value;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_SHADOW_MAP_ENABLED;
    event.toggle_shadow_map_enabled = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishSetShadowMapModeEvent(nanoem_u32_t value)
{
    Nanoem__Application__SetShadowMapModeEvent base = NANOEM__APPLICATION__SET_SHADOW_MAP_MODE_EVENT__INIT;
    base.value = value;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_SET_SHADOW_MAP_MODE;
    event.set_shadow_map_mode = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishSetShadowMapDistanceEvent(nanoem_f32_t value)
{
    Nanoem__Application__SetShadowMapDistanceEvent base = NANOEM__APPLICATION__SET_SHADOW_MAP_DISTANCE_EVENT__INIT;
    base.value = value;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_SET_SHADOW_MAP_DISTANCE;
    event.set_shadow_map_distance = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishToggleActiveModelAddBlendEnabledEvent(bool value)
{
    Nanoem__Application__ToggleActiveModelAddBlendEnabledEvent base =
        NANOEM__APPLICATION__TOGGLE_ACTIVE_MODEL_ADD_BLEND_ENABLED_EVENT__INIT;
    base.value = value;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_ACTIVE_MODEL_ADD_BLEND_ENABLED;
    event.toggle_active_model_add_blend_enabled = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishToggleActiveModelShadowMapEnabledEvent(bool value)
{
    Nanoem__Application__ToggleActiveModelShadowMapEnabledEvent base =
        NANOEM__APPLICATION__TOGGLE_ACTIVE_MODEL_SHADOW_MAP_ENABLED_EVENT__INIT;
    base.value = value;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_ACTIVE_MODEL_SHADOW_MAP_ENABLED;
    event.toggle_active_model_shadow_map_enabled = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishToggleActiveModelVisibleEvent(bool value)
{
    Nanoem__Application__ToggleActiveModelVisibleEvent base =
        NANOEM__APPLICATION__TOGGLE_ACTIVE_MODEL_VISIBLE_EVENT__INIT;
    base.value = value;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_ACTIVE_MODEL_VISIBLE;
    event.toggle_active_model_visible = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishToggleActiveModelComputeShaderSkinningEnabledEvent(bool value)
{
    Nanoem__Application__ToggleActiveModelComputeShaderSkinningEnabledEvent base =
        NANOEM__APPLICATION__TOGGLE_ACTIVE_MODEL_COMPUTE_SHADER_SKINNING_ENABLED_EVENT__INIT;
    base.value = value;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_ACTIVE_MODEL_COMPUTE_SHADER_SKINNING_ENABLED;
    event.toggle_active_model_compute_shader_skinning_enabled = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishToggleActiveModelShowAllBonesEvent(bool value)
{
    Nanoem__Application__ToggleActiveModelShowAllBonesEvent base =
        NANOEM__APPLICATION__TOGGLE_ACTIVE_MODEL_SHOW_ALL_BONES_EVENT__INIT;
    base.value = value;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_ACTIVE_MODEL_SHOW_ALL_BONES;
    event.toggle_active_model_show_all_bones = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishToggleActiveModelShowAllRigidBodiesEvent(bool value)
{
    Nanoem__Application__ToggleActiveModelShowAllRigidBodiesEvent base =
        NANOEM__APPLICATION__TOGGLE_ACTIVE_MODEL_SHOW_ALL_RIGID_BODIES_EVENT__INIT;
    base.value = value;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_ACTIVE_MODEL_SHOW_ALL_RIGID_BODIES;
    event.toggle_active_model_show_all_rigid_bodies = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishToggleActiveModelShowAllVertexFacesEvent(bool value)
{
    Nanoem__Application__ToggleActiveModelShowAllVertexFacesEvent base =
        NANOEM__APPLICATION__TOGGLE_ACTIVE_MODEL_SHOW_ALL_VERTEX_FACES_EVENT__INIT;
    base.value = value;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_ACTIVE_MODEL_SHOW_ALL_VERTEX_FACES;
    event.toggle_active_model_show_all_vertex_faces = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishToggleActiveModelShowAllVertexPointsEvent(bool value)
{
    Nanoem__Application__ToggleActiveModelShowAllVertexPointsEvent base =
        NANOEM__APPLICATION__TOGGLE_ACTIVE_MODEL_SHOW_ALL_VERTEX_POINTS_EVENT__INIT;
    base.value = value;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_ACTIVE_MODEL_SHOW_ALL_VERTEX_POINTS;
    event.toggle_active_model_show_all_vertex_points = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishToggleActiveModelVertexShaderSkinningEnabledEvent(bool value)
{
    Nanoem__Application__ToggleActiveModelVertexShaderSkinningEnabledEvent base =
        NANOEM__APPLICATION__TOGGLE_ACTIVE_MODEL_VERTEX_SHADER_SKINNING_ENABLED_EVENT__INIT;
    base.value = value;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_ACTIVE_MODEL_VERTEX_SHADER_SKINNING;
    event.toggle_active_model_vertex_shader_skinning = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishConsumePassEvent(nanoem_u64_t globalFrameIndex)
{
    Nanoem__Application__ConsumePassEvent base = NANOEM__APPLICATION__CONSUME_PASS_EVENT__INIT;
    base.global_frame_index = globalFrameIndex;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_CONSUME_PASS;
    event.consume_pass = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishAddModalDialogEvent()
{
    Nanoem__Application__AddModalDialogEvent base = NANOEM__APPLICATION__ADD_MODAL_DIALOG_EVENT__INIT;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_ADD_MODAL_DIALOG;
    event.add_modal_dialog = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishClearModalDialogEvent()
{
    Nanoem__Application__ClearModalDialogEvent base = NANOEM__APPLICATION__CLEAR_MODAL_DIALOG_EVENT__INIT;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_CLEAR_MODAL_DIALOG;
    event.clear_modal_dialog = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishSetLanguageEvent(nanoem_u32_t value)
{
    Nanoem__Application__SetLanguageEvent base = NANOEM__APPLICATION__SET_LANGUAGE_EVENT__INIT;
    base.value = value;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_SET_LANGUAGE;
    event.set_language = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishQueryOpenSingleFileDialogEvent(
    nanoem_u32_t value, const StringList &allowedExtensions)
{
    Nanoem__Application__QueryOpenSingleFileDialogEvent base =
        NANOEM__APPLICATION__QUERY_OPEN_SINGLE_FILE_DIALOG_EVENT__INIT;
    base.type = value;
    MutableStringList ae;
    internal::ApplicationUtils::allocateStringList(
        allowedExtensions, base.allowed_extensions, base.n_allowed_extensions, ae);
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_QUERY_OPEN_SINGLE_FILE_DIALOG;
    event.query_open_single_file_dialog = &base;
    sendEventMessage(event);
    internal::ApplicationUtils::freeStringList(base.allowed_extensions);
}

void
BaseApplicationService::EventPublisher::publishQueryOpenMultipleFilesDialogEvent(
    nanoem_u32_t value, const StringList &allowedExtensions)
{
    Nanoem__Application__QueryOpenMultipleFilesDialogEvent base =
        NANOEM__APPLICATION__QUERY_OPEN_MULTIPLE_FILES_DIALOG_EVENT__INIT;
    base.type = value;
    MutableStringList ae;
    internal::ApplicationUtils::allocateStringList(
        allowedExtensions, base.allowed_extensions, base.n_allowed_extensions, ae);
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_QUERY_OPEN_MULTIPLE_FILES_DIALOG;
    event.query_open_multiple_files_dialog = &base;
    sendEventMessage(event);
    internal::ApplicationUtils::freeStringList(base.allowed_extensions);
}

void
BaseApplicationService::EventPublisher::publishQuerySaveFileDialogEvent(
    nanoem_u32_t value, const StringList &allowedExtensions)
{
    Nanoem__Application__QuerySaveFileDialogEvent base = NANOEM__APPLICATION__QUERY_SAVE_FILE_DIALOG_EVENT__INIT;
    base.type = value;
    MutableStringList ae;
    internal::ApplicationUtils::allocateStringList(
        allowedExtensions, base.allowed_extensions, base.n_allowed_extensions, ae);
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_QUERY_SAVE_FILE_DIALOG;
    event.query_save_file_dialog = &base;
    sendEventMessage(event);
    internal::ApplicationUtils::freeStringList(base.allowed_extensions);
}

void
BaseApplicationService::EventPublisher::publishToggleProjectPlayingWithLoopEnabledEvent(bool value)
{
    Nanoem__Application__ToggleProjectPlayingWithLoopEnabledEvent base =
        NANOEM__APPLICATION__TOGGLE_PROJECT_PLAYING_WITH_LOOP_ENABLED_EVENT__INIT;
    base.value = value;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_PROJECT_PLAYING_WITH_LOOP_ENABLED;
    event.toggle_project_playing_with_loop_enabled = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishToggleActiveAccessoryAddBlendEnabledEvent(bool value)
{
    Nanoem__Application__ToggleActiveAccessoryAddBlendEnabledEvent base =
        NANOEM__APPLICATION__TOGGLE_ACTIVE_ACCESSORY_ADD_BLEND_ENABLED_EVENT__INIT;
    base.value = value;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_ACTIVE_ACCESSORY_ADD_BLEND_ENABLED;
    event.toggle_active_accessory_add_blend_enabled = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishToggleActiveAccessoryGroundShadowEnabledEvent(bool value)
{
    Nanoem__Application__ToggleActiveAccessoryShadowEnabledEvent base =
        NANOEM__APPLICATION__TOGGLE_ACTIVE_ACCESSORY_SHADOW_ENABLED_EVENT__INIT;
    base.value = value;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_ACTIVE_ACCESSORY_SHADOW_ENABLED;
    event.toggle_active_accessory_shadow_enabled = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishToggleActiveAccessoryVisibleEvent(bool value)
{
    Nanoem__Application__ToggleActiveAccessoryVisibleEvent base =
        NANOEM__APPLICATION__TOGGLE_ACTIVE_ACCESSORY_VISIBLE_EVENT__INIT;
    base.value = value;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_ACTIVE_ACCESSORY_VISIBLE;
    event.toggle_active_accessory_visible = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishToggleModelEditingEnabledEvent(bool value)
{
    Nanoem__Application__ToggleModelEditingEnabledEvent base =
        NANOEM__APPLICATION__TOGGLE_MODEL_EDITING_ENABLED_EVENT__INIT;
    base.value = value;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_MODEL_EDITING_ENABLED;
    event.toggle_model_editing_enabled = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishUpdateProgressEvent(
    nanoem_u32_t value, nanoem_u32_t total, nanoem_u32_t type, const char *text)
{
    MutableString s;
    Nanoem__Application__UpdateProgressEvent base = NANOEM__APPLICATION__UPDATE_PROGRESS_EVENT__INIT;
    base.value = value;
    base.total = total;
    base.type_case = static_cast<Nanoem__Application__UpdateProgressEvent__TypeCase>(type);
    base.text = StringUtils::cloneString(text, s);
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_UPDATE_PROGRESS;
    event.update_progress = &base;
    sendEventMessage(event);
    m_service->postEmptyApplicationEvent();
}

void
BaseApplicationService::EventPublisher::publishStartProgressEvent(
    const char *title, const char *text, nanoem_u32_t total)
{
    MutableString titleString, textString;
    Nanoem__Application__StartProgressEvent base = NANOEM__APPLICATION__START_PROGRESS_EVENT__INIT;
    base.title = StringUtils::cloneString(title, titleString);
    base.text = StringUtils::cloneString(text, textString);
    base.total = total;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_START_PROGRESS;
    event.start_progress = &base;
    sendEventMessage(event);
    m_service->postEmptyApplicationEvent();
}

void
BaseApplicationService::EventPublisher::publishStopProgressEvent()
{
    Nanoem__Application__StopProgressEvent base = NANOEM__APPLICATION__STOP_PROGRESS_EVENT__INIT;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_STOP_PROGRESS;
    event.stop_progress = &base;
    sendEventMessage(event);
    m_service->postEmptyApplicationEvent();
}

void
BaseApplicationService::EventPublisher::publishSetupProjectEvent(
    const Vector2 &windowSize, nanoem_f32_t windowDevicePixelRatio, nanoem_f32_t viewportDevicePixelRatio)
{
    Nanoem__Application__SetupProjectEvent base = NANOEM__APPLICATION__SETUP_PROJECT_EVENT__INIT;
    base.window_width = static_cast<int>(windowSize.x);
    base.window_height = static_cast<int>(windowSize.y);
    base.window_device_pixel_ratio = windowDevicePixelRatio;
    base.viewport_device_pixel_ratio = viewportDevicePixelRatio;
    base.has_viewport_device_pixel_ratio = 1;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_SETUP_PROJECT;
    event.setup_project = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishSetEditingModeEvent(nanoem_u32_t value)
{
    Nanoem__Application__SetEditingModeEvent base = NANOEM__APPLICATION__SET_EDITING_MODE_EVENT__INIT;
    base.value = value;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_SET_EDITING_MODE;
    event.set_editing_mode = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishCompleteExportingImageConfigurationEvent(
    const StringList &availableExtensions)
{
    Nanoem__Application__CompleteExportImageConfigurationEvent base =
        NANOEM__APPLICATION__COMPLETE_EXPORT_IMAGE_CONFIGURATION_EVENT__INIT;
    MutableStringList ae;
    internal::ApplicationUtils::allocateStringList(
        availableExtensions, base.available_extensions, base.n_available_extensions, ae);
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_COMPLETE_EXPORT_IMAGE_CONFIGURATION;
    event.complete_export_image_configuration = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishCompleteExportingVideoConfigurationEvent(
    const StringList &availableExtensions)
{
    Nanoem__Application__CompleteExportVideoConfigurationEvent base =
        NANOEM__APPLICATION__COMPLETE_EXPORT_VIDEO_CONFIGURATION_EVENT__INIT;
    MutableStringList ae;
    internal::ApplicationUtils::allocateStringList(
        availableExtensions, base.available_extensions, base.n_available_extensions, ae);
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_COMPLETE_EXPORT_VIDEO_CONFIGURATION;
    event.complete_export_video_configuration = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishCanCopyEvent(bool value)
{
    Nanoem__Application__CanCopyEvent base = NANOEM__APPLICATION__CAN_COPY_EVENT__INIT;
    base.value = value ? 1 : 0;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_CAN_COPY_EVENT;
    event.can_copy_event = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishCanPasteEvent(bool value)
{
    Nanoem__Application__CanPasteEvent base = NANOEM__APPLICATION__CAN_PASTE_EVENT__INIT;
    base.value = value ? 1 : 0;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_CAN_PASTE_EVENT;
    event.can_paste_event = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishSetWindowDevicePixelRatioEvent(nanoem_f32_t value)
{
    Nanoem__Application__SetWindowDevicePixelRatioEvent base =
        NANOEM__APPLICATION__SET_WINDOW_DEVICE_PIXEL_RATIO_EVENT__INIT;
    base.value = value;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_SET_WINDOW_DEVICE_PIXEL_RATIO_EVENT;
    event.set_window_device_pixel_ratio_event = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishSetViewportDevicePixelRatioEvent(nanoem_f32_t value)
{
    Nanoem__Application__SetViewportDevicePixelRatioEvent base =
        NANOEM__APPLICATION__SET_VIEWPORT_DEVICE_PIXEL_RATIO_EVENT__INIT;
    base.value = value;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_SET_VIEWPORT_DEVICE_PIXEL_RATIO_EVENT;
    event.set_viewport_device_pixel_ratio_event = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishDisableCursorEvent(const Vector2 &value)
{
    Nanoem__Application__DisableCursorEvent base = NANOEM__APPLICATION__DISABLE_CURSOR_EVENT__INIT;
    base.x = nanoem_i32_t(value.x);
    base.y = nanoem_i32_t(value.y);
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_DISABLE_CURSOR;
    event.disable_cursor = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishEnableCursorEvent(const Vector2 &value)
{
    Nanoem__Application__EnableCursorEvent base = NANOEM__APPLICATION__ENABLE_CURSOR_EVENT__INIT;
    base.x = nanoem_i32_t(value.x);
    base.y = nanoem_i32_t(value.y);
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_ENABLE_CURSOR;
    event.enable_cursor = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishQuitApplicationEvent()
{
    Nanoem__Application__QuitApplicationEvent base = NANOEM__APPLICATION__QUIT_APPLICATION_EVENT__INIT;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.timestamp = internal::ApplicationUtils::timestamp();
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_QUIT_APPLICATION;
    event.quit_application = &base;
    sendEventMessage(event);
}

void
BaseApplicationService::EventPublisher::publishErrorEvent(const Error &error)
{
    if (error.hasReason() && !error.isCancelled()) {
        Nanoem__Application__ErrorEvent base = NANOEM__APPLICATION__ERROR_EVENT__INIT;
        MutableString reason, suggestion;
        StringUtils::copyString(error.reasonConstString(), reason);
        StringUtils::copyString(error.recoverySuggestionConstString(), suggestion);
        base.code = Inline::saturateInt32(error.code());
        base.reason = reason.data();
        base.recovery_suggestion = suggestion.data();
        Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
        event.timestamp = internal::ApplicationUtils::timestamp();
        event.type_case = NANOEM__APPLICATION__EVENT__TYPE_ERROR;
        event.error = &base;
        sendEventMessage(event);
        if (g_sentryAvailable) {
            sentry_value_t reason = g_sentryMaskStringProc(error.reasonConstString()),
                           breadcrumb = sentry_value_new_breadcrumb(nullptr, sentry_value_as_string(reason));
            sentry_value_set_by_key(breadcrumb, "category", sentry_value_new_string("error"));
            sentry_value_set_by_key(
                breadcrumb, "domain", sentry_value_new_string(Error::convertDomainToString(error.domain())));
            sentry_add_breadcrumb(breadcrumb);
            sentry_value_decref(reason);
        }
    }
}

void
BaseApplicationService::EventPublisher::sendEventMessage(const Nanoem__Application__Event &event)
{
    m_service->sendEventMessage(&event);
}

Vector2UI16
BaseApplicationService::minimumRequiredWindowSize() NANOEM_DECL_NOEXCEPT
{
    return internal::ImGuiWindow::kMinimumMainWindowSize;
}

void
BaseApplicationService::setup() NANOEM_DECL_NOEXCEPT
{
    if (!g_initialized) {
        stm_setup();
        g_initialized = true;
    }
}

void *
BaseApplicationService::openSentryDll(const SentryDescription &desc)
{
    void *handle = bx::dlopen(desc.m_dllFilePath);
    if (handle) {
        bool valid = true;
        Inline::resolveSymbol(handle, "sentry_options_new", sentry_options_new, valid);
        Inline::resolveSymbol(handle, "sentry_options_set_debug", sentry_options_set_debug, valid);
        Inline::resolveSymbol(handle, "sentry_options_set_dsn", sentry_options_set_dsn, valid);
        Inline::resolveSymbol(handle, "sentry_options_set_release", sentry_options_set_release, valid);
        Inline::resolveSymbol(handle, "sentry_options_set_handler_path", sentry_options_set_handler_path, valid);
        Inline::resolveSymbol(handle, "sentry_options_set_database_path", sentry_options_set_database_path, valid);
        Inline::resolveSymbol(handle, "sentry_options_set_transport", sentry_options_set_transport, valid);
        Inline::resolveSymbol(handle, "sentry_init", sentry_init, valid);
        Inline::resolveSymbol(handle, "sentry_value_new_breadcrumb", sentry_value_new_breadcrumb, valid);
        Inline::resolveSymbol(handle, "sentry_value_new_int32", sentry_value_new_int32, valid);
        Inline::resolveSymbol(handle, "sentry_value_new_double", sentry_value_new_double, valid);
        Inline::resolveSymbol(handle, "sentry_value_new_string", sentry_value_new_string, valid);
        Inline::resolveSymbol(handle, "sentry_value_new_object", sentry_value_new_object, valid);
        Inline::resolveSymbol(handle, "sentry_value_set_by_key", sentry_value_set_by_key, valid);
        Inline::resolveSymbol(handle, "sentry_value_as_string", sentry_value_as_string, valid);
        Inline::resolveSymbol(handle, "sentry_value_decref", sentry_value_decref, valid);
        Inline::resolveSymbol(handle, "sentry_add_breadcrumb", sentry_add_breadcrumb, valid);
        Inline::resolveSymbol(handle, "sentry_set_extra", sentry_set_extra, valid);
        Inline::resolveSymbol(handle, "sentry_set_tag", sentry_set_tag, valid);
        Inline::resolveSymbol(handle, "sentry_set_user", sentry_set_user, valid);
        Inline::resolveSymbol(handle, "sentry_envelope_free", sentry_envelope_free, valid);
        Inline::resolveSymbol(handle, "sentry_envelope_serialize", sentry_envelope_serialize, valid);
        Inline::resolveSymbol(handle, "sentry_free", sentry_free, valid);
        Inline::resolveSymbol(handle, "sentry_transport_new", sentry_transport_new, valid);
        Inline::resolveSymbol(handle, "sentry_transport_set_state", sentry_transport_set_state, valid);
        Inline::resolveSymbol(handle, "sentry_transport_set_free_func", sentry_transport_set_free_func, valid);
        Inline::resolveSymbol(handle, "sentry_transport_set_startup_func", sentry_transport_set_startup_func, valid);
        Inline::resolveSymbol(handle, "sentry_transport_set_shutdown_func", sentry_transport_set_shutdown_func, valid);
        Inline::resolveSymbol(handle, "sentry_transport_free", sentry_transport_free, valid);
        Inline::resolveSymbol(handle, "sentry_shutdown", sentry_shutdown, valid);
        if (valid) {
            sentry_options_t *options = sentry_options_new();
            sentry_options_set_release(options, nanoemGetVersionString());
            sentry_options_set_handler_path(options, desc.m_handlerFilePath);
            sentry_options_set_database_path(options, desc.m_databaseDirectoryPath);
            if (SentryDescription::PFN_transport_send_envelope sendEnvelope = desc.m_transportSendEnvelope) {
                sentry_transport_t *transport = sentry_transport_new(sendEnvelope);
                sentry_transport_set_state(transport, desc.m_transportUserData);
                sentry_options_set_transport(options, transport);
            }
#if !defined(NDEBUG)
            sentry_options_set_debug(options, 1);
#endif
            if (const char *dsn = desc.m_dsn) {
                sentry_options_set_dsn(options, dsn);
            }
            sentry_init(options);
            if (const char *deviceModel = desc.m_deviceModelName) {
                sentry_set_tag("device.model", deviceModel);
            }
            if (const char *localeName = desc.m_localeName) {
                sentry_set_tag("nanoem.locale", localeName);
            }
            if (const char *rendererName = desc.m_rendererName) {
                sentry_set_tag("nanoem.renderer", rendererName);
            }
            if (const char *clientUUID = desc.m_clientUUID) {
                sentry_value_t user = sentry_value_new_object();
                sentry_value_set_by_key(user, "id", sentry_value_new_string(clientUUID));
                sentry_set_user(user);
            }
            sentry_set_tag("nanoem.model.editable", desc.m_isModelEditingEnabled ? "true" : "false");
            sentry_value_t breadcrumb = sentry_value_new_breadcrumb(nullptr, nullptr);
            sentry_value_set_by_key(breadcrumb, "category", sentry_value_new_string("init"));
            sentry_add_breadcrumb(breadcrumb);
            g_sentryAvailable = true;
            g_sentryMaskStringProc = desc.m_maskString ? desc.m_maskString : defaultMaskStringFunc;
        }
        else {
            bx::dlclose(handle);
            handle = nullptr;
        }
    }
    return handle;
}

void
BaseApplicationService::closeSentryDll(void *handle)
{
    if (handle) {
        sentry_shutdown();
        g_sentryAvailable = false;
        g_sentryMaskStringProc = nullptr;
        bx::dlclose(handle);
    }
}

BaseApplicationService::BaseApplicationService(const JSON_Value *root)
    : m_applicationConfiguration(root)
    , m_defaultFileManager(nullptr)
    , m_stateController(nullptr)
    , m_eventPublisherPtr(nullptr)
    , m_translatorPtr(nullptr)
    , m_applicationPendingChangeConfiguration(nullptr)
    , m_capturingPassState(nullptr)
    , m_window(nullptr)
    , m_confirmer(this)
    , m_sharedCancelPublisherRepository(this)
    , m_sharedDebugCaptureRepository(this)
    , m_defaultAuxFlags(Project::kDrawTypeBoneTooltip)
    , m_dllHandle(nullptr)
    , m_initialized(false)
{
    m_context = { SG_INVALID_ID };
    m_defaultFileManager = nanoem_new(DefaultFileManager(this));
    m_eventPublisher = nanoem_new(EventPublisher(this));
    m_stateController = nanoem_new(StateController(this, m_defaultFileManager));
    m_applicationPendingChangeConfiguration = json_value_init_object();
    setEventPublisher(m_eventPublisher);
    setFileManager(m_defaultFileManager);
}

BaseApplicationService::~BaseApplicationService() NANOEM_DECL_NOEXCEPT
{
    json_value_free(m_applicationPendingChangeConfiguration);
    m_applicationPendingChangeConfiguration = nullptr;
    nanoem_delete_safe(m_defaultFileManager);
    nanoem_delete_safe(m_eventPublisher);
    nanoem_delete_safe(m_stateController);
    nanoem_delete_safe(m_window);
}

void
BaseApplicationService::initialize(nanoem_f32_t windowDevicePixelRatio, nanoem_f32_t viewportDevicePixelRatio)
{
    m_window = nanoem_new(internal::ImGuiWindow(this));
    m_window->initialize(windowDevicePixelRatio, viewportDevicePixelRatio);
}

void
BaseApplicationService::destroy()
{
    SG_PUSH_GROUP("BaseApplication::destroy");
    if (m_capturingPassState) {
        while (!m_capturingPassState->transitDestruction(nullptr)) {
        }
        nanoem_delete_safe(m_capturingPassState);
    }
    m_sharedResourceRepository.destroy();
    m_window->destroy();
    SG_POP_GROUP();
}

Project *
BaseApplicationService::createProject(const Vector2UI16 &logicalPixelWindowSize, sg_pixel_format pixelFormat,
    nanoem_f32_t windowDevicePixelRatio, nanoem_f32_t viewportDevicePixelRatio, const char *dllPath)
{
    ApplicationPreference preference(this);
    m_eventPublisherPtr->publishSetupProjectEvent(
        logicalPixelWindowSize, windowDevicePixelRatio, viewportDevicePixelRatio);
    m_window->reset();
    Error error;
    Project::Injector injector;
    injector.m_audioPlayer = createAudioPlayer();
    injector.m_applicationConfiguration = m_applicationConfiguration;
    injector.m_backgroundVideoRenderer = createBackgroundVideoRenderer();
    injector.m_confirmerPtr = &m_confirmer;
    injector.m_dllPath = dllPath;
    injector.m_fileManagerPtr = fileManager();
    injector.m_skinDeformerFactory = createSkinDeformerFactory();
    injector.m_eventPublisherPtr = eventPublisher();
    injector.m_primitive2DPtr = m_window->primitiveContext();
    injector.m_rendererCapability = createRendererCapability();
    injector.m_sharedCancelPublisherRepositoryPtr = sharedCancelPublisherRepository();
    injector.m_sharedDebugCaptureRepository = sharedDebugCaptureRepository();
    injector.m_sharedResourceRepositoryPtr = sharedResourceRepository();
    injector.m_translatorPtr = translator();
    injector.m_unicodeStringFactoryRepository = &m_unicodeStringFactoryRepository;
    injector.m_windowSize = logicalPixelWindowSize;
    injector.m_pixelFormat = pixelFormat;
    injector.m_windowDevicePixelRatio = windowDevicePixelRatio;
    injector.m_viewportDevicePixelRatio = viewportDevicePixelRatio;
    injector.m_preferredUndoCount = preference.undoSoftLimit();
    Project *project = nanoem_new(Project(injector));
    if (!project->initialize(error)) {
        error.addModalDialog(this);
    }
    project->setEffectPluginEnabled(preference.isEffectEnabled());
    project->setCompiledEffectCacheEnabled(preference.isEffectCacheEnabled());
    const Vector2UI16 devicePixelWindowSize(Vector2(logicalPixelWindowSize) * project->windowDevicePixelRatio());
    m_window->resizeDevicePixelWindowSize(devicePixelWindowSize);
    if (g_sentryAvailable) {
        sentry_value_t breadcrumb = sentry_value_new_breadcrumb(nullptr, nullptr);
        sentry_value_t data = sentry_value_new_object();
        sentry_value_set_by_key(data, "width", sentry_value_new_int32(devicePixelWindowSize.x));
        sentry_value_set_by_key(data, "height", sentry_value_new_int32(devicePixelWindowSize.y));
        sentry_value_set_by_key(breadcrumb, "category", sentry_value_new_string("new"));
        sentry_value_set_by_key(breadcrumb, "data", data);
        sentry_add_breadcrumb(breadcrumb);
    }
    return project;
}

void
BaseApplicationService::destroyProject(Project *project)
{
    if (nanoem_likely(project)) {
        project->stop();
        if (project->hasTransientPath()) {
            FileUtils::TransientPath path(project->transientPath());
            FileUtils::deleteTransientFile(path);
            project->setTransientPath(path);
        }
        project->destroy();
        nanoem_delete(project);
    }
}

void
BaseApplicationService::draw(Project *project, Project::IViewportOverlay *overlay)
{
    if (nanoem_likely(project)) {
        Error error;
        project->drawShadowMap();
        project->drawAllOffscreenRenderTargets();
        project->drawViewport();
        bool stopped = false;
        if (m_capturingPassState) {
            stopped = m_capturingPassState->capture(project, error);
        }
        project->flushAllCommandBuffers();
        if (stopped) {
            stopCapture(project);
            if (g_sentryAvailable) {
                sentry_value_t breadcrumb = sentry_value_new_breadcrumb(nullptr, nullptr);
                sentry_value_set_by_key(breadcrumb, "category", sentry_value_new_string("capture.stop"));
                sentry_add_breadcrumb(breadcrumb);
            }
            else if (error.hasReason()) {
                error.addModalDialog(this);
            }
        }
        m_window->drawAll2DPrimitives(project, overlay, m_defaultAuxFlags);
        m_window->drawAllWindows(project, m_stateController->state(), m_defaultAuxFlags);
    }
}

const char *
BaseApplicationService::translateMenuItem(nanoem_u32_t type) const NANOEM_DECL_NOEXCEPT
{
    return translator()->translate(
        ApplicationMenuBuilder::menuItemString(static_cast<ApplicationMenuBuilder::MenuItemType>(type)));
}

bool
BaseApplicationService::dispatchMenuItemAction(Project *project, nanoem_u32_t type, Error &error)
{
    nanoem_parameter_assert(project, "must NOT be nullptr");
    bool handled = true;
    switch (type) {
    case ApplicationMenuBuilder::kMenuItemTypeEditUndo: {
        project->handleUndoAction();
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeEditRedo: {
        project->handleRedoAction();
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeEditCopy: {
        if (project->hasInputTextFocus()) {
            project->setCursorModifiers(Project::kCursorModifierTypeControl);
            m_window->setKeyPressed(kKeyType_C);
        }
        else {
            project->handleCopyAction(error);
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeEditCut: {
        if (project->hasInputTextFocus()) {
            project->setCursorModifiers(Project::kCursorModifierTypeControl);
            m_window->setKeyPressed(kKeyType_X);
        }
        else {
            project->handleCutAction(error);
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeEditPaste: {
        if (project->hasInputTextFocus()) {
            project->setCursorModifiers(Project::kCursorModifierTypeControl);
            m_window->setKeyPressed(kKeyType_P);
        }
        else {
            project->handlePasteAction(error);
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeEditBoneOpenParameterDialog: {
        m_window->openBoneParametersDialog(project);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeEditBoneOpenCorrectionDialog: {
        m_window->openBoneCorrectionDialog(project);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeEditBoneOpenBiasDialog: {
        m_window->openBoneBiasDialog(project);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeEditBoneResetAngle: {
        if (Model *activeModel = project->activeModel()) {
            activeModel->registerResetActiveBoneTransformCommand(Model::kResetTypeOrientation);
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeEditCameraOpenParameterDialog: {
        m_window->openCameraParametersDialog(project);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeEditCameraOpenCorrectionDialog: {
        m_window->openCameraCorrectionDialog(project);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeEditCameraResetAngle: {
        ICamera *camera = project->activeCamera();
        camera->setAngle(Constants::kZeroV3);
        camera->update();
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeEditMorphOpenCorrectionDialog: {
        m_window->openMorphCorrectionDialog(project);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeEditMorphRemoveAllLipKeyframes: {
        CommandRegistrator registrator(project);
        registrator.registerRemoveAllLipMorphKeyframesCommand();
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeEditMorphRemoveAllEyeKeyframes: {
        CommandRegistrator registrator(project);
        registrator.registerRemoveAllEyeMorphKeyframesCommand();
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeEditMorphRemoveAllEyebrowKeyframes: {
        CommandRegistrator registrator(project);
        registrator.registerRemoveAllEyebrowMorphKeyframesCommand();
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeEditMorphResetAllMorphs: {
        if (Model *activeModel = project->activeModel()) {
            activeModel->registerResetAllMorphWeightsCommand();
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeEditMorphRegisterAllMorphs: {
        CommandRegistrator registrator(project);
        registrator.registerAddMorphKeyframesCommandByAllMorphs();
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeEditOpenEffectParameterWindow: {
        m_window->openEffectParameterDialog(project);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeEditOpenModelParameterWindow: {
        m_window->openModelParameterDialog(project);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeEditSelectAll: {
        project->selectAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeEditPreference: {
        m_window->openPreferenceDialog();
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeProjectPlay: {
        project->play();
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeProjectStop: {
        project->stop();
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeProjectOpenViewportDialog: {
        m_window->openViewportDialog(project);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeProjectOpenDrawOrderDialog: {
        m_window->openModelDrawOrderDialog(project);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeProjectOpenTransformOrderDialog: {
        m_window->openModelTransformOrderDialog(project);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeProjectEnableLoop: {
        project->setLoopEnabled(project->isLoopEnabled() ? false : true);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeProjectEnableGrid: {
        Grid *grid = project->grid();
        grid->setVisible(grid->isVisible() ? false : true);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeProjectEnableGroundShadow: {
        project->setGroundShadowEnabled(project->isGroundShadowEnabled() ? false : true);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeProjectEnableEffect: {
        project->setEffectPluginEnabled(project->isEffectPluginEnabled() ? false : true);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeProjectEnableHighResolutionViewport: {
        const nanoem_f32_t windowDevicePixelRatio = project->windowDevicePixelRatio();
        const bool enabled = windowDevicePixelRatio > project->viewportDevicePixelRatio();
        project->setViewportDevicePixelRatio(enabled ? windowDevicePixelRatio : 1.0f);
        m_window->setAntiAliasEnabled(enabled || project->sampleLevel() > 0);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeProjectEnableMSAAx16:
    case ApplicationMenuBuilder::kMenuItemTypeProjectEnableMSAAx8:
    case ApplicationMenuBuilder::kMenuItemTypeProjectEnableMSAAx4:
    case ApplicationMenuBuilder::kMenuItemTypeProjectEnableMSAAx2: {
        const uint32_t level = ApplicationMenuBuilder::kMenuItemTypeProjectDisableMSAA - (type + 1);
        project->setSampleLevel(level);
        m_window->setAntiAliasEnabled(true);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeProjectDisableMSAA: {
        project->setSampleLevel(0);
        m_window->setAntiAliasEnabled(project->viewportDevicePixelRatio() > 1.0f);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeProjectPhysicsSimulationEnableAnytime: {
        project->setPhysicsSimulationMode(PhysicsEngine::kSimulationModeEnableAnytime);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeProjectPhysicsSimulationEnablePlaying: {
        project->setPhysicsSimulationMode(PhysicsEngine::kSimulationModeEnablePlaying);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeProjectPhysicsSimulationEnableTracing: {
        project->setPhysicsSimulationMode(PhysicsEngine::kSimulationModeEnableTracing);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeProjectPhysicsSimulationDisable: {
        project->setPhysicsSimulationMode(PhysicsEngine::kSimulationModeDisable);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeProjectPhysicsSimulationBakeAllMotions:
    case ApplicationMenuBuilder::kMenuItemTypeProjectPhysicsSimulationBakeAllMotionsWithIK: {
        const bool enableConstraints =
            type == ApplicationMenuBuilder::kMenuItemTypeProjectPhysicsSimulationBakeAllMotionsWithIK;
        CommandRegistrator registrator(project);
        registrator.registerBakeAllModelMotionsCommand(enableConstraints, error);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeProjectPhysicsSimulationConfiguration: {
        m_window->openPhysicsEngineDialog(project);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeProjectPhysicsSimulationEnableDrawingWireframe: {
        nanoem_u32_t flags = project->physicsEngine()->debugGeometryFlags();
        EnumUtils::setEnabled(PhysicsEngine::kDebugDrawWireframe, flags,
            EnumUtils::isEnabled(PhysicsEngine::kDebugDrawWireframe, flags) ? false : true);
        project->setPhysicsSimulationEngineDebugFlags(flags);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeProjectPhysicsSimulationEnableDrawingAABB: {
        nanoem_u32_t flags = project->physicsEngine()->debugGeometryFlags();
        EnumUtils::setEnabled(PhysicsEngine::kDebugDrawAabb, flags,
            EnumUtils::isEnabled(PhysicsEngine::kDebugDrawAabb, flags) ? false : true);
        project->setPhysicsSimulationEngineDebugFlags(flags);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeProjectPhysicsSimulationEnableDrawingContactPoints: {
        nanoem_u32_t flags = project->physicsEngine()->debugGeometryFlags();
        EnumUtils::setEnabled(PhysicsEngine::kDebugDrawContactPoints, flags,
            EnumUtils::isEnabled(PhysicsEngine::kDebugDrawContactPoints, flags) ? false : true);
        project->setPhysicsSimulationEngineDebugFlags(flags);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeProjectPhysicsSimulationEnableDrawingConstraints: {
        nanoem_u32_t flags = project->physicsEngine()->debugGeometryFlags();
        EnumUtils::setEnabled(PhysicsEngine::kDebugDrawConstraints, flags,
            EnumUtils::isEnabled(PhysicsEngine::kDebugDrawConstraints, flags) ? false : true);
        project->setPhysicsSimulationEngineDebugFlags(flags);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeProjectPhysicsSimulationEnableDrawingConstraintLimit: {
        nanoem_u32_t flags = project->physicsEngine()->debugGeometryFlags();
        EnumUtils::setEnabled(PhysicsEngine::kDebugDrawConstraintLimits, flags,
            EnumUtils::isEnabled(PhysicsEngine::kDebugDrawConstraintLimits, flags) ? false : true);
        project->setPhysicsSimulationEngineDebugFlags(flags);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeProjectPreferredMotionFPSUnlimited: {
        project->setPreferredMotionFPS(UINT32_MAX, true);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeProjectPreferredMotionFPS60: {
        project->setPreferredMotionFPS(60, false);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeProjectPreferredMotionFPS30: {
        project->setPreferredMotionFPS(30, false);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeProjectClearAudioSource: {
        project->clearAudioSource(error);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeProjectClearBackgroundVideo: {
        project->clearBackgroundVideo();
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeProjectEnableFPSCounter: {
        project->setFPSCounterEnabled(project->isFPSCounterEnabled() ? false : true);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeProjectEnablePerformanceMonitor: {
        project->setPerformanceMonitorEnabled(project->isPerformanceMonitorEnabled() ? false : true);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeCameraPresetTop: {
        ICamera *camera = project->activeCamera();
        camera->setAngle(glm::radians(Vector3(90, 0, 0)));
        camera->update();
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeCameraPresetLeft: {
        ICamera *camera = project->activeCamera();
        camera->setAngle(glm::radians(Vector3(0, -90, 0)));
        camera->update();
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeCameraPresetRight: {
        ICamera *camera = project->activeCamera();
        camera->setAngle(glm::radians(Vector3(0, 90, 0)));
        camera->update();
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeCameraPresetBottom: {
        ICamera *camera = project->activeCamera();
        camera->setAngle(glm::radians(Vector3(-90, 0, 0)));
        camera->update();
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeCameraPresetFront: {
        ICamera *camera = project->activeCamera();
        camera->setAngle(Constants::kZeroV3);
        camera->update();
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeCameraPresetBack: {
        ICamera *camera = project->activeCamera();
        camera->setAngle(glm::radians(Vector3(0, 180, 0)));
        camera->update();
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeCameraRegisterKeyframe: {
        CommandRegistrator registrator(project);
        registrator.registerAddCameraKeyframesCommandByCurrentLocalFrameIndex();
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeCameraRemoveAllSelectedKeyframes: {
        CommandRegistrator registrator(project);
        registrator.registerRemoveCameraKeyframesCommandByCurrentLocalFrameIndex();
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeCameraReset: {
        project->activeCamera()->reset();
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeLightSelfShadowDisable: {
        project->setShadowMapEnabled(false);
        project->shadowCamera()->setCoverageMode(ShadowCamera::kCoverageModeTypeNone);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeLightSelfShadowEnableWithMode1: {
        project->setShadowMapEnabled(true);
        project->shadowCamera()->setCoverageMode(ShadowCamera::kCoverageModeType1);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeLightSelfShadowEnableWithMode2: {
        project->setShadowMapEnabled(true);
        project->shadowCamera()->setCoverageMode(ShadowCamera::kCoverageModeType2);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeLightSelfShadowRegisterKeyframe: {
        CommandRegistrator registrator(project);
        registrator.registerAddSelfShadowKeyframesCommandByCurrentLocalFrameIndex();
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeLightSelfShadowRemoveAllSelectedKeyframes: {
        CommandRegistrator registrator(project);
        registrator.registerRemoveSelfShadowKeyframesCommandByCurrentLocalFrameIndex();
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeLightSelfShadowReset: {
        project->shadowCamera()->reset();
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeLightRegisterKeyframe: {
        CommandRegistrator registrator(project);
        registrator.registerAddLightKeyframesCommandByCurrentLocalFrameIndex();
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeLightRemoveAllSelectedKeyframes: {
        CommandRegistrator registrator(project);
        registrator.registerRemoveLightKeyframesCommandByCurrentLocalFrameIndex();
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeLightReset: {
        project->activeLight()->reset();
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeModelSelectAllBoneKeyframesFromSelectedBoneSet: {
        if (Model *activeModel = project->activeModel()) {
            project->selectAllBoneKeyframesFromSelectedBoneSet(activeModel);
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeModelExpandAllTracks: {
        project->expandAllTracks();
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeModelCollapseAllTracks: {
        project->collapseAllTracks();
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeModelEditModeSelect: {
        project->setEditingMode(Project::kEditingModeSelect);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeModelEditModeRotate: {
        project->setEditingMode(Project::kEditingModeRotate);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeModelEditModeMove: {
        project->setEditingMode(Project::kEditingModeMove);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeModelResetBoneAxisX: {
        if (Model *activeModel = project->activeModel()) {
            activeModel->registerResetActiveBoneTransformCommand(Model::kResetTypeTranslationAxisX);
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeModelResetBoneAxisY: {
        if (Model *activeModel = project->activeModel()) {
            activeModel->registerResetActiveBoneTransformCommand(Model::kResetTypeTranslationAxisY);
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeModelResetBoneAxisZ: {
        if (Model *activeModel = project->activeModel()) {
            activeModel->registerResetActiveBoneTransformCommand(Model::kResetTypeTranslationAxisZ);
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeModelResetBoneOrientation: {
        if (Model *activeModel = project->activeModel()) {
            activeModel->registerResetActiveBoneTransformCommand(Model::kResetTypeOrientation);
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeModelMorphSetZero: {
        setMorphWeight(0.0f, project);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeModelMorphSetHalf: {
        setMorphWeight(0.5f, project);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeModelMorphSetOne: {
        setMorphWeight(1.0f, project);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeModelEnableAddBlend: {
        if (Model *activeModel = project->activeModel()) {
            activeModel->setAddBlendEnabled(activeModel->isAddBlendEnabled() ? false : true);
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeModelEnableShadowMap: {
        if (Model *activeModel = project->activeModel()) {
            activeModel->setShadowMapEnabled(activeModel->isShadowMapEnabled() ? false : true);
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeModelEnableVisible: {
        if (Model *activeModel = project->activeModel()) {
            activeModel->setVisible(activeModel->isVisible() ? false : true);
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeModelPreferenceEnableShowAllBones: {
        if (Model *activeModel = project->activeModel()) {
            activeModel->setShowAllBones(activeModel->isShowAllBones() ? false : true);
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeModelPreferenceEnableShowRigidBodies: {
        if (Model *activeModel = project->activeModel()) {
            activeModel->setShowAllRigidBodies(activeModel->isShowAllRigidBodies() ? false : true);
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeModelPreferenceEnableShowJoints: {
        if (Model *activeModel = project->activeModel()) {
            activeModel->setShowAllJoints(activeModel->isShowAllJoints() ? false : true);
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeModelPreferenceEnableShowVertexFaces: {
        if (Model *activeModel = project->activeModel()) {
            activeModel->setShowAllVertexFaces(activeModel->isShowAllVertexFaces() ? false : true);
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeModelPreferenceEnableShowVertexPoints: {
        if (Model *activeModel = project->activeModel()) {
            activeModel->setShowAllVertexPoints(activeModel->isShowAllVertexPoints() ? false : true);
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeModelRegisterKeyframe: {
        CommandRegistrator registrator(project);
        registrator.registerAddModelKeyframesCommandByCurrentLocalFrameIndex();
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeModelRemoveAllSelectedKeyframes: {
        CommandRegistrator registrator(project);
        registrator.registerRemoveModelKeyframesCommandByCurrentLocalFrameIndex();
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeModelEdgeConfiguraiton: {
        m_window->openModelEdgeDialog(project);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeModelReset: {
        if (Model *activeModel = project->activeModel()) {
            activeModel->reset();
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeModelDelete: {
        if (Model *activeModel = project->activeModel()) {
            addModalDialog(ModalDialogFactory::createConfirmDeletingModelDialog(activeModel, this));
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeAccessoryRegisterKeyframe: {
        if (Accessory *activeAccessory = project->activeAccessory()) {
            CommandRegistrator registrator(project);
            registrator.registerAddAccessoryKeyframesCommandByCurrentLocalFrameIndex(activeAccessory);
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeAccessoryRemoveAllSelectedKeyframes: {
        if (Accessory *activeAccessory = project->activeAccessory()) {
            CommandRegistrator registrator(project);
            registrator.registerRemoveAccessoryKeyframesCommandByCurrentLocalFrameIndex(activeAccessory);
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeAccessoryEnableAddBlend: {
        if (Accessory *activeAccessory = project->activeAccessory()) {
            activeAccessory->setAddBlendEnabled(activeAccessory->isAddBlendEnabled() ? false : true);
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeAccessoryEnableShadow: {
        if (Accessory *activeAccessory = project->activeAccessory()) {
            activeAccessory->setShadowMapEnabled(activeAccessory->isShadowMapEnabled() ? false : true);
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeAccessoryEnableVisible: {
        if (Accessory *activeAccessory = project->activeAccessory()) {
            activeAccessory->setVisible(activeAccessory->isVisible() ? false : true);
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeAccessoryReset: {
        if (Accessory *activeAccessory = project->activeAccessory()) {
            activeAccessory->reset();
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeAccessoryDelete: {
        if (Accessory *activeAccessory = project->activeAccessory()) {
            addModalDialog(ModalDialogFactory::createConfirmDeletingAccessoryDialog(activeAccessory, this));
        }
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeMotionRemoveTimelineFrame: {
        CommandRegistrator registrator(project);
        registrator.registerRemoveTimelineFrameCommand();
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeMotionInsertEmptyTimelineFrame: {
        CommandRegistrator registrator(project);
        registrator.registerInsertEmptyTimelineFrameCommand();
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeMotionReset: {
        CommandRegistrator registrator(project);
        registrator.registerInitializeMotionCommand(error);
        break;
    }
    case ApplicationMenuBuilder::kMenuItemTypeHelpAbout: {
        addModalDialog(ModalDialogFactory::createAboutDialog(this));
        break;
    }
    default:
        handled = false;
        break;
    }
    error.notify(eventPublisher());
    return handled;
}

void
BaseApplicationService::setKeyPressed(KeyType key)
{
    m_window->setKeyPressed(key);
}

void
BaseApplicationService::setKeyReleased(KeyType key)
{
    m_window->setKeyReleased(key);
}

void
BaseApplicationService::setUnicodePressed(nanoem_u32_t value)
{
    m_window->setUnicodePressed(value);
}

void
BaseApplicationService::dispatchCommandMessage(const nanoem_u8_t *data, size_t size, Project *project, bool forceUndo)
{
    if (Nanoem__Application__Command *command =
            nanoem__application__command__unpack(g_protobufc_allocator, size, data)) {
        if (forceUndo && command->type_case == NANOEM__APPLICATION__COMMAND__TYPE__NOT_SET) {
            command->type_case = NANOEM__APPLICATION__COMMAND__TYPE_UNDO;
        }
        Error error;
        bool succeeded = handleCommandMessage(command, project, error);
        nanoem__application__command__free_unpacked(command, g_protobufc_allocator);
        if (!succeeded) {
            error.addModalDialog(this);
        }
        else if (error.hasReason()) {
            error.notify(eventPublisher());
        }
    }
}

void
BaseApplicationService::sendLoadingAllModelIOPluginsEventMessage(int language)
{
    DefaultFileManager::ModelIOPluginList plugins;
    m_defaultFileManager->getAllModelIOPlugins(plugins);
    if (!plugins.empty()) {
        Nanoem__Application__CompleteLoadingAllModelIOPluginsEvent base =
            NANOEM__APPLICATION__COMPLETE_LOADING_ALL_MODEL_IOPLUGINS_EVENT__INIT;
        MutableStringList names, descriptions, functions;
        if (!plugins.empty()) {
            base.n_items = plugins.size();
            base.items = new Nanoem__Application__Plugin *[base.n_items];
            names.resize(base.n_items);
            descriptions.resize(base.n_items);
            nanoem_rsize_t i = 0;
            for (DefaultFileManager::ModelIOPluginList::const_iterator it = plugins.begin(), end = plugins.end();
                 it != end; ++it) {
                PluginFactory::ModelIOPluginProxy proxy(*it);
                proxy.setLanguage(language);
                const nanoem_rsize_t offset = functions.size();
                Nanoem__Application__Plugin *plugin = base.items[i] = nanoem_new(Nanoem__Application__Plugin);
                functions.resize(offset + proxy.countAllFunctions());
                internal::ApplicationUtils::fillPluginMessage(
                    proxy, plugin, names[i], descriptions[i], functions, offset);
                i++;
            }
        }
        Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
        event.type_case = NANOEM__APPLICATION__EVENT__TYPE_COMPLETE_LOADING_ALL_MODEL_IO_PLUGINS;
        event.complete_loading_all_model_io_plugins = &base;
        sendEventMessage(&event);
        for (nanoem_rsize_t i = 0, numItems = base.n_items; i < numItems; i++) {
            nanoem_delete(base.items[i]);
        }
        delete[] base.items;
    }
}

void
BaseApplicationService::sendLoadingAllMotionIOPluginsEventMessage(int language)
{
    DefaultFileManager::MotionIOPluginList plugins;
    m_defaultFileManager->getAllMotionIOPlugins(plugins);
    if (!plugins.empty()) {
        Nanoem__Application__CompleteLoadingAllMotionIOPluginsEvent base =
            NANOEM__APPLICATION__COMPLETE_LOADING_ALL_MODEL_IOPLUGINS_EVENT__INIT;
        MutableStringList names, descriptions, functions;
        if (!plugins.empty()) {
            base.n_items = plugins.size();
            base.items = new Nanoem__Application__Plugin *[base.n_items];
            names.resize(base.n_items);
            descriptions.resize(base.n_items);
            nanoem_rsize_t i = 0;
            for (DefaultFileManager::MotionIOPluginList::const_iterator it = plugins.begin(), end = plugins.end();
                 it != end; ++it) {
                PluginFactory::MotionIOPluginProxy proxy(*it);
                proxy.setLanguage(language);
                const nanoem_rsize_t offset = functions.size();
                Nanoem__Application__Plugin *plugin = base.items[i] = nanoem_new(Nanoem__Application__Plugin);
                functions.resize(offset + proxy.countAllFunctions());
                internal::ApplicationUtils::fillPluginMessage(
                    proxy, plugin, names[i], descriptions[i], functions, offset);
                i++;
            }
        }
        Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
        event.type_case = NANOEM__APPLICATION__EVENT__TYPE_COMPLETE_LOADING_ALL_MOTION_IO_PLUGINS;
        event.complete_loading_all_motion_io_plugins = &base;
        sendEventMessage(&event);
        for (nanoem_rsize_t i = 0, numItems = base.n_items; i < numItems; i++) {
            nanoem_delete(base.items[i]);
        }
        delete[] base.items;
    }
}

IPrimitive2D *
BaseApplicationService::primitiveContext() NANOEM_DECL_NOEXCEPT
{
    nanoem_assert(m_window, "must NOT be nullptr");
    return m_window->primitiveContext();
}

const IModalDialog *
BaseApplicationService::currentModalDialog() const NANOEM_DECL_NOEXCEPT
{
    return m_window->currentModalDialog();
}

IModalDialog *
BaseApplicationService::currentModalDialog() NANOEM_DECL_NOEXCEPT
{
    return m_window->currentModalDialog();
}

void
BaseApplicationService::addModalDialog(IModalDialog *value)
{
    m_window->addModalDialog(value);
}

void
BaseApplicationService::clearAllModalDialog()
{
    m_window->clearAllModalDialogs();
}

bool
BaseApplicationService::hasModalDialog() const NANOEM_DECL_NOEXCEPT
{
    return m_window->hasModalDialog();
}

sg_pixel_format
BaseApplicationService::defaultPassPixelFormat() const NANOEM_DECL_NOEXCEPT
{
    return SG_PIXELFORMAT_RGBA8;
}

void
BaseApplicationService::beginDefaultPass(
    nanoem_u32_t /* windowID */, const sg_pass_action &pa, int width, int height, int &sampleCount)
{
    sg::begin_default_pass(&pa, width, height);
    sampleCount = 1;
}

void
BaseApplicationService::endDefaultPass()
{
    sg::end_pass();
}

BaseApplicationClient *
BaseApplicationService::menubarApplicationClient()
{
    return nullptr;
}

IVideoRecorder *
BaseApplicationService::createVideoRecorder()
{
    return nullptr;
}

void
BaseApplicationService::destroyVideoRecorder(IVideoRecorder *recorder)
{
    nanoem_delete(recorder);
}

bool
BaseApplicationService::hasVideoRecorder() const NANOEM_DECL_NOEXCEPT
{
    return false;
}

bool
BaseApplicationService::isRendererAvailable(const char *value) const NANOEM_DECL_NOEXCEPT
{
    return StringUtils::equals(value, kRendererOpenGL);
}

const IProjectHolder *
BaseApplicationService::projectHolder() const NANOEM_DECL_NOEXCEPT
{
    return m_stateController;
}

IProjectHolder *
BaseApplicationService::projectHolder() NANOEM_DECL_NOEXCEPT
{
    return m_stateController;
}

DefaultFileManager *
BaseApplicationService::defaultFileManager() NANOEM_DECL_NOEXCEPT
{
    return m_defaultFileManager;
}

const ITranslator *
BaseApplicationService::translator() const NANOEM_DECL_NOEXCEPT
{
    nanoem_assert(m_translatorPtr, "must not be nullptr");
    return m_translatorPtr;
}

ITranslator *
BaseApplicationService::translator() NANOEM_DECL_NOEXCEPT
{
    nanoem_assert(m_translatorPtr, "must not be nullptr");
    return m_translatorPtr;
}

void
BaseApplicationService::setTranslator(ITranslator *value)
{
    nanoem_assert(value, "must not be nullptr");
    m_translatorPtr = value;
}

IEventPublisher *
BaseApplicationService::eventPublisher() NANOEM_DECL_NOEXCEPT
{
    return m_eventPublisherPtr;
}

void
BaseApplicationService::setEventPublisher(IEventPublisher *value)
{
    nanoem_assert(value, "must not be nullptr");
    m_eventPublisherPtr = value;
}

const IFileManager *
BaseApplicationService::fileManager() const NANOEM_DECL_NOEXCEPT
{
    nanoem_assert(m_fileManagerPtr, "must not be nullptr");
    return m_fileManagerPtr;
}

IFileManager *
BaseApplicationService::fileManager() NANOEM_DECL_NOEXCEPT
{
    nanoem_assert(m_fileManagerPtr, "must not be nullptr");
    return m_fileManagerPtr;
}

void
BaseApplicationService::setFileManager(IFileManager *value)
{
    nanoem_parameter_assert(value, "must not be nullptr");
    m_fileManagerPtr = value;
}

const JSON_Value *
BaseApplicationService::applicationConfiguration() const NANOEM_DECL_NOEXCEPT
{
    return m_applicationConfiguration;
}

JSON_Value *
BaseApplicationService::applicationPendingChangeConfiguration() NANOEM_DECL_NOEXCEPT
{
    return m_applicationPendingChangeConfiguration;
}

nanoem_u32_t
BaseApplicationService::defaultAuxFlags() const NANOEM_DECL_NOEXCEPT
{
    return m_defaultAuxFlags;
}

void
BaseApplicationService::setDefaultAuxFlags(nanoem_u32_t value)
{
    m_defaultAuxFlags = value;
}

IAudioPlayer *
BaseApplicationService::createAudioPlayer()
{
    return nanoem_new(TimeBasedAudioPlayer);
}

ICancelPublisher *
BaseApplicationService::createCancelPublisher()
{
    return nullptr;
}

IDebugCapture *
BaseApplicationService::createDebugCapture()
{
    return nullptr;
}

IBackgroundVideoRenderer *
BaseApplicationService::createBackgroundVideoRenderer()
{
    return nanoem_new(StubBackgroundVideoRenderer);
}

Project::IRendererCapability *
BaseApplicationService::createRendererCapability()
{
    return nanoem_new(StubRendererCapability);
}

Project::ISkinDeformerFactory *
BaseApplicationService::createSkinDeformerFactory()
{
    return nullptr;
}

bool
BaseApplicationService::loadFromFile(const URI &fileURI, Project *project, IFileManager::DialogType type, Error &error)
{
    /* intercept pass to swap new loaded project */
    bool result = false;
    if (type == IFileManager::kDialogTypeOpenProject) {
        Project *newProject = m_stateController->createProject();
        if (m_defaultFileManager->loadProject(fileURI, newProject, error)) {
            int language = project->castLanguage();
            setProject(newProject);
            sendLoadingAllModelIOPluginsEventMessage(language);
            sendLoadingAllMotionIOPluginsEventMessage(language);
            result = true;
        }
        else {
            destroyProject(newProject);
        }
    }
    else if (nanoem_likely(project)) {
        result = fileManager()->loadFromFile(fileURI, type, project, error);
    }
    return result;
}

bool
BaseApplicationService::saveAsFile(const URI &fileURI, Project *project, IFileManager::DialogType type, Error &error)
{
    bool result = false;
    if (nanoem_likely(project)) {
        result = fileManager()->saveAsFile(fileURI, type, project, error);
    }
    return result;
}

bool
BaseApplicationService::handleCommandMessage(Nanoem__Application__Command *command, Project *project, Error &error)
{
    undo_command_t *undoCommandPtr = nullptr;
    bool succeeded = true;
    switch (command->type_case) {
    case NANOEM__APPLICATION__COMMAND__TYPE_ACTIVATE: {
        if (nanoem_likely(project)) {
            handleResumeProject();
            project->setActive(true);
            setUIVisible(true);
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_DEACTIVATE: {
        if (nanoem_likely(project)) {
            handleSuspendProject();
            project->setActive(false);
            setUIVisible(false);
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_DESTROY: {
        if (IDebugCapture *debugCapture = m_sharedDebugCaptureRepository.debugCapture()) {
            debugCapture->start(nullptr);
        }
        beginDrawContext();
        handleDestructApplication();
        setProject(nullptr);
        m_stateController->consumeDefaultPass();
        m_defaultFileManager->destroyAllPlugins();
        endDrawContext();
        if (IDebugCapture *debugCapture = m_sharedDebugCaptureRepository.debugCapture()) {
            debugCapture->stop();
        }
        Nanoem__Application__CompleteDestructionEvent base = NANOEM__APPLICATION__COMPLETE_DESTRUCTION_EVENT__INIT;
        Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
        event.type_case = NANOEM__APPLICATION__EVENT__TYPE_COMPLETE_DESTRUCTION;
        event.complete_destruction = &base;
        sendEventMessage(&event);
        postEmptyApplicationEvent();
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_INITIALIZE: {
        const Nanoem__Application__InitializeCommand *commandPtr = command->initialize;
        sg_desc desc;
        Inline::clearZeroMemory(desc);
        m_dllHandle = sg::openSharedLibrary(commandPtr->sokol_dll_path);
        typedef void *(*PFN_sgx_malloc_t)(void *, size_t, const char *, int);
        typedef void (*PFN_sgx_free_t)(void *, void *, const char *, int);
        typedef void (*PFN_sgx_logger_t)(void *, const char *, const char *, int);
        typedef void(APIENTRY * PFN_sgx_install_allocator_hooks)(
            PFN_sgx_malloc_t, PFN_sgx_free_t, PFN_sgx_logger_t, void *);
        if (PFN_sgx_install_allocator_hooks sgx_install_allocator_hooks =
                reinterpret_cast<PFN_sgx_install_allocator_hooks>(
                    bx::dlsym(m_dllHandle, "sgx_install_allocator_hooks"))) {
            sgx_install_allocator_hooks(allocateSGXMemory, releaseSGXMemory, handleSGXMessage, this);
        }
        desc.buffer_pool_size = glm::clamp(commandPtr->buffer_pool_size, 1024u, 0xffffu);
        desc.image_pool_size = glm::clamp(commandPtr->image_pool_size, 4096u, 0xffffu);
        desc.shader_pool_size = glm::clamp(commandPtr->shader_pool_size, 1024u, 0xffffu);
        desc.pipeline_pool_size = glm::clamp(commandPtr->pipeline_pool_size, 1024u, 0xffffu);
        desc.pass_pool_size = glm::clamp(commandPtr->pass_pool_size, 512u, 0xffffu);
        desc.uniform_buffer_size = desc.staging_buffer_size =
            glm::clamp(commandPtr->mtl_global_uniform_buffer_size, 0x10000u, 0x7fffffu);
        beginDrawContext();
        handleSetupGraphicsEngine(desc);
        sg::setup(&desc);
        const sg_backend backend = sg::query_backend();
        if (backend == SG_BACKEND_GLCORE33) {
            m_context = sg::setup_context();
        }
        const nanoem_f32_t windowDevicePixelRatio = glm::max(commandPtr->window_device_pixel_ratio, 1.0f);
        const nanoem_f32_t viewportDevicePixelRatio = glm::max(commandPtr->viewport_viewport_pixel_ratio, 1.0f);
        initialize(windowDevicePixelRatio, viewportDevicePixelRatio);
        const Vector2UI16 viewportSize(commandPtr->window_width, commandPtr->window_height);
        createDefaultRenderTarget(Vector2(viewportSize) * windowDevicePixelRatio);
        sg_pixel_format pixelFormat = SG_PIXELFORMAT_RGBA8;
        if (commandPtr->has_pixel_format) {
            pixelFormat = static_cast<sg_pixel_format>(commandPtr->pixel_format);
        }
        nanoem_u32_t fps = commandPtr->preferred_editing_fps;
        m_stateController->newProject(
            viewportSize, "", pixelFormat, windowDevicePixelRatio, viewportDevicePixelRatio, fps);
        handleInitializeApplication();
        endDrawContext();
        m_initialized = true;
        Nanoem__Application__InitializationCompleteEvent base =
            NANOEM__APPLICATION__INITIALIZATION_COMPLETE_EVENT__INIT;
        Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
        event.type_case = NANOEM__APPLICATION__EVENT__TYPE_INITIALIZATION_COMPLETE;
        event.initialization_complete = &base;
        sendEventMessage(&event);
        postEmptyApplicationEvent();
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_TERMINATE: {
        if (m_dllHandle) {
            beginDrawContext();
            destroy();
            sg::shutdown();
            sg::closeSharedLibrary(m_dllHandle);
            endDrawContext();
        }
        m_initialized = false;
        Nanoem__Application__CompleteTerminationEvent base = NANOEM__APPLICATION__COMPLETE_TERMINATION_EVENT__INIT;
        Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
        event.type_case = NANOEM__APPLICATION__EVENT__TYPE_COMPLETE_TERMINATION;
        event.complete_termination = &base;
        sendEventMessage(&event);
        handleTerminateApplication();
        postEmptyApplicationEvent();
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_VIEWPORT_RESIZED: {
        if (nanoem_likely(project)) {
            const nanoem_f32_t devicePixelRatio = project->isResetAllPassesPending()
                ? project->pendingWindowDevicePixelRatio()
                : project->windowDevicePixelRatio();
            const Nanoem__Application__ViewportResizedCommand *commandPtr = command->viewport_resized;
            const Vector2UI16 logicalPixelWindowSize(commandPtr->width, commandPtr->height),
                devicePixelWindowSize(Vector2(logicalPixelWindowSize) * devicePixelRatio);
            resizeDefaultRenderTarget(devicePixelWindowSize, project);
            project->resizeWindowSize(logicalPixelWindowSize);
            m_window->resizeDevicePixelWindowSize(devicePixelWindowSize);
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_WINDOW_RESIZED: {
        if (nanoem_likely(project)) {
            const nanoem_f32_t devicePixelRatio = project->isResetAllPassesPending()
                ? project->pendingWindowDevicePixelRatio()
                : project->windowDevicePixelRatio();
            const Nanoem__Application__WindowResizedCommand *commandPtr = command->window_resized;
            const Vector2UI16 logicalPixelWindowSize(commandPtr->width, commandPtr->height),
                devicePixelWindowSize(Vector2(logicalPixelWindowSize) * devicePixelRatio);
            resizeDefaultRenderTarget(devicePixelWindowSize, project);
            project->resizeWindowSize(logicalPixelWindowSize);
            m_window->resizeDevicePixelWindowSize(devicePixelWindowSize);
            if (g_sentryAvailable) {
                sentry_value_t breadcrumb = sentry_value_new_breadcrumb(nullptr, nullptr);
                sentry_value_t data = sentry_value_new_object();
                sentry_value_set_by_key(data, "width", sentry_value_new_int32(logicalPixelWindowSize.x));
                sentry_value_set_by_key(data, "height", sentry_value_new_int32(logicalPixelWindowSize.y));
                sentry_value_set_by_key(breadcrumb, "category", sentry_value_new_string("resized"));
                sentry_value_set_by_key(breadcrumb, "data", data);
                sentry_add_breadcrumb(breadcrumb);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_LOAD_FILE: {
        const Nanoem__Application__LoadFileCommand *commandPtr = command->load_file;
        const Nanoem__Application__URI *uri = commandPtr->file_uri;
        const URI &fileURI = URI::createFromFilePath(uri->absolute_path, uri->fragment);
        IFileManager::DialogType type = static_cast<IFileManager::DialogType>(commandPtr->type);
        if (g_sentryAvailable) {
            sentry_value_t breadcrumb = sentry_value_new_breadcrumb(nullptr, nullptr);
            sentry_value_t data = sentry_value_new_object();
            sentry_value_set_by_key(data, "path", g_sentryMaskStringProc(fileURI.absolutePathConstString()));
            sentry_value_set_by_key(breadcrumb, "category", sentry_value_new_string("loading"));
            sentry_value_set_by_key(breadcrumb, "data", data);
            sentry_add_breadcrumb(breadcrumb);
        }
        if (IDebugCapture *debugCapture = m_sharedDebugCaptureRepository.debugCapture()) {
            debugCapture->start(nullptr);
        }
        beginDrawContext();
        const nanoem_u64_t startedAt = stm_now();
        succeeded = loadFromFile(fileURI, project, type, error);
        const nanoem_u64_t interval = stm_since(startedAt);
        endDrawContext();
        if (IDebugCapture *debugCapture = m_sharedDebugCaptureRepository.debugCapture()) {
            debugCapture->stop();
        }
        Nanoem__Application__CompleteLoadingFileEvent base = NANOEM__APPLICATION__COMPLETE_LOADING_FILE_EVENT__INIT;
        Nanoem__Application__URI uri2;
        MutableString absolutePath, fragment;
        base.file_uri = internal::ApplicationUtils::assignURI(&uri2, absolutePath, fragment, fileURI);
        base.type = commandPtr->type;
        base.ticks = interval;
        base.has_ticks = 1;
        Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
        event.type_case = NANOEM__APPLICATION__EVENT__TYPE_COMPLETE_LOADING_FILE;
        event.complete_loading_file = &base;
        sendEventMessage(&event);
        if (g_sentryAvailable) {
            if (succeeded) {
                sentry_value_t breadcrumb = sentry_value_new_breadcrumb(nullptr, nullptr);
                sentry_value_t data = sentry_value_new_object();
                sentry_value_set_by_key(data, "path", g_sentryMaskStringProc(fileURI.absolutePathConstString()));
                sentry_value_set_by_key(data, "seconds", sentry_value_new_double(stm_sec(interval)));
                sentry_value_set_by_key(breadcrumb, "category", sentry_value_new_string("loaded"));
                sentry_value_set_by_key(breadcrumb, "data", data);
                sentry_add_breadcrumb(breadcrumb);
            }
            else if (error.isCancelled()) {
                sentry_value_t breadcrumb = sentry_value_new_breadcrumb(nullptr, nullptr);
                sentry_value_t data = sentry_value_new_object();
                sentry_value_set_by_key(data, "path", g_sentryMaskStringProc(fileURI.absolutePathConstString()));
                sentry_value_set_by_key(data, "seconds", sentry_value_new_double(stm_sec(interval)));
                sentry_value_set_by_key(breadcrumb, "category", sentry_value_new_string("cancelled"));
                sentry_value_set_by_key(breadcrumb, "data", data);
                sentry_add_breadcrumb(breadcrumb);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SAVE_FILE: {
        const Nanoem__Application__LoadFileCommand *commandPtr = command->load_file;
        const Nanoem__Application__URI *uri = commandPtr->file_uri;
        const URI &fileURI = URI::createFromFilePath(uri->absolute_path, uri->fragment);
        IFileManager::DialogType type = static_cast<IFileManager::DialogType>(commandPtr->type);
        if (g_sentryAvailable) {
            sentry_value_t breadcrumb = sentry_value_new_breadcrumb(nullptr, nullptr);
            sentry_value_t data = sentry_value_new_object();
            sentry_value_set_by_key(data, "path", g_sentryMaskStringProc(fileURI.absolutePathConstString()));
            sentry_value_set_by_key(breadcrumb, "category", sentry_value_new_string("saving"));
            sentry_value_set_by_key(breadcrumb, "data", data);
            sentry_add_breadcrumb(breadcrumb);
        }
        const nanoem_u64_t startedAt = stm_now();
        succeeded = saveAsFile(fileURI, project, type, error);
        const nanoem_u64_t interval = stm_since(startedAt);
        sendSaveFileMessage(fileURI, commandPtr->type, interval, succeeded);
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_GET_ALL_MODEL_BONES: {
        if (nanoem_likely(project)) {
            Nanoem__Application__GetAllModelBonesResponseEvent base =
                NANOEM__APPLICATION__GET_ALL_MODEL_BONES_RESPONSE_EVENT__INIT;
            MutableStringList boneNameList;
            if (const Model *model = project->findModelByHandle(command->get_all_model_bones->model_handle)) {
                nanoem_rsize_t numBones;
                nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(model->data(), &numBones);
                base.n_bones = numBones;
                base.bones = new char *[numBones];
                base.model_handle = model->handle();
                boneNameList.resize(numBones);
                for (nanoem_rsize_t i = 0; i < numBones; i++) {
                    const model::Bone *bone = model::Bone::cast(bones[i]);
                    base.bones[i] = StringUtils::cloneString(bone->nameConstString(), boneNameList[i]);
                }
            }
            Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
            event.type_case = NANOEM__APPLICATION__EVENT__TYPE_GET_ALL_MODEL_BONES;
            event.get_all_model_bones = &base;
            sendQueryEventMessage(&event, command);
            delete[] base.bones;
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_GET_ALL_MODEL_MORPHS: {
        if (nanoem_likely(project)) {
            Nanoem__Application__GetAllModelMorphsResponseEvent base =
                NANOEM__APPLICATION__GET_ALL_MODEL_MORPHS_RESPONSE_EVENT__INIT;
            MutableStringList eyeMorphList, eyebrowMorphList, lipMorphList, otherMorphList;
            if (const Model *model = project->findModelByHandle(command->get_all_model_morphs->model_handle)) {
                nanoem_rsize_t numMorphs;
                nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(model->data(), &numMorphs);
                MorphMap morphMap;
                for (nanoem_rsize_t i = 0; i < numMorphs; i++) {
                    const nanoem_model_morph_t *morph = morphs[i];
                    morphMap[nanoemModelMorphGetCategory(morph)].push_back(morph);
                }
                internal::ApplicationUtils::fillAllMorphItems(
                    morphMap, NANOEM_MODEL_MORPH_CATEGORY_EYE, base.eye_morphs, base.n_eye_morphs, eyeMorphList);
                internal::ApplicationUtils::fillAllMorphItems(morphMap, NANOEM_MODEL_MORPH_CATEGORY_EYEBROW,
                    base.eyebrow_morphs, base.n_eyebrow_morphs, eyebrowMorphList);
                internal::ApplicationUtils::fillAllMorphItems(
                    morphMap, NANOEM_MODEL_MORPH_CATEGORY_LIP, base.lip_morphs, base.n_lip_morphs, lipMorphList);
                internal::ApplicationUtils::fillAllMorphItems(morphMap, NANOEM_MODEL_MORPH_CATEGORY_OTHER,
                    base.other_morphs, base.n_other_morphs, otherMorphList);
            }
            Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
            event.type_case = NANOEM__APPLICATION__EVENT__TYPE_GET_ALL_MODEL_MORPHS;
            event.get_all_model_morphs = &base;
            sendQueryEventMessage(&event, command);
            delete[] base.eye_morphs;
            delete[] base.eyebrow_morphs;
            delete[] base.lip_morphs;
            delete[] base.other_morphs;
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_PING_PONG: {
        Nanoem__Application__PingPongEvent base = NANOEM__APPLICATION__PING_PONG_EVENT__INIT;
        Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
        event.type_case = NANOEM__APPLICATION__EVENT__TYPE_PING_PONG;
        event.ping_pong = &base;
        sendQueryEventMessage(&event, command);
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_IS_PROJECT_DIRTY: {
        if (nanoem_likely(project)) {
            Nanoem__Application__IsProjectDirtyResponseEvent base =
                NANOEM__APPLICATION__IS_PROJECT_DIRTY_RESPONSE_EVENT__INIT;
            base.dirty = project->isDirty();
            Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
            event.type_case = NANOEM__APPLICATION__EVENT__TYPE_IS_PROJECT_DIRTY;
            event.is_project_dirty = &base;
            sendQueryEventMessage(&event, command);
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_NEW_PROJECT: {
        handleNewProject();
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SAVE_PROJECT: {
        if (nanoem_likely(project)) {
            const URI fileURI(project->fileURI());
            if (fileURI.isEmpty()) {
                m_window->openSaveProjectDialog(project);
            }
            else {
                const nanoem_u64_t startedAt = stm_now();
                succeeded = saveAsFile(fileURI, project, IFileManager::kDialogTypeSaveProjectFile, error);
                const nanoem_u64_t interval = stm_since(startedAt);
                sendSaveFileMessage(fileURI, IFileManager::kDialogTypeSaveProjectFile, interval, succeeded);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_GET_PROJECT_FILE_URI: {
        if (nanoem_likely(project)) {
            Nanoem__Application__GetProjectFileURIResponseEvent base =
                NANOEM__APPLICATION__GET_PROJECT_FILE_URIRESPONSE_EVENT__INIT;
            Nanoem__Application__URI uri;
            MutableString absolutePath, fragment;
            base.file_uri = internal::ApplicationUtils::assignURI(&uri, absolutePath, fragment, project->fileURI());
            Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
            event.type_case = NANOEM__APPLICATION__EVENT__TYPE_GET_PROJECT_FILE_URI;
            event.get_project_file_uri = &base;
            sendQueryEventMessage(&event, command);
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_CHANGE_DEVICE_PIXEL_RATIO: {
        nanoem_f32_t devicePixelRatio = command->change_device_pixel_ratio->value;
        if (nanoem_likely(project)) {
            const Vector2 devicePixelWindowSize(Vector2(project->windowSize()) * devicePixelRatio);
            resizeDefaultRenderTarget(devicePixelWindowSize, project);
            project->setWindowDevicePixelRatio(devicePixelRatio);
            project->setViewportDevicePixelRatio(devicePixelRatio);
            m_window->setFontPointSize(internal::ImGuiWindow::kFontSize * devicePixelRatio);
            m_window->setDevicePixelRatio(devicePixelRatio);
            m_window->resizeDevicePixelWindowSize(devicePixelWindowSize);
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_CONFIRM_BEFORE_NEW_PROJECT: {
        const ITranslator *tr = translator();
        const char *title = tr->translate("nanoem.window.confirm-before-new-project.title");
        const char *message = tr->translate("nanoem.window.confirm-before-new-project.message");
        ModalDialogFactory::SaveConfirmDialog callback(handleSaveOnNewProject, handleDiscardOnNewProject);
        addModalDialog(ModalDialogFactory::createConfirmSavingDialog(this, title, message, callback, this));
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_CONFIRM_BEFORE_OPEN_PROJECT: {
        const ITranslator *tr = translator();
        const char *title = tr->translate("nanoem.window.confirm-before-open-project.title");
        const char *message = tr->translate("nanoem.window.confirm-before-open-project.message");
        ModalDialogFactory::SaveConfirmDialog callback(handleSaveOnOpenProject, handleDiscardOnOpenProject);
        addModalDialog(ModalDialogFactory::createConfirmSavingDialog(this, title, message, callback, this));
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_CONFIRM_BEFORE_EXIT_APPLICATION: {
        const ITranslator *tr = translator();
        const char *title = tr->translate("nanoem.window.confirm-before-exit-application.title");
        const char *message = tr->translate("nanoem.window.confirm-before-exit-application.message");
        ModalDialogFactory::SaveConfirmDialog callback(handleSaveOnExitApplication, handleDiscardOnExitApplication);
        addModalDialog(ModalDialogFactory::createConfirmSavingDialog(this, title, message, callback, this));
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_LOAD_ALL_DECODER_PLUGINS: {
        const Nanoem__Application__LoadAllDecoderPluginsCommand *commandPtr = command->load_all_decoder_plugins;
        URIList fileURIs;
        for (size_t i = 0, numPlugins = commandPtr->n_file_uris; i < numPlugins; i++) {
            const Nanoem__Application__URI *uri = commandPtr->file_uris[i];
            fileURIs.push_back(URI::createFromFilePath(uri->absolute_path, uri->fragment));
        }
        {
            StringSet audioExtensions;
            m_defaultFileManager->initializeAllAudioDecoderPlugins(fileURIs, audioExtensions);
            Nanoem__Application__AvailableAllImportingAudioExtensionsEvent base =
                NANOEM__APPLICATION__AVAILABLE_ALL_IMPORTING_AUDIO_EXTENSIONS_EVENT__INIT;
            base.n_extensions = audioExtensions.size();
            base.extensions = new char *[base.n_extensions];
            size_t index = 0;
            MutableStringList audioExtensionStringList;
            audioExtensionStringList.resize(base.n_extensions);
            for (StringSet::const_iterator it = audioExtensions.begin(), end = audioExtensions.end(); it != end;
                 ++it, ++index) {
                base.extensions[index] = StringUtils::cloneString(it->c_str(), audioExtensionStringList[index]);
            }
            Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
            event.type_case = NANOEM__APPLICATION__EVENT__TYPE_AVAILABLE_ALL_IMPORTING_AUDIO_EXTENSIONS;
            event.available_all_importing_audio_extensions = &base;
            sendQueryEventMessage(&event, command);
            delete[] base.extensions;
        }
        {
            StringSet videoExtensions;
            m_defaultFileManager->initializeAllVideoDecoderPlugins(fileURIs, videoExtensions);
            Nanoem__Application__AvailableAllImportingVideoExtensionsEvent base =
                NANOEM__APPLICATION__AVAILABLE_ALL_IMPORTING_VIDEO_EXTENSIONS_EVENT__INIT;
            base.n_extensions = videoExtensions.size();
            base.extensions = new char *[base.n_extensions];
            size_t index = 0;
            MutableStringList videoExtensionStringList;
            videoExtensionStringList.resize(videoExtensions.size());
            for (StringSet::const_iterator it = videoExtensions.begin(), end = videoExtensions.end(); it != end;
                 ++it, ++index) {
                base.extensions[index] = StringUtils::cloneString(it->c_str(), videoExtensionStringList[index]);
            }
            Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
            event.type_case = NANOEM__APPLICATION__EVENT__TYPE_AVAILABLE_ALL_IMPORTING_VIDEO_EXTENSIONS;
            event.available_all_importing_video_extensions = &base;
            sendQueryEventMessage(&event, command);
            delete[] base.extensions;
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_LOAD_ALL_ENCODER_PLUGINS: {
        const Nanoem__Application__LoadAllEncoderPluginsCommand *commandPtr = command->load_all_encoder_plugins;
        URIList fileURIs;
        for (size_t i = 0, numPlugins = commandPtr->n_file_uris; i < numPlugins; i++) {
            const Nanoem__Application__URI *uri = commandPtr->file_uris[i];
            fileURIs.push_back(URI::createFromFilePath(uri->absolute_path, uri->fragment));
        }
        StringSet videoExtensions;
        m_defaultFileManager->initializeAllEncoderPlugins(fileURIs, videoExtensions);
        Nanoem__Application__AvailableAllExportingVideoExtensionsEvent base =
            NANOEM__APPLICATION__AVAILABLE_ALL_EXPORTING_VIDEO_EXTENSIONS_EVENT__INIT;
        base.n_extensions = videoExtensions.size();
        base.extensions = new char *[base.n_extensions];
        size_t index = 0;
        MutableStringList videoExtensionStringList;
        videoExtensionStringList.resize(videoExtensions.size());
        for (StringSet::const_iterator it = videoExtensions.begin(), end = videoExtensions.end(); it != end;
             ++it, ++index) {
            base.extensions[index] = StringUtils::cloneString(it->c_str(), videoExtensionStringList[index]);
        }
        Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
        event.type_case = NANOEM__APPLICATION__EVENT__TYPE_AVAILABLE_ALL_EXPORTING_VIDEO_EXTENSIONS;
        event.available_all_exporting_video_extensions = &base;
        sendQueryEventMessage(&event, command);
        delete[] base.extensions;
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_LOAD_ALL_MODEL_IO_PLUGINS: {
        const Nanoem__Application__LoadAllModelIOPluginsCommand *commandPtr = command->load_all_model_io_plugins;
        URIList fileURIs;
        for (size_t i = 0, numPlugins = commandPtr->n_file_uris; i < numPlugins; i++) {
            const Nanoem__Application__URI *uri = commandPtr->file_uris[i];
            fileURIs.push_back(URI::createFromFilePath(uri->absolute_path, uri->fragment));
        }
        m_defaultFileManager->initializeAllModelIOPlugins(fileURIs);
        sendLoadingAllModelIOPluginsEventMessage(nanoem_likely(project) ? project->castLanguage() : 0);
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_LOAD_ALL_MOTION_IO_PLUGINS: {
        const Nanoem__Application__LoadAllMotionIOPluginsCommand *commandPtr = command->load_all_motion_io_plugins;
        URIList fileURIs;
        for (size_t i = 0, numPlugins = commandPtr->n_file_uris; i < numPlugins; i++) {
            const Nanoem__Application__URI *uri = commandPtr->file_uris[i];
            fileURIs.push_back(URI::createFromFilePath(uri->absolute_path, uri->fragment));
        }
        m_defaultFileManager->initializeAllMotionIOPlugins(fileURIs);
        sendLoadingAllMotionIOPluginsEventMessage(nanoem_likely(project) ? project->castLanguage() : 0);
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REQUEST_EXPORT_IMAGE_CONFIGURATION: {
        if (nanoem_likely(project)) {
            const Project::ModelList models(project->allModels());
            bool dirty = false;
            for (Project::ModelList::const_iterator it = models.begin(), end = models.end(); it != end; ++it) {
                const Model *model = *it;
                dirty |= model->hasAnyDirtyBone() || model->hasAnyDirtyMorph();
            }
            if (!dirty) {
                internal::CapturingPassAsImageState *state =
                    nanoem_new(internal::CapturingPassAsImageState(m_stateController, project));
                nanoem_delete(m_capturingPassState);
                m_capturingPassState = state;
                state->addExportImageDialog(project);
            }
            else {
                const ITranslator *tr = translator();
                const char *reason = tr->translate("nanoem.error.model.dirty.reason"),
                           *recoverySuggestion = tr->translate("nanoem.error.model.dirty.recovery-suggestion");
                error = Error(reason, recoverySuggestion, Error::kDomainTypeApplication);
                succeeded = false;
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REQUEST_EXPORT_VIDEO_CONFIGURATION: {
        if (Project *project = m_stateController->currentProject()) {
            internal::CapturingPassAsVideoState *state =
                nanoem_new(internal::CapturingPassAsVideoState(m_stateController, project));
            nanoem_delete(m_capturingPassState);
            m_capturingPassState = state;
            state->addExportVideoDialog(project);
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_EXECUTE_EXPORTING_IMAGE: {
        const Nanoem__Application__ExecuteExportingImageCommand *commandPtr = command->execute_exporting_image;
        const Nanoem__Application__URI *uri = commandPtr->file_uri;
        Project *project = m_stateController->currentProject();
        startCapture(project, URI::createFromFilePath(uri->absolute_path, uri->fragment), error);
        if (g_sentryAvailable && !error.hasReason()) {
            sentry_value_t breadcrumb = sentry_value_new_breadcrumb(nullptr, nullptr);
            sentry_value_t data = sentry_value_new_object();
            sentry_value_set_by_key(data, "path", g_sentryMaskStringProc(uri->absolute_path));
            sentry_value_set_by_key(data, "type", sentry_value_new_string("image"));
            sentry_value_set_by_key(breadcrumb, "category", sentry_value_new_string("capture.start"));
            sentry_value_set_by_key(breadcrumb, "data", data);
            sentry_add_breadcrumb(breadcrumb);
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_EXECUTE_EXPORTING_VIDEO: {
        const Nanoem__Application__ExecuteExportingVideoCommand *commandPtr = command->execute_exporting_video;
        const Nanoem__Application__URI *uri = commandPtr->file_uri;
        Project *project = m_stateController->currentProject();
        startCapture(project, URI::createFromFilePath(uri->absolute_path, uri->fragment), error);
        if (g_sentryAvailable && !error.hasReason()) {
            sentry_value_t breadcrumb = sentry_value_new_breadcrumb(nullptr, nullptr);
            sentry_value_t data = sentry_value_new_object();
            sentry_value_set_by_key(data, "path", g_sentryMaskStringProc(uri->absolute_path));
            sentry_value_set_by_key(data, "type", sentry_value_new_string("video"));
            sentry_value_set_by_key(breadcrumb, "category", sentry_value_new_string("capture.start"));
            sentry_value_set_by_key(breadcrumb, "data", data);
            sentry_add_breadcrumb(breadcrumb);
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_START_DEBUG_CAPTURE: {
        if (IDebugCapture *debugCapture = m_sharedDebugCaptureRepository.debugCapture()) {
            debugCapture->start(nullptr);
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_STOP_DEBUG_CAPTURE: {
        if (IDebugCapture *debugCapture = m_sharedDebugCaptureRepository.debugCapture()) {
            debugCapture->stop();
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_GET_BACKGROUND_IMAGE_TEXTURE_HANDLE: {
        if (nanoem_likely(project)) {
            Nanoem__Application__GetBackgroundImageTextureHandleResponseEvent base =
                NANOEM__APPLICATION__GET_BACKGROUND_IMAGE_TEXTURE_HANDLE_RESPONSE_EVENT__INIT;
            base.handle = project->backgroundImageHandle().id;
            Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
            event.type_case = NANOEM__APPLICATION__EVENT__TYPE_GET_BACKGROUND_IMAGE_TEXTURE_HANDLE;
            event.get_background_image_texture_handle = &base;
            sendQueryEventMessage(&event, command);
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SAVE_POINT: {
        const Nanoem__Application__SavePointCommand *c = command->save_point;
        const Nanoem__Application__URI *uri = c->file_uri;
        const URI &fileURI = URI::createFromFilePath(uri->absolute_path, uri->fragment);
        succeeded = loadFromFile(fileURI, project, IFileManager::kDialogTypeOpenProject, error);
        if (succeeded) {
            for (size_t i = 0; i < c->n_accessories; i++) {
                const Nanoem__Application__RedoLoadAccessoryCommand *handle = c->accessories[i];
                if (Accessory *accessory = project->findAccessoryByName(handle->name)) {
                    project->setRedoDrawable(handle->accessory_handle, accessory);
                }
            }
            Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
            for (size_t i = 0; i < c->n_models; i++) {
                Nanoem__Application__RedoLoadModelCommand *handle = c->models[i];
                if (Model *model = project->findModelByName(handle->name)) {
                    command.redo_load_model = handle;
                    model->readLoadCommandMessage(&command);
                    project->setRedoDrawable(handle->model_handle, model);
                }
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_GET_ALL_ACCESSORIES: {
        if (nanoem_likely(project)) {
            const Project::AccessoryList &accessories = project->allAccessories();
            nanoem_rsize_t numItems = accessories.size();
            Nanoem__Application__GetAllAccessoriesResponseEvent__Item **items =
                new Nanoem__Application__GetAllAccessoriesResponseEvent__Item *[numItems];
            nanoem_rsize_t i = 0;
            MutableStringList names(numItems);
            for (Project::AccessoryList::const_iterator it = accessories.begin(), end = accessories.end(); it != end;
                 ++it) {
                const Accessory *accessory = *it;
                Nanoem__Application__GetAllAccessoriesResponseEvent__Item *item = items[i] =
                    nanoem_new(Nanoem__Application__GetAllAccessoriesResponseEvent__Item);
                nanoem__application__get_all_accessories_response_event__item__init(item);
                StringUtils::cloneString(accessory->nameConstString(), names[i]);
                item->handle = accessory->handle();
                item->name = names[i].data();
                item->is_active = project->activeAccessory() == accessory;
                i++;
            }
            Nanoem__Application__GetAllAccessoriesResponseEvent base =
                NANOEM__APPLICATION__GET_ALL_ACCESSORIES_RESPONSE_EVENT__INIT;
            base.items = items;
            base.n_items = numItems;
            Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
            event.type_case = NANOEM__APPLICATION__EVENT__TYPE_GET_ALL_ACCESSORIES;
            event.get_all_accessories = &base;
            sendQueryEventMessage(&event, command);
            for (i = 0; i < numItems; i++) {
                nanoem_delete(items[i]);
            }
            delete[] items;
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_GET_ALL_MODELS: {
        if (nanoem_likely(project)) {
            const Project::ModelList &accessories = project->allModels();
            nanoem_rsize_t numItems = accessories.size();
            Nanoem__Application__GetAllModelsResponseEvent__Item **items =
                new Nanoem__Application__GetAllModelsResponseEvent__Item *[numItems];
            nanoem_rsize_t i = 0;
            MutableStringList names(numItems);
            for (Project::ModelList::const_iterator it = accessories.begin(), end = accessories.end(); it != end;
                 ++it) {
                const Model *model = *it;
                Nanoem__Application__GetAllModelsResponseEvent__Item *item = items[i] =
                    nanoem_new(Nanoem__Application__GetAllModelsResponseEvent__Item);
                nanoem__application__get_all_models_response_event__item__init(item);
                StringUtils::cloneString(model->nameConstString(), names[i]);
                item->handle = model->handle();
                item->name = names[i].data();
                item->is_active = project->activeModel() == model;
                i++;
            }
            Nanoem__Application__GetAllModelsResponseEvent base =
                NANOEM__APPLICATION__GET_ALL_MODELS_RESPONSE_EVENT__INIT;
            base.items = items;
            base.n_items = numItems;
            Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
            event.type_case = NANOEM__APPLICATION__EVENT__TYPE_GET_ALL_MODELS;
            event.get_all_models = &base;
            sendQueryEventMessage(&event, command);
            for (i = 0; i < numItems; i++) {
                nanoem_delete(items[i]);
            }
            delete[] items;
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_CURSOR_MOVED: {
        const Nanoem__Application__CursorMovedCommand *commandPtr = command->cursor_moved;
        const Vector3SI32 point(commandPtr->x, commandPtr->y, commandPtr->modifiers);
        const Vector2SI32 delta(commandPtr->delta_x, commandPtr->delta_y);
        m_stateController->handlePointerMove(point, delta);
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_CURSOR_PRESSED: {
        const Nanoem__Application__CursorPressedCommand *commandPtr = command->cursor_pressed;
        const Vector3SI32 point(commandPtr->x, commandPtr->y, commandPtr->modifiers);
        m_stateController->handlePointerPress(point, static_cast<Project::CursorType>(commandPtr->type));
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_CURSOR_RELEASED: {
        const Nanoem__Application__CursorReleasedCommand *commandPtr = command->cursor_released;
        const Vector3SI32 point(commandPtr->x, commandPtr->y, commandPtr->modifiers);
        m_stateController->handlePointerRelease(point, static_cast<Project::CursorType>(commandPtr->type));
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_CURSOR_SCROLLED: {
        const Nanoem__Application__CursorScrolledCommand *commandPtr = command->cursor_scrolled;
        const Vector3SI32 point(commandPtr->x, commandPtr->y, commandPtr->modifiers);
        const Vector2SI32 delta(commandPtr->delta_x, commandPtr->delta_y);
        m_stateController->handlePointerScroll(point, delta);
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_POINTER_MOVED: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__PointerMovedCommand *commandPtr = command->pointer_moved;
            const Vector2 ws(project->windowSize());
            const Vector3SI32 point(commandPtr->x * ws.x, commandPtr->y * ws.y, 0);
            m_stateController->handlePointerMove(point, Vector2());
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_POINTER_PRESSED: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__PointerPressedCommand *commandPtr = command->pointer_pressed;
            const Vector2 ws(project->windowSize());
            const Vector3SI32 point(commandPtr->x * ws.x, commandPtr->y * ws.y, 0);
            m_stateController->handlePointerPress(point, static_cast<Project::CursorType>(commandPtr->type));
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_POINTER_RELEASED: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__PointerReleasedCommand *commandPtr = command->pointer_released;
            const Vector2 ws(project->windowSize());
            const Vector3SI32 point(commandPtr->x * ws.x, commandPtr->y * ws.y, 0);
            m_stateController->handlePointerRelease(point, static_cast<Project::CursorType>(commandPtr->type));
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_POINTER_SCROLLED: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__PointerScrolledCommand *commandPtr = command->pointer_scrolled;
            const Vector2 ws(project->windowSize());
            const Vector3SI32 point(commandPtr->x * ws.x, commandPtr->y * ws.y, 0);
            const Vector2SI32 delta(commandPtr->delta_x * ws.x, commandPtr->delta_y * ws.y);
            m_stateController->handlePointerScroll(point, delta);
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_KEY_PRESSED: {
        const KeyType key = static_cast<KeyType>(command->key_pressed->value);
        setKeyPressed(key);
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_KEY_RELEASED: {
        const KeyType key = static_cast<KeyType>(command->key_released->value);
        setKeyReleased(key);
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_MENU_ACTION: {
        dispatchMenuItemAction(project, command->menu_action->value, error);
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_RENDER_FRAME: {
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_UNICODE_INPUT: {
        setUnicodePressed(command->unicode_input->value);
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_ACTIVE_ACCESSORY: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SetActiveAccessoryCommand *commandPtr = command->set_active_accessory;
            project->setActiveAccessory(project->findAccessoryByHandle(commandPtr->accessory_handle));
            writeRedoMessage(command, project, error);
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_ACTIVE_MODEL: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SetActiveModelCommand *commandPtr = command->set_active_model;
            project->setActiveModel(project->findModelByHandle(commandPtr->model_handle));
            writeRedoMessage(command, project, error);
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_ACTIVE_MODEL_BONE: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SetActiveModelBoneCommand *commandPtr = command->set_active_model_bone;
            if (Model *model = project->activeModel()) {
                if (const nanoem_model_bone_t *bone = model->findBone(commandPtr->name)) {
                    model->setActiveBone(bone);
                    writeRedoMessage(command, project, error);
                }
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_ACTIVE_MODEL_MORPH: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SetActiveModelMorphCommand *commandPtr = command->set_active_model_morph;
            if (Model *model = project->activeModel()) {
                if (const nanoem_model_morph_t *morph = model->findMorph(commandPtr->name)) {
                    model->setActiveMorph(morph);
                    if (commandPtr->has_apply_category && commandPtr->apply_category) {
                        model->setActiveMorph(nanoemModelMorphGetCategory(morph), morph);
                        writeRedoMessage(command, project, error);
                    }
                }
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SEEK: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SeekCommand *commandPtr = command->seek;
            const nanoem_frame_index_t value =
                nanoem_frame_index_t(glm::clamp(commandPtr->local_frame_index, uint64_t(0), uint64_t(UINT32_MAX)));
            project->seek(value, true);
        }
        /* do not call writeRedoMessage due to many calls */
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_ACCESSORY_OPACITY: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SetAccessoryOpacityCommand *commandPtr = command->set_accessory_opacity;
            Accessory *accessory = commandPtr->has_accessory_handle
                ? project->findAccessoryByHandle(commandPtr->accessory_handle)
                : project->activeAccessory();
            if (accessory) {
                nanoem_f32_t value = commandPtr->opacity;
                if (commandPtr->has_can_undo && commandPtr->can_undo) {
                    internal::AccessoryOpacityValueState *state =
                        nanoem_new(internal::AccessoryOpacityValueState(accessory));
                    state->setValue(&value);
                    state->commit();
                    nanoem_delete(state);
                }
                else {
                    accessory->setOpacity(value);
                }
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_ACCESSORY_ORIENTATION: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SetAccessoryOrientationCommand *commandPtr = command->set_accessory_orientation;
            Accessory *accessory = commandPtr->has_accessory_handle
                ? project->findAccessoryByHandle(commandPtr->accessory_handle)
                : project->activeAccessory();
            if (accessory) {
                const Vector3 value(commandPtr->x, commandPtr->y, commandPtr->z);
                if (commandPtr->has_can_undo && commandPtr->can_undo) {
                    internal::AccessoryOrientationVectorValueState *state =
                        nanoem_new(internal::AccessoryOrientationVectorValueState(accessory));
                    state->setValue(glm::value_ptr(value));
                    state->commit();
                    nanoem_delete(state);
                }
                else {
                    accessory->setOrientation(value);
                }
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_ACCESSORY_SCALE_FACTOR: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SetAccessoryScaleFactorCommand *commandPtr = command->set_accessory_scale_factor;
            Accessory *accessory = commandPtr->has_accessory_handle
                ? project->findAccessoryByHandle(commandPtr->accessory_handle)
                : project->activeAccessory();
            if (accessory) {
                nanoem_f32_t value = commandPtr->scale_factor;
                if (commandPtr->has_can_undo && commandPtr->can_undo) {
                    internal::AccessoryScaleFactorValueState *state =
                        nanoem_new(internal::AccessoryScaleFactorValueState(accessory));
                    state->setValue(&value);
                    state->commit();
                    nanoem_delete(state);
                }
                else {
                    accessory->setOpacity(value);
                }
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_ACCESSORY_TRANSLATION: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SetAccessoryTranslationCommand *commandPtr = command->set_accessory_translation;
            Accessory *accessory = commandPtr->has_accessory_handle
                ? project->findAccessoryByHandle(commandPtr->accessory_handle)
                : project->activeAccessory();
            if (accessory) {
                const Vector3 value(commandPtr->x, commandPtr->y, commandPtr->z);
                if (commandPtr->has_can_undo && commandPtr->can_undo) {
                    internal::AccessoryTranslationVectorValueState *state =
                        nanoem_new(internal::AccessoryTranslationVectorValueState(accessory));
                    state->setValue(glm::value_ptr(value));
                    state->commit();
                    nanoem_delete(state);
                }
                else {
                    accessory->setOrientation(value);
                }
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_CAMERA_ANGLE: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SetCameraAngleCommand *commandPtr = command->set_camera_angle;
            const Vector3 value(commandPtr->x, commandPtr->y, commandPtr->z);
            ICamera *camera = project->globalCamera();
            if (commandPtr->has_can_undo && commandPtr->can_undo) {
                internal::CameraAngleVectorValueState *state =
                    nanoem_new(internal::CameraAngleVectorValueState(project, camera));
                state->setValue(glm::value_ptr(value));
                state->commit();
                nanoem_delete(state);
            }
            else {
                camera->setAngle(value);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_CAMERA_DISTANCE: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SetCameraDistanceCommand *commandPtr = command->set_camera_distance;
            const nanoem_f32_t value = commandPtr->distance;
            ICamera *camera = project->globalCamera();
            if (commandPtr->has_can_undo && commandPtr->can_undo) {
                internal::CameraDistanceVectorValueState *state =
                    nanoem_new(internal::CameraDistanceVectorValueState(project, camera));
                state->setValue(&value);
                state->commit();
                nanoem_delete(state);
            }
            else {
                camera->setDistance(value);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_CAMERA_FOV: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SetCameraFovCommand *commandPtr = command->set_camera_fov;
            ICamera *camera = project->globalCamera();
            if (commandPtr->has_can_undo && commandPtr->can_undo) {
                const nanoem_f32_t value = glm::radians(nanoem_f32_t(commandPtr->fov));
                internal::CameraFovVectorValueState *state =
                    nanoem_new(internal::CameraFovVectorValueState(project, camera));
                state->setValue(&value);
                state->commit();
                nanoem_delete(state);
            }
            else {
                camera->setFov(commandPtr->fov);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_CAMERA_LOOK_AT: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SetCameraLookAtCommand *commandPtr = command->set_camera_look_at;
            const Vector3 value(commandPtr->x, commandPtr->y, commandPtr->z);
            ICamera *camera = project->globalCamera();
            if (commandPtr->has_can_undo && commandPtr->can_undo) {
                internal::CameraLookAtVectorValueState *state =
                    nanoem_new(internal::CameraLookAtVectorValueState(project, camera));
                state->setValue(glm::value_ptr(value));
                state->commit();
                nanoem_delete(state);
            }
            else {
                camera->setLookAt(value);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_LIGHT_COLOR: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SetLightColorCommand *commandPtr = command->set_light_color;
            const Vector3 value(commandPtr->red, commandPtr->green, commandPtr->blue);
            ILight *light = project->globalLight();
            if (commandPtr->has_can_undo && commandPtr->can_undo) {
                internal::LightColorVectorValueState *state =
                    nanoem_new(internal::LightColorVectorValueState(project, light));
                state->setValue(glm::value_ptr(value));
                state->commit();
                nanoem_delete(state);
            }
            else {
                light->setColor(value);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_LIGHT_DIRECTION: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SetLightDirectionCommand *commandPtr = command->set_light_direction;
            const Vector3 value(commandPtr->x, commandPtr->y, commandPtr->z);
            ILight *light = project->globalLight();
            if (commandPtr->has_can_undo && commandPtr->can_undo) {
                internal::LightDirectionVectorValueState *state =
                    nanoem_new(internal::LightDirectionVectorValueState(project, light));
                state->setValue(glm::value_ptr(value));
                state->commit();
                nanoem_delete(state);
            }
            else {
                light->setDirection(value);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_MODEL_BONE_ORIENTATION: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SetModelBoneOrientationCommand *commandPtr = command->set_model_bone_orientation;
            Model *model = commandPtr->has_model_handle ? project->findModelByHandle(commandPtr->model_handle)
                                                        : project->activeModel();
            if (model && commandPtr->bone_name) {
                if (const nanoem_model_bone_t *bonePtr = model->findBone(commandPtr->bone_name)) {
                    const Quaternion value(commandPtr->w, commandPtr->x, commandPtr->y, commandPtr->z);
                    model::Bone *bone = model::Bone::cast(bonePtr);
                    if (commandPtr->has_can_undo && commandPtr->can_undo) {
                        internal::BoneOrientationValueState *state =
                            nanoem_new(internal::BoneOrientationValueState(bonePtr, model, project));
                        state->setValue(glm::value_ptr(value));
                        state->commit();
                        nanoem_delete(state);
                    }
                    else {
                        bone->setLocalUserOrientation(value);
                    }
                }
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_MODEL_BONE_TRANSLATION: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SetModelBoneTranslationCommand *commandPtr = command->set_model_bone_translation;
            Model *model = commandPtr->has_model_handle ? project->findModelByHandle(commandPtr->model_handle)
                                                        : project->activeModel();
            if (model && commandPtr->bone_name) {
                if (const nanoem_model_bone_t *bonePtr = model->findBone(commandPtr->bone_name)) {
                    const Vector3 value(commandPtr->x, commandPtr->y, commandPtr->z);
                    model::Bone *bone = model::Bone::cast(bonePtr);
                    if (commandPtr->has_can_undo && commandPtr->can_undo) {
                        internal::BoneTranslationValueState *state =
                            nanoem_new(internal::BoneTranslationValueState(bonePtr, model, project));
                        state->setValue(glm::value_ptr(value));
                        state->commit();
                        nanoem_delete(state);
                    }
                    else {
                        bone->setLocalUserTranslation(value);
                    }
                }
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_MODEL_MORPH_WEIGHT: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SetModelMorphWeightCommand *commandPtr = command->set_model_morph_weight;
            Model *model = commandPtr->has_model_handle ? project->findModelByHandle(commandPtr->model_handle)
                                                        : project->activeModel();
            if (model && commandPtr->morph_name) {
                if (const nanoem_model_morph_t *morphPtr = model->findMorph(commandPtr->morph_name)) {
                    const nanoem_f32_t value = commandPtr->weight;
                    model::Morph *morph = model::Morph::cast(morphPtr);
                    morph->setWeight(value);
                }
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REGISTER_ACCESSORY_KEYFRAME: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__RegisterAccessoryKeyframeCommand *commandPtr =
                command->register_accessory_keyframe;
            Accessory *accessory = commandPtr->has_accessory_handle
                ? project->findAccessoryByHandle(commandPtr->accessory_handle)
                : project->activeAccessory();
            if (accessory) {
                CommandRegistrator registrator(project);
                registrator.registerAddAccessoryKeyframesCommandByCurrentLocalFrameIndex(accessory);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REGISTER_ALL_SELECTED_BONE_KEYFRAMES: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__RegisterAllSelectedBoneKeyframesCommand *commandPtr =
                command->register_all_selected_bone_keyframes;
            Model *model = commandPtr->has_model_handle ? project->findModelByHandle(commandPtr->model_handle)
                                                        : project->activeModel();
            if (model) {
                const nanoem_rsize_t numNames = commandPtr->n_bone_names;
                CommandRegistrator registrator(project);
                if (numNames > 0) {
                    const nanoem_frame_index_t frameIndex = project->currentLocalFrameIndex();
                    Motion::BoneFrameIndexSetMap boneFrameIndexListMap;
                    for (nanoem_rsize_t i = 0; i < numNames; i++) {
                        if (const nanoem_model_bone_t *bone = model->findBone(commandPtr->bone_names[i])) {
                            boneFrameIndexListMap[bone].insert(frameIndex);
                        }
                    }
                    registrator.registerAddBoneKeyframesCommand(
                        boneFrameIndexListMap, model, project->resolveMotion(model));
                }
                else {
                    IModelObjectSelection *selection = model->selection();
                    const model::Bone::Set bones(selection->allBoneSet());
                    selection->addAllDirtyBones();
                    for (model::Bone::Set::const_iterator it = bones.begin(), end = bones.end(); it != end; ++it) {
                        selection->addBone(*it);
                    }
                    registrator.registerAddBoneKeyframesCommandBySelectedBoneSet(model);
                }
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REGISTER_ALL_SELECTED_MORPH_KEYFRAMES: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__RegisterAllSelectedMorphKeyframesCommand *commandPtr =
                command->register_all_selected_morph_keyframes;
            Model *model = commandPtr->has_model_handle ? project->findModelByHandle(commandPtr->model_handle)
                                                        : project->activeModel();
            if (model) {
                const nanoem_rsize_t numNames = commandPtr->n_morph_names;
                CommandRegistrator registrator(project);
                if (numNames > 0) {
                    Motion::MorphFrameIndexSetMap morphFrameIndexListMap;
                    const nanoem_frame_index_t frameIndex = project->currentLocalFrameIndex();
                    for (nanoem_rsize_t i = 0; i < numNames; i++) {
                        const nanoem_model_morph_t *morph = model->findMorph(commandPtr->morph_names[i]);
                        const nanoem_model_morph_category_t category = nanoemModelMorphGetCategory(morph);
                        if (category > NANOEM_MODEL_MORPH_CATEGORY_FIRST_ENUM &&
                            category < NANOEM_MODEL_MORPH_CATEGORY_MAX_ENUM) {
                            morphFrameIndexListMap[morph].insert(frameIndex);
                        }
                    }
                    registrator.registerAddMorphKeyframesCommand(
                        morphFrameIndexListMap, model, project->resolveMotion(model));
                }
                else {
                    registrator.registerAddMorphKeyframesCommandByAllMorphs(model);
                }
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REGISTER_CAMERA_KEYFRAME: {
        if (nanoem_likely(project)) {
            CommandRegistrator registrator(project);
            registrator.registerAddCameraKeyframesCommandByCurrentLocalFrameIndex();
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REGISTER_LIGHT_KEYFRAME: {
        if (nanoem_likely(project)) {
            CommandRegistrator registrator(project);
            registrator.registerAddLightKeyframesCommandByCurrentLocalFrameIndex();
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REGISTER_MODEL_KEYFRAME: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__RegisterModelKeyframeCommand *commandPtr = command->register_model_keyframe;
            Model *model = commandPtr->has_model_handle ? project->findModelByHandle(commandPtr->model_handle)
                                                        : project->activeModel();
            if (model) {
                CommandRegistrator registrator(project);
                registrator.registerAddModelKeyframesCommandByCurrentLocalFrameIndex(model);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REGISTER_SELF_SHADOW_KEYFRAME: {
        if (nanoem_likely(project)) {
            CommandRegistrator registrator(project);
            registrator.registerAddSelfShadowKeyframesCommandByCurrentLocalFrameIndex();
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REMOVE_ACCESSORY_KEYFRAME: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__RemoveAccessoryKeyframeCommand *commandPtr = command->remove_accessory_keyframe;
            Accessory *accessory = commandPtr->has_accessory_handle
                ? project->findAccessoryByHandle(commandPtr->accessory_handle)
                : project->activeAccessory();
            if (accessory) {
                CommandRegistrator registrator(project);
                registrator.registerRemoveAccessoryKeyframesCommandByCurrentLocalFrameIndex(accessory);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REMOVE_ALL_SELECTED_BONE_KEYFRAMES: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__RemoveAllSelectedBoneKeyframesCommand *commandPtr =
                command->remove_all_selected_bone_keyframes;
            Model *model = commandPtr->has_model_handle ? project->findModelByHandle(commandPtr->model_handle)
                                                        : project->activeModel();
            if (model) {
                const nanoem_rsize_t numNames = commandPtr->n_bone_names;
                CommandRegistrator registrator(project);
                if (numNames > 0) {
                    const nanoem_frame_index_t frameIndex = project->currentLocalFrameIndex();
                    Motion::BoneFrameIndexSetMap boneFrameIndexListMap;
                    for (nanoem_rsize_t i = 0; i < numNames; i++) {
                        if (const nanoem_model_bone_t *bone = model->findBone(commandPtr->bone_names[i])) {
                            boneFrameIndexListMap[bone].insert(frameIndex);
                        }
                    }
                    registrator.registerRemoveBoneKeyframesCommand(
                        boneFrameIndexListMap, model, project->resolveMotion(model));
                }
                else {
                    registrator.registerRemoveBoneKeyframesCommandByCurrentLocalFrameIndex(model);
                }
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REMOVE_ALL_SELECTED_MORPH_KEYFRAMES: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__RemoveAllSelectedMorphKeyframesCommand *commandPtr =
                command->remove_all_selected_morph_keyframes;
            Model *model = commandPtr->has_model_handle ? project->findModelByHandle(commandPtr->model_handle)
                                                        : project->activeModel();
            if (model) {
                const nanoem_rsize_t numNames = commandPtr->n_morph_names;
                CommandRegistrator registrator(project);
                if (numNames > 0) {
                    Motion::MorphFrameIndexSetMap morphFrameIndexListMap;
                    const nanoem_frame_index_t frameIndex = project->currentLocalFrameIndex();
                    for (nanoem_rsize_t i = 0; i < numNames; i++) {
                        const nanoem_model_morph_t *morph = model->findMorph(commandPtr->morph_names[i]);
                        const nanoem_model_morph_category_t category = nanoemModelMorphGetCategory(morph);
                        if (category > NANOEM_MODEL_MORPH_CATEGORY_FIRST_ENUM &&
                            category < NANOEM_MODEL_MORPH_CATEGORY_MAX_ENUM) {
                            morphFrameIndexListMap[morph].insert(frameIndex);
                        }
                    }
                    registrator.registerRemoveMorphKeyframesCommand(
                        morphFrameIndexListMap, model, project->resolveMotion(model));
                }
                else {
                    registrator.registerRemoveMorphKeyframesCommandByCurrentLocalFrameIndex(model);
                }
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REMOVE_CAMERA_KEYFRAME: {
        if (nanoem_likely(project)) {
            CommandRegistrator registrator(project);
            registrator.registerRemoveCameraKeyframesCommandByCurrentLocalFrameIndex();
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REMOVE_LIGHT_KEYFRAME: {
        if (nanoem_likely(project)) {
            CommandRegistrator registrator(project);
            registrator.registerRemoveLightKeyframesCommandByCurrentLocalFrameIndex();
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REMOVE_MODEL_KEYFRAME: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__RemoveModelKeyframeCommand *commandPtr = command->remove_model_keyframe;
            Model *model = commandPtr->has_model_handle ? project->findModelByHandle(commandPtr->model_handle)
                                                        : project->activeModel();
            if (model) {
                CommandRegistrator registrator(project);
                registrator.registerRemoveModelKeyframesCommandByCurrentLocalFrameIndex(model);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REMOVE_SELF_SHADOW_KEYFRAME: {
        if (nanoem_likely(project)) {
            CommandRegistrator registrator(project);
            registrator.registerRemoveSelfShadowKeyframesCommandByCurrentLocalFrameIndex();
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_UPDATE_MODEL: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__UpdateModelCommand *commandPtr = command->update_model;
            Model *model = commandPtr->has_model_handle ? project->findModelByHandle(commandPtr->model_handle)
                                                        : project->activeModel();
            if (model) {
                model->resetAllMorphDeformStates();
                model->deformAllMorphs(true);
                model->performAllBonesTransform();
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_BONE_BEZIER_CONTROL_POINT: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SetBoneBezierControlPointCommand *commandPtr =
                command->set_bone_bezier_control_point;
            Model *model = commandPtr->has_model_handle ? project->findModelByHandle(commandPtr->model_handle)
                                                        : project->activeModel();
            if (model && commandPtr->bone_name) {
                if (const nanoem_model_bone_t *bonePtr = model->findBone(commandPtr->bone_name)) {
                    const Vector4U8 value(commandPtr->x0, commandPtr->y0, commandPtr->x1, commandPtr->y1);
                    model::Bone *bone = model::Bone::cast(bonePtr);
                    bone->setBezierControlPoints(
                        static_cast<nanoem_motion_bone_keyframe_interpolation_type_t>(commandPtr->type), value);
                }
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_CAMERA_BEZIER_CONTROL_POINT: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SetCameraBezierControlPointCommand *commandPtr =
                command->set_camera_bezier_control_point;
            const Vector4U8 value(commandPtr->x0, commandPtr->y0, commandPtr->x1, commandPtr->y1);
            project->globalCamera()->setBezierControlPoints(
                static_cast<nanoem_motion_camera_keyframe_interpolation_type_t>(commandPtr->type), value);
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SELECT_ALL_BONES: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SelectAllBonesCommand *commandPtr = command->select_all_bones;
            Model *model = commandPtr->has_model_handle ? project->findModelByHandle(commandPtr->model_handle)
                                                        : project->activeModel();
            if (model) {
                model->selection()->addAllBones();
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SELECT_ALL_DIRTY_BONES: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SelectAllDirtyBonesCommand *commandPtr = command->select_all_dirty_bones;
            Model *model = commandPtr->has_model_handle ? project->findModelByHandle(commandPtr->model_handle)
                                                        : project->activeModel();
            if (model) {
                model->selection()->addAllDirtyBones();
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SELECT_ALL_MOVABLE_BONES: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SelectAllMovableBonesCommand *commandPtr = command->select_all_movable_bones;
            Model *model = commandPtr->has_model_handle ? project->findModelByHandle(commandPtr->model_handle)
                                                        : project->activeModel();
            if (model) {
                model->selection()->addAllMovableBones();
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SELECT_BONE: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SelectBoneCommand *commandPtr = command->select_bone;
            Model *model = commandPtr->has_model_handle ? project->findModelByHandle(commandPtr->model_handle)
                                                        : project->activeModel();
            if (model && commandPtr->bone_name) {
                if (const nanoem_model_bone_t *bonePtr = model->findBone(commandPtr->bone_name)) {
                    model->selection()->addBone(bonePtr);
                }
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_CLEAR_SELECT_ALL_BONES: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SelectAllMovableBonesCommand *commandPtr = command->select_all_movable_bones;
            Model *model = commandPtr->has_model_handle ? project->findModelByHandle(commandPtr->model_handle)
                                                        : project->activeModel();
            if (model) {
                model->selection()->removeAllBones();
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_CLEAR_SELECT_BONE: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SelectBoneCommand *commandPtr = command->select_bone;
            Model *model = commandPtr->has_model_handle ? project->findModelByHandle(commandPtr->model_handle)
                                                        : project->activeModel();
            if (model && commandPtr->bone_name) {
                if (const nanoem_model_bone_t *bonePtr = model->findBone(commandPtr->bone_name)) {
                    model->selection()->removeBone(bonePtr);
                }
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SELECT_ALL_COLUMN_MOTION_KEYFRAMES: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SelectAllColumnMotionKeyframesCommand *commandPtr =
                command->select_all_column_motion_keyframes;
            Motion *motion = commandPtr->has_motion_handle ? project->findMotionByHandle(commandPtr->motion_handle)
                                                           : project->resolveMotion(project->activeModel());
            if (motion) {
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SELECT_ALL_ROW_MOTION_KEYFRAMES: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SelectAllRowMotionKeyframesCommand *commandPtr =
                command->select_all_row_motion_keyframes;
            Motion *motion = commandPtr->has_motion_handle ? project->findMotionByHandle(commandPtr->motion_handle)
                                                           : project->resolveMotion(project->activeModel());
            if (motion) {
                IMotionKeyframeSelection *selection = motion->selection();
                const char *trackName = commandPtr->track_name;
                nanoem_frame_index_t end = project->duration();
                if (const Model *model = project->activeModel()) {
                    selection->addBoneKeyframes(model->findBone(trackName), 0, end);
                    selection->addMorphKeyframes(model->findMorph(trackName), 0, end);
                }
                else {
                    selection->addAccessoryKeyframes(0, end);
                    selection->addCameraKeyframes(0, end);
                    selection->addLightKeyframes(0, end);
                }
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SELECT_MOTION_KEYFRAME: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SelectMotionKeyframeCommand *commandPtr = command->select_motion_keyframe;
            Motion *motion = commandPtr->has_motion_handle ? project->findMotionByHandle(commandPtr->motion_handle)
                                                           : project->resolveMotion(project->activeModel());
            if (motion) {
                IMotionKeyframeSelection *selection = motion->selection();
                const char *trackName = commandPtr->track_name;
                const nanoem_frame_index_t frameIndexFrom = nanoem_frame_index_t(commandPtr->local_frame_index);
                nanoem_frame_index_t frameIndexTo = frameIndexFrom + 1;
                if (const Model *model = project->activeModel()) {
                    selection->addBoneKeyframes(model->findBone(trackName), frameIndexFrom, frameIndexTo);
                    selection->addMorphKeyframes(model->findMorph(trackName), frameIndexFrom, frameIndexTo);
                }
                else {
                    selection->addAccessoryKeyframes(frameIndexFrom, frameIndexTo);
                    selection->addCameraKeyframes(frameIndexFrom, frameIndexTo);
                    selection->addLightKeyframes(frameIndexFrom, frameIndexTo);
                }
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_CLEAR_SELECT_ALL_MOTION_KEYFRAMES: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__ClearSelectAllMotionKeyframesCommand *commandPtr =
                command->clear_select_all_motion_keyframes;
            Motion *motion = commandPtr->has_motion_handle ? project->findMotionByHandle(commandPtr->motion_handle)
                                                           : project->resolveMotion(project->activeModel());
            if (motion) {
                motion->selection()->clearAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_UPDATE_CURRENT_FPS: {
        /* do nothing */
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_RELOAD_ACCESSORY_EFFECT: {
        if (nanoem_likely(project)) {
            Nanoem__Application__ReloadAccessoryEffectCommand *commandPtr = command->reload_accessory_effect;
            if (Accessory *accessory = project->findAccessoryByHandle(commandPtr->accessory_handle)) {
                Progress progress(project, 0);
                succeeded = project->reloadDrawableEffect(accessory, progress, error);
                if (g_sentryAvailable) {
                    sentry_value_t breadcrumb = sentry_value_new_breadcrumb(nullptr, nullptr);
                    sentry_value_t data = sentry_value_new_object();
                    sentry_value_set_by_key(breadcrumb, "category", sentry_value_new_string("effect.reload.accessory"));
                    sentry_value_set_by_key(data, "handle", sentry_value_new_int32(commandPtr->accessory_handle));
                    sentry_value_set_by_key(breadcrumb, "data", data);
                    sentry_add_breadcrumb(breadcrumb);
                }
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_RELOAD_MODEL_EFFECT: {
        if (nanoem_likely(project)) {
            Nanoem__Application__ReloadModelEffectCommand *commandPtr = command->reload_model_effect;
            if (Model *model = project->findModelByHandle(commandPtr->model_handle)) {
                Progress progress(project, 0);
                succeeded = project->reloadDrawableEffect(model, progress, error);
                if (g_sentryAvailable) {
                    sentry_value_t breadcrumb = sentry_value_new_breadcrumb(nullptr, nullptr);
                    sentry_value_t data = sentry_value_new_object();
                    sentry_value_set_by_key(breadcrumb, "category", sentry_value_new_string("effect.reload.model"));
                    sentry_value_set_by_key(data, "handle", sentry_value_new_int32(commandPtr->model_handle));
                    sentry_value_set_by_key(breadcrumb, "data", data);
                    sentry_add_breadcrumb(breadcrumb);
                }
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_ACCESSORY_OUTSIDE_PARENT: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SetAccessoryOutsideParentCommand *commandPtr =
                command->set_accessory_outside_parent;
            const Model *parentModel = project->findModelByName(commandPtr->parent_model_name);
            Accessory *accessory = project->findAccessoryByHandle(commandPtr->accessory_handle);
            if (accessory && parentModel) {
                const nanoem_model_bone_t *parentBonePtr = parentModel->findBone(commandPtr->parent_model_bone_name);
                if (parentBonePtr) {
                    const model::Bone *parentBone = model::Bone::cast(parentBonePtr);
                    const StringPair value(parentModel->canonicalName(), parentBone->canonicalName());
                    accessory->setOutsideParent(value);
                }
                writeRedoMessage(command, project, error);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_MODEL_CONSTRAINT_STATE: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SetModelConstraintStateCommand *commandPtr = command->set_model_constraint_state;
            if (Model *model = project->findModelByHandle(commandPtr->model_handle)) {
                const char *constraintName = commandPtr->constraint_name;
                nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
                StringUtils::UnicodeStringScope scope(factory);
                const nanoem_model_constraint_t *constraintPtr = 0;
                if (StringUtils::tryGetString(factory, constraintName, StringUtils::length(constraintName), scope)) {
                    constraintPtr = model->findConstraint(scope.value());
                }
                if (constraintPtr) {
                    model::Constraint *constraint = model::Constraint::cast(constraintPtr);
                    constraint->setEnabled(commandPtr->value != 0);
                    writeRedoMessage(command, project, error);
                }
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_MODEL_EDGE_COLOR: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SetModelEdgeColorCommand *commandPtr = command->set_model_edge_color;
            if (Model *model = project->findModelByHandle(commandPtr->model_handle)) {
                const Vector4 color(commandPtr->red, commandPtr->green, commandPtr->blue, commandPtr->alpha);
                model->setEdgeColor(color);
                writeRedoMessage(command, project, error);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_MODEL_EDGE_SIZE: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SetModelEdgeSizeCommand *commandPtr = command->set_model_edge_size;
            if (Model *model = project->findModelByHandle(commandPtr->model_handle)) {
                model->setEdgeSizeScaleFactor(commandPtr->value);
                writeRedoMessage(command, project, error);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_MODEL_OUTSIDE_PARENT: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SetModelOutsideParentCommand *commandPtr = command->set_model_outside_parent;
            Model *model = project->findModelByHandle(commandPtr->model_handle);
            if (model) {
                const nanoem_model_bone_t *bonePtr = model->findBone(commandPtr->bone_name);
                model->setOutsideParent(
                    bonePtr, StringPair(commandPtr->parent_model_name, commandPtr->parent_model_bone_name));
            }
            writeRedoMessage(command, project, error);
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_MODEL_VISIBLE: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SetModelVisibleCommand *commandPtr = command->set_model_visible;
            if (Model *model = project->findModelByHandle(commandPtr->model_handle)) {
                model->setVisible(commandPtr->value != 0);
            }
            writeRedoMessage(command, project, error);
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_ADD_ACCESSORY_KEYFRAME: {
        if (nanoem_likely(project)) {
            Accessory *accessory =
                project->resolveRedoAccessory(command->redo_add_accessory_keyframe->accessory_handle);
            Motion *motion = project->resolveMotion(accessory);
            if (accessory && motion) {
                performRedo(command::AddAccessoryKeyframeCommand::create(command, accessory, motion),
                    project->undoStack(), undoCommandPtr);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_ADD_BONE_KEYFRAME: {
        if (nanoem_likely(project)) {
            Model *model = project->resolveRedoModel(command->redo_add_bone_keyframe->model_handle);
            Motion *motion = project->resolveMotion(model);
            if (model && motion) {
                performRedo(command::AddBoneKeyframeCommand::create(command, model, motion), model->undoStack(),
                    undoCommandPtr);
                nanoem_rsize_t numKeyframes = 0;
                nanoemMotionGetAllBoneKeyframeObjects(project->resolveMotion(model)->data(), &numKeyframes);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_ADD_CAMERA_KEYFRAME: {
        if (nanoem_likely(project)) {
            performRedo(
                command::AddCameraKeyframeCommand::create(command, project->globalCamera(), project->cameraMotion()),
                project->undoStack(), undoCommandPtr);
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_ADD_LIGHT_KEYFRAME: {
        if (nanoem_likely(project)) {
            performRedo(
                command::AddLightKeyframeCommand::create(command, project->globalLight(), project->lightMotion()),
                project->undoStack(), undoCommandPtr);
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_ADD_MODEL_KEYFRAME: {
        if (nanoem_likely(project)) {
            Model *model = project->resolveRedoModel(command->redo_add_model_keyframe->model_handle);
            Motion *motion = project->resolveMotion(model);
            if (model && motion) {
                performRedo(command::AddModelKeyframeCommand::create(command, model, motion), model->undoStack(),
                    undoCommandPtr);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_ADD_MORPH_KEYFRAME: {
        if (nanoem_likely(project)) {
            Model *model = project->resolveRedoModel(command->redo_add_morph_keyframe->model_handle);
            Motion *motion = project->resolveMotion(model);
            if (model && motion) {
                performRedo(command::AddMorphKeyframeCommand::create(command, model, motion), model->undoStack(),
                    undoCommandPtr);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_ADD_SELF_SHADOW_KEYFRAME: {
        if (nanoem_likely(project)) {
            performRedo(command::AddSelfShadowKeyframeCommand::create(
                            command, project->shadowCamera(), project->selfShadowMotion()),
                project->undoStack(), undoCommandPtr);
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_LOAD_ACCESSORY: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__RedoLoadAccessoryCommand *c = command->redo_load_accessory;
            if (c->content_case == NANOEM__APPLICATION__REDO_LOAD_ACCESSORY_COMMAND__CONTENT_FILE_URI) {
                const URI &fileURI = URI::createFromFilePath(c->file_uri->absolute_path, c->file_uri->fragment);
                if (project->fileManager()->loadFromFile(
                        fileURI, IFileManager::kDialogTypeLoadModelFile, project, error)) {
                    Accessory *accessory = project->allAccessories().back();
                    project->setRedoDrawable(c->accessory_handle, accessory);
                }
                else {
                    error.notify(eventPublisher());
                }
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_LOAD_MODEL: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__RedoLoadModelCommand *c = command->redo_load_model;
            if (c->content_case == NANOEM__APPLICATION__REDO_LOAD_MODEL_COMMAND__CONTENT_FILE_URI) {
                const URI &fileURI = URI::createFromFilePath(c->file_uri->absolute_path, c->file_uri->fragment);
                if (project->fileManager()->loadFromFile(
                        fileURI, IFileManager::kDialogTypeLoadModelFile, project, error)) {
                    Model *model = project->activeModel();
                    model->readLoadCommandMessage(command);
                    project->setRedoDrawable(c->model_handle, model);
                }
                else {
                    error.notify(eventPublisher());
                }
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_LOAD_MOTION: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__RedoLoadMotionCommand *c = command->redo_load_motion;
            if (c->content_case == NANOEM__APPLICATION__REDO_LOAD_MOTION_COMMAND__CONTENT_FILE_URI) {
                const URI &fileURI = URI::createFromFilePath(c->file_uri->absolute_path, c->file_uri->fragment);
                switch (c->type) {
                case NANOEM__APPLICATION__REDO_LOAD_MOTION_COMMAND__TYPE__ACCESSORY: {

                    break;
                }
                case NANOEM__APPLICATION__REDO_LOAD_MOTION_COMMAND__TYPE__CAMERA: {
                    if (!project->fileManager()->loadFromFile(
                            fileURI, IFileManager::kDialogTypeOpenCameraMotionFile, project, error)) {
                        error.notify(eventPublisher());
                    }
                    break;
                }
                case NANOEM__APPLICATION__REDO_LOAD_MOTION_COMMAND__TYPE__LIGHT: {
                    if (!project->fileManager()->loadFromFile(
                            fileURI, IFileManager::kDialogTypeOpenLightMotionFile, project, error)) {
                        error.notify(eventPublisher());
                    }
                    break;
                }
                case NANOEM__APPLICATION__REDO_LOAD_MOTION_COMMAND__TYPE__MODEL: {
                    Model *lastActiveModel = project->activeModel();
                    if (Model *model = project->resolveRedoModel(c->handle)) {
                        project->setActiveModel(model);
                        if (!project->fileManager()->loadFromFile(
                                fileURI, IFileManager::kDialogTypeOpenModelMotionFile, project, error)) {
                            error.notify(eventPublisher());
                        }
                        project->setActiveModel(lastActiveModel);
                    }
                    break;
                }
                case NANOEM__APPLICATION__REDO_LOAD_MOTION_COMMAND__TYPE__SELF_SHADOW: {
                    break;
                }
                default:
                    break;
                }
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_ACCESSORY_KEYFRAME: {
        if (nanoem_likely(project)) {
            Accessory *accessory = project->resolveRedoAccessory(command->remove_accessory_keyframe->accessory_handle);
            Motion *motion = project->resolveMotion(accessory);
            if (accessory && motion) {
                performRedo(command::RemoveAccessoryKeyframeCommand::create(command, accessory, motion),
                    project->undoStack(), undoCommandPtr);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_BONE_KEYFRAME: {
        if (nanoem_likely(project)) {
            Model *model = project->resolveRedoModel(command->redo_remove_bone_keyframe->model_handle);
            Motion *motion = project->resolveMotion(model);
            if (model && motion) {
                performRedo(command::RemoveBoneKeyframeCommand::create(command, model, motion), model->undoStack(),
                    undoCommandPtr);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_CAMERA_KEYFRAME: {
        if (nanoem_likely(project)) {
            performRedo(
                command::RemoveCameraKeyframeCommand::create(command, project->globalCamera(), project->cameraMotion()),
                project->undoStack(), undoCommandPtr);
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_LIGHT_KEYFRAME: {
        if (nanoem_likely(project)) {
            performRedo(
                command::RemoveLightKeyframeCommand::create(command, project->globalLight(), project->lightMotion()),
                project->undoStack(), undoCommandPtr);
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_MODEL_KEYFRAME: {
        if (nanoem_likely(project)) {
            Model *model = project->resolveRedoModel(command->redo_remove_model_keyframe->model_handle);
            Motion *motion = project->resolveMotion(model);
            if (model && motion) {
                performRedo(command::RemoveModelKeyframeCommand::create(command, model, motion), model->undoStack(),
                    undoCommandPtr);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_MORPH_KEYFRAME: {
        if (nanoem_likely(project)) {
            Model *model = project->resolveRedoModel(command->redo_remove_morph_keyframe->model_handle);
            Motion *motion = project->resolveMotion(model);
            if (model && motion) {
                performRedo(command::RemoveMorphKeyframeCommand::create(command, model, motion), model->undoStack(),
                    undoCommandPtr);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_SELF_SHADOW_KEYFRAME: {
        if (nanoem_likely(project)) {
            performRedo(command::RemoveSelfShadowKeyframeCommand::create(
                            command, project->shadowCamera(), project->selfShadowMotion()),
                project->undoStack(), undoCommandPtr);
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_SAVE_MOTION_SNAPSHOT: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__RedoSaveMotionSnapshotCommand *c = command->redo_save_motion_snapshot;
            undo_stack_t *stack = nullptr;
            if (c->has_handle) {
                if (Model *model = project->resolveRedoModel(c->handle)) {
                    stack = model->undoStack();
                }
            }
            performRedo(command::MotionSnapshotCommand::create(command, project), stack ? stack : project->undoStack(),
                undoCommandPtr);
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_INSERT_EMPTY_TIMELINE_FRAME: {
        if (nanoem_likely(project)) {
            performRedo(command::InsertEmptyTimelineFrameCommand::create(command, project), project->undoStack(),
                undoCommandPtr);
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_TIMELINE_FRAME: {
        if (nanoem_likely(project)) {
            performRedo(
                command::RemoveTimelineFrameCommand::create(command, project), project->undoStack(), undoCommandPtr);
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_TRANSFORM_BONE: {
        if (nanoem_likely(project)) {
            Model *model = project->resolveRedoModel(command->redo_transform_bone->model_handle);
            if (model) {
                performRedo(
                    command::TransformBoneCommand::create(command, model, project), model->undoStack(), undoCommandPtr);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_TRANSFORM_MORPH: {
        if (nanoem_likely(project)) {
            Model *model = project->resolveRedoModel(command->redo_transform_morph->model_handle);
            if (model) {
                performRedo(command::TransformMorphCommand::create(command, model, project), model->undoStack(),
                    undoCommandPtr);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_UNDO: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__UndoCommand *c = command->undo;
            undo_stack_t *stack = nullptr;
            undo_command_on_persist_undo_t cb = undoCommandGetOnPersistUndoCallback(undoCommandPtr);
            undoCommandSetOnPersistUndoCallback(undoCommandPtr, nullptr);
            if (c->has_model_handle) {
                if (Model *model = project->resolveRedoModel(c->model_handle)) {
                    stack = model->undoStack();
                }
            }
            undoStackUndo(stack ? stack : project->undoStack());
            undoCommandSetOnPersistUndoCallback(undoCommandPtr, cb);
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_UPDATE_ACCESSORY: {
        if (nanoem_likely(project)) {
            Accessory *accessory = project->resolveRedoAccessory(command->redo_update_accessory->accessory_handle);
            if (accessory) {
                performRedo(
                    command::UpdateAccessoryCommand::create(command, accessory), project->undoStack(), undoCommandPtr);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_UPDATE_CAMERA: {
        if (nanoem_likely(project)) {
            performRedo(command::UpdateCameraCommand::create(command, project->globalCamera(), project),
                project->undoStack(), undoCommandPtr);
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_UPDATE_LIGHT: {
        if (nanoem_likely(project)) {
            performRedo(command::UpdateLightCommand::create(command, project->globalLight(), project),
                project->undoStack(), undoCommandPtr);
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_BATCH_UNDO_COMMAND_LIST: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__RedoBatchUndoCommandListCommand *c = command->redo_batch_undo_command_list;
            Model *model = nullptr;
            undo_stack_t *stack = nullptr;
            if (c->has_model_handle) {
                if (Model *model = project->resolveRedoModel(c->model_handle)) {
                    stack = model->undoStack();
                }
            }
            performRedo(command::BatchUndoCommandListCommand::create(command, model, project),
                stack ? stack : project->undoStack(), undoCommandPtr);
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_ACCESSORY_ADD_BLEND_ENABLED: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SetAccessoryAddBlendEnabledCommand *c = command->set_accessory_add_blend_enabled;
            if (Accessory *accessory = project->findAccessoryByHandle(c->accessory_handle)) {
                accessory->setAddBlendEnabled(c->value != 0);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_ACCESSORY_SHADOW_ENABLED: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SetAccessoryShadowEnabledCommand *c = command->set_accessory_shadow_enabled;
            if (Accessory *accessory = project->findAccessoryByHandle(c->accessory_handle)) {
                accessory->setShadowMapEnabled(c->value != 0);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_ACCESSORY_VISIBLE: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SetAccessoryVisibleCommand *c = command->set_accessory_visible;
            if (Accessory *accessory = project->findAccessoryByHandle(c->accessory_handle)) {
                accessory->setVisible(c->value != 0);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_DRAWABLE_ORDER_INDEX: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SetDrawableOrderIndexCommand *c = command->set_drawable_order_index;
            if (Accessory *accessory = project->findAccessoryByHandle(c->handle)) {
            }
            else if (Model *model = project->findModelByHandle(c->handle)) {
            }
            DebugUtils::markUnimplemented("NANOEM__APPLICATION__COMMAND__TYPE_SET_DRAWABLE_ORDER_INDEX");
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_MODEL_ADD_BLEND_ENABLED: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SetModelAddBlendEnabledCommand *c = command->set_model_add_blend_enabled;
            if (Model *model = project->findModelByHandle(c->model_handle)) {
                model->setAddBlendEnabled(c->value != 0);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_MODEL_SHADOW_ENABLED: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SetModelShadowEnabledCommand *c = command->set_model_shadow_enabled;
            if (Model *model = project->findModelByHandle(c->model_handle)) {
                DebugUtils::markUnimplemented(
                    "NANOEM__APPLICATION__COMMAND__TYPE_SET_MODEL_SHADOW_ENABLED not implemented");
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_MODEL_SHADOW_MAP_ENABLED: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SetModelShadowMapEnabledCommand *c = command->set_model_shadow_map_enabled;
            if (Model *model = project->findModelByHandle(c->model_handle)) {
                model->setShadowMapEnabled(c->value != 0);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_MODEL_TRANSFORM_ORDER_INDEX: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SetModelTransformOrderIndexCommand *c = command->set_model_transform_order_index;
            if (Model *model = project->findModelByHandle(c->model_handle)) {
                DebugUtils::markUnimplemented(
                    "NANOEM__APPLICATION__COMMAND__TYPE_SET_MODEL_TRANSFORM_ORDER_INDEX not implemented");
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_MODEL_PHYSICS_SIMULATION_ENABLED: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SetModelPhysicsSimulationEnabledCommand *c =
                command->set_model_physics_simulation_enabled;
            if (Model *model = project->findModelByHandle(c->model_handle)) {
                model->setPhysicsSimulationEnabled(c->value != 0);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_CAMERA_KEYFRAME_INTERPOLATION: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SetCameraKeyframeInterpolationCommand *c =
                command->set_camera_keyframe_interpolation;
            ICamera *camera = project->globalCamera();
            camera->setBezierControlPoints(NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_X,
                internal::ApplicationUtils::toU8V(c->interpolation->x));
            camera->setBezierControlPoints(NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Y,
                internal::ApplicationUtils::toU8V(c->interpolation->y));
            camera->setBezierControlPoints(NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Z,
                internal::ApplicationUtils::toU8V(c->interpolation->z));
            camera->setBezierControlPoints(NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_ANGLE,
                internal::ApplicationUtils::toU8V(c->interpolation->angle));
            camera->setBezierControlPoints(NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FOV,
                internal::ApplicationUtils::toU8V(c->interpolation->fov));
            camera->setBezierControlPoints(NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_DISTANCE,
                internal::ApplicationUtils::toU8V(c->interpolation->distance));
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_MODEL_BONE_KEYFRAME_INTERPOLATION: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__SetModelBoneKeyframeInterpolationCommand *c =
                command->set_model_bone_keyframe_interpolation;
            if (Model *model = project->findModelByHandle(c->model_handle)) {
                const nanoem_model_bone_t *bonePtr = model->findBone(c->bone_name);
                if (model::Bone *bone = model::Bone::cast(bonePtr)) {
                    bone->setBezierControlPoints(NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_X,
                        internal::ApplicationUtils::toU8V(c->interpolation->x));
                    bone->setBezierControlPoints(NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Y,
                        internal::ApplicationUtils::toU8V(c->interpolation->y));
                    bone->setBezierControlPoints(NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Z,
                        internal::ApplicationUtils::toU8V(c->interpolation->z));
                    bone->setBezierControlPoints(NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_ORIENTATION,
                        internal::ApplicationUtils::toU8V(c->interpolation->orientation));
                }
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_DELETE_ACCESSORY: {
        if (nanoem_likely(project)) {
            if (Accessory *accessory =
                    project->resolveRedoAccessory(command->redo_delete_accessory->accessory_handle)) {
                project->removeAccessory(accessory);
                project->destroyAccessory(accessory);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_DELETE_MODEL: {
        if (nanoem_likely(project)) {
            if (Model *model = project->resolveRedoModel(command->redo_delete_model->model_handle)) {
                project->removeModel(model);
                project->destroyModel(model);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_UPDATE_PERFORMANCE_MONITOR: {
        const Nanoem__Application__UpdatePerformanceMonitorCommand *c = command->update_performance_monitor;
        m_window->setCurrentCPUPercentage(c->current_cpu_percentage);
        m_window->setCurrentMemoryBytes(c->current_resident_memory_bytes);
        m_window->setMaxMemoryBytes(c->max_resident_memory_bytes);
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SCREEN_CURSOR_PRESS: {
        const Nanoem__Application__ScreenCursorPressCommand *commandPtr = command->screen_cursor_press;
        m_window->setScreenCursorPress(commandPtr->type, commandPtr->x, commandPtr->y, commandPtr->modifiers);
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SCREEN_CURSOR_MOVE: {
        const Nanoem__Application__ScreenCursorMoveCommand *commandPtr = command->screen_cursor_move;
        m_window->setScreenCursorMove(commandPtr->x, commandPtr->y, commandPtr->modifiers);
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SCREEN_CURSOR_RELEASE: {
        const Nanoem__Application__ScreenCursorReleaseCommand *commandPtr = command->screen_cursor_release;
        m_window->setScreenCursorRelease(commandPtr->type, commandPtr->x, commandPtr->y, commandPtr->modifiers);
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_EXECUTE_MODEL_IO_PLUGIN: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__ExecuteModelIOPluginCommand *commandPtr = command->execute_model_io_plugin;
            DefaultFileManager::ModelIOPluginList plugins;
            m_defaultFileManager->getAllModelIOPlugins(plugins);
            Model *activeModel = project->activeModel();
            if (!activeModel) {
                const ITranslator *tr = translator();
                error = Error(tr->translate("nanoem.error.plugin.model.no-active-model.reason"),
                    tr->translate("nanoem.error.plugin.model.no-active-model.recovery-suggestion"),
                    Error::kDomainTypePlugin);
                succeeded = false;
            }
            else if (commandPtr->plugin_handle < plugins.size()) {
                ByteArray input;
                activeModel->setCodecType(NANOEM_CODEC_TYPE_UTF8);
                succeeded = activeModel->save(input, error);
                if (succeeded) {
                    const int functionIndex = commandPtr->function_handle;
                    plugin::ModelIOPlugin *plugin = plugins[commandPtr->plugin_handle];
                    PluginFactory::ModelIOPluginProxy proxy(plugin);
                    ByteArray layout;
                    proxy.setLanguage(project->castLanguage());
                    proxy.setFunction(functionIndex, error);
                    proxy.setInputData(input, error);
                    proxy.setupSelection(activeModel, error);
                    proxy.setProjectDescription(project, error);
                    if (!error.hasReason()) {
                        if (proxy.getUIWindowLayout(layout, error)) {
                            m_window->openModelIOPluginDialog(project, plugin, input, layout, functionIndex);
                        }
                        else {
                            proxy.execute(functionIndex, input, activeModel, project, error);
                        }
                        if (g_sentryAvailable) {
                            sentry_value_t breadcrumb = sentry_value_new_breadcrumb(nullptr, nullptr);
                            sentry_value_t data = sentry_value_new_object();
                            sentry_value_set_by_key(data, "name", sentry_value_new_string(proxy.name().c_str()));
                            sentry_value_set_by_key(data, "version", sentry_value_new_string(proxy.version().c_str()));
                            sentry_value_set_by_key(data, "function", sentry_value_new_int32(functionIndex));
                            sentry_value_set_by_key(
                                breadcrumb, "category", sentry_value_new_string("plugin.model.execute"));
                            sentry_value_set_by_key(breadcrumb, "data", data);
                            sentry_add_breadcrumb(breadcrumb);
                        }
                    }
                    succeeded = !error.hasReason();
                }
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_EXECUTE_MOTION_IO_PLUGIN: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__ExecuteMotionIOPluginCommand *commandPtr = command->execute_motion_io_plugin;
            DefaultFileManager::MotionIOPluginList plugins;
            m_defaultFileManager->getAllMotionIOPlugins(plugins);
            if (commandPtr->plugin_handle < plugins.size()) {
                plugin::MotionIOPlugin *plugin = plugins[commandPtr->plugin_handle];
                Model *activeModel = project->activeModel();
                Accessory *activeAccessory = project->activeAccessory();
                ByteArray input;
                if (Motion *motion = project->resolveMotion(activeModel)) {
                    succeeded = motion->save(input, activeModel, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL, error);
                }
                else if (Motion *motion = project->resolveMotion(activeAccessory)) {
                    succeeded = motion->save(input, nullptr, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL, error);
                }
                else {
                    Motion *tempMotion = project->createMotion();
                    tempMotion->mergeAllKeyframes(project->cameraMotion());
                    tempMotion->mergeAllKeyframes(project->lightMotion());
                    tempMotion->mergeAllKeyframes(project->selfShadowMotion());
                    succeeded = tempMotion->save(input, nullptr, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL, error);
                    project->destroyMotion(tempMotion);
                }
                if (succeeded) {
                    const int functionIndex = commandPtr->function_handle;
                    PluginFactory::MotionIOPluginProxy proxy(plugin);
                    proxy.setLanguage(project->castLanguage());
                    proxy.setFunction(functionIndex, error);
                    proxy.setInputData(input, error);
                    proxy.setProjectDescription(project, error);
                    succeeded = !error.hasReason();
                    if (Motion *motion = project->resolveMotion(activeModel)) {
                        proxy.setupModelSelection(motion, activeModel, error);
                    }
                    else if (Motion *motion = project->resolveMotion(activeAccessory)) {
                        proxy.setupAccessorySelection(motion, error);
                    }
                    if (!error.hasReason()) {
                        ByteArray layout;
                        if (proxy.getUIWindowLayout(layout, error)) {
                            m_window->openMotionIOPluginDialog(project, plugin, input, layout, functionIndex);
                        }
                        else {
                            proxy.execute(functionIndex, input, project, error);
                        }
                        if (g_sentryAvailable) {
                            sentry_value_t breadcrumb = sentry_value_new_breadcrumb(nullptr, nullptr);
                            sentry_value_t data = sentry_value_new_object();
                            sentry_value_set_by_key(data, "name", sentry_value_new_string(proxy.name().c_str()));
                            sentry_value_set_by_key(data, "version", sentry_value_new_string(proxy.version().c_str()));
                            sentry_value_set_by_key(data, "function", sentry_value_new_int32(functionIndex));
                            sentry_value_set_by_key(
                                breadcrumb, "category", sentry_value_new_string("plugin.motion.execute"));
                            sentry_value_set_by_key(breadcrumb, "data", data);
                            sentry_add_breadcrumb(breadcrumb);
                        }
                    }
                    succeeded = succeeded && !error.hasReason();
                }
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_SET_EFFECT_PLUGIN_ENABLED: {
        if (nanoem_likely(project)) {
            project->setEffectPluginEnabled(command->set_effect_plugin_enabled->value != 0);
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_QUERY_OPEN_SINGLE_FILE_DIALOG: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__QueryOpenSingleFileDialogCommand *commandPtr =
                command->query_open_single_file_dialog;
            const Nanoem__Application__URI *uri = commandPtr->file_uri;
            const URI &fileURI = URI::createFromFilePath(uri->absolute_path, uri->fragment);
            IFileManager::DialogType dialogType = static_cast<IFileManager::DialogType>(commandPtr->type);
            succeeded = project->fileManager()->loadFromFileWithModalDialog(fileURI, dialogType, project, error);
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_QUERY_OPEN_MULTIPLE_FILES_DIALOG: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__QueryOpenMultipleFilesDialogCommand *commandPtr =
                command->query_open_multiple_files_dialog;
            IFileManager *fileManager = project->fileManager();
            IFileManager::DialogType dialogType = static_cast<IFileManager::DialogType>(commandPtr->type);
            for (nanoem_rsize_t i = 0, numItems = commandPtr->n_file_uri; i < numItems; i++) {
                const Nanoem__Application__URI *uri = commandPtr->file_uri[i];
                const URI &fileURI = URI::createFromFilePath(uri->absolute_path, uri->fragment);
                succeeded &= fileManager->loadFromFileWithModalDialog(fileURI, dialogType, project, error);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_QUERY_SAVE_FILE_DIALOG: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__QuerySaveFileDialogCommand *commandPtr = command->query_save_file_dialog;
            const Nanoem__Application__URI *uri = commandPtr->file_uri;
            const URI &fileURI = URI::createFromFilePath(uri->absolute_path, uri->fragment);
            IFileManager::DialogType dialogType = static_cast<IFileManager::DialogType>(commandPtr->type);
            succeeded = project->fileManager()->saveAsFile(fileURI, dialogType, project, error);
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_DROP_FILE: {
        const Nanoem__Application__DropFileCommand *commandPtr = command->drop_file;
        const Nanoem__Application__URI *uri = commandPtr->file_uri;
        const URI &fileURI = URI::createFromFilePath(uri->absolute_path, uri->fragment);
        if (g_sentryAvailable) {
            sentry_value_t breadcrumb = sentry_value_new_breadcrumb(nullptr, nullptr);
            sentry_value_t data = sentry_value_new_object();
            sentry_value_set_by_key(data, "path", g_sentryMaskStringProc(fileURI.absolutePathConstString()));
            sentry_value_set_by_key(breadcrumb, "category", sentry_value_new_string("dropping"));
            sentry_value_set_by_key(breadcrumb, "data", data);
            sentry_add_breadcrumb(breadcrumb);
        }
        const nanoem_u64_t startedAt = stm_now();
        if (Project::isLoadableExtension(fileURI)) {
            succeeded = loadFromFile(fileURI, project, IFileManager::kDialogTypeOpenProject, error);
        }
        else if (Motion::isLoadableExtension(fileURI)) {
            const IFileManager::DialogType type = project->activeModel()
                ? IFileManager::kDialogTypeOpenModelMotionFile
                : IFileManager::kDialogTypeOpenCameraMotionFile;
            succeeded = loadFromFile(fileURI, project, type, error);
        }
        else if (Model::isLoadableExtension(fileURI) || Accessory::isLoadableExtension(fileURI)) {
            succeeded = loadFromFile(fileURI, project, IFileManager::kDialogTypeOpenModelFile, error);
        }
        else if (Effect::isLoadableExtension(fileURI)) {
            succeeded = loadFromFile(fileURI, project, IFileManager::kDialogTypeOpenEffectFile, error);
        }
        else if (fileURI.pathExtension() == String("wav")) {
            succeeded = loadFromFile(fileURI, project, IFileManager::kDialogTypeOpenAudioFile, error);
        }
        const nanoem_u64_t interval = stm_since(startedAt);
        if (g_sentryAvailable) {
            if (succeeded) {
                sentry_value_t breadcrumb = sentry_value_new_breadcrumb(nullptr, nullptr);
                sentry_value_t data = sentry_value_new_object();
                sentry_value_set_by_key(data, "path", g_sentryMaskStringProc(fileURI.absolutePathConstString()));
                sentry_value_set_by_key(data, "seconds", sentry_value_new_double(stm_sec(interval)));
                sentry_value_set_by_key(breadcrumb, "category", sentry_value_new_string("dropped"));
                sentry_value_set_by_key(breadcrumb, "data", data);
                sentry_add_breadcrumb(breadcrumb);
            }
            else if (error.isCancelled()) {
                sentry_value_t breadcrumb = sentry_value_new_breadcrumb(nullptr, nullptr);
                sentry_value_t data = sentry_value_new_object();
                sentry_value_set_by_key(data, "path", g_sentryMaskStringProc(fileURI.absolutePathConstString()));
                sentry_value_set_by_key(data, "seconds", sentry_value_new_double(stm_sec(interval)));
                sentry_value_set_by_key(breadcrumb, "category", sentry_value_new_string("cancelled"));
                sentry_value_set_by_key(breadcrumb, "data", data);
                sentry_add_breadcrumb(breadcrumb);
            }
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_GET_HANDLE_FILE_URI: {
        if (nanoem_likely(project)) {
            const Nanoem__Application__GetHandleFileURIRequestCommand *commandPtr = command->get_handle_file_uri;
            Nanoem__Application__GetHandleFileURIResponseEvent base =
                NANOEM__APPLICATION__GET_HANDLE_FILE_URIRESPONSE_EVENT__INIT;
            Nanoem__Application__URI uri;
            MutableString absolutePath, fragment;
            base.handle = commandPtr->handle;
            if (const Model *model = project->findModelByHandle(commandPtr->handle)) {
                base.file_uri = internal::ApplicationUtils::assignURI(&uri, absolutePath, fragment, model->fileURI());
            }
            else if (const Accessory *accessory = project->findAccessoryByHandle(commandPtr->handle)) {
                base.file_uri =
                    internal::ApplicationUtils::assignURI(&uri, absolutePath, fragment, accessory->fileURI());
            }
            else {
                base.file_uri = internal::ApplicationUtils::assignURI(&uri, absolutePath, fragment, URI());
            }
            Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
            event.type_case = NANOEM__APPLICATION__EVENT__TYPE_GET_HANDLE_FILE_URI;
            event.get_handle_file_uri = &base;
            sendQueryEventMessage(&event, command);
        }
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_CONFIRM_BEFORE_EXPORTING_IMAGE: {
        const ITranslator *tr = translator();
        const char *title = tr->translate("nanoem.window.confirm-before-exporting-image.title");
        const char *message = tr->translate("nanoem.window.confirm-before-exporting-image.message");
        ModalDialogFactory::SaveConfirmDialog callback(handleSaveOnNewProject, handleDiscardOnNewProject);
        addModalDialog(ModalDialogFactory::createConfirmSavingDialog(this, title, message, callback, this));
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_CONFIRM_BEFORE_EXPORTING_VIDEO: {
        const ITranslator *tr = translator();
        const char *title = tr->translate("nanoem.window.confirm-before-exporting-video.title");
        const char *message = tr->translate("nanoem.window.confirm-before-exporting-video.message");
        ModalDialogFactory::SaveConfirmDialog callback(handleSaveOnNewProject, handleDiscardOnNewProject);
        addModalDialog(ModalDialogFactory::createConfirmSavingDialog(this, title, message, callback, this));
        break;
    }
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_ADD_BONE:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_ADD_CONSTRAINT:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_ADD_JOINT:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_ADD_LABEL:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_ADD_MATERIAL:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_ADD_MORPH:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_ADD_RIGID_BODY:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_ADD_VERTEX:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_UPDATE_BONE:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_UPDATE_CONSTRAINT:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_UPDATE_JOINT:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_UPDATE_LABEL:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_UPDATE_MATERIAL:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_UPDATE_MORPH:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_UPDATE_RIGID_BODY:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_UPDATE_VERTEX:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_MODEL_HEADER:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_BONE:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_CONSTRAINT:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_JOINT:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_LABEL:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_MATERIAL:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_MORPH:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_RIGID_BODY:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_VERTEX:
    case NANOEM__APPLICATION__COMMAND__TYPE_REDO_SAVE_MODEL_SNAPSHOT:
        break;
    default:
        break;
    }
    return succeeded;
}

void
BaseApplicationService::handleSetupGraphicsEngine(sg_desc & /* desc */)
{
    /* do nothing */
}

void
BaseApplicationService::handleInitializeApplication()
{
    /* do nothing */
}

void
BaseApplicationService::handleNewProject()
{
    m_stateController->newProject();
    if (Project *project = m_stateController->currentProject()) {
        int language = project->castLanguage();
        sendLoadingAllModelIOPluginsEventMessage(language);
        sendLoadingAllMotionIOPluginsEventMessage(language);
    }
}

void
BaseApplicationService::handleSuspendProject()
{
    /* do nothing */
}

void
BaseApplicationService::handleResumeProject()
{
    /* do nothing */
}

void
BaseApplicationService::handleDestructApplication()
{
    /* do nothing */
}

void
BaseApplicationService::handleTerminateApplication()
{
    /* do nothing */
}

void
BaseApplicationService::postEmptyApplicationEvent()
{
    /* do nothing */
}

void
BaseApplicationService::beginDrawContext()
{
    if (nanoem_likely(m_context.id != SG_INVALID_ID)) {
        sg::activate_context(m_context);
    }
}

void
BaseApplicationService::endDrawContext()
{
    /* do nothing */
}

void
BaseApplicationService::presentDefaultPass(const Project * /* project */)
{
    /* do nothing */
}

void
BaseApplicationService::createDefaultRenderTarget(const Vector2UI16 & /* devicePixelWindowSize */)
{
    /* do nothing */
}

void
BaseApplicationService::resizeDefaultRenderTarget(
    const Vector2UI16 & /* devicePixelWindowSize */, const Project * /* project */)
{
    /* do nothing */
}

void
BaseApplicationService::destroyDefaultRenderTarget()
{
    /* do nothing */
}

Project::ISharedCancelPublisherRepository *
BaseApplicationService::sharedCancelPublisherRepository() NANOEM_DECL_NOEXCEPT
{
    return &m_sharedCancelPublisherRepository;
}

Project::ISharedDebugCaptureRepository *
BaseApplicationService::sharedDebugCaptureRepository() NANOEM_DECL_NOEXCEPT
{
    return &m_sharedDebugCaptureRepository;
}

Project::ISharedResourceRepository *
BaseApplicationService::sharedResourceRepository() NANOEM_DECL_NOEXCEPT
{
    return &m_sharedResourceRepository;
}

nanoem_rsize_t
BaseApplicationService::sizeofEventMessage(const Nanoem__Application__Event *event) NANOEM_DECL_NOEXCEPT
{
    return nanoem__application__event__get_packed_size(event);
}

void
BaseApplicationService::packEventMessage(const Nanoem__Application__Event *event, nanoem_u8_t *data)
{
    nanoem__application__event__pack(event, data);
}

void
BaseApplicationService::startCapture(Project *project, const URI &fileURI, Error &error)
{
    if (project && m_capturingPassState && !fileURI.isEmpty()) {
        m_capturingPassState->setFileURI(fileURI);
        m_capturingPassState->save(project);
        if (m_capturingPassState->start(error)) {
            addModalDialog(m_capturingPassState->createDialog());
            project->eventPublisher()->publishPlayEvent(project->duration(), 0);
        }
        else {
            stopCapture(project);
            error.addModalDialog(this);
        }
    }
    else {
        internal::CapturingPassState *state = m_capturingPassState;
        m_capturingPassState = nullptr;
        nanoem_delete(state);
    }
}

void
BaseApplicationService::stopCapture(Project *project)
{
    if (m_capturingPassState) {
        if (m_capturingPassState->transitDestruction(project)) {
            nanoem_delete_safe(m_capturingPassState);
        }
        if (project) {
            project->eventPublisher()->publishStopEvent(project->duration(), 0);
        }
    }
}

void
BaseApplicationService::setProject(Project *project)
{
    stopCapture(project);
    m_stateController->setProject(project);
}

void *
BaseApplicationService::resolveDllProcAddress(const char *name)
{
    return bx::dlsym(m_dllHandle, name);
}

void
BaseApplicationService::drawDefaultPass()
{
    Project *project = m_stateController->currentProject();
    if (m_initialized && project) {
        const bool active = project->isActive();
        bool resetAllPasses = false;
        if (active) {
            OPTICK_FRAME(__PRETTY_FUNCTION__);
            beginDrawContext();
            draw(project, m_stateController);
            sg::commit();
            presentDefaultPass(project);
            resetAllPasses = project->resetAllPasses();
            if (m_stateController->globalFrameIndex() % 2 == 0) {
                postEmptyApplicationEvent();
            }
            m_stateController->consumeDefaultPass();
            endDrawContext();
        }
    }
}

BaseApplicationService::Confirmer::Confirmer(BaseApplicationService *applicationPtr)
    : m_parent(applicationPtr)
{
}

void
BaseApplicationService::Confirmer::seek(nanoem_frame_index_t frameIndex, Project *project)
{
    if (project->isConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL)) {
        // TODO: translation
        ModalDialogFactory::StandardConfirmDialogCallbackPair callback(handleSeek);
        nanoem_frame_index_t *copyofTimeIndex = nanoem_new(nanoem_frame_index_t);
        *copyofTimeIndex = frameIndex;
        m_parent->addModalDialog(ModalDialogFactory::createStandardConfirmDialog(
            m_parent, "confirm", "confirm seek", callback, copyofTimeIndex));
    }
    else {
        project->seek(frameIndex, true);
    }
}

void
BaseApplicationService::Confirmer::play(Project *project)
{
    if (project->isConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL)) {
        // TODO: translation
        ModalDialogFactory::StandardConfirmDialogCallbackPair callback(handlePlay);
        m_parent->addModalDialog(
            ModalDialogFactory::createStandardConfirmDialog(m_parent, "confirm", "confirm play", callback, nullptr));
    }
    else {
        project->play();
    }
}

void
BaseApplicationService::Confirmer::resume(Project *project)
{
    if (project->isConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL)) {
        // TODO: translation
        ModalDialogFactory::StandardConfirmDialogCallbackPair callback(handleResume);
        m_parent->addModalDialog(
            ModalDialogFactory::createStandardConfirmDialog(m_parent, "confirm", "confirm resume", callback, nullptr));
    }
    else {
        project->resume(true);
    }
}

IModalDialog *
BaseApplicationService::Confirmer::handleSeek(void *userData, Project *project)
{
    nanoem_frame_index_t *frameIndex = static_cast<nanoem_frame_index_t *>(userData);
    project->seek(*frameIndex, true);
    nanoem_delete(frameIndex);
    return nullptr;
}

IModalDialog *
BaseApplicationService::Confirmer::handlePlay(void * /* userData */, Project *project)
{
    project->play();
    return nullptr;
}

IModalDialog *
BaseApplicationService::Confirmer::handleResume(void * /* userData */, Project *project)
{
    project->resume(true);
    return nullptr;
}

BaseApplicationService::SharedCancelPublisherRepository::SharedCancelPublisherRepository(
    BaseApplicationService *service)
    : m_service(service)
    , m_cancelPublisher(nullptr)
{
}

BaseApplicationService::SharedCancelPublisherRepository::~SharedCancelPublisherRepository() NANOEM_DECL_NOEXCEPT
{
    if (m_cancelPublisher) {
        m_cancelPublisher->stop();
        nanoem_delete_safe(m_cancelPublisher);
    }
}

ICancelPublisher *
BaseApplicationService::SharedCancelPublisherRepository::cancelPublisher()
{
    if (!m_cancelPublisher) {
        m_cancelPublisher = m_service->createCancelPublisher();
        if (m_cancelPublisher) {
            m_cancelPublisher->start();
        }
    }
    return m_cancelPublisher;
}

BaseApplicationService::SharedDebugCaptureRepository::SharedDebugCaptureRepository(BaseApplicationService *service)
    : m_service(service)
    , m_debugCapture(nullptr)
{
}

BaseApplicationService::SharedDebugCaptureRepository::~SharedDebugCaptureRepository() NANOEM_DECL_NOEXCEPT
{
    nanoem_delete_safe(m_debugCapture);
}

IDebugCapture *
BaseApplicationService::SharedDebugCaptureRepository::debugCapture()
{
    if (!m_debugCapture) {
        m_debugCapture = m_service->createDebugCapture();
    }
    return m_debugCapture;
}

BaseApplicationService::SharedResourceRepository::SharedResourceRepository()
    : m_accessoryProgramBundle(nullptr)
    , m_modelProgramBundle(nullptr)
    , m_effectGlobalUniform(nullptr)
    , m_sharedToonImagesInitialized(false)
    , m_sharedToonColorsInitialized(false)
{
    Inline::clearZeroMemory(m_sharedToonColors);
    Inline::clearZeroMemory(m_sharedToonImages);
    for (size_t i = 0; i < BX_COUNTOF(m_sharedToonColors); i++) {
        m_sharedToonColors[i] = Constants::kZeroV4;
    }
    for (size_t i = 0; i < BX_COUNTOF(m_sharedToonImages); i++) {
        Image *imageRef = m_sharedToonImages[i] = nanoem_new(Image);
        Inline::clearZeroMemory(imageRef->m_description);
        imageRef->m_handle = { SG_INVALID_ID };
    }
}

BaseApplicationService::SharedResourceRepository::~SharedResourceRepository() NANOEM_DECL_NOEXCEPT
{
    for (size_t i = 0; i < BX_COUNTOF(m_sharedToonImages); i++) {
        nanoem_delete_safe(m_sharedToonImages[i]);
    }
    nanoem_delete_safe(m_accessoryProgramBundle);
    nanoem_delete_safe(m_modelProgramBundle);
    nanoem_delete_safe(m_effectGlobalUniform);
}

void
BaseApplicationService::SharedResourceRepository::destroy()
{
    for (size_t i = 0; i < BX_COUNTOF(m_sharedToonImages); i++) {
        sg::destroy_image(m_sharedToonImages[i]->m_handle);
    }
    if (m_modelProgramBundle) {
        m_modelProgramBundle->destroy();
    }
    if (m_accessoryProgramBundle) {
        m_accessoryProgramBundle->destroy();
    }
}

AccessoryProgramBundle *
BaseApplicationService::SharedResourceRepository::accessoryProgramBundle()
{
    if (!m_accessoryProgramBundle) {
        m_accessoryProgramBundle = nanoem_new(AccessoryProgramBundle);
    }
    return m_accessoryProgramBundle;
}

ModelProgramBundle *
BaseApplicationService::SharedResourceRepository::modelProgramBundle()
{
    if (!m_modelProgramBundle) {
        m_modelProgramBundle = nanoem_new(ModelProgramBundle);
    }
    return m_modelProgramBundle;
}

effect::GlobalUniform *
BaseApplicationService::SharedResourceRepository::effectGlobalUniform()
{
    if (!m_effectGlobalUniform) {
        m_effectGlobalUniform = nanoem_new(effect::GlobalUniform);
    }
    return m_effectGlobalUniform;
}

const IImageView *
BaseApplicationService::SharedResourceRepository::toonImage(int value)
{
    const Image *imagePtr = nullptr;
    if (!m_sharedToonImagesInitialized) {
        resources::initializeSharedToonTextures(m_sharedToonImages, m_sharedToonImageData);
        m_sharedToonImagesInitialized = true;
    }
    if (value >= 0 && value < 10) {
        imagePtr = m_sharedToonImages[value];
    }
    return imagePtr;
}

Vector4
BaseApplicationService::SharedResourceRepository::toonColor(int value)
{
    Vector4 color(Constants::kZeroV3, 1.0f);
    if (!m_sharedToonColorsInitialized) {
        resources::initializeSharedToonColors(m_sharedToonColors);
        m_sharedToonColorsInitialized = true;
    }
    if (value >= 0 && value < 10) {
        color = m_sharedToonColors[value];
    }
    return color;
}

BaseApplicationService::UnicodeStringFactoryRepository::UnicodeStringFactoryRepository()
    : m_factory(nanoemUnicodeStringFactoryCreateEXT(nullptr))
{
    nanoem_assert(m_factory, "must not be nullptr");
}

BaseApplicationService::UnicodeStringFactoryRepository::~UnicodeStringFactoryRepository() NANOEM_DECL_NOEXCEPT
{
    nanoemUnicodeStringFactoryDestroyEXT(m_factory);
    m_factory = nullptr;
}

nanoem_unicode_string_factory_t *
BaseApplicationService::UnicodeStringFactoryRepository::unicodeStringFactory() const NANOEM_DECL_NOEXCEPT
{
    return m_factory;
}

bool
BaseApplicationService::isUIVisible() const NANOEM_DECL_NOEXCEPT
{
    return m_window->isVisible();
}

void
BaseApplicationService::setUIVisible(bool value)
{
    m_window->setVisible(value);
}

IModalDialog *
BaseApplicationService::handleSaveOnNewProject(void *userData, Project * /* project */)
{
    BaseApplicationService *service = static_cast<BaseApplicationService *>(userData);
    service->sendSaveAfterConfirmEventMessage();
    return nullptr;
}

IModalDialog *
BaseApplicationService::handleDiscardOnNewProject(void *userData, Project * /* project */)
{
    BaseApplicationService *service = static_cast<BaseApplicationService *>(userData);
    service->sendDiscardAfterConfirmEventMessage();
    return nullptr;
}

IModalDialog *
BaseApplicationService::handleSaveOnOpenProject(void *userData, Project * /* project */)
{
    BaseApplicationService *service = static_cast<BaseApplicationService *>(userData);
    service->sendSaveAfterConfirmEventMessage();
    return nullptr;
}

IModalDialog *
BaseApplicationService::handleDiscardOnOpenProject(void *userData, Project * /* project */)
{
    BaseApplicationService *service = static_cast<BaseApplicationService *>(userData);
    service->sendDiscardAfterConfirmEventMessage();
    return nullptr;
}

IModalDialog *
BaseApplicationService::handleSaveOnExitApplication(void *userData, Project * /* project */)
{
    BaseApplicationService *service = static_cast<BaseApplicationService *>(userData);
    service->sendSaveAfterConfirmEventMessage();
    return nullptr;
}

IModalDialog *
BaseApplicationService::handleDiscardOnExitApplication(void *userData, Project * /* project */)
{
    BaseApplicationService *service = static_cast<BaseApplicationService *>(userData);
    service->sendDiscardAfterConfirmEventMessage();
    return nullptr;
}

void *
BaseApplicationService::allocateSGXMemory(void *opaque, size_t size, const char *file, int line)
{
    BX_UNUSED_1(opaque);
    return bx::realloc(g_sokol_allocator, nullptr, size, 0, file, Inline::roundInt32(line));
}

void
BaseApplicationService::releaseSGXMemory(void *opaque, void *ptr, const char *file, int line) NANOEM_DECL_NOEXCEPT
{
    BX_UNUSED_1(opaque);
    bx::free(g_sokol_allocator, ptr, 0, file, Inline::roundInt32(line));
}

void
BaseApplicationService::handleSGXMessage(void *opaque, const char *message, const char *file, int line)
{
    BaseApplicationService *self = static_cast<BaseApplicationService *>(opaque);
    uint32_t key = bx::hash<bx::HashMurmur2A>(message);
    HandledSGXMessageSet::const_iterator it = self->m_handledSGXMessages.find(key);
    if (it == self->m_handledSGXMessages.end()) {
        if (g_sentryAvailable) {
            sentry_value_t breadcrumb = sentry_value_new_breadcrumb(nullptr, message);
            sentry_value_set_by_key(breadcrumb, "category", sentry_value_new_string("error"));
            sentry_add_breadcrumb(breadcrumb);
        }
#if defined(NANOEM_ENABLE_DEBUG_LABEL)
        DebugUtils::print("[%s:%d] %s", file, line, message);
#else
        BX_UNUSED_2(file, line);
#endif
        self->m_handledSGXMessages.insert(key);
    }
    SG_INSERT_MARKER(message);
}

void
BaseApplicationService::writeRedoMessage(Nanoem__Application__Command *command, Project *project, Error &error)
{
    /* overwrite latest timestamp to prevent sender latency lag */
    command->timestamp = stm_now();
    project->writeRedoMessage(command, error);
}

void
BaseApplicationService::performRedo(undo_command_t *commandPtr, undo_stack_t *stack, undo_command_t *&commandPtrRef)
{
    undo_command_on_persist_redo_t cb = undoCommandGetOnPersistRedoCallback(commandPtr);
    undoCommandSetOnPersistRedoCallback(commandPtr, nullptr);
    undoStackPushCommand(stack, commandPtr);
    undoCommandSetOnPersistRedoCallback(commandPtr, cb);
    commandPtrRef = commandPtr;
}

void
BaseApplicationService::setMorphWeight(float value, Project *project)
{
    if (Model *activeModel = project->activeModel()) {
        if (model::Morph *morph = model::Morph::cast(activeModel->activeMorph())) {
            morph->setWeight(value);
            activeModel->resetAllMorphDeformStates();
            activeModel->deformAllMorphs(true);
            activeModel->performAllBonesTransform();
        }
    }
}

void
BaseApplicationService::sendSaveFileMessage(const URI &fileURI, uint32_t type, uint64_t interval, bool succeeded)
{
    Nanoem__Application__CompleteSavingFileEvent base = NANOEM__APPLICATION__COMPLETE_SAVING_FILE_EVENT__INIT;
    Nanoem__Application__URI uri2;
    MutableString absolutePath, fragment;
    base.file_uri = internal::ApplicationUtils::assignURI(&uri2, absolutePath, fragment, fileURI);
    base.type = type;
    base.ticks = interval;
    base.has_ticks = 1;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_COMPLETE_SAVING_FILE;
    event.complete_saving_file = &base;
    sendEventMessage(&event);
    if (g_sentryAvailable && succeeded) {
        sentry_value_t breadcrumb = sentry_value_new_breadcrumb(nullptr, nullptr);
        sentry_value_t data = sentry_value_new_object();
        sentry_value_set_by_key(data, "path", g_sentryMaskStringProc(fileURI.absolutePathConstString()));
        sentry_value_set_by_key(data, "seconds", sentry_value_new_double(stm_sec(interval)));
        sentry_value_set_by_key(breadcrumb, "category", sentry_value_new_string("saved"));
        sentry_value_set_by_key(breadcrumb, "data", data);
        sentry_add_breadcrumb(breadcrumb);
    }
}

void
BaseApplicationService::sendQueryEventMessage(
    Nanoem__Application__Event *event, const Nanoem__Application__Command *command)
{
    event->timestamp = internal::ApplicationUtils::timestamp();
    if (command) {
        event->has_requested_timestamp = 1;
        event->requested_timestamp = command->timestamp;
    }
    sendEventMessage(event);
}

void
BaseApplicationService::sendSaveAfterConfirmEventMessage()
{
    Nanoem__Application__SaveProjectAfterConfirmEvent base =
        NANOEM__APPLICATION__SAVE_PROJECT_AFTER_CONFIRM_EVENT__INIT;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_SAVE_PROJECT_AFTER_CONFIRM;
    event.save_project_after_confirm = &base;
    sendQueryEventMessage(&event, nullptr);
}

void
BaseApplicationService::sendDiscardAfterConfirmEventMessage()
{
    Nanoem__Application__DiscardProjectAfterConfirmEvent base =
        NANOEM__APPLICATION__DISCARD_PROJECT_AFTER_CONFIRM_EVENT__INIT;
    Nanoem__Application__Event event = NANOEM__APPLICATION__EVENT__INIT;
    event.type_case = NANOEM__APPLICATION__EVENT__TYPE_DISCARD_PROJECT_AFTER_CONFIRM;
    event.discard_project_after_confirm = &base;
    sendQueryEventMessage(&event, nullptr);
}

} /* namespace nanoem */
