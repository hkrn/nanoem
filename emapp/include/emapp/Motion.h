/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_MOTION_H_
#define NANOEM_EMAPP_MOTION_H_

#include "emapp/BezierCurve.h"
#include "emapp/URI.h"

#include "nanoem/ext/mutable.h"

namespace nanoem {

class Accessory;
class BezierCurve;
class Error;
class ICamera;
class ILight;
class IMotionKeyframeSelection;
class IWriter;
class Model;
class Project;
class ShadowCamera;
class URI;

class Motion NANOEM_DECL_SEALED : private NonCopyable {
public:
    typedef tinystl::vector<nanoem_frame_index_t, TinySTLAllocator> FrameIndexList;
    typedef tinystl::unordered_set<nanoem_frame_index_t, TinySTLAllocator> FrameIndexSet;
    typedef tinystl::vector<const nanoem_motion_accessory_keyframe_t *, TinySTLAllocator> AccessoryKeyframeList;
    typedef tinystl::vector<const nanoem_motion_bone_keyframe_t *, TinySTLAllocator> BoneKeyframeList;
    typedef tinystl::vector<const nanoem_motion_camera_keyframe_t *, TinySTLAllocator> CameraKeyframeList;
    typedef tinystl::vector<const nanoem_motion_light_keyframe_t *, TinySTLAllocator> LightKeyframeList;
    typedef tinystl::vector<const nanoem_motion_model_keyframe_t *, TinySTLAllocator> ModelKeyframeList;
    typedef tinystl::vector<const nanoem_motion_morph_keyframe_t *, TinySTLAllocator> MorphKeyframeList;
    typedef tinystl::vector<const nanoem_motion_self_shadow_keyframe_t *, TinySTLAllocator> SelfShadowKeyframeList;
    typedef tinystl::unordered_set<const nanoem_motion_accessory_keyframe_t *, TinySTLAllocator> AccessoryKeyframeSet;
    typedef tinystl::unordered_set<const nanoem_motion_bone_keyframe_t *, TinySTLAllocator> BoneKeyframeSet;
    typedef tinystl::unordered_set<const nanoem_motion_camera_keyframe_t *, TinySTLAllocator> CameraKeyframeSet;
    typedef tinystl::unordered_set<const nanoem_motion_light_keyframe_t *, TinySTLAllocator> LightKeyframeSet;
    typedef tinystl::unordered_set<const nanoem_motion_model_keyframe_t *, TinySTLAllocator> ModelKeyframeSet;
    typedef tinystl::unordered_set<const nanoem_motion_morph_keyframe_t *, TinySTLAllocator> MorphKeyframeSet;
    typedef tinystl::unordered_set<const nanoem_motion_self_shadow_keyframe_t *, TinySTLAllocator>
        SelfShadowKeyframeSet;
    typedef tinystl::unordered_map<const nanoem_model_bone_t *, FrameIndexSet, TinySTLAllocator> BoneFrameIndexSetMap;
    typedef tinystl::unordered_map<const nanoem_model_morph_t *, FrameIndexSet, TinySTLAllocator> MorphFrameIndexSetMap;
    enum SortDirectionType {
        kSortDirectionTypeFirstEnum,
        kSortDirectionTypeAscend,
        kSortDirectionTypeDescend,
        kSortDirectionTypeMaxEnum
    };
    struct SelectionState;
    struct CorrectionScalarFactor {
        CorrectionScalarFactor()
            : m_mul(1)
            , m_add(0)
        {
        }
        CorrectionScalarFactor(const nanoem_f32_t mul, const nanoem_f32_t add)
            : m_mul(mul)
            , m_add(add)
        {
        }
        nanoem_f32_t m_mul;
        nanoem_f32_t m_add;
    };
    struct CorrectionVectorFactor {
        CorrectionVectorFactor()
            : m_mul(Vector3(1))
            , m_add(Vector3(0))
        {
        }
        CorrectionVectorFactor(const Vector3 &mul, const Vector3 &add)
            : m_mul(mul)
            , m_add(add)
        {
        }
        Vector3 m_mul;
        Vector3 m_add;
    };
    struct KeyframeBound {
        KeyframeBound(nanoem_frame_index_t p, nanoem_frame_index_t c, nanoem_frame_index_t n)
            : m_previous(p)
            , m_current(c)
            , m_next(n)
        {
        }
        nanoem_frame_index_t m_previous;
        nanoem_frame_index_t m_current;
        nanoem_frame_index_t m_next;
    };

