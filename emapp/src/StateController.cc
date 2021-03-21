/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/StateController.h"

#include "emapp/BaseApplicationService.h"
#include "emapp/EnumUtils.h"
#include "emapp/IAudioPlayer.h"
#include "emapp/ICamera.h"
#include "emapp/IEventPublisher.h"
#include "emapp/IFileManager.h"
#include "emapp/IModelObjectSelection.h"
#include "emapp/IPrimitive2D.h"
#include "emapp/Project.h"
#include "emapp/StringUtils.h"
#include "emapp/internal/DebugDrawer.h"
#include "emapp/internal/DraggingBackgroundVideoState.h"
#include "emapp/internal/DraggingBoneState.h"
#include "emapp/internal/DraggingCameraState.h"
#include "emapp/private/CommonInclude.h"

#include "glm/gtx/vector_query.hpp"
#include "sokol/sokol_time.h"

namespace nanoem {

class StateController::IState {
public:
    virtual ~IState() NANOEM_DECL_NOEXCEPT
    {
    }
    virtual void onPress(const Vector3SI32 &logicalScalePosition) = 0;
    virtual void onMove(const Vector3SI32 &logicalScalePosition, const Vector2SI32 &delta) = 0;
    virtual void onRelease(const Vector3SI32 &logicalScalePosition) = 0;
    virtual void onDrawPrimitive2D(IPrimitive2D *primitive) = 0;
    virtual void onDrawGizmo3D() = 0;
};

namespace {

class BaseState : public StateController::IState {
protected:
    BaseState(StateController *stateController, BaseApplicationService *application)
        : m_stateControllerPtr(stateController)
        , m_applicationPtr(application)
        , m_lastPosition(0)
    {
        nanoem_parameter_assert(m_stateControllerPtr, "must not be nullptr");
        nanoem_parameter_assert(m_applicationPtr, "must not be nullptr");
    }

    void
    onDrawPrimitive2D(IPrimitive2D * /* primitive */) NANOEM_DECL_OVERRIDE
    {
    }
    void
    onDrawGizmo3D() NANOEM_DECL_OVERRIDE
    {
    }

    StateController *m_stateControllerPtr;
    BaseApplicationService *m_applicationPtr;
    Vector2SI32 m_lastPosition;
};

class BaseDraggingObjectState : public BaseState {
public:
    static nanoem_f32_t
    scaleFactor(const Vector3SI32 &logicalScalePosition) NANOEM_DECL_NOEXCEPT
    {
        nanoem_f32_t scaleFactor;
        if (EnumUtils::isEnabledT<int>(Project::kCursorModifierTypeControl, logicalScalePosition.z)) {
            scaleFactor = 0.1f;
        }
        else if (EnumUtils::isEnabledT<int>(Project::kCursorModifierTypeShift, logicalScalePosition.z)) {
            scaleFactor = 10.0f;
        }
        else {
            scaleFactor = 1.0f;
        }
        return scaleFactor;
    }

    BaseDraggingObjectState(StateController *stateController, BaseApplicationService *application, bool canUpdateAngle)
        : BaseState(stateController, application)
        , m_canUpdateAngle(canUpdateAngle)
        , m_lastDraggingState(nullptr)
        , m_scaleFactor(0.0f)
    {
    }
    ~BaseDraggingObjectState() NANOEM_DECL_NOEXCEPT
    {
        nanoem_delete_safe(m_lastDraggingState);
    }

protected:
    void
    onMove(const Vector3SI32 &logicalScalePosition, const Vector2SI32 &delta) NANOEM_DECL_OVERRIDE
    {
        if (m_lastDraggingState) {
            m_lastDraggingState->setScaleFactor(m_scaleFactor * scaleFactor(logicalScalePosition));
            m_lastDraggingState->transform(logicalScalePosition);
        }
        else if (canUpdateCameraAngle()) {
            updateCameraAngle(delta);
        }
        m_lastPosition = logicalScalePosition;
    }
    void
    onRelease(const Vector3SI32 &logicalScalePosition) NANOEM_DECL_OVERRIDE
    {
        if (m_lastDraggingState) {
            m_lastDraggingState->commit(logicalScalePosition);
        }
        if (Model *model = m_stateControllerPtr->currentProject()->activeModel()) {
            model->setTransformAxisType(Model::kAxisTypeMaxEnum);
        }
        m_lastPosition = Vector2();
    }

    virtual internal::IDraggingState *createTranslateState(
        int axisIndex, const Vector3SI32 &logicalScalePosition, Project *project) = 0;
    virtual internal::IDraggingState *createOrientateState(
        int axisIndex, const Vector3SI32 &logicalScalePosition, Project *project) = 0;
    virtual internal::IDraggingState *createCameraLookAtState(
        const Vector3SI32 &logicalScalePosition, Project *project) = 0;
    virtual internal::IDraggingState *createCameraZoomState(
        const Vector3SI32 &logicalScalePosition, Project *project) = 0;

    internal::IDraggingState *
    createDraggingState(Project::RectangleType rectangleType, const Vector3SI32 &logicalScalePosition, Project *project)
    {
        int axisIndex;
        const bool orientate =
            rectangleType >= Project::kRectangleOrientateX && rectangleType <= Project::kRectangleOrientateZ;
        const bool translate =
            rectangleType >= Project::kRectangleTranslateX && rectangleType <= Project::kRectangleTranslateZ;
        internal::IDraggingState *draggingState = nullptr;
        if (orientate) {
            axisIndex = rectangleType - Project::kRectangleOrientateX;
            draggingState = createOrientateState(axisIndex, logicalScalePosition, project);
        }
        else if (translate) {
            axisIndex = rectangleType - Project::kRectangleTranslateX;
            draggingState = createTranslateState(axisIndex, logicalScalePosition, project);
        }
        else if (rectangleType == Project::kRectangleTransformCoordinateType) {
            project->toggleTransformCoordinateType();
        }
        else if (rectangleType == Project::kRectangleCameraLookAt) {
            if (isBackgroundVideoOperation(logicalScalePosition, project)) {
                draggingState = nanoem_new(internal::MoveBackgroundVideoState(project, logicalScalePosition));
            }
            else {
                draggingState = createCameraLookAtState(logicalScalePosition, project);
            }
        }
        else if (rectangleType == Project::kRectangleCameraZoom) {
            if (isBackgroundVideoOperation(logicalScalePosition, project)) {
                draggingState = nanoem_new(internal::ZoomBackgroundVideoState(project, logicalScalePosition));
            }
            else {
                draggingState = createCameraZoomState(logicalScalePosition, project);
            }
        }
        return draggingState;
    }
    void
    setDraggingState(internal::IDraggingState *draggingState, const Vector3SI32 &logicalScalePosition)
    {
        if (draggingState) {
            const nanoem_f32_t sf = draggingState->scaleFactor();
            draggingState->setScaleFactor(sf * scaleFactor(logicalScalePosition));
            m_lastDraggingState = draggingState;
            m_scaleFactor = sf;
        }
    }
    void
    updateCameraAngle(const Vector2SI32 &delta)
    {
        ICamera *camera = m_stateControllerPtr->currentProject()->activeCamera();
        camera->setAngle(glm::radians(glm::degrees(camera->angle()) + Vector3(delta.y, delta.x, 0)));
        camera->update();
    }
    bool
    canUpdateCameraAngle() const NANOEM_DECL_NOEXCEPT
    {
        const Project *project = m_stateControllerPtr->currentProject();
        return m_canUpdateAngle && !(m_applicationPtr->hasModalDialog() || project->audioPlayer()->isPlaying());
    }

private:
    static bool
    isBackgroundVideoOperation(const Vector3SI32 &logicalScalePosition, const Project *project) NANOEM_DECL_NOEXCEPT
    {
        return project->hasBackgroundImageHandle() &&
            EnumUtils::isEnabledT<int>(Project::kCursorModifierTypeShift, logicalScalePosition.z);
    }

    const bool m_canUpdateAngle;
    internal::IDraggingState *m_lastDraggingState;
    nanoem_f32_t m_scaleFactor;
};

class DraggingBoneState NANOEM_DECL_SEALED : public BaseDraggingObjectState {
public:
    static const nanoem_model_bone_t *
    findHoverBone(const Vector3SI32 &value, const Project *project, Model::AxisType &axisType)
    {
        const nanoem_model_bone_t *bone = nullptr;
        nanoem_rsize_t boneIndex;
        DraggingBoneState::handlePointerIntersects(value, project, bone, boneIndex, axisType);
        return bone;
    }

