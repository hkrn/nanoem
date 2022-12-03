/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/emapp.h"
#include "emapp/integration/IExecutor.h"

#include "bx/commandline.h"
#include "nanoem/ext/document.h"
#include "sokol/sokol_time.h"

#include "glm/gtc/type_ptr.hpp"
#include "spdlog/spdlog.h"

/* force using logging with NANOEM_ENABLE_LOGGING */
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace test {
namespace {

static URI
concatFileURI(const String &documentPathPrefix, const char *appendPath)
{
    String newPath(documentPathPrefix), canonicalizedPath;
    newPath.append("/");
    newPath.append(appendPath);
    FileUtils::canonicalizePathSeparator(newPath, canonicalizedPath);
    return URI::createFromFilePath(canonicalizedPath);
}

} /* namespace anonymous */

class LoadPMMExecutor : public IExecutor, private NonCopyable {
public:
    static const nanoem_u32_t kPeriodicallySaveSeconds;
    static const ApplicationMenuBuilder::MenuItemType kPhysicsSimulationMode;

    LoadPMMExecutor(const URI &fileURI, const bx::CommandLine *cmd, ThreadedApplicationService *service,
        ThreadedApplicationClient *client, bx::Mutex *eventLock);
    ~LoadPMMExecutor();

    void start() NANOEM_DECL_OVERRIDE;
    void finish() NANOEM_DECL_OVERRIDE;
    void setUndoFactor(nanoem_f32_t value);

private:
    typedef tinystl::unordered_map<String, nanoem_u16_t, TinySTLAllocator> AccessoryHandleMap;
    typedef tinystl::unordered_map<String, nanoem_u16_t, TinySTLAllocator> ModelHandleMap;

    int run();
    void registerEventHandlers();
    URI resolveFilePath(const nanoem_unicode_string_t *path, const String &documentPathPrefix);
    nanoem_u16_t findAccessoryHandle(const nanoem_document_accessory_t *value) const;
    nanoem_u16_t findModelHandle(const nanoem_document_model_t *value) const;

    void sendDocument(const nanoem_document_t *document, const String &documentPath);
    void sendAllCameraKeyframes(const nanoem_document_t *document);
    void sendAllLightKeyframes(const nanoem_document_t *document);
    void sendAllAccessoryKeyframes(const nanoem_document_accessory_t *accessory);
    void sendAllBoneKeyframes(const nanoem_document_model_t *model);
    void sendAllModelKeyframes(const nanoem_document_model_t *model);
    void sendAllMorphKeyframes(const nanoem_document_model_t *model);
    void sendAllOutsideParents(const nanoem_document_t *document);
    void sendUndoAction();
    void sendSaveAction();
    void sendSaveActionPeriodically();
    void waitForAction(bool skippable);

    const URI m_fileURI;
    const bx::CommandLine *m_commandLine;
    ThreadedApplicationService *m_service;
    ThreadedApplicationClient *m_client;
    bx::Mutex *m_eventLock;
    bx::Thread m_thread;
    bx::Semaphore m_consumeFrameSema;
    bx::Semaphore m_setActiveModelSema;
    bx::Semaphore m_savingProjectSema;
    tinystl::pair<bx::Semaphore, String> m_loadAccessorySema;
    tinystl::pair<bx::Semaphore, String> m_loadModelSema;
    AccessoryHandleMap m_accessory2HandleMap;
    ModelHandleMap m_model2HandleMap;
    nanoem_u64_t m_lastTick;
    bx::RngShr3 m_rng;
    nanoem_f32_t m_undoFactor;
    nanoem_u32_t m_saveSecondsInterval;
};

const nanoem_u32_t LoadPMMExecutor::kPeriodicallySaveSeconds = 5;
const ApplicationMenuBuilder::MenuItemType LoadPMMExecutor::kPhysicsSimulationMode =
    ApplicationMenuBuilder::kMenuItemTypeProjectPhysicsSimulationEnablePlaying;

LoadPMMExecutor::LoadPMMExecutor(const URI &fileURI, const bx::CommandLine *cmd, ThreadedApplicationService *service,
    ThreadedApplicationClient *client, bx::Mutex *eventLock)
    : m_fileURI(fileURI)
    , m_commandLine(cmd)
    , m_service(service)
    , m_client(client)
    , m_eventLock(eventLock)
    , m_lastTick(stm_now())
    , m_rng(nanoem_u32_t(stm_ms(m_lastTick)))
    , m_undoFactor(0.1f)
    , m_saveSecondsInterval(
          cmd->hasArg("seconds") ? strtol(cmd->findOption("seconds"), nullptr, 10) : kPeriodicallySaveSeconds)
{
    registerEventHandlers();
}

LoadPMMExecutor::~LoadPMMExecutor()
{
}

void
LoadPMMExecutor::start()
{
    m_thread.init(
        [](bx::Thread *thread, void *userData) {
            LoadPMMExecutor *self = static_cast<LoadPMMExecutor *>(userData);
            thread->setThreadName("com.github.nanoem.macos.test.LoadPMMExecutor");
            return self->run();
        },
        this);
}

void
LoadPMMExecutor::finish()
{
    m_thread.shutdown();
}

void
LoadPMMExecutor::setUndoFactor(nanoem_f32_t value)
{
    m_undoFactor = value;
}

int
LoadPMMExecutor::run()
{
    const char *path = m_commandLine->findOption("pmm", "input.pmm");
    FileReaderScope scope(m_service->translator());
    Error error;
    if (scope.open(URI::createFromFilePath(path), error)) {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        ByteArray bytes;
        FileUtils::read(scope, bytes, error);
        nanoem_unicode_string_factory_t *factory = m_service->projectHolder()->currentProject()->unicodeStringFactory();
        nanoem_buffer_t *buffer = nanoemBufferCreate(bytes.data(), bytes.size(), &status);
        nanoem_document_t *document = nanoemDocumentCreate(factory, &status);
        if (nanoemDocumentLoadFromBuffer(document, buffer, &status)) {
            String canonicalizedPath;
            FileUtils::canonicalizePathSeparator(path, canonicalizedPath);
            sendDocument(document, canonicalizedPath);
        }
        nanoemDocumentDestroy(document);
        nanoemBufferDestroy(buffer);
    }
    else {
        EMLOG_WARN("Cannot load PMM file: {}", path);
    }
    {
        bx::MutexScope scope(*m_eventLock);
        BX_UNUSED_1(scope);
        m_client->sendDestroyMessage();
        EMLOG_INFO("Destroy message has been sent: lock={}", static_cast<const void *>(m_eventLock));
    }
    return 0;
}

void
LoadPMMExecutor::registerEventHandlers()
{
    m_client->addAddAccessoryEventListener(
        [](void *userData, nanoem_u16_t handle, const char *name) {
            LoadPMMExecutor *self = static_cast<LoadPMMExecutor *>(userData);
            if (self->m_accessory2HandleMap.insert(tinystl::make_pair(String(name), handle)).second) {
                EMLOG_INFO("{} is already registered", name);
            }
            self->m_loadAccessorySema.first.post();
        },
        this, false);
    m_client->addAddModelEventListener(
        [](void *userData, nanoem_u16_t handle, const char *name) {
            LoadPMMExecutor *self = static_cast<LoadPMMExecutor *>(userData);
            if (self->m_model2HandleMap.insert(tinystl::make_pair(String(name), handle)).second) {
                EMLOG_INFO("{} is already registered", name);
            }
            self->m_loadModelSema.first.post();
        },
        this, false);
    m_client->addSetActiveModelEventListener(
        [](void *userData, nanoem_u16_t /* handle */, const char * /* name */) {
            LoadPMMExecutor *self = static_cast<LoadPMMExecutor *>(userData);
            self->m_setActiveModelSema.post();
        },
        this, false);
    m_client->addConsumePassEventListener(
        [](void *userData, nanoem_u64_t /* globalFrameIndex */) {
            LoadPMMExecutor *self = static_cast<LoadPMMExecutor *>(userData);
            self->m_consumeFrameSema.post();
        },
        this, false);
}

URI
LoadPMMExecutor::resolveFilePath(const nanoem_unicode_string_t *path, const String &documentPathPrefix)
{
    static const char kUserFileNeedleType1[] = "/UserFile/";
    static const char kUserFileNeedleType2[] = "UserFile/";
    String filePath, canonicalizedPath;
    StringUtils::getUtf8String(path, m_service->projectHolder()->currentProject()->unicodeStringFactory(), filePath);
    FileUtils::canonicalizePathSeparator(filePath, canonicalizedPath);
    URI fileURI;
    if (FileUtils::exists(canonicalizedPath.c_str())) {
        fileURI = URI::createFromFilePath(canonicalizedPath);
    }
    if (fileURI.isEmpty()) {
        if (const char *needle = m_commandLine->findOption("pmm-needle")) {
            const bx::StringView view(bx::strFind(canonicalizedPath.c_str(), needle));
            if (!view.isEmpty()) {
                fileURI = concatFileURI(documentPathPrefix, view.getTerm());
            }
        }
    }
    if (fileURI.isEmpty() &&
        bx::strCmp(canonicalizedPath.c_str(), kUserFileNeedleType2, sizeof(kUserFileNeedleType2) - 1) == 0) {
        fileURI = concatFileURI(documentPathPrefix, canonicalizedPath.c_str() + sizeof(kUserFileNeedleType2) - 1);
    }
    if (fileURI.isEmpty()) {
        const bx::StringView view(bx::strFind(canonicalizedPath.c_str(), kUserFileNeedleType1));
        if (!view.isEmpty()) {
            fileURI = concatFileURI(documentPathPrefix, view.getTerm());
        }
    }
    return fileURI;
}

nanoem_u16_t
LoadPMMExecutor::findAccessoryHandle(const nanoem_document_accessory_t *value) const
{
    String name;
    nanoem_unicode_string_factory_t *factory = m_service->projectHolder()->currentProject()->unicodeStringFactory();
    StringUtils::getUtf8String(nanoemDocumentAccessoryGetName(value), factory, name);
    AccessoryHandleMap::const_iterator it = m_accessory2HandleMap.find(name);
    return it != m_accessory2HandleMap.end() ? it->second : 0;
}

nanoem_u16_t
LoadPMMExecutor::findModelHandle(const nanoem_document_model_t *value) const
{
    String name;
    nanoem_unicode_string_factory_t *factory = m_service->projectHolder()->currentProject()->unicodeStringFactory();
    StringUtils::getUtf8String(nanoemDocumentModelGetName(value, NANOEM_LANGUAGE_TYPE_FIRST_ENUM), factory, name);
    ModelHandleMap::const_iterator it = m_model2HandleMap.find(name);
    return it != m_model2HandleMap.end() ? it->second : 0;
}

void
LoadPMMExecutor::sendDocument(const nanoem_document_t *document, const String &documentPath)
{
    const String doucmentPathPrefix(URI::stringByDeletingLastPathComponent(documentPath));
    {
        bx::MutexScope scope(*m_eventLock);
        BX_UNUSED_1(scope);
        m_client->sendMenuActionMessage(ApplicationMenuBuilder::kMenuItemTypeProjectEnableEffect);
        m_client->sendMenuActionMessage(kPhysicsSimulationMode);
    }
    nanoem_rsize_t numAccessories;
    nanoem_unicode_string_factory_t *factory = m_service->projectHolder()->currentProject()->unicodeStringFactory();
    nanoem_document_accessory_t *const *accessorys = nanoemDocumentGetAllAccessoryObjects(document, &numAccessories);
    for (nanoem_rsize_t i = 0; i < numAccessories; i++) {
        const nanoem_document_accessory_t *accessory = accessorys[i];
        const URI &fileURI = resolveFilePath(nanoemDocumentAccessoryGetPath(accessory), doucmentPathPrefix);
        if (Accessory::isLoadableExtension(fileURI) && FileUtils::exists(fileURI)) {
            StringUtils::getUtf8String(nanoemDocumentAccessoryGetName(accessory), factory, m_loadAccessorySema.second);
            {
                bx::MutexScope scope(*m_eventLock);
                BX_UNUSED_1(scope);
                m_client->sendLoadFileMessage(fileURI, IFileManager::kDialogTypeLoadModelFile);
            }
            m_loadAccessorySema.first.wait();
            sendAllAccessoryKeyframes(accessory);
        }
        else {
            EMLOG_WARN("Cannot load accessory and will be skipped: {}", fileURI.absolutePathConstString());
        }
    }
    sendAllCameraKeyframes(document);
    sendAllLightKeyframes(document);
    nanoem_rsize_t numModels;
    nanoem_document_model_t *const *models = nanoemDocumentGetAllModelObjects(document, &numModels);
    for (nanoem_rsize_t i = 0; i < numModels; i++) {
        const nanoem_document_model_t *model = models[i];
        const URI &fileURI = resolveFilePath(nanoemDocumentModelGetPath(model), doucmentPathPrefix);
        if (Model::isLoadableExtension(fileURI) && FileUtils::exists(fileURI)) {
            StringUtils::getUtf8String(
                nanoemDocumentModelGetName(model, NANOEM_LANGUAGE_TYPE_FIRST_ENUM), factory, m_loadModelSema.second);
            {
                bx::MutexScope scope(*m_eventLock);
                BX_UNUSED_1(scope);
                m_client->sendLoadFileMessage(fileURI, IFileManager::kDialogTypeLoadModelFile);
            }
            m_loadModelSema.first.wait();
            ModelHandleMap::const_iterator it = m_model2HandleMap.find(m_loadModelSema.second);
            if (it != m_model2HandleMap.end()) {
                nanoem_u16_t handle = it->second;
                bx::MutexScope scope(*m_eventLock);
                BX_UNUSED_1(scope);
                m_client->sendSetActiveModelMessage(handle);
            }
            m_setActiveModelSema.wait();
            sendAllBoneKeyframes(model);
            sendAllModelKeyframes(model);
            sendAllMorphKeyframes(model);
        }
        else {
            EMLOG_WARN("Cannot load model and will be skipped: {}", fileURI.absolutePathConstString());
        }
    }
    sendAllOutsideParents(document);
    if (const nanoem_unicode_string_t *path = nanoemDocumentGetAudioPath(document)) {
        const URI &fileURI = resolveFilePath(path, doucmentPathPrefix);
        if (FileUtils::exists(fileURI)) {
            bx::MutexScope scope(*m_eventLock);
            BX_UNUSED_1(scope);
            m_client->sendLoadFileMessage(fileURI, IFileManager::kDialogTypeOpenAudioFile);
        }
        else {
            EMLOG_WARN("Cannot load audio and will be skipped: {}", fileURI.absolutePathConstString());
        }
    }
    if (const nanoem_unicode_string_t *path = nanoemDocumentGetBackgroundVideoPath(document)) {
        const URI &fileURI = resolveFilePath(path, doucmentPathPrefix);
        if (FileUtils::exists(fileURI)) {
            bx::MutexScope scope(*m_eventLock);
            BX_UNUSED_1(scope);
            m_client->sendLoadFileMessage(fileURI, IFileManager::kDialogTypeOpenVideoFile);
        }
        else {
            EMLOG_WARN("Cannot load background video and will be skipped: {}", fileURI.absolutePathConstString());
        }
    }
    else if (const nanoem_unicode_string_t *path = nanoemDocumentGetBackgroundImagePath(document)) {
        const URI &fileURI = resolveFilePath(path, doucmentPathPrefix);
        if (FileUtils::exists(fileURI)) {
            bx::MutexScope scope(*m_eventLock);
            BX_UNUSED_1(scope);
            m_client->sendLoadFileMessage(fileURI, IFileManager::kDialogTypeOpenVideoFile);
        }
        else {
            EMLOG_WARN("Cannot load background image and will be skipped: {}", fileURI.absolutePathConstString());
        }
    }
    {
        bx::MutexScope scope(*m_eventLock);
        BX_UNUSED_1(scope);
        m_client->sendMenuActionMessage(ApplicationMenuBuilder::kMenuItemTypeProjectPhysicsSimulationEnableAnytime);
    }
    sendSaveAction();
    EMLOG_INFO("All operations are marked as completed: path={}", m_fileURI.absolutePathConstString());
}

void
LoadPMMExecutor::sendAllCameraKeyframes(const nanoem_document_t *document)
{
    nanoem_rsize_t numKeyframes;
    const nanoem_document_camera_t *camera = nanoemDocumentGetCameraObject(document);
    nanoem_document_camera_keyframe_t *const *keyframes =
        nanoemDocumentCameraGetAllCameraKeyframeObjects(camera, &numKeyframes);
    int currentPercent = 0, lastPercent = 0;
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        const nanoem_document_camera_keyframe_t *keyframe = keyframes[i];
        const nanoem_document_base_keyframe_t *base = nanoemDocumentCameraKeyframeGetBaseKeyframeObject(keyframe);
        nanoem_frame_index_t frameIndex = nanoemDocumentBaseKeyframeGetFrameIndex(base);
        m_client->sendSeekMessage(frameIndex);
        m_client->sendSetCameraAngleMessage(
            glm::make_vec3(nanoemDocumentCameraKeyframeGetAngle(keyframe)) * Vector3(-1, 1, 1), true);
        m_client->sendSetCameraDistanceMessage(-nanoemDocumentCameraKeyframeGetDistance(keyframe), true);
        m_client->sendSetCameraFovMessage(nanoemDocumentCameraKeyframeGetFov(keyframe), true);
        m_client->sendSetCameraLookAtMessage(glm::make_vec3(nanoemDocumentCameraKeyframeGetLookAt(keyframe)), true);
        const glm::u8vec4 values[] = {
            glm::make_vec4(nanoemDocumentCameraKeyframeGetInterpolation(
                keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_X)),
            glm::make_vec4(nanoemDocumentCameraKeyframeGetInterpolation(
                keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Y)),
            glm::make_vec4(nanoemDocumentCameraKeyframeGetInterpolation(
                keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Z)),
            glm::make_vec4(nanoemDocumentCameraKeyframeGetInterpolation(
                keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_ANGLE)),
            glm::make_vec4(nanoemDocumentCameraKeyframeGetInterpolation(
                keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FOV)),
            glm::make_vec4(nanoemDocumentCameraKeyframeGetInterpolation(
                keyframe, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_DISTANCE)),
        };
        m_client->sendSetCameraKeyframeInterpolationMessage(values);
        m_client->sendRegisterCameraKeyframeMessage();
        sendUndoAction();
        currentPercent = int((i / nanoem_f64_t(numKeyframes)) * 100.0);
        if (lastPercent != currentPercent) {
            EMLOG_INFO("Registering camera keyframes: progress={}% i={} keyframes={}", currentPercent, i, numKeyframes);
            lastPercent = currentPercent;
        }
    }
    sendSaveAction();
    EMLOG_INFO("Complete registering all camera keyframes: keyframes={}", numKeyframes);
}

