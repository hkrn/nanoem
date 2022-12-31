/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/Motion.h"

#include "emapp/Accessory.h"
#include "emapp/EnumUtils.h"
#include "emapp/Error.h"
#include "emapp/FileUtils.h"
#include "emapp/ICamera.h"
#include "emapp/ILight.h"
#include "emapp/ListUtils.h"
#include "emapp/Model.h"
#include "emapp/PerspectiveCamera.h"
#include "emapp/Project.h"
#include "emapp/ShadowCamera.h"
#include "emapp/StringUtils.h"
#include "emapp/internal/MotionKeyframeSelection.h"
#include "emapp/model/Bone.h"
#include "emapp/model/Morph.h"
#include "emapp/private/CommonInclude.h"

#include "bx/handlealloc.h"
#include "protoc/application.pb-c.h"
#include "sokol/sokol_time.h"

#ifdef NANOEM_ENABLE_NMD
#include "nanoem/ext/motion.h"
#else
#define nanoemMotionLoadFromBufferNMD(a, b, c, d) NANOEM_STATUS_UNKNOWN
#define nanoemMutableMotionSaveToBufferNMD(a, b, c) NANOEM_STATUS_UNKNOWN
#endif

namespace nanoem {
namespace {

struct Merger {
    static void transformBoneKeyframeReversed(nanoem_mutable_motion_bone_keyframe_t *keyframe);

    Merger(const nanoem_motion_t *source, nanoem_unicode_string_factory_t *factory, nanoem_motion_t *opaque,
        bool _override);
    ~Merger() NANOEM_DECL_NOEXCEPT;

    void addAccessoryKeyframe(const nanoem_motion_accessory_keyframe_t *keyframe, nanoem_frame_index_t frameIndex);
    void mergeAllAccessoryKeyframes();
    void reverseBoneKeyframe(const nanoem_motion_bone_keyframe_t *origin, const String &newName);
    void reverseBoneKeyframe(const nanoem_motion_bone_keyframe_t *origin, StringSet &reversedBoneNameSet);
    void addBoneKeyframe(const nanoem_motion_bone_keyframe_t *origin, bool reverse, StringSet &reversedBoneNameSet);
    void mergeAllBoneKeyframes(bool reverse);
    void addCameraKeyframe(nanoem_motion_camera_keyframe_t *keyframe, nanoem_frame_index_t frameIndex);
    void mergeAllCameraKeyframes();
    void addLightKeyframe(const nanoem_motion_light_keyframe_t *keyframe, nanoem_frame_index_t frameIndex);
    void mergeAllLightKeyframes();
    void addModelKeyframe(const nanoem_motion_model_keyframe_t *keyframe, nanoem_frame_index_t frameIndex);
    void mergeAllModelKeyframes();
    void addMorphKeyframe(const nanoem_motion_morph_keyframe_t *keyframe, const nanoem_unicode_string_t *name,
        nanoem_frame_index_t frameIndex);
    void mergeAllMorphKeyframes();
    void addSelfShadowKeyframe(const nanoem_motion_self_shadow_keyframe_t *keyframe, nanoem_frame_index_t frameIndex);
    void mergeAllSelfShadowKeyframes();

    const nanoem_motion_t *m_source;
    const bool m_override;
    nanoem_unicode_string_factory_t *m_factory;
    nanoem_mutable_motion_t *m_dest;
    nanoem_status_t m_status;
};

void
Merger::transformBoneKeyframeReversed(nanoem_mutable_motion_bone_keyframe_t *keyframe)
{
    const nanoem_motion_bone_keyframe_t *origin = nanoemMutableMotionBoneKeyframeGetOriginObject(keyframe);
    const nanoem_f32_t *translation = nanoemMotionBoneKeyframeGetTranslation(origin);
    const nanoem_f32_t *orientation = nanoemMotionBoneKeyframeGetOrientation(origin);
    nanoemMutableMotionBoneKeyframeSetTranslation(
        keyframe, glm::value_ptr(Vector4(-translation[0], translation[1], translation[2], 0)));
    nanoemMutableMotionBoneKeyframeSetOrientation(
        keyframe, glm::value_ptr(Quaternion(orientation[3], orientation[0], -orientation[1], -orientation[2])));
}

Merger::Merger(
    const nanoem_motion_t *source, nanoem_unicode_string_factory_t *factory, nanoem_motion_t *opaque, bool _override)
    : m_source(source)
    , m_override(_override)
    , m_factory(factory)
    , m_dest(nullptr)
    , m_status(NANOEM_STATUS_SUCCESS)
{
    m_dest = nanoemMutableMotionCreateAsReference(opaque, &m_status);
}

Merger::~Merger() NANOEM_DECL_NOEXCEPT
{
    nanoemMutableMotionDestroy(m_dest);
    m_dest = nullptr;
    m_source = nullptr;
}

void
Merger::addAccessoryKeyframe(const nanoem_motion_accessory_keyframe_t *keyframe, nanoem_frame_index_t frameIndex)
{
    nanoem_motion_t *destOrigin = nanoemMutableMotionGetOriginObject(m_dest);
    nanoem_mutable_motion_accessory_keyframe_t *newKeyframe =
        nanoemMutableMotionAccessoryKeyframeCreate(destOrigin, &m_status);
    nanoemMutableMotionAccessoryKeyframeCopy(newKeyframe, keyframe, &m_status);
    nanoemMutableMotionAddAccessoryKeyframe(m_dest, newKeyframe, frameIndex, &m_status);
    nanoemMutableMotionAccessoryKeyframeDestroy(newKeyframe);
}

void
Merger::mergeAllAccessoryKeyframes()
{
    nanoem_rsize_t numKeyframes;
    nanoem_motion_t *destOrigin = nanoemMutableMotionGetOriginObject(m_dest);
    nanoem_motion_accessory_keyframe_t *const *keyframes =
        nanoemMotionGetAllAccessoryKeyframeObjects(m_source, &numKeyframes);
    for (nanoem_frame_index_t i = 0; i < numKeyframes; i++) {
        nanoem_motion_accessory_keyframe_t *keyframe = keyframes[i];
        nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionAccessoryKeyframeGetKeyframeObject(keyframe));
        if (!m_override && !nanoemMotionFindAccessoryKeyframeObject(destOrigin, frameIndex)) {
            addAccessoryKeyframe(keyframe, frameIndex);
        }
        else if (m_override) {
            nanoem_mutable_motion_accessory_keyframe_t *newKeyframe =
                nanoemMutableMotionAccessoryKeyframeCreateByFound(destOrigin, frameIndex, &m_status);
            if (newKeyframe) {
                nanoemMutableMotionAccessoryKeyframeCopy(newKeyframe, keyframe, &m_status);
                nanoemMutableMotionAccessoryKeyframeDestroy(newKeyframe);
            }
            else {
                addAccessoryKeyframe(keyframe, frameIndex);
            }
        }
    }
}

void
Merger::reverseBoneKeyframe(const nanoem_motion_bone_keyframe_t *origin, const String &newName)
{
    nanoem_motion_t *destOrigin = nanoemMutableMotionGetOriginObject(m_dest);
    StringUtils::UnicodeStringScope scope(m_factory);
    if (StringUtils::tryGetString(m_factory, newName, scope)) {
        const nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionBoneKeyframeGetKeyframeObject(origin));
        nanoem_mutable_motion_bone_keyframe_t *keyframe =
            nanoemMutableMotionBoneKeyframeCreateByFound(destOrigin, scope.value(), frameIndex, &m_status);
        if (keyframe) {
            nanoemMutableMotionBoneKeyframeCopy(keyframe, origin);
            transformBoneKeyframeReversed(keyframe);
        }
        else {
            keyframe = nanoemMutableMotionBoneKeyframeCreate(destOrigin, &m_status);
            nanoemMutableMotionBoneKeyframeCopy(keyframe, origin);
            transformBoneKeyframeReversed(keyframe);
            nanoemMutableMotionAddBoneKeyframe(m_dest, keyframe, scope.value(), frameIndex, &m_status);
        }
        nanoemMutableMotionBoneKeyframeDestroy(keyframe);
    }
}

void
Merger::reverseBoneKeyframe(const nanoem_motion_bone_keyframe_t *origin, StringSet &reversedBoneNameSet)
{
    static const nanoem_u8_t kLeftInJapanese[] = { 0xe5, 0xb7, 0xa6, 0x0 },
                             kRightInJapanese[] = { 0xe5, 0x8f, 0xb3, 0x0 };
    String utf8Name;
    StringUtils::getUtf8String(nanoemMotionBoneKeyframeGetName(origin), m_factory, utf8Name);
    if (StringUtils::hasPrefix(utf8Name.c_str(), reinterpret_cast<const char *>(kLeftInJapanese))) {
        const String &newName =
            StringUtils::substitutedPrefixString(reinterpret_cast<const char *>(kRightInJapanese), utf8Name.c_str());
        if (reversedBoneNameSet.find(newName) == reversedBoneNameSet.end()) {
            reverseBoneKeyframe(origin, newName);
            reversedBoneNameSet.insert(newName);
        }
    }
    if (StringUtils::hasPrefix(utf8Name.c_str(), reinterpret_cast<const char *>(kRightInJapanese))) {
        const String &newName =
            StringUtils::substitutedPrefixString(reinterpret_cast<const char *>(kLeftInJapanese), utf8Name.c_str());
        if (reversedBoneNameSet.find(newName) == reversedBoneNameSet.end()) {
            reverseBoneKeyframe(origin, newName);
            reversedBoneNameSet.insert(newName);
        }
    }
}

void
Merger::addBoneKeyframe(const nanoem_motion_bone_keyframe_t *origin, bool reverse, StringSet &reversedBoneNameSet)
{
    nanoem_motion_t *destOrigin = nanoemMutableMotionGetOriginObject(m_dest);
    nanoem_mutable_motion_bone_keyframe_t *newKeyframe = nanoemMutableMotionBoneKeyframeCreate(destOrigin, &m_status);
    nanoemMutableMotionBoneKeyframeCopy(newKeyframe, origin);
    if (reverse) {
        reverseBoneKeyframe(origin, reversedBoneNameSet);
    }
    const nanoem_unicode_string_t *name = nanoemMotionBoneKeyframeGetName(origin);
    const nanoem_frame_index_t frameIndex =
        nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionBoneKeyframeGetKeyframeObject(origin));
    nanoemMutableMotionAddBoneKeyframe(m_dest, newKeyframe, name, frameIndex, &m_status);
    nanoemMutableMotionBoneKeyframeDestroy(newKeyframe);
}

void
Merger::mergeAllBoneKeyframes(bool reverse)
{
    nanoem_rsize_t numKeyframes;
    nanoem_motion_t *destOrigin = nanoemMutableMotionGetOriginObject(m_dest);
    nanoem_motion_bone_keyframe_t *const *keyframes = nanoemMotionGetAllBoneKeyframeObjects(m_source, &numKeyframes);
    StringSet reversedBoneNameSet;
    for (nanoem_frame_index_t i = 0; i < numKeyframes; i++) {
        nanoem_motion_bone_keyframe_t *keyframe = keyframes[i];
        nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionBoneKeyframeGetKeyframeObject(keyframe));
        const nanoem_unicode_string_t *name = nanoemMotionBoneKeyframeGetName(keyframe);
        if (!m_override && !nanoemMotionFindBoneKeyframeObject(destOrigin, name, frameIndex)) {
            addBoneKeyframe(keyframe, reverse, reversedBoneNameSet);
        }
        else if (m_override) {
            nanoem_mutable_motion_bone_keyframe_t *newKeyframe =
                nanoemMutableMotionBoneKeyframeCreateByFound(destOrigin, name, frameIndex, &m_status);
            if (newKeyframe) {
                nanoemMutableMotionBoneKeyframeCopy(newKeyframe, keyframe);
                if (reverse) {
                    reverseBoneKeyframe(keyframe, reversedBoneNameSet);
                }
                nanoemMutableMotionBoneKeyframeDestroy(newKeyframe);
            }
            else {
                addBoneKeyframe(keyframe, reverse, reversedBoneNameSet);
            }
        }
    }
}

void
Merger::addCameraKeyframe(nanoem_motion_camera_keyframe_t *keyframe, nanoem_frame_index_t frameIndex)
{
    nanoem_motion_t *destOrigin = nanoemMutableMotionGetOriginObject(m_dest);
    nanoem_mutable_motion_camera_keyframe_t *newKeyframe =
        nanoemMutableMotionCameraKeyframeCreate(destOrigin, &m_status);
    nanoemMutableMotionCameraKeyframeCopy(newKeyframe, keyframe);
    nanoemMutableMotionAddCameraKeyframe(m_dest, newKeyframe, frameIndex, &m_status);
    nanoemMutableMotionCameraKeyframeDestroy(newKeyframe);
}

void
Merger::mergeAllCameraKeyframes()
{
    nanoem_rsize_t numKeyframes;
    nanoem_motion_t *destOrigin = nanoemMutableMotionGetOriginObject(m_dest);
    nanoem_motion_camera_keyframe_t *const *keyframes =
        nanoemMotionGetAllCameraKeyframeObjects(m_source, &numKeyframes);
    for (nanoem_frame_index_t i = 0; i < numKeyframes; i++) {
        nanoem_motion_camera_keyframe_t *keyframe = keyframes[i];
        nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionCameraKeyframeGetKeyframeObject(keyframe));
        if (!m_override && !nanoemMotionFindCameraKeyframeObject(destOrigin, frameIndex)) {
            addCameraKeyframe(keyframe, frameIndex);
        }
        else if (m_override) {
            nanoem_mutable_motion_camera_keyframe_t *newKeyframe =
                nanoemMutableMotionCameraKeyframeCreateByFound(destOrigin, frameIndex, &m_status);
            if (newKeyframe) {
                nanoemMutableMotionCameraKeyframeCopy(newKeyframe, keyframe);
                nanoemMutableMotionCameraKeyframeDestroy(newKeyframe);
            }
            else {
                addCameraKeyframe(keyframe, frameIndex);
            }
        }
    }
}

void
Merger::addLightKeyframe(const nanoem_motion_light_keyframe_t *keyframe, nanoem_frame_index_t frameIndex)
{
    nanoem_motion_t *destOrigin = nanoemMutableMotionGetOriginObject(m_dest);
    nanoem_mutable_motion_light_keyframe_t *newKeyframe = nanoemMutableMotionLightKeyframeCreate(destOrigin, &m_status);
    nanoemMutableMotionLightKeyframeCopy(newKeyframe, keyframe);
    nanoemMutableMotionAddLightKeyframe(m_dest, newKeyframe, frameIndex, &m_status);
    nanoemMutableMotionLightKeyframeDestroy(newKeyframe);
}

