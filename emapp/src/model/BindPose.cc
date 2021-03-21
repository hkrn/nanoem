/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/model/BindPose.h"

#include "emapp/Error.h"
#include "emapp/IModelObjectSelection.h"
#include "emapp/ListUtils.h"
#include "emapp/Model.h"
#include "emapp/StringUtils.h"
#include "emapp/model/Bone.h"
#include "emapp/private/CommonInclude.h"

#include "../CommandMessage.inl"

namespace nanoem {
namespace model {
namespace {

struct Parser {
    enum State {
        kStateFirstEnum,
        kStateSignature = kStateFirstEnum,
        kStateTargetModel,
        kStateNumBones,
        kStateBoneIndex,
        kStateBoneName,
        kStateBoneTranslation,
        kStateBoneOrientation,
        kStateMorphIndex,
        kStateMorphName,
        kStateMorphWeight,
        kStateMaxEnum
    };
    Parser(Model *model, Error &error)
        : m_model(model)
        , m_error(error)
        , m_factory(nullptr)
        , m_state(kStateFirstEnum)
        , m_offset(0)
        , m_line(1)
        , m_lastCharacter(0)
        , m_depth(0)
        , m_ignore(false)
    {
        m_factory = model->project()->unicodeStringFactory();
    }
    void
    take(char ch)
    {
        if (ch == '\n' || ch == ';') {
            if (m_offset > 0) {
                reset();
                parseToken();
            }
            if (ch == '\n') {
                m_ignore = false;
                m_line++;
            }
        }
        else if (ch == '/') {
            m_ignore = m_lastCharacter == '/';
        }
        else if (ch == '\t' || ch == '\r') {
        }
        else if (ch == '{') {
            m_depth++;
            parseToken();
        }
        else if (ch == '}') {
            m_depth--;
        }
        else if (!m_ignore && m_offset < sizeof(m_buffer)) {
            m_buffer[m_offset++] = ch;
        }
        m_lastCharacter = ch;
    }
    void
    parseToken()
    {
        switch (m_state) {
        case kStateSignature: {
            static const char kSignature[] = "Vocaloid Pose Data file";
            if (StringUtils::equals(m_buffer, kSignature, sizeof(kSignature) - 1)) {
                reset();
                m_state = kStateTargetModel;
            }
            else {
                m_error = Error("Invalid .vpd header", nullptr, Error::kDomainTypeApplication);
            }
            break;
        }
        case kStateTargetModel: {
            m_state = kStateNumBones;
            break;
        }
        case kStateNumBones: {
            m_numObjects = StringUtils::parseInteger(m_buffer, nullptr);
            if (m_numObjects > 0) {
                clear();
                m_state = kStateBoneIndex;
            }
            else {
                m_error = Error("Bone count is not number", nullptr, Error::kDomainTypeApplication);
            }
            break;
        }
        case kStateBoneIndex: {
            if (StringUtils::equals(m_buffer, "Bone", 4)) {
                m_objectIndex = StringUtils::parseInteger(m_buffer + 4, nullptr);
                reset();
                m_state = kStateBoneName;
            }
            break;
        }
        case kStateBoneName: {
            StringUtils::getUtf8String(
                m_buffer, StringUtils::length(m_buffer), NANOEM_CODEC_TYPE_SJIS, m_factory, m_objectName);
            m_state = kStateBoneTranslation;
            break;
        }
        case kStateBoneTranslation: {
            if (const nanoem_model_bone_t *bonePtr = m_model->findBone(m_objectName)) {
                model::Bone *bone = model::Bone::cast(bonePtr);
                char *stringPtr = m_buffer;
                const nanoem_f32_t x = StringUtils::parseFloat(stringPtr, &stringPtr);
                StringUtils::parseComma(const_cast<const char **>(&stringPtr));
                const nanoem_f32_t y = StringUtils::parseFloat(stringPtr, &stringPtr);
                StringUtils::parseComma(const_cast<const char **>(&stringPtr));
                const nanoem_f32_t z = StringUtils::parseFloat(stringPtr, &stringPtr);
                bone->setLocalUserTranslation(Vector3(x, y, z));
                bone->setDirty(true);
                clear();
            }
            m_state = kStateBoneOrientation;
            break;
        }
        case kStateBoneOrientation: {
            if (const nanoem_model_bone_t *bonePtr = m_model->findBone(m_objectName)) {
                model::Bone *bone = model::Bone::cast(bonePtr);
                char *stringPtr = m_buffer;
                const nanoem_f32_t x = StringUtils::parseFloat(stringPtr, &stringPtr);
                StringUtils::parseComma(const_cast<const char **>(&stringPtr));
                const nanoem_f32_t y = StringUtils::parseFloat(stringPtr, &stringPtr);
                StringUtils::parseComma(const_cast<const char **>(&stringPtr));
                const nanoem_f32_t z = StringUtils::parseFloat(stringPtr, &stringPtr);
                StringUtils::parseComma(const_cast<const char **>(&stringPtr));
                const nanoem_f32_t w = StringUtils::parseFloat(stringPtr, &stringPtr);
                bone->setLocalUserOrientation(Quaternion(w, x, y, z));
                bone->setDirty(true);
                clear();
            }
            m_state = m_objectIndex < m_numObjects ? kStateBoneIndex : kStateMorphIndex;
            break;
        }
        case kStateMorphIndex: {
            if (StringUtils::equals(m_buffer, "Morph", 5)) {
                m_objectIndex = StringUtils::parseInteger(m_buffer + 5, nullptr);
                reset();
                m_state = kStateMorphName;
            }
            break;
        }
        case kStateMorphName: {
            StringUtils::getUtf8String(
                m_buffer, StringUtils::length(m_buffer), NANOEM_CODEC_TYPE_SJIS, m_factory, m_objectName);
            break;
        }
        case kStateMorphWeight: {
            if (const nanoem_model_morph_t *morphPtr = m_model->findMorph(m_objectName)) {
                model::Morph *morph = model::Morph::cast(morphPtr);
                morph->setWeight(StringUtils::parseFloat(m_buffer, nullptr));
                clear();
            }
            m_state = m_objectIndex < m_numObjects ? kStateBoneIndex : kStateMorphIndex;
            break;
        }
        default:
            break;
        }
    }
    void
    reset()
    {
        m_buffer[glm::min(m_offset, sizeof(m_buffer) - 1)] = 0;
        m_offset = 0;
    }
    void
    clear()
    {
        Inline::clearZeroMemory(m_buffer);
        m_offset = 0;
    }
    bool
    checkState()
    {
        bool result = false;
        switch (m_state) {
        case kStateFirstEnum:
        case kStateMaxEnum:
        case kStateTargetModel:
            if (!m_error.hasReason()) {
                m_error = Error("Invalid .vpd header", nullptr, Error::kDomainTypeApplication);
            }
            break;
        case kStateNumBones:
            if (!m_error.hasReason()) {
                m_error = Error("Bone count is not number", nullptr, Error::kDomainTypeApplication);
            }
            break;
        case kStateBoneName:
        case kStateBoneIndex:
        case kStateMorphName:
        case kStateMorphIndex:
        case kStateMorphWeight:
        case kStateBoneOrientation:
        case kStateBoneTranslation:
            result = true;
            break;
        }
        return result;
    }

