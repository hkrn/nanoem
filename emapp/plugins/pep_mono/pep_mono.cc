/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#define NOMINMAX
#include "emapp/sdk/Model.h"

#include <algorithm>
#include <string>
#include <vector>

#include "emapp/src/protoc/common.pb-c.c"
#include "emapp/src/protoc/common.pb-c.h"
#include "emapp/src/protoc/plugin.pb-c.c"
#include "emapp/src/protoc/plugin.pb-c.h"

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/debug-helpers.h"
#include "mono/metadata/environment.h"
#include "mono/metadata/mono-config.h"
#include "mono/metadata/threads.h"

#if defined(_WIN32)
#define PATH_MAX MAX_PATH
#endif

namespace {

using namespace nanoem::application::plugin;

static const char kPEPluginNamespace[] = "nanoem.Application.Plugin.PEPluginProxyHost";
static const char kPluginFactoryClassName[] = "PluginFactory";

class PEPlugin {
public:
    PEPlugin(const char *location)
        : m_domain(nullptr)
        , m_assembly(nullptr)
        , m_factory(nullptr)
        , m_languageIndex(0)
        , m_functionIndex(-1)
    {
        m_domain = mono_jit_init_version("nanoem", nullptr);
        if (m_domain) {
            mono_thread_attach(m_domain);
            char basePath[PATH_MAX] = {}, bufferPath[PATH_MAX] = {};
            if (const char *p = strrchr(location, '/')) {
                size_t length = p - location;
                if (length < sizeof(basePath)) {
                    strncpy(basePath, location, length);
                    basePath[length] = 0;
                }
            }
            snprintf(bufferPath, sizeof(bufferPath), "%s/PEPlugin.dll", basePath);
            m_assembly = mono_domain_assembly_open(m_domain, bufferPath);
            if (m_assembly) {
                MonoImage *image = mono_assembly_get_image(m_assembly);
                if (MonoClass *factoryClass =
                        mono_class_from_name(image, kPEPluginNamespace, kPluginFactoryClassName)) {
                    m_factory = mono_object_new(m_domain, factoryClass);
                    if (m_factory) {
                        mono_runtime_object_init(m_factory);
                        snprintf(bufferPath, sizeof(bufferPath), "%s/ModelIO", basePath);
                        MonoString *pluginDir = mono_string_new(m_domain, bufferPath);
                        void *args[] = { pluginDir };
                        MonoObject *exception = nullptr;
                        callMethod(m_factory, ":LoadAll", args, &exception);
                        if (MonoArray *registeredPlugins =
                                reinterpret_cast<MonoArray *>(callGetProperty(m_factory, "AllRegisteredPlugins"))) {
                            const uintptr_t count = mono_array_length(registeredPlugins);
                            for (uintptr_t i = 0; i < count; i++) {
                                PluginAssembly plugin;
                                MonoObject *item = mono_array_get(registeredPlugins, MonoObject *, i);
                                if (MonoString *assemblyPathString =
                                        reinterpret_cast<MonoString *>(callGetProperty(item, "AssemblyPath"))) {
                                    char *assemblyPath = mono_string_to_utf8(assemblyPathString);
                                    plugin.second = assemblyPath;
                                    mono_free(assemblyPath);
                                    MonoObject *option = callGetProperty(item, "Option");
                                    MonoString *registerMenuTextString =
                                        reinterpret_cast<MonoString *>(callGetProperty(option, "RegisterMenuText"));
                                    if (registerMenuTextString && mono_string_length(registerMenuTextString) > 0) {
                                        char *registerMenuText = mono_string_to_utf8(registerMenuTextString);
                                        plugin.first = registerMenuText;
                                        mono_free(registerMenuText);
                                        m_pluginAssemblies.push_back(plugin);
                                    }
                                    else {
                                        MonoString *nameString =
                                            reinterpret_cast<MonoString *>(callGetProperty(item, "Name"));
                                        if (nameString && mono_string_length(nameString) > 0) {
                                            char *name = mono_string_to_utf8(nameString);
                                            plugin.first = name;
                                            mono_free(name);
                                            m_pluginAssemblies.push_back(plugin);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    ~PEPlugin()
    {
        if (m_domain) {
            mono_jit_cleanup(m_domain);
            m_assembly = nullptr;
            m_factory = nullptr;
            m_domain = nullptr;
        }
    }

    void
    setLanguage(int value)
    {
        m_languageIndex = value;
    }
    const char *
    name() const
    {
        return "PEPlugin";
    }
    const char *
    description() const
    {
        return "PEPlugin compatible interface";
    }
    const char *
    version() const
    {
        return "v1.0.0";
    }
    int
    countAllFunctions() const
    {
        return Inline::saturateInt32(m_pluginAssemblies.size());
    }
    const char *
    functionName(int index) const
    {
        return index >= 0 && index < countAllFunctions() ? m_pluginAssemblies[index].first.c_str() : nullptr;
    }
    void
    setFunction(int value)
    {
        m_functionIndex = value >= 0 && value < countAllFunctions() ? value : -1;
    }
    void
    setAudioDescription(const nanoem_u8_t *data, nanoem_u32_t size)
    {
        if (Nanoem__Application__Plugin__AudioDescription *message =
                nanoem__application__plugin__audio_description__unpack(nullptr, size, data)) {
            nanoem__application__plugin__audio_description__free_unpacked(message, nullptr);
        }
    }
    void
    setCameraDescription(const nanoem_u8_t *data, nanoem_u32_t size)
    {
        if (Nanoem__Application__Plugin__CameraDescription *message =
                nanoem__application__plugin__camera_description__unpack(nullptr, size, data)) {
            nanoem__application__plugin__camera_description__free_unpacked(message, nullptr);
        }
    }
    void
    setLightDescription(const nanoem_u8_t *data, nanoem_u32_t size)
    {
        if (Nanoem__Application__Plugin__LightDescription *message =
                nanoem__application__plugin__light_description__unpack(nullptr, size, data)) {
            nanoem__application__plugin__light_description__free_unpacked(message, nullptr);
        }
    }
    void
    setAllSelectedVertexObjectIndices(const int *data, nanoem_u32_t length)
    {
        setSelectedModelObjectArray("AllSelectedVertices", data, length);
    }
    void
    setAllSelectedMaterialObjectIndices(const int *data, nanoem_u32_t length)
    {
        setSelectedModelObjectArray("AllSelectedMaterials", data, length);
    }
    void
    setAllSelectedBoneObjectIndices(const int *data, nanoem_u32_t length)
    {
        setSelectedModelObjectArray("AllSelectedBones", data, length);
    }
    void
    setAllSelectedConstraintObjectIndices(const int *data, nanoem_u32_t length)
    {
        setSelectedModelObjectArray("AllSelectedConstraints", data, length);
    }
    void
    setAllSelectedMorphObjectIndices(const int *data, nanoem_u32_t length)
    {
        setSelectedModelObjectArray("AllSelectedMorphs", data, length);
    }
    void
    setAllSelectedLabelObjectIndices(const int *data, nanoem_u32_t length)
    {
        setSelectedModelObjectArray("AllSelectedLabels", data, length);
    }
    void
    setAllSelectedRigidBodyObjectIndices(const int *data, nanoem_u32_t length)
    {
        setSelectedModelObjectArray("AllSelectedRigidBodies", data, length);
    }
    void
    setAllSelectedJointObjectIndices(const int *data, nanoem_u32_t length)
    {
        setSelectedModelObjectArray("AllSelectedJoints", data, length);
    }
    void
    setInputAudio(const nanoem_u8_t *data, nanoem_u32_t length)
    {
        nanoem_mark_unused(data);
        nanoem_mark_unused(length);
    }
    void
    setInputData(const nanoem_u8_t *data, nanoem_u32_t length)
    {
        m_inputData.assign(data, data + length);
    }
    void
    execute()
    {
        if (m_functionIndex >= 0 && m_functionIndex < countAllFunctions()) {
            const std::string &assemblyPath = m_pluginAssemblies[m_functionIndex].second;
            nanoem_u32_t size = m_inputData.size();
            MonoArray *inputData = mono_array_new(m_domain, mono_get_byte_class(), size);
            MonoArray *outputData = mono_array_new(m_domain, mono_get_byte_class(), 0);
            for (size_t i = 0; i < size; i++) {
                mono_array_set(inputData, nanoem_u8_t, i, m_inputData[i]);
            }
            void *args[] = { mono_string_new_len(m_domain, assemblyPath.c_str(), assemblyPath.size()), inputData,
                &outputData };
            MonoObject *exception = nullptr;
            callMethod(m_factory, ":Execute", args, &exception);
            m_inputData.clear();
            m_outputData.clear();
            if (!exception) {
                size = mono_array_length(outputData);
                for (size_t i = 0; i < size; i++) {
                    m_outputData.push_back(mono_array_get(outputData, nanoem_u8_t, i));
                }
            }
            else if (MonoString *s = mono_object_to_string(exception, nullptr)) {
                char *cs = mono_string_to_utf8(s);
                m_failureReason.assign(cs, mono_string_length(s));
                mono_free(cs);
            }
        }
    }
    nanoem_rsize_t
    outputSize() const
    {
        return m_outputData.size();
    }
    void
    getOutputData(nanoem_u8_t *data, nanoem_u32_t length)
    {
        memcpy(data, m_outputData.data(), std::min(length, Inline::saturateInt32U(m_outputData.size())));
    }
    void
    loadUIWindowLayoutData()
    {
    }
    nanoem_rsize_t
    getUIWindowLayoutDataSize() const
    {
        return 0;
    }
    void
    getUIWindowLayoutData(nanoem_u8_t *data, nanoem_u32_t length)
    {
        nanoem_mark_unused(data);
        nanoem_mark_unused(length);
    }
    void
    setUIWindowLayoutData(const nanoem_u8_t *data, nanoem_u32_t length)
    {
        nanoem_mark_unused(data);
        nanoem_mark_unused(length);
    }
    void
    setUIComponentData(const char *id, const nanoem_u8_t *data, nanoem_u32_t length)
    {
        nanoem_mark_unused(id);
        nanoem_mark_unused(data);
        nanoem_mark_unused(length);
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
    typedef std::vector<nanoem_u8_t> ByteArray;
    typedef std::vector<int> ObjectIndexList;
    typedef std::pair<std::string, std::string> PluginAssembly;
    typedef std::vector<PluginAssembly> PluginAssemblyList;

    static MonoMethod *
    findMethod(MonoClass *classInstance, const char *name)
    {
        MonoMethodDesc *desc = mono_method_desc_new(name, false);
        MonoMethod *method = mono_method_desc_search_in_class(desc, classInstance);
        mono_method_desc_free(desc);
        return method;
    }
    static MonoMethod *
    findGetProperty(MonoClass *classInstance, const char *name)
    {
        MonoMethod *method = nullptr;
        if (MonoProperty *prop = mono_class_get_property_from_name(classInstance, name)) {
            method = mono_property_get_get_method(prop);
        }
        return method;
    }
    static MonoMethod *
    findGetProperty(MonoObject *objectInstance, const char *name)
    {
        return findGetProperty(mono_object_get_class(objectInstance), name);
    }
    static MonoObject *
    callGetProperty(MonoObject *objectInstance, const char *name)
    {
        MonoObject *result = nullptr;
        if (MonoMethod *method = findGetProperty(objectInstance, name)) {
            result = mono_runtime_invoke(method, objectInstance, nullptr, nullptr);
        }
        return result;
    }
    static MonoMethod *
    findSetProperty(MonoClass *classInstance, const char *name)
    {
        MonoMethod *method = nullptr;
        if (MonoProperty *prop = mono_class_get_property_from_name(classInstance, name)) {
            method = mono_property_get_set_method(prop);
        }
        return method;
    }
    static MonoMethod *
    findSetProperty(MonoObject *objectInstance, const char *name)
    {
        return findSetProperty(mono_object_get_class(objectInstance), name);
    }
    static void
    callSetProperty(MonoObject *objectInstance, const char *name, MonoObject *value)
    {
        if (MonoMethod *method = findSetProperty(objectInstance, name)) {
            void *args[] = { value };
            mono_runtime_invoke(method, objectInstance, args, nullptr);
        }
    }
    static MonoObject *
    callMethod(MonoObject *objectInstance, const char *name, void **args, MonoObject **exception)
    {
        MonoObject *result = nullptr;
        if (MonoMethod *method = findMethod(mono_object_get_class(objectInstance), name)) {
            result = mono_runtime_invoke(method, objectInstance, args, exception);
        }
        return result;
    }
    void
    setSelectedModelObjectArray(const char *name, const int *data, nanoem_u32_t length)
    {
        MonoArray *value = mono_array_new(m_domain, mono_get_int32_class(), length);
        for (size_t i = 0; i < length; i++) {
            mono_array_set(value, int, i, data[i]);
        }
        callSetProperty(m_factory, name, reinterpret_cast<MonoObject *>(value));
    }

    MonoDomain *m_domain;
    MonoAssembly *m_assembly;
    MonoObject *m_factory;
    PluginAssemblyList m_pluginAssemblies;
    ByteArray m_inputData;
    ByteArray m_outputData;
    std::string m_failureReason;
    std::string m_recoverySuggestion;
    int m_languageIndex;
    int m_functionIndex;
};

} /* namespace anonymous */

struct nanoem_application_plugin_model_io_t : PEPlugin {
    nanoem_application_plugin_model_io_t(const char *location)
        : PEPlugin(location)
    {
    }
};

void APIENTRY
nanoemApplicationPluginModelIOInitialize(void)
{
#if defined(_WIN32)
    char assemblyPath[PATH_MAX], configPath[PATH_MAX];
    auto getKey = [](const char *key, char *value, DWORD *length) {
        HKEY result;
        if (RegOpenKeyA(HKEY_LOCAL_MACHINE, "Software\\Mono", &result) == S_OK) {
            DWORD type;
            RegGetValueA(result, nullptr, key, RRF_RT_REG_SZ, &type, value, length);
            RegCloseKey(result);
        }
    };
    DWORD length = sizeof(assemblyPath);
    getKey("FrameworkAssemblyDirectory", assemblyPath, &length);
    length = sizeof(configPath);
    getKey("MonoConfigDir", configPath, &length);
    mono_set_dirs(assemblyPath, configPath);
#endif
    mono_set_crash_chaining(true);
    mono_set_signal_chaining(true);
    mono_config_parse(nullptr);
}

nanoem_u32_t
nanoemApplicationPluginModelGetABIVersion()
{
    return NANOEM_APPLICATION_PLUGIN_MODEL_ABI_VERSION;
}

nanoem_u32_t
nanoemApplicationPluginModelGetAPIVersion()
{
    return NANOEM_APPLICATION_PLUGIN_MODEL_API_VERSION;
}

nanoem_application_plugin_model_io_t *APIENTRY
nanoemApplicationPluginModelIOCreate(void)
{
    return nullptr;
}

nanoem_application_plugin_model_io_t *APIENTRY
nanoemApplicationPluginModelIOCreateWithLocation(const char *location)
{
    return new nanoem_application_plugin_model_io_t(location);
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
nanoemApplicationPluginModelIOSetFunction(
    nanoem_application_plugin_model_io_t *plugin, int index, nanoem_application_plugin_status_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setFunction(index);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetAudioDescription(nanoem_application_plugin_model_io_t *plugin, const nanoem_u8_t *data,
    nanoem_u32_t length, nanoem_application_plugin_status_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setAudioDescription(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetCameraDescription(nanoem_application_plugin_model_io_t *plugin,
    const nanoem_u8_t *data, nanoem_u32_t length, nanoem_application_plugin_status_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setCameraDescription(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetLightDescription(nanoem_application_plugin_model_io_t *plugin, const nanoem_u8_t *data,
    nanoem_u32_t length, nanoem_application_plugin_status_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setLightDescription(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetInputAudioData(nanoem_application_plugin_model_io_t *plugin, const nanoem_u8_t *data,
    nanoem_u32_t length, nanoem_application_plugin_status_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setInputAudio(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetInputModelData(nanoem_application_plugin_model_io_t *plugin, const nanoem_u8_t *data,
    nanoem_u32_t length, nanoem_application_plugin_status_t *status)
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
    nanoem_u32_t /* length */, nanoem_application_plugin_status_t *status)
{
    if (nanoem_is_not_null(plugin)) {
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetAllSelectedVertexObjectIndices(nanoem_application_plugin_model_io_t *plugin,
    const int *data, nanoem_u32_t length, nanoem_application_plugin_status_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setAllSelectedVertexObjectIndices(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetAllSelectedMaterialObjectIndices(nanoem_application_plugin_model_io_t *plugin,
    const int *data, nanoem_u32_t length, nanoem_application_plugin_status_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setAllSelectedMaterialObjectIndices(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetAllSelectedBoneObjectIndices(nanoem_application_plugin_model_io_t *plugin,
    const int *data, nanoem_u32_t length, nanoem_application_plugin_status_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setAllSelectedBoneObjectIndices(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetAllSelectedConstraintObjectIndices(nanoem_application_plugin_model_io_t *plugin,
    const int *data, nanoem_u32_t length, nanoem_application_plugin_status_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setAllSelectedConstraintObjectIndices(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetAllSelectedMorphObjectIndices(nanoem_application_plugin_model_io_t *plugin,
    const int *data, nanoem_u32_t length, nanoem_application_plugin_status_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setAllSelectedMorphObjectIndices(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetAllSelectedLabelObjectIndices(nanoem_application_plugin_model_io_t *plugin,
    const int *data, nanoem_u32_t length, nanoem_application_plugin_status_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setAllSelectedLabelObjectIndices(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetAllSelectedRigidBodyObjectIndices(nanoem_application_plugin_model_io_t *plugin,
    const int *data, nanoem_u32_t length, nanoem_application_plugin_status_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setAllSelectedRigidBodyObjectIndices(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetAllSelectedJointObjectIndices(nanoem_application_plugin_model_io_t *plugin,
    const int *data, nanoem_u32_t length, nanoem_application_plugin_status_t *status)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->setAllSelectedJointObjectIndices(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOExecute(
    nanoem_application_plugin_model_io_t *plugin, nanoem_application_plugin_status_t *status)
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
nanoemApplicationPluginModelIOGetOutputModelData(nanoem_application_plugin_model_io_t *plugin, nanoem_u8_t *data,
    nanoem_u32_t length, nanoem_application_plugin_status_t *status)
{
    if (nanoem_likely(plugin && data)) {
        plugin->getOutputData(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOLoadUIWindowLayout(
    nanoem_application_plugin_model_io_t *plugin, nanoem_application_plugin_status_t *status)
{
    if (nanoem_likely(plugin)) {
        plugin->loadUIWindowLayoutData();
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
nanoemApplicationPluginModelIOGetUIWindowLayoutData(nanoem_application_plugin_model_io_t *plugin, nanoem_u8_t *data,
    nanoem_u32_t length, nanoem_application_plugin_status_t *status)
{
    if (nanoem_likely(plugin && data)) {
        plugin->getUIWindowLayoutData(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetUIWindowLayoutData(nanoem_application_plugin_model_io_t *plugin,
    const nanoem_u8_t *data, nanoem_u32_t length, nanoem_application_plugin_status_t *status)
{
    if (nanoem_likely(plugin && data)) {
        plugin->setUIWindowLayoutData(data, length);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginModelIOSetUIComponentLayoutData(nanoem_application_plugin_model_io_t *plugin, const char *id,
    const nanoem_u8_t *data, nanoem_u32_t length, int *reloadLayout, nanoem_application_plugin_status_t *status)
{
    if (nanoem_likely(plugin && id && data && reloadLayout)) {
        plugin->setUIComponentData(id, data, length);
        *reloadLayout = 1;
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