    static const String kNMDFormatExtension;
    static const String kVMDFormatExtension;
    static const nanoem_u8_t kCameraAndLightTargetModelName[];
    static const nanoem_frame_index_t kMaxFrameIndex;

    static StringList loadableExtensions();
    static StringSet loadableExtensionsSet();
    static bool isLoadableExtension(const String &extension);
    static bool isLoadableExtension(const URI &fileURI);
    static bool addFrameIndexDelta(
        int value, nanoem_frame_index_t frameIndex, nanoem_frame_index_t &newFrameIndex) NANOEM_DECL_NOEXCEPT;
    static bool subtractFrameIndexDelta(
        int value, nanoem_frame_index_t frameIndex, nanoem_frame_index_t &newFrameIndex) NANOEM_DECL_NOEXCEPT;
    static void copyAllAccessoryKeyframes(nanoem_motion_accessory_keyframe_t *const *keyframes,
        nanoem_rsize_t numKeyframes, nanoem_mutable_motion_t *motion, int offset, nanoem_status_t &status);
    static void copyAllAccessoryKeyframes(
        const nanoem_motion_t *source, nanoem_mutable_motion_t *motion, int offset, nanoem_status_t &status);
    static void copyAllBoneKeyframes(nanoem_motion_bone_keyframe_t *const *keyframes, nanoem_rsize_t numKeyframes,
        const Model *model, nanoem_mutable_motion_t *motion, int offset, nanoem_status_t &status);
    static void copyAllBoneKeyframes(const nanoem_motion_t *source, const Model *model, nanoem_mutable_motion_t *motion,
        int offset, nanoem_status_t &status);
    static void copyAllCameraKeyframes(nanoem_motion_camera_keyframe_t *const *keyframes, nanoem_rsize_t numKeyframes,
        nanoem_mutable_motion_t *motion, int offset, nanoem_status_t &status);
    static void copyAllCameraKeyframes(
        const nanoem_motion_t *source, nanoem_mutable_motion_t *motion, int offset, nanoem_status_t &status);
    static void copyAllLightKeyframes(nanoem_motion_light_keyframe_t *const *keyframes, nanoem_rsize_t numKeyframes,
        nanoem_mutable_motion_t *motion, int offset, nanoem_status_t &status);
    static void copyAllLightKeyframes(
        const nanoem_motion_t *source, nanoem_mutable_motion_t *motion, int offset, nanoem_status_t &status);
    static void copyAllModelKeyframes(nanoem_motion_model_keyframe_t *const *keyframes, nanoem_rsize_t numKeyframes,
        nanoem_mutable_motion_t *motion, int offset, nanoem_status_t &status);
    static void copyAllModelKeyframes(
        const nanoem_motion_t *source, nanoem_mutable_motion_t *motion, int offset, nanoem_status_t &status);
    static void copyAllMorphKeyframes(nanoem_motion_morph_keyframe_t *const *keyframes, nanoem_rsize_t numKeyframes,
        const Model *model, nanoem_mutable_motion_t *motion, int offset, nanoem_status_t &status);
    static void copyAllMorphKeyframes(const nanoem_motion_t *source, const Model *model,
        nanoem_mutable_motion_t *motion, int offset, nanoem_status_t &status);
    static void copyAllSelfShadowKeyframes(nanoem_motion_self_shadow_keyframe_t *const *keyframes,
        nanoem_rsize_t numKeyframes, nanoem_mutable_motion_t *motion, int offset, nanoem_status_t &status);
    static void copyAllSelfShadowKeyframes(
        const nanoem_motion_t *source, nanoem_mutable_motion_t *motion, int offset, nanoem_status_t &status);
    static void sortAllKeyframes(AccessoryKeyframeList &keyframes, SortDirectionType type) NANOEM_DECL_NOEXCEPT;
    static void sortAllKeyframes(BoneKeyframeList &keyframes, SortDirectionType type) NANOEM_DECL_NOEXCEPT;
    static void sortAllKeyframes(CameraKeyframeList &keyframes, SortDirectionType type) NANOEM_DECL_NOEXCEPT;
    static void sortAllKeyframes(LightKeyframeList &keyframes, SortDirectionType type) NANOEM_DECL_NOEXCEPT;
    static void sortAllKeyframes(ModelKeyframeList &keyframes, SortDirectionType type) NANOEM_DECL_NOEXCEPT;
    static void sortAllKeyframes(MorphKeyframeList &keyframes, SortDirectionType type) NANOEM_DECL_NOEXCEPT;
    static void sortAllKeyframes(SelfShadowKeyframeList &keyframes, SortDirectionType type) NANOEM_DECL_NOEXCEPT;

