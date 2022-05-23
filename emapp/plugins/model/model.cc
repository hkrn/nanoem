#include "emapp/sdk/Model.h"
#include "emapp/sdk/UI.h"

#include "emapp/src/protoc/common.pb-c.c"
#include "emapp/src/protoc/plugin.pb-c.c"
#include "nanoem/ext/mutable.c"
#include "nanoem/nanoem.c"

#include <string>
#include <vector>

namespace {

using namespace nanoem::application::plugin;

class ModelIOPlugin {
public:
    const char *kWindowId = "plugin.model.test";
    const char *kWindowTitle = "ModelIO Plugin Test";

    ModelIOPlugin()
        : m_currentModel(nullptr)
        , m_inputBuffer(nullptr)
        , m_intermediateBuffer(nullptr)
        , m_outputBuffer(nullptr)
        , m_languageIndex(0)
        , m_functionIndex(0)
    {
    }
    ~ModelIOPlugin()
    {
        clearUIWindowLayout();
        nanoemModelDestroy(m_currentModel);
        m_currentModel = nullptr;
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
        m_languageIndex = value;
    }
    const char *
    name() const
    {
        return "This is a test.";
    }
    const char *
    description() const
    {
        return "This is a test.";
    }
    const char *
    version() const
    {
        return "v1.0.0";
    }
    int
    countAllFunctions() const
    {
        return 10;
    }
    const char *
    functionName(int index) const
    {
        switch (index) {
        case 0:
            return "UI";
        case 1:
            return "State";
        case 2:
            return "Vertex";
        case 3:
            return "Material";
        case 4:
            return "Bone";
        case 5:
            return "Morph";
        case 6:
            return "Label";
        case 7:
            return "RigidBody";
        case 8:
            return "Joint";
        case 9:
            return "SoftBody";
        default:
            return "Unknown";
        }
    }
    void
    setFunction(int value)
    {
        m_functionIndex = value;
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
    setAllSelectedVertexObjectIndices(const int *data, nanoem_u32_t length)
    {
        m_selectedVertexObjectIndices = ObjectIndexList(data, data + length);
    }
    void
    setAllSelectedMaterialObjectIndices(const int *data, nanoem_u32_t length)
    {
        m_selectedMaterialObjectIndices = ObjectIndexList(data, data + length);
    }
    void
    setAllSelectedBoneObjectIndices(const int *data, nanoem_u32_t length)
    {
        m_selectedBoneObjectIndices = ObjectIndexList(data, data + length);
    }
    void
    setAllSelectedConstraintObjectIndices(const int *data, nanoem_u32_t length)
    {
        m_selectedConstraintObjectIndices = ObjectIndexList(data, data + length);
    }
    void
    setAllSelectedMorphObjectIndices(const int *data, nanoem_u32_t length)
    {
        m_selectedMorphObjectIndices = ObjectIndexList(data, data + length);
    }
    void
    setAllSelectedLabelObjectIndices(const int *data, nanoem_u32_t length)
    {
        m_selectedLabelObjectIndices = ObjectIndexList(data, data + length);
    }
    void
    setAllSelectedRigidBodyObjectIndices(const int *data, nanoem_u32_t length)
    {
        m_selectedRigidBodyObjectIndices = ObjectIndexList(data, data + length);
    }
    void
    setAllSelectedJointObjectIndices(const int *data, nanoem_u32_t length)
    {
        m_selectedJointObjectIndices = ObjectIndexList(data, data + length);
    }
    void
    setAllSelectedSoftBodyObjectIndices(const int *data, nanoem_u32_t length)
    {
        m_selectedSoftBodyObjectIndices = ObjectIndexList(data, data + length);
    }
    void
    setAllMaskedVertexObjectIndices(const int *data, nanoem_u32_t length)
    {
        m_maskedVertexObjectIndices = ObjectIndexList(data, data + length);
    }
    void
    setAllMaskedMaterialObjectIndices(const int *data, nanoem_u32_t length)
    {
        m_maskedMaterialObjectIndices = ObjectIndexList(data, data + length);
    }
    void
    setAllMaskedBoneObjectIndices(const int *data, nanoem_u32_t length)
    {
        m_maskedBoneObjectIndices = ObjectIndexList(data, data + length);
    }
    void
    setAllMaskedConstraintObjectIndices(const int *data, nanoem_u32_t length)
    {
        m_maskedConstraintObjectIndices = ObjectIndexList(data, data + length);
    }
    void
    setAllMaskedMorphObjectIndices(const int *data, nanoem_u32_t length)
    {
        m_maskedMorphObjectIndices = ObjectIndexList(data, data + length);
    }
    void
    setAllMaskedLabelObjectIndices(const int *data, nanoem_u32_t length)
    {
        m_maskedLabelObjectIndices = ObjectIndexList(data, data + length);
    }
    void
    setAllMaskedRigidBodyObjectIndices(const int *data, nanoem_u32_t length)
    {
        m_maskedRigidBodyObjectIndices = ObjectIndexList(data, data + length);
    }
    void
    setAllMaskedJointObjectIndices(const int *data, nanoem_u32_t length)
    {
        m_maskedJointObjectIndices = ObjectIndexList(data, data + length);
    }
    void
    setAllMaskedSoftBodyObjectIndices(const int *data, nanoem_u32_t length)
    {
        m_maskedSoftBodyObjectIndices = ObjectIndexList(data, data + length);
    }
    void
    setEditingModeEnabled(bool value)
    {
        m_editingModeEnabled = value;
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
        nanoemModelDestroy(m_currentModel);
        nanoemBufferDestroy(m_inputBuffer);
        m_inputBuffer = nanoemBufferCreate(data, length, &status);
        nanoemModelLoadFromBuffer(m_currentModel, m_inputBuffer, &status);
    }
    void
    execute()
    {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoemMutableBufferDestroy(m_outputBuffer);
        m_outputBuffer = nanoemMutableBufferCreate(&status);
        switch (m_functionIndex) {
        case 1: {
            fprintf(stderr, "State Executed: enabled = %d\n", m_editingModeEnabled);
            break;
        }
        case 2: {
            fprintf(stderr, "Vertex Executed: selected=%lu, masked=%lu\n", m_selectedVertexObjectIndices.size(),
                m_maskedVertexObjectIndices.size());
            break;
        }
        case 3: {
            fprintf(stderr, "Material Executed: selected=%lu, masked=%lu\n", m_selectedMaterialObjectIndices.size(),
                m_maskedMaterialObjectIndices.size());
            break;
        }
        case 4: {
            fprintf(stderr, "Bone Executed: selected=%lu, masked=%lu\n", m_selectedBoneObjectIndices.size(),
                m_maskedBoneObjectIndices.size());
            break;
        }
        case 5: {
            fprintf(stderr, "Morph Executed: selected=%lu, masked=%lu\n", m_selectedMorphObjectIndices.size(),
                m_maskedMorphObjectIndices.size());
            break;
        }
        case 6: {
            fprintf(stderr, "Label Executed: selected=%lu, masked=%lu\n", m_selectedLabelObjectIndices.size(),
                m_maskedLabelObjectIndices.size());
            break;
        }
        case 7: {
            fprintf(stderr, "RigidBody Executed: selected=%lu, masked=%lu\n", m_selectedRigidBodyObjectIndices.size(),
                m_maskedRigidBodyObjectIndices.size());
            break;
        }
        case 8: {
            fprintf(stderr, "Joint Executed: selected=%lu, masked=%lu\n", m_selectedJointObjectIndices.size(),
                m_maskedJointObjectIndices.size());
            break;
        }
        case 9: {
            fprintf(stderr, "SoftBody Executed: selected=%lu, masked=%lu\n", m_selectedSoftBodyObjectIndices.size(),
                m_maskedSoftBodyObjectIndices.size());
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
        switch (m_functionIndex) {
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
        switch (m_functionIndex) {
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
    setUIComponentData(const char *id, const nanoem_u8_t *data, nanoem_u32_t length, int *reloadLayout)
    {
        switch (m_functionIndex) {
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
    typedef std::vector<int> ObjectIndexList;
    typedef std::vector<Nanoem__Application__Plugin__UIComponent *> ComponentList;
    nanoem_model_t *m_currentModel;
    nanoem_buffer_t *m_inputBuffer;
    nanoem_buffer_t *m_intermediateBuffer;
    nanoem_mutable_buffer_t *m_outputBuffer;
    ObjectIndexList m_selectedVertexObjectIndices;
    ObjectIndexList m_selectedMaterialObjectIndices;
    ObjectIndexList m_selectedBoneObjectIndices;
    ObjectIndexList m_selectedConstraintObjectIndices;
    ObjectIndexList m_selectedMorphObjectIndices;
    ObjectIndexList m_selectedLabelObjectIndices;
    ObjectIndexList m_selectedRigidBodyObjectIndices;
    ObjectIndexList m_selectedJointObjectIndices;
    ObjectIndexList m_selectedSoftBodyObjectIndices;
    ObjectIndexList m_maskedVertexObjectIndices;
    ObjectIndexList m_maskedMaterialObjectIndices;
    ObjectIndexList m_maskedBoneObjectIndices;
    ObjectIndexList m_maskedConstraintObjectIndices;
    ObjectIndexList m_maskedMorphObjectIndices;
    ObjectIndexList m_maskedLabelObjectIndices;
    ObjectIndexList m_maskedRigidBodyObjectIndices;
    ObjectIndexList m_maskedJointObjectIndices;
    ObjectIndexList m_maskedSoftBodyObjectIndices;
    ComponentList m_components;
    std::string m_failureReason;
    std::string m_recoverySuggestion;
    int m_languageIndex;
    int m_functionIndex;
    bool m_editingModeEnabled;
};

} /* namespace anonymous */

struct nanoem_application_plugin_model_io_t : ModelIOPlugin { };

nanoem_u32_t APIENTRY
nanoemApplicationPluginModelIOGetABIVersion()
{
    return NANOEM_APPLICATION_PLUGIN_MODEL_ABI_VERSION;
}

void APIENTRY
nanoemApplicationPluginModelIOInitialize(void)
{
}

nanoem_application_plugin_model_io_t *APIENTRY
nanoemApplicationPluginModelIOCreate(void)
{
    return new nanoem_application_plugin_model_io_t;
}

nanoem_application_plugin_status_t APIENTRY
nanoemApplicationPluginModelIOSetLanguage(nanoem_application_plugin_model_io_t *plugin, int value)
{
    nanoem_application_plugin_status_t status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    if (nanoem_is_not_null(plugin)) {
        plugin->setLanguage(value);
        status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
    }
    return status;
}

const char *APIENTRY
nanoemApplicationPluginModelIOGetName(const nanoem_application_plugin_model_io_t *plugin)
{
    return nanoem_is_not_null(plugin) ? plugin->name() : nullptr;
}

const char *APIENTRY
nanoemApplicationPluginModelIOGetDescription(const nanoem_application_plugin_model_io_t *plugin)
{
    return nanoem_is_not_null(plugin) ? plugin->description() : nullptr;
}

const char *APIENTRY
nanoemApplicationPluginModelIOGetVersion(const nanoem_application_plugin_model_io_t *plugin)
{
    return nanoem_is_not_null(plugin) ? plugin->version() : nullptr;
}

int APIENTRY
nanoemApplicationPluginModelIOCountAllFunctions(const nanoem_application_plugin_model_io_t *plugin)
{
    return nanoem_is_not_null(plugin) ? plugin->countAllFunctions() : 0;
}

const char *APIENTRY
nanoemApplicationPluginModelIOGetFunctionName(const nanoem_application_plugin_model_io_t *plugin, int index)
{
    return nanoem_is_not_null(plugin) ? plugin->functionName(index) : nullptr;
}

void APIENTRY
nanoemApplicationPluginModelIOSetFunction(nanoem_application_plugin_model_io_t *plugin, int index, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setFunction(index);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetAudioDescription(
    nanoem_application_plugin_model_io_t *plugin, const nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setAudioDescription(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetCameraDescription(
    nanoem_application_plugin_model_io_t *plugin, const nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setCameraDescription(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetLightDescription(
    nanoem_application_plugin_model_io_t *plugin, const nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setLightDescription(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetInputAudioData(
    nanoem_application_plugin_model_io_t *plugin, const nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setInputAudio(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetInputModelData(
    nanoem_application_plugin_model_io_t *plugin, const nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setInputData(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetAudioData(nanoem_application_plugin_model_io_t *plugin, const nanoem_u8_t * /* data */,
    nanoem_u32_t /* length */, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetAllSelectedVertexObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setAllSelectedVertexObjectIndices(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetAllSelectedMaterialObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setAllSelectedMaterialObjectIndices(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetAllSelectedBoneObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setAllSelectedBoneObjectIndices(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetAllSelectedConstraintObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setAllSelectedConstraintObjectIndices(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetAllSelectedMorphObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setAllSelectedMorphObjectIndices(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetAllSelectedLabelObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setAllSelectedLabelObjectIndices(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetAllSelectedRigidBodyObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setAllSelectedRigidBodyObjectIndices(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetAllSelectedJointObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setAllSelectedJointObjectIndices(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetAllSelectedSoftBodyObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setAllSelectedSoftBodyObjectIndices(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetAllMaskedVertexObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setAllMaskedVertexObjectIndices(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetAllMaskedMaterialObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setAllMaskedMaterialObjectIndices(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetAllMaskedBoneObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setAllMaskedBoneObjectIndices(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetAllMaskedConstraintObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setAllMaskedConstraintObjectIndices(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetAllMaskedMorphObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setAllMaskedMorphObjectIndices(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetAllMaskedLabelObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setAllMaskedLabelObjectIndices(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetAllMaskedRigidBodyObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setAllMaskedRigidBodyObjectIndices(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetAllMaskedJointObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setAllMaskedJointObjectIndices(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetAllMaskedSoftBodyObjectIndices(
    nanoem_application_plugin_model_io_t *plugin, const int *data, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setAllMaskedSoftBodyObjectIndices(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetEditingModeEnabled(nanoem_application_plugin_model_io_t *plugin, int value)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setEditingModeEnabled(value != 0);
    }
}

void APIENTRY
nanoemApplicationPluginModelIOExecute(nanoem_application_plugin_model_io_t *plugin, nanoem_i32_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->execute();
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOGetOutputModelDataSize(nanoem_application_plugin_model_io_t *plugin, nanoem_u32_t *length)
{
    if (nanoem_likely(plugin && length)) {
        *length = Inline::saturateInt32U(plugin->outputSize());
    }
}

void APIENTRY
nanoemApplicationPluginModelIOGetOutputModelData(
    nanoem_application_plugin_model_io_t *plugin, nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (nanoem_likely(plugin && data)) {
        plugin->getOutputData(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOLoadUIWindowLayout(nanoem_application_plugin_model_io_t *plugin, nanoem_i32_t *status)
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
nanoemApplicationPluginModelIOGetUIWindowLayoutDataSize(
    nanoem_application_plugin_model_io_t *plugin, nanoem_u32_t *length)
{
    if (nanoem_likely(plugin && length)) {
        *length = Inline::saturateInt32U(plugin->getUIWindowLayoutDataSize());
    }
}

void APIENTRY
nanoemApplicationPluginModelIOGetUIWindowLayoutData(
    nanoem_application_plugin_model_io_t *plugin, nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status)
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
nanoemApplicationPluginModelIOSetUIComponentLayoutData(nanoem_application_plugin_model_io_t *plugin, const char *id,
    const nanoem_u8_t *data, nanoem_u32_t length, int *reloadLayout, nanoem_i32_t *status)
{
    if (nanoem_likely(plugin && id && data && reloadLayout)) {
        plugin->setUIComponentData(id, data, length, reloadLayout);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

const char *APIENTRY
nanoemApplicationPluginModelIOGetFailureReason(const nanoem_application_plugin_model_io_t *plugin)
{
    return nanoem_is_not_null(plugin) ? plugin->failureReason() : nullptr;
}

const char *APIENTRY
nanoemApplicationPluginModelIOGetRecoverySuggestion(const nanoem_application_plugin_model_io_t *plugin)
{
    return nanoem_is_not_null(plugin) ? plugin->recoverySuggestion() : nullptr;
}

void APIENTRY
nanoemApplicationPluginModelIODestroy(nanoem_application_plugin_model_io_t *plugin)
{
    delete plugin;
}

void APIENTRY
nanoemApplicationPluginModelIOTerminate(void)
{
}