void
Merger::mergeAllLightKeyframes()
{
    nanoem_rsize_t numKeyframes;
    nanoem_motion_t *destOrigin = nanoemMutableMotionGetOriginObject(m_dest);
    nanoem_motion_light_keyframe_t *const *keyframes = nanoemMotionGetAllLightKeyframeObjects(m_source, &numKeyframes);
    for (nanoem_frame_index_t i = 0; i < numKeyframes; i++) {
        nanoem_motion_light_keyframe_t *keyframe = keyframes[i];
        nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionLightKeyframeGetKeyframeObject(keyframe));
        if (!m_override && !nanoemMotionFindLightKeyframeObject(destOrigin, frameIndex)) {
            addLightKeyframe(keyframe, frameIndex);
        }
        else if (m_override) {
            nanoem_mutable_motion_light_keyframe_t *newKeyframe =
                nanoemMutableMotionLightKeyframeCreateByFound(destOrigin, frameIndex, &m_status);
            if (newKeyframe) {
                nanoemMutableMotionLightKeyframeCopy(newKeyframe, keyframe);
                nanoemMutableMotionLightKeyframeDestroy(newKeyframe);
            }
            else {
                addLightKeyframe(keyframe, frameIndex);
            }
        }
    }
}

void
Merger::addModelKeyframe(const nanoem_motion_model_keyframe_t *keyframe, nanoem_frame_index_t frameIndex)
{
    nanoem_motion_t *destOrigin = nanoemMutableMotionGetOriginObject(m_dest);
    nanoem_mutable_motion_model_keyframe_t *newKeyframe = nanoemMutableMotionModelKeyframeCreate(destOrigin, &m_status);
    nanoemMutableMotionModelKeyframeCopy(newKeyframe, keyframe, &m_status);
    nanoemMutableMotionAddModelKeyframe(m_dest, newKeyframe, frameIndex, &m_status);
    nanoemMutableMotionModelKeyframeDestroy(newKeyframe);
}

void
Merger::mergeAllModelKeyframes()
{
    nanoem_rsize_t numKeyframes;
    nanoem_motion_t *destOrigin = nanoemMutableMotionGetOriginObject(m_dest);
    nanoem_motion_model_keyframe_t *const *keyframes = nanoemMotionGetAllModelKeyframeObjects(m_source, &numKeyframes);
    for (nanoem_frame_index_t i = 0; i < numKeyframes; i++) {
        nanoem_motion_model_keyframe_t *keyframe = keyframes[i];
        nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionModelKeyframeGetKeyframeObject(keyframe));
        if (!m_override && !nanoemMotionFindModelKeyframeObject(destOrigin, frameIndex)) {
            addModelKeyframe(keyframe, frameIndex);
        }
        else if (m_override) {
            nanoem_mutable_motion_model_keyframe_t *newKeyframe =
                nanoemMutableMotionModelKeyframeCreateByFound(destOrigin, frameIndex, &m_status);
            if (newKeyframe) {
                nanoemMutableMotionModelKeyframeCopy(newKeyframe, keyframe, &m_status);
                nanoemMutableMotionModelKeyframeDestroy(newKeyframe);
            }
            else {
                addModelKeyframe(keyframe, frameIndex);
            }
        }
    }
}

void
Merger::addMorphKeyframe(const nanoem_motion_morph_keyframe_t *keyframe, const nanoem_unicode_string_t *name,
    nanoem_frame_index_t frameIndex)
{
    nanoem_motion_t *destOrigin = nanoemMutableMotionGetOriginObject(m_dest);
    nanoem_mutable_motion_morph_keyframe_t *newKeyframe = nanoemMutableMotionMorphKeyframeCreate(destOrigin, &m_status);
    nanoemMutableMotionMorphKeyframeCopy(newKeyframe, keyframe);
    nanoemMutableMotionAddMorphKeyframe(m_dest, newKeyframe, name, frameIndex, &m_status);
    nanoemMutableMotionMorphKeyframeDestroy(newKeyframe);
}

void
Merger::mergeAllMorphKeyframes()
{
    nanoem_rsize_t numKeyframes;
    nanoem_motion_t *destOrigin = nanoemMutableMotionGetOriginObject(m_dest);
    nanoem_motion_morph_keyframe_t *const *keyframes = nanoemMotionGetAllMorphKeyframeObjects(m_source, &numKeyframes);
    for (nanoem_frame_index_t i = 0; i < numKeyframes; i++) {
        nanoem_motion_morph_keyframe_t *keyframe = keyframes[i];
        nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionMorphKeyframeGetKeyframeObject(keyframe));
        const nanoem_unicode_string_t *name = nanoemMotionMorphKeyframeGetName(keyframe);
        if (!m_override && !nanoemMotionFindMorphKeyframeObject(destOrigin, name, frameIndex)) {
            addMorphKeyframe(keyframe, name, frameIndex);
        }
        else if (m_override) {
            nanoem_mutable_motion_morph_keyframe_t *newKeyframe =
                nanoemMutableMotionMorphKeyframeCreateByFound(destOrigin, name, frameIndex, &m_status);
            if (newKeyframe) {
                nanoemMutableMotionMorphKeyframeCopy(newKeyframe, keyframe);
                nanoemMutableMotionMorphKeyframeDestroy(newKeyframe);
            }
            else {
                addMorphKeyframe(keyframe, name, frameIndex);
            }
        }
    }
}

void
Merger::addSelfShadowKeyframe(const nanoem_motion_self_shadow_keyframe_t *keyframe, nanoem_frame_index_t frameIndex)
{
    nanoem_motion_t *destOrigin = nanoemMutableMotionGetOriginObject(m_dest);
    nanoem_mutable_motion_self_shadow_keyframe_t *newKeyframe =
        nanoemMutableMotionSelfShadowKeyframeCreate(destOrigin, &m_status);
    nanoemMutableMotionSelfShadowKeyframeCopy(newKeyframe, keyframe);
    nanoemMutableMotionAddSelfShadowKeyframe(m_dest, newKeyframe, frameIndex, &m_status);
    nanoemMutableMotionSelfShadowKeyframeDestroy(newKeyframe);
}

void
Merger::mergeAllSelfShadowKeyframes()
{
    nanoem_rsize_t numKeyframes;
    nanoem_motion_t *destOrigin = nanoemMutableMotionGetOriginObject(m_dest);
    nanoem_motion_self_shadow_keyframe_t *const *keyframes =
        nanoemMotionGetAllSelfShadowKeyframeObjects(m_source, &numKeyframes);
    for (nanoem_frame_index_t i = 0; i < numKeyframes; i++) {
        nanoem_motion_self_shadow_keyframe_t *keyframe = keyframes[i];
        nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionSelfShadowKeyframeGetKeyframeObject(keyframe));
        if (!m_override && !nanoemMotionFindSelfShadowKeyframeObject(destOrigin, frameIndex)) {
            addSelfShadowKeyframe(keyframe, frameIndex);
        }
        else if (m_override) {
            nanoem_mutable_motion_self_shadow_keyframe_t *newKeyframe =
                nanoemMutableMotionSelfShadowKeyframeCreateByFound(destOrigin, frameIndex, &m_status);
            if (newKeyframe) {
                nanoemMutableMotionSelfShadowKeyframeCopy(newKeyframe, keyframe);
                nanoemMutableMotionSelfShadowKeyframeDestroy(newKeyframe);
            }
            else {
                addSelfShadowKeyframe(keyframe, frameIndex);
            }
        }
    }
}

static int
compareKeyframeAscend(
    const nanoem_motion_keyframe_object_t *lvalue, const nanoem_motion_keyframe_object_t *rvalue) NANOEM_DECL_NOEXCEPT
{
    const nanoem_frame_index_t lv = nanoemMotionKeyframeObjectGetFrameIndex(lvalue);
    const nanoem_frame_index_t rv = nanoemMotionKeyframeObjectGetFrameIndex(rvalue);
    if (lv < rv) {
        return -1;
    }
    else if (lv > rv) {
        return 1;
    }
    else {
        return 0;
    }
}

static int
compareKeyframeDescend(
    const nanoem_motion_keyframe_object_t *lvalue, const nanoem_motion_keyframe_object_t *rvalue) NANOEM_DECL_NOEXCEPT
{
    const nanoem_frame_index_t lv = nanoemMotionKeyframeObjectGetFrameIndex(lvalue);
    const nanoem_frame_index_t rv = nanoemMotionKeyframeObjectGetFrameIndex(rvalue);
    if (lv > rv) {
        return -1;
    }
    else if (lv < rv) {
        return 1;
    }
    else {
        return 0;
    }
}

} /* namespace anonymous */

const String Motion::kNMDFormatExtension = String("nmd");
const String Motion::kVMDFormatExtension = String("vmd");
const nanoem_u8_t Motion::kCameraAndLightTargetModelName[] = { 0xe3, 0x82, 0xab, 0xe3, 0x83, 0xa1, 0xe3, 0x83, 0xa9,
    0xe3, 0x83, 0xbb, 0xe7, 0x85, 0xa7, 0xe6, 0x98, 0x8e, 0 }; /* "Camera and Light" in Japanese */
const nanoem_frame_index_t Motion::kMaxFrameIndex = nanoem_frame_index_t(~0);

struct Motion::SelectionState NANOEM_DECL_SEALED {
    typedef tinystl::unordered_map<String, Motion::FrameIndexSet, TinySTLAllocator> NamedFrameIndexMap;

    void
    save(const Motion *motion)
    {
        const IMotionKeyframeSelection *selection = motion->selection();
        nanoem_unicode_string_factory_t *factory = motion->project()->unicodeStringFactory();
        String utf8;
        Motion::AccessoryKeyframeList accessoryKeyframes;
        selection->getAll(accessoryKeyframes, nullptr);
        for (Motion::AccessoryKeyframeList::const_iterator it = accessoryKeyframes.begin(),
                                                           end = accessoryKeyframes.end();
             it != end; ++it) {
            const nanoem_frame_index_t frameIndex =
                nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionAccessoryKeyframeGetKeyframeObject(*it));
            m_selectedAccessoryKeyframeIndices.insert(frameIndex);
        }
        Motion::BoneKeyframeList boneKeyframes;
        selection->getAll(boneKeyframes, nullptr);
        for (Motion::BoneKeyframeList::const_iterator it = boneKeyframes.begin(), end = boneKeyframes.end(); it != end;
             ++it) {
            const nanoem_unicode_string_t *name = nanoemMotionBoneKeyframeGetName(*it);
            const nanoem_frame_index_t frameIndex =
                nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionBoneKeyframeGetKeyframeObject(*it));
            StringUtils::getUtf8String(name, factory, utf8);
            m_selectedBoneKeyframeIndices[utf8].insert(frameIndex);
        }
        Motion::CameraKeyframeList cameraKeyframes;
        selection->getAll(cameraKeyframes, nullptr);
        for (Motion::CameraKeyframeList::const_iterator it = cameraKeyframes.begin(), end = cameraKeyframes.end();
             it != end; ++it) {
            const nanoem_frame_index_t frameIndex =
                nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionCameraKeyframeGetKeyframeObject(*it));
            m_selectedCameraKeyframeIndices.insert(frameIndex);
        }
        Motion::LightKeyframeList lightKeyframes;
        selection->getAll(lightKeyframes, nullptr);
        for (Motion::LightKeyframeList::const_iterator it = lightKeyframes.begin(), end = lightKeyframes.end();
             it != end; ++it) {
            const nanoem_frame_index_t frameIndex =
                nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionLightKeyframeGetKeyframeObject(*it));
            m_selectedLightKeyframeIndices.insert(frameIndex);
        }
        Motion::ModelKeyframeList modelKeyframes;
        selection->getAll(modelKeyframes, nullptr);
        for (Motion::ModelKeyframeList::const_iterator it = modelKeyframes.begin(), end = modelKeyframes.end();
             it != end; ++it) {
            const nanoem_frame_index_t frameIndex =
                nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionModelKeyframeGetKeyframeObject(*it));
            m_selectedModelKeyframeIndices.insert(frameIndex);
        }
        Motion::MorphKeyframeList morphKeyframes;
        selection->getAll(morphKeyframes, nullptr);
        for (Motion::MorphKeyframeList::const_iterator it = morphKeyframes.begin(), end = morphKeyframes.end();
             it != end; ++it) {
            const nanoem_unicode_string_t *name = nanoemMotionMorphKeyframeGetName(*it);
            const nanoem_frame_index_t frameIndex =
                nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionMorphKeyframeGetKeyframeObject(*it));
            StringUtils::getUtf8String(name, factory, utf8);
            m_selectedMorphKeyframeIndices[utf8].insert(frameIndex);
        }
    }
    void
    restore(Motion *motion) const
    {
        nanoem_unicode_string_factory_t *factory = motion->project()->unicodeStringFactory();
        IMotionKeyframeSelection *selection = motion->selection();
        StringUtils::UnicodeStringScope scope(factory);
        for (Motion::FrameIndexSet::const_iterator it = m_selectedAccessoryKeyframeIndices.begin(),
                                                   end = m_selectedAccessoryKeyframeIndices.end();
             it != end; ++it) {
            selection->add(motion->findAccessoryKeyframe(*it));
        }
        for (NamedFrameIndexMap::const_iterator it = m_selectedBoneKeyframeIndices.begin(),
                                                end = m_selectedBoneKeyframeIndices.end();
             it != end; ++it) {
            if (StringUtils::tryGetString(factory, it->first, scope)) {
                for (Motion::FrameIndexSet::const_iterator it2 = it->second.begin(), end2 = it->second.end();
                     it2 != end2; ++it2) {
                    selection->add(motion->findBoneKeyframe(scope.value(), *it2));
                }
            }
        }
        for (Motion::FrameIndexSet::const_iterator it = m_selectedCameraKeyframeIndices.begin(),
                                                   end = m_selectedCameraKeyframeIndices.end();
             it != end; ++it) {
            selection->add(motion->findCameraKeyframe(*it));
        }
        for (Motion::FrameIndexSet::const_iterator it = m_selectedLightKeyframeIndices.begin(),
                                                   end = m_selectedLightKeyframeIndices.end();
             it != end; ++it) {
            selection->add(motion->findLightKeyframe(*it));
        }
        for (Motion::FrameIndexSet::const_iterator it = m_selectedModelKeyframeIndices.begin(),
                                                   end = m_selectedModelKeyframeIndices.end();
             it != end; ++it) {
            selection->add(motion->findModelKeyframe(*it));
        }
        for (NamedFrameIndexMap::const_iterator it = m_selectedMorphKeyframeIndices.begin(),
                                                end = m_selectedMorphKeyframeIndices.end();
             it != end; ++it) {
            if (StringUtils::tryGetString(factory, it->first, scope)) {
                for (Motion::FrameIndexSet::const_iterator it2 = it->second.begin(), end2 = it->second.end();
                     it2 != end2; ++it2) {
                    selection->add(motion->findMorphKeyframe(scope.value(), *it2));
                }
            }
        }
    }

    Motion::FrameIndexSet m_selectedAccessoryKeyframeIndices;
    NamedFrameIndexMap m_selectedBoneKeyframeIndices;
    Motion::FrameIndexSet m_selectedCameraKeyframeIndices;
    Motion::FrameIndexSet m_selectedLightKeyframeIndices;
    Motion::FrameIndexSet m_selectedModelKeyframeIndices;
    NamedFrameIndexMap m_selectedMorphKeyframeIndices;
};

