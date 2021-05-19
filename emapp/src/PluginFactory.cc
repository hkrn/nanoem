/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/PluginFactory.h"

#include "emapp/Accessory.h"
#include "emapp/IModelObjectSelection.h"
#include "emapp/IMotionKeyframeSelection.h"
#include "emapp/Model.h"
#include "emapp/Motion.h"
#include "emapp/Progress.h"
#include "emapp/Project.h"
#include "emapp/StringUtils.h"
#include "emapp/command/BatchUndoCommandListCommand.h"
#include "emapp/command/ModelSnapshotCommand.h"
#include "emapp/command/MotionSnapshotCommand.h"
#include "emapp/plugin/DecoderPlugin.h"
#include "emapp/plugin/EffectPlugin.h"
#include "emapp/plugin/EncoderPlugin.h"
#include "emapp/plugin/ModelIOPlugin.h"
#include "emapp/plugin/MotionIOPlugin.h"
#include "emapp/private/CommonInclude.h"
#include "emapp/sdk/Decoder.h"
#include "emapp/sdk/Effect.h"
#include "emapp/sdk/Encoder.h"

namespace nanoem {

PluginFactory::DecoderPluginProxy::DecoderPluginProxy(plugin::DecoderPlugin *plugin)
    : m_plugin(plugin)
{
}

PluginFactory::DecoderPluginProxy::~DecoderPluginProxy() NANOEM_DECL_NOEXCEPT
{
}

bool
PluginFactory::DecoderPluginProxy::loadAudio(const URI &fileURI, nanoem_u32_t fps, Error &error)
{
    bool succeeded = false;
    if (m_plugin) {
        m_plugin->setOption(NANOEM_APPLICATION_PLUGIN_DECODER_OPTION_FPS, fps, error);
        succeeded = m_plugin->open(fileURI, error);
    }
    return succeeded;
}

bool
PluginFactory::DecoderPluginProxy::loadVideo(const URI &fileURI, nanoem_u32_t fps, Error &error)
{
    bool succeeded = false;
    if (m_plugin) {
        m_plugin->setOption(NANOEM_APPLICATION_PLUGIN_DECODER_OPTION_FPS, fps, error);
        succeeded = m_plugin->open(fileURI, error);
    }
    return succeeded;
}

bool
PluginFactory::DecoderPluginProxy::decodeAudioFrame(nanoem_frame_index_t frameIndex, ByteArray &bytes, Error &error)
{
    return m_plugin ? m_plugin->decodeAudioFrame(frameIndex, bytes, error) : false;
}

bool
PluginFactory::DecoderPluginProxy::decodeVideoFrame(nanoem_frame_index_t frameIndex, ByteArray &bytes, Error &error)
{
    return m_plugin ? m_plugin->decodeVideoFrame(frameIndex, bytes, error) : false;
}

nanoem_u32_t
PluginFactory::DecoderPluginProxy::numBits() const NANOEM_DECL_NOEXCEPT
{
    return m_plugin ? m_plugin->audioFormatValue(NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_NUM_BITS) : 0;
}

nanoem_u32_t
PluginFactory::DecoderPluginProxy::numChannels() const NANOEM_DECL_NOEXCEPT
{
    return m_plugin ? m_plugin->audioFormatValue(NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_NUM_CHANNELS) : 0;
}

nanoem_u32_t
PluginFactory::DecoderPluginProxy::frequency() const NANOEM_DECL_NOEXCEPT
{
    return m_plugin ? m_plugin->audioFormatValue(NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_FREQUENCY) : 0;
}

nanoem_u32_t
PluginFactory::DecoderPluginProxy::width() const NANOEM_DECL_NOEXCEPT
{
    return m_plugin ? m_plugin->videoFormatValue(NANOEM_APPLICATION_PLUGIN_DECODER_VIDEO_FORMAT_WIDTH) : 0;
}

nanoem_u32_t
PluginFactory::DecoderPluginProxy::height() const NANOEM_DECL_NOEXCEPT
{
    return m_plugin ? m_plugin->videoFormatValue(NANOEM_APPLICATION_PLUGIN_DECODER_VIDEO_FORMAT_HEIGHT) : 0;
}

nanoem_u32_t
PluginFactory::DecoderPluginProxy::duration() const NANOEM_DECL_NOEXCEPT
{
    return m_plugin ? glm::max(m_plugin->audioDuration(), m_plugin->videoDuration()) : 0;
}

StringList
PluginFactory::DecoderPluginProxy::availableDecodingAudioExtensions() const
{
    StringList extensions;
    if (m_plugin) {
        m_plugin->getAvailableAudioFormatExtensions(extensions);
    }
    return extensions;
}

Error
PluginFactory::DecoderPluginProxy::error() const
{
    return m_plugin ? Error(m_plugin->failureReason(), m_plugin->recoverySuggestion(), Error::kDomainTypePlugin)
                    : Error();
}

StringList
PluginFactory::DecoderPluginProxy::availableDecodingVideoExtensions() const
{
    StringList extensions;
    if (m_plugin) {
        m_plugin->getAvailableVideoFormatExtensions(extensions);
    }
    return extensions;
}

PluginFactory::EncoderPluginProxy::EncoderPluginProxy(plugin::EncoderPlugin *plugin)
    : m_plugin(plugin)
{
}

PluginFactory::EncoderPluginProxy::~EncoderPluginProxy() NANOEM_DECL_NOEXCEPT
{
}

void
PluginFactory::EncoderPluginProxy::setSize(const glm::ivec2 &value, Error &error)
{
    if (m_plugin) {
        m_plugin->setOption(NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_WIDTH, value.x, error);
        m_plugin->setOption(NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_HEIGHT, value.y, error);
    }
}

bool
PluginFactory::EncoderPluginProxy::getUIWindowLayout(ByteArray &value, Error &error)
{
    if (m_plugin) {
        m_plugin->getUIWindowLayout(value, error);
    }
    return !value.empty();
}

void
PluginFactory::EncoderPluginProxy::setUIComponentLayout(
    const char *id, const ByteArray &value, bool &reloadLayout, Error &error)
{
    if (m_plugin) {
        m_plugin->setUIComponentLayout(id, value, reloadLayout, error);
    }
}

void
PluginFactory::EncoderPluginProxy::cancel(Error &error)
{
    if (m_plugin) {
        m_plugin->interrupt();
        m_plugin->close(error);
        m_plugin->wait();
    }
}

StringList
PluginFactory::EncoderPluginProxy::availableEncodingVideoFormatExtensions() const
{
    StringList extensions;
    if (m_plugin) {
        m_plugin->getAvailableVideoFormatExtensions(extensions);
    }
    return extensions;
}

String
PluginFactory::EncoderPluginProxy::name() const
{
    return m_plugin ? m_plugin->name() : String();
}

Error
PluginFactory::EncoderPluginProxy::error() const
{
    return m_plugin ? Error(m_plugin->failureReason(), m_plugin->recoverySuggestion(), Error::kDomainTypePlugin)
                    : Error();
}

PluginFactory::EffectPluginProxy::EffectPluginProxy(plugin::EffectPlugin *plugin)
    : m_plugin(plugin)
{
}

PluginFactory::EffectPluginProxy::~EffectPluginProxy() NANOEM_DECL_NOEXCEPT
{
}

bool
PluginFactory::EffectPluginProxy::compile(const URI &fileURI, ByteArray &output)
{
    bool succeeded = false;
    if (m_plugin) {
        succeeded = m_plugin->compile(fileURI, output);
    }
    return succeeded;
}

bool
PluginFactory::EffectPluginProxy::compile(const String &source, ByteArray &output)
{
    bool succeeded = false;
    if (m_plugin) {
        succeeded = m_plugin->compile(source, output);
    }
    return succeeded;
}

void
PluginFactory::EffectPluginProxy::addIncludeSource(const String &path, const ByteArray &bytes)
{
    if (m_plugin) {
        m_plugin->addIncludeSource(path, bytes.data(), bytes.size());
    }
}

StringList
PluginFactory::EffectPluginProxy::availableExtensions() const
{
    return m_plugin ? m_plugin->availableExtensions() : StringList();
}

Error
PluginFactory::EffectPluginProxy::error() const
{
    return m_plugin ? Error(m_plugin->failureReason(), m_plugin->recoverySuggestion(), Error::kDomainTypePlugin)
                    : Error();
}

void
PluginFactory::EffectPluginProxy::setShaderVersion(int value, Error &error)
{
    if (m_plugin) {
        m_plugin->setOption(NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_SHADER_VERSION, value, error);
    }
}

void
PluginFactory::EffectPluginProxy::setMipmapEnabled(bool value, Error &error)
{
    if (m_plugin) {
        m_plugin->setOption(NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_ENABLE_MME_MIPMAP, value ? 1 : 0, error);
    }
}

PluginFactory::ModelIOPluginProxy::ModelIOPluginProxy(plugin::ModelIOPlugin *plugin)
    : m_plugin(plugin)
    , m_enableBackup(true)
{
}

void
PluginFactory::ModelIOPluginProxy::setLanguage(int value)
{
    if (m_plugin) {
        m_plugin->setLanguage(value);
    }
}

String
PluginFactory::ModelIOPluginProxy::name() const
{
    return m_plugin ? m_plugin->name() : String();
}

String
PluginFactory::ModelIOPluginProxy::description() const
{
    return m_plugin ? m_plugin->description() : String();
}

String
PluginFactory::ModelIOPluginProxy::version() const
{
    return m_plugin ? m_plugin->version() : String();
}

int
PluginFactory::ModelIOPluginProxy::countAllFunctions() const NANOEM_DECL_NOEXCEPT
{
    return m_plugin ? m_plugin->countAllFunctions() : 0;
}

String
PluginFactory::ModelIOPluginProxy::functionName(int value) const
{
    return m_plugin ? m_plugin->functionName(value) : String();
}

void
PluginFactory::ModelIOPluginProxy::setFunction(int value, Error &error)
{
    if (m_plugin) {
        m_plugin->setFunction(value, error);
    }
}

void
PluginFactory::ModelIOPluginProxy::setProjectDescription(Project *project, Error &error)
{
    if (m_plugin) {
        m_plugin->setInputAudio(project->audioPlayer(), error);
        m_plugin->setInputCamera(project->globalCamera(), error);
        m_plugin->setInputLight(project->globalLight(), error);
    }
}

void
PluginFactory::ModelIOPluginProxy::setInputData(const ByteArray &bytes, Error &error)
{
    if (m_plugin) {
        m_plugin->setInputData(bytes, error);
    }
}

void
PluginFactory::ModelIOPluginProxy::setBackupEnabled(bool value)
{
    m_enableBackup = value;
}

void
PluginFactory::ModelIOPluginProxy::setupSelection(const Model *model, Error &error)
{
    if (m_plugin) {
        const IModelObjectSelection *selection = model->selection();
        IntList indices;
        const model::Vertex::Set vertices(selection->allVertexSet());
        for (model::Vertex::Set::const_iterator it = vertices.begin(), end = vertices.end(); it != end; ++it) {
            indices.push_back(model::Vertex::index(*it));
        }
        m_plugin->setAllSelectedVertexObjectIndices(indices.data(), indices.size(), error);
        indices.clear();
        const model::Material::Set materials(selection->allMaterialSet());
        for (model::Material::Set::const_iterator it = materials.begin(), end = materials.end(); it != end; ++it) {
            indices.push_back(model::Material::index(*it));
        }
        m_plugin->setAllSelectedMaterialObjectIndices(indices.data(), indices.size(), error);
        indices.clear();
        const model::Bone::Set bones(selection->allBoneSet());
        for (model::Bone::Set::const_iterator it = bones.begin(), end = bones.end(); it != end; ++it) {
            indices.push_back(model::Bone::index(*it));
        }
        m_plugin->setAllSelectedBoneObjectIndices(indices.data(), indices.size(), error);
        indices.clear();
        const model::Morph::Set morphs(selection->allMorphSet());
        for (model::Morph::Set::const_iterator it = morphs.begin(), end = morphs.end(); it != end; ++it) {
            indices.push_back(model::Morph::index(*it));
        }
        m_plugin->setAllSelectedMorphObjectIndices(indices.data(), indices.size(), error);
        indices.clear();
        const model::Label::Set labels(selection->allLabelSet());
        for (model::Label::Set::const_iterator it = labels.begin(), end = labels.end(); it != end; ++it) {
            indices.push_back(model::Label::index(*it));
        }
        m_plugin->setAllSelectedLabelObjectIndices(indices.data(), indices.size(), error);
        indices.clear();
        const model::RigidBody::Set rigidBodies(selection->allRigidBodySet());
        for (model::RigidBody::Set::const_iterator it = rigidBodies.begin(), end = rigidBodies.end(); it != end; ++it) {
            indices.push_back(model::RigidBody::index(*it));
        }
        m_plugin->setAllSelectedRigidBodyObjectIndices(indices.data(), indices.size(), error);
        indices.clear();
        const model::Joint::Set joints(selection->allJointSet());
        for (model::Joint::Set::const_iterator it = joints.begin(), end = joints.end(); it != end; ++it) {
            indices.push_back(model::Joint::index(*it));
        }
        m_plugin->setAllSelectedJointObjectIndices(indices.data(), indices.size(), error);
        indices.clear();
        const model::SoftBody::Set softBodies(selection->allSoftBodySet());
        for (model::SoftBody::Set::const_iterator it = softBodies.begin(), end = softBodies.end(); it != end; ++it) {
            indices.push_back(model::SoftBody::index(*it));
        }
        m_plugin->setAllSelectedSoftBodyObjectIndices(indices.data(), indices.size(), error);
    }
}

void
PluginFactory::ModelIOPluginProxy::setupEditingMask(const Model *model, Error &error)
{
    if (m_plugin) {
        IntList indices;
        nanoem_rsize_t numObjects;
        nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(model->data(), &numObjects);
        for (nanoem_rsize_t i = 0; i < numObjects; i++) {
            const nanoem_model_vertex_t *vertexPtr = vertices[i];
            const model::Vertex *vertex = model::Vertex::cast(vertexPtr);
            if (vertex && vertex->isEditingMasked()) {
                indices.push_back(model::Vertex::index(vertexPtr));
            }
        }
        m_plugin->setAllMaskedVertexObjectIndices(indices.data(), indices.size(), error);
        indices.clear();
        nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(model->data(), &numObjects);
        for (nanoem_rsize_t i = 0; i < numObjects; i++) {
            const nanoem_model_material_t *materialPtr = materials[i];
            const model::Material *material = model::Material::cast(materialPtr);
            if (material && material->isVisible()) {
                indices.push_back(model::Material::index(materialPtr));
            }
        }
        m_plugin->setAllMaskedMaterialObjectIndices(indices.data(), indices.size(), error);
        indices.clear();
        nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(model->data(), &numObjects);
        for (nanoem_rsize_t i = 0; i < numObjects; i++) {
            const nanoem_model_bone_t *bonePtr = bones[i];
            const model::Bone *bone = model::Bone::cast(bonePtr);
            if (bone && bone->isEditingMasked()) {
                indices.push_back(model::Bone::index(bonePtr));
            }
        }
        m_plugin->setAllMaskedBoneObjectIndices(indices.data(), indices.size(), error);
        indices.clear();
        nanoem_model_rigid_body_t *const *rigidBodies = nanoemModelGetAllRigidBodyObjects(model->data(), &numObjects);
        for (nanoem_rsize_t i = 0; i < numObjects; i++) {
            const nanoem_model_rigid_body_t *rigidBodyPtr = rigidBodies[i];
            const model::RigidBody *rigidBody = model::RigidBody::cast(rigidBodyPtr);
            if (rigidBody && rigidBody->isEditingMasked()) {
                indices.push_back(model::RigidBody::index(rigidBodyPtr));
            }
        }
        m_plugin->setAllMaskedRigidBodyObjectIndices(indices.data(), indices.size(), error);
        indices.clear();
        nanoem_model_joint_t *const *joints = nanoemModelGetAllJointObjects(model->data(), &numObjects);
        for (nanoem_rsize_t i = 0; i < numObjects; i++) {
            const nanoem_model_joint_t *jointPtr = joints[i];
            const model::Joint *joint = model::Joint::cast(jointPtr);
            if (joint && joint->isEditingMasked()) {
                indices.push_back(model::Joint::index(jointPtr));
            }
        }
        m_plugin->setAllMaskedJointObjectIndices(indices.data(), indices.size(), error);
        indices.clear();
        nanoem_model_soft_body_t *const *softBodies = nanoemModelGetAllSoftBodyObjects(model->data(), &numObjects);
        for (nanoem_rsize_t i = 0; i < numObjects; i++) {
            const nanoem_model_soft_body_t *softBodyPtr = softBodies[i];
            const model::SoftBody *joint = model::SoftBody::cast(softBodyPtr);
            if (joint && joint->isEditingMasked()) {
                indices.push_back(model::SoftBody::index(softBodyPtr));
            }
        }
        m_plugin->setAllMaskedSoftBodyObjectIndices(indices.data(), indices.size(), error);
    }
}

void
PluginFactory::ModelIOPluginProxy::setAllSelectedVertexObjectIndices(const IntList &value, Error &error)
{
    if (m_plugin) {
        m_plugin->setAllSelectedVertexObjectIndices(value.data(), value.size(), error);
    }
}

void
PluginFactory::ModelIOPluginProxy::setAllSelectedMaterialObjectIndices(const IntList &value, Error &error)
{
    if (m_plugin) {
        m_plugin->setAllSelectedMaterialObjectIndices(value.data(), value.size(), error);
    }
}

void
PluginFactory::ModelIOPluginProxy::setAllSelectedBoneObjectIndices(const IntList &value, Error &error)
{
    if (m_plugin) {
        m_plugin->setAllSelectedBoneObjectIndices(value.data(), value.size(), error);
    }
}

void
PluginFactory::ModelIOPluginProxy::setAllSelectedMorphObjectIndices(const IntList &value, Error &error)
{
    if (m_plugin) {
        m_plugin->setAllSelectedMorphObjectIndices(value.data(), value.size(), error);
    }
}

void
PluginFactory::ModelIOPluginProxy::setAllSelectedLabelObjectIndices(const IntList &value, Error &error)
{
    if (m_plugin) {
        m_plugin->setAllSelectedLabelObjectIndices(value.data(), value.size(), error);
    }
}

void
PluginFactory::ModelIOPluginProxy::setAllSelectedRigidBodyObjectIndices(const IntList &value, Error &error)
{
    if (m_plugin) {
        m_plugin->setAllSelectedRigidBodyObjectIndices(value.data(), value.size(), error);
    }
}

void
PluginFactory::ModelIOPluginProxy::setAllSelectedJointObjectIndices(const IntList &value, Error &error)
{
    if (m_plugin) {
        m_plugin->setAllSelectedJointObjectIndices(value.data(), value.size(), error);
    }
}

bool
PluginFactory::ModelIOPluginProxy::getUIWindowLayout(ByteArray &value, Error &error)
{
    if (m_plugin) {
        m_plugin->getUIWindowLayout(value, error);
    }
    return !value.empty();
}

void
PluginFactory::ModelIOPluginProxy::setUIComponentLayout(
    const char *id, const ByteArray &value, bool &reloadLayout, Error &error)
{
    if (m_plugin) {
        m_plugin->setUIComponentLayout(id, value, reloadLayout, error);
    }
}

void
PluginFactory::ModelIOPluginProxy::execute(
    int functionIndex, const ByteArray &input, Model *model, Project *project, Error &error)
{
    if (m_plugin) {
        char title[Inline::kNameStackBufferSize], message[Inline::kLongNameStackBufferSize];
        StringUtils::format(title, sizeof(title), "Executing %s...", m_plugin->functionName(functionIndex));
        StringUtils::format(
            message, sizeof(message), "Executing %s of %s...", m_plugin->functionName(functionIndex), m_plugin->name());
        Progress pluginProgress(project, title, message, 0);
        ByteArray output;
        Model *lastActiveModel = project->activeModel();
        const Project::EditingMode editingMode = project->editingMode();
        project->setActiveModel(nullptr);
        if (m_plugin->execute(error)) {
            m_plugin->getOutputData(output, error);
            model->clear();
            if (!output.empty() && model->load(output, error)) {
                model->setCodecType(NANOEM_CODEC_TYPE_UTF16);
                pluginProgress.complete();
                const URI fileURI(model->fileURI());
                if (m_enableBackup) {
                    FileWriterScope scope;
                    String backupPath(fileURI.absolutePathByDeletingLastPathComponent()),
                        filename(URI::stringByDeletingPathExtension(fileURI.lastPathComponentConstString()));
                    backupPath.append("/");
                    backupPath.append(filename.c_str());
                    backupPath.append("_backup_at_");
                    char buffer[32];
                    StringUtils::formatDateTimeLocal(buffer, sizeof(buffer), "%Y%m%d_%H%M%S");
                    backupPath.append(buffer);
                    backupPath.append(".pmx");
                    if (scope.open(URI::createFromFilePath(backupPath), error)) {
                        FileUtils::write(scope.writer(), input, error);
                        if (!error.hasReason()) {
                            scope.commit(error);
                        }
                        else {
                            scope.rollback(error);
                        }
                    }
                }
                if (!error.hasReason()) {
                    FileWriterScope scope;
                    if (scope.open(fileURI, error) && model->save(scope.writer(), error)) {
                        scope.commit(error);
                    }
                }
                model->pushUndo(command::ModelSnapshotCommand::create(model, input));
            }
            else {
                pluginProgress.complete();
                Progress reloadModelProgress(project, 0);
                model->clear();
                model->load(input, error);
                model->setCodecType(NANOEM_CODEC_TYPE_UTF16);
                model->setupAllBindings();
                model->upload();
                model->loadAllImages(reloadModelProgress, error);
                if (Motion *motion = project->resolveMotion(model)) {
                    motion->initialize(model);
                }
                reloadModelProgress.complete();
            }
            project->performModelSkinDeformer(model);
        }
        project->setActiveModel(lastActiveModel);
        project->setEditingMode(editingMode);
    }
}

PluginFactory::ModelIOPluginProxy::~ModelIOPluginProxy() NANOEM_DECL_NOEXCEPT
{
}

PluginFactory::MotionIOPluginProxy::MotionIOPluginProxy(plugin::MotionIOPlugin *plugin)
    : m_plugin(plugin)
{
}

PluginFactory::MotionIOPluginProxy::~MotionIOPluginProxy() NANOEM_DECL_NOEXCEPT
{
}

void
PluginFactory::MotionIOPluginProxy::setLanguage(int value)
{
    if (m_plugin) {
        m_plugin->setLanguage(value);
    }
}

String
PluginFactory::MotionIOPluginProxy::name() const
{
    return m_plugin->name();
}

String
PluginFactory::MotionIOPluginProxy::description() const
{
    return m_plugin->description();
}

String
PluginFactory::MotionIOPluginProxy::version() const
{
    return m_plugin->version();
}

int
PluginFactory::MotionIOPluginProxy::countAllFunctions() const NANOEM_DECL_NOEXCEPT
{
    return m_plugin->countAllFunctions();
}

String
PluginFactory::MotionIOPluginProxy::functionName(int value) const
{
    return m_plugin->functionName(value);
}

void
PluginFactory::MotionIOPluginProxy::setFunction(int value, Error &error)
{
    if (m_plugin) {
        m_plugin->setFunction(value, error);
    }
}

void
PluginFactory::MotionIOPluginProxy::setProjectDescription(Project *project, Error &error)
{
    if (m_plugin) {
        m_plugin->setInputAudio(project->audioPlayer(), error);
        m_plugin->setInputCamera(project->globalCamera(), error);
        m_plugin->setInputLight(project->globalLight(), error);
        m_plugin->setInputActiveModel(project->activeModel(), error);
    }
}

void
PluginFactory::MotionIOPluginProxy::setInputData(const ByteArray &bytes, Error &error)
{
    if (m_plugin) {
        m_plugin->setInputMotionData(bytes, error);
    }
}

bool
PluginFactory::MotionIOPluginProxy::getUIWindowLayout(ByteArray &value, Error &error)
{
    if (m_plugin) {
        m_plugin->getUIWindowLayout(value, error);
    }
    return !value.empty();
}

void
PluginFactory::MotionIOPluginProxy::setUIComponentLayout(
    const char *id, const ByteArray &value, bool &reloadLayout, Error &error)
{
    if (m_plugin) {
        m_plugin->setUIComponentLayout(id, value, reloadLayout, error);
    }
}

void
PluginFactory::MotionIOPluginProxy::setupAccessorySelection(const Motion *activeMotion, Error &error)
{
    Motion::FrameIndexList indices;
    Motion::AccessoryKeyframeList keyframes;
    activeMotion->selection()->getAll(keyframes, nullptr);
    for (Motion::AccessoryKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end; ++it) {
        indices.push_back(nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionAccessoryKeyframeGetKeyframeObject(*it)));
    }
    setAllSelectedAccessoryKeyframes(indices, error);
}