void
LoadPMMExecutor::sendAllLightKeyframes(const nanoem_document_t *document)
{
    nanoem_rsize_t numKeyframes;
    const nanoem_document_light_t *light = nanoemDocumentGetLightObject(document);
    nanoem_document_light_keyframe_t *const *keyframes =
        nanoemDocumentLightGetAllLightKeyframeObjects(light, &numKeyframes);
    int currentPercent = 0, lastPercent = 0;
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        const nanoem_document_light_keyframe_t *keyframe = keyframes[i];
        const nanoem_document_base_keyframe_t *base = nanoemDocumentLightKeyframeGetBaseKeyframeObject(keyframe);
        nanoem_frame_index_t frameIndex = nanoemDocumentBaseKeyframeGetFrameIndex(base);
        m_client->sendSeekMessage(frameIndex);
        m_client->sendSetLightColorMessage(glm::make_vec3(nanoemDocumentLightKeyframeGetColor(keyframe)), true);
        m_client->sendSetLightDirectionMessage(glm::make_vec3(nanoemDocumentLightKeyframeGetDirection(keyframe)), true);
        m_client->sendRegisterLightKeyframeMessage();
        sendUndoAction();
        currentPercent = int((i / nanoem_f64_t(numKeyframes)) * 100.0);
        if (lastPercent != currentPercent) {
            EMLOG_INFO("Registering light keyframes: progress={}% i={} keyframes={}", currentPercent, i, numKeyframes);
            lastPercent = currentPercent;
        }
    }
    sendSaveAction();
    EMLOG_INFO("Complete registering all light keyframes: keyframes={}", numKeyframes);
}