StringList
Motion::loadableExtensions()
{
    static const String kLoadableMotionExtensions[] = { kNMDFormatExtension, kVMDFormatExtension, String() };
    return StringList(
        &kLoadableMotionExtensions[0], &kLoadableMotionExtensions[BX_COUNTOF(kLoadableMotionExtensions) - 1]);
}

StringSet
Motion::loadableExtensionsSet()
{
    return ListUtils::toSetFromList<String>(loadableExtensions());
}

bool
Motion::isLoadableExtension(const String &extension)
{
    return FileUtils::isLoadableExtension(extension, loadableExtensionsSet());
}

bool
Motion::isLoadableExtension(const URI &fileURI)
{
    return isLoadableExtension(fileURI.pathExtension());
}

bool
Motion::addFrameIndexDelta(
    int value, nanoem_frame_index_t frameIndex, nanoem_frame_index_t &newFrameIndex) NANOEM_DECL_NOEXCEPT
{
    bool result = false;
    if (value > 0) {
        const nanoem_frame_index_t delta = static_cast<nanoem_frame_index_t>(value);
        if (frameIndex <= kMaxFrameIndex - delta) {
            newFrameIndex = frameIndex + delta;
            result = true;
        }
    }
    else if (value < 0) {
        const nanoem_frame_index_t delta = static_cast<nanoem_frame_index_t>(glm::abs(value));
        if (frameIndex >= delta) {
            newFrameIndex = frameIndex - delta;
            result = true;
        }
    }
    return result;
}

bool
Motion::subtractFrameIndexDelta(
    int value, nanoem_frame_index_t frameIndex, nanoem_frame_index_t &newFrameIndex) NANOEM_DECL_NOEXCEPT
{
    bool result = false;
    if (value > 0) {
        const nanoem_frame_index_t delta = static_cast<nanoem_frame_index_t>(value);
        if (frameIndex >= delta) {
            newFrameIndex = frameIndex - delta;
            result = true;
        }
    }
    else if (value < 0) {
        const nanoem_frame_index_t delta = static_cast<nanoem_frame_index_t>(glm::abs(value));
        if (frameIndex <= kMaxFrameIndex - delta) {
            newFrameIndex = frameIndex + delta;
            result = true;
        }
    }
    return result;
}

void
Motion::copyAllAccessoryKeyframes(nanoem_motion_accessory_keyframe_t *const *keyframes, nanoem_rsize_t numKeyframes,
    nanoem_mutable_motion_t *motion, int offset, nanoem_status_t &status)
{
    nanoem_motion_t *originMotion = nanoemMutableMotionGetOriginObject(motion);
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        const nanoem_motion_accessory_keyframe_t *accessoryKeyframe = keyframes[i];
        nanoem_mutable_motion_accessory_keyframe_t *mutableAccessoryKeyframe =
            nanoemMutableMotionAccessoryKeyframeCreate(originMotion, &status);
        nanoem_frame_index_t frameIndex = nanoemMotionKeyframeObjectGetFrameIndexWithOffset(
            nanoemMotionAccessoryKeyframeGetKeyframeObject(accessoryKeyframe), offset);
        nanoemMutableMotionAccessoryKeyframeCopy(mutableAccessoryKeyframe, accessoryKeyframe, &status);
        copyAccessoryOutsideParent(accessoryKeyframe, mutableAccessoryKeyframe);
        nanoemMutableMotionAddAccessoryKeyframe(motion, mutableAccessoryKeyframe, frameIndex, &status);
        nanoemMutableMotionAccessoryKeyframeDestroy(mutableAccessoryKeyframe);
        if (status == NANOEM_STATUS_ERROR_MOTION_ACCESSORY_KEYFRAME_ALREADY_EXISTS) {
            status = NANOEM_STATUS_SUCCESS;
        }
        else if (status != NANOEM_STATUS_SUCCESS) {
            break;
        }
    }
    nanoemMutableMotionSortAllKeyframes(motion);
}

void
Motion::copyAllAccessoryKeyframes(
    const nanoem_motion_t *source, nanoem_mutable_motion_t *motion, int offset, nanoem_status_t &status)
{
    nanoem_parameter_assert(motion, "must not be nullptr");
    nanoem_rsize_t numKeyframes;
    nanoem_motion_accessory_keyframe_t *const *keyframes =
        nanoemMotionGetAllAccessoryKeyframeObjects(source, &numKeyframes);
    copyAllAccessoryKeyframes(keyframes, numKeyframes, motion, offset, status);
}

void
Motion::copyAllBoneKeyframes(nanoem_motion_bone_keyframe_t *const *keyframes, nanoem_rsize_t numKeyframes,
    const IMotionKeyframeSelection *selection, const Model *model, nanoem_mutable_motion_t *motion, int offset,
    nanoem_status_t &status)
{
    if (model) {
        nanoem_motion_t *originMotion = nanoemMutableMotionGetOriginObject(motion);
        for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
            const nanoem_motion_bone_keyframe_t *keyframe = keyframes[i];
            const nanoem_unicode_string_t *name = nanoemMotionBoneKeyframeGetName(keyframe);
            if (model->containsBone(name)) {
                nanoem_frame_index_t frameIndex = nanoemMotionKeyframeObjectGetFrameIndexWithOffset(
                    nanoemMotionBoneKeyframeGetKeyframeObject(keyframe), offset);
                nanoem_mutable_motion_bone_keyframe_t *mutableBoneKeyframe =
                    nanoemMutableMotionBoneKeyframeCreate(originMotion, &status);
                nanoemMutableMotionBoneKeyframeCopy(mutableBoneKeyframe, keyframe);
                nanoemMotionKeyframeObjectSetSelected(
                    nanoemMotionBoneKeyframeGetKeyframeObjectMutable(
                        nanoemMutableMotionBoneKeyframeGetOriginObject(mutableBoneKeyframe)),
                    selection ? selection->contains(keyframe) : false);
                nanoemMutableMotionAddBoneKeyframe(motion, mutableBoneKeyframe, name, frameIndex, &status);
                nanoemMutableMotionBoneKeyframeDestroy(mutableBoneKeyframe);
                if (status == NANOEM_STATUS_ERROR_MOTION_BONE_KEYFRAME_ALREADY_EXISTS) {
                    status = NANOEM_STATUS_SUCCESS;
                }
                else if (status != NANOEM_STATUS_SUCCESS) {
                    break;
                }
            }
        }
        nanoemMutableMotionSortAllKeyframes(motion);
    }
}

void
Motion::copyAllBoneKeyframes(const nanoem_motion_t *source, const IMotionKeyframeSelection *selection,
    const Model *model, nanoem_mutable_motion_t *motion, int offset, nanoem_status_t &status)
{
    nanoem_rsize_t numKeyframes;
    nanoem_motion_bone_keyframe_t *const *keyframes = nanoemMotionGetAllBoneKeyframeObjects(source, &numKeyframes);
    copyAllBoneKeyframes(keyframes, numKeyframes, selection, model, motion, offset, status);
}

void
Motion::copyAllCameraKeyframes(nanoem_motion_camera_keyframe_t *const *keyframes, nanoem_rsize_t numKeyframes,
    nanoem_mutable_motion_t *motion, int offset, nanoem_status_t &status)
{
    nanoem_motion_t *originMotion = nanoemMutableMotionGetOriginObject(motion);
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        const nanoem_motion_camera_keyframe_t *keyframe = keyframes[i];
        nanoem_mutable_motion_camera_keyframe_t *mutableCameraKeyframe =
            nanoemMutableMotionCameraKeyframeCreate(originMotion, &status);
        nanoem_frame_index_t frameIndex = nanoemMotionKeyframeObjectGetFrameIndexWithOffset(
            nanoemMotionCameraKeyframeGetKeyframeObject(keyframe), offset);
        nanoemMutableMotionCameraKeyframeCopy(mutableCameraKeyframe, keyframe);
        nanoemMutableMotionAddCameraKeyframe(motion, mutableCameraKeyframe, frameIndex, &status);
        nanoemMutableMotionCameraKeyframeDestroy(mutableCameraKeyframe);
        if (status == NANOEM_STATUS_ERROR_MOTION_CAMERA_KEYFRAME_ALREADY_EXISTS) {
            status = NANOEM_STATUS_SUCCESS;
        }
        else if (status != NANOEM_STATUS_SUCCESS) {
            break;
        }
    }
    nanoemMutableMotionSortAllKeyframes(motion);
}

void
Motion::copyAllCameraKeyframes(
    const nanoem_motion_t *source, nanoem_mutable_motion_t *motion, int offset, nanoem_status_t &status)
{
    nanoem_parameter_assert(motion, "must not be nullptr");
    nanoem_rsize_t numKeyframes;
    nanoem_motion_camera_keyframe_t *const *keyframes = nanoemMotionGetAllCameraKeyframeObjects(source, &numKeyframes);
    copyAllCameraKeyframes(keyframes, numKeyframes, motion, offset, status);
}

void
Motion::copyAllLightKeyframes(nanoem_motion_light_keyframe_t *const *keyframes, nanoem_rsize_t numKeyframes,
    nanoem_mutable_motion_t *motion, int offset, nanoem_status_t &status)
{
    nanoem_motion_t *originMotion = nanoemMutableMotionGetOriginObject(motion);
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        const nanoem_motion_light_keyframe_t *keyframe = keyframes[i];
        nanoem_mutable_motion_light_keyframe_t *mutableLightKeyframe =
            nanoemMutableMotionLightKeyframeCreate(originMotion, &status);
        nanoem_frame_index_t frameIndex = nanoemMotionKeyframeObjectGetFrameIndexWithOffset(
            nanoemMotionLightKeyframeGetKeyframeObject(keyframe), offset);
        nanoemMutableMotionLightKeyframeCopy(mutableLightKeyframe, keyframe);
        nanoemMutableMotionAddLightKeyframe(motion, mutableLightKeyframe, frameIndex, &status);
        nanoemMutableMotionLightKeyframeDestroy(mutableLightKeyframe);
        if (status == NANOEM_STATUS_ERROR_MOTION_LIGHT_KEYFRAME_ALREADY_EXISTS) {
            status = NANOEM_STATUS_SUCCESS;
        }
        else if (status != NANOEM_STATUS_SUCCESS) {
            break;
        }
    }
    nanoemMutableMotionSortAllKeyframes(motion);
}

void
Motion::copyAllLightKeyframes(
    const nanoem_motion_t *source, nanoem_mutable_motion_t *motion, int offset, nanoem_status_t &status)
{
    nanoem_parameter_assert(motion, "must not be nullptr");
    nanoem_rsize_t numKeyframes;
    nanoem_motion_light_keyframe_t *const *keyframes = nanoemMotionGetAllLightKeyframeObjects(source, &numKeyframes);
    copyAllLightKeyframes(keyframes, numKeyframes, motion, offset, status);
}

void
Motion::copyAllModelKeyframes(nanoem_motion_model_keyframe_t *const *keyframes, nanoem_rsize_t numKeyframes,
    const IMotionKeyframeSelection *selection, nanoem_mutable_motion_t *motion, int offset, nanoem_status_t &status)
{
    nanoem_motion_t *originMotion = nanoemMutableMotionGetOriginObject(motion);
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        const nanoem_motion_model_keyframe_t *keyframe = keyframes[i];
        nanoem_mutable_motion_model_keyframe_t *mutableModelKeyframe =
            nanoemMutableMotionModelKeyframeCreate(originMotion, &status);
        nanoem_frame_index_t frameIndex = nanoemMotionKeyframeObjectGetFrameIndexWithOffset(
            nanoemMotionModelKeyframeGetKeyframeObject(keyframe), offset);
        nanoemMutableMotionModelKeyframeCopy(mutableModelKeyframe, keyframe, &status);
        nanoemMotionKeyframeObjectSetSelected(
            nanoemMotionModelKeyframeGetKeyframeObjectMutable(
                nanoemMutableMotionModelKeyframeGetOriginObject(mutableModelKeyframe)),
            selection ? selection->contains(keyframe) : false);
        nanoemMutableMotionAddModelKeyframe(motion, mutableModelKeyframe, frameIndex, &status);
        nanoemMutableMotionModelKeyframeDestroy(mutableModelKeyframe);
        if (status == NANOEM_STATUS_ERROR_MOTION_MODEL_KEYFRAME_ALREADY_EXISTS) {
            status = NANOEM_STATUS_SUCCESS;
        }
        else if (status != NANOEM_STATUS_SUCCESS) {
            break;
        }
    }
}

void
Motion::copyAllModelKeyframes(const nanoem_motion_t *source, const IMotionKeyframeSelection *selection,
    nanoem_mutable_motion_t *motion, int offset, nanoem_status_t &status)
{
    nanoem_rsize_t numKeyframes;
    nanoem_motion_model_keyframe_t *const *modelKeyframes =
        nanoemMotionGetAllModelKeyframeObjects(source, &numKeyframes);
    copyAllModelKeyframes(modelKeyframes, numKeyframes, selection, motion, offset, status);
}