    DraggingBoneState(StateController *stateController, BaseApplicationService *application, bool canUpdateAngle)
        : BaseDraggingObjectState(stateController, application, canUpdateAngle)
    {
    }
    ~DraggingBoneState() NANOEM_DECL_NOEXCEPT
    {
    }

    internal::IDraggingState *
    createTranslateState(int axisIndex, const Vector3SI32 &logicalScalePosition, Project *project) NANOEM_DECL_OVERRIDE
    {
        internal::IDraggingState *state = nullptr;
        Model *model = project->activeModel();
        if (model && model->selection()->areAllBonesMovable()) {
            state = nanoem_new(
                internal::AxisAlignedTranslateActiveBoneState(project, model, logicalScalePosition, axisIndex));
        }
        return state;
    }
    internal::IDraggingState *
    createOrientateState(int axisIndex, const Vector3SI32 &logicalScalePosition, Project *project) NANOEM_DECL_OVERRIDE
    {
        internal::IDraggingState *state = nullptr;
        Model *model = project->activeModel();
        if (model && model->selection()->areAllBonesRotateable()) {
            state = nanoem_new(
                internal::AxisAlignedOrientateActiveBoneState(project, model, logicalScalePosition, axisIndex));
        }
        return state;
    }
    internal::IDraggingState *
    createCameraLookAtState(const Vector3SI32 &logicalScalePosition, Project *project) NANOEM_DECL_OVERRIDE
    {
        return nanoem_new(
            internal::CameraLookAtState(project, project->activeModel()->localCamera(), logicalScalePosition));
    }
    internal::IDraggingState *
    createCameraZoomState(const Vector3SI32 &logicalScalePosition, Project *project) NANOEM_DECL_OVERRIDE
    {
        return nanoem_new(
            internal::CameraZoomState(project, project->activeModel()->localCamera(), logicalScalePosition));
    }

    void
    onPress(const Vector3SI32 &logicalScalePosition) NANOEM_DECL_OVERRIDE
    {
        Project::RectangleType rectangleType = Project::kRectangleTypeMaxEnum;
        internal::IDraggingState *draggingState = nullptr;
        Project *project = m_stateControllerPtr->currentProject();
        Model *model = project->activeModel();
        if (project->isPlaying()) {
            /* do nothing */
        }
        else if (project->intersectsTransformHandle(logicalScalePosition, rectangleType)) {
            draggingState = createDraggingState(rectangleType, logicalScalePosition, project);
        }
        else if (const nanoem_model_bone_t *bonePtr = model->activeBone()) {
            const ICamera *camera = project->activeCamera();
            const model::Bone *bone = model::Bone::cast(bonePtr);
            const Vector2 &lastBoneCursorPosition =
                camera->toScreenCoordinate(Vector3(model->worldTransform(bone->worldTransform())[3]));
            if (model->transformAxisType() != Model::kAxisTypeMaxEnum &&
                !glm::isNull(lastBoneCursorPosition, Constants::kEpsilon)) {
                const Project::EditingMode editingMode = project->editingMode();
                const IModelObjectSelection *selection = model->selection();
                if (editingMode == Project::kEditingModeRotate && selection->areAllBonesRotateable()) {
                    draggingState = nanoem_new(internal::OrientateActiveBoneState(
                        project, model, logicalScalePosition, lastBoneCursorPosition));
                }
                else if (editingMode == Project::kEditingModeMove && selection->areAllBonesMovable()) {
                    draggingState = nanoem_new(internal::TranslateActiveBoneState(
                        project, model, logicalScalePosition, lastBoneCursorPosition));
                }
            }
        }
        if (draggingState) {
            setDraggingState(draggingState, logicalScalePosition);
        }
        else {
            const nanoem_model_bone_t *bone = nullptr;
            Model::AxisType axisType;
            nanoem_rsize_t boneIndex = m_stateControllerPtr->boneIndex();
            if (handlePointerIntersects(logicalScalePosition, project, bone, boneIndex, axisType)) {
                m_stateControllerPtr->setBoneIndex(boneIndex);
                model->selection()->toggleSelectAndActiveBone(bone, project->isMultipleBoneSelectionEnabled());
            }
        }
    }

private:
    static Vector2SI32
    deviceScaleCursorActiveBoneInWindow(const Project *project) NANOEM_DECL_NOEXCEPT
    {
        Vector2SI32 deviceScaleCursor(0);
        if (const Model *model = project->activeModel()) {
            if (const model::Bone *bone = model::Bone::cast(model->activeBone())) {
                deviceScaleCursor =
                    project->activeCamera()->toDeviceScreenCoordinateInWindow(bone->worldTransformOrigin());
            }
        }
        return deviceScaleCursor;
    }
    static bool
    handlePointerIntersects(const Vector2SI32 &logicalScalePosition, const Project *project,
        const nanoem_model_bone_t *&bone, nanoem_rsize_t &boneIndex, Model::AxisType &axisType) NANOEM_DECL_NOEXCEPT
    {
        const Vector2SI32 deviceScalePosition(Vector2(logicalScalePosition) * project->windowDevicePixelRatio());
        bool intersected = false;
        axisType = Model::kAxisTypeMaxEnum;
        switch (project->editingMode()) {
        case Project::kEditingModeSelect: {
            if (const Model *model = project->activeModel()) {
                bone = model->intersectsBone(deviceScalePosition, boneIndex);
                intersected = bone != nullptr;
            }
            break;
        }
        case Project::kEditingModeMove: {
            axisType = selectTranslationAxisType(deviceScalePosition, project);
            intersected = axisType != Model::kAxisTypeMaxEnum;
            break;
        }
        case Project::kEditingModeRotate: {
            axisType = selectOrientationAxisType(deviceScalePosition, project);
            intersected = axisType != Model::kAxisTypeMaxEnum;
            break;
        }
        default:
            break;
        }
        return intersected;
    }
    static Model::AxisType
    selectTranslationAxisType(const Vector2SI32 &deviceScalePosition, const Project *project) NANOEM_DECL_NOEXCEPT
    {
        const Vector2SI32 &center = deviceScaleCursorActiveBoneInWindow(project);
        nanoem_f32_t length = project->deviceScaleCircleRadius() * 10.0f, width = project->deviceScaleCircleRadius(),
                     offset = width * 0.5f;
        const Vector4SI32 rectCenter(center.x - offset, center.y - offset, width, width);
        const Vector4SI32 rectX(center.x - offset, center.y - offset, length, width);
        const Vector4SI32 rectY(center.x - offset, center.y - length - offset, width, length);
        Model::AxisType axisType(Model::kAxisTypeMaxEnum);
        if (Inline::intersectsRectPoint(rectCenter, deviceScalePosition)) {
            axisType = Model::kAxisCenter;
        }
        else if (Inline::intersectsRectPoint(rectX, deviceScalePosition)) {
            axisType = Model::kAxisX;
        }
        else if (Inline::intersectsRectPoint(rectY, deviceScalePosition)) {
            axisType = Model::kAxisY;
        }
        return axisType;
    }
    static Model::AxisType
    selectOrientationAxisType(const Vector2SI32 &deviceScalePosition, const Project *project) NANOEM_DECL_NOEXCEPT
    {
        const Vector2 center(deviceScaleCursorActiveBoneInWindow(project));
        nanoem_f32_t radius = project->deviceScaleCircleRadius() * 7.5f, width = project->deviceScaleCircleRadius(),
                     woffset = width * 0.5f;
        nanoem_f32_t distance = glm::distance(Vector2(deviceScalePosition), center);
        const Vector4SI32 rectCenter(center.x - woffset, center.y - woffset, width, width);
        const Vector4SI32 rectX(center.x - woffset, center.y - radius, width, radius * 2.0f);
        const Vector4SI32 rectZ(center.x - radius, center.y - woffset, radius * 2.0f, width);
        Model::AxisType axisType(Model::kAxisTypeMaxEnum);
        if (distance < radius + woffset && distance > radius - woffset) {
            axisType = Model::kAxisY;
        }
        else if (Inline::intersectsRectPoint(rectCenter, deviceScalePosition)) {
            axisType = Model::kAxisCenter;
        }
        else if (Inline::intersectsRectPoint(rectX, deviceScalePosition)) {
            axisType = Model::kAxisX;
        }
        else if (Inline::intersectsRectPoint(rectZ, deviceScalePosition)) {
            axisType = Model::kAxisZ;
        }
        return axisType;
    }
};

class DraggingCameraState NANOEM_DECL_SEALED : public BaseDraggingObjectState {
public:
    DraggingCameraState(StateController *stateController, BaseApplicationService *application, bool canUpdateAngle)
        : BaseDraggingObjectState(stateController, application, canUpdateAngle)
    {
    }
    ~DraggingCameraState() NANOEM_DECL_NOEXCEPT
    {
    }