void
LoadPMMExecutor::sendAllAccessoryKeyframes(const nanoem_document_accessory_t *accessory)
{
    nanoem_rsize_t numKeyframes;
    nanoem_document_accessory_keyframe_t *const *keyframes =
        nanoemDocumentAccessoryGetAllAccessoryKeyframeObjects(accessory, &numKeyframes);
    String parentModelNameString, parentBoneNameString;
    int currentPercent = 0, lastPercent = 0;
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        const nanoem_document_accessory_keyframe_t *keyframe = keyframes[i];
        const nanoem_document_base_keyframe_t *base = nanoemDocumentAccessoryKeyframeGetBaseKeyframeObject(keyframe);
        nanoem_frame_index_t frameIndex = nanoemDocumentBaseKeyframeGetFrameIndex(base);
        m_client->sendSeekMessage(frameIndex);
        nanoem_u16_t handle = findAccessoryHandle(accessory);
        m_client->sendSetAccessoryTranslationMessage(
            handle, glm::make_vec3(nanoemDocumentAccessoryKeyframeGetTranslation(keyframe)), true);
        m_client->sendSetAccessoryOrientationMessage(
            handle, glm::make_vec3(nanoemDocumentAccessoryKeyframeGetOrientation(keyframe)), true);
        m_client->sendSetAccessoryOpacityMessage(handle, nanoemDocumentAccessoryKeyframeGetOpacity(keyframe), true);
        m_client->sendSetAccessoryScaleFactorMessage(
            handle, nanoemDocumentAccessoryKeyframeGetScaleFactor(keyframe), true);
        m_client->sendSetAccessoryAddBlendEnabledMessage(
            handle, nanoemDocumentAccessoryIsAddBlendEnabled(accessory) != 0);
        m_client->sendSetAccessoryShadowEnabledMessage(
            handle, nanoemDocumentAccessoryKeyframeIsShadowEnabled(keyframe) != 0);
        m_client->sendSetAccessoryVisibleMessage(handle, nanoemDocumentAccessoryKeyframeIsVisible(keyframe) != 0);
        m_client->sendRegisterAccessoryKeyframeMessage(handle);
        sendUndoAction();
        currentPercent = int((i / nanoem_f64_t(numKeyframes)) * 100.0);
        if (lastPercent != currentPercent) {
            EMLOG_INFO(
                "Registering accessory keyframes: progress={}% i={} keyframes={}", currentPercent, i, numKeyframes);
            lastPercent = currentPercent;
        }
    }
    nanoem_u16_t handle = findAccessoryHandle(accessory);
    m_client->sendSetDrawableOrderIndexMessage(handle, nanoemDocumentAccessoryGetDrawOrderIndex(accessory));
    sendSaveAction();
    EMLOG_INFO("Complete registering all accessory keyframes: keyframes={}", numKeyframes);
}