void
Motion::copyAllMorphKeyframes(nanoem_motion_morph_keyframe_t *const *keyframes, nanoem_rsize_t numKeyframes,
    const IMotionKeyframeSelection *selection, const Model *model, nanoem_mutable_motion_t *motion, int offset,
    nanoem_status_t &status)
{
    if (model) {
        nanoem_motion_t *originMotion = nanoemMutableMotionGetOriginObject(motion);
        for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
            const nanoem_motion_morph_keyframe_t *keyframe = keyframes[i];
            const nanoem_unicode_string_t *name = nanoemMotionMorphKeyframeGetName(keyframe);
            if (model->containsMorph(name)) {
                nanoem_frame_index_t frameIndex = nanoemMotionKeyframeObjectGetFrameIndexWithOffset(
                    nanoemMotionMorphKeyframeGetKeyframeObject(keyframe), offset);
                nanoem_mutable_motion_morph_keyframe_t *mutableMorphKeyframe =
                    nanoemMutableMotionMorphKeyframeCreate(originMotion, &status);
                nanoemMutableMotionMorphKeyframeCopy(mutableMorphKeyframe, keyframe);
                nanoemMotionKeyframeObjectSetSelected(
                    nanoemMotionMorphKeyframeGetKeyframeObjectMutable(
                        nanoemMutableMotionMorphKeyframeGetOriginObject(mutableMorphKeyframe)),
                    selection ? selection->contains(keyframe) : false);
                nanoemMutableMotionAddMorphKeyframe(motion, mutableMorphKeyframe, name, frameIndex, &status);
                nanoemMutableMotionMorphKeyframeDestroy(mutableMorphKeyframe);
                if (status == NANOEM_STATUS_ERROR_MOTION_MORPH_KEYFRAME_ALREADY_EXISTS) {
                    status = NANOEM_STATUS_SUCCESS;
                }
                else if (status != NANOEM_STATUS_SUCCESS) {
                    break;
                }
            }
        }
        nanoemMutableMotionSortAllKeyframes(motion);
    }
}

void
Motion::copyAllMorphKeyframes(const nanoem_motion_t *source, const IMotionKeyframeSelection *selection,
    const Model *model, nanoem_mutable_motion_t *motion, int offset, nanoem_status_t &status)
{
    nanoem_rsize_t numKeyframes;
    nanoem_motion_morph_keyframe_t *const *keyframes = nanoemMotionGetAllMorphKeyframeObjects(source, &numKeyframes);
    copyAllMorphKeyframes(keyframes, numKeyframes, selection, model, motion, offset, status);
}

void
Motion::copyAllSelfShadowKeyframes(nanoem_motion_self_shadow_keyframe_t *const *keyframes, nanoem_rsize_t numKeyframes,
    nanoem_mutable_motion_t *motion, int offset, nanoem_status_t &status)
{
    nanoem_motion_t *originMotion = nanoemMutableMotionGetOriginObject(motion);
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        const nanoem_motion_self_shadow_keyframe_t *keyframe = keyframes[i];
        nanoem_mutable_motion_self_shadow_keyframe_t *mutableSelfShadowKeyframe =
            nanoemMutableMotionSelfShadowKeyframeCreate(originMotion, &status);
        nanoem_frame_index_t frameIndex = nanoemMotionKeyframeObjectGetFrameIndexWithOffset(
            nanoemMotionSelfShadowKeyframeGetKeyframeObject(keyframe), offset);
        nanoemMutableMotionSelfShadowKeyframeCopy(mutableSelfShadowKeyframe, keyframe);
        nanoemMutableMotionAddSelfShadowKeyframe(motion, mutableSelfShadowKeyframe, frameIndex, &status);
        nanoemMutableMotionSelfShadowKeyframeDestroy(mutableSelfShadowKeyframe);
        if (status == NANOEM_STATUS_ERROR_MOTION_SELF_SHADOW_KEYFRAME_ALREADY_EXISTS) {
            status = NANOEM_STATUS_SUCCESS;
        }
        else if (status != NANOEM_STATUS_SUCCESS) {
            break;
        }
    }
    nanoemMutableMotionSortAllKeyframes(motion);
}

void
Motion::copyAllSelfShadowKeyframes(
    const nanoem_motion_t *source, nanoem_mutable_motion_t *motion, int offset, nanoem_status_t &status)
{
    nanoem_parameter_assert(motion, "must not be nullptr");
    nanoem_rsize_t numKeyframes;
    nanoem_motion_self_shadow_keyframe_t *const *keyframes =
        nanoemMotionGetAllSelfShadowKeyframeObjects(source, &numKeyframes);
    copyAllSelfShadowKeyframes(keyframes, numKeyframes, motion, offset, status);
}

void
Motion::sortAllKeyframes(AccessoryKeyframeList &keyframes, SortDirectionType type) NANOEM_DECL_NOEXCEPT
{
    struct Sorter {
        static int
        sortAscend(const void *left, const void *right) NANOEM_DECL_NOEXCEPT
        {
            const nanoem_motion_accessory_keyframe_t *lk =
                *static_cast<nanoem_motion_accessory_keyframe_t *const *>(left);
            const nanoem_motion_accessory_keyframe_t *rk =
                *static_cast<nanoem_motion_accessory_keyframe_t *const *>(right);
            return compareKeyframeAscend(
                nanoemMotionAccessoryKeyframeGetKeyframeObject(lk), nanoemMotionAccessoryKeyframeGetKeyframeObject(rk));
        }
        static int
        sortDescend(const void *left, const void *right) NANOEM_DECL_NOEXCEPT
        {
            const nanoem_motion_accessory_keyframe_t *lk =
                *static_cast<nanoem_motion_accessory_keyframe_t *const *>(left);
            const nanoem_motion_accessory_keyframe_t *rk =
                *static_cast<nanoem_motion_accessory_keyframe_t *const *>(right);
            return compareKeyframeDescend(
                nanoemMotionAccessoryKeyframeGetKeyframeObject(lk), nanoemMotionAccessoryKeyframeGetKeyframeObject(rk));
        }
    };
    qsort(keyframes.data(), keyframes.size(), sizeof(keyframes[0]),
        type == kSortDirectionTypeAscend ? Sorter::sortAscend : Sorter::sortDescend);
}

void
Motion::sortAllKeyframes(BoneKeyframeList &keyframes, SortDirectionType type) NANOEM_DECL_NOEXCEPT
{
    struct Sorter {
        static int
        sortAscend(const void *left, const void *right) NANOEM_DECL_NOEXCEPT
        {
            const nanoem_motion_bone_keyframe_t *lvalue = *static_cast<nanoem_motion_bone_keyframe_t *const *>(left);
            const nanoem_motion_bone_keyframe_t *rvalue = *static_cast<nanoem_motion_bone_keyframe_t *const *>(right);
            return compareKeyframeAscend(
                nanoemMotionBoneKeyframeGetKeyframeObject(lvalue), nanoemMotionBoneKeyframeGetKeyframeObject(rvalue));
        }
        static int
        sortDescend(const void *left, const void *right) NANOEM_DECL_NOEXCEPT
        {
            const nanoem_motion_bone_keyframe_t *lvalue = *static_cast<nanoem_motion_bone_keyframe_t *const *>(left);
            const nanoem_motion_bone_keyframe_t *rvalue = *static_cast<nanoem_motion_bone_keyframe_t *const *>(right);
            return compareKeyframeDescend(
                nanoemMotionBoneKeyframeGetKeyframeObject(lvalue), nanoemMotionBoneKeyframeGetKeyframeObject(rvalue));
        }
    };
    qsort(keyframes.data(), keyframes.size(), sizeof(keyframes[0]),
        type == kSortDirectionTypeAscend ? Sorter::sortAscend : Sorter::sortDescend);
}

void
Motion::sortAllKeyframes(CameraKeyframeList &keyframes, SortDirectionType type) NANOEM_DECL_NOEXCEPT
{
    struct Sorter {
        static int
        sortAscend(const void *left, const void *right) NANOEM_DECL_NOEXCEPT
        {
            const nanoem_motion_camera_keyframe_t *lvalue =
                *static_cast<nanoem_motion_camera_keyframe_t *const *>(left);
            const nanoem_motion_camera_keyframe_t *rvalue =
                *static_cast<nanoem_motion_camera_keyframe_t *const *>(right);
            return compareKeyframeAscend(nanoemMotionCameraKeyframeGetKeyframeObject(lvalue),
                nanoemMotionCameraKeyframeGetKeyframeObject(rvalue));
        }
        static int
        sortDescend(const void *left, const void *right) NANOEM_DECL_NOEXCEPT
        {
            const nanoem_motion_camera_keyframe_t *lvalue =
                *static_cast<nanoem_motion_camera_keyframe_t *const *>(left);
            const nanoem_motion_camera_keyframe_t *rvalue =
                *static_cast<nanoem_motion_camera_keyframe_t *const *>(right);
            return compareKeyframeDescend(nanoemMotionCameraKeyframeGetKeyframeObject(lvalue),
                nanoemMotionCameraKeyframeGetKeyframeObject(rvalue));
        }
    };
    qsort(keyframes.data(), keyframes.size(), sizeof(keyframes[0]),
        type == kSortDirectionTypeAscend ? Sorter::sortAscend : Sorter::sortDescend);
}

void
Motion::sortAllKeyframes(LightKeyframeList &keyframes, SortDirectionType type) NANOEM_DECL_NOEXCEPT
{
    struct Sorter {
        static int
        sortAscend(const void *left, const void *right) NANOEM_DECL_NOEXCEPT
        {
            const nanoem_motion_light_keyframe_t *lvalue = *static_cast<nanoem_motion_light_keyframe_t *const *>(left);
            const nanoem_motion_light_keyframe_t *rvalue = *static_cast<nanoem_motion_light_keyframe_t *const *>(right);
            return compareKeyframeAscend(
                nanoemMotionLightKeyframeGetKeyframeObject(lvalue), nanoemMotionLightKeyframeGetKeyframeObject(rvalue));
        }
        static int
        sortDescend(const void *left, const void *right) NANOEM_DECL_NOEXCEPT
        {
            const nanoem_motion_light_keyframe_t *lvalue = *static_cast<nanoem_motion_light_keyframe_t *const *>(left);
            const nanoem_motion_light_keyframe_t *rvalue = *static_cast<nanoem_motion_light_keyframe_t *const *>(right);
            return compareKeyframeDescend(
                nanoemMotionLightKeyframeGetKeyframeObject(lvalue), nanoemMotionLightKeyframeGetKeyframeObject(rvalue));
        }
    };
    qsort(keyframes.data(), keyframes.size(), sizeof(keyframes[0]),
        type == kSortDirectionTypeAscend ? Sorter::sortAscend : Sorter::sortDescend);
}

void
Motion::sortAllKeyframes(ModelKeyframeList &keyframes, SortDirectionType type) NANOEM_DECL_NOEXCEPT
{
    struct Sorter {
        static int
        sortAscend(const void *left, const void *right) NANOEM_DECL_NOEXCEPT
        {
            const nanoem_motion_model_keyframe_t *lvalue = *static_cast<nanoem_motion_model_keyframe_t *const *>(left);
            const nanoem_motion_model_keyframe_t *rvalue = *static_cast<nanoem_motion_model_keyframe_t *const *>(right);
            return compareKeyframeAscend(
                nanoemMotionModelKeyframeGetKeyframeObject(lvalue), nanoemMotionModelKeyframeGetKeyframeObject(rvalue));
        }
        static int
        sortDescend(const void *left, const void *right) NANOEM_DECL_NOEXCEPT
        {
            const nanoem_motion_model_keyframe_t *lvalue = *static_cast<nanoem_motion_model_keyframe_t *const *>(left);
            const nanoem_motion_model_keyframe_t *rvalue = *static_cast<nanoem_motion_model_keyframe_t *const *>(right);
            return compareKeyframeDescend(
                nanoemMotionModelKeyframeGetKeyframeObject(lvalue), nanoemMotionModelKeyframeGetKeyframeObject(rvalue));
        }
    };
    qsort(keyframes.data(), keyframes.size(), sizeof(keyframes[0]),
        type == kSortDirectionTypeAscend ? Sorter::sortAscend : Sorter::sortDescend);
}

void
Motion::sortAllKeyframes(MorphKeyframeList &keyframes, SortDirectionType type) NANOEM_DECL_NOEXCEPT
{
    struct Sorter {
        static int
        sortAscend(const void *left, const void *right) NANOEM_DECL_NOEXCEPT
        {
            const nanoem_motion_morph_keyframe_t *lvalue = *static_cast<nanoem_motion_morph_keyframe_t *const *>(left);
            const nanoem_motion_morph_keyframe_t *rvalue = *static_cast<nanoem_motion_morph_keyframe_t *const *>(right);
            return compareKeyframeAscend(
                nanoemMotionMorphKeyframeGetKeyframeObject(lvalue), nanoemMotionMorphKeyframeGetKeyframeObject(rvalue));
        }
        static int
        sortDescend(const void *left, const void *right) NANOEM_DECL_NOEXCEPT
        {
            const nanoem_motion_morph_keyframe_t *lvalue = *static_cast<nanoem_motion_morph_keyframe_t *const *>(left);
            const nanoem_motion_morph_keyframe_t *rvalue = *static_cast<nanoem_motion_morph_keyframe_t *const *>(right);
            return compareKeyframeDescend(
                nanoemMotionMorphKeyframeGetKeyframeObject(lvalue), nanoemMotionMorphKeyframeGetKeyframeObject(rvalue));
        }
    };
    qsort(keyframes.data(), keyframes.size(), sizeof(keyframes[0]),
        type == kSortDirectionTypeAscend ? Sorter::sortAscend : Sorter::sortDescend);
}

void
Motion::sortAllKeyframes(SelfShadowKeyframeList &keyframes, SortDirectionType type) NANOEM_DECL_NOEXCEPT
{
    struct Sorter {
        static int
        sortAscend(const void *left, const void *right) NANOEM_DECL_NOEXCEPT
        {
            const nanoem_motion_self_shadow_keyframe_t *lvalue =
                *static_cast<nanoem_motion_self_shadow_keyframe_t *const *>(left);
            const nanoem_motion_self_shadow_keyframe_t *rvalue =
                *static_cast<nanoem_motion_self_shadow_keyframe_t *const *>(right);
            return compareKeyframeAscend(nanoemMotionSelfShadowKeyframeGetKeyframeObject(lvalue),
                nanoemMotionSelfShadowKeyframeGetKeyframeObject(rvalue));
        }
        static int
        sortDescend(const void *left, const void *right) NANOEM_DECL_NOEXCEPT
        {
            const nanoem_motion_self_shadow_keyframe_t *lvalue =
                *static_cast<nanoem_motion_self_shadow_keyframe_t *const *>(left);
            const nanoem_motion_self_shadow_keyframe_t *rvalue =
                *static_cast<nanoem_motion_self_shadow_keyframe_t *const *>(right);
            return compareKeyframeDescend(nanoemMotionSelfShadowKeyframeGetKeyframeObject(lvalue),
                nanoemMotionSelfShadowKeyframeGetKeyframeObject(rvalue));
        }
    };
    qsort(keyframes.data(), keyframes.size(), sizeof(keyframes[0]),
        type == kSortDirectionTypeAscend ? Sorter::sortAscend : Sorter::sortDescend);
}