    internal::IDraggingState *
    createTranslateState(int axisIndex, const Vector3SI32 &logicalScalePosition, Project *project) NANOEM_DECL_OVERRIDE
    {
        return nanoem_new(internal::AxisAlignedTranslateCameraState(
            project, project->globalCamera(), logicalScalePosition, axisIndex));
    }
    internal::IDraggingState *
    createOrientateState(int axisIndex, const Vector3SI32 &logicalScalePosition, Project *project) NANOEM_DECL_OVERRIDE
    {
        return nanoem_new(internal::AxisAlignedOrientateCameraState(
            project, project->globalCamera(), logicalScalePosition, axisIndex));
    }
    internal::IDraggingState *
    createCameraLookAtState(const Vector3SI32 &logicalScalePosition, Project *project) NANOEM_DECL_OVERRIDE
    {
        return nanoem_new(internal::CameraLookAtState(project, project->globalCamera(), logicalScalePosition));
    }
    internal::IDraggingState *
    createCameraZoomState(const Vector3SI32 &logicalScalePosition, Project *project) NANOEM_DECL_OVERRIDE
    {
        return nanoem_new(internal::CameraZoomState(project, project->globalCamera(), logicalScalePosition));
    }

    void
    onPress(const Vector3SI32 &logicalScalePosition) NANOEM_DECL_OVERRIDE
    {
        Project::RectangleType rectangleType = Project::kRectangleTypeMaxEnum;
        Project *project = m_stateControllerPtr->currentProject();
        if (!project->isPlaying() && project->intersectsTransformHandle(logicalScalePosition, rectangleType)) {
            setDraggingState(createDraggingState(rectangleType, logicalScalePosition, project), logicalScalePosition);
        }
    }
};

class BaseSelectionState : public BaseDraggingObjectState {
protected:
    BaseSelectionState(StateController *stateController, BaseApplicationService *application)
        : BaseDraggingObjectState(stateController, application, false)
        , m_rect(0)
    {
    }
    ~BaseSelectionState() NANOEM_DECL_NOEXCEPT
    {
    }

    virtual const char *name() const = 0;
    virtual void commitSelection(Model *model, const Project *project, bool removeAll) = 0;

    internal::IDraggingState *
    createTranslateState(int axisIndex, const Vector3SI32 &logicalScalePosition, Project *project) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_3(axisIndex, logicalScalePosition, project)
        return nullptr;
    }
    internal::IDraggingState *
    createOrientateState(int axisIndex, const Vector3SI32 &logicalScalePosition, Project *project) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_3(axisIndex, logicalScalePosition, project)
        return nullptr;
    }
    internal::IDraggingState *
    createCameraLookAtState(const Vector3SI32 &logicalScalePosition, Project *project) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_2(logicalScalePosition, project);
        return nullptr;
    }
    internal::IDraggingState *
    createCameraZoomState(const Vector3SI32 &logicalScalePosition, Project *project) NANOEM_DECL_OVERRIDE
    {
        BX_UNUSED_2(logicalScalePosition, project);
        return nullptr;
    }
    Vector4
    deviceScaleRect(const Project *project) const
    {
        return Vector4(m_rect) * project->windowDevicePixelRatio();
    }
    Matrix4x4
    pivotMatrix(const Model *activeModel) const
    {
        typedef tinystl::unordered_map<const nanoem_model_material_t *, nanoem_rsize_t, TinySTLAllocator>
            MaterialOffsetMap;
        const IModelObjectSelection *selection = activeModel->selection();
        Matrix4x4 matrix(0);
        Vector3 aabbMin(FLT_MAX), aabbMax(FLT_MIN);
        switch (selection->editingType()) {
        case IModelObjectSelection::kEditingTypeBone: {
            const model::Bone::Set selectedBoneSet(selection->allBoneSet());
            if (!selectedBoneSet.empty()) {
                for (model::Bone::Set::const_iterator it = selectedBoneSet.begin(), end = selectedBoneSet.end();
                     it != end; ++it) {
                    const nanoem_model_bone_t *bonePtr = *it;
                    const Vector3 origin(model::Bone::origin(bonePtr));
                    aabbMin = glm::min(aabbMin, origin);
                    aabbMax = glm::max(aabbMax, origin);
                }
                matrix = glm::translate(Constants::kIdentity, (aabbMax + aabbMin) * 0.5f);
            }
            break;
        }
        case IModelObjectSelection::kEditingTypeFace: {
            const IModelObjectSelection::VertexIndexSet selectedBoneSet(selection->allVertexIndexSet());
            if (!selectedBoneSet.empty()) {
                nanoem_rsize_t numVertices;
                nanoem_model_vertex_t *const *vertices =
                    nanoemModelGetAllVertexObjects(activeModel->data(), &numVertices);
                for (IModelObjectSelection::VertexIndexSet::const_iterator it = selectedBoneSet.begin(),
                                                                           end = selectedBoneSet.end();
                     it != end; ++it) {
                    const nanoem_u32_t vertexIndex = *it;
                    const Vector3 origin(glm::make_vec3(nanoemModelVertexGetOrigin(vertices[vertexIndex])));
                    aabbMin = glm::min(aabbMin, origin);
                    aabbMax = glm::max(aabbMax, origin);
                }
                matrix = glm::translate(Constants::kIdentity, (aabbMax + aabbMin) * 0.5f);
            }
            break;
        }
        case IModelObjectSelection::kEditingTypeJoint: {
            const model::Joint::Set selectedJointSet(selection->allJointSet());
            if (!selectedJointSet.empty()) {
                for (model::Joint::Set::const_iterator it = selectedJointSet.begin(), end = selectedJointSet.end();
                     it != end; ++it) {
                    const nanoem_model_joint_t *jointPtr = *it;
                    const Vector3 origin(glm::make_vec3(nanoemModelJointGetOrigin(jointPtr)));
                    aabbMin = glm::min(aabbMin, origin);
                    aabbMax = glm::max(aabbMax, origin);
                }
                matrix = glm::translate(Constants::kIdentity, (aabbMax + aabbMin) * 0.5f);
            }
            break;
        }
        case IModelObjectSelection::kEditingTypeMaterial: {
            const model::Material::Set selectedMaterialSet(selection->allMaterialSet());
            if (!selectedMaterialSet.empty()) {
                nanoem_rsize_t numMaterials, numVertices, numVertexIndices;
                const nanoem_model_t *opaque = activeModel->data();
                nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(opaque, &numMaterials);
                nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(opaque, &numVertices);
                const nanoem_u32_t *vertexIndices = nanoemModelGetAllVertexIndices(opaque, &numVertexIndices);
                MaterialOffsetMap materialOffsets;
                for (nanoem_rsize_t i = 0, offset = 0; i < numMaterials; i++) {
                    const nanoem_model_material_t *material = materials[i];
                    materialOffsets.insert(tinystl::make_pair(material, offset));
                    offset += nanoemModelMaterialGetNumVertexIndices(material);
                }
                for (model::Material::Set::const_iterator it = selectedMaterialSet.begin(),
                                                          end = selectedMaterialSet.end();
                     it != end; ++it) {
                    const nanoem_model_material_t *materialPtr = *it;
                    MaterialOffsetMap::const_iterator it2 = materialOffsets.find(materialPtr);
                    if (it2 != materialOffsets.end()) {
                        nanoem_rsize_t offset = it2->second,
                                       indices = nanoemModelMaterialGetNumVertexIndices(materialPtr);
                        for (nanoem_rsize_t i = 0; i < indices; i++) {
                            const nanoem_u32_t vertexIndex = vertexIndices[offset + i];
                            const Vector3 origin(glm::make_vec3(nanoemModelVertexGetOrigin(vertices[vertexIndex])));
                            aabbMin = glm::min(aabbMin, origin);
                            aabbMax = glm::max(aabbMax, origin);
                        }
                    }
                }
                matrix = glm::translate(Constants::kIdentity, (aabbMax + aabbMin) * 0.5f);
            }
            break;
        }
        case IModelObjectSelection::kEditingTypeRigidBody: {
            const model::RigidBody::Set selectedRigidBodySet(selection->allRigidBodySet());
            if (!selectedRigidBodySet.empty()) {
                for (model::RigidBody::Set::const_iterator it = selectedRigidBodySet.begin(),
                                                           end = selectedRigidBodySet.end();
                     it != end; ++it) {
                    const nanoem_model_rigid_body_t *rigidBodyPtr = *it;
                    const Vector3 origin(glm::make_vec3(nanoemModelRigidBodyGetOrigin(rigidBodyPtr)));
                    aabbMin = glm::min(aabbMin, origin);
                    aabbMax = glm::max(aabbMax, origin);
                }
                matrix = glm::translate(Constants::kIdentity, (aabbMax + aabbMin) * 0.5f);
            }
            break;
        }
        case IModelObjectSelection::kEditingTypeSoftBody: {
            const model::SoftBody::Set selectedSoftBodySet(selection->allSoftBodySet());
            if (!selectedSoftBodySet.empty()) {
                nanoem_rsize_t numMaterials, numVertices, numVertexIndices;
                const nanoem_model_t *opaque = activeModel->data();
                nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(opaque, &numMaterials);
                nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(opaque, &numVertices);
                const nanoem_u32_t *vertexIndices = nanoemModelGetAllVertexIndices(opaque, &numVertexIndices);
                MaterialOffsetMap materialOffsets;
                for (nanoem_rsize_t i = 0, offset = 0; i < numMaterials; i++) {
                    const nanoem_model_material_t *material = materials[i];
                    materialOffsets.insert(tinystl::make_pair(material, offset));
                    offset += nanoemModelMaterialGetNumVertexIndices(material);
                }
                for (model::SoftBody::Set::const_iterator it = selectedSoftBodySet.begin(),
                                                          end = selectedSoftBodySet.end();
                     it != end; ++it) {
                    const nanoem_model_soft_body_t *softBodyPtr = *it;
                    const nanoem_model_material_t *materialPtr = nanoemModelSoftBodyGetMaterialObject(softBodyPtr);
                    MaterialOffsetMap::const_iterator it2 = materialOffsets.find(materialPtr);
                    if (it2 != materialOffsets.end()) {
                        nanoem_rsize_t offset = it2->second,
                                       indices = nanoemModelMaterialGetNumVertexIndices(materialPtr);
                        for (nanoem_rsize_t i = 0; i < indices; i++) {
                            const nanoem_u32_t vertexIndex = vertexIndices[offset + i];
                            const Vector3 origin(glm::make_vec3(nanoemModelVertexGetOrigin(vertices[vertexIndex])));
                            aabbMin = glm::min(aabbMin, origin);
                            aabbMax = glm::max(aabbMax, origin);
                        }
                    }
                }
                matrix = glm::translate(Constants::kIdentity, (aabbMax + aabbMin) * 0.5f);
            }
            break;
        }
        case IModelObjectSelection::kEditingTypeVertex: {
            const model::Vertex::Set selectedVertexSet(selection->allVertexSet());
            if (!selectedVertexSet.empty()) {
                for (model::Vertex::Set::const_iterator it = selectedVertexSet.begin(), end = selectedVertexSet.end();
                     it != end; ++it) {
                    const nanoem_model_vertex_t *vertexPtr = *it;
                    const Vector3 origin(glm::make_vec3(nanoemModelVertexGetOrigin(vertexPtr)));
                    aabbMin = glm::min(aabbMin, origin);
                    aabbMax = glm::max(aabbMax, origin);
                }
                matrix = glm::translate(Constants::kIdentity, (aabbMax + aabbMin) * 0.5f);
            }
            break;
        }
        default:
            break;
        }
        return matrix;
    }