void
LoadPMMExecutor::sendAllBoneKeyframes(const nanoem_document_model_t *model)
{
    nanoem_rsize_t numKeyframes;
    nanoem_document_model_bone_keyframe_t *const *keyframes =
        nanoemDocumentModelGetAllBoneKeyframeObjects(model, &numKeyframes);
    nanoem_unicode_string_factory_t *factory = m_service->projectHolder()->currentProject()->unicodeStringFactory();
    StringList nameList;
    String nameString;
    int currentPercent = 0, lastPercent = 0;
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        const nanoem_document_model_bone_keyframe_t *keyframe = keyframes[i];
        const nanoem_document_base_keyframe_t *base = nanoemDocumentModelBoneKeyframeGetBaseKeyframeObject(keyframe);
        const nanoem_unicode_string_t *name = nanoemDocumentModelBoneKeyframeGetName(keyframe);
        const Vector4 &translation = glm::make_vec4(nanoemDocumentModelBoneKeyframeGetTranslation(keyframe));
        const Quaternion &orientation = glm::make_quat(nanoemDocumentModelBoneKeyframeGetOrientation(keyframe));
        nanoem_frame_index_t frameIndex = nanoemDocumentBaseKeyframeGetFrameIndex(base);
        StringUtils::getUtf8String(name, factory, nameString);
        nameList.clear();
        nameList.push_back(nameString);
        nanoem_u16_t handle = findModelHandle(model);
        m_client->sendClearSelectAllBonesMessage(handle);
        m_client->sendSeekMessage(frameIndex);
        m_client->sendSetActiveModelBoneMessage(nameString.c_str());
        waitForAction(true);
        m_client->sendSetModelBoneTranslationMessage(handle, nameString.c_str(), translation, true);
        waitForAction(true);
        m_client->sendSetModelBoneOrientationMessage(handle, nameString.c_str(), orientation, true);
        waitForAction(true);
        const glm::u8vec4 values[] = {
            glm::make_vec4(nanoemDocumentModelBoneKeyframeGetInterpolation(
                keyframe, NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_X)),
            glm::make_vec4(nanoemDocumentModelBoneKeyframeGetInterpolation(
                keyframe, NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Y)),
            glm::make_vec4(nanoemDocumentModelBoneKeyframeGetInterpolation(
                keyframe, NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Z)),
            glm::make_vec4(nanoemDocumentModelBoneKeyframeGetInterpolation(
                keyframe, NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_ORIENTATION)),
        };
        m_client->sendSetModelBoneKeyframeInterpolationMessage(handle, values);
        m_client->sendRegisterAllSelectedBoneKeyframesMessage(handle, nameList);
        sendUndoAction();
        currentPercent = int((i / nanoem_f64_t(numKeyframes)) * 100.0);
        if (lastPercent != currentPercent) {
            EMLOG_INFO("Registering bone keyframes: progress={}% i={} keyframes={}", currentPercent, i, numKeyframes);
            lastPercent = currentPercent;
        }
    }
    sendSaveAction();
    EMLOG_INFO("Complete registering all bone keyframes: keyframes={}", numKeyframes);
}