void
PluginFactory::MotionIOPluginProxy::setupModelSelection(const Motion *activeMotion, const Model *model, Error &error)
{
    typedef tinystl::unordered_map<String, Motion::FrameIndexList, TinySTLAllocator> NamedFrameIndexMap;
    Motion::FrameIndexList indices;
    String name;
    nanoem_unicode_string_factory_t *factory = model->project()->unicodeStringFactory();
    NamedFrameIndexMap namedIndices;
    {
        Motion::BoneKeyframeList keyframes;
        activeMotion->selection()->getAll(keyframes, nullptr);
        for (Motion::BoneKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end; ++it) {
            StringUtils::getUtf8String(nanoemMotionBoneKeyframeGetName(*it), factory, name);
            namedIndices[name].push_back(
                nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionBoneKeyframeGetKeyframeObject(*it)));
        }
        for (NamedFrameIndexMap::const_iterator it = namedIndices.begin(), end = namedIndices.end(); it != end; ++it) {
            setAllNamedSelectedBoneKeyframes(it->first, it->second, error);
        }
    }
    {
        Motion::MorphKeyframeList keyframes;
        activeMotion->selection()->getAll(keyframes, nullptr);
        for (Motion::MorphKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end; ++it) {
            StringUtils::getUtf8String(nanoemMotionMorphKeyframeGetName(*it), factory, name);
            namedIndices[name].push_back(
                nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionMorphKeyframeGetKeyframeObject(*it)));
        }
        for (NamedFrameIndexMap::const_iterator it = namedIndices.begin(), end = namedIndices.end(); it != end; ++it) {
            setAllNamedSelectedMorphKeyframes(it->first, it->second, error);
        }
    }
    {
        Motion::ModelKeyframeList keyframes;
        activeMotion->selection()->getAll(keyframes, nullptr);
        for (Motion::ModelKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end; ++it) {
            indices.push_back(nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionModelKeyframeGetKeyframeObject(*it)));
        }
        setAllSelectedModelKeyframes(indices, error);
    }
}

