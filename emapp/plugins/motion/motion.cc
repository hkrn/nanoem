#include "emapp/sdk/Motion.h"
#include "emapp/sdk/UI.h"

#include "nanoem/ext/mutable.c"
#include "nanoem/nanoem.c"

#include <string>
#include <unordered_map>
#include <vector>

#include "emapp/src/protoc/common.pb-c.c"
#include "emapp/src/protoc/plugin.pb-c.c"

namespace {

using namespace nanoem::application::plugin;

class MotionIOPlugin {
public:
    MotionIOPlugin()
        : m_currentMotion(nullptr)
        , m_inputBuffer(nullptr)
        , m_intermediateBuffer(nullptr)
        , m_outputBuffer(nullptr)
        , m_language(0)
        , m_function(0)
    {
    }
    ~MotionIOPlugin()
    {
        clearUIWindowLayout();
        nanoemMotionDestroy(m_currentMotion);
        m_currentMotion = nullptr;
        nanoemBufferDestroy(m_inputBuffer);
        m_inputBuffer = nullptr;
        nanoemBufferDestroy(m_intermediateBuffer);
        m_intermediateBuffer = nullptr;
        nanoemMutableBufferDestroy(m_outputBuffer);
        m_outputBuffer = nullptr;
    }

    void
    setLanguage(int value)
    {
        m_language = value;
    }
    const char *
    name() const
    {
        return "";
    }
    const char *
    description() const
    {
        return "";
    }
    const char *
    version() const
    {
        return "";
    }
    int
    countAllFunctions() const
    {
        return 6;
    }
    const char *
    functionName(int index) const
    {
        switch (index) {
        case 0:
            return "Function 1";
        case 1:
            return "Function 2";
        case 2:
            return "Function 3";
        case 3:
            return "Function 4";
        case 4:
            return "Function 5";
        default:
            return "Unknown";
        }
    }
    void
    setFunction(int value)
    {
        m_function = value;
    }
    void
    setAllNamedSelectedBoneKeyframes(const char *name, const nanoem_frame_index_t *frameIndices, nanoem_u32_t length)
    {
        m_boneKeyframeIndices[name] = KeyframeIndexList(frameIndices, frameIndices + length);
    }
    void
    setAllNamedSelectedMorphKeyframes(const char *name, const nanoem_frame_index_t *frameIndices, nanoem_u32_t length)
    {
        m_morphKeyframeIndices[name] = KeyframeIndexList(frameIndices, frameIndices + length);
    }
    void
    setAllSelectedAccessoryKeyframes(const nanoem_frame_index_t *frameIndices, nanoem_u32_t length)
    {
        m_accessoryKeyframeIndices = KeyframeIndexList(frameIndices, frameIndices + length);
    }
    void
    setAllSelectedCameraKeyframes(const nanoem_frame_index_t *frameIndices, nanoem_u32_t length)
    {
        m_cameraKeyframeIndices = KeyframeIndexList(frameIndices, frameIndices + length);
    }
    void
    setAllSelectedLightKeyframes(const nanoem_frame_index_t *frameIndices, nanoem_u32_t length)
    {
        m_lightKeyframeIndices = KeyframeIndexList(frameIndices, frameIndices + length);
    }
    void
    setAllSelectedModelKeyframes(const nanoem_frame_index_t *frameIndices, nanoem_u32_t length)
    {
        m_modelKeyframeIndices = KeyframeIndexList(frameIndices, frameIndices + length);
    }
    void
    setAllSelectedSelfShadowKeyframes(const nanoem_frame_index_t *frameIndices, nanoem_u32_t length)
    {
        m_selfShadowKeyframeIndices = KeyframeIndexList(frameIndices, frameIndices + length);
    }
    void
    setAudioDescription(const nanoem_u8_t *data, nanoem_u32_t size)
    {
        if (Nanoem__Application__Plugin__AudioDescription *message =
                nanoem__application__plugin__audio_description__unpack(nullptr, size, data)) {
            fprintf(stderr, "audio = %p\n", message);
            nanoem__application__plugin__audio_description__free_unpacked(message, nullptr);
        }
    }
    void
    setCameraDescription(const nanoem_u8_t *data, nanoem_u32_t size)
    {
        if (Nanoem__Application__Plugin__CameraDescription *message =
                nanoem__application__plugin__camera_description__unpack(nullptr, size, data)) {
            fprintf(stderr, "camera = %p\n", message);
            nanoem__application__plugin__camera_description__free_unpacked(message, nullptr);
        }
    }
    void
    setLightDescription(const nanoem_u8_t *data, nanoem_u32_t size)
    {
        if (Nanoem__Application__Plugin__LightDescription *message =
                nanoem__application__plugin__light_description__unpack(nullptr, size, data)) {
            fprintf(stderr, "light = %p\n", message);
            nanoem__application__plugin__light_description__free_unpacked(message, nullptr);
        }
    }
    void
    setInputActiveModel(const nanoem_u8_t *data, nanoem_u32_t length)
    {
        fprintf(stderr, "%p:%d\n", data, length);
    }
    void
    setInputAudio(const nanoem_u8_t *data, nanoem_u32_t length)
    {
        fprintf(stderr, "%p:%d\n", data, length);
    }
    void
    setInputData(const nanoem_u8_t *data, nanoem_u32_t length)
    {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoemMotionDestroy(m_currentMotion);
        nanoemBufferDestroy(m_inputBuffer);
        m_inputBuffer = nanoemBufferCreate(data, length, &status);
        nanoemMotionLoadFromBuffer(m_currentMotion, m_inputBuffer, 0, &status);
    }
    void
    execute()
    {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoemMutableBufferDestroy(m_outputBuffer);
        m_outputBuffer = nanoemMutableBufferCreate(&status);
        switch (m_function) {
        case 0: {
            fprintf(stderr, "Function 1 executed\n");
            break;
        }
        case 1: {
            fprintf(stderr, "Function 2 executed\n");
            break;
        }
        case 2: {
            fprintf(stderr, "Function 3 executed\n");
            break;
        }
        case 3: {
            fprintf(stderr, "Function 4 executed\n");
            break;
        }
        case 4: {
            fprintf(stderr, "Function 5 executed\n");
            break;
        }
        default:
            fprintf(stderr, "Unknown executed\n");
            break;
        }
        nanoemBufferDestroy(m_intermediateBuffer);
        m_intermediateBuffer = nanoemMutableBufferCreateBufferObject(m_outputBuffer, &status);
    }
    nanoem_rsize_t
    outputSize() const
    {
        return nanoemBufferGetLength(m_intermediateBuffer);
    }
    void
    getOutputData(nanoem_u8_t *data, nanoem_u32_t length)
    {
        memcpy(data, nanoemBufferGetDataPtr(m_intermediateBuffer), length);
    }
    void
    loadUIWindowLayout(nanoem_application_plugin_status_t *status)
    {
        const char *selectables[] = { "Select 1", "Select 2", "Select 3" };
        clearUIWindowLayout();
        m_components.push_back(createLabel("label"));
        m_components.push_back(createButton("button", "button"));
        m_components.push_back(createCheckbox("check", "check", true));
        m_components.push_back(createSeparator());
        m_components.push_back(createRadiobox("radio", "radio", true));
        m_components.push_back(createSelectable("selectable", "selectable"));
        m_components.push_back(createCombobox("selectables", selectables, 3, 0));
        m_components.push_back(createDragScalarN("drag", "drag", 42, 0, 84));
        m_components.push_back(createInputScalarN("input", "input", 42, 1, 1));
        m_components.push_back(createSliderScalarN("slider", "slider", 42, 0, 84));
        nanoem_application_plugin_status_assign_success(status);
    }
    void
    clearUIWindowLayout()
    {
        for (auto component : m_components) {
            destroyComponent(component);
        }
        m_components.clear();
    }
    nanoem_rsize_t
    getUIWindowLayoutDataSize() const
    {
        switch (m_function) {
        case 0: {
            Nanoem__Application__Plugin__UIWindow window = NANOEM__APPLICATION__PLUGIN__UIWINDOW__INIT;
            window.n_items = m_components.size();
            window.items = const_cast<Nanoem__Application__Plugin__UIComponent **>(m_components.data());
            return nanoem__application__plugin__uiwindow__get_packed_size(&window);
        }
        default:
            return 0;
        }
    }
    void
    getUIWindowLayoutData(nanoem_u8_t *data, nanoem_u32_t length, nanoem_application_plugin_status_t *status)
    {
        switch (m_function) {
        case 0: {
            Nanoem__Application__Plugin__UIWindow window = NANOEM__APPLICATION__PLUGIN__UIWINDOW__INIT;
            window.n_items = m_components.size();
            window.items = const_cast<Nanoem__Application__Plugin__UIComponent **>(m_components.data());
            if (nanoem__application__plugin__uiwindow__get_packed_size(&window) <= length) {
                nanoem__application__plugin__uiwindow__pack(&window, data);
            }
            else {
                nanoem_application_plugin_status_assign_error(
                    status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
            }
            break;
        }
        default:
            break;
        }
    }
    void
    setUIComponentLayoutData(const char *id, const nanoem_u8_t *data, nanoem_u32_t length, int *reloadLayout)
    {
        switch (m_function) {
        default:
            nanoem_mark_unused(id);
            nanoem_mark_unused(data);
            nanoem_mark_unused(length);
            nanoem_mark_unused(reloadLayout);
            break;
        }
    }
    const char *
    failureReason() const
    {
        return m_failureReason.c_str();
    }
    const char *
    recoverySuggestion() const
    {
        return m_recoverySuggestion.c_str();
    }

private:
    typedef std::vector<Nanoem__Application__Plugin__UIComponent *> ComponentList;
    typedef std::vector<nanoem_frame_index_t> KeyframeIndexList;
    typedef std::unordered_map<std::string, KeyframeIndexList> NamedKeyframeIndexListMap;
    nanoem_motion_t *m_currentMotion;
    nanoem_buffer_t *m_inputBuffer;
    nanoem_buffer_t *m_intermediateBuffer;
    nanoem_mutable_buffer_t *m_outputBuffer;
    NamedKeyframeIndexListMap m_boneKeyframeIndices;
    NamedKeyframeIndexListMap m_morphKeyframeIndices;
    KeyframeIndexList m_accessoryKeyframeIndices;
    KeyframeIndexList m_cameraKeyframeIndices;
    KeyframeIndexList m_lightKeyframeIndices;
    KeyframeIndexList m_modelKeyframeIndices;
    KeyframeIndexList m_selfShadowKeyframeIndices;
    ComponentList m_components;
    std::string m_failureReason;
    std::string m_recoverySuggestion;
    int m_language;
    int m_function;
};

} /* namespace anonymous */

struct nanoem_application_plugin_motion_io_t : MotionIOPlugin { };

nanoem_u32_t APIENTRY
nanoemApplicationPluginMotionIOGetABIVersion()
{
    return NANOEM_APPLICATION_PLUGIN_MOTION_ABI_VERSION;
}

void APIENTRY
nanoemApplicationPluginMotionIOInitialize(void)
{
}

nanoem_application_plugin_motion_io_t *APIENTRY
nanoemApplicationPluginMotionIOCreate(void)
{
    return new nanoem_application_plugin_motion_io_t;
}

nanoem_application_plugin_status_t APIENTRY
nanoemApplicationPluginMotionIOSetLanguage(nanoem_application_plugin_motion_io_t *plugin, int value)
{
    nanoem_application_plugin_status_t status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    if (nanoem_is_not_null(plugin)) {
        plugin->setLanguage(value);
        status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
    }
    return status;
}

const char *APIENTRY
nanoemApplicationPluginMotionIOGetName(const nanoem_application_plugin_motion_io_t *plugin)
{
    return nanoem_is_not_null(plugin) ? plugin->name() : nullptr;
}

const char *APIENTRY
nanoemApplicationPluginMotionIOGetDescription(const nanoem_application_plugin_motion_io_t *plugin)
{
    return nanoem_is_not_null(plugin) ? plugin->description() : nullptr;
}

const char *APIENTRY
nanoemApplicationPluginMotionIOGetVersion(const nanoem_application_plugin_motion_io_t *plugin)
{
    return nanoem_is_not_null(plugin) ? plugin->version() : nullptr;
}

int APIENTRY
nanoemApplicationPluginMotionIOCountAllFunctions(const nanoem_application_plugin_motion_io_t *plugin)
{
    return nanoem_is_not_null(plugin) ? plugin->countAllFunctions() : 0;
}

const char *APIENTRY
nanoemApplicationPluginMotionIOGetFunctionName(const nanoem_application_plugin_motion_io_t *plugin, int index)
{
    return nanoem_is_not_null(plugin) ? plugin->functionName(index) : nullptr;
}

void APIENTRY
nanoemApplicationPluginMotionIOSetFunction(
    nanoem_application_plugin_motion_io_t *plugin, int index, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setFunction(index);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginMotionIOSetAllNamedSelectedBoneKeyframes(nanoem_application_plugin_motion_io_t *plugin,
    const char *name, const nanoem_frame_index_t *frameIndices, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setAllNamedSelectedBoneKeyframes(name, frameIndices, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginMotionIOSetAllNamedSelectedMorphKeyframes(nanoem_application_plugin_motion_io_t *plugin,
    const char *name, const nanoem_frame_index_t *frameIndices, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setAllNamedSelectedMorphKeyframes(name, frameIndices, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginMotionIOSetAllSelectedAccessoryKeyframes(nanoem_application_plugin_motion_io_t *plugin,
    const nanoem_frame_index_t *frameIndices, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setAllSelectedAccessoryKeyframes(frameIndices, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginMotionIOSetAllSelectedCameraKeyframes(nanoem_application_plugin_motion_io_t *plugin,
    const nanoem_frame_index_t *frameIndices, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setAllSelectedCameraKeyframes(frameIndices, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginMotionIOSetAllSelectedLightKeyframes(nanoem_application_plugin_motion_io_t *plugin,
    const nanoem_frame_index_t *frameIndices, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setAllSelectedLightKeyframes(frameIndices, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginMotionIOSetAllSelectedModelKeyframes(nanoem_application_plugin_motion_io_t *plugin,
    const nanoem_frame_index_t *frameIndices, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setAllSelectedModelKeyframes(frameIndices, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginMotionIOSetAllSelectedSelfShadowKeyframes(nanoem_application_plugin_motion_io_t *plugin,
    const nanoem_frame_index_t *frameIndices, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setAllSelectedSelfShadowKeyframes(frameIndices, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginMotionIOSetAudioDescription(
    nanoem_application_plugin_motion_io_t *plugin, const nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setAudioDescription(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginMotionIOSetCameraDescription(
    nanoem_application_plugin_motion_io_t *plugin, const nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setCameraDescription(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginMotionIOSetLightDescription(
    nanoem_application_plugin_motion_io_t *plugin, const nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setLightDescription(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginMotionIOSetInputActiveModelData(
    nanoem_application_plugin_motion_io_t *plugin, const nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setInputActiveModel(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginMotionIOSetInputAudioData(
    nanoem_application_plugin_motion_io_t *plugin, const nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setInputAudio(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginMotionIOSetInputMotionData(
    nanoem_application_plugin_motion_io_t *plugin, const nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setInputData(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginMotionIOExecute(nanoem_application_plugin_motion_io_t *plugin, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->execute();
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginMotionIOGetOutputMotionDataSize(
    nanoem_application_plugin_motion_io_t *plugin, nanoem_u32_t *length)
{
    if (nanoem_likely(plugin && length)) {
        *length = Inline::saturateInt32U(plugin->outputSize());
    }
}

void APIENTRY
nanoemApplicationPluginMotionIOGetOutputMotionData(
    nanoem_application_plugin_motion_io_t *plugin, nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_likely(plugin && data)) {
        plugin->getOutputData(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginMotionIOLoadUIWindowLayout(nanoem_application_plugin_motion_io_t *plugin, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    if (nanoem_likely(plugin)) {
        plugin->loadUIWindowLayout(statusPtr);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginMotionIOGetUIWindowLayoutDataSize(
    nanoem_application_plugin_motion_io_t *plugin, nanoem_u32_t *length)
{
    if (nanoem_likely(plugin && length)) {
        *length = Inline::saturateInt32U(plugin->getUIWindowLayoutDataSize());
    }
}

void APIENTRY
nanoemApplicationPluginMotionIOGetUIWindowLayoutData(
    nanoem_application_plugin_motion_io_t *plugin, nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    if (nanoem_likely(plugin && data)) {
        plugin->getUIWindowLayoutData(data, length, statusPtr);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginMotionIOSetUIComponentLayoutData(nanoem_application_plugin_motion_io_t *plugin, const char *id,
    const nanoem_u8_t *data, nanoem_u32_t length, int *reloadLayout, nanoem_i32_t *status)
{
    if (nanoem_likely(plugin && id && data && reloadLayout)) {
        plugin->setUIComponentLayoutData(id, data, length, reloadLayout);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

const char *APIENTRY
nanoemApplicationPluginMotionIOGetFailureReason(const nanoem_application_plugin_motion_io_t *plugin)
{
    return nanoem_is_not_null(plugin) ? plugin->failureReason() : nullptr;
}

const char *APIENTRY
nanoemApplicationPluginMotionIOGetRecoverySuggestion(const nanoem_application_plugin_motion_io_t *plugin)
{
    return nanoem_is_not_null(plugin) ? plugin->recoverySuggestion() : nullptr;
}

void APIENTRY
nanoemApplicationPluginMotionIODestroy(nanoem_application_plugin_motion_io_t *plugin)
{
    delete plugin;
}

void APIENTRY
nanoemApplicationPluginMotionIOTerminate(void)
{
}