private:
    void
    onPress(const Vector3SI32 &logicalScalePosition) NANOEM_DECL_OVERRIDE
    {
        Project *project = m_stateControllerPtr->currentProject();
        Project::RectangleType rectangleType;
        if (project->isPlaying()) {
            /* do nothing */
        }
        else if (project->intersectsTransformHandle(logicalScalePosition, rectangleType)) {
            setDraggingState(createDraggingState(rectangleType, logicalScalePosition, project), logicalScalePosition);
        }
        else {
            updateRegion(Vector4SI32(logicalScalePosition.x, logicalScalePosition.y, 0, 0));
        }
        m_lastPosition = logicalScalePosition;
    }
    void
    onMove(const Vector3SI32 &logicalScalePosition, const Vector2SI32 & /* delta */) NANOEM_DECL_OVERRIDE
    {
        Project *project = m_stateControllerPtr->currentProject();
        project->setLogicalPixelMovingCursorPosition(logicalScalePosition);
        updateRegion(Vector4SI32(m_lastPosition, Vector2SI32(logicalScalePosition) - m_lastPosition));
    }
    void
    onRelease(const Vector3SI32 &logicalScalePosition) NANOEM_DECL_OVERRIDE
    {
        Project *project = m_stateControllerPtr->currentProject();
        bool removeAll =
            EnumUtils::isEnabledT<int>(Project::kCursorModifierTypeShift, logicalScalePosition.z) ? false : true;
        updateRegion(Vector4SI32(m_lastPosition, Vector2SI32(logicalScalePosition) - m_lastPosition));
        if (Model *model = project->activeModel()) {
            commitSelection(model, project, removeAll);
            model->setPivotMatrix(pivotMatrix(model));
        }
        m_lastPosition = Vector2();
    }
    void
    onDrawPrimitive2D(IPrimitive2D *primitive) NANOEM_DECL_OVERRIDE
    {
        Project *project = m_stateControllerPtr->currentProject();
        nanoem_f32_t deviceScaleRatio = project->windowDevicePixelRatio();
        const Vector4SI32 rect(Vector4(m_rect) * deviceScaleRatio);
        primitive->fillRect(rect, Vector4(1, 0, 0, 0.5f), 0);
        primitive->strokeRect(rect, Vector4(1, 0, 0, 1), 0, deviceScaleRatio);
    }
    void
    updateRegion(const Vector4SI32 &value)
    {
        Project *project = m_stateControllerPtr->currentProject();
        m_rect = value - Vector4SI32(Vector2UI16(project->logicalScaleUniformedViewportImageRect()), 0, 0);
        if (value.z < 0) {
            m_rect.z *= -1;
            m_rect.x -= m_rect.z;
        }
        if (value.w < 0) {
            m_rect.w *= -1;
            m_rect.y -= m_rect.w;
        }
    }

    Vector4SI32 m_rect;
};

class DraggingBoxSelectedBoneState NANOEM_DECL_SEALED : public BaseSelectionState {
public:
    DraggingBoxSelectedBoneState(StateController *stateController, BaseApplicationService *application)
        : BaseSelectionState(stateController, application)
    {
    }
    ~DraggingBoxSelectedBoneState() NANOEM_DECL_NOEXCEPT
    {
    }

private:
    const char *
    name() const NANOEM_DECL_OVERRIDE
    {
        return "nanoem.gui.viewport.box-selection";
    }
    void
    commitSelection(Model *model, const Project *project, bool removeAll) NANOEM_DECL_OVERRIDE
    {
        const Vector4SI32 rect(deviceScaleRect(project));
        nanoem_rsize_t numBones;
        nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(model->data(), &numBones);
        const ICamera *camera = project->activeCamera();
        IModelObjectSelection *selection = model->selection();
        if (removeAll) {
            selection->removeAllBones();
        }
        for (nanoem_rsize_t i = 0; i < numBones; i++) {
            const nanoem_model_bone_t *bonePtr = bones[i];
            const model::Bone *bone = model::Bone::cast(bonePtr);
            const Vector3 position(model->worldTransform(bone->worldTransform())[3]);
            const Vector2 coord(camera->toDeviceScreenCoordinateInViewport(position));
            if (Inline::intersectsRectPoint(rect, coord)) {
                selection->addBone(bonePtr);
            }
        }
    }
};