    Motion(Project *project, nanoem_u16_t handle);
    ~Motion() NANOEM_DECL_NOEXCEPT;

    void initialize(const Accessory *accessory);
    void initialize(const Model *model);
    void initialize(const ILight *light);
    void initialize(const ICamera *camera);
    void initialize(const ShadowCamera *shadow);

    bool load(const nanoem_u8_t *bytes, size_t length, nanoem_frame_index_t offset, Error &error);
    bool load(const ByteArray &bytes, nanoem_frame_index_t offset, Error &error);
    bool save(IWriter *writer, const Model *model, nanoem_u32_t flags, Error &error) const;
    bool save(ByteArray &bytes, const Model *model, nanoem_u32_t flags, Error &error) const;
    void writeLoadCameraCommandMessage(const URI &fileURI, Error &error);
    void writeLoadLightCommandMessage(const URI &fileURI, Error &error);
    void writeLoadModelCommandMessage(nanoem_u16_t handle, const URI &fileURI, Error &error);
    void mergeAllKeyframes(const Motion *source);
    void overrideAllKeyframes(const Motion *source, bool reverse);
    void clearAllKeyframes();
    void correctAllSelectedBoneKeyframes(
        const CorrectionVectorFactor &translation, const CorrectionVectorFactor &orientation);
    void correctAllSelectedCameraKeyframes(const CorrectionVectorFactor &lookAt, const CorrectionVectorFactor &angle,
        const CorrectionScalarFactor &distance);
    void correctAllSelectedMorphKeyframes(const CorrectionScalarFactor &weight);
    void scaleAllAccessoryKeyframesIn(nanoem_frame_index_t from, nanoem_frame_index_t to, nanoem_f32_t scaleFactor);
    void scaleAllBoneKeyframesIn(
        const Model *model, nanoem_frame_index_t from, nanoem_frame_index_t to, nanoem_f32_t scaleFactor);
    void scaleAllLightKeyframesIn(nanoem_frame_index_t from, nanoem_frame_index_t to, nanoem_f32_t scaleFactor);
    void scaleAllModelKeyframesIn(nanoem_frame_index_t from, nanoem_frame_index_t to, nanoem_f32_t scaleFactor);
    void scaleAllMorphKeyframesIn(
        const Model *model, nanoem_frame_index_t from, nanoem_frame_index_t to, nanoem_f32_t scaleFactor);
    void scaleAllSelfShadowKeyframesIn(nanoem_frame_index_t from, nanoem_frame_index_t to, nanoem_f32_t scaleFactor);
    bool testAllMissingModelObjects(const Model *model, StringSet &bones, StringSet &morphs) const;

    void destroyState(SelectionState *&selectionState) const NANOEM_DECL_NOEXCEPT;
    void saveState(SelectionState *&selectionState) const;
    void restoreState(const SelectionState *selectionState);

    const nanoem_motion_accessory_keyframe_t *findAccessoryKeyframe(
        nanoem_frame_index_t frameIndex) const NANOEM_DECL_NOEXCEPT;
    const nanoem_motion_bone_keyframe_t *findBoneKeyframe(
        const nanoem_unicode_string_t *name, nanoem_frame_index_t frameIndex) const NANOEM_DECL_NOEXCEPT;
    const nanoem_motion_camera_keyframe_t *findCameraKeyframe(
        nanoem_frame_index_t frameIndex) const NANOEM_DECL_NOEXCEPT;
    const nanoem_motion_light_keyframe_t *findLightKeyframe(nanoem_frame_index_t frameIndex) const NANOEM_DECL_NOEXCEPT;
    const nanoem_motion_model_keyframe_t *findModelKeyframe(nanoem_frame_index_t frameIndex) const NANOEM_DECL_NOEXCEPT;
    const nanoem_motion_morph_keyframe_t *findMorphKeyframe(
        const nanoem_unicode_string_t *name, nanoem_frame_index_t frameIndex) const NANOEM_DECL_NOEXCEPT;
    const nanoem_motion_self_shadow_keyframe_t *findSelfShadowKeyframe(
        nanoem_frame_index_t frameIndex) const NANOEM_DECL_NOEXCEPT;
    nanoem_rsize_t countAllKeyframes() const NANOEM_DECL_NOEXCEPT;