void
PluginFactory::MotionIOPluginProxy::setAllNamedSelectedBoneKeyframes(
    const String &name, const Motion::FrameIndexList &indices, Error &error)
{
    if (m_plugin) {
        m_plugin->setAllNamedSelectedBoneKeyframes(name.c_str(), indices.data(), indices.size(), error);
    }
}

void
PluginFactory::MotionIOPluginProxy::setAllNamedSelectedMorphKeyframes(
    const String &name, const Motion::FrameIndexList &indices, Error &error)
{
    if (m_plugin) {
        m_plugin->setAllNamedSelectedMorphKeyframes(name.c_str(), indices.data(), indices.size(), error);
    }
}

void
PluginFactory::MotionIOPluginProxy::setAllSelectedAccessoryKeyframes(
    const Motion::FrameIndexList &indices, Error &error)
{
    if (m_plugin) {
        m_plugin->setAllSelectedAccessoryKeyframes(indices.data(), indices.size(), error);
    }
}

void
PluginFactory::MotionIOPluginProxy::setAllSelectedCameraKeyframes(const Motion::FrameIndexList &indices, Error &error)
{
    if (m_plugin) {
        m_plugin->setAllSelectedCameraKeyframes(indices.data(), indices.size(), error);
    }
}

void
PluginFactory::MotionIOPluginProxy::setAllSelectedLightKeyframes(const Motion::FrameIndexList &indices, Error &error)
{
    if (m_plugin) {
        m_plugin->setAllSelectedLightKeyframes(indices.data(), indices.size(), error);
    }
}