Motion::Motion(Project *project, nanoem_u16_t handle)
    : m_project(project)
    , m_selection(nullptr)
    , m_opaque(nullptr)
    , m_formatType(NANOEM_MOTION_FORMAT_TYPE_NMD)
    , m_handle(handle)
    , m_dirty(false)
{
    nanoem_assert(m_project, "must not be nullptr");
    m_opaque = nanoemMotionCreate(m_project->unicodeStringFactory(), nullptr);
    m_selection = nanoem_new(internal::MotionKeyframeSelection(this));
}

Motion::~Motion() NANOEM_DECL_NOEXCEPT
{
    for (BezierCurve::Map::const_iterator it = m_bezierCurvesData.begin(), end = m_bezierCurvesData.end(); it != end;
         ++it) {
        nanoem_delete(it->second);
    }
    nanoem_delete_safe(m_selection);
    m_bezierCurvesData.clear();
    nanoemMotionDestroy(m_opaque);
    m_opaque = nullptr;
}

void
Motion::initialize(const Accessory * /* accessory */)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_motion_t *mutableMotion = nanoemMutableMotionCreateAsReference(m_opaque, &status);
    if (!findAccessoryKeyframe(0)) {
        nanoem_mutable_motion_accessory_keyframe_t *mutableAccessoryKeyframe =
            nanoemMutableMotionAccessoryKeyframeCreate(m_opaque, &status);
        nanoemMutableMotionAccessoryKeyframeSetTranslation(
            mutableAccessoryKeyframe, glm::value_ptr(Constants::kZeroV4));
        nanoemMutableMotionAccessoryKeyframeSetOrientation(mutableAccessoryKeyframe, glm::value_ptr(Constants::kZeroQ));
        nanoemMutableMotionAccessoryKeyframeSetOpacity(mutableAccessoryKeyframe, 1.0f);
        nanoemMutableMotionAccessoryKeyframeSetScaleFactor(mutableAccessoryKeyframe, 1.0f);
        nanoemMutableMotionAccessoryKeyframeSetShadowEnabled(mutableAccessoryKeyframe, 1);
        nanoemMutableMotionAccessoryKeyframeSetAddBlendEnabled(mutableAccessoryKeyframe, 0);
        nanoemMutableMotionAccessoryKeyframeSetVisible(mutableAccessoryKeyframe, 1);
        nanoemMutableMotionAddAccessoryKeyframe(mutableMotion, mutableAccessoryKeyframe, 0, &status);
        nanoemMutableMotionAccessoryKeyframeDestroy(mutableAccessoryKeyframe);
    }
    nanoemMutableMotionDestroy(mutableMotion);
}

void
Motion::initialize(const Model *model)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_motion_t *mutableMotion = nanoemMutableMotionCreateAsReference(m_opaque, &status);
    nanoem_rsize_t numObjects;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(model->data(), &numObjects);
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        const nanoem_model_bone_t *bonePtr = bones[i];
        const nanoem_unicode_string_t *name = nanoemModelBoneGetName(bonePtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        if (!findBoneKeyframe(name, 0)) {
            const model::Bone *bone = model::Bone::cast(bonePtr);
            nanoem_mutable_motion_bone_keyframe_t *mutableBoneKeyframe =
                nanoemMutableMotionBoneKeyframeCreate(m_opaque, &status);
            nanoemMutableMotionBoneKeyframeSetTranslation(
                mutableBoneKeyframe, glm::value_ptr(Vector4(bone->localUserTranslation(), 1)));
            nanoemMutableMotionBoneKeyframeSetOrientation(
                mutableBoneKeyframe, glm::value_ptr(bone->localUserOrientation()));
            for (int j = NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM;
                 j < NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; j++) {
                nanoemMutableMotionBoneKeyframeSetInterpolation(mutableBoneKeyframe,
                    nanoem_motion_bone_keyframe_interpolation_type_t(j),
                    glm::value_ptr(model::Bone::kDefaultBezierControlPoint));
            }
            nanoemMutableMotionBoneKeyframeSetStageIndex(mutableBoneKeyframe, 0);
            nanoemMutableMotionBoneKeyframeSetPhysicsSimulationEnabled(mutableBoneKeyframe, 1);
            nanoemMutableMotionAddBoneKeyframe(mutableMotion, mutableBoneKeyframe, name, 0, &status);
            nanoemMutableMotionBoneKeyframeDestroy(mutableBoneKeyframe);
        }
    }
    nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(model->data(), &numObjects);
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        const nanoem_model_morph_t *morphPtr = morphs[i];
        const nanoem_unicode_string_t *name = nanoemModelMorphGetName(morphPtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        if (!findMorphKeyframe(name, 0)) {
            const model::Morph *morph = model::Morph::cast(morphPtr);
            nanoem_mutable_motion_morph_keyframe_t *mutableMorphKeyframe =
                nanoemMutableMotionMorphKeyframeCreate(m_opaque, &status);
            nanoemMutableMotionMorphKeyframeSetWeight(mutableMorphKeyframe, morph->weight());
            nanoemMutableMotionAddMorphKeyframe(mutableMotion, mutableMorphKeyframe, name, 0, &status);
            nanoemMutableMotionMorphKeyframeDestroy(mutableMorphKeyframe);
        }
    }
    if (!findModelKeyframe(0)) {
        nanoem_mutable_motion_model_keyframe_t *mutableModelKeyframe =
            nanoemMutableMotionModelKeyframeCreate(m_opaque, &status);
        nanoemMutableMotionModelKeyframeSetVisible(mutableModelKeyframe, true);
        nanoemMutableMotionModelKeyframeSetPhysicsSimulationEnabled(mutableModelKeyframe, true);
        nanoemMutableMotionModelKeyframeSetAddBlendEnabled(mutableModelKeyframe, false);
        nanoemMutableMotionModelKeyframeSetEdgeColor(mutableModelKeyframe, glm::value_ptr(Vector4(0, 0, 0, 1)));
        nanoemMutableMotionModelKeyframeSetEdgeScaleFactor(mutableModelKeyframe, 1.0f);
        if (nanoem_model_constraint_t *const *constraints =
                nanoemModelGetAllConstraintObjects(model->data(), &numObjects)) {
            for (nanoem_rsize_t i = 0; i < numObjects; i++) {
                const nanoem_model_constraint_t *constraint = constraints[i];
                const nanoem_model_bone_t *targetBone = nanoemModelConstraintGetTargetBoneObject(constraint);
                const nanoem_unicode_string_t *name =
                    nanoemModelBoneGetName(targetBone, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
                nanoem_mutable_motion_model_keyframe_constraint_state_t *mutableConstraintState =
                    nanoemMutableMotionModelKeyframeConstraintStateCreateMutable(mutableModelKeyframe, &status);
                nanoemMutableMotionModelKeyframeConstraintStateSetBoneName(mutableConstraintState, name, &status);
                nanoemMutableMotionModelKeyframeConstraintStateSetEnabled(mutableConstraintState, true);
                nanoemMutableMotionModelKeyframeAddConstraintState(
                    mutableModelKeyframe, mutableConstraintState, &status);
                nanoemMutableMotionModelKeyframeConstraintStateDestroy(mutableConstraintState);
            }
        }
        else {
            nanoemModelGetAllBoneObjects(model->data(), &numObjects);
            for (nanoem_rsize_t i = 0; i < numObjects; i++) {
                const nanoem_model_bone_t *bone = bones[i];
                if (nanoemModelBoneGetConstraintObject(bone)) {
                    nanoem_mutable_motion_model_keyframe_constraint_state_t *mutableConstraintState =
                        nanoemMutableMotionModelKeyframeConstraintStateCreateMutable(mutableModelKeyframe, &status);
                    const nanoem_unicode_string_t *name = nanoemModelBoneGetName(bone, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
                    nanoemMutableMotionModelKeyframeConstraintStateSetBoneName(mutableConstraintState, name, &status);
                    nanoemMutableMotionModelKeyframeConstraintStateSetEnabled(mutableConstraintState, true);
                    nanoemMutableMotionModelKeyframeAddConstraintState(
                        mutableModelKeyframe, mutableConstraintState, &status);
                    nanoemMutableMotionModelKeyframeConstraintStateDestroy(mutableConstraintState);
                }
            }
        }
        nanoemMutableMotionAddModelKeyframe(mutableMotion, mutableModelKeyframe, 0, &status);
        nanoemMutableMotionModelKeyframeDestroy(mutableModelKeyframe);
    }
    nanoemMutableMotionDestroy(mutableMotion);
}

void
Motion::initialize(const ILight *light)
{
    if (!findLightKeyframe(0)) {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoem_mutable_motion_t *m = nanoemMutableMotionCreateAsReference(m_opaque, &status);
        nanoem_mutable_motion_light_keyframe_t *k = nanoemMutableMotionLightKeyframeCreate(m_opaque, &status);
        nanoemMutableMotionLightKeyframeSetColor(k, glm::value_ptr(Vector4(light->color(), 1)));
        nanoemMutableMotionLightKeyframeSetDirection(k, glm::value_ptr(Vector4(light->direction(), 0)));
        nanoemMutableMotionAddLightKeyframe(m, k, 0, &status);
        nanoemMutableMotionLightKeyframeDestroy(k);
        nanoemMutableMotionDestroy(m);
    }
}

void
Motion::initialize(const ICamera *camera)
{
    if (!findCameraKeyframe(0)) {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoem_mutable_motion_t *m = nanoemMutableMotionCreateAsReference(m_opaque, &status);
        nanoem_mutable_motion_camera_keyframe_t *k = nanoemMutableMotionCameraKeyframeCreate(m_opaque, &status);
        nanoemMutableMotionCameraKeyframeSetLookAt(k, glm::value_ptr(Vector4(camera->lookAt(), 0)));
        nanoemMutableMotionCameraKeyframeSetAngle(k, glm::value_ptr(Vector4(camera->angle(), 0)));
        nanoemMutableMotionCameraKeyframeSetFov(k, camera->fov());
        nanoemMutableMotionCameraKeyframeSetDistance(k, -camera->distance());
        nanoemMutableMotionCameraKeyframeSetPerspectiveView(k, camera->isPerspective());
        for (int i = NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM;
             i < NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; i++) {
            nanoem_motion_camera_keyframe_interpolation_type_t type =
                nanoem_motion_camera_keyframe_interpolation_type_t(i);
            nanoemMutableMotionCameraKeyframeSetInterpolation(
                k, type, glm::value_ptr(PerspectiveCamera::kDefaultBezierControlPoint));
        }
        nanoemMutableMotionAddCameraKeyframe(m, k, 0, &status);
        nanoemMutableMotionCameraKeyframeDestroy(k);
        nanoemMutableMotionDestroy(m);
    }
}

void
Motion::initialize(const ShadowCamera *shadow)
{
    if (!findSelfShadowKeyframe(0)) {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoem_mutable_motion_t *m = nanoemMutableMotionCreateAsReference(m_opaque, &status);
        nanoem_mutable_motion_self_shadow_keyframe_t *k =
            nanoemMutableMotionSelfShadowKeyframeCreate(m_opaque, &status);
        nanoemMutableMotionSelfShadowKeyframeSetDistance(k, shadow->distance());
        nanoemMutableMotionSelfShadowKeyframeSetMode(k, shadow->coverageMode());
        nanoemMutableMotionAddSelfShadowKeyframe(m, k, 0, &status);
        nanoemMutableMotionSelfShadowKeyframeDestroy(k);
        nanoemMutableMotionDestroy(m);
    }
}

bool
Motion::load(const nanoem_u8_t *bytes, size_t length, nanoem_frame_index_t offset, Error &error)
{
    nanoem_parameter_assert(bytes, "must not be nullptr");
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_buffer_t *buffer = nanoemBufferCreate(bytes, length, &status);
    switch (m_formatType) {
    case NANOEM_MOTION_FORMAT_TYPE_NMD: {
        nanoemMotionLoadFromBufferNMD(m_opaque, buffer, offset, &status);
        break;
    }
    case NANOEM_MOTION_FORMAT_TYPE_VMD: {
        nanoemMotionLoadFromBuffer(m_opaque, buffer, offset, &status);
        break;
    }
    default:
        break;
    }
    nanoemBufferDestroy(buffer);
    bool succeeded = status == NANOEM_STATUS_SUCCESS;
    if (!succeeded) {
        char message[Error::kMaxReasonLength];
        StringUtils::format(message, sizeof(message), "Cannot load the motion: %s",
            Error::convertStatusToMessage(status, m_project->translator()));
        error = Error(message, status, Error::kDomainTypeNanoem);
    }
    return succeeded;
}

bool
Motion::load(const ByteArray &bytes, nanoem_frame_index_t offset, Error &error)
{
    return load(bytes.data(), bytes.size(), offset, error);
}

bool
Motion::save(IWriter *writer, const Model *model, nanoem_u32_t flags, Error &error) const
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_motion_t *mutableMotion = nanoemMutableMotionCreate(m_project->unicodeStringFactory(), &status);
    bool result = false;
    if (EnumUtils::isEnabled(flags, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ACCESSORY)) {
        copyAllAccessoryKeyframes(data(), mutableMotion, 0, status);
    }
    if (status == NANOEM_STATUS_SUCCESS && EnumUtils::isEnabled(flags, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_CAMERA)) {
        copyAllCameraKeyframes(data(), mutableMotion, 0, status);
    }
    if (status == NANOEM_STATUS_SUCCESS && EnumUtils::isEnabled(flags, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_LIGHT)) {
        copyAllLightKeyframes(data(), mutableMotion, 0, status);
    }
    if (status == NANOEM_STATUS_SUCCESS && EnumUtils::isEnabled(flags, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MODEL) &&
        model) {
        copyAllModelKeyframeParameters(data(), m_selection, model, mutableMotion, 0, status);
        if (status == NANOEM_STATUS_SUCCESS) {
            nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
            StringUtils::UnicodeStringScope us(factory);
            if (StringUtils::tryGetString(factory, model->canonicalName(), us)) {
                nanoemMutableMotionSetTargetModelName(mutableMotion, us.value(), &status);
            }
        }
    }
    if (status == NANOEM_STATUS_SUCCESS &&
        EnumUtils::isEnabled(flags, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_SELFSHADOW)) {
        copyAllSelfShadowKeyframes(data(), mutableMotion, 0, status);
    }
    if (status == NANOEM_STATUS_SUCCESS &&
        (EnumUtils::isEnabled(flags, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_CAMERA) ||
            EnumUtils::isEnabled(flags, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_LIGHT))) {
        nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
        StringUtils::UnicodeStringScope us(factory);
        if (StringUtils::tryGetString(factory, reinterpret_cast<const char *>(kCameraAndLightTargetModelName), us)) {
            nanoemMutableMotionSetTargetModelName(mutableMotion, us.value(), &status);
        }
    }
    if (status == NANOEM_STATUS_SUCCESS) {
        result = internalSave(mutableMotion, writer, error);
    }
    else {
        char message[Error::kMaxReasonLength];
        StringUtils::format(message, sizeof(message), "Cannot save the motion: %s",
            Error::convertStatusToMessage(status, m_project->translator()));
        error = Error(message, status, Error::kDomainTypeNanoem);
    }
    return result;
}