    Model *m_model;
    Error &m_error;
    nanoem_unicode_string_factory_t *m_factory;
    State m_state;
    String m_objectName;
    char m_buffer[Inline::kNameStackBufferSize];
    nanoem_rsize_t m_offset;
    nanoem_rsize_t m_line;
    nanoem_u8_t m_lastCharacter;
    int m_numObjects;
    int m_objectIndex;
    int m_depth;
    bool m_ignore;
};

} /* namespace anonymous */

BindPose::Parameter::Parameter()
    : m_bonePtr(nullptr)
    , m_localMorphTranslation(Constants::kZeroV3)
    , m_localUserTranslation(Constants::kZeroV3)
    , m_localMorphOrientation(Constants::kZeroQ)
    , m_localUserOrientation(Constants::kZeroQ)
{
    for (int i = NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM;
         i < NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; i++) {
        m_bezierControlPoints[i] = model::Bone::kDefaultBezierControlPoint;
    }
}

BindPose::Parameter::Parameter(const Parameter &value)
    : m_bonePtr(value.m_bonePtr)
    , m_localMorphTranslation(value.m_localMorphTranslation)
    , m_localUserTranslation(value.m_localUserTranslation)
    , m_localMorphOrientation(value.m_localMorphOrientation)
    , m_localUserOrientation(value.m_localUserOrientation)
{
    for (int i = NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM;
         i < NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; i++) {
        m_bezierControlPoints[i] = value.m_bezierControlPoints[i];
    }
}

void
BindPose::Parameter::save(const nanoem_model_bone_t *bonePtr)
{
    const model::Bone *bone = model::Bone::cast(bonePtr);
    m_bonePtr = bonePtr;
    m_localMorphTranslation = bone->localMorphTranslation();
    m_localUserTranslation = bone->localUserTranslation();
    m_localMorphOrientation = bone->localMorphOrientation();
    m_localUserOrientation = bone->localUserOrientation();
    for (int i = NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM;
         i < NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; i++) {
        m_bezierControlPoints[i] = bone->bezierControlPoints(nanoem_motion_bone_keyframe_interpolation_type_t(i));
    }
}

void
BindPose::Parameter::restore(const nanoem_model_bone_t *bonePtr) const
{
    if (model::Bone *bone = model::Bone::cast(bonePtr)) {
        bone->setLocalMorphTranslation(m_localMorphTranslation);
        bone->setLocalUserTranslation(m_localUserTranslation);
        bone->setLocalMorphOrientation(m_localMorphOrientation);
        bone->setLocalUserOrientation(m_localUserOrientation);
        for (int i = NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM;
             i < NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; i++) {
            bone->setBezierControlPoints(nanoem_motion_bone_keyframe_interpolation_type_t(i), m_bezierControlPoints[i]);
        }
    }
}

StringList
BindPose::loadableExtensions()
{
    static const String kLoadableModelExtensions[] = { String("vpd"), String() };
    return StringList(
        &kLoadableModelExtensions[0], &kLoadableModelExtensions[BX_COUNTOF(kLoadableModelExtensions) - 1]);
}

StringSet
BindPose::loadableExtensionsSet()
{
    return ListUtils::toSetFromList<String>(loadableExtensions());
}

bool
BindPose::isLoadableExtension(const String &extension)
{
    return FileUtils::isLoadableExtension(extension, loadableExtensionsSet());
}

bool
BindPose::isLoadableExtension(const URI &fileURI)
{
    return isLoadableExtension(fileURI.pathExtension());
}

void
BindPose::releaseAllMessages(void *states, size_t numStates)
{
    Nanoem__Application__RedoTransformBoneCommand__State **statesPtr =
        static_cast<Nanoem__Application__RedoTransformBoneCommand__State **>(states);
    for (size_t i = 0; i < numStates; i++) {
        Nanoem__Application__RedoTransformBoneCommand__State *state = statesPtr[i];
        CommandMessageUtil::releaseVector(state->local_user_translation);
        CommandMessageUtil::releaseQuaternion(state->local_user_orientation);
        CommandMessageUtil::releaseVector(state->local_morph_translation);
        CommandMessageUtil::releaseQuaternion(state->local_morph_orientation);
        CommandMessageUtil::releaseInterpolation(state->interpolation->x);
        CommandMessageUtil::releaseInterpolation(state->interpolation->y);
        CommandMessageUtil::releaseInterpolation(state->interpolation->z);
        CommandMessageUtil::releaseInterpolation(state->interpolation->orientation);
        nanoem_delete(state->interpolation);
        nanoem_delete(state);
    }
    delete[] statesPtr;
}

BindPose::BindPose()
{
}

BindPose::BindPose(const BindPose &value)
{
    m_parameters.assign(value.m_parameters.begin(), value.m_parameters.end());
}

BindPose::~BindPose() NANOEM_DECL_NOEXCEPT
{
    m_parameters.clear();
}

bool
BindPose::load(Model *model, const nanoem_u8_t *data, nanoem_rsize_t size, Error &error) const
{
    Parser parser(model, error);
    nanoem_rsize_t offset = 0;
    while (offset < size && !error.hasReason()) {
        nanoem_u8_t ch = data[offset];
        parser.take(ch);
        offset++;
    }
    return parser.checkState();
}

bool
BindPose::load(Model *model, const ByteArray &bytes, Error &error) const
{
    return load(model, bytes.data(), bytes.size(), error);
}

bool
BindPose::save(const Model *model, IWriter *writer, Error &error) const
{
    nanoem_parameter_assert(writer, "must NOT be nullptr");
    String buffer;
    static const nanoem_u8_t kCommentParentModelName[] = { 0x90, 0x65, 0x83, 0x74, 0x83, 0x40, 0x83, 0x43, 0x83, 0x8b,
        0x96, 0xbc, 0x0 };
    nanoem_rsize_t length;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_unicode_string_factory_t *factory = model->project()->unicodeStringFactory();
    nanoem_u8_t *modelName = nanoemUnicodeStringFactoryGetByteArrayEncoding(factory,
        nanoemModelGetName(model->data(), NANOEM_LANGUAGE_TYPE_FIRST_ENUM), &length, NANOEM_CODEC_TYPE_SJIS, &status);
    StringUtils::format(
        buffer, "Vocaloid Pose Data file\r\n\r\n%s.osm;\t\t// %s\r\n", modelName, kCommentParentModelName);
    nanoemUnicodeStringFactoryDestroyByteArray(factory, modelName);
    nanoem_rsize_t numBones, numMorphs;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(model->data(), &numBones);
    int numVisibleBones = 0;
    for (nanoem_rsize_t i = 0; i < numBones; i++) {
        const nanoem_model_bone_t *bone = bones[i];
        if (nanoemModelBoneIsVisible(bone) || model->isConstraintEffectorBone(bone)) {
            numVisibleBones++;
        }
    }
    static const nanoem_u8_t kCommentAllBoneCount[] = { 0x91, 0x8d, 0x83, 0x7c, 0x81, 0x5b, 0x83, 0x59, 0x83, 0x7b,
        0x81, 0x5b, 0x83, 0x93, 0x90, 0x94, 0x0 };
    StringUtils::format(buffer, "%d;\t\t\t\t// %s\r\n\r\n", numVisibleBones, kCommentAllBoneCount);
    for (nanoem_rsize_t i = 0, j = 0; i < numBones; i++) {
        const nanoem_model_bone_t *bonePtr = bones[i];
        if (nanoemModelBoneIsVisible(bonePtr) || model->isConstraintEffectorBone(bonePtr)) {
            const model::Bone *bone = model::Bone::cast(bonePtr);
            nanoem_u8_t *boneName = nanoemUnicodeStringFactoryGetByteArrayEncoding(factory,
                nanoemModelBoneGetName(bonePtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM), &length, NANOEM_CODEC_TYPE_SJIS,
                &status);
            StringUtils::format(buffer, "Bone%jd{%s\r\n", j, boneName);
            nanoemUnicodeStringFactoryDestroyByteArray(factory, boneName);
            const Vector3 t(bone->localUserTranslation());
            const Quaternion &o = bone->localUserOrientation();
            StringUtils::format(buffer, "  %.6f,%.6f,%.6f;\t\t\t\t// trans x,y,z\r\n", t.x, t.y, t.z);
            StringUtils::format(buffer, "  %.6f,%.6f,%.6f,%6f;\t\t// Quaternion x,y,z,w\r\n", o.x, o.y, o.z, o.w);
            buffer.append("}\r\n\r\n");
            j++;
        }
    }
    nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(model->data(), &numMorphs);
    for (nanoem_rsize_t i = 0; i < numMorphs; i++) {
        const nanoem_model_morph_t *morphPtr = morphs[i];
        const model::Morph *morph = model::Morph::cast(morphPtr);
        nanoem_u8_t *morphName = nanoemUnicodeStringFactoryGetByteArrayEncoding(factory,
            nanoemModelMorphGetName(morphPtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM), &length, NANOEM_CODEC_TYPE_SJIS,
            &status);
        StringUtils::format(buffer, "Morph%jd{%s\r\n", i, morphName);
        nanoemUnicodeStringFactoryDestroyByteArray(factory, morphName);
        StringUtils::format(buffer, "  %.6f;\r\n", morph->weight());
        buffer.append("}\r\n\r\n");
    }
    FileUtils::write(writer, buffer, error);
    return !error.hasReason();
}

void
BindPose::saveAllMessages(void *states, size_t &numStates) const
{
    Nanoem__Application__RedoTransformBoneCommand__State ***statesPtrPtr =
        static_cast<Nanoem__Application__RedoTransformBoneCommand__State ***>(states);
    numStates = m_parameters.size();
    Nanoem__Application__RedoTransformBoneCommand__State **statesPtr = *statesPtrPtr =
        new Nanoem__Application__RedoTransformBoneCommand__State *[numStates];
    nanoem_rsize_t offset = 0;
    for (tinystl::vector<Parameter, TinySTLAllocator>::const_iterator it = m_parameters.begin(),
                                                                      end = m_parameters.end();
         it != end; ++it) {
        const Parameter &p = *it;
        Nanoem__Application__RedoTransformBoneCommand__State *state =
            nanoem_new(Nanoem__Application__RedoTransformBoneCommand__State);
        nanoem__application__redo_transform_bone_command__state__init(state);
        state->bone_index = model::Bone::index(p.m_bonePtr);
        CommandMessageUtil::setVector(p.m_localUserTranslation, state->local_user_translation);
        CommandMessageUtil::setQuaternion(p.m_localUserOrientation, state->local_user_orientation);
        CommandMessageUtil::setVector(p.m_localMorphTranslation, state->local_morph_translation);
        CommandMessageUtil::setQuaternion(p.m_localMorphOrientation, state->local_morph_orientation);
        state->interpolation = nanoem_new(Nanoem__Application__BoneInterpolation);
        nanoem__application__bone_interpolation__init(state->interpolation);
        CommandMessageUtil::setInterpolation(
            p.m_bezierControlPoints[NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_X],
            state->interpolation->x);
        CommandMessageUtil::setInterpolation(
            p.m_bezierControlPoints[NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Y],
            state->interpolation->y);
        CommandMessageUtil::setInterpolation(
            p.m_bezierControlPoints[NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Z],
            state->interpolation->z);
        CommandMessageUtil::setInterpolation(
            p.m_bezierControlPoints[NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_ORIENTATION],
            state->interpolation->orientation);
        statesPtr[offset++] = state;
    }
}

void
BindPose::restoreAllMessages(const Model *model, void *states, size_t numStates)
{
    const Nanoem__Application__RedoTransformBoneCommand__State **statesPtr =
        static_cast<const Nanoem__Application__RedoTransformBoneCommand__State **>(states);
    for (size_t i = 0; i < numStates; i++) {
        const Nanoem__Application__RedoTransformBoneCommand__State *state = statesPtr[i];
        Parameter p;
        p.m_bonePtr = model->findRedoBone(state->bone_index);
        CommandMessageUtil::getVector(state->local_user_translation, p.m_localUserTranslation);
        CommandMessageUtil::getQuaternion(state->local_user_orientation, p.m_localUserOrientation);
        CommandMessageUtil::getVector(state->local_morph_translation, p.m_localMorphTranslation);
        CommandMessageUtil::getQuaternion(state->local_morph_orientation, p.m_localMorphOrientation);
        CommandMessageUtil::getInterpolation(state->interpolation->x,
            p.m_bezierControlPoints[NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_X]);
        CommandMessageUtil::getInterpolation(state->interpolation->y,
            p.m_bezierControlPoints[NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Y]);
        CommandMessageUtil::getInterpolation(state->interpolation->z,
            p.m_bezierControlPoints[NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Z]);
        CommandMessageUtil::getInterpolation(state->interpolation->orientation,
            p.m_bezierControlPoints[NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_ORIENTATION]);
        m_parameters.push_back(p);
    }
}

} /* namespace model */
} /* namespace nanoem */