class DraggingVertexSelectionState NANOEM_DECL_SEALED : public BaseSelectionState {
public:
    DraggingVertexSelectionState(StateController *stateController, BaseApplicationService *application)
        : BaseSelectionState(stateController, application)
    {
    }
    ~DraggingVertexSelectionState() NANOEM_DECL_NOEXCEPT
    {
    }

private:
    const char *
    name() const NANOEM_DECL_OVERRIDE
    {
        return "nanoem.gui.viewport.select.vertex";
    }
    void
    commitSelection(Model *model, const Project *project, bool removeAll) NANOEM_DECL_OVERRIDE
    {
        const Vector4SI32 rect(deviceScaleRect(project));
        nanoem_rsize_t numVertices;
        nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(model->data(), &numVertices);
        const ICamera *camera = project->activeCamera();
        IModelObjectSelection *selection = model->selection();
        if (removeAll) {
            selection->removeAllVertices();
        }
        for (nanoem_rsize_t i = 0; i < numVertices; i++) {
            const nanoem_model_vertex_t *vertexPtr = vertices[i];
            const model::Vertex *vertex = model::Vertex::cast(vertexPtr);
            const bx::simd128_t v = vertex->m_simd.m_origin;
            const Vector3 position(bx::simd_x(v), bx::simd_y(v), bx::simd_z(v));
            const Vector2 coord(camera->toDeviceScreenCoordinateInViewport(position));
            if (Inline::intersectsRectPoint(rect, coord)) {
                selection->addVertex(vertexPtr);
            }
        }
    }
};

class DraggingFaceSelectionState NANOEM_DECL_SEALED : public BaseSelectionState {
public:
    DraggingFaceSelectionState(StateController *stateController, BaseApplicationService *application)
        : BaseSelectionState(stateController, application)
    {
    }
    ~DraggingFaceSelectionState() NANOEM_DECL_NOEXCEPT
    {
    }

private:
    const char *
    name() const NANOEM_DECL_OVERRIDE
    {
        return "nanoem.gui.viewport.select.face";
    }
    void
    commitSelection(Model *model, const Project *project, bool removeAll) NANOEM_DECL_OVERRIDE
    {
        const Vector4SI32 rect(deviceScaleRect(project));
        nanoem_rsize_t numVertices, numMaterials, numIndices;
        nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(model->data(), &numVertices);
        nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(model->data(), &numMaterials);
        const nanoem_u32_t *indices = nanoemModelGetAllVertexIndices(model->data(), &numIndices);
        const ICamera *camera = project->activeCamera();
        IModelObjectSelection *selection = model->selection();
        if (removeAll) {
            selection->removeAllFaces();
        }
        for (nanoem_rsize_t i = 0, offset = 0; i < numMaterials; i++) {
            const nanoem_f32_t numVI = nanoemModelMaterialGetNumVertexIndices(materials[i]);
            for (nanoem_rsize_t j = 0; j < numVI; j += 3) {
                const nanoem_rsize_t o = offset + j;
                const nanoem_u32_t i0 = indices[o], i1 = indices[o + 1], i2 = indices[o + 2];
                const nanoem_model_vertex_t *v0 = vertices[i0], *v1 = vertices[i1], *v2 = vertices[i2];
                const Vector3 o0(glm::make_vec3(nanoemModelVertexGetOrigin(v0))),
                    o1(glm::make_vec3(nanoemModelVertexGetOrigin(v1))),
                    o2(glm::make_vec3(nanoemModelVertexGetOrigin(v2))),
                    baryCenter(o0 + (o1 - o0) * 0.5f + (o2 - o0) * 0.5f);
                const Vector2 coord(camera->toDeviceScreenCoordinateInViewport(baryCenter));
                if (Inline::intersectsRectPoint(rect, coord)) {
                    const Vector3UI32 face(i0, i1, i2);
                    selection->addFace(face);
                }
            }
        }
    }
};

class DraggingMaterialSelectionState NANOEM_DECL_SEALED : public BaseSelectionState {
public:
    DraggingMaterialSelectionState(StateController *stateController, BaseApplicationService *application)
        : BaseSelectionState(stateController, application)
    {
    }
    ~DraggingMaterialSelectionState() NANOEM_DECL_NOEXCEPT
    {
    }

private:
    const char *
    name() const NANOEM_DECL_OVERRIDE
    {
        return "nanoem.gui.viewport.select.material";
    }
    void
    commitSelection(Model *model, const Project *project, bool removeAll) NANOEM_DECL_OVERRIDE
    {
        const Vector4SI32 rect(deviceScaleRect(project));
        nanoem_rsize_t numVertices, numMaterials, numIndices;
        nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(model->data(), &numVertices);
        nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(model->data(), &numMaterials);
        const nanoem_u32_t *indices = nanoemModelGetAllVertexIndices(model->data(), &numIndices);
        const ICamera *camera = project->activeCamera();
        IModelObjectSelection *selection = model->selection();
        if (removeAll) {
            selection->removeAllMaterials();
        }
        for (nanoem_rsize_t i = 0, offset = 0; i < numMaterials; i++) {
            const nanoem_model_material_t *materialPtr = materials[i];
            const nanoem_f32_t numVI = nanoemModelMaterialGetNumVertexIndices(materialPtr);
            Vector3 aabbMin(FLT_MAX), aabbMax(FLT_MIN);
            for (nanoem_rsize_t j = 0; j < numVI; j += 3) {
                const nanoem_rsize_t o = offset + j;
                const nanoem_u32_t i0 = indices[o], i1 = indices[o + 1], i2 = indices[o + 2];
                const nanoem_model_vertex_t *v0 = vertices[i0], *v1 = vertices[i1], *v2 = vertices[i2];
                const Vector3 o0(glm::make_vec3(nanoemModelVertexGetOrigin(v0))),
                    o1(glm::make_vec3(nanoemModelVertexGetOrigin(v1))),
                    o2(glm::make_vec3(nanoemModelVertexGetOrigin(v2)));
                aabbMin = glm::min(aabbMin, o0);
                aabbMin = glm::min(aabbMin, o1);
                aabbMin = glm::min(aabbMin, o2);
                aabbMax = glm::max(aabbMax, o0);
                aabbMax = glm::max(aabbMax, o1);
                aabbMax = glm::max(aabbMax, o2);
            }
            const Vector3 baryCenter((aabbMin + aabbMax) * 0.5f);
            const Vector2 coord(camera->toDeviceScreenCoordinateInViewport(baryCenter));
            if (Inline::intersectsRectPoint(rect, coord)) {
                selection->addMaterial(materialPtr);
            }
        }
    }
};

class DraggingBoneSelectionState NANOEM_DECL_SEALED : public BaseSelectionState {
public:
    DraggingBoneSelectionState(StateController *stateController, BaseApplicationService *application)
        : BaseSelectionState(stateController, application)
    {
    }
    ~DraggingBoneSelectionState() NANOEM_DECL_NOEXCEPT
    {
    }

private:
    const char *
    name() const NANOEM_DECL_OVERRIDE
    {
        return "nanoem.gui.viewport.select.bone";
    }
    void
    commitSelection(Model *model, const Project *project, bool removeAll) NANOEM_DECL_OVERRIDE
    {
        const Vector4SI32 rect(deviceScaleRect(project));
        nanoem_rsize_t numBones;
        nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(model->data(), &numBones);
        const ICamera *camera = project->activeCamera();
        IModelObjectSelection *selection = model->selection();
        if (removeAll) {
            selection->removeAllBones();
        }
        for (nanoem_rsize_t i = 0; i < numBones; i++) {
            const nanoem_model_bone_t *bonePtr = bones[i];
            const model::Bone *bone = model::Bone::cast(bonePtr);
            const Vector3 position(model->worldTransform(bone->worldTransform())[3]);
            const Vector2 coord(camera->toDeviceScreenCoordinateInViewport(position));
            if (Inline::intersectsRectPoint(rect, coord)) {
                selection->addBone(bonePtr);
            }
        }
    }
};