void
PluginFactory::MotionIOPluginProxy::setAllSelectedModelKeyframes(const Motion::FrameIndexList &indices, Error &error)
{
    if (m_plugin) {
        m_plugin->setAllSelectedModelKeyframes(indices.data(), indices.size(), error);
    }
}

void
PluginFactory::MotionIOPluginProxy::setAllSelectedSelfShadowKeyframes(
    const Motion::FrameIndexList &indices, Error &error)
{
    if (m_plugin) {
        m_plugin->setAllSelectedSelfShadowKeyframes(indices.data(), indices.size(), error);
    }
}

void
PluginFactory::MotionIOPluginProxy::execute(int functionIndex, const ByteArray &input, Project *project, Error &error)
{
    if (m_plugin) {
        char title[Inline::kNameStackBufferSize], message[Inline::kLongNameStackBufferSize];
        StringUtils::format(title, sizeof(title), "Executing %s...", m_plugin->functionName(functionIndex));
        StringUtils::format(
            message, sizeof(message), "Executing %s of %s...", m_plugin->functionName(functionIndex), m_plugin->name());
        Progress pluginProgress(project, title, message, 0);
        Motion::FrameIndexList indices;
        Accessory *activeAccessory = project->activeAccessory();
        Model *activeModel = project->activeModel();
        if (Motion *activeMotion = project->resolveMotion(activeModel)) {
            activeModel->pushUndo(execute(input, activeMotion, activeModel,
                NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_BONE | NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MODEL |
                    NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MORPH,
                error));
        }
        else if (Motion *activeMotion = project->resolveMotion(activeAccessory)) {
            project->pushUndo(
                execute(input, activeMotion, nullptr, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ACCESSORY, error));
        }
        else {
            command::BatchUndoCommandListCommand::UndoCommandList commands;
            Motion *motion = nullptr;
            {
                Motion::CameraKeyframeList keyframes;
                Motion::FrameIndexList indices;
                motion = project->cameraMotion();
                motion->selection()->getAll(keyframes, nullptr);
                for (Motion::CameraKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end();
                     it != end; ++it) {
                    indices.push_back(
                        nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionCameraKeyframeGetKeyframeObject(*it)));
                }
                setAllSelectedCameraKeyframes(indices, error);
                if (undo_command_t *command =
                        execute(input, motion, nullptr, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_CAMERA, error)) {
                    commands.push_back(command);
                }
                indices.clear();
            }
            {
                Motion::LightKeyframeList keyframes;
                motion = project->lightMotion();
                motion->selection()->getAll(keyframes, nullptr);
                for (Motion::LightKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end;
                     ++it) {
                    indices.push_back(
                        nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionLightKeyframeGetKeyframeObject(*it)));
                }
                setAllSelectedLightKeyframes(indices, error);
                if (undo_command_t *command =
                        execute(input, motion, nullptr, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_LIGHT, error)) {
                    commands.push_back(command);
                }
                indices.clear();
            }
            {
                Motion::SelfShadowKeyframeList keyframes;
                motion = project->selfShadowMotion();
                motion->selection()->getAll(keyframes, nullptr);
                for (Motion::SelfShadowKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end();
                     it != end; ++it) {
                    indices.push_back(
                        nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionSelfShadowKeyframeGetKeyframeObject(*it)));
                }
                setAllSelectedSelfShadowKeyframes(indices, error);
                if (undo_command_t *command =
                        execute(input, motion, nullptr, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_SELFSHADOW, error)) {
                    commands.push_back(command);
                }
                indices.clear();
            }
            if (!commands.empty()) {
                project->pushUndo(command::BatchUndoCommandListCommand::create(commands, nullptr, project));
            }
        }
        pluginProgress.complete();
    }
}

