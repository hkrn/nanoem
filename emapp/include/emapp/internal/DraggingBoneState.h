/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#ifndef NANOEM_EMAPP_INTERNAL_DRAGGINGBONESTATE
#define NANOEM_EMAPP_INTERNAL_DRAGGINGBONESTATE

#include "emapp/Model.h"
#include "emapp/PhysicsEngine.h"
#include "emapp/internal/IDraggingState.h"
#include "emapp/model/BindPose.h"
#include "emapp/model/Bone.h"

namespace nanoem {

class Model;
class Project;

namespace internal {

class DraggingBoneState : public IDraggingState, private NonCopyable {
public:
    DraggingBoneState(Project *project, Model *model, const Vector2SI32 &pressedCursorPosition,
        const Vector2SI32 &lastBoneCursorPosition);
    ~DraggingBoneState();

    void commit(const Vector2SI32 &logicalScalePosition) NANOEM_DECL_OVERRIDE;

    nanoem_f32_t scaleFactor() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setScaleFactor(nanoem_f32_t value) NANOEM_DECL_OVERRIDE;

protected:
    static Vector3 handleDragAxis(const nanoem_model_bone_t *bone, Model::TransformCoordinateType type, int index);
    static Vector3 handleDragAxis(const nanoem_model_bone_t *bone, int index);
    static model::Bone::List createBoneList(const nanoem_model_bone_t *bone);
    void performTranslationTransform(
        const tinystl::pair<Vector3, const nanoem_model_bone_t *> &translation, const model::Bone::List &bones);
    void performOrientationTransform(
        const tinystl::pair<Quaternion, const nanoem_model_bone_t *> &orientation, const model::Bone::List &bones);
    model::BindPose::Parameter findBindPoseParameter(
        const nanoem_model_bone_t *bone, const Model *model) const NANOEM_DECL_NOEXCEPT;
    Model *activeModel() const NANOEM_DECL_NOEXCEPT;
    Vector3 accumulatedDeltaPosition() const NANOEM_DECL_NOEXCEPT;
    Vector3 cursorMoveDelta(const Vector2SI32 &value) const NANOEM_DECL_NOEXCEPT;
    Vector2 cursorDelta(const Vector2 &logicalScalePosition) const NANOEM_DECL_NOEXCEPT;
    Vector2SI32 pressedCursorPosition() const NANOEM_DECL_NOEXCEPT;
    Vector2SI32 lastBoneCursorPosition() const NANOEM_DECL_NOEXCEPT;
    void updateLastCursorPosition(const Vector2 &logicalScalePosition, const Vector3 &delta);
    bool isShouldSetCursorPosition() const NANOEM_DECL_NOEXCEPT;
    void setShouldSetCursorPosition(bool value);

private:
    void setCameraLocked(bool value);

    Project *m_project;
    Model *m_model;
    const Vector2SI32 m_pressedCursorPosition;
    const Vector2SI32 m_lastBoneCursorPosition;
    model::BindPose m_lastBindPose;
    Vector3 m_accumulatedDeltaPosition;
    Vector2 m_lastPressedCursorPosition;
    nanoem_f32_t m_scaleFactor;
    bool m_shouldSetCursorPosition;
};

class TranslateActiveBoneState NANOEM_DECL_SEALED : public DraggingBoneState {
public:
    TranslateActiveBoneState(Project *project, Model *model, const Vector2SI32 &pressedCursorPosition,
        const Vector2SI32 &lastBoneCursorPosition);

    void transform(const Vector2SI32 &logicalScalePosition) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    Vector3 m_baseLocalTranslation;
};

class OrientateActiveBoneState NANOEM_DECL_SEALED : public DraggingBoneState {
public:
    OrientateActiveBoneState(Project *project, Model *model, const Vector2SI32 &pressedCursorPosition,
        const Vector2SI32 &lastBoneCursorPosition);

    void transform(const Vector2SI32 &logicalScalePosition) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    nanoem_f32_t angleCursorFromBone(const Vector2SI32 &value) const;
};

class AxisAlignedTranslateActiveBoneState NANOEM_DECL_SEALED : public DraggingBoneState {
public:
    AxisAlignedTranslateActiveBoneState(
        Project *project, Model *model, const Vector2 &pressedCursorPosition, int axisIndex);

    void transform(const Vector2SI32 &logicalScalePosition) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    const int m_axisIndex;
};

class AxisAlignedOrientateActiveBoneState NANOEM_DECL_SEALED : public DraggingBoneState {
public:
    AxisAlignedOrientateActiveBoneState(
        Project *project, Model *model, const Vector2 &pressedCursorPosition, int axisIndex);

    void transform(const Vector2SI32 &logicalScalePosition) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    const int m_axisIndex;
};

} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_DRAGGINGBONESTATE */