void
LoadPMMExecutor::sendAllModelKeyframes(const nanoem_document_model_t *model)
{
    nanoem_rsize_t numKeyframes;
    nanoem_document_model_keyframe_t *const *keyframes =
        nanoemDocumentModelGetAllModelKeyframeObjects(model, &numKeyframes);
    String boneNameString, parentBoneNameString, parentModelNameString;
    int currentPercent = 0, lastPercent = 0;
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        const nanoem_document_model_keyframe_t *keyframe = keyframes[i];
        const nanoem_document_base_keyframe_t *base = nanoemDocumentModelKeyframeGetBaseKeyframeObject(keyframe);
        nanoem_frame_index_t frameIndex = nanoemDocumentBaseKeyframeGetFrameIndex(base);
        m_client->sendSeekMessage(frameIndex);
        nanoem_u16_t handle = findModelHandle(model);
        m_client->sendSetModelVisibleMessage(handle, nanoemDocumentModelKeyframeIsVisible(keyframe) != 0);
        m_client->sendSetModelEdgeSizeMessage(handle, nanoemDocumentModelGetEdgeWidth(model));
        m_client->sendSetModelAddBlendEnabledMessage(handle, nanoemDocumentModelIsBlendEnabled(model) != 0);
        m_client->sendSetModelShadowMapEnabledMessage(handle, nanoemDocumentModelIsSelfShadowEnabled(model) != 0);
        m_client->sendRegisterModelKeyframeMessage(handle);
        sendUndoAction();
        currentPercent = int((i / nanoem_f64_t(numKeyframes)) * 100.0);
        if (lastPercent != currentPercent) {
            EMLOG_INFO("Registering model keyframes: progress={}% i={} keyframes={}", currentPercent, i, numKeyframes);
            lastPercent = currentPercent;
        }
    }
    nanoem_u16_t handle = findModelHandle(model);
    m_client->sendSetDrawableOrderIndexMessage(handle, nanoemDocumentModelGetTransformOrderIndex(model));
    m_client->sendSetModelTransformOrderIndexMessage(handle, nanoemDocumentModelGetTransformOrderIndex(model));
    sendSaveAction();
    EMLOG_INFO("Complete registering all model keyframes: keyframes={}", numKeyframes);
}