undo_command_t *
PluginFactory::MotionIOPluginProxy::execute(
    const ByteArray &input, Motion *motion, const Model *model, nanoem_u32_t flags, Error &error)
{
    undo_command_t *command = nullptr;
    if (m_plugin && m_plugin->execute(error)) {
        ByteArray output;
        m_plugin->getOutputData(output, error);
        motion->clearAllKeyframes();
        if (!output.empty() && motion->load(output, 0, error)) {
            command = command::MotionSnapshotCommand::create(motion, model, input, flags);
        }
        else {
            motion->clearAllKeyframes();
            motion->load(input, 0, error);
        }
    }
    return command;
}

plugin::DecoderPlugin *
PluginFactory::createDecoderPlugin(const URI &fileURI, IEventPublisher *publisher)
{
    plugin::DecoderPlugin *plugin = nanoem_new(plugin::DecoderPlugin(publisher));
    if (!plugin->load(fileURI) || !plugin->create()) {
        destroyDecoderPlugin(plugin);
        plugin = nullptr;
    }
    return plugin;
}

plugin::EncoderPlugin *
PluginFactory::createEncoderPlugin(const URI &fileURI, IEventPublisher *publisher)
{
    plugin::EncoderPlugin *plugin = nanoem_new(plugin::EncoderPlugin(publisher));
    if (!plugin->load(fileURI) || !plugin->create()) {
        destroyEncoderPlugin(plugin);
        plugin = nullptr;
    }
    return plugin;
}