class DraggingRigidBodySelectionState NANOEM_DECL_SEALED : public BaseSelectionState {
public:
    DraggingRigidBodySelectionState(StateController *stateController, BaseApplicationService *application)
        : BaseSelectionState(stateController, application)
    {
    }
    ~DraggingRigidBodySelectionState() NANOEM_DECL_NOEXCEPT
    {
    }

private:
    const char *
    name() const NANOEM_DECL_OVERRIDE
    {
        return "nanoem.gui.viewport.select.rigid-body";
    }
    void
    commitSelection(Model *model, const Project *project, bool removeAll) NANOEM_DECL_OVERRIDE
    {
        const Vector4SI32 rect(deviceScaleRect(project));
        nanoem_rsize_t numRigidBodies;
        nanoem_model_rigid_body_t *const *bodies = nanoemModelGetAllRigidBodyObjects(model->data(), &numRigidBodies);
        const ICamera *camera = project->activeCamera();
        IModelObjectSelection *selection = model->selection();
        if (removeAll) {
            selection->removeAllRigidBodies();
        }
        Matrix4x4 worldTransform;
        for (nanoem_rsize_t i = 0; i < numRigidBodies; i++) {
            const nanoem_model_rigid_body_t *bodyPtr = bodies[i];
            const model::RigidBody *body = model::RigidBody::cast(bodyPtr);
            worldTransform = body->worldTransform();
            const Vector3 position(model->worldTransform(worldTransform)[3]);
            const Vector2 coord(camera->toDeviceScreenCoordinateInViewport(position));
            if (Inline::intersectsRectPoint(rect, coord)) {
                selection->addRigidBody(bodyPtr);
            }
        }
    }
};

class DraggingJointSelectionState NANOEM_DECL_SEALED : public BaseSelectionState {
public:
    DraggingJointSelectionState(StateController *stateController, BaseApplicationService *application)
        : BaseSelectionState(stateController, application)
    {
    }
    ~DraggingJointSelectionState() NANOEM_DECL_NOEXCEPT
    {
    }

private:
    const char *
    name() const NANOEM_DECL_OVERRIDE
    {
        return "nanoem.gui.viewport.select.joint";
    }
    void
    commitSelection(Model *model, const Project *project, bool removeAll) NANOEM_DECL_OVERRIDE
    {
        const Vector4SI32 rect(deviceScaleRect(project));
        nanoem_rsize_t numJoints;
        nanoem_model_joint_t *const *joints = nanoemModelGetAllJointObjects(model->data(), &numJoints);
        const ICamera *camera = project->activeCamera();
        IModelObjectSelection *selection = model->selection();
        if (removeAll) {
            selection->removeAllJoints();
        }
        Matrix4x4 worldTransformA, worldTransformB;
        for (nanoem_rsize_t i = 0; i < numJoints; i++) {
            const nanoem_model_joint_t *jointPtr = joints[i];
            const model::Joint *joint = model::Joint::cast(jointPtr);
            joint->getWorldTransformA(glm::value_ptr(worldTransformA));
            const Vector3 positionA(model->worldTransform(worldTransformA)[3]);
            const Vector2 coordA(camera->toDeviceScreenCoordinateInViewport(positionA));
            if (Inline::intersectsRectPoint(rect, coordA)) {
                selection->addJoint(jointPtr);
            }
            const Vector3 positionB(model->worldTransform(worldTransformB)[3]);
            const Vector2 coordB(camera->toDeviceScreenCoordinateInViewport(positionB));
            if (Inline::intersectsRectPoint(rect, coordB)) {
                selection->addJoint(jointPtr);
            }
        }
    }
};

class DraggingMoveCameraLookAtState NANOEM_DECL_SEALED : public BaseState {
public:
    DraggingMoveCameraLookAtState(StateController *stateController, BaseApplicationService *application)
        : BaseState(stateController, application)
        , m_lastDraggingState(nullptr)
        , m_scaleFactor(0.0f)
    {
    }
    ~DraggingMoveCameraLookAtState() NANOEM_DECL_NOEXCEPT
    {
    }

    void
    onPress(const Vector3SI32 &logicalScalePosition) NANOEM_DECL_OVERRIDE
    {
        Project *project = m_stateControllerPtr->currentProject();
        m_lastDraggingState =
            nanoem_new(internal::CameraLookAtState(project, project->globalCamera(), logicalScalePosition));
        m_scaleFactor = m_lastDraggingState->scaleFactor();
    }
    void
    onMove(const Vector3SI32 &logicalScalePosition, const Vector2SI32 & /* delta */) NANOEM_DECL_OVERRIDE
    {
        if (m_lastDraggingState) {
            m_lastDraggingState->setScaleFactor(
                m_scaleFactor * BaseDraggingObjectState::scaleFactor(logicalScalePosition));
            m_lastDraggingState->transform(logicalScalePosition);
        }
    }
    void
    onRelease(const Vector3SI32 &logicalScalePosition) NANOEM_DECL_OVERRIDE
    {
        if (m_lastDraggingState) {
            m_lastDraggingState->commit(logicalScalePosition);
            nanoem_delete_safe(m_lastDraggingState);
        }
    }

    internal::IDraggingState *m_lastDraggingState;
    nanoem_f32_t m_scaleFactor;
};

class UndoState NANOEM_DECL_SEALED : public BaseState {
public:
    UndoState(StateController *stateController, BaseApplicationService *application)
        : BaseState(stateController, application)
    {
    }
    ~UndoState() NANOEM_DECL_NOEXCEPT
    {
    }

    void
    onPress(const Vector3SI32 & /* logicalScalePosition */) NANOEM_DECL_OVERRIDE
    {
    }
    void
    onMove(const Vector3SI32 & /* logicalScalePosition */, const Vector2SI32 & /* delta */) NANOEM_DECL_OVERRIDE
    {
    }
    void
    onRelease(const Vector3SI32 & /* logicalScalePosition */) NANOEM_DECL_OVERRIDE
    {
        m_stateControllerPtr->currentProject()->handleUndoAction();
    }
};

class RedoState NANOEM_DECL_SEALED : public BaseState {
public:
    RedoState(StateController *stateController, BaseApplicationService *application)
        : BaseState(stateController, application)
    {
    }
    ~RedoState() NANOEM_DECL_NOEXCEPT
    {
    }

    void
    onPress(const Vector3SI32 & /* logicalScalePosition */) NANOEM_DECL_OVERRIDE
    {
    }
    void
    onMove(const Vector3SI32 & /* logicalScalePosition */, const Vector2SI32 & /* delta */) NANOEM_DECL_OVERRIDE
    {
    }
    void
    onRelease(const Vector3SI32 & /* logicalScalePosition */) NANOEM_DECL_OVERRIDE
    {
        m_stateControllerPtr->currentProject()->handleRedoAction();
    }
};