bool
Motion::save(ByteArray &bytes, const Model *model, nanoem_u32_t flags, Error &error) const
{
    MemoryWriter writer(&bytes);
    return save(&writer, model, flags, error);
}

void
Motion::writeLoadCameraCommandMessage(const URI &fileURI, Error &error)
{
    internalWriteLoadCommandMessage(
        NANOEM__APPLICATION__REDO_LOAD_MOTION_COMMAND__TYPE__CAMERA, bx::kInvalidHandle, fileURI, error);
}

void
Motion::writeLoadLightCommandMessage(const URI &fileURI, Error &error)
{
    internalWriteLoadCommandMessage(
        NANOEM__APPLICATION__REDO_LOAD_MOTION_COMMAND__TYPE__LIGHT, bx::kInvalidHandle, fileURI, error);
}

void
Motion::writeLoadModelCommandMessage(nanoem_u16_t handle, const URI &fileURI, Error &error)
{
    internalWriteLoadCommandMessage(NANOEM__APPLICATION__REDO_LOAD_MOTION_COMMAND__TYPE__MODEL, handle, fileURI, error);
}

void
Motion::mergeAllKeyframes(const Motion *source)
{
    internalMergeAllKeyframes(source, false, false);
}

void
Motion::overrideAllKeyframes(const Motion *source, bool reverse)
{
    internalMergeAllKeyframes(source, true, reverse);
}

void
Motion::clearAllKeyframes()
{
    for (BezierCurve::Map::const_iterator it = m_bezierCurvesData.begin(), end = m_bezierCurvesData.end(); it != end;
         ++it) {
        nanoem_delete(it->second);
    }
    m_bezierCurvesData.clear();
    m_keyframeBezierCurves.clear();
    m_selection->clearAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL);
    nanoemMotionDestroy(m_opaque);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_opaque = nanoemMotionCreate(m_project->unicodeStringFactory(), &status);
    m_dirty = false;
}

void
Motion::correctAllSelectedBoneKeyframes(
    const CorrectionVectorFactor &translation, const CorrectionVectorFactor &orientation)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    Motion::BoneKeyframeList keyframes;
    m_selection->getAll(keyframes, nullptr);
    for (BoneKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end; ++it) {
        const nanoem_unicode_string_t *name = nanoemMotionBoneKeyframeGetName(*it);
        const nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionBoneKeyframeGetKeyframeObject(*it));
        if (nanoem_mutable_motion_bone_keyframe_t *keyframe =
                nanoemMutableMotionBoneKeyframeCreateByFound(m_opaque, name, frameIndex, &status)) {
            nanoem_motion_bone_keyframe_t *origin = nanoemMutableMotionBoneKeyframeGetOriginObject(keyframe);
            const Vector3 t(glm::make_vec3(nanoemMotionBoneKeyframeGetTranslation(origin)));
            const Vector3 &actualTranslation =
                t * Constants::kTranslateDirection * translation.m_mul + translation.m_add;
            nanoemMutableMotionBoneKeyframeSetTranslation(keyframe, glm::value_ptr(Vector4(actualTranslation, 1)));
            const Quaternion &o = glm::make_quat(glm::value_ptr(
                glm::make_vec4(nanoemMotionBoneKeyframeGetOrientation(origin)) * Constants::kOrientateDirection));
            const Vector3 &actualOrientation =
                glm::degrees(glm::eulerAngles(o)) * orientation.m_mul + orientation.m_add;
            nanoemMutableMotionBoneKeyframeSetOrientation(
                keyframe, glm::value_ptr(Quaternion(glm::radians(actualOrientation))));
            nanoemMutableMotionBoneKeyframeDestroy(keyframe);
        }
    }
}

void
Motion::correctAllSelectedCameraKeyframes(
    const CorrectionVectorFactor &lookAt, const CorrectionVectorFactor &angle, const CorrectionScalarFactor &distance)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    Motion::CameraKeyframeList keyframes;
    m_selection->getAll(keyframes, nullptr);
    for (CameraKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end; ++it) {
        const nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionCameraKeyframeGetKeyframeObject(*it));
        if (nanoem_mutable_motion_camera_keyframe_t *keyframe =
                nanoemMutableMotionCameraKeyframeCreateByFound(m_opaque, frameIndex, &status)) {
            nanoem_motion_camera_keyframe_t *origin = nanoemMutableMotionCameraKeyframeGetOriginObject(keyframe);
            const Vector3 &actualLookAt =
                glm::make_vec3(nanoemMotionCameraKeyframeGetLookAt(origin)) * lookAt.m_mul + lookAt.m_add;
            nanoemMutableMotionCameraKeyframeSetLookAt(keyframe, glm::value_ptr(Vector4(actualLookAt, 1)));
            const Vector3 &actualAngle = glm::make_vec3(nanoemMotionCameraKeyframeGetAngle(origin)) * angle.m_mul +
                angle.m_add * PerspectiveCamera::kAngleScaleFactor;
            nanoemMutableMotionCameraKeyframeSetAngle(keyframe, glm::value_ptr(Vector4(actualAngle, 0)));
            nanoemMutableMotionCameraKeyframeSetDistance(
                keyframe, nanoemMotionCameraKeyframeGetDistance(origin) * distance.m_mul + distance.m_add);
            nanoemMutableMotionCameraKeyframeDestroy(keyframe);
        }
    }
}

void
Motion::correctAllSelectedMorphKeyframes(const CorrectionScalarFactor &weight)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    Motion::MorphKeyframeList keyframes;
    m_selection->getAll(keyframes, nullptr);
    for (MorphKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end; ++it) {
        const nanoem_unicode_string_t *name = nanoemMotionMorphKeyframeGetName(*it);
        const nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionMorphKeyframeGetKeyframeObject(*it));
        if (nanoem_mutable_motion_morph_keyframe_t *keyframe =
                nanoemMutableMotionMorphKeyframeCreateByFound(m_opaque, name, frameIndex, &status)) {
            nanoem_motion_morph_keyframe_t *origin = nanoemMutableMotionMorphKeyframeGetOriginObject(keyframe);
            nanoemMutableMotionMorphKeyframeSetWeight(
                keyframe, nanoemMotionMorphKeyframeGetWeight(origin) * weight.m_mul + weight.m_add);
            nanoemMutableMotionMorphKeyframeDestroy(keyframe);
        }
    }
}

void
Motion::scaleAllAccessoryKeyframesIn(nanoem_frame_index_t from, nanoem_frame_index_t to, nanoem_f32_t scaleFactor)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    if (scaleFactor > 1) {
        nanoem_mutable_motion_t *m = nanoemMutableMotionCreateAsReference(m_opaque, &status);
        m_selection->clearAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ACCESSORY);
        for (nanoem_frame_index_t i = to; i > from; i--) {
            if (nanoem_mutable_motion_accessory_keyframe_t *k =
                    nanoemMutableMotionAccessoryKeyframeCreateByFound(m_opaque, i, &status)) {
                const nanoem_frame_index_t dest = static_cast<nanoem_frame_index_t>((i - from) * scaleFactor) + from;
                nanoemMutableMotionRemoveAccessoryKeyframe(m, k, &status);
                nanoemMutableMotionAddAccessoryKeyframe(m, k, dest, &status);
                nanoemMutableMotionAccessoryKeyframeDestroy(k);
            }
        }
        nanoemMutableMotionDestroy(m);
    }
    else if (scaleFactor < 1) {
        nanoem_mutable_motion_t *m = nanoemMutableMotionCreateAsReference(m_opaque, &status);
        m_selection->clearAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ACCESSORY);
        for (nanoem_frame_index_t startFrom = from + 1, i = startFrom; i <= to; i++) {
            const nanoem_frame_index_t dest = static_cast<nanoem_frame_index_t>((i - from) * scaleFactor) + from;
            if (dest == startFrom) {
                continue;
            }
            else if (nanoem_mutable_motion_accessory_keyframe_t *k =
                         nanoemMutableMotionAccessoryKeyframeCreateByFound(m_opaque, i, &status)) {
                nanoemMutableMotionRemoveAccessoryKeyframe(m, k, &status);
                nanoemMutableMotionAddAccessoryKeyframe(m, k, dest, &status);
                nanoem_frame_index_t increment = 0;
                do {
                    nanoemMutableMotionAddAccessoryKeyframe(m, k, dest + increment, &status);
                    increment++;
                } while (status == NANOEM_STATUS_ERROR_MOTION_ACCESSORY_KEYFRAME_ALREADY_EXISTS);
                nanoemMutableMotionAccessoryKeyframeDestroy(k);
            }
        }
        nanoemMutableMotionDestroy(m);
    }
}

void
Motion::scaleAllBoneKeyframesIn(
    const Model *model, nanoem_frame_index_t from, nanoem_frame_index_t to, nanoem_f32_t scaleFactor)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_rsize_t numBones;
    if (scaleFactor > 1) {
        nanoem_mutable_motion_t *m = nanoemMutableMotionCreateAsReference(m_opaque, &status);
        nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(model->data(), &numBones);
        m_selection->clearAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_BONE);
        for (nanoem_frame_index_t i = to; i > from; i--) {
            for (nanoem_rsize_t j = 0; j < numBones; j++) {
                const nanoem_unicode_string_t *name = nanoemModelBoneGetName(bones[j], NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
                if (nanoem_mutable_motion_bone_keyframe_t *k =
                        nanoemMutableMotionBoneKeyframeCreateByFound(m_opaque, name, i, &status)) {
                    const nanoem_frame_index_t dest =
                        static_cast<nanoem_frame_index_t>((i - from) * scaleFactor) + from;
                    nanoemMutableMotionRemoveBoneKeyframe(m, k, &status);
                    nanoemMutableMotionAddBoneKeyframe(m, k, name, dest, &status);
                    nanoemMutableMotionBoneKeyframeDestroy(k);
                }
            }
        }
        nanoemMutableMotionDestroy(m);
    }
    else if (scaleFactor < 1) {
        nanoem_mutable_motion_t *m = nanoemMutableMotionCreateAsReference(m_opaque, &status);
        nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(model->data(), &numBones);
        m_selection->clearAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_BONE);
        for (nanoem_frame_index_t startFrom = from + 1, i = startFrom; i <= to; i++) {
            for (nanoem_rsize_t j = 0; j < numBones; j++) {
                const nanoem_unicode_string_t *name = nanoemModelBoneGetName(bones[j], NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
                const nanoem_frame_index_t dest = static_cast<nanoem_frame_index_t>((i - from) * scaleFactor) + from;
                if (dest == startFrom) {
                    continue;
                }
                else if (nanoem_mutable_motion_bone_keyframe_t *k =
                             nanoemMutableMotionBoneKeyframeCreateByFound(m_opaque, name, i, &status)) {
                    nanoemMutableMotionRemoveBoneKeyframe(m, k, &status);
                    nanoemMutableMotionAddBoneKeyframe(m, k, name, dest, &status);
                    nanoem_frame_index_t increment = 0;
                    do {
                        nanoemMutableMotionAddBoneKeyframe(m, k, name, dest + increment, &status);
                        increment++;
                    } while (status == NANOEM_STATUS_ERROR_MOTION_BONE_KEYFRAME_ALREADY_EXISTS);
                    nanoemMutableMotionBoneKeyframeDestroy(k);
                }
            }
        }
        nanoemMutableMotionDestroy(m);
    }
}

void
Motion::scaleAllLightKeyframesIn(nanoem_frame_index_t from, nanoem_frame_index_t to, nanoem_f32_t scaleFactor)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    if (scaleFactor > 1) {
        nanoem_mutable_motion_t *m = nanoemMutableMotionCreateAsReference(m_opaque, &status);
        m_selection->clearAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_LIGHT);
        for (nanoem_frame_index_t i = to; i > from; i--) {
            if (nanoem_mutable_motion_light_keyframe_t *k =
                    nanoemMutableMotionLightKeyframeCreateByFound(m_opaque, i, &status)) {
                const nanoem_frame_index_t dest = static_cast<nanoem_frame_index_t>((i - from) * scaleFactor) + from;
                nanoemMutableMotionRemoveLightKeyframe(m, k, &status);
                nanoemMutableMotionAddLightKeyframe(m, k, dest, &status);
                nanoemMutableMotionLightKeyframeDestroy(k);
            }
        }
        nanoemMutableMotionDestroy(m);
    }
    else if (scaleFactor < 1) {
        nanoem_mutable_motion_t *m = nanoemMutableMotionCreateAsReference(m_opaque, &status);
        m_selection->clearAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_LIGHT);
        for (nanoem_frame_index_t startFrom = from + 1, i = startFrom; i <= to; i++) {
            const nanoem_frame_index_t dest = static_cast<nanoem_frame_index_t>((i - from) * scaleFactor) + from;
            if (dest == startFrom) {
                continue;
            }
            else if (nanoem_mutable_motion_light_keyframe_t *k =
                         nanoemMutableMotionLightKeyframeCreateByFound(m_opaque, i, &status)) {
                nanoemMutableMotionRemoveLightKeyframe(m, k, &status);
                nanoemMutableMotionAddLightKeyframe(m, k, dest, &status);
                nanoem_frame_index_t increment = 0;
                do {
                    nanoemMutableMotionAddLightKeyframe(m, k, dest + increment, &status);
                    increment++;
                } while (status == NANOEM_STATUS_ERROR_MOTION_LIGHT_KEYFRAME_ALREADY_EXISTS);
                nanoemMutableMotionLightKeyframeDestroy(k);
            }
        }
        nanoemMutableMotionDestroy(m);
    }
}