plugin::EffectPlugin *
PluginFactory::createEffectPlugin(const URI &fileURI, IEventPublisher *publisher, Error &error)
{
    plugin::EffectPlugin *plugin = nanoem_new(plugin::EffectPlugin(publisher));
    if (plugin->load(fileURI) && plugin->create()) {
        const sg_backend backend = sg::query_backend();
        const bool isMetal = sg::is_backend_metal(backend),
#if defined(NANOEM_ENABLE_DEBUG_LABEL) || defined(_WIN32) /* workaround for X3577 warning on win32 */
                   isDebug = true;
#else
                   isDebug = false;
#endif
        plugin->setOption(NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_OPTIMIZATION, !isDebug, error);
        if (isMetal) {
            plugin->setOption(NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_OUTPUT_MSL, 1, error);
        }
        else if (backend == SG_BACKEND_D3D11) {
            plugin->setOption(NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_OUTPUT_HLSL, 1, error);
        }
        else if (backend == SG_BACKEND_GLES3) {
            plugin->setOption(NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_OUTPUT_ESSL, 1, error);
            plugin->setOption(NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_SHADER_VERSION, 300, error);
        }
        else if (backend == SG_BACKEND_GLCORE33) {
            plugin->setOption(NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_SHADER_VERSION, 330, error);
        }
    }
    else {
        destroyEffectPlugin(plugin);
        plugin = nullptr;
    }
    return plugin;
}