    const Project *project() const NANOEM_DECL_NOEXCEPT;
    Project *project() NANOEM_DECL_NOEXCEPT;
    const IMotionKeyframeSelection *selection() const NANOEM_DECL_NOEXCEPT;
    IMotionKeyframeSelection *selection() NANOEM_DECL_NOEXCEPT;
    nanoem_frame_index_t duration() const NANOEM_DECL_NOEXCEPT;
    const nanoem_motion_t *data() const NANOEM_DECL_NOEXCEPT;
    nanoem_motion_t *data() NANOEM_DECL_NOEXCEPT;
    nanoem_motion_format_type_t format() const NANOEM_DECL_NOEXCEPT;
    void setFormat(nanoem_motion_format_type_t value);
    void setFormat(const String &value);
    void setFormat(const URI &value);
    StringMap annotations() const;
    void setAnnotations(const StringMap &value);
    URI fileURI() const;
    void setFileURI(const URI &value);
    nanoem_u16_t handle() const NANOEM_DECL_NOEXCEPT;
    bool isDirty() const NANOEM_DECL_NOEXCEPT;
    void setDirty(bool value);

    static nanoem_f32_t coefficient(const nanoem_motion_accessory_keyframe_t *prev,
        const nanoem_motion_accessory_keyframe_t *next, nanoem_frame_index_t frameIndex) NANOEM_DECL_NOEXCEPT;
    static nanoem_f32_t coefficient(const nanoem_motion_bone_keyframe_t *prev,
        const nanoem_motion_bone_keyframe_t *next, nanoem_frame_index_t frameIndex) NANOEM_DECL_NOEXCEPT;
    static nanoem_f32_t coefficient(const nanoem_motion_camera_keyframe_t *prev,
        const nanoem_motion_camera_keyframe_t *next, nanoem_frame_index_t frameIndex) NANOEM_DECL_NOEXCEPT;
    static nanoem_f32_t coefficient(const nanoem_motion_light_keyframe_t *prev,
        const nanoem_motion_light_keyframe_t *next, nanoem_frame_index_t frameIndex) NANOEM_DECL_NOEXCEPT;
    static nanoem_f32_t coefficient(const nanoem_motion_model_keyframe_t *prev,
        const nanoem_motion_model_keyframe_t *next, nanoem_frame_index_t frameIndex) NANOEM_DECL_NOEXCEPT;
    static nanoem_f32_t coefficient(const nanoem_motion_morph_keyframe_t *prev,
        const nanoem_motion_morph_keyframe_t *next, nanoem_frame_index_t frameIndex) NANOEM_DECL_NOEXCEPT;
    nanoem_f32_t bezierCurve(const nanoem_motion_bone_keyframe_t *prev, const nanoem_motion_bone_keyframe_t *next,
        nanoem_motion_bone_keyframe_interpolation_type_t index, nanoem_f32_t value) const;

private:
    typedef tinystl::unordered_map<const nanoem_motion_bone_keyframe_t *, BezierCurve *, TinySTLAllocator>
        KeyframeBezierCurveMap;
    static nanoem_f32_t coefficient(nanoem_frame_index_t prevFrameIndex, nanoem_frame_index_t nextFrameIndex,
        nanoem_frame_index_t frameIndex) NANOEM_DECL_NOEXCEPT;
    static void copyAccessoryOutsideParent(const nanoem_motion_accessory_keyframe_t *keyframe,
        nanoem_mutable_motion_accessory_keyframe_t *mutableAccessoryKeyframe);
    static void copyAllAccessoryEffectParameters(const nanoem_motion_accessory_keyframe_t *keyframe,
        nanoem_mutable_motion_accessory_keyframe_t *mutableAccessoryKeyframe, nanoem_status_t &status);
    static void copyAllModelKeyframeParameters(const nanoem_motion_t *source, const Model *model,
        nanoem_mutable_motion_t *motion, int offset, nanoem_status_t &status);
    void internalWriteLoadCommandMessage(nanoem_u32_t type, nanoem_u16_t handle, const URI &fileURI, Error &error);
    bool internalSave(nanoem_mutable_motion_t *mutableMotion, IWriter *bytes, Error &error) const;
    void internalMergeAllKeyframes(const Motion *source, bool _override, bool reverse);

    Project *m_project;
    IMotionKeyframeSelection *m_selection;
    nanoem_motion_t *m_opaque;
    mutable BezierCurve::Map m_bezierCurvesData;
    mutable KeyframeBezierCurveMap m_keyframeBezierCurves;
    StringMap m_annotations;
    URI m_fileURI;
    nanoem_motion_format_type_t m_formatType;
    nanoem_u16_t m_handle;
    bool m_dirty;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_MOTION_H_ */