void
Motion::scaleAllModelKeyframesIn(nanoem_frame_index_t from, nanoem_frame_index_t to, nanoem_f32_t scaleFactor)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    if (scaleFactor > 1) {
        nanoem_mutable_motion_t *m = nanoemMutableMotionCreateAsReference(m_opaque, &status);
        m_selection->clearAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MODEL);
        for (nanoem_frame_index_t i = to; i > from; i--) {
            if (nanoem_mutable_motion_model_keyframe_t *k =
                    nanoemMutableMotionModelKeyframeCreateByFound(m_opaque, i, &status)) {
                const nanoem_frame_index_t dest = static_cast<nanoem_frame_index_t>((i - from) * scaleFactor) + from;
                nanoemMutableMotionRemoveModelKeyframe(m, k, &status);
                nanoemMutableMotionAddModelKeyframe(m, k, dest, &status);
                nanoemMutableMotionModelKeyframeDestroy(k);
            }
        }
        nanoemMutableMotionDestroy(m);
    }
    else if (scaleFactor < 1) {
        nanoem_mutable_motion_t *m = nanoemMutableMotionCreateAsReference(m_opaque, &status);
        m_selection->clearAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MODEL);
        for (nanoem_frame_index_t startFrom = from + 1, i = startFrom; i <= to; i++) {
            const nanoem_frame_index_t dest = static_cast<nanoem_frame_index_t>((i - from) * scaleFactor) + from;
            if (dest == startFrom) {
                continue;
            }
            else if (nanoem_mutable_motion_model_keyframe_t *k =
                         nanoemMutableMotionModelKeyframeCreateByFound(m_opaque, i, &status)) {
                nanoemMutableMotionRemoveModelKeyframe(m, k, &status);
                nanoemMutableMotionAddModelKeyframe(m, k, dest, &status);
                nanoem_frame_index_t increment = 0;
                do {
                    nanoemMutableMotionAddModelKeyframe(m, k, dest + increment, &status);
                    increment++;
                } while (status == NANOEM_STATUS_ERROR_MOTION_MODEL_KEYFRAME_ALREADY_EXISTS);
                nanoemMutableMotionModelKeyframeDestroy(k);
            }
        }
        nanoemMutableMotionDestroy(m);
    }
}

void
Motion::scaleAllMorphKeyframesIn(
    const Model *model, nanoem_frame_index_t from, nanoem_frame_index_t to, nanoem_f32_t scaleFactor)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_rsize_t numMorphs;
    if (scaleFactor > 1) {
        nanoem_mutable_motion_t *m = nanoemMutableMotionCreateAsReference(m_opaque, &status);
        nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(model->data(), &numMorphs);
        m_selection->clearAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MORPH);
        for (nanoem_frame_index_t i = to; i > from; i--) {
            for (nanoem_rsize_t j = 0; j < numMorphs; j++) {
                const nanoem_unicode_string_t *name =
                    nanoemModelMorphGetName(morphs[j], NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
                if (nanoem_mutable_motion_morph_keyframe_t *k =
                        nanoemMutableMotionMorphKeyframeCreateByFound(m_opaque, name, i, &status)) {
                    const nanoem_frame_index_t dest =
                        static_cast<nanoem_frame_index_t>((i - from) * scaleFactor) + from;
                    nanoemMutableMotionRemoveMorphKeyframe(m, k, &status);
                    nanoemMutableMotionAddMorphKeyframe(m, k, name, dest, &status);
                    nanoemMutableMotionMorphKeyframeDestroy(k);
                }
            }
        }
        nanoemMutableMotionDestroy(m);
    }
    else if (scaleFactor < 1) {
        nanoem_mutable_motion_t *m = nanoemMutableMotionCreateAsReference(m_opaque, &status);
        nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(model->data(), &numMorphs);
        m_selection->clearAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MORPH);
        for (nanoem_frame_index_t startFrom = from + 1, i = startFrom; i <= to; i++) {
            for (nanoem_rsize_t j = 0; j < numMorphs; j++) {
                const nanoem_unicode_string_t *name =
                    nanoemModelMorphGetName(morphs[j], NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
                const nanoem_frame_index_t dest = static_cast<nanoem_frame_index_t>((i - from) * scaleFactor) + from;
                if (dest == startFrom) {
                    continue;
                }
                else if (nanoem_mutable_motion_morph_keyframe_t *k =
                             nanoemMutableMotionMorphKeyframeCreateByFound(m_opaque, name, i, &status)) {
                    nanoemMutableMotionRemoveMorphKeyframe(m, k, &status);
                    nanoemMutableMotionAddMorphKeyframe(m, k, name, dest, &status);
                    nanoem_frame_index_t increment = 0;
                    do {
                        nanoemMutableMotionAddMorphKeyframe(m, k, name, dest + increment, &status);
                        increment++;
                    } while (status == NANOEM_STATUS_ERROR_MOTION_MORPH_KEYFRAME_ALREADY_EXISTS);
                    nanoemMutableMotionMorphKeyframeDestroy(k);
                }
            }
        }
        nanoemMutableMotionDestroy(m);
    }
}

void
Motion::scaleAllSelfShadowKeyframesIn(nanoem_frame_index_t from, nanoem_frame_index_t to, nanoem_f32_t scaleFactor)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    if (scaleFactor > 1) {
        nanoem_mutable_motion_t *m = nanoemMutableMotionCreateAsReference(m_opaque, &status);
        m_selection->clearAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_SELFSHADOW);
        for (nanoem_frame_index_t i = to; i > from; i--) {
            if (nanoem_mutable_motion_self_shadow_keyframe_t *k =
                    nanoemMutableMotionSelfShadowKeyframeCreateByFound(m_opaque, i, &status)) {
                const nanoem_frame_index_t dest = static_cast<nanoem_frame_index_t>((i - from) * scaleFactor) + from;
                nanoemMutableMotionRemoveSelfShadowKeyframe(m, k, &status);
                nanoemMutableMotionAddSelfShadowKeyframe(m, k, dest, &status);
                nanoemMutableMotionSelfShadowKeyframeDestroy(k);
            }
        }
        nanoemMutableMotionDestroy(m);
    }
    else if (scaleFactor < 1) {
        nanoem_mutable_motion_t *m = nanoemMutableMotionCreateAsReference(m_opaque, &status);
        m_selection->clearAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_SELFSHADOW);
        for (nanoem_frame_index_t startFrom = from + 1, i = startFrom; i <= to; i++) {
            const nanoem_frame_index_t dest = static_cast<nanoem_frame_index_t>((i - from) * scaleFactor) + from;
            if (dest == startFrom) {
                continue;
            }
            else if (nanoem_mutable_motion_self_shadow_keyframe_t *k =
                         nanoemMutableMotionSelfShadowKeyframeCreateByFound(m_opaque, i, &status)) {
                nanoemMutableMotionRemoveSelfShadowKeyframe(m, k, &status);
                nanoemMutableMotionAddSelfShadowKeyframe(m, k, dest, &status);
                nanoem_frame_index_t increment = 0;
                do {
                    nanoemMutableMotionAddSelfShadowKeyframe(m, k, dest + increment, &status);
                    increment++;
                } while (status == NANOEM_STATUS_ERROR_MOTION_SELF_SHADOW_KEYFRAME_ALREADY_EXISTS);
                nanoemMutableMotionSelfShadowKeyframeDestroy(k);
            }
        }
        nanoemMutableMotionDestroy(m);
    }
}

void
Motion::selectAllModelObjectKeyframes(const Model *model)
{
    m_selection->addAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MODEL);
    {
        nanoem_rsize_t numKeyframes;
        nanoem_motion_bone_keyframe_t *const *keyframes = nanoemMotionGetAllBoneKeyframeObjects(data(), &numKeyframes);
        for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
            const nanoem_motion_bone_keyframe_t *keyframe = keyframes[i];
            const nanoem_unicode_string_t *name = nanoemMotionBoneKeyframeGetName(keyframe);
            if (model->containsBone(name)) {
                m_selection->add(keyframe);
            }
        }
    }
    {
        nanoem_rsize_t numKeyframes;
        nanoem_motion_morph_keyframe_t *const *keyframes =
            nanoemMotionGetAllMorphKeyframeObjects(data(), &numKeyframes);
        for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
            const nanoem_motion_morph_keyframe_t *keyframe = keyframes[i];
            const nanoem_unicode_string_t *name = nanoemMotionMorphKeyframeGetName(keyframe);
            if (model->containsMorph(name)) {
                m_selection->add(keyframe);
            }
        }
    }
}

bool
Motion::testAllMissingModelObjects(const Model *model, StringSet &bones, StringSet &morphs) const
{
    nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
    {
        nanoem_rsize_t numKeyframes;
        nanoem_motion_bone_keyframe_t *const *keyframes = nanoemMotionGetAllBoneKeyframeObjects(data(), &numKeyframes);
        bones.clear();
        for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
            const nanoem_motion_bone_keyframe_t *keyframe = keyframes[i];
            const nanoem_motion_keyframe_object_t *ko = nanoemMotionBoneKeyframeGetKeyframeObject(keyframe);
            if (nanoemMotionKeyframeObjectGetFrameIndex(ko) > 0) {
                const nanoem_unicode_string_t *name = nanoemMotionBoneKeyframeGetName(keyframe);
                if (!model->containsBone(name)) {
                    String s;
                    StringUtils::getUtf8String(name, factory, s);
                    bones.insert(s);
                }
            }
        }
    }
    {
        nanoem_rsize_t numKeyframes;
        nanoem_motion_morph_keyframe_t *const *keyframes =
            nanoemMotionGetAllMorphKeyframeObjects(data(), &numKeyframes);
        morphs.clear();
        for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
            const nanoem_motion_morph_keyframe_t *keyframe = keyframes[i];
            const nanoem_motion_keyframe_object_t *ko = nanoemMotionMorphKeyframeGetKeyframeObject(keyframe);
            if (nanoemMotionKeyframeObjectGetFrameIndex(ko) > 0) {
                const nanoem_unicode_string_t *name = nanoemMotionMorphKeyframeGetName(keyframe);
                if (!model->containsMorph(name)) {
                    String s;
                    StringUtils::getUtf8String(name, factory, s);
                    morphs.insert(s);
                }
            }
        }
    }
    return bones.empty() && morphs.empty();
}

void
Motion::destroyState(SelectionState *&selectionState) const NANOEM_DECL_NOEXCEPT
{
    nanoem_delete_safe(selectionState);
}

void
Motion::saveState(SelectionState *&selectionState) const
{
    destroyState(selectionState);
    selectionState = nanoem_new(SelectionState);
    selectionState->save(this);
}

void
Motion::restoreState(const SelectionState *selectionState)
{
    if (selectionState) {
        selectionState->restore(this);
    }
}

const nanoem_motion_accessory_keyframe_t *
Motion::findAccessoryKeyframe(nanoem_frame_index_t frameIndex) const NANOEM_DECL_NOEXCEPT
{
    return nanoemMotionFindAccessoryKeyframeObject(m_opaque, frameIndex);
}

const nanoem_motion_bone_keyframe_t *
Motion::findBoneKeyframe(
    const nanoem_unicode_string_t *name, nanoem_frame_index_t frameIndex) const NANOEM_DECL_NOEXCEPT
{
    return nanoemMotionFindBoneKeyframeObject(m_opaque, name, frameIndex);
}

const nanoem_motion_camera_keyframe_t *
Motion::findCameraKeyframe(nanoem_frame_index_t frameIndex) const NANOEM_DECL_NOEXCEPT
{
    return nanoemMotionFindCameraKeyframeObject(m_opaque, frameIndex);
}

const nanoem_motion_light_keyframe_t *
Motion::findLightKeyframe(nanoem_frame_index_t frameIndex) const NANOEM_DECL_NOEXCEPT
{
    return nanoemMotionFindLightKeyframeObject(m_opaque, frameIndex);
}

const nanoem_motion_model_keyframe_t *
Motion::findModelKeyframe(nanoem_frame_index_t frameIndex) const NANOEM_DECL_NOEXCEPT
{
    return nanoemMotionFindModelKeyframeObject(m_opaque, frameIndex);
}

const nanoem_motion_morph_keyframe_t *
Motion::findMorphKeyframe(
    const nanoem_unicode_string_t *name, nanoem_frame_index_t frameIndex) const NANOEM_DECL_NOEXCEPT
{
    return nanoemMotionFindMorphKeyframeObject(m_opaque, name, frameIndex);
}

const nanoem_motion_self_shadow_keyframe_t *
Motion::findSelfShadowKeyframe(nanoem_frame_index_t frameIndex) const NANOEM_DECL_NOEXCEPT
{
    return nanoemMotionFindSelfShadowKeyframeObject(m_opaque, frameIndex);
}

nanoem_rsize_t
Motion::countAllKeyframes() const NANOEM_DECL_NOEXCEPT
{
    nanoem_rsize_t numKeyframes, numTotalKeyframes = 0;
    nanoemMotionGetAllAccessoryKeyframeObjects(m_opaque, &numKeyframes);
    numTotalKeyframes += numKeyframes;
    nanoemMotionGetAllBoneKeyframeObjects(m_opaque, &numKeyframes);
    numTotalKeyframes += numKeyframes;
    nanoemMotionGetAllCameraKeyframeObjects(m_opaque, &numKeyframes);
    numTotalKeyframes += numKeyframes;
    nanoemMotionGetAllLightKeyframeObjects(m_opaque, &numKeyframes);
    numTotalKeyframes += numKeyframes;
    nanoemMotionGetAllModelKeyframeObjects(m_opaque, &numKeyframes);
    numTotalKeyframes += numKeyframes;
    nanoemMotionGetAllMorphKeyframeObjects(m_opaque, &numKeyframes);
    numTotalKeyframes += numKeyframes;
    nanoemMotionGetAllSelfShadowKeyframeObjects(m_opaque, &numKeyframes);
    numTotalKeyframes += numKeyframes;
    return numTotalKeyframes;
}

const Project *
Motion::project() const NANOEM_DECL_NOEXCEPT
{
    return m_project;
}

Project *
Motion::project() NANOEM_DECL_NOEXCEPT
{
    return m_project;
}

const IMotionKeyframeSelection *
Motion::selection() const NANOEM_DECL_NOEXCEPT
{
    return m_selection;
}

IMotionKeyframeSelection *
Motion::selection() NANOEM_DECL_NOEXCEPT
{
    return m_selection;
}

nanoem_frame_index_t
Motion::duration() const NANOEM_DECL_NOEXCEPT
{
    return glm::min(nanoemMotionGetMaxFrameIndex(m_opaque), Project::kMaximumBaseDuration);
}

const nanoem_motion_t *
Motion::data() const NANOEM_DECL_NOEXCEPT
{
    return m_opaque;
}

nanoem_motion_t *
Motion::data() NANOEM_DECL_NOEXCEPT
{
    return m_opaque;
}

nanoem_motion_format_type_t
Motion::format() const NANOEM_DECL_NOEXCEPT
{
    return m_formatType;
}

void
Motion::setFormat(nanoem_motion_format_type_t value)
{
    m_formatType = value;
}

void
Motion::setFormat(const String &value)
{
    if (StringUtils::equalsIgnoreCase(value.c_str(), kNMDFormatExtension.c_str())) {
        setFormat(NANOEM_MOTION_FORMAT_TYPE_NMD);
    }
    else if (StringUtils::equalsIgnoreCase(value.c_str(), kVMDFormatExtension.c_str())) {
        setFormat(NANOEM_MOTION_FORMAT_TYPE_VMD);
    }
}