plugin::ModelIOPlugin *
PluginFactory::createModelIOPlugin(const URI &fileURI, IEventPublisher *publisher)
{
    plugin::ModelIOPlugin *plugin = nanoem_new(plugin::ModelIOPlugin(publisher));
    if (!plugin->load(fileURI) || !plugin->create()) {
        destroyModelIOPlugin(plugin);
        plugin = nullptr;
    }
    return plugin;
}

plugin::MotionIOPlugin *
PluginFactory::createMotionIOPlugin(const URI &fileURI, IEventPublisher *publisher)
{
    plugin::MotionIOPlugin *plugin = nanoem_new(plugin::MotionIOPlugin(publisher));
    if (!plugin->load(fileURI) || !plugin->create()) {
        destroyMotionIOPlugin(plugin);
        plugin = nullptr;
    }
    return plugin;
}

void
PluginFactory::destroyDecoderPlugin(plugin::DecoderPlugin *plugin)
{
    if (plugin) {
        plugin->destroy();
        plugin->unload();
        nanoem_delete(plugin);
    }
}

void
PluginFactory::destroyEncoderPlugin(plugin::EncoderPlugin *plugin)
{
    if (plugin) {
        plugin->destroy();
        plugin->unload();
        nanoem_delete(plugin);
    }
}

void
PluginFactory::destroyEffectPlugin(plugin::EffectPlugin *plugin)
{
    if (plugin) {
        plugin->destroy();
        plugin->unload();
        nanoem_delete(plugin);
    }
}

void
PluginFactory::destroyModelIOPlugin(plugin::ModelIOPlugin *plugin)
{
    if (plugin) {
        plugin->destroy();
        plugin->unload();
        nanoem_delete(plugin);
    }
}

void
PluginFactory::destroyMotionIOPlugin(plugin::MotionIOPlugin *plugin)
{
    if (plugin) {
        plugin->destroy();
        plugin->unload();
        nanoem_delete(plugin);
    }
}

} /* namespace nanoem */