void
LoadPMMExecutor::sendAllMorphKeyframes(const nanoem_document_model_t *model)
{
    nanoem_rsize_t numKeyframes;
    nanoem_document_model_morph_keyframe_t *const *keyframes =
        nanoemDocumentModelGetAllMorphKeyframeObjects(model, &numKeyframes);
    nanoem_unicode_string_factory_t *factory = m_service->projectHolder()->currentProject()->unicodeStringFactory();
    StringList nameList;
    String nameString;
    int currentPercent = 0, lastPercent = 0;
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        const nanoem_document_model_morph_keyframe_t *keyframe = keyframes[i];
        const nanoem_document_base_keyframe_t *base = nanoemDocumentModelMorphKeyframeGetBaseKeyframeObject(keyframe);
        const nanoem_unicode_string_t *name = nanoemDocumentModelMorphKeyframeGetName(keyframe);
        nanoem_frame_index_t frameIndex = nanoemDocumentBaseKeyframeGetFrameIndex(base);
        nanoem_f32_t weight = nanoemDocumentModelMorphKeyframeGetWeight(keyframe);
        StringUtils::getUtf8String(name, factory, nameString);
        nameList.clear();
        nameList.push_back(nameString);
        m_client->sendSeekMessage(frameIndex);
        m_client->sendSetActiveModelMorphMessage(nameString.c_str(), true);
        nanoem_u16_t handle = findModelHandle(model);
        waitForAction(true);
        m_client->sendSetModelMorphWeightMessage(handle, nameString.c_str(), weight, true);
        waitForAction(true);
        m_client->sendRegisterAllSelectedMorphKeyframesMessage(handle, nameList);
        sendUndoAction();
        currentPercent = int((i / nanoem_f64_t(numKeyframes)) * 100.0);
        if (lastPercent != currentPercent) {
            EMLOG_INFO("Registering morph keyframes: progress={}% i={} keyframes={}", currentPercent, i, numKeyframes);
            lastPercent = currentPercent;
        }
    }
    sendSaveAction();
    EMLOG_INFO("Complete registering all morph keyframes: keyframes={}", numKeyframes);
}