void
Motion::setFormat(const URI &value)
{
    setFormat(value.pathExtension());
}

StringMap
Motion::annotations() const
{
    return m_annotations;
}

void
Motion::setAnnotations(const StringMap &value)
{
    m_annotations = value;
}

URI
Motion::fileURI() const
{
    return m_fileURI;
}

void
Motion::setFileURI(const URI &value)
{
    m_fileURI = value;
}

nanoem_u16_t
Motion::handle() const NANOEM_DECL_NOEXCEPT
{
    return m_handle;
}

bool
Motion::isDirty() const NANOEM_DECL_NOEXCEPT
{
    return m_dirty;
}

void
Motion::setDirty(bool value)
{
    m_dirty = value;
}

nanoem_f32_t
Motion::coefficient(const nanoem_motion_accessory_keyframe_t *prev, const nanoem_motion_accessory_keyframe_t *next,
    nanoem_frame_index_t frameIndex) NANOEM_DECL_NOEXCEPT
{
    nanoem_frame_index_t prevFrameIndex =
        nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionAccessoryKeyframeGetKeyframeObject(prev));
    nanoem_frame_index_t nextFrameIndex =
        nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionAccessoryKeyframeGetKeyframeObject(next));
    return coefficient(prevFrameIndex, nextFrameIndex, frameIndex);
}

nanoem_f32_t
Motion::coefficient(const nanoem_motion_bone_keyframe_t *prev, const nanoem_motion_bone_keyframe_t *next,
    nanoem_frame_index_t frameIndex) NANOEM_DECL_NOEXCEPT
{
    nanoem_frame_index_t prevFrameIndex =
        nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionBoneKeyframeGetKeyframeObject(prev));
    nanoem_frame_index_t nextFrameIndex =
        nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionBoneKeyframeGetKeyframeObject(next));
    return coefficient(prevFrameIndex, nextFrameIndex, frameIndex);
}

nanoem_f32_t
Motion::coefficient(const nanoem_motion_camera_keyframe_t *prev, const nanoem_motion_camera_keyframe_t *next,
    nanoem_frame_index_t frameIndex) NANOEM_DECL_NOEXCEPT
{
    nanoem_frame_index_t prevFrameIndex =
        nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionCameraKeyframeGetKeyframeObject(prev));
    nanoem_frame_index_t nextFrameIndex =
        nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionCameraKeyframeGetKeyframeObject(next));
    return coefficient(prevFrameIndex, nextFrameIndex, frameIndex);
}

nanoem_f32_t
Motion::coefficient(const nanoem_motion_light_keyframe_t *prev, const nanoem_motion_light_keyframe_t *next,
    nanoem_frame_index_t frameIndex) NANOEM_DECL_NOEXCEPT
{
    nanoem_frame_index_t prevFrameIndex =
        nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionLightKeyframeGetKeyframeObject(prev));
    nanoem_frame_index_t nextFrameIndex =
        nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionLightKeyframeGetKeyframeObject(next));
    return coefficient(prevFrameIndex, nextFrameIndex, frameIndex);
}

nanoem_f32_t
Motion::coefficient(const nanoem_motion_model_keyframe_t *prev, const nanoem_motion_model_keyframe_t *next,
    nanoem_frame_index_t frameIndex) NANOEM_DECL_NOEXCEPT
{
    nanoem_frame_index_t prevFrameIndex =
        nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionModelKeyframeGetKeyframeObject(prev));
    nanoem_frame_index_t nextFrameIndex =
        nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionModelKeyframeGetKeyframeObject(next));
    return coefficient(prevFrameIndex, nextFrameIndex, frameIndex);
}

nanoem_f32_t
Motion::coefficient(const nanoem_motion_morph_keyframe_t *prev, const nanoem_motion_morph_keyframe_t *next,
    nanoem_frame_index_t frameIndex) NANOEM_DECL_NOEXCEPT
{
    nanoem_frame_index_t prevFrameIndex =
        nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionMorphKeyframeGetKeyframeObject(prev));
    nanoem_frame_index_t nextFrameIndex =
        nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionMorphKeyframeGetKeyframeObject(next));
    return coefficient(prevFrameIndex, nextFrameIndex, frameIndex);
}

nanoem_f32_t
Motion::bezierCurve(const nanoem_motion_bone_keyframe_t *prev, const nanoem_motion_bone_keyframe_t *next,
    nanoem_motion_bone_keyframe_interpolation_type_t index, nanoem_f32_t value) const
{
    const nanoem_u8_t *parameters = nanoemMotionBoneKeyframeGetInterpolation(next, index);
    BezierCurve *curve;
    KeyframeBezierCurveMap::const_iterator it = m_keyframeBezierCurves.find(next);
    if (it != m_keyframeBezierCurves.end()) {
        curve = it->second;
    }
    else {
        const nanoem_frame_index_t interval =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionBoneKeyframeGetKeyframeObject(next)) -
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionBoneKeyframeGetKeyframeObject(prev));
        const nanoem_u64_t hash = BezierCurve::toHash(parameters, interval);
        BezierCurve::Map::const_iterator it2 = m_bezierCurvesData.find(hash);
        if (it2 != m_bezierCurvesData.end()) {
            curve = it2->second;
            m_keyframeBezierCurves.insert(tinystl::make_pair(next, curve));
        }
        else {
            const Vector2 c0(parameters[0], parameters[1]), c1(parameters[2], parameters[3]);
            curve = nanoem_new(BezierCurve(c0, c1, interval));
            m_bezierCurvesData.insert(tinystl::make_pair(hash, curve));
            m_keyframeBezierCurves.insert(tinystl::make_pair(next, curve));
        }
    }
    return curve->value(value);
}

nanoem_f32_t
Motion::coefficient(nanoem_frame_index_t prevFrameIndex, nanoem_frame_index_t nextFrameIndex,
    nanoem_frame_index_t frameIndex) NANOEM_DECL_NOEXCEPT
{
    const nanoem_frame_index_t interval = nextFrameIndex - prevFrameIndex;
    const nanoem_f32_t coef =
        (prevFrameIndex == nextFrameIndex) ? 1.0f : (frameIndex - prevFrameIndex) / (nanoem_f32_t) interval;
    return coef;
}

void
Motion::copyAccessoryOutsideParent(const nanoem_motion_accessory_keyframe_t *keyframe,
    nanoem_mutable_motion_accessory_keyframe_t *mutableAccessoryKeyframe)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    const nanoem_motion_outside_parent_t *outsideParent = nanoemMotionAccessoryKeyframeGetOutsideParent(keyframe);
    nanoem_mutable_motion_outside_parent_t *mutableOutsideParent =
        nanoemMutableMotionOutsideParentCreateFromAccessoryKeyframe(
            nanoemMutableMotionAccessoryKeyframeGetOriginObject(mutableAccessoryKeyframe), &status);
    nanoemMutableMotionOutsideParentSetTargetObjectName(
        mutableOutsideParent, nanoemMotionOutsideParentGetTargetObjectName(outsideParent), &status);
    nanoemMutableMotionOutsideParentSetTargetBoneName(
        mutableOutsideParent, nanoemMotionOutsideParentGetTargetBoneName(outsideParent), &status);
    nanoemMutableMotionAccessoryKeyframeSetOutsideParent(mutableAccessoryKeyframe, mutableOutsideParent, &status);
    nanoemMutableMotionOutsideParentDestroy(mutableOutsideParent);
}

void
Motion::copyAllAccessoryEffectParameters(const nanoem_motion_accessory_keyframe_t *keyframe,
    nanoem_mutable_motion_accessory_keyframe_t *mutableAccessoryKeyframe, nanoem_status_t &status)
{
    nanoem_rsize_t numParameters;
    nanoem_motion_effect_parameter_t *const *parameters =
        nanoemMotionAccessoryKeyframeGetAllEffectParameterObjects(keyframe, &numParameters);
    nanoem_motion_accessory_keyframe_t *originMutableKeyframe =
        nanoemMutableMotionAccessoryKeyframeGetOriginObject(mutableAccessoryKeyframe);
    status = NANOEM_STATUS_SUCCESS;
    for (nanoem_rsize_t i = 0; i < numParameters; i++) {
        const nanoem_motion_effect_parameter_t *parameter = parameters[i];
        nanoem_mutable_motion_effect_parameter_t *mutableEffectParameter =
            nanoemMutableMotionEffectParameterCreateFromAccessoryKeyframe(originMutableKeyframe, &status);
        nanoemMutableMotionEffectParameterSetName(
            mutableEffectParameter, nanoemMotionEffectParameterGetName(parameter), &status);
        nanoemMutableMotionEffectParameterSetType(
            mutableEffectParameter, nanoemMotionEffectParameterGetType(parameter));
        nanoemMutableMotionEffectParameterSetValue(
            mutableEffectParameter, nanoemMotionEffectParameterGetValue(parameter));
        nanoemMutableMotionAccessoryKeyframeAddEffectParameter(
            mutableAccessoryKeyframe, mutableEffectParameter, &status);
        nanoemMutableMotionEffectParameterDestroy(mutableEffectParameter);
    }
}

void
Motion::copyAllModelKeyframeParameters(const nanoem_motion_t *source, const IMotionKeyframeSelection *selection,
    const Model *model, nanoem_mutable_motion_t *motion, int offset, nanoem_status_t &status)
{
    nanoem_parameter_assert(model, "must not be nullptr");
    nanoem_parameter_assert(motion, "must not be nullptr");
    if (status == NANOEM_STATUS_SUCCESS) {
        copyAllBoneKeyframes(source, selection, model, motion, offset, status);
    }
    if (status == NANOEM_STATUS_SUCCESS) {
        copyAllModelKeyframes(source, selection, motion, offset, status);
    }
    if (status == NANOEM_STATUS_SUCCESS) {
        copyAllMorphKeyframes(source, selection, model, motion, offset, status);
    }
    if (status == NANOEM_STATUS_SUCCESS) {
        const nanoem_unicode_string_t *name = nanoemModelGetName(model->data(), NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        nanoemMutableMotionSetTargetModelName(motion, name, &status);
        nanoemMutableMotionSortAllKeyframes(motion);
    }
}

void
Motion::internalWriteLoadCommandMessage(nanoem_u32_t type, nanoem_u16_t handle, const URI &fileURI, Error &error)
{
    Nanoem__Application__RedoLoadMotionCommand command = NANOEM__APPLICATION__REDO_LOAD_MOTION_COMMAND__INIT;
    MutableString pathString, fragmentString, nameString;
    command.type = static_cast<Nanoem__Application__RedoLoadMotionCommand__Type>(type);
    command.content_case = NANOEM__APPLICATION__REDO_LOAD_MOTION_COMMAND__CONTENT_FILE_URI;
    Nanoem__Application__URI uri = NANOEM__APPLICATION__URI__INIT;
    uri.absolute_path = StringUtils::cloneString(fileURI.absolutePathConstString(), pathString);
    uri.fragment = StringUtils::cloneString(fileURI.fragmentConstString(), fragmentString);
    command.file_uri = &uri;
    if (handle != bx::kInvalidHandle) {
        command.has_handle = 1;
        command.handle = handle;
    }
    Nanoem__Application__Command action = NANOEM__APPLICATION__COMMAND__INIT;
    action.timestamp = stm_now();
    action.redo_load_motion = &command;
    action.type_case = NANOEM__APPLICATION__COMMAND__TYPE_REDO_LOAD_MOTION;
    m_project->writeRedoMessage(&action, error);
}

bool
Motion::internalSave(nanoem_mutable_motion_t *mutableMotion, IWriter *writer, Error &error) const
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_buffer_t *mutableBuffer = nanoemMutableBufferCreate(&status);
    for (StringMap::const_iterator it = m_annotations.begin(), end = m_annotations.end(); it != end; ++it) {
        nanoemMutableMotionSetAnnotation(mutableMotion, it->first.c_str(), it->second.c_str(), &status);
    }
    char dateTimeBuffer[32];
    StringUtils::formatDateTimeUTC(dateTimeBuffer, sizeof(dateTimeBuffer), "%Y-%m-%dT%H:%M:%SZ");
    nanoemMutableMotionSetAnnotation(mutableMotion, "generator.name", "nanoem", &status);
    nanoemMutableMotionSetAnnotation(mutableMotion, "generator.version", nanoemGetVersionString(), &status);
    const char *dateTimeCreated = nanoemMotionGetAnnotation(m_opaque, "datetime.created");
    nanoemMutableMotionSetAnnotation(
        mutableMotion, "datetime.created", dateTimeCreated ? dateTimeCreated : dateTimeBuffer, &status);
    nanoemMutableMotionSetAnnotation(mutableMotion, "datetime.updated", dateTimeBuffer, &status);
    bool succeeded = false;
    switch (m_formatType) {
    case NANOEM_MOTION_FORMAT_TYPE_VMD: {
        nanoemMutableMotionSaveToBuffer(mutableMotion, mutableBuffer, &status);
        break;
    }
    case NANOEM_MOTION_FORMAT_TYPE_NMD: {
        nanoemMutableMotionSaveToBufferNMD(mutableMotion, mutableBuffer, &status);
        break;
    }
    default:
        break;
    }
    if (status == NANOEM_STATUS_SUCCESS) {
        nanoem_buffer_t *buffer = nanoemMutableBufferCreateBufferObject(mutableBuffer, &status);
        FileUtils::write(writer, buffer, error);
        nanoemBufferDestroy(buffer);
        succeeded = !error.hasReason();
    }
    else {
        String message;
        StringUtils::format(
            message, "Cannot save the dest: %s", Error::convertStatusToMessage(status, m_project->translator()));
        error = Error(message.c_str(), status, Error::kDomainTypeNanoem);
    }
    nanoemMutableMotionDestroy(mutableMotion);
    nanoemMutableBufferDestroy(mutableBuffer);
    return succeeded;
}

void
Motion::internalMergeAllKeyframes(const Motion *source, bool _override, bool reverse)
{
    Merger merger(source->data(), m_project->unicodeStringFactory(), m_opaque, _override);
    merger.mergeAllAccessoryKeyframes();
    merger.mergeAllBoneKeyframes(reverse);
    merger.mergeAllCameraKeyframes();
    merger.mergeAllLightKeyframes();
    merger.mergeAllModelKeyframes();
    merger.mergeAllMorphKeyframes();
    merger.mergeAllSelfShadowKeyframes();
    setDirty(true);
}

} /* namespace nanoem */
