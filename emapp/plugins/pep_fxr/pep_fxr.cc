/*
   Copyright (c) 2015-2021 hkrn All rights reserved

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

#include "coreclr_delegates.h"
#include "hostfxr.h"
#include "nethost.h"

#if defined(_WIN32)
#define PATH_MAX MAX_PATH
#define _T(str) L(str)
#define reverseStringIndexOf(str, c) wcsrchr((str), L(c))
#else
#include <dlfcn.h>
#define _T(str) (str)
#define reverseStringIndexOf(str, c) strrchr((str), (c))
#endif

namespace {

using namespace nanoem::application::plugin;

class PEPluginFXR {
public:
    PEPluginFXR(const char *location)
    {
        mutable_string_t rootPath;
        if (loadHostFXR(location, rootPath)) {
            bool valid = true;
            resolveFunction("hostfxr_initialize_for_runtime_config", _hostfxr_initialize_for_runtime_config, valid);
            resolveFunction("hostfxr_get_runtime_delegate", _hostfxr_get_runtime_delegate, valid);
            resolveFunction("hostfxr_get_runtime_properties", _hostfxr_get_runtime_properties, valid);
            resolveFunction("hostfxr_get_runtime_property_value", _hostfxr_get_runtime_property_value, valid);
            resolveFunction("hostfxr_set_runtime_property_value", _hostfxr_set_runtime_property_value, valid);
            resolveFunction("hostfxr_set_error_writer", _hostfxr_set_error_writer_fn, valid);
            resolveFunction("hostfxr_close", _hostfxr_close, valid);
            if (valid) {
                const char *locationLastPathPtr = strrchr(location, '/');
                char_t *rootLastPathPtr = reverseStringIndexOf(rootPath.data(), '/');
                if (locationLastPathPtr && rootLastPathPtr) {
                    const string_t baseDirectoryPath(rootPath.data(), rootLastPathPtr);
                    m_managedDirectoryPath.assign(location, locationLastPathPtr);
                    m_managedDirectoryPath.append("/Managed");
                    m_userPluginDirectoryPath.assign(location, locationLastPathPtr);
                    m_userPluginDirectoryPath.append("/User");
                    string_t configPath(baseDirectoryPath);
                    configPath.append(_T("/Managed/runtimeconfig.json"));
                    _hostfxr_initialize_for_runtime_config(configPath.c_str(), nullptr, &m_hostFXRHandle);
                    if (m_hostFXRHandle) {
                        string_t dllPath(baseDirectoryPath);
                        dllPath.append(_T("/Managed/PEPlugin.dll"));
                        _hostfxr_set_error_writer_fn([](const char_t *message) { Inline::debugPrintf("%s", message); });
                        _hostfxr_get_runtime_delegate(m_hostFXRHandle, hdt_load_assembly_and_get_function_pointer,
                            reinterpret_cast<void **>(&_load_assembly_and_get_function_pointer));
                        resolvePluginFactoryMethod(dllPath, _T("Execute"), _PluginFactory_Execute);
                        resolvePluginFactoryMethod(dllPath, _T("FindAllPlugins"), _PluginFactory_FindAllPlugins);
                        if (_PluginFactory_FindAllPlugins) {
                            const FindAllPluginsArguments args = { this, errorCallback, findPluginCallback,
                                m_managedDirectoryPath.c_str(), Inline::saturateInt32(m_managedDirectoryPath.size()),
                                m_userPluginDirectoryPath.c_str(),
                                Inline::saturateInt32(m_userPluginDirectoryPath.size()) };
                            _PluginFactory_FindAllPlugins(&args, sizeof(args));
                        }
                    }
                }
            }
        }
    }
    ~PEPluginFXR()
    {
        if (m_hostFXRHandle) {
            _hostfxr_close(m_hostFXRHandle);
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
        m_allSelectedVertices.assign(data, data + length);
    }
    void
    setAllSelectedMaterialObjectIndices(const int *data, nanoem_u32_t length)
    {
        m_allSelectedMaterials.assign(data, data + length);
    }
    void
    setAllSelectedBoneObjectIndices(const int *data, nanoem_u32_t length)
    {
        m_allSelectedBones.assign(data, data + length);
    }
    void
    setAllSelectedConstraintObjectIndices(const int *data, nanoem_u32_t length)
    {
        m_allSelectedConstraints.assign(data, data + length);
    }
    void
    setAllSelectedMorphObjectIndices(const int *data, nanoem_u32_t length)
    {
        m_allSelectedMorphs.assign(data, data + length);
    }
    void
    setAllSelectedLabelObjectIndices(const int *data, nanoem_u32_t length)
    {
        m_allSelectedLabels.assign(data, data + length);
    }
    void
    setAllSelectedRigidBodyObjectIndices(const int *data, nanoem_u32_t length)
    {
        m_allSelectedRigidBodies.assign(data, data + length);
    }
    void
    setAllSelectedJointObjectIndices(const int *data, nanoem_u32_t length)
    {
        m_allSelectedJoints.assign(data, data + length);
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
    execute(nanoem_application_plugin_status_t *status)
    {
        if (_PluginFactory_Execute && m_functionIndex >= 0 && m_functionIndex < countAllFunctions()) {
            const std::string &assemblyPath = m_pluginAssemblies[m_functionIndex].second;
            ExecuteArguments args = { this, errorCallback, outputModelDataCallback, m_managedDirectoryPath.c_str(),
                Inline::saturateInt32(m_managedDirectoryPath.size()), m_userPluginDirectoryPath.c_str(),
                Inline::saturateInt32(m_userPluginDirectoryPath.size()), assemblyPath.data(),
                Inline::saturateInt32(assemblyPath.size()), m_allSelectedVertices.data(),
                Inline::saturateInt32U(m_allSelectedVertices.size()), m_allSelectedMaterials.data(),
                Inline::saturateInt32U(m_allSelectedMaterials.size()), m_allSelectedBones.data(),
                Inline::saturateInt32U(m_allSelectedBones.size()), m_allSelectedConstraints.data(),
                Inline::saturateInt32U(m_allSelectedConstraints.size()), m_allSelectedMorphs.data(),
                Inline::saturateInt32U(m_allSelectedMorphs.size()), m_allSelectedLabels.data(),
                Inline::saturateInt32U(m_allSelectedLabels.size()), m_allSelectedRigidBodies.data(),
                Inline::saturateInt32U(m_allSelectedRigidBodies.size()), m_allSelectedJoints.data(),
                Inline::saturateInt32U(m_allSelectedJoints.size()), m_inputData.data(),
                Inline::saturateInt32U(m_inputData.size()) };
            m_failureReason.clear();
            _PluginFactory_Execute(&args, Inline::saturateInt32U(sizeof(args)));
            if (!m_failureReason.empty()) {
                nanoem_application_plugin_status_assign_error(
                    status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_REFER_REASON);
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
    using ErrorCallback = void (*)(void *userData, const char *message);
    struct FindAllPluginsArguments {
        using FindPluginCallback = void (*)(void *userData, const char *name, const char *pluginPath);
        void *m_callbackUserData;
        ErrorCallback m_errorCallback;
        FindPluginCallback m_findPluginCallback;
        const char *m_managedDirectoryPath;
        int m_managedDirectoryPathLength;
        const char *m_userPluginDirectoryPath;
        int m_userPluginDirectoryPathLength;
    };
    struct ExecuteArguments {
        using OutputModelCallback = void (*)(void *userData, const nanoem_u8_t *data, nanoem_u32_t size);
        void *m_callbackUserData;
        ErrorCallback m_errorCallback;
        OutputModelCallback m_outputModelCallback;
        const char *m_managedDirectoryPath;
        int m_managedDirectoryPathLength;
        const char *m_userPluginDirectoryPath;
        int m_userPluginDirectoryPathLength;
        const char *m_assemblyPath;
        int m_assemblyPathLength;
        int *m_allSelectedVertexIndices;
        nanoem_u32_t m_allSelectedVertexIndicesLength;
        int *m_allSelectedMaterialIndices;
        nanoem_u32_t m_allSelectedMaterialIndicesLength;
        int *m_allSelectedBoneIndices;
        nanoem_u32_t m_allSelectedBoneIndicesLength;
        int *m_allSelectedConstraintIndices;
        nanoem_u32_t m_allSelectedConstraintIndicesLength;
        int *m_allSelectedMorphIndices;
        nanoem_u32_t m_allSelectedMorphIndicesLength;
        int *m_allSelectedLabelIndices;
        nanoem_u32_t m_allSelectedLabelIndicesLength;
        int *m_allSelectedRigidBodyIndices;
        nanoem_u32_t m_allSelectedRigidBodyIndicesLength;
        int *m_allSelectedJointIndices;
        nanoem_u32_t m_allSelectedJointIndicesLength;
        nanoem_u8_t *m_inputModelData;
        nanoem_u32_t m_inputModelDataSize;
    };

    using string_t = std::basic_string<char_t>;
    using mutable_string_t = std::vector<char_t>;
    using ByteArray = std::vector<nanoem_u8_t>;
    using ObjectIndexList = std::vector<int>;
    using PluginAssembly = std::pair<std::string, std::string>;
    using PluginAssemblyList = std::vector<PluginAssembly>;
    using pfn_PluginFactory_Execute = void (*)(const ExecuteArguments *, nanoem_u32_t);
    using pfn_PluginFactory_FindAllPlugins = void (*)(const FindAllPluginsArguments *, nanoem_u32_t);

    static void
    errorCallback(void *userData, const char *message)
    {
        auto self = static_cast<PEPluginFXR *>(userData);
        self->m_failureReason.assign(message);
    }
    static void
    findPluginCallback(void *userData, const char *name, const char *pluginPath)
    {
        auto self = static_cast<PEPluginFXR *>(userData);
        self->m_pluginAssemblies.push_back(std::make_pair(name, pluginPath));
    }
    static void
    outputModelDataCallback(void *userData, const nanoem_u8_t *data, nanoem_u32_t size)
    {
        auto self = static_cast<PEPluginFXR *>(userData);
        self->m_outputData.assign(data, data + size);
    }
    bool
    loadHostFXR(const char *location, mutable_string_t &rootPath)
    {
#if defined(_WIN32)
        const int locationLength = Inline::saturateInt32(strlen(location));
        rootPath.resize(MultiByteToWideChar(CP_UTF8, 0, location, locationLength, nullptr, 0));
        MultiByteToWideChar(
            CP_UTF8, 0, location, locationLength, rootPath.data(), Inline::saturateInt32(rootPath.size()));
        rootPath.push_back(0);
        mutable_string_t hostFXRPath;
        size_t hostFXRPathLength;
        get_hostfxr_path(nullptr, &hostFXRPathLength, nullptr);
        hostFXRPath.resize(hostFXRPathLength);
        get_hostfxr_path(hostFXRPath.data(), &hostFXRPathLength, nullptr);
        m_hostFXRHandle = LoadLibraryW(hostFXRPath.data());
#else
        rootPath.assign(location, location + strlen(location));
        rootPath.push_back(0);
        mutable_string_t hostFXRPath;
        size_t hostFXRPathLength;
        get_hostfxr_path(nullptr, &hostFXRPathLength, nullptr);
        hostFXRPath.resize(hostFXRPathLength);
        get_hostfxr_path(hostFXRPath.data(), &hostFXRPathLength, nullptr);
        m_hostFXRHandle = dlopen(hostFXRPath.data(), RTLD_LAZY);
#endif
        return m_hostFXRHandle != nullptr;
    }

    template <typename T>
    inline void
    resolveFunction(const char *name, T &fn, bool &valid)
    {
#if defined(_WIN32)
        fn = reinterpret_cast<T>(GetProcAddress(static_cast<HMODULE>(m_hostFXRHandle), name));
#else
        fn = reinterpret_cast<T>(dlsym(m_hostFXRHandle, name));
#endif
        valid &= fn != nullptr;
    }
    template <typename T>
    inline void
    resolvePluginFactoryMethod(const string_t &dllPath, const char_t *name, T &fn)
    {
        const char_t *dllType = _T("nanoem.Application.Plugin.PEPluginProxyHost.PluginFactoryFXR, PEPlugin");
        _load_assembly_and_get_function_pointer(
            dllPath.data(), dllType, name, nullptr, nullptr, reinterpret_cast<void **>(&fn));
    }

    PluginAssemblyList m_pluginAssemblies;
    ByteArray m_inputData;
    ByteArray m_outputData;
    ObjectIndexList m_allSelectedVertices;
    ObjectIndexList m_allSelectedMaterials;
    ObjectIndexList m_allSelectedConstraints;
    ObjectIndexList m_allSelectedBones;
    ObjectIndexList m_allSelectedMorphs;
    ObjectIndexList m_allSelectedLabels;
    ObjectIndexList m_allSelectedRigidBodies;
    ObjectIndexList m_allSelectedJoints;
    std::string m_failureReason;
    std::string m_recoverySuggestion;
    hostfxr_initialize_for_runtime_config_fn _hostfxr_initialize_for_runtime_config = nullptr;
    hostfxr_get_runtime_delegate_fn _hostfxr_get_runtime_delegate = nullptr;
    hostfxr_set_error_writer_fn _hostfxr_set_error_writer_fn = nullptr;
    hostfxr_get_runtime_properties_fn _hostfxr_get_runtime_properties = nullptr;
    hostfxr_get_runtime_property_value_fn _hostfxr_get_runtime_property_value = nullptr;
    hostfxr_set_runtime_property_value_fn _hostfxr_set_runtime_property_value = nullptr;
    load_assembly_and_get_function_pointer_fn _load_assembly_and_get_function_pointer = nullptr;
    hostfxr_close_fn _hostfxr_close = nullptr;
    pfn_PluginFactory_Execute _PluginFactory_Execute = nullptr;
    pfn_PluginFactory_FindAllPlugins _PluginFactory_FindAllPlugins = nullptr;
    std::string m_managedDirectoryPath;
    std::string m_userPluginDirectoryPath;
    void *m_hostFXRHandle = nullptr;
    int m_languageIndex = 0;
    int m_functionIndex = -1;
};

} /* namespace anonymous */

struct nanoem_application_plugin_model_io_t : PEPluginFXR {
    nanoem_application_plugin_model_io_t(const char *location)
        : PEPluginFXR(location)
    {
    }
};

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

void APIENTRY
nanoemApplicationPluginModelIOInitialize(void)
{
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
        plugin->execute(status);
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