void
LoadPMMExecutor::sendAllOutsideParents(const nanoem_document_t *document)
{
    String name;
    nanoem_rsize_t numModels, numAccessories, numOutsideParents, numKeyframes;
    nanoem_unicode_string_factory_t *factory = m_service->projectHolder()->currentProject()->unicodeStringFactory();
    nanoem_document_accessory_t *const *accessorys = nanoemDocumentGetAllAccessoryObjects(document, &numAccessories);
    for (nanoem_rsize_t i = 0; i < numAccessories; i++) {
        const nanoem_document_accessory_t *accessory = accessorys[i];
        StringUtils::getUtf8String(nanoemDocumentAccessoryGetName(accessory), factory, name);
        AccessoryHandleMap::const_iterator it = m_accessory2HandleMap.find(name);
        if (it != m_accessory2HandleMap.end()) {
            nanoem_u16_t handle = it->second;
            nanoem_rsize_t numKeyframes;
            nanoem_document_accessory_keyframe_t *const *keyframes =
                nanoemDocumentAccessoryGetAllAccessoryKeyframeObjects(accessory, &numKeyframes);
            String parentModelNameString, parentBoneNameString;
            for (nanoem_rsize_t j = 0; j < numKeyframes; j++) {
                const nanoem_document_accessory_keyframe_t *keyframe = keyframes[j];
                const nanoem_document_base_keyframe_t *base =
                    nanoemDocumentAccessoryKeyframeGetBaseKeyframeObject(keyframe);
                nanoem_frame_index_t frameIndex = nanoemDocumentBaseKeyframeGetFrameIndex(base);
                nanoemDocumentAccessoryKeyframeGetParentModelObject(keyframe);
                const nanoem_document_model_t *parentModel =
                    nanoemDocumentAccessoryKeyframeGetParentModelObject(keyframe);
                const nanoem_unicode_string_t *parentBoneName =
                    nanoemDocumentAccessoryKeyframeGetParentModelBoneName(keyframe);
                StringUtils::getUtf8String(nanoemDocumentModelGetName(parentModel, NANOEM_LANGUAGE_TYPE_FIRST_ENUM),
                    factory, parentModelNameString);
                StringUtils::getUtf8String(parentBoneName, factory, parentBoneNameString);
                m_client->sendSeekMessage(frameIndex);
                m_client->sendSetAccessoryOutsideParentMessage(handle, parentModelNameString, parentBoneNameString);
                m_client->sendRegisterAccessoryKeyframeMessage(handle);
                sendUndoAction();
            }
        }
    }
    nanoem_document_model_t *const *models = nanoemDocumentGetAllModelObjects(document, &numModels);
    for (nanoem_rsize_t i = 0; i < numModels; i++) {
        const nanoem_document_model_t *model = models[i];
        StringUtils::getUtf8String(nanoemDocumentModelGetName(model, NANOEM_LANGUAGE_TYPE_FIRST_ENUM), factory, name);
        ModelHandleMap::const_iterator it = m_model2HandleMap.find(name);
        if (it != m_model2HandleMap.end()) {
            nanoem_u16_t handle = it->second;
            nanoem_document_model_keyframe_t *const *keyframes =
                nanoemDocumentModelGetAllModelKeyframeObjects(model, &numKeyframes);
            String boneNameString, parentBoneNameString, parentModelNameString;
            for (nanoem_rsize_t j = 0; j < numKeyframes; j++) {
                const nanoem_document_model_keyframe_t *keyframe = keyframes[j];
                const nanoem_document_base_keyframe_t *base =
                    nanoemDocumentModelKeyframeGetBaseKeyframeObject(keyframe);
                nanoem_frame_index_t frameIndex = nanoemDocumentBaseKeyframeGetFrameIndex(base);
                nanoem_document_outside_parent_t *const *parents =
                    nanoemDocumentModelKeyframeGetAllOutsideParentObjects(keyframe, &numOutsideParents);
                m_client->sendSeekMessage(frameIndex);
                waitForAction(false);
                for (nanoem_rsize_t k = 0; k < numOutsideParents; k++) {
                    const nanoem_document_outside_parent_t *parent = parents[k];
                    const nanoem_unicode_string_t *parentBoneName = nanoemDocumentOutsideParentGetBoneName(parent);
                    const nanoem_document_model_t *parentModel = nanoemDocumentOutsideParentGetModelObject(parent);
                    const nanoem_unicode_string_t *parentModelName =
                        nanoemDocumentModelGetName(parentModel, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
                    StringUtils::getUtf8String(parentBoneName, factory, parentBoneNameString);
                    StringUtils::getUtf8String(parentModelName, factory, parentModelNameString);
                    m_client->sendSetModelOutsideParentMessage(
                        handle, boneNameString, parentBoneNameString, parentModelNameString);
                    m_client->sendRegisterModelKeyframeMessage(handle);
                    waitForAction(false);
                }
            }
        }
    }
    sendSaveAction();
}

void
LoadPMMExecutor::sendUndoAction()
{
    waitForAction(false);
    if (bx::frnd(&m_rng) < m_undoFactor) {
        m_client->sendMenuActionMessage(ApplicationMenuBuilder::kMenuItemTypeEditUndo);
        waitForAction(false);
        m_client->sendMenuActionMessage(ApplicationMenuBuilder::kMenuItemTypeEditRedo);
        waitForAction(false);
    }
    sendSaveActionPeriodically();
}

void
LoadPMMExecutor::sendSaveAction()
{
#ifdef BX_PLATFORM_OSX
    if (m_commandLine->hasArg("enable-macos-project")) {
        String newPath;
        bx::stringPrintf(newPath, "%s.nanoem", m_fileURI.absolutePathByDeletingPathExtension().c_str());
        m_client->addCompleteSavingFileEventListener(
            [](void *userData, const URI &fileURI, nanoem_u32_t type, nanoem_u64_t ticks) {
                BX_UNUSED_1(type);
                fprintf(stderr, "saved = { filePath: \"%s\", seconds: %.2f }", fileURI.absolutePathConstString(),
                    stm_sec(ticks));
                auto self = static_cast<LoadPMMExecutor *>(userData);
                self->m_savingProjectSema.post();
            },
            this, true);
        {
            bx::MutexScope scope(*m_eventLock);
            BX_UNUSED_1(scope);
            m_client->sendSaveFileMessage(URI::createFromFilePath(newPath), IFileManager::kDialogTypeSaveProjectFile);
        }
        m_savingProjectSema.wait();
        {
            bx::MutexScope scope(*m_eventLock);
            BX_UNUSED_1(scope);
            m_client->sendMenuActionMessage(kPhysicsSimulationMode);
        }
    }
#endif
    if (!m_commandLine->hasArg("disable-saving-project")) {
        m_client->addCompleteSavingFileEventListener(
            [](void *userData, const URI &fileURI, nanoem_u32_t type, nanoem_u64_t ticks) {
                fprintf(stderr, "saved = { filePath: \"%s\", seconds: %.2f }", fileURI.absolutePathConstString(),
                    stm_sec(ticks));
                BX_UNUSED_1(type);
                auto self = static_cast<LoadPMMExecutor *>(userData);
                self->m_client->addCompleteLoadingFileEventListener(
                    [](void *userData, const URI &fileURI, nanoem_u32_t type, nanoem_u64_t ticks) {
                        fprintf(stderr, "loaded = { filePath: \"%s\", seconds: %.2f }",
                            fileURI.absolutePathConstString(), stm_sec(ticks));
                        BX_UNUSED_1(type);
                        auto self = static_cast<LoadPMMExecutor *>(userData);
                        self->m_savingProjectSema.post();
                    },
                    self, true);
                self->m_accessory2HandleMap.clear();
                self->m_model2HandleMap.clear();
                {
                    bx::MutexScope scope(*self->m_eventLock);
                    BX_UNUSED_1(scope);
                    self->m_client->sendLoadFileMessage(self->m_fileURI, IFileManager::kDialogTypeOpenProject);
                }
            },
            this, true);
        {
            bx::MutexScope scope(*m_eventLock);
            BX_UNUSED_1(scope);
            m_client->sendSaveFileMessage(m_fileURI, IFileManager::kDialogTypeSaveProjectFile);
        }
        m_savingProjectSema.wait();
    }
}

void
LoadPMMExecutor::sendSaveActionPeriodically()
{
    nanoem_u64_t now = stm_now();
    if (stm_sec(stm_diff(now, m_lastTick)) > m_saveSecondsInterval) {
        sendSaveAction();
        m_lastTick = now;
    }
}

void
LoadPMMExecutor::waitForAction(bool skippable)
{
    if (!skippable) {
        m_consumeFrameSema.wait();
    }
}

class LoadAllEffectsExecutor : public test::IExecutor, private NonCopyable {
public:
    LoadAllEffectsExecutor(const URI &fileURI, const bx::CommandLine *cmd, ThreadedApplicationService *service,
        ThreadedApplicationClient *client);
    ~LoadAllEffectsExecutor();

    void start();
    void finish();

private:
    static const int kSleepMilliseconds = 100;
    int run();
    void waitForAction(bool skippable);

    const URI m_fileURI;
    const bx::CommandLine *m_commandLine;
    ThreadedApplicationClient *m_client;
    bx::Thread m_thread;
    bx::Semaphore m_consumeFrameSema;
    bx::Semaphore m_loadingDrawableLock;
};

LoadAllEffectsExecutor::LoadAllEffectsExecutor(const URI &fileURI, const bx::CommandLine *cmd,
    ThreadedApplicationService * /* service */, ThreadedApplicationClient *client)
    : m_fileURI(fileURI)
    , m_commandLine(cmd)
    , m_client(client)
{
    m_client->addAddAccessoryEventListener(
        [](void *userData, nanoem_u16_t /* handle */, const char * /* name */) {
            LoadAllEffectsExecutor *self = static_cast<LoadAllEffectsExecutor *>(userData);
            bx::sleep(kSleepMilliseconds);
            self->m_loadingDrawableLock.post();
        },
        this, false);
    m_client->addAddModelEventListener(
        [](void *userData, nanoem_u16_t /* handle */, const char * /* name */) {
            LoadAllEffectsExecutor *self = static_cast<LoadAllEffectsExecutor *>(userData);
            bx::sleep(kSleepMilliseconds);
            self->m_loadingDrawableLock.post();
        },
        this, false);
    m_client->addConsumePassEventListener(
        [](void *userData, nanoem_u64_t /* globalFrameIndex */) {
            LoadAllEffectsExecutor *self = static_cast<LoadAllEffectsExecutor *>(userData);
            self->m_consumeFrameSema.post();
        },
        this, false);
}

LoadAllEffectsExecutor::~LoadAllEffectsExecutor()
{
}

void
LoadAllEffectsExecutor::start()
{
    m_thread.init(
        [](bx::Thread *thread, void *userData) {
            LoadAllEffectsExecutor *self = static_cast<LoadAllEffectsExecutor *>(userData);
            thread->setThreadName("com.github.nanoem.macos.test.LoadAllAccessoriesExecutor");
            return self->run();
        },
        this);
}

void
LoadAllEffectsExecutor::finish()
{
    m_thread.shutdown();
}

int
LoadAllEffectsExecutor::run()
{
    if (FILE *fp = fopen(m_fileURI.absolutePathConstString(), "r")) {
        char buffer[1024];
        int offset = 0, start = strtol(m_commandLine->findOption("offset", "0"), nullptr, 0);
        while (fgets(buffer, sizeof(buffer), fp)) {
            offset++;
            if (offset <= start || buffer[0] == '#') {
                continue;
            }
            buffer[strcspn(buffer, "\r\n")] = 0;
            const URI &fileURI = URI::createFromFilePath(buffer);
            if (FileUtils::exists(fileURI)) {
                EMLOG_DEBUG("PATH: {}", buffer);
                m_client->sendMenuActionMessage(ApplicationMenuBuilder::kMenuItemTypeProjectEnableEffect);
                m_client->sendLoadFileMessage(fileURI, IFileManager::kDialogTypeLoadModelFile);
                m_loadingDrawableLock.wait();
                m_client->sendNewProjectMessage();
                waitForAction(false);
            }
        }
        fclose(fp);
    }
    else {
        EMLOG_WARN("Cannot find {}", m_fileURI.absolutePathConstString());
    }
    m_client->sendDestroyMessage();
    return 0;
}

void
LoadAllEffectsExecutor::waitForAction(bool skippable)
{
    if (!skippable) {
        m_consumeFrameSema.wait();
    }
}

class LoadAllModelsExecutor : public test::IExecutor, private NonCopyable {
public:
    LoadAllModelsExecutor(const URI &fileURI, const bx::CommandLine *cmd, ThreadedApplicationService *service,
        ThreadedApplicationClient *client);
    ~LoadAllModelsExecutor();

    void start();
    void finish();

private:
    static const int kSleepMilliseconds = 1000;
    int run();
    void waitForAction(bool skippable);

    const URI m_fileURI;
    const bx::CommandLine *m_commandLine;
    ThreadedApplicationClient *m_client;
    bx::Thread m_thread;
    bx::Semaphore m_consumeFrameSema;
    bx::Semaphore m_loadingDrawableLock;
};

LoadAllModelsExecutor::LoadAllModelsExecutor(const URI &fileURI, const bx::CommandLine *cmd,
    ThreadedApplicationService * /* service */, ThreadedApplicationClient *client)
    : m_fileURI(fileURI)
    , m_commandLine(cmd)
    , m_client(client)
{
    m_client->addAddAccessoryEventListener(
        [](void *userData, nanoem_u16_t /* handle */, const char * /* name */) {
            LoadAllModelsExecutor *self = static_cast<LoadAllModelsExecutor *>(userData);
            bx::sleep(kSleepMilliseconds);
            self->m_loadingDrawableLock.post();
        },
        this, false);
    m_client->addAddModelEventListener(
        [](void *userData, nanoem_u16_t /* handle */, const char * /* name */) {
            LoadAllModelsExecutor *self = static_cast<LoadAllModelsExecutor *>(userData);
            bx::sleep(kSleepMilliseconds);
            self->m_loadingDrawableLock.post();
        },
        this, false);
    m_client->addConsumePassEventListener(
        [](void *userData, nanoem_u64_t /* globalFrameIndex */) {
            LoadAllModelsExecutor *self = static_cast<LoadAllModelsExecutor *>(userData);
            self->m_consumeFrameSema.post();
        },
        this, false);
}

LoadAllModelsExecutor::~LoadAllModelsExecutor()
{
}

void
LoadAllModelsExecutor::start()
{
    m_thread.init(
        [](bx::Thread *thread, void *userData) {
            LoadAllModelsExecutor *self = static_cast<LoadAllModelsExecutor *>(userData);
            thread->setThreadName("com.github.nanoem.macos.test.LoadAllModelsExecutor");
            return self->run();
        },
        this);
}

void
LoadAllModelsExecutor::finish()
{
    m_thread.shutdown();
}

int
LoadAllModelsExecutor::run()
{
    if (FILE *fp = fopen(m_fileURI.absolutePathConstString(), "r")) {
        char buffer[1024];
        int offset = 0, start = strtol(m_commandLine->findOption("offset", "0"), nullptr, 0);
        while (fgets(buffer, sizeof(buffer), fp)) {
            offset++;
            if (offset <= start || buffer[0] != '/') {
                continue;
            }
            buffer[strcspn(buffer, "\r\n")] = 0;
            const URI &fileURI = URI::createFromFilePath(buffer);
            if (FileUtils::exists(fileURI)) {
                EMLOG_INFO("PATH: {}", buffer);
                m_client->sendLoadFileMessage(fileURI, IFileManager::kDialogTypeLoadModelFile);
                m_loadingDrawableLock.wait();
                m_client->sendNewProjectMessage();
                waitForAction(false);
            }
        }
        fclose(fp);
    }
    m_client->sendDestroyMessage();
    return 0;
}

void
LoadAllModelsExecutor::waitForAction(bool skippable)
{
    if (!skippable) {
        m_consumeFrameSema.wait();
    }
}

} /* namespace test */
} /* namespace nanoem */