class DrawUtil NANOEM_DECL_SEALED : private NonCopyable {
public:
    static Vector2SI32
    deviceScaleCursorActiveBoneInViewport(const Project *project)
    {
        Vector2SI32 deviceScaleCursor(0);
        if (const Model *model = project->activeModel()) {
            if (const model::Bone *bone = model::Bone::cast(model->activeBone())) {
                deviceScaleCursor =
                    project->activeCamera()->toDeviceScreenCoordinateInViewport(bone->worldTransformOrigin());
            }
        }
        return deviceScaleCursor;
    }
    static void
    drawBoneMoveHandle(IPrimitive2D *primitive, const Project *project)
    {
        const Vector2 center(deviceScaleCursorActiveBoneInViewport(project));
        nanoem_f32_t length = project->deviceScaleCircleRadius() * 10.0f,
                     thickness = project->logicalScaleCircleRadius();
        primitive->strokeLine(center, Vector2(center.x + length, center.y), Vector4(1, 0, 0, 1), thickness);
        primitive->strokeLine(center, Vector2(center.x, center.y - length), Vector4(0, 1, 0, 1), thickness);
    }
    static void
    drawBoneRotateHandle(IPrimitive2D *primitive, const Project *project)
    {
        const Vector2 center(deviceScaleCursorActiveBoneInViewport(project));
        nanoem_f32_t radius = project->deviceScaleCircleRadius() * 7.5f,
                     thickness = project->logicalScaleCircleRadius() * 1.5f;
        const nanoem_f32_t kappa = 0.5522847498307933984022516322796f;
        const nanoem_f32_t x1 = center.x - radius;
        const nanoem_f32_t y1 = center.y - radius;
        const nanoem_f32_t x2 = center.x + radius;
        const nanoem_f32_t y2 = center.y + radius;
        primitive->strokeLine(
            Vector2(center.x - radius, center.y), Vector2(center.x + radius, center.y), Vector4(0, 0, 1, 1), thickness);
        primitive->strokeLine(
            Vector2(center.x, center.y - radius), Vector2(center.x, center.y + radius), Vector4(1, 0, 0, 1), thickness);
        primitive->strokeCurve(Vector2(x1, center.y + 1), Vector2(x1, center.y - radius * kappa),
            Vector2(center.x - radius * kappa, y1), Vector2(center.x + 1, y1), Vector4(0, 1, 0, 1), thickness);
        primitive->strokeCurve(Vector2(center.x - 1, y1), Vector2(center.x + radius * kappa, y1),
            Vector2(x2, center.y - radius * kappa), Vector2(x2, center.y + 1), Vector4(0, 1, 0, 1), thickness);
        primitive->strokeCurve(Vector2(x2, center.y - 1), Vector2(x2, center.y + radius * kappa),
            Vector2(center.x + radius * kappa, y2), Vector2(center.x - 1, y2), Vector4(0, 1, 0, 1), thickness);
        primitive->strokeCurve(Vector2(center.x + 1, y2), Vector2(center.x - radius * kappa, y2),
            Vector2(x1, center.y + radius * kappa), Vector2(x1, center.y - 1), Vector4(0, 1, 0, 1), thickness);
    }
    static void
    drawActiveBonePoint(IPrimitive2D *primitive, const Vector4 &activeBoneColor, const Project *project)
    {
        const Vector2SI32 &center = deviceScaleCursorActiveBoneInViewport(project);
        if (center.x != 0 && center.y != 0) {
            const nanoem_f32_t radius = project->deviceScaleCircleRadius(), extent = radius * 2;
            primitive->fillCircle(Vector4(center.x - radius, center.y - radius, extent, extent), activeBoneColor);
        }
    }
    static void
    drawCameraLookAtPoint(IPrimitive2D *primitive, const ICamera *camera, const Project *project)
    {
        const Vector2 center(camera->toDeviceScreenCoordinateInViewport(camera->boundLookAt()));
        const nanoem_f32_t circleRadius = project->deviceScaleCircleRadius();
        const nanoem_f32_t innerRadius = circleRadius * 0.65f;
        primitive->strokeCircle(
            Vector4(center.x - circleRadius, center.y - circleRadius, circleRadius * 2, circleRadius * 2),
            Vector4(1, 0, 0, 1), 2.0f);
        primitive->fillCircle(Vector4(center.x - innerRadius, center.y - innerRadius, innerRadius * 2, innerRadius * 2),
            Vector4(1, 0, 0, 1));
    }
};

} /* namespace anonymous */

StateController::StateController(BaseApplicationService *application, IFileManager *fileManager)
    : m_applicationPtr(application)
    , m_fileManager(fileManager)
    , m_state(nullptr)
    , m_project(nullptr, QueuedProjectList())
    , m_lastEditingMode(Project::kEditingModeNone)
    , m_boneIndex(0)
    , m_frameIndex(0, 0)
    , m_elapsedTime(0, stm_now())
{
    nanoem_parameter_assert(m_applicationPtr, "must not be nullptr");
}

StateController::~StateController() NANOEM_DECL_NOEXCEPT
{
    setState(nullptr);
}

void
StateController::drawPrimitive2D(IPrimitive2D *primitive, nanoem_u32_t flags)
{
    if (Project *project = currentProject()) {
        if (m_state) {
            m_state->onDrawPrimitive2D(primitive);
        }
        const Vector2SI32 &deviceScaleCursor = project->deviceScaleMovingCursorPosition();
        const Project::ModelList &allModels = project->allModels();
        Vector4 activeBoneColor(1, 0, 0, 1);
        for (Project::ModelList::const_iterator it = allModels.begin(), end = allModels.end(); it != end; ++it) {
            Model *model = *it;
            if (!model->isVisible() || model != project->activeModel()) {
                continue;
            }
            if (EnumUtils::isEnabled(Project::kDrawTypeBoneConnections, flags)) {
                model->drawBoneConnections(deviceScaleCursor);
            }
            if (EnumUtils::isEnabled(Project::kDrawTypeConstraintConnections, flags)) {
                model->drawConstraintConnections(deviceScaleCursor);
            }
            if (EnumUtils::isEnabled(Project::kDrawTypeConstraintHeatmaps, flags)) {
                model->drawConstraintsHeatMap();
            }
        }
        if (EnumUtils::isEnabled(Project::kDrawTypeBoneMoveHandle, flags)) {
            activeBoneColor = Vector4(1, 1, 0, 1);
            DrawUtil::drawBoneMoveHandle(primitive, project);
        }
        if (EnumUtils::isEnabled(Project::kDrawTypeBoneRotateHandle, flags)) {
            activeBoneColor = Vector4(1, 1, 0, 1);
            DrawUtil::drawBoneRotateHandle(primitive, project);
        }
        if (EnumUtils::isEnabled(Project::kDrawTypeActiveBone, flags)) {
            DrawUtil::drawActiveBonePoint(primitive, activeBoneColor, project);
        }
        const ICamera *camera = project->activeCamera();
        if (EnumUtils::isEnabled(Project::kDrawTypeCameraLookAt, flags) && camera) {
            DrawUtil::drawCameraLookAtPoint(primitive, camera, project);
        }
    }
}

const Project *
StateController::currentProject() const
{
    return m_project.first;
}

Project *
StateController::currentProject()
{
    return m_project.first;
}

BaseApplicationService *
StateController::application() NANOEM_DECL_NOEXCEPT
{
    return m_applicationPtr;
}

IFileManager *
StateController::fileManager() NANOEM_DECL_NOEXCEPT
{
    return m_fileManager;
}

const StateController::IState *
StateController::state() const NANOEM_DECL_NOEXCEPT
{
    return m_state;
}

StateController::IState *
StateController::state() NANOEM_DECL_NOEXCEPT
{
    return m_state;
}

void
StateController::setState(IState *state)
{
    if (state != m_state) {
        nanoem_delete(m_state);
        m_state = state;
    }
}

void
StateController::consumeDefaultPass()
{
    const nanoem_u64_t globalFrameIndex = m_frameIndex.first++;
    if (Project *project = currentProject()) {
        project->update();
        project->setLastScrollDelta(Vector2());
        const nanoem_u64_t currentTimeTicks = stm_now();
        const nanoem_f64_t currentUptimeSeconds = stm_sec(stm_diff(currentTimeTicks, m_elapsedTime.second));
        project->setUptimeSeconds(currentUptimeSeconds);
        if (stm_sec(stm_diff(currentTimeTicks, m_elapsedTime.first)) >= 1.0) {
            project->setActualFPS(nanoem_u32_t(globalFrameIndex - m_frameIndex.second));
            m_frameIndex.second = globalFrameIndex;
            m_elapsedTime.first = currentTimeTicks;
        }
        project->eventPublisher()->publishConsumePassEvent(globalFrameIndex);
    }
    QueuedProjectList &deletables = m_project.second;
    if (!deletables.empty()) {
        for (QueuedProjectList::const_iterator it = deletables.begin(), end = deletables.end(); it != end; ++it) {
            Project *deletable = *it;
            m_applicationPtr->destroyProject(deletable);
        }
        deletables.clear();
    }
}

void
StateController::handlePointerScroll(const Vector3SI32 &logicalScalePosition, const Vector2SI32 &delta)
{
    if (Project *project = currentProject()) {
        if (intersectsViewportLayoutRect(project, logicalScalePosition) && !project->audioPlayer()->isPlaying() &&
            delta.y != 0) {
            ICamera *camera = project->activeCamera();
            camera->setDistance(
                camera->distance() + delta.y * BaseDraggingObjectState::scaleFactor(logicalScalePosition));
            camera->update();
            if (project->editingMode() != Project::kEditingModeSelect) {
                const Project::ModelList &allModels = project->allModels();
                for (Project::ModelList::const_iterator it = allModels.begin(), end = allModels.end(); it != end;
                     ++it) {
                    Model *model = *it;
                    if (model->edgeSizeScaleFactor() > 0.0f) {
                        model->markStagingVertexBufferDirty();
                    }
                }
            }
        }
        project->setLastScrollDelta(delta);
    }
}

