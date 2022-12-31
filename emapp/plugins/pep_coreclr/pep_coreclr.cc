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

#if defined(_WIN32)
#define PATH_MAX MAX_PATH
#define CORECLR_PATH_SEPARATOR ";"
#else
#include <dirent.h>
#include <dlfcn.h>
#include <unistd.h>
#define CORECLR_PATH_SEPARATOR ":"
#endif

namespace {

using namespace nanoem::application::plugin;

class PEPluginCoreCLR {
public:
    PEPluginCoreCLR(const char *location)
    {
        if (const char *locationLastPathPtr = strrchr(location, '/')) {
            const std::string baseDirectoryPath(location, locationLastPathPtr);
            m_managedDirectoryPath.assign(baseDirectoryPath);
            m_managedDirectoryPath.append("/Managed/");
            m_userPluginDirectoryPath.assign(baseDirectoryPath);
            m_userPluginDirectoryPath.append("/User/");
            std::string rootPath, version, netCoreAppPath;
            findDotnetRootPath(rootPath, version, netCoreAppPath);
            std::string coreCLRPath(netCoreAppPath);
#if defined(_WIN32)
            coreCLRPath.append("coreclr.dll");
            m_dllHandle = LoadLibraryExA(coreCLRPath.c_str(), nullptr, 0);
#else
#if defined(__APPLE__)
            coreCLRPath.append("libcoreclr.dylib");
#else
            coreCLRPath.append("libcoreclr.so");
#endif
            m_dllHandle = dlopen(coreCLRPath.data(), RTLD_LAZY);
#endif
            bool valid = true;
            resolveFunction("coreclr_initialize", _coreclr_initialize, valid);
            resolveFunction("coreclr_create_delegate", _coreclr_create_delegate, valid);
            resolveFunction("coreclr_shutdown", _coreclr_shutdown, valid);
            if (valid) {
                initializeCoreCLR(baseDirectoryPath, rootPath, version, netCoreAppPath);
                const char *assemblyName = "PEP";
                const char *typeName = "nanoem.PEP.PluginFactoryFXR";
                _coreclr_create_delegate(m_clrHandle, m_domainID, assemblyName, typeName, "FindAllPlugins",
                    reinterpret_cast<void **>(&_PluginFactory_FindAllPlugins));
                _coreclr_create_delegate(m_clrHandle, m_domainID, assemblyName, typeName, "Execute",
                    reinterpret_cast<void **>(&_PluginFactory_Execute));
                if (_PluginFactory_FindAllPlugins) {
                    const FindAllPluginsArguments args = { this, errorCallback, findPluginCallback,
                        m_managedDirectoryPath.c_str(), Inline::saturateInt32(m_managedDirectoryPath.size()),
                        m_userPluginDirectoryPath.c_str(), Inline::saturateInt32(m_userPluginDirectoryPath.size()) };
                    _PluginFactory_FindAllPlugins(&args, sizeof(args));
                    std::sort(m_pluginAssemblies.begin(), m_pluginAssemblies.end(),
                        [](const PluginAssembly &left, const PluginAssembly &right) {
                            return left.first.compare(right.first) < 0;
                        });
                }
            }
        }
    }
    ~PEPluginCoreCLR()
    {
        if (m_clrHandle) {
            _coreclr_shutdown(m_clrHandle, m_domainID);
            m_clrHandle = nullptr;
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
        const char *result = nullptr;
        if (index >= 0 && index < countAllFunctions()) {
            auto functionIndex = Inline::roundInt32(index);
            result = m_pluginAssemblies[functionIndex].first.c_str();
        }
        return result;
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
        if (m_functionIndex >= 0 && m_functionIndex < countAllFunctions() && _PluginFactory_Execute) {
            auto functionIndex = Inline::roundInt32(m_functionIndex);
            const std::string &assemblyPath = m_pluginAssemblies[functionIndex].second;
            ExecuteArguments args = { this, errorCallback, outputModelDataCallback, m_managedDirectoryPath.data(),
                Inline::saturateInt32(m_managedDirectoryPath.size()), m_userPluginDirectoryPath.data(),
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

    using pfn_coreclr_initialize = int (*)(const char *exePath, const char *appDomainFriendlyName, int propertyCount,
        const char **propertyKeys, const char **propertyValues, void **hostHandle, unsigned int *domainID);
    using pfn_coreclr_create_delegate = int (*)(void *hostHandle, unsigned int domainID,
        const char *entryPointAssemblyName, const char *entryPointTypeName, const char *entryPointMethodName,
        void **delegate);
    using pfn_coreclr_shutdown = int (*)(void *hostHandle, unsigned int domainID);
    using pfn_PluginFactory_Execute = void (*)(const ExecuteArguments *, nanoem_u32_t);
    using pfn_PluginFactory_FindAllPlugins = void (*)(const FindAllPluginsArguments *, nanoem_u32_t);

    using ByteArray = std::vector<nanoem_u8_t>;
    using ObjectIndexList = std::vector<int>;
    using PluginAssembly = std::pair<std::string, std::string>;
    using PluginAssemblyList = std::vector<PluginAssembly>;

    static void
    errorCallback(void *userData, const char *message)
    {
        auto self = static_cast<PEPluginCoreCLR *>(userData);
        self->m_failureReason.assign(message);
    }
    static void
    findPluginCallback(void *userData, const char *name, const char *pluginPath)
    {
        auto self = static_cast<PEPluginCoreCLR *>(userData);
        self->m_pluginAssemblies.push_back(std::make_pair(name, pluginPath));
    }
    static void
    outputModelDataCallback(void *userData, const nanoem_u8_t *data, nanoem_u32_t size)
    {
        auto self = static_cast<PEPluginCoreCLR *>(userData);
        self->m_outputData.assign(data, data + size);
    }
#if defined(_WIN32)
    static bool
    findRegistry(const char *path, const char *key, char *value, DWORD length)
    {
        HKEY result;
        bool found = false;
        if (RegOpenKeyA(HKEY_LOCAL_MACHINE, path, &result) == S_OK) {
            DWORD type;
            found = RegGetValueA(result, nullptr, key, RRF_RT_REG_SZ, &type, value, &length) == S_OK;
            RegCloseKey(result);
        }
        return found;
    }
#endif
    static void
    replaceFilePathSeparator(std::string &value)
    {
#if defined(_WIN32)
        std::replace(value.begin(), value.end(), '/', '\\');
#else
        nanoem_mark_unused(value);
#endif
    }
    static void
    findDotnetRootPath(std::string &rootPath, std::string &version, std::string &netCoreAppPath)
    {
#if defined(_WIN32)
        char tempDotnetCorePath[PATH_MAX], tempDotnetVersion[16];
        findRegistry("Software\\WOW6432Node\\dotnet\\Setup\\InstalledVersions\\x64", "InstallLocation",
            tempDotnetCorePath, sizeof(tempDotnetCorePath));
        findRegistry("Software\\WOW6432Node\\dotnet\\Setup\\InstalledVersions\\x64\\hostfxr", "Version",
            tempDotnetVersion, sizeof(tempDotnetVersion));
        rootPath.assign(tempDotnetCorePath);
        version.assign(tempDotnetVersion);
#else
        if (char *envPathValue = getenv("PATH")) {
            const char *p = envPathValue, *q;
            auto finder = [&rootPath, &version](const char *p, const char *q) {
                std::string candidatePath;
                q ? candidatePath.assign(p, q) : candidatePath.append(p);
                std::string coreclrPath(candidatePath);
                coreclrPath.append("/shared/Microsoft.NETCore.App/3.0.0/");
#if defined(__APPLE__)
                coreclrPath.append("libcoreclr.dylib");
#else
                coreclrPath.append("libcoreclr.so");
#endif
                if (access(coreclrPath.c_str(), F_OK) == 0) {
                    rootPath = candidatePath;
                    version = "3.0.0";
                    return true;
                }
                return false;
            };
            while ((q = strchr(p, ':')) != nullptr) {
                if (finder(p, q)) {
                    break;
                }
                p = q + 1;
            }
            finder(p, q);
        }
#endif
        std::replace(rootPath.begin(), rootPath.end(), '\\', '/');
        if (!rootPath.empty()) {
            netCoreAppPath.assign(rootPath);
            netCoreAppPath.append("/shared/Microsoft.NETCore.App/");
            netCoreAppPath.append(version);
            netCoreAppPath.append("/");
        }
    }
    static void
    findTrustedPlatformAssemblies(const char *directory, std::string &value)
    {
#if defined(_WIN32)
        std::string basePath(directory);
        basePath.append("/*.dll");
        WIN32_FIND_DATAA find;
        HANDLE handle = FindFirstFileA(basePath.c_str(), &find);
        if (handle != INVALID_HANDLE_VALUE) {
            do {
                value.append(directory);
                value.append("/");
                value.append(find.cFileName);
                value.append(CORECLR_PATH_SEPARATOR);
            } while (FindNextFileA(handle, &find));
            FindClose(handle);
            replaceFilePathSeparator(value);
        }
#else
        if (DIR *dir = opendir(directory)) {
            struct dirent *entry = nullptr;
            while ((entry = readdir(dir)) != nullptr) {
                const char *p = strrchr(entry->d_name, '.');
                if (StringUtils::equals(p, ".dll")) {
                    value.append(directory);
                    value.append("/");
                    value.append(entry->d_name, entry->d_namlen);
                    value.append(CORECLR_PATH_SEPARATOR);
                }
            }
            closedir(dir);
        }
#endif
    }
    template <typename T>
    inline void
    resolveFunction(const char *name, T &fn, bool &valid)
    {
        if (m_dllHandle) {
#if defined(_WIN32)
            fn = reinterpret_cast<T>(GetProcAddress(static_cast<HMODULE>(m_dllHandle), name));
#else
            fn = reinterpret_cast<T>(dlsym(m_dllHandle, name));
#endif
        }
        valid &= fn != nullptr;
    }

    void
    initializeCoreCLR(const std::string &baseDirectoryPath, const std::string &rootPath, const std::string &version,
        const std::string &netCoreAppPath)
    {
        static const char *kPropertyKeys[] = { "FX_PRODUCT_VERSION", "AppDomainCompatSwitch",
            "TRUSTED_PLATFORM_ASSEMBLIES", "NATIVE_DLL_SEARCH_DIRECTORIES", "APP_PATHS", "PLATFORM_RESOURCE_ROOTS",
            "FX_DEPS_FILE", "APP_CONTEXT_BASE_DIRECTORY", "APP_CONTEXT_DEPS_FILES", "JIT_PATH" };
        std::string windowsDesktopAppPath(rootPath);
        windowsDesktopAppPath.append("/shared/Microsoft.WindowsDesktop.App/");
        windowsDesktopAppPath.append(version);
        windowsDesktopAppPath.append("/");
        std::string fxDepsFile(netCoreAppPath);
        fxDepsFile.append("Microsoft.NETCore.App.deps.json");
        replaceFilePathSeparator(fxDepsFile);
        std::string nativeDllSearchDirectories;
        nativeDllSearchDirectories.append(windowsDesktopAppPath);
        nativeDllSearchDirectories.append(CORECLR_PATH_SEPARATOR);
        nativeDllSearchDirectories.append(netCoreAppPath);
        nativeDllSearchDirectories.append(CORECLR_PATH_SEPARATOR);
        nativeDllSearchDirectories.append(baseDirectoryPath);
        replaceFilePathSeparator(nativeDllSearchDirectories);
        std::string platformResourceRoots;
        platformResourceRoots.append(netCoreAppPath);
        replaceFilePathSeparator(platformResourceRoots);
        std::string appContextDepsFiles;
        appContextDepsFiles.append(windowsDesktopAppPath);
        appContextDepsFiles.append("Microsoft.WindowsDesktop.App.deps.json");
        appContextDepsFiles.append(CORECLR_PATH_SEPARATOR);
        appContextDepsFiles.append(netCoreAppPath);
        appContextDepsFiles.append("Microsoft.NETCore.App.deps.json");
        appContextDepsFiles.append(CORECLR_PATH_SEPARATOR);
        appContextDepsFiles.append(baseDirectoryPath);
        appContextDepsFiles.append("/PEPlugin.deps.json");
        replaceFilePathSeparator(appContextDepsFiles);
        std::string jitPath(netCoreAppPath);
#if defined(_WIN32)
        jitPath.append("clrjit.dll");
#elif defined(__APPLE__)
        jitPath.append("libclrjit.dylib");
#else
        jitPath.append("libclrjit.so");
#endif
        replaceFilePathSeparator(jitPath);
        std::string trustedPlatformAssemblies, appPaths;
        findTrustedPlatformAssemblies(netCoreAppPath.c_str(), trustedPlatformAssemblies);
        findTrustedPlatformAssemblies(windowsDesktopAppPath.c_str(), trustedPlatformAssemblies);
        findTrustedPlatformAssemblies(m_managedDirectoryPath.c_str(), trustedPlatformAssemblies);
        std::string userPluginDirectoryPath(m_userPluginDirectoryPath);
        replaceFilePathSeparator(userPluginDirectoryPath);
        const char *kPropertyValues[] = {
            /* FX_PRODUCT_VERSION */
            "3.0",
            /* AppDomainCompatSwitch */
            "UseLatestBehaviorWhenTFMNotSpecified",
            trustedPlatformAssemblies.c_str(),
            nativeDllSearchDirectories.c_str(),
            appPaths.c_str(),
            platformResourceRoots.c_str(),
            fxDepsFile.c_str(),
            userPluginDirectoryPath.c_str(),
            appContextDepsFiles.c_str(),
            jitPath.c_str(),
        };
        const size_t numProperties = sizeof(kPropertyKeys) / sizeof(kPropertyKeys[0]);
        _coreclr_initialize(userPluginDirectoryPath.c_str(), "nanoem", numProperties, kPropertyKeys, kPropertyValues,
            &m_clrHandle, &m_domainID);
    }

    pfn_coreclr_initialize _coreclr_initialize = nullptr;
    pfn_coreclr_create_delegate _coreclr_create_delegate = nullptr;
    pfn_coreclr_shutdown _coreclr_shutdown = nullptr;
    pfn_PluginFactory_Execute _PluginFactory_Execute = nullptr;
    pfn_PluginFactory_FindAllPlugins _PluginFactory_FindAllPlugins = nullptr;
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
    std::string m_managedDirectoryPath;
    std::string m_userPluginDirectoryPath;
    std::string m_failureReason;
    std::string m_recoverySuggestion;
    void *m_dllHandle = nullptr;
    void *m_clrHandle = nullptr;
    unsigned int m_domainID = 0;
    int m_languageIndex = 0;
    int m_functionIndex = -1;
};

} /* namespace anonymous */

struct nanoem_application_plugin_model_io_t : PEPluginCoreCLR {
    nanoem_application_plugin_model_io_t(const char *location)
        : PEPluginCoreCLR(location)
    {
    }
};

nanoem_u32_t
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