void
StateController::handlePointerPress(const Vector3SI32 &logicalScalePosition, Project::CursorType type)
{
    if (Project *project = currentProject()) {
        switch (type) {
        case Project::kCursorTypeMouseLeft: {
            if (project->isPrimaryCursorTypeLeft()) {
                setPrimaryDraggingState(project, logicalScalePosition);
            }
            else {
                setSecondaryDraggingState(project, logicalScalePosition);
            }
            break;
        }
        case Project::kCursorTypeMouseMiddle: {
            setState(nanoem_new(DraggingMoveCameraLookAtState(this, m_applicationPtr)));
            break;
        }
        case Project::kCursorTypeMouseRight: {
            if (!project->isPrimaryCursorTypeLeft()) {
                setPrimaryDraggingState(project, logicalScalePosition);
            }
            else {
                setSecondaryDraggingState(project, logicalScalePosition);
            }
            break;
        }
        case Project::kCursorTypeMouseUndo: {
            setState(nanoem_new(UndoState(this, m_applicationPtr)));
            break;
        }
        case Project::kCursorTypeMouseRedo: {
            setState(nanoem_new(RedoState(this, m_applicationPtr)));
            break;
        }
        default:
            break;
        }
        if (m_state) {
            m_state->onPress(logicalScalePosition);
        }
        project->setLogicalPixelLastCursorPosition(type, logicalScalePosition, true);
        project->setCursorModifiers(static_cast<nanoem_u32_t>(logicalScalePosition.z));
    }
}

void
StateController::handlePointerMove(const Vector3SI32 &logicalScalePosition, const Vector2SI32 &delta)
{
    if (Project *project = currentProject()) {
        if (m_state) {
            m_state->onMove(logicalScalePosition, delta);
        }
        else {
            if (Model *model = project->activeModel()) {
                Model::AxisType axisType;
                const nanoem_model_bone_t *bonePtr =
                    DraggingBoneState::findHoverBone(logicalScalePosition, project, axisType);
                model->selection()->setHoveredBone(const_cast<nanoem_model_bone_t *>(bonePtr));
                model->setTransformAxisType(axisType);
            }
            project->setLogicalPixelMovingCursorPosition(logicalScalePosition);
            project->setCursorModifiers(static_cast<nanoem_u32_t>(logicalScalePosition.z));
        }
    }
}

void
StateController::handlePointerRelease(const Vector3SI32 &logicalScalePosition, Project::CursorType type)
{
    if (Project *project = currentProject()) {
        if (m_state) {
            m_state->onRelease(logicalScalePosition);
            setState(nullptr);
        }
        project->setLogicalPixelLastCursorPosition(type, logicalScalePosition, false);
        project->setCursorModifiers(static_cast<nanoem_u32_t>(logicalScalePosition.z));
    }
}

Project *
StateController::createProject()
{
    Project *newProject = 0;
    if (Project *lastProject = currentProject()) {
        newProject = m_applicationPtr->createProject(lastProject->windowSize(), lastProject->viewportPixelFormat(),
            lastProject->windowDevicePixelRatio(), lastProject->viewportDevicePixelRatio(), m_dllPath.c_str());
        newProject->setPreferredEditingFPS(lastProject->preferredEditingFPS());
        newProject->setPrimaryCursorTypeLeft(lastProject->isPrimaryCursorTypeLeft());
        newProject->setActive(true);
    }
    else {
        newProject =
            m_applicationPtr->createProject(Vector2UI16(1), SG_PIXELFORMAT_RGBA8, 1.0f, 1.0f, m_dllPath.c_str());
    }
    return newProject;
}

void
StateController::newProject()
{
    setProject(createProject());
}

void
StateController::newProject(const Vector2UI16 &windowSize, const char *dllPath, sg_pixel_format pixelFormat,
    nanoem_f32_t windowDevicePixelRatio, nanoem_f32_t viewportDevicePixelRatio, nanoem_u32_t fps)
{
    Project *newProject = m_applicationPtr->createProject(
        windowSize, pixelFormat, windowDevicePixelRatio, viewportDevicePixelRatio, dllPath);
    newProject->setPreferredEditingFPS(fps);
    setProject(newProject);
    m_dllPath = dllPath;
}

void
StateController::setProject(Project *newProject)
{
    m_project.second.push_back(m_project.first);
    m_project.first = newProject;
    if (newProject) {
        dd::initialize(newProject->sharedDebugDrawer());
    }
    else if (!newProject) {
        dd::shutdown();
    }
}

void
StateController::toggleEditingMode(Project::EditingMode value)
{
    if (Project *project = currentProject()) {
        if (project->editingMode() == value) {
            project->setEditingMode(m_lastEditingMode);
        }
        else {
            m_lastEditingMode = project->editingMode();
            project->setEditingMode(value);
        }
    }
}

nanoem_u64_t
StateController::globalFrameIndex() const NANOEM_DECL_NOEXCEPT
{
    return m_frameIndex.first;
}

nanoem_rsize_t
StateController::boneIndex() const NANOEM_DECL_NOEXCEPT
{
    return m_boneIndex;
}

void
StateController::setBoneIndex(nanoem_rsize_t value)
{
    m_boneIndex = value;
}

bool
StateController::intersectsViewportLayoutRect(
    const Project *project, const Vector2SI32 &logicalScalePosition) const NANOEM_DECL_NOEXCEPT
{
    const Vector4 rect(project->logicalScaleUniformedViewportLayoutRect());
    bool intersected = Inline::intersectsRectPoint(rect, logicalScalePosition) && project->isViewportHovered() &&
        !m_applicationPtr->hasModalDialog();
    return intersected;
}

void
StateController::setPrimaryDraggingState(Project *project, const Vector2SI32 &logicalScalePosition)
{
    if (intersectsViewportLayoutRect(project, logicalScalePosition)) {
        IState *state = nullptr;
        if (const Model *model = project->activeModel()) {
            const IModelObjectSelection *selection = model->selection();
            if (!project->isModelEditingEnabled()) {
                if (selection->isBoxSelectedBoneModeEnabled()) {
                    state = nanoem_new(DraggingBoxSelectedBoneState(this, m_applicationPtr));
                }
                else {
                    state = nanoem_new(DraggingBoneState(this, m_applicationPtr, true));
                }
            }
            else {
                switch (selection->editingType()) {
                case IModelObjectSelection::kEditingTypeBone:
                    state = nanoem_new(DraggingBoneSelectionState(this, m_applicationPtr));
                    break;
                case IModelObjectSelection::kEditingTypeFace:
                    state = nanoem_new(DraggingFaceSelectionState(this, m_applicationPtr));
                    break;
                case IModelObjectSelection::kEditingTypeJoint:
                    state = nanoem_new(DraggingJointSelectionState(this, m_applicationPtr));
                    break;
                case IModelObjectSelection::kEditingTypeMaterial:
                    state = nanoem_new(DraggingMaterialSelectionState(this, m_applicationPtr));
                    break;
                case IModelObjectSelection::kEditingTypeRigidBody:
                    state = nanoem_new(DraggingRigidBodySelectionState(this, m_applicationPtr));
                    break;
                case IModelObjectSelection::kEditingTypeVertex:
                    state = nanoem_new(DraggingVertexSelectionState(this, m_applicationPtr));
                    break;
                case IModelObjectSelection::kEditingTypeInfo:
                case IModelObjectSelection::kEditingTypeLabel:
                case IModelObjectSelection::kEditingTypeMorph:
                case IModelObjectSelection::kEditingTypeSoftBody:
                case IModelObjectSelection::kEditingTypeNone:
                default:
                    state = nanoem_new(DraggingCameraState(this, m_applicationPtr, true));
                    break;
                }
            }
        }
        else {
            state = nanoem_new(DraggingCameraState(this, m_applicationPtr, true));
        }
        setState(state);
    }
}

void
StateController::setSecondaryDraggingState(Project *project, const Vector2SI32 &logicalScalePosition)
{
    if (intersectsViewportLayoutRect(project, logicalScalePosition)) {
        IState *state = nullptr;
        if (project->activeModel() && !project->isModelEditingEnabled()) {
            state = nanoem_new(DraggingBoneState(this, m_applicationPtr, false));
        }
        else {
            state = nanoem_new(DraggingCameraState(this, m_applicationPtr, false));
        }
        setState(state);
    }
}

} /* namespace nanoem */
