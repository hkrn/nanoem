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
#include "emapp/IState.h"
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

namespace {

class BaseState : public IState, private NonCopyable {
public:
    static bool isDraggingBoneState(const IState *value) NANOEM_DECL_NOEXCEPT;

protected:
    BaseState(StateController *stateController, BaseApplicationService *application);

    void onDrawPrimitive2D(IPrimitive2D * /* primitive */) NANOEM_DECL_OVERRIDE;
    bool isGrabbingHandle() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

    StateController *m_stateControllerPtr;
    BaseApplicationService *m_applicationPtr;
    Vector2SI32 m_lastLogicalScalePosition;
    bool m_isGrabbingHandle;
};

bool
BaseState::isDraggingBoneState(const IState *value) NANOEM_DECL_NOEXCEPT
{
    bool result = false;
    if (value) {
        switch (value->type()) {
        case IState::kTypeDraggingBoneAxisAlignedOrientateActiveBoneState:
        case IState::kTypeDraggingBoneAxisAlignedTranslateActiveBoneState:
        case IState::kTypeDraggingBoneOrientateActiveBoneState:
        case IState::kTypeDraggingBoneTranslateActiveBoneState:
            result = true;
            break;
        default:
            break;
        }
    }
    return value;
}

BaseState::BaseState(StateController *stateController, BaseApplicationService *application)
    : m_stateControllerPtr(stateController)
    , m_applicationPtr(application)
    , m_lastLogicalScalePosition(0)
    , m_isGrabbingHandle(false)
{
    nanoem_parameter_assert(m_stateControllerPtr, "must not be nullptr");
    nanoem_parameter_assert(m_applicationPtr, "must not be nullptr");
}

void
BaseState::onDrawPrimitive2D(IPrimitive2D *)
{
}

bool
BaseState::isGrabbingHandle() const NANOEM_DECL_NOEXCEPT
{
    return m_isGrabbingHandle;
}

class BaseDraggingObjectState : public BaseState {
public:
    static nanoem_f32_t scaleFactor(const Vector3SI32 &logicalScaleCursorPosition) NANOEM_DECL_NOEXCEPT;

    BaseDraggingObjectState(StateController *stateController, BaseApplicationService *application, bool canUpdateAngle);
    ~BaseDraggingObjectState() NANOEM_DECL_NOEXCEPT;

    Type type() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

protected:
    void onMove(const Vector3SI32 &logicalScaleCursorPosition, const Vector2SI32 &delta) NANOEM_DECL_OVERRIDE;
    void onRelease(const Vector3SI32 &logicalScaleCursorPosition) NANOEM_DECL_OVERRIDE;

    virtual internal::IDraggingState *createTranslateState(
        int axisIndex, const Vector3SI32 &logicalScaleCursorPosition, Project *project) = 0;
    virtual internal::IDraggingState *createOrientateState(
        int axisIndex, const Vector3SI32 &logicalScaleCursorPosition, Project *project) = 0;
    virtual internal::IDraggingState *createCameraLookAtState(
        const Vector3SI32 &logicalScaleCursorPosition, Project *project) = 0;
    virtual internal::IDraggingState *createCameraZoomState(
        const Vector3SI32 &logicalScaleCursorPosition, Project *project) = 0;

    internal::IDraggingState *createDraggingState(
        Project::RectangleType rectangleType, const Vector3SI32 &logicalScaleCursorPosition, Project *project);
    void setDraggingState(internal::IDraggingState *draggingState, const Vector3SI32 &logicalScaleCursorPosition);
    void updateCameraAngle(const Vector2SI32 &delta);
    void setType(IState::Type value);
    bool canUpdateCameraAngle() const NANOEM_DECL_NOEXCEPT;

private:
    static bool isBackgroundVideoOperation(
        const Vector3SI32 &logicalScaleCursorPosition, const Project *project) NANOEM_DECL_NOEXCEPT;

    const bool m_canUpdateAngle;
    internal::IDraggingState *m_lastDraggingState;
    IState::Type m_type;
    nanoem_f32_t m_scaleFactor;
};

BaseDraggingObjectState::BaseDraggingObjectState(
    StateController *stateController, BaseApplicationService *application, bool canUpdateAngle)
    : BaseState(stateController, application)
    , m_canUpdateAngle(canUpdateAngle)
    , m_lastDraggingState(nullptr)
    , m_scaleFactor(0.0f)
    , m_type(IState::kTypeMaxEnum)
{
}

BaseDraggingObjectState::~BaseDraggingObjectState() NANOEM_DECL_NOEXCEPT
{
    nanoem_delete_safe(m_lastDraggingState);
}

IState::Type
BaseDraggingObjectState::type() const NANOEM_DECL_NOEXCEPT
{
    return m_type;
}

void
BaseDraggingObjectState::onMove(const Vector3SI32 &logicalScaleCursorPosition, const Vector2SI32 &delta)
{
    if (m_lastDraggingState) {
        m_lastDraggingState->setScaleFactor(m_scaleFactor * scaleFactor(logicalScaleCursorPosition));
        m_lastDraggingState->transform(logicalScaleCursorPosition);
    }
    else if (canUpdateCameraAngle()) {
        updateCameraAngle(delta);
    }
    m_lastLogicalScalePosition = logicalScaleCursorPosition;
}

void
BaseDraggingObjectState::onRelease(const Vector3SI32 &logicalScaleCursorPosition)
{
    if (m_lastDraggingState) {
        m_lastDraggingState->commit(logicalScaleCursorPosition);
    }
    else if (Project *project = m_stateControllerPtr->currentProject()) {
        if (Model *model = project->activeModel()) {
            model->setTransformAxisType(Model::kAxisTypeMaxEnum);
        }
    }
    m_lastLogicalScalePosition = Vector2();
}

internal::IDraggingState *
BaseDraggingObjectState::createDraggingState(
    Project::RectangleType rectangleType, const Vector3SI32 &logicalScaleCursorPosition, Project *project)
{
    int axisIndex;
    const bool orientate =
        rectangleType >= Project::kRectangleOrientateX && rectangleType <= Project::kRectangleOrientateZ;
    const bool translate =
        rectangleType >= Project::kRectangleTranslateX && rectangleType <= Project::kRectangleTranslateZ;
    internal::IDraggingState *draggingState = nullptr;
    if (orientate) {
        axisIndex = rectangleType - Project::kRectangleOrientateX;
        draggingState = createOrientateState(axisIndex, logicalScaleCursorPosition, project);
    }
    else if (translate) {
        axisIndex = rectangleType - Project::kRectangleTranslateX;
        draggingState = createTranslateState(axisIndex, logicalScaleCursorPosition, project);
    }
    else if (rectangleType == Project::kRectangleTransformCoordinateType) {
        project->toggleTransformCoordinateType();
    }
    else if (rectangleType == Project::kRectangleCameraLookAt) {
        if (isBackgroundVideoOperation(logicalScaleCursorPosition, project)) {
            draggingState = nanoem_new(internal::MoveBackgroundVideoState(project, logicalScaleCursorPosition));
        }
        else {
            draggingState = createCameraLookAtState(logicalScaleCursorPosition, project);
        }
    }
    else if (rectangleType == Project::kRectangleCameraZoom) {
        if (isBackgroundVideoOperation(logicalScaleCursorPosition, project)) {
            draggingState = nanoem_new(internal::ZoomBackgroundVideoState(project, logicalScaleCursorPosition));
        }
        else {
            draggingState = createCameraZoomState(logicalScaleCursorPosition, project);
        }
    }
    return draggingState;
}

void
BaseDraggingObjectState::setDraggingState(
    internal::IDraggingState *draggingState, const Vector3SI32 &logicalScaleCursorPosition)
{
    if (draggingState) {
        const nanoem_f32_t sf = draggingState->scaleFactor();
        draggingState->setScaleFactor(sf * scaleFactor(logicalScaleCursorPosition));
        m_lastDraggingState = draggingState;
        m_scaleFactor = sf;
        m_isGrabbingHandle = true;
    }
}

void
BaseDraggingObjectState::updateCameraAngle(const Vector2SI32 &delta)
{
    if (Project *project = m_stateControllerPtr->currentProject()) {
        ICamera *camera = project->activeCamera();
        camera->setAngle(glm::radians(glm::degrees(camera->angle()) + Vector3(delta.y, delta.x, 0)));
        camera->update();
    }
}

void
BaseDraggingObjectState::setType(IState::Type value)
{
    m_type = value;
}

bool
BaseDraggingObjectState::canUpdateCameraAngle() const NANOEM_DECL_NOEXCEPT
{
    bool canUpdate = m_canUpdateAngle;
    if (const Project *project = m_stateControllerPtr->currentProject()) {
        canUpdate &= !(m_applicationPtr->hasModalDialog() || project->audioPlayer()->isPlaying());
    }
    return canUpdate;
}

bool
BaseDraggingObjectState::isBackgroundVideoOperation(
    const Vector3SI32 &logicalScaleCursorPosition, const Project *project) NANOEM_DECL_NOEXCEPT
{
    return project->hasBackgroundImageHandle() &&
        EnumUtils::isEnabledT<int>(Project::kCursorModifierTypeShift, logicalScaleCursorPosition.z);
}

class DraggingBoneState NANOEM_DECL_SEALED : public BaseDraggingObjectState {
public:
    static const nanoem_model_bone_t *findHoverBone(
        const Vector3SI32 &value, const Project *project, Model::AxisType &axisType);

    DraggingBoneState(StateController *stateController, BaseApplicationService *application, bool canUpdateAngle);
    ~DraggingBoneState() NANOEM_DECL_NOEXCEPT;

    internal::IDraggingState *createTranslateState(
        int axisIndex, const Vector3SI32 &logicalScaleCursorPosition, Project *project) NANOEM_DECL_OVERRIDE;
    internal::IDraggingState *createOrientateState(
        int axisIndex, const Vector3SI32 &logicalScaleCursorPosition, Project *project) NANOEM_DECL_OVERRIDE;
    internal::IDraggingState *createCameraLookAtState(
        const Vector3SI32 &logicalScaleCursorPosition, Project *project) NANOEM_DECL_OVERRIDE;
    internal::IDraggingState *createCameraZoomState(
        const Vector3SI32 &logicalScaleCursorPosition, Project *project) NANOEM_DECL_OVERRIDE;

    void onPress(const Vector3SI32 &logicalScaleCursorPosition) NANOEM_DECL_OVERRIDE;

private:
    static Vector2SI32 deviceScaleCursorActiveBoneInWindow(const Project *project) NANOEM_DECL_NOEXCEPT;
    static Vector2SI32 logicalScaleCursorActiveBoneInWindow(const Project *project) NANOEM_DECL_NOEXCEPT;
    static bool handlePointerIntersects(const Vector2SI32 &logicalScaleCursorPosition, const Project *project,
        const nanoem_model_bone_t *&bone, nanoem_rsize_t &boneIndex, Model::AxisType &axisType) NANOEM_DECL_NOEXCEPT;
    static Model::AxisType selectTranslationAxisType(
        const Vector2SI32 &deviceScaleCursorPosition, const Project *project) NANOEM_DECL_NOEXCEPT;
    static Model::AxisType selectOrientationAxisType(
        const Vector2SI32 &deviceScaleCursorPosition, const Project *project) NANOEM_DECL_NOEXCEPT;
};

const nanoem_model_bone_t *
DraggingBoneState::findHoverBone(const Vector3SI32 &value, const Project *project, Model::AxisType &axisType)
{
    const nanoem_model_bone_t *bone = nullptr;
    nanoem_rsize_t boneIndex;
    DraggingBoneState::handlePointerIntersects(value, project, bone, boneIndex, axisType);
    return bone;
}

DraggingBoneState::DraggingBoneState(
    StateController *stateController, BaseApplicationService *application, bool canUpdateAngle)
    : BaseDraggingObjectState(stateController, application, canUpdateAngle)
{
}

DraggingBoneState::~DraggingBoneState() NANOEM_DECL_NOEXCEPT
{
}

internal::IDraggingState *
DraggingBoneState::createTranslateState(int axisIndex, const Vector3SI32 &logicalScaleCursorPosition, Project *project)
{
    internal::IDraggingState *state = nullptr;
    Model *model = project->activeModel();
    if (model && model->selection()->areAllBonesMovable()) {
        state = nanoem_new(
            internal::AxisAlignedTranslateActiveBoneState(project, model, logicalScaleCursorPosition, axisIndex));
        setType(IState::kTypeDraggingBoneAxisAlignedTranslateActiveBoneState);
    }
    return state;
}

internal::IDraggingState *
DraggingBoneState::createOrientateState(int axisIndex, const Vector3SI32 &logicalScaleCursorPosition, Project *project)
{
    internal::IDraggingState *state = nullptr;
    Model *model = project->activeModel();
    if (model && model->selection()->areAllBonesRotateable()) {
        state = nanoem_new(
            internal::AxisAlignedOrientateActiveBoneState(project, model, logicalScaleCursorPosition, axisIndex));
        setType(IState::kTypeDraggingBoneAxisAlignedOrientateActiveBoneState);
    }
    return state;
}

internal::IDraggingState *
DraggingBoneState::createCameraLookAtState(const Vector3SI32 &logicalScaleCursorPosition, Project *project)
{
    ICamera *camera = project->activeModel()->localCamera();
    internal::IDraggingState *state =
        nanoem_new(internal::CameraLookAtState(project, camera, logicalScaleCursorPosition));
    setType(IState::kTypeDraggingCameraLookAtState);
    return state;
}

internal::IDraggingState *
DraggingBoneState::createCameraZoomState(const Vector3SI32 &logicalScaleCursorPosition, Project *project)
{
    ICamera *camera = project->activeModel()->localCamera();
    internal::IDraggingState *state =
        nanoem_new(internal::CameraZoomState(project, camera, logicalScaleCursorPosition));
    setType(IState::kTypeDraggingCameraZoomState);
    return state;
}

void
DraggingBoneState::onPress(const Vector3SI32 &logicalScaleCursorPosition)
{
    Project::RectangleType rectangleType = Project::kRectangleTypeMaxEnum;
    internal::IDraggingState *draggingState = nullptr;
    Project *project = m_stateControllerPtr->currentProject();
    Model *model = project->activeModel();
    if (!project || !model || project->isPlaying()) {
        /* do nothing */
    }
    else if (project->intersectsTransformHandle(logicalScaleCursorPosition, rectangleType)) {
        draggingState = createDraggingState(rectangleType, logicalScaleCursorPosition, project);
    }
    else if (const nanoem_model_bone_t *bonePtr = model->activeBone()) {
        const Vector2SI32 activeBoneCursorPosition(logicalScaleCursorActiveBoneInWindow(project));
        if (model->transformAxisType() != Model::kAxisTypeMaxEnum) {
            const Project::EditingMode editingMode = project->editingMode();
            const IModelObjectSelection *selection = model->selection();
            if (editingMode == Project::kEditingModeRotate && selection->areAllBonesRotateable()) {
                draggingState = nanoem_new(internal::OrientateActiveBoneState(
                    project, model, logicalScaleCursorPosition, activeBoneCursorPosition));
                setType(IState::kTypeDraggingBoneOrientateActiveBoneState);
            }
            else if (editingMode == Project::kEditingModeMove && selection->areAllBonesMovable()) {
                draggingState = nanoem_new(internal::TranslateActiveBoneState(
                    project, model, logicalScaleCursorPosition, activeBoneCursorPosition));
                setType(IState::kTypeDraggingBoneTranslateActiveBoneState);
            }
        }
    }
    if (draggingState) {
        setDraggingState(draggingState, logicalScaleCursorPosition);
    }
    else if (project && project->editingMode() == Project::kEditingModeSelect) {
        const nanoem_model_bone_t *bone = nullptr;
        Model::AxisType axisType;
        nanoem_rsize_t boneIndex = m_stateControllerPtr->boneIndex();
        if (handlePointerIntersects(logicalScaleCursorPosition, project, bone, boneIndex, axisType)) {
            m_stateControllerPtr->setBoneIndex(boneIndex);
            model->selection()->toggleSelectAndActiveBone(bone, project->isMultipleBoneSelectionEnabled());
        }
    }
}

Vector2SI32
DraggingBoneState::deviceScaleCursorActiveBoneInWindow(const Project *project) NANOEM_DECL_NOEXCEPT
{
    Vector2SI32 deviceScaleCursor(0);
    if (const Model *model = project->activeModel()) {
        if (const model::Bone *bone = model::Bone::cast(model->activeBone())) {
            deviceScaleCursor = project->activeCamera()->toDeviceScreenCoordinateInWindow(bone->worldTransformOrigin());
        }
    }
    return deviceScaleCursor;
}

Vector2SI32
DraggingBoneState::logicalScaleCursorActiveBoneInWindow(const Project *project) NANOEM_DECL_NOEXCEPT
{
    return Vector2(deviceScaleCursorActiveBoneInWindow(project)) * (1.0f / project->windowDevicePixelRatio());
}

bool
DraggingBoneState::handlePointerIntersects(const Vector2SI32 &logicalScaleCursorPosition, const Project *project,
    const nanoem_model_bone_t *&bone, nanoem_rsize_t &boneIndex, Model::AxisType &axisType) NANOEM_DECL_NOEXCEPT
{
    const Vector2SI32 deviceScaleCursorPosition(
        Vector2(logicalScaleCursorPosition) * project->windowDevicePixelRatio());
    bool intersected = false;
    axisType = Model::kAxisTypeMaxEnum;
    switch (project->editingMode()) {
    case Project::kEditingModeSelect: {
        if (const Model *model = project->activeModel()) {
            bone = model->intersectsBone(deviceScaleCursorPosition, boneIndex);
            intersected = bone != nullptr;
        }
        break;
    }
    case Project::kEditingModeMove: {
        axisType = selectTranslationAxisType(deviceScaleCursorPosition, project);
        intersected = axisType != Model::kAxisTypeMaxEnum;
        break;
    }
    case Project::kEditingModeRotate: {
        axisType = selectOrientationAxisType(deviceScaleCursorPosition, project);
        intersected = axisType != Model::kAxisTypeMaxEnum;
        break;
    }
    default:
        break;
    }
    return intersected;
}

Model::AxisType
DraggingBoneState::selectTranslationAxisType(
    const Vector2SI32 &deviceScaleCursorPosition, const Project *project) NANOEM_DECL_NOEXCEPT
{
    const Vector2SI32 center(deviceScaleCursorActiveBoneInWindow(project));
    nanoem_f32_t length = project->deviceScaleCircleRadius() * 10.0f, width = project->deviceScaleCircleRadius(),
                 offset = width * 0.5f;
    const Vector4SI32 rectCenter(center.x - offset, center.y - offset, width, width),
        rectX(center.x - offset, center.y - offset, length, width),
        rectY(center.x - offset, center.y - length - offset, width, length);
    Model::AxisType axisType(Model::kAxisTypeMaxEnum);
    if (Inline::intersectsRectPoint(rectCenter, deviceScaleCursorPosition)) {
        axisType = Model::kAxisCenter;
    }
    else if (Inline::intersectsRectPoint(rectX, deviceScaleCursorPosition)) {
        axisType = Model::kAxisX;
    }
    else if (Inline::intersectsRectPoint(rectY, deviceScaleCursorPosition)) {
        axisType = Model::kAxisY;
    }
    return axisType;
}

Model::AxisType
DraggingBoneState::selectOrientationAxisType(
    const Vector2SI32 &deviceScaleCursorPosition, const Project *project) NANOEM_DECL_NOEXCEPT
{
    const Vector2 center(deviceScaleCursorActiveBoneInWindow(project));
    nanoem_f32_t radius = project->deviceScaleCircleRadius() * 7.5f, width = project->deviceScaleCircleRadius(),
                 woffset = width * 0.5f, distance = glm::distance(Vector2(deviceScaleCursorPosition), center);
    const Vector4SI32 rectCenter(center.x - woffset, center.y - woffset, width, width),
        rectX(center.x - woffset, center.y - radius, width, radius * 2.0f),
        rectZ(center.x - radius, center.y - woffset, radius * 2.0f, width);
    Model::AxisType axisType(Model::kAxisTypeMaxEnum);
    if (distance < radius + woffset && distance > radius - woffset) {
        axisType = Model::kAxisY;
    }
    else if (Inline::intersectsRectPoint(rectCenter, deviceScaleCursorPosition)) {
        axisType = Model::kAxisCenter;
    }
    else if (Inline::intersectsRectPoint(rectX, deviceScaleCursorPosition)) {
        axisType = Model::kAxisX;
    }
    else if (Inline::intersectsRectPoint(rectZ, deviceScaleCursorPosition)) {
        axisType = Model::kAxisZ;
    }
    return axisType;
}

class DraggingCameraState NANOEM_DECL_SEALED : public BaseDraggingObjectState {
public:
    DraggingCameraState(StateController *stateController, BaseApplicationService *application, bool canUpdateAngle);
    ~DraggingCameraState() NANOEM_DECL_NOEXCEPT;

    internal::IDraggingState *createTranslateState(
        int axisIndex, const Vector3SI32 &logicalScaleCursorPosition, Project *project) NANOEM_DECL_OVERRIDE;
    internal::IDraggingState *createOrientateState(
        int axisIndex, const Vector3SI32 &logicalScaleCursorPosition, Project *project) NANOEM_DECL_OVERRIDE;
    internal::IDraggingState *createCameraLookAtState(
        const Vector3SI32 &logicalScaleCursorPosition, Project *project) NANOEM_DECL_OVERRIDE;
    internal::IDraggingState *createCameraZoomState(
        const Vector3SI32 &logicalScaleCursorPosition, Project *project) NANOEM_DECL_OVERRIDE;

    void onPress(const Vector3SI32 &logicalScaleCursorPosition) NANOEM_DECL_OVERRIDE;
};

DraggingCameraState::DraggingCameraState(
    StateController *stateController, BaseApplicationService *application, bool canUpdateAngle)
    : BaseDraggingObjectState(stateController, application, canUpdateAngle)
{
}

DraggingCameraState::~DraggingCameraState() NANOEM_DECL_NOEXCEPT
{
}

internal::IDraggingState *
DraggingCameraState::createTranslateState(
    int axisIndex, const Vector3SI32 &logicalScaleCursorPosition, Project *project)
{
    ICamera *camera = project->globalCamera();
    internal::IDraggingState *state = nanoem_new(internal::AxisAlignedTranslateCameraState(project, camera, logicalScaleCursorPosition, axisIndex));
    setType(IState::kTypeDraggingCameraAxisAlignedTranslateCameraState);
    return state;
}

internal::IDraggingState *
DraggingCameraState::createOrientateState(
    int axisIndex, const Vector3SI32 &logicalScaleCursorPosition, Project *project)
{
    ICamera *camera = project->globalCamera();
    internal::IDraggingState *state =
        nanoem_new(internal::AxisAlignedOrientateCameraState(project, camera, logicalScaleCursorPosition, axisIndex));
    setType(IState::kTypeDraggingCameraAxisAlignedOrientateCameraState);
    return state;
}

internal::IDraggingState *
DraggingCameraState::createCameraLookAtState(const Vector3SI32 &logicalScaleCursorPosition, Project *project)
{
    ICamera *camera = project->globalCamera();
    internal::IDraggingState *state =
        nanoem_new(internal::CameraLookAtState(project, camera, logicalScaleCursorPosition));
    setType(IState::kTypeDraggingCameraLookAtState);
    return state;
}

internal::IDraggingState *
DraggingCameraState::createCameraZoomState(const Vector3SI32 &logicalScaleCursorPosition, Project *project)
{
    ICamera *camera = project->globalCamera();
    internal::IDraggingState *state =
        nanoem_new(internal::CameraZoomState(project, camera, logicalScaleCursorPosition));
    setType(IState::kTypeDraggingCameraZoomState);
    return state;
}

void
DraggingCameraState::onPress(const Vector3SI32 &logicalScaleCursorPosition)
{
    Project::RectangleType rectangleType = Project::kRectangleTypeMaxEnum;
    Project *project = m_stateControllerPtr->currentProject();
    if (project && !project->isPlaying() &&
        project->intersectsTransformHandle(logicalScaleCursorPosition, rectangleType)) {
        setDraggingState(
            createDraggingState(rectangleType, logicalScaleCursorPosition, project), logicalScaleCursorPosition);
    }
}

struct ISelector {
    virtual void begin(const Vector4SI32 &value, const Project *project) = 0;
    virtual void update(const Vector4SI32 &value, const Project *project) = 0;
    virtual void end(const Vector4SI32 &value, const Project *project) = 0;
    virtual void draw(IPrimitive2D *primitive, nanoem_f32_t devicePixelRatio) = 0;
    virtual bool contains(const Vector2SI32 &coord) const NANOEM_DECL_NOEXCEPT = 0;
};

class RectangleSelector NANOEM_DECL_SEALED : public ISelector, private NonCopyable {
public:
    RectangleSelector();
    ~RectangleSelector() NANOEM_DECL_NOEXCEPT;

    void begin(const Vector4SI32 &logicalScaleRect, const Project *project) NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void update(const Vector4SI32 &logicalScaleRect, const Project *project) NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void end(const Vector4SI32 &logicalScaleRect, const Project *project) NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void draw(IPrimitive2D *primitive, nanoem_f32_t devicePixelRatio) NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool contains(const Vector2SI32 &deviceScaleCursorPosition) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void updateRectangle(const Vector4SI32 &logicalScaleRect, const Project *project);

private:
    Vector4SI32 m_logicalScaleRect;
    Vector4SI32 m_deviceScaleRect;
};

RectangleSelector::RectangleSelector()
    : m_logicalScaleRect(0)
    , m_deviceScaleRect(0)
{
}

RectangleSelector::~RectangleSelector() NANOEM_DECL_NOEXCEPT
{
}

void
RectangleSelector::begin(const Vector4SI32 &logicalScaleRect, const Project *project) NANOEM_DECL_NOEXCEPT
{
    updateRectangle(logicalScaleRect, project);
}

void
RectangleSelector::update(const Vector4SI32 &logicalScaleRect, const Project *project) NANOEM_DECL_NOEXCEPT
{
    updateRectangle(logicalScaleRect, project);
}

void
RectangleSelector::end(const Vector4SI32 &logicalScaleRect, const Project *project) NANOEM_DECL_NOEXCEPT
{
    updateRectangle(logicalScaleRect, project);
}

void
RectangleSelector::draw(IPrimitive2D *primitive, nanoem_f32_t devicePixelRatio) NANOEM_DECL_NOEXCEPT
{
    static const Vector4 kOpaqueRed(1, 0, 0, 1);
    static const Vector4 kHalfOpacityRed(1, 0, 0, 0.5f);
    primitive->fillRect(m_deviceScaleRect, kHalfOpacityRed, 0);
    primitive->strokeRect(m_deviceScaleRect, kOpaqueRed, 0, devicePixelRatio);
}

bool
RectangleSelector::contains(const Vector2SI32 &deviceScaleCursorPosition) const NANOEM_DECL_NOEXCEPT
{
    return Inline::intersectsRectPoint(m_deviceScaleRect, deviceScaleCursorPosition);
}

void
RectangleSelector::updateRectangle(const Vector4SI32 &logicalScaleRect, const Project *project)
{
    const Vector4UI16 imageRect(project->logicalScaleUniformedViewportImageRect());
    m_logicalScaleRect = logicalScaleRect - Vector4SI32(imageRect.x, imageRect.y, 0, 0);
    if (logicalScaleRect.z < 0) {
        m_logicalScaleRect.z *= -1;
        m_logicalScaleRect.x -= m_logicalScaleRect.z;
    }
    if (logicalScaleRect.w < 0) {
        m_logicalScaleRect.w *= -1;
        m_logicalScaleRect.y -= m_logicalScaleRect.w;
    }
    m_deviceScaleRect = Vector4(m_logicalScaleRect) * project->windowDevicePixelRatio();
}

#if 0
struct CircleSelector : ISelector {
    CircleSelector()
        : m_radius(0)
    {
    }
    ~CircleSelector() NANOEM_DECL_NOEXCEPT
    {
    }

    void
    begin(const Vector4SI32 &logicalScaleRect, const Project *project) NANOEM_DECL_NOEXCEPT_OVERRIDE
    {
        m_center = Vector2(logicalScaleRect.x, logicalScaleRect.y) * project->windowDevicePixelRatio();
    }
    void
    update(const Vector4SI32 &logicalScaleRect, const Project *project) NANOEM_DECL_NOEXCEPT_OVERRIDE
    {
        m_radius = glm::length(Vector2(logicalScaleRect.z, logicalScaleRect.w) * project->windowDevicePixelRatio());
    }
    void
    end(const Vector4SI32 &logicalScaleRect, const Project *project) NANOEM_DECL_NOEXCEPT_OVERRIDE
    {
        m_radius = glm::length(Vector2(logicalScaleRect.z, logicalScaleRect.w) * project->windowDevicePixelRatio());
    }
    void
    draw(IPrimitive2D *primitive, nanoem_f32_t devicePixelRatio) NANOEM_DECL_NOEXCEPT_OVERRIDE
    {
        static const Vector4 kOpaqueRed(1, 0, 0, 1);
        static const Vector4 kHalfOpacityRed(1, 0, 0, 0.5f);
        const Vector4 rect(m_center.x, m_center.y - glm::abs(m_radius), m_radius, m_radius);
        primitive->fillCircle(rect, kHalfOpacityRed);
        primitive->strokeCircle(rect, kOpaqueRed, devicePixelRatio);
    }
    bool
    contains(const Vector2SI32 &deviceScaleCursorPosition) const NANOEM_DECL_NOEXCEPT_OVERRIDE
    {
        return false;
    }

    Vector2 m_center;
    nanoem_f32_t m_radius;
};
#endif

class BaseSelectionState : public BaseDraggingObjectState {
protected:
    static void assignAxisAlignedBoundingBox(const Vector3 &value, Vector3 &aabbMin, Vector3 &aabbMax);
    static void assignPivotMatrixFromAABB(const Vector3 &aabbMin, const Vector3 &aabbMax, Matrix4x4 &matrix);

    BaseSelectionState(StateController *stateController, BaseApplicationService *application);
    ~BaseSelectionState() NANOEM_DECL_NOEXCEPT;

    virtual const char *name() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void commitSelection(Model *model, const Project *project, bool removeAll) = 0;

    internal::IDraggingState *createTranslateState(
        int axisIndex, const Vector3SI32 &logicalScaleCursorPosition, Project *project) NANOEM_DECL_OVERRIDE;
    internal::IDraggingState *createOrientateState(
        int axisIndex, const Vector3SI32 &logicalScaleCursorPosition, Project *project) NANOEM_DECL_OVERRIDE;
    internal::IDraggingState *createCameraLookAtState(
        const Vector3SI32 &logicalScaleCursorPosition, Project *project) NANOEM_DECL_OVERRIDE;
    internal::IDraggingState *createCameraZoomState(
        const Vector3SI32 &logicalScaleCursorPosition, Project *project) NANOEM_DECL_OVERRIDE;
    Matrix4x4 pivotMatrix(const Model *activeModel) const;

    const ISelector *currentSelector() const NANOEM_DECL_NOEXCEPT;
    ISelector *currentSelector();

    RectangleSelector m_rectangleSelector;

public:
    void onPress(const Vector3SI32 &logicalScaleCursorPosition) NANOEM_DECL_OVERRIDE;
    void onMove(const Vector3SI32 &logicalScaleCursorPosition, const Vector2SI32 & /* delta */) NANOEM_DECL_OVERRIDE;
    void onRelease(const Vector3SI32 &logicalScaleCursorPosition) NANOEM_DECL_OVERRIDE;
    void onDrawPrimitive2D(IPrimitive2D *primitive) NANOEM_DECL_OVERRIDE;
};

void
BaseSelectionState::assignAxisAlignedBoundingBox(const Vector3 &value, Vector3 &aabbMin, Vector3 &aabbMax)
{
    aabbMin = glm::min(aabbMin, value);
    aabbMax = glm::max(aabbMax, value);
}

void
BaseSelectionState::assignPivotMatrixFromAABB(const Vector3 &aabbMin, const Vector3 &aabbMax, Matrix4x4 &matrix)
{
    matrix = glm::translate(Constants::kIdentity, (aabbMax + aabbMin) * 0.5f);
}

BaseSelectionState::BaseSelectionState(StateController *stateController, BaseApplicationService *application)
    : BaseDraggingObjectState(stateController, application, false)
{
}

BaseSelectionState::~BaseSelectionState() NANOEM_DECL_NOEXCEPT
{
}

internal::IDraggingState *
BaseSelectionState::createTranslateState(int axisIndex, const Vector3SI32 &logicalScaleCursorPosition, Project *project)
{
    BX_UNUSED_3(axisIndex, logicalScaleCursorPosition, project)
    return nullptr;
}

internal::IDraggingState *
BaseSelectionState::createOrientateState(int axisIndex, const Vector3SI32 &logicalScaleCursorPosition, Project *project)
{
    BX_UNUSED_3(axisIndex, logicalScaleCursorPosition, project)
    return nullptr;
}

internal::IDraggingState *
BaseSelectionState::createCameraLookAtState(const Vector3SI32 &logicalScaleCursorPosition, Project *project)
{
    BX_UNUSED_2(logicalScaleCursorPosition, project);
    return nullptr;
}

internal::IDraggingState *
BaseSelectionState::createCameraZoomState(const Vector3SI32 &logicalScaleCursorPosition, Project *project)
{
    BX_UNUSED_2(logicalScaleCursorPosition, project);
    return nullptr;
}

Matrix4x4
BaseSelectionState::pivotMatrix(const Model *activeModel) const
{
    typedef tinystl::unordered_map<const nanoem_model_material_t *, nanoem_rsize_t, TinySTLAllocator> MaterialOffsetMap;
    const IModelObjectSelection *selection = activeModel->selection();
    Matrix4x4 matrix(0);
    Vector3 aabbMin(FLT_MAX), aabbMax(-FLT_MAX);
    switch (selection->editingType()) {
    case IModelObjectSelection::kEditingTypeBone: {
        const model::Bone::Set selectedBoneSet(selection->allBoneSet());
        if (!selectedBoneSet.empty()) {
            for (model::Bone::Set::const_iterator it = selectedBoneSet.begin(), end = selectedBoneSet.end(); it != end;
                 ++it) {
                const nanoem_model_bone_t *bonePtr = *it;
                const Vector3 origin(model::Bone::origin(bonePtr));
                assignAxisAlignedBoundingBox(origin, aabbMin, aabbMax);
            }
            assignPivotMatrixFromAABB(aabbMin, aabbMax, matrix);
        }
        break;
    }
    case IModelObjectSelection::kEditingTypeFace: {
        const IModelObjectSelection::VertexIndexSet selectedBoneSet(selection->allVertexIndexSet());
        if (!selectedBoneSet.empty()) {
            nanoem_rsize_t numVertices;
            nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(activeModel->data(), &numVertices);
            for (IModelObjectSelection::VertexIndexSet::const_iterator it = selectedBoneSet.begin(),
                                                                       end = selectedBoneSet.end();
                 it != end; ++it) {
                const nanoem_u32_t vertexIndex = *it;
                const Vector3 origin(glm::make_vec3(nanoemModelVertexGetOrigin(vertices[vertexIndex])));
                assignAxisAlignedBoundingBox(origin, aabbMin, aabbMax);
            }
            assignPivotMatrixFromAABB(aabbMin, aabbMax, matrix);
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
                assignAxisAlignedBoundingBox(origin, aabbMin, aabbMax);
            }
            assignPivotMatrixFromAABB(aabbMin, aabbMax, matrix);
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
            for (model::Material::Set::const_iterator it = selectedMaterialSet.begin(), end = selectedMaterialSet.end();
                 it != end; ++it) {
                const nanoem_model_material_t *materialPtr = *it;
                MaterialOffsetMap::const_iterator it2 = materialOffsets.find(materialPtr);
                if (it2 != materialOffsets.end()) {
                    nanoem_rsize_t offset = it2->second, indices = nanoemModelMaterialGetNumVertexIndices(materialPtr);
                    for (nanoem_rsize_t i = 0; i < indices; i++) {
                        const nanoem_u32_t vertexIndex = vertexIndices[offset + i];
                        const Vector3 origin(glm::make_vec3(nanoemModelVertexGetOrigin(vertices[vertexIndex])));
                        assignAxisAlignedBoundingBox(origin, aabbMin, aabbMax);
                    }
                }
            }
            assignPivotMatrixFromAABB(aabbMin, aabbMax, matrix);
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
                assignAxisAlignedBoundingBox(origin, aabbMin, aabbMax);
            }
            assignPivotMatrixFromAABB(aabbMin, aabbMax, matrix);
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
            for (model::SoftBody::Set::const_iterator it = selectedSoftBodySet.begin(), end = selectedSoftBodySet.end();
                 it != end; ++it) {
                const nanoem_model_soft_body_t *softBodyPtr = *it;
                const nanoem_model_material_t *materialPtr = nanoemModelSoftBodyGetMaterialObject(softBodyPtr);
                MaterialOffsetMap::const_iterator it2 = materialOffsets.find(materialPtr);
                if (it2 != materialOffsets.end()) {
                    nanoem_rsize_t offset = it2->second, indices = nanoemModelMaterialGetNumVertexIndices(materialPtr);
                    for (nanoem_rsize_t i = 0; i < indices; i++) {
                        const nanoem_u32_t vertexIndex = vertexIndices[offset + i];
                        const Vector3 origin(glm::make_vec3(nanoemModelVertexGetOrigin(vertices[vertexIndex])));
                        assignAxisAlignedBoundingBox(origin, aabbMin, aabbMax);
                    }
                }
            }
            assignPivotMatrixFromAABB(aabbMin, aabbMax, matrix);
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
            assignPivotMatrixFromAABB(aabbMin, aabbMax, matrix);
        }
        break;
    }
    default:
        break;
    }
    return matrix;
}

const ISelector *
BaseSelectionState::currentSelector() const NANOEM_DECL_NOEXCEPT
{
    return &m_rectangleSelector;
}

ISelector *
BaseSelectionState::currentSelector()
{
    return &m_rectangleSelector;
}

void
BaseSelectionState::onPress(const Vector3SI32 &logicalScaleCursorPosition)
{
    if (Project *project = m_stateControllerPtr->currentProject()) {
        Project::RectangleType rectangleType;
        if (project->isPlaying()) {
            /* do nothing */
        }
        else if (project->intersectsTransformHandle(logicalScaleCursorPosition, rectangleType)) {
            setDraggingState(
                createDraggingState(rectangleType, logicalScaleCursorPosition, project), logicalScaleCursorPosition);
        }
        else {
            const Vector4SI32 rect(logicalScaleCursorPosition.x, logicalScaleCursorPosition.y, 0, 0);
            currentSelector()->begin(rect, project);
        }
        m_lastLogicalScalePosition = logicalScaleCursorPosition;
    }
}

void
BaseSelectionState::onMove(const Vector3SI32 &logicalScaleCursorPosition, const Vector2SI32 &)
{
    if (Project *project = m_stateControllerPtr->currentProject()) {
        project->setLogicalPixelMovingCursorPosition(logicalScaleCursorPosition);
        const Vector4SI32 rect(
            m_lastLogicalScalePosition, Vector2SI32(logicalScaleCursorPosition) - m_lastLogicalScalePosition);
        currentSelector()->update(rect, project);
    }
}

void
BaseSelectionState::onRelease(const Vector3SI32 &logicalScaleCursorPosition)
{
    if (Project *project = m_stateControllerPtr->currentProject()) {
        bool removeAll =
            EnumUtils::isEnabledT<int>(Project::kCursorModifierTypeShift, logicalScaleCursorPosition.z) ? false : true;
        const Vector4SI32 rect(
            m_lastLogicalScalePosition, Vector2SI32(logicalScaleCursorPosition) - m_lastLogicalScalePosition);
        currentSelector()->end(rect, project);
        if (Model *model = project->activeModel()) {
            commitSelection(model, project, removeAll);
            model->setPivotMatrix(pivotMatrix(model));
        }
        m_lastLogicalScalePosition = Vector2();
    }
}

void
BaseSelectionState::onDrawPrimitive2D(IPrimitive2D *primitive)
{
    if (const Project *project = m_stateControllerPtr->currentProject()) {
        nanoem_f32_t deviceScaleRatio = project->windowDevicePixelRatio();
        currentSelector()->draw(primitive, deviceScaleRatio);
    }
}

class DraggingBoxSelectedBoneState NANOEM_DECL_SEALED : public BaseSelectionState {
public:
    DraggingBoxSelectedBoneState(StateController *stateController, BaseApplicationService *application);
    ~DraggingBoxSelectedBoneState() NANOEM_DECL_NOEXCEPT;

    Type type() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void commitSelection(Model *model, const Project *project, bool removeAll) NANOEM_DECL_OVERRIDE;
};

DraggingBoxSelectedBoneState::DraggingBoxSelectedBoneState(
    StateController *stateController, BaseApplicationService *application)
    : BaseSelectionState(stateController, application)
{
}

DraggingBoxSelectedBoneState::~DraggingBoxSelectedBoneState() NANOEM_DECL_NOEXCEPT
{
}

IState::Type
DraggingBoxSelectedBoneState::type() const NANOEM_DECL_NOEXCEPT
{
    return kTypeDraggingBoxSelectedBoneState;
}

const char *
DraggingBoxSelectedBoneState::name() const NANOEM_DECL_NOEXCEPT
{
    return "nanoem.gui.viewport.box-selection";
}

void
DraggingBoxSelectedBoneState::commitSelection(Model *model, const Project *project, bool removeAll)
{
    nanoem_rsize_t numBones;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(model->data(), &numBones);
    const ICamera *camera = project->activeCamera();
    IModelObjectSelection *selection = model->selection();
    if (removeAll) {
        selection->removeAllBones();
    }
    const ISelector *selector = currentSelector();
    for (nanoem_rsize_t i = 0; i < numBones; i++) {
        const nanoem_model_bone_t *bonePtr = bones[i];
        const model::Bone *bone = model::Bone::cast(bonePtr);
        const Vector3 position(model->worldTransform(bone->worldTransform())[3]);
        const Vector2SI32 coord(camera->toDeviceScreenCoordinateInViewport(position));
        if (selector->contains(coord)) {
            selection->addBone(bonePtr);
        }
    }
}

class DraggingVertexSelectionState NANOEM_DECL_SEALED : public BaseSelectionState {
public:
    DraggingVertexSelectionState(StateController *stateController, BaseApplicationService *application);
    ~DraggingVertexSelectionState() NANOEM_DECL_NOEXCEPT;

    Type type() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void commitSelection(Model *model, const Project *project, bool removeAll) NANOEM_DECL_OVERRIDE;
};

DraggingVertexSelectionState::DraggingVertexSelectionState(
    StateController *stateController, BaseApplicationService *application)
    : BaseSelectionState(stateController, application)
{
}

DraggingVertexSelectionState::~DraggingVertexSelectionState() NANOEM_DECL_NOEXCEPT
{
}

IState::Type
DraggingVertexSelectionState::type() const NANOEM_DECL_NOEXCEPT
{
    return kTypeDraggingVertexSelectionState;
}

const char *
DraggingVertexSelectionState::name() const NANOEM_DECL_NOEXCEPT
{
    return "nanoem.gui.viewport.select.vertex";
}

void
DraggingVertexSelectionState::commitSelection(Model *model, const Project *project, bool removeAll)
{
    nanoem_rsize_t numVertices;
    nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(model->data(), &numVertices);
    const ICamera *camera = project->activeCamera();
    IModelObjectSelection *selection = model->selection();
    if (removeAll) {
        selection->removeAllVertices();
    }
    const ISelector *selector = currentSelector();
    for (nanoem_rsize_t i = 0; i < numVertices; i++) {
        const nanoem_model_vertex_t *vertexPtr = vertices[i];
        const model::Vertex *vertex = model::Vertex::cast(vertexPtr);
        const bx::simd128_t v = vertex->m_simd.m_origin;
        const Vector3 position(bx::simd_x(v), bx::simd_y(v), bx::simd_z(v));
        const Vector2SI32 coord(camera->toDeviceScreenCoordinateInViewport(position));
        if (selector->contains(coord)) {
            selection->addVertex(vertexPtr);
        }
    }
}

class DraggingFaceSelectionState NANOEM_DECL_SEALED : public BaseSelectionState {
public:
    DraggingFaceSelectionState(StateController *stateController, BaseApplicationService *application);
    ~DraggingFaceSelectionState() NANOEM_DECL_NOEXCEPT;

    Type type() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void commitSelection(Model *model, const Project *project, bool removeAll) NANOEM_DECL_OVERRIDE;
};

DraggingFaceSelectionState::DraggingFaceSelectionState(
    StateController *stateController, BaseApplicationService *application)
    : BaseSelectionState(stateController, application)
{
}

DraggingFaceSelectionState::~DraggingFaceSelectionState() NANOEM_DECL_NOEXCEPT
{
}

IState::Type
DraggingFaceSelectionState::type() const NANOEM_DECL_NOEXCEPT
{
    return kTypeDraggingFaceSelectionState;
}

const char *
DraggingFaceSelectionState::name() const NANOEM_DECL_NOEXCEPT
{
    return "nanoem.gui.viewport.select.face";
}

void
DraggingFaceSelectionState::commitSelection(Model *model, const Project *project, bool removeAll)
{
    nanoem_rsize_t numVertices, numMaterials, numVertexIndices;
    nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(model->data(), &numVertices);
    nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(model->data(), &numMaterials);
    const nanoem_u32_t *indices = nanoemModelGetAllVertexIndices(model->data(), &numVertexIndices);
    const ICamera *camera = project->activeCamera();
    IModelObjectSelection *selection = model->selection();
    if (removeAll) {
        selection->removeAllFaces();
    }
    const ISelector *selector = currentSelector();
    for (nanoem_rsize_t i = 0, offset = 0; i < numMaterials; i++) {
        const nanoem_f32_t numVI = nanoemModelMaterialGetNumVertexIndices(materials[i]);
        for (nanoem_rsize_t j = 0; j < numVI; j += 3) {
            const nanoem_rsize_t o = offset + j;
            const nanoem_u32_t i0 = indices[o], i1 = indices[o + 1], i2 = indices[o + 2];
            const nanoem_model_vertex_t *v0 = vertices[i0], *v1 = vertices[i1], *v2 = vertices[i2];
            const Vector3 o0(glm::make_vec3(nanoemModelVertexGetOrigin(v0))),
                o1(glm::make_vec3(nanoemModelVertexGetOrigin(v1))), o2(glm::make_vec3(nanoemModelVertexGetOrigin(v2))),
                baryCenter(o0 + (o1 - o0) * 0.5f + (o2 - o0) * 0.5f);
            const Vector2SI32 coord(camera->toDeviceScreenCoordinateInViewport(baryCenter));
            if (selector->contains(coord)) {
                const Vector3UI32 face(i0, i1, i2);
                selection->addFace(face);
            }
        }
    }
}

class DraggingMaterialSelectionState NANOEM_DECL_SEALED : public BaseSelectionState {
public:
    DraggingMaterialSelectionState(StateController *stateController, BaseApplicationService *application);
    ~DraggingMaterialSelectionState() NANOEM_DECL_NOEXCEPT;

    Type type() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void commitSelection(Model *model, const Project *project, bool removeAll) NANOEM_DECL_OVERRIDE;
};

DraggingMaterialSelectionState::DraggingMaterialSelectionState(
    StateController *stateController, BaseApplicationService *application)
    : BaseSelectionState(stateController, application)
{
}

DraggingMaterialSelectionState::~DraggingMaterialSelectionState() NANOEM_DECL_NOEXCEPT
{
}

IState::Type
DraggingMaterialSelectionState::type() const NANOEM_DECL_NOEXCEPT
{
    return kTypeDraggingMaterialSelectionState;
}

const char *
DraggingMaterialSelectionState::name() const NANOEM_DECL_NOEXCEPT
{
    return "nanoem.gui.viewport.select.material";
}

void
DraggingMaterialSelectionState::commitSelection(Model *model, const Project *project, bool removeAll)
{
    nanoem_rsize_t numVertices, numMaterials, numVertexIndices;
    nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(model->data(), &numVertices);
    nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(model->data(), &numMaterials);
    const nanoem_u32_t *indices = nanoemModelGetAllVertexIndices(model->data(), &numVertexIndices);
    const ICamera *camera = project->activeCamera();
    IModelObjectSelection *selection = model->selection();
    if (removeAll) {
        selection->removeAllMaterials();
    }
    const ISelector *selector = currentSelector();
    for (nanoem_rsize_t i = 0, offset = 0; i < numMaterials; i++) {
        const nanoem_model_material_t *materialPtr = materials[i];
        const nanoem_f32_t numVI = nanoemModelMaterialGetNumVertexIndices(materialPtr);
        Vector3 aabbMin(FLT_MAX), aabbMax(FLT_MIN);
        for (nanoem_rsize_t j = 0; j < numVI; j += 3) {
            const nanoem_rsize_t o = offset + j;
            const nanoem_u32_t i0 = indices[o], i1 = indices[o + 1], i2 = indices[o + 2];
            const nanoem_model_vertex_t *v0 = vertices[i0], *v1 = vertices[i1], *v2 = vertices[i2];
            const Vector3 o0(glm::make_vec3(nanoemModelVertexGetOrigin(v0))),
                o1(glm::make_vec3(nanoemModelVertexGetOrigin(v1))), o2(glm::make_vec3(nanoemModelVertexGetOrigin(v2)));
            aabbMin = glm::min(aabbMin, o0);
            aabbMin = glm::min(aabbMin, o1);
            aabbMin = glm::min(aabbMin, o2);
            aabbMax = glm::max(aabbMax, o0);
            aabbMax = glm::max(aabbMax, o1);
            aabbMax = glm::max(aabbMax, o2);
        }
        const Vector3 baryCenter((aabbMin + aabbMax) * 0.5f);
        const Vector2SI32 coord(camera->toDeviceScreenCoordinateInViewport(baryCenter));
        if (selector->contains(coord)) {
            selection->addMaterial(materialPtr);
        }
    }
}

class DraggingBoneSelectionState NANOEM_DECL_SEALED : public BaseSelectionState {
public:
    DraggingBoneSelectionState(StateController *stateController, BaseApplicationService *application);
    ~DraggingBoneSelectionState() NANOEM_DECL_NOEXCEPT;

    Type type() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void commitSelection(Model *model, const Project *project, bool removeAll) NANOEM_DECL_OVERRIDE;
};

DraggingBoneSelectionState::DraggingBoneSelectionState(
    StateController *stateController, BaseApplicationService *application)
    : BaseSelectionState(stateController, application)
{
}

DraggingBoneSelectionState::~DraggingBoneSelectionState() NANOEM_DECL_NOEXCEPT
{
}

IState::Type
DraggingBoneSelectionState::type() const NANOEM_DECL_NOEXCEPT
{
    return kTypeDraggingBoneSelectionState;
}

const char *
DraggingBoneSelectionState::name() const NANOEM_DECL_NOEXCEPT
{
    return "nanoem.gui.viewport.select.bone";
}

void
DraggingBoneSelectionState::commitSelection(Model *model, const Project *project, bool removeAll)
{
    nanoem_rsize_t numBones;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(model->data(), &numBones);
    const ICamera *camera = project->activeCamera();
    IModelObjectSelection *selection = model->selection();
    if (removeAll) {
        selection->removeAllBones();
    }
    const ISelector *selector = currentSelector();
    for (nanoem_rsize_t i = 0; i < numBones; i++) {
        const nanoem_model_bone_t *bonePtr = bones[i];
        const model::Bone *bone = model::Bone::cast(bonePtr);
        const Vector3 position(model->worldTransform(bone->worldTransform())[3]);
        const Vector2SI32 coord(camera->toDeviceScreenCoordinateInViewport(position));
        if (selector->contains(coord)) {
            selection->addBone(bonePtr);
        }
    }
}

class DraggingRigidBodySelectionState NANOEM_DECL_SEALED : public BaseSelectionState {
public:
    DraggingRigidBodySelectionState(StateController *stateController, BaseApplicationService *application);
    ~DraggingRigidBodySelectionState() NANOEM_DECL_NOEXCEPT;

    Type type() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void commitSelection(Model *model, const Project *project, bool removeAll) NANOEM_DECL_OVERRIDE;
};

DraggingRigidBodySelectionState::DraggingRigidBodySelectionState(
    StateController *stateController, BaseApplicationService *application)
    : BaseSelectionState(stateController, application)
{
}

DraggingRigidBodySelectionState::~DraggingRigidBodySelectionState() NANOEM_DECL_NOEXCEPT
{
}

IState::Type
DraggingRigidBodySelectionState::type() const NANOEM_DECL_NOEXCEPT
{
    return kTypeDraggingRigidBodySelectionState;
}

const char *
DraggingRigidBodySelectionState::name() const NANOEM_DECL_NOEXCEPT
{
    return "nanoem.gui.viewport.select.rigid-body";
}

void
DraggingRigidBodySelectionState::commitSelection(Model *model, const Project *project, bool removeAll)
{
    nanoem_rsize_t numRigidBodies;
    nanoem_model_rigid_body_t *const *bodies = nanoemModelGetAllRigidBodyObjects(model->data(), &numRigidBodies);
    const ICamera *camera = project->activeCamera();
    IModelObjectSelection *selection = model->selection();
    if (removeAll) {
        selection->removeAllRigidBodies();
    }
    Matrix4x4 worldTransform;
    const ISelector *selector = currentSelector();
    for (nanoem_rsize_t i = 0; i < numRigidBodies; i++) {
        const nanoem_model_rigid_body_t *bodyPtr = bodies[i];
        const model::RigidBody *body = model::RigidBody::cast(bodyPtr);
        worldTransform = body->worldTransform();
        const Vector3 position(model->worldTransform(worldTransform)[3]);
        const Vector2SI32 coord(camera->toDeviceScreenCoordinateInViewport(position));
        if (selector->contains(coord)) {
            selection->addRigidBody(bodyPtr);
        }
    }
}

class DraggingJointSelectionState NANOEM_DECL_SEALED : public BaseSelectionState {
public:
    DraggingJointSelectionState(StateController *stateController, BaseApplicationService *application);
    ~DraggingJointSelectionState() NANOEM_DECL_NOEXCEPT;

    Type type() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void commitSelection(Model *model, const Project *project, bool removeAll) NANOEM_DECL_OVERRIDE;
};

DraggingJointSelectionState::DraggingJointSelectionState(
    StateController *stateController, BaseApplicationService *application)
    : BaseSelectionState(stateController, application)
{
}

DraggingJointSelectionState::~DraggingJointSelectionState() NANOEM_DECL_NOEXCEPT
{
}

IState::Type
DraggingJointSelectionState::type() const NANOEM_DECL_NOEXCEPT
{
    return kTypeDraggingJointSelectionState;
}

const char *
DraggingJointSelectionState::name() const NANOEM_DECL_NOEXCEPT
{
    return "nanoem.gui.viewport.select.joint";
}

void
DraggingJointSelectionState::commitSelection(Model *model, const Project *project, bool removeAll)
{
    nanoem_rsize_t numJoints;
    nanoem_model_joint_t *const *joints = nanoemModelGetAllJointObjects(model->data(), &numJoints);
    const ICamera *camera = project->activeCamera();
    IModelObjectSelection *selection = model->selection();
    if (removeAll) {
        selection->removeAllJoints();
    }
    Matrix4x4 worldTransformA, worldTransformB;
    const ISelector *selector = currentSelector();
    for (nanoem_rsize_t i = 0; i < numJoints; i++) {
        const nanoem_model_joint_t *jointPtr = joints[i];
        const model::Joint *joint = model::Joint::cast(jointPtr);
        joint->getWorldTransformA(glm::value_ptr(worldTransformA));
        const Vector3 positionA(model->worldTransform(worldTransformA)[3]);
        const Vector2SI32 coordA(camera->toDeviceScreenCoordinateInViewport(positionA));
        if (selector->contains(coordA)) {
            selection->addJoint(jointPtr);
        }
        const Vector3 positionB(model->worldTransform(worldTransformB)[3]);
        const Vector2SI32 coordB(camera->toDeviceScreenCoordinateInViewport(positionB));
        if (selector->contains(coordB)) {
            selection->addJoint(jointPtr);
        }
    }
}

class DraggingSoftBodySelectionState NANOEM_DECL_SEALED : public BaseSelectionState {
public:
    DraggingSoftBodySelectionState(StateController *stateController, BaseApplicationService *application);
    ~DraggingSoftBodySelectionState() NANOEM_DECL_NOEXCEPT;

    Type type() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void commitSelection(Model *model, const Project *project, bool removeAll) NANOEM_DECL_OVERRIDE;

private:
    typedef tinystl::unordered_map<const nanoem_model_material_t *, Vector3, TinySTLAllocator> MaterialBaryCenterMap;

    void getMaterialMap(Model *model, MaterialBaryCenterMap &baryCenters);
};

DraggingSoftBodySelectionState::DraggingSoftBodySelectionState(
    StateController *stateController, BaseApplicationService *application)
    : BaseSelectionState(stateController, application)
{
}

DraggingSoftBodySelectionState::~DraggingSoftBodySelectionState() NANOEM_DECL_NOEXCEPT
{
}

IState::Type
DraggingSoftBodySelectionState::type() const NANOEM_DECL_NOEXCEPT
{
    return kTypeDraggingSoftBodySelectionState;
}

const char *
DraggingSoftBodySelectionState::name() const NANOEM_DECL_NOEXCEPT
{
    return "nanoem.gui.viewport.select.soft-body";
}

void
DraggingSoftBodySelectionState::commitSelection(Model *model, const Project *project, bool removeAll)
{
    nanoem_rsize_t numSoftBodies;
    nanoem_model_soft_body_t *const *bodies = nanoemModelGetAllSoftBodyObjects(model->data(), &numSoftBodies);
    const ICamera *camera = project->activeCamera();
    IModelObjectSelection *selection = model->selection();
    if (removeAll) {
        selection->removeAllSoftBodies();
    }
    MaterialBaryCenterMap baryCenters;
    getMaterialMap(model, baryCenters);
    const ISelector *selector = currentSelector();
    for (nanoem_rsize_t i = 0; i < numSoftBodies; i++) {
        const nanoem_model_soft_body_t *bodyPtr = bodies[i];
        const nanoem_model_material_t *materialPtr = nanoemModelSoftBodyGetMaterialObject(bodyPtr);
        MaterialBaryCenterMap::const_iterator it = baryCenters.find(materialPtr);
        if (it != baryCenters.end()) {
            const Vector3 position(it->second);
            const Vector2SI32 coord(camera->toDeviceScreenCoordinateInViewport(position));
            if (selector->contains(coord)) {
                selection->addSoftBody(bodyPtr);
            }
        }
    }
}

void
DraggingSoftBodySelectionState::getMaterialMap(
    Model *model, DraggingSoftBodySelectionState::MaterialBaryCenterMap &baryCenters)
{
    nanoem_rsize_t numMaterials, numVertices, numVertexIndices;
    nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(model->data(), &numMaterials);
    nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(model->data(), &numVertices);
    const nanoem_u32_t *indices = nanoemModelGetAllVertexIndices(model->data(), &numVertexIndices);
    for (nanoem_rsize_t i = 0, offset = 0; i < numMaterials; i++) {
        const nanoem_model_material_t *materialPtr = materials[i];
        const nanoem_f32_t numVI = nanoemModelMaterialGetNumVertexIndices(materialPtr);
        Vector3 aabbMin(FLT_MAX), aabbMax(FLT_MIN);
        for (nanoem_rsize_t j = 0; j < numVI; j += 3) {
            const nanoem_rsize_t o = offset + j;
            const nanoem_u32_t i0 = indices[o], i1 = indices[o + 1], i2 = indices[o + 2];
            const nanoem_model_vertex_t *v0 = vertices[i0], *v1 = vertices[i1], *v2 = vertices[i2];
            const Vector3 o0(glm::make_vec3(nanoemModelVertexGetOrigin(v0))),
                o1(glm::make_vec3(nanoemModelVertexGetOrigin(v1))), o2(glm::make_vec3(nanoemModelVertexGetOrigin(v2)));
            aabbMin = glm::min(aabbMin, o0);
            aabbMin = glm::min(aabbMin, o1);
            aabbMin = glm::min(aabbMin, o2);
            aabbMax = glm::max(aabbMax, o0);
            aabbMax = glm::max(aabbMax, o1);
            aabbMax = glm::max(aabbMax, o2);
        }
        const Vector3 baryCenter((aabbMin + aabbMax) * 0.5f);
        baryCenters.insert(tinystl::make_pair(materialPtr, baryCenter));
    }
}

class DraggingMoveCameraLookAtState NANOEM_DECL_SEALED : public BaseState {
public:
    DraggingMoveCameraLookAtState(StateController *stateController, BaseApplicationService *application);
    ~DraggingMoveCameraLookAtState() NANOEM_DECL_NOEXCEPT;

    void onPress(const Vector3SI32 &logicalScaleCursorPosition) NANOEM_DECL_OVERRIDE;
    void onMove(const Vector3SI32 &logicalScaleCursorPosition, const Vector2SI32 & /* delta */) NANOEM_DECL_OVERRIDE;
    void onRelease(const Vector3SI32 &logicalScaleCursorPosition) NANOEM_DECL_OVERRIDE;
    Type type() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

    internal::IDraggingState *m_lastDraggingState;
    nanoem_f32_t m_scaleFactor;
};

DraggingMoveCameraLookAtState::DraggingMoveCameraLookAtState(
    StateController *stateController, BaseApplicationService *application)
    : BaseState(stateController, application)
    , m_lastDraggingState(nullptr)
    , m_scaleFactor(0.0f)
{
}

DraggingMoveCameraLookAtState::~DraggingMoveCameraLookAtState() NANOEM_DECL_NOEXCEPT
{
}

void
DraggingMoveCameraLookAtState::onPress(const Vector3SI32 &logicalScaleCursorPosition)
{
    if (Project *project = m_stateControllerPtr->currentProject()) {
        m_lastDraggingState =
            nanoem_new(internal::CameraLookAtState(project, project->globalCamera(), logicalScaleCursorPosition));
        m_scaleFactor = m_lastDraggingState->scaleFactor();
    }
}

void
DraggingMoveCameraLookAtState::onMove(const Vector3SI32 &logicalScaleCursorPosition, const Vector2SI32 &)
{
    if (m_lastDraggingState) {
        m_lastDraggingState->setScaleFactor(
            m_scaleFactor * BaseDraggingObjectState::scaleFactor(logicalScaleCursorPosition));
        m_lastDraggingState->transform(logicalScaleCursorPosition);
    }
}

void
DraggingMoveCameraLookAtState::onRelease(const Vector3SI32 &logicalScaleCursorPosition)
{
    if (m_lastDraggingState) {
        m_lastDraggingState->commit(logicalScaleCursorPosition);
        nanoem_delete_safe(m_lastDraggingState);
    }
}

IState::Type
DraggingMoveCameraLookAtState::type() const NANOEM_DECL_NOEXCEPT
{
    return kTypeDraggingMoveCameraLookAtState;
}

class BaseCreatingBoneState : public IState {
public:
    void onPress(const Vector3SI32 &logicalCursorPosition) NANOEM_DECL_OVERRIDE;
    void onMove(const Vector3SI32 &logicalCursorPosition, const Vector2SI32 &delta) NANOEM_DECL_OVERRIDE;
    void onRelease(const Vector3SI32 &logicalCursorPosition) NANOEM_DECL_OVERRIDE;
    void onDrawPrimitive2D(IPrimitive2D *primitive) NANOEM_DECL_OVERRIDE;
    bool isGrabbingHandle() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

protected:
    BaseCreatingBoneState(StateController *stateController);
    ~BaseCreatingBoneState() NANOEM_DECL_NOEXCEPT;

private:
    StateController *m_stateControllerPtr;
    const nanoem_model_bone_t *m_sourceBone;
    nanoem_mutable_model_bone_t *m_destinationBone;
};

BaseCreatingBoneState::BaseCreatingBoneState(StateController *stateController)
    : m_stateControllerPtr(stateController)
    , m_sourceBone(nullptr)
    , m_destinationBone(nullptr)
{
}

BaseCreatingBoneState::~BaseCreatingBoneState() NANOEM_DECL_NOEXCEPT
{
    nanoemMutableModelBoneDestroy(m_destinationBone);
    m_destinationBone = nullptr;
    m_sourceBone = nullptr;
}

void
BaseCreatingBoneState::onPress(const Vector3SI32 &logicalCursorPosition)
{
    if (Project *project = m_stateControllerPtr->currentProject()) {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoem_rsize_t boneIndex = 0;
        if (Model *activeModel = project->activeModel()) {
            const Vector2 cursorPosition(Vector2(logicalCursorPosition) * project->windowDevicePixelRatio());
            if (const nanoem_model_bone_t *sourceBonePtr = activeModel->intersectsBone(cursorPosition, boneIndex)) {
                m_destinationBone = nanoemMutableModelBoneCreate(activeModel->data(), &status);
                nanoemMutableModelBoneSetParentBoneObject(m_destinationBone, sourceBonePtr);
                nanoemMutableModelBoneSetTargetBoneObject(m_destinationBone, sourceBonePtr);
                nanoemMutableModelBoneSetOrigin(m_destinationBone, nanoemModelBoneGetOrigin(sourceBonePtr));
                nanoemMutableModelBoneSetRotateable(m_destinationBone, true);
                nanoemMutableModelBoneSetVisible(m_destinationBone, true);
                nanoemMutableModelBoneSetUserHandleable(m_destinationBone, true);
                nanoem_model_bone_t *destinationBonePtr = nanoemMutableModelBoneGetOriginObject(m_destinationBone);
                model::Bone *destinationBone = model::Bone::create();
                destinationBone->bind(destinationBonePtr);
                destinationBone->updateLocalTransform(destinationBonePtr);
                m_sourceBone = sourceBonePtr;
            }
        }
    }
}

void
BaseCreatingBoneState::onMove(const Vector3SI32 &logicalCursorPosition, const Vector2SI32 & /* delta */)
{
    if (Project *project = m_stateControllerPtr->currentProject()) {
        if (const Model *activeModel = project->activeModel()) {
            if (nanoem_model_bone_t *destinationBonePtr = nanoemMutableModelBoneGetOriginObject(m_destinationBone)) {
                const ICamera *camera = project->activeCamera();
                Vector3 cursorPosition;
                camera->castRay(logicalCursorPosition, cursorPosition);
                nanoemMutableModelBoneSetOrigin(m_destinationBone, glm::value_ptr(Vector4(cursorPosition, 1)));
                model::Bone *destinationBone = model::Bone::cast(destinationBonePtr);
                destinationBone->updateLocalTransform(destinationBonePtr);
            }
        }
    }
}

void
BaseCreatingBoneState::onRelease(const Vector3SI32 & /* logicalCursorPosition */)
{
    if (Project *project = m_stateControllerPtr->currentProject()) {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        if (Model *activeModel = project->activeModel()) {
            nanoem_model_bone_t *destinationBonePtr = nanoemMutableModelBoneGetOriginObject(m_destinationBone);
            if (model::Bone *destinationBone = model::Bone::cast(destinationBonePtr)) {
                destinationBone->resetLanguage(
                    destinationBonePtr, project->unicodeStringFactory(), project->castLanguage());
                nanoem_mutable_model_t *mutableModel =
                    nanoemMutableModelCreateAsReference(activeModel->data(), &status);
                nanoemMutableModelInsertBoneObject(mutableModel, m_destinationBone, -1, &status);
                nanoemMutableModelDestroy(mutableModel);
                activeModel->addBone(destinationBonePtr);
            }
        }
    }
}

void
BaseCreatingBoneState::onDrawPrimitive2D(IPrimitive2D *primitive)
{
    if (Project *project = m_stateControllerPtr->currentProject()) {
        if (Model *activeModel = project->activeModel()) {
            if (const nanoem_model_bone_t *destinationBonePtr =
                    nanoemMutableModelBoneGetOriginObject(m_destinationBone)) {
                activeModel->drawBoneConnections(primitive, destinationBonePtr, m_sourceBone, 1.0f);
                activeModel->drawBonePoint(primitive, destinationBonePtr, Vector2(0));
            }
        }
    }
}

bool
BaseCreatingBoneState::isGrabbingHandle() const NANOEM_DECL_NOEXCEPT
{
    return false;
}

class CreatingParentBoneState NANOEM_DECL_SEALED : public BaseCreatingBoneState {
public:
    CreatingParentBoneState(StateController *stateController);
    ~CreatingParentBoneState() NANOEM_DECL_NOEXCEPT;

    Type type() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

CreatingParentBoneState::CreatingParentBoneState(StateController *stateController)
    : BaseCreatingBoneState(stateController)
{
}

CreatingParentBoneState::~CreatingParentBoneState() NANOEM_DECL_NOEXCEPT
{
}

IState::Type
CreatingParentBoneState::type() const NANOEM_DECL_NOEXCEPT
{
    return kTypeCreatingParentBoneState;
}

class CreatingChildBoneState NANOEM_DECL_SEALED : public BaseCreatingBoneState {
public:
    CreatingChildBoneState(StateController *stateController);
    ~CreatingChildBoneState() NANOEM_DECL_NOEXCEPT;

    Type type() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

CreatingChildBoneState::CreatingChildBoneState(StateController *stateController)
    : BaseCreatingBoneState(stateController)
{
}

CreatingChildBoneState::~CreatingChildBoneState() NANOEM_DECL_NOEXCEPT
{
}

IState::Type
CreatingChildBoneState::type() const NANOEM_DECL_NOEXCEPT
{
    return kTypeCreatingChildBoneState;
}

class UndoState NANOEM_DECL_SEALED : public BaseState {
public:
    UndoState(StateController *stateController, BaseApplicationService *application);
    ~UndoState() NANOEM_DECL_NOEXCEPT;

    void onPress(const Vector3SI32 & /* logicalScaleCursorPosition */) NANOEM_DECL_OVERRIDE;
    void onMove(
        const Vector3SI32 & /* logicalScaleCursorPosition */, const Vector2SI32 & /* delta */) NANOEM_DECL_OVERRIDE;
    void onRelease(const Vector3SI32 & /* logicalScaleCursorPosition */) NANOEM_DECL_OVERRIDE;
    Type type() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

UndoState::UndoState(StateController *stateController, BaseApplicationService *application)
    : BaseState(stateController, application)
{
}

UndoState::~UndoState() NANOEM_DECL_NOEXCEPT
{
}

void
UndoState::onPress(const Vector3SI32 &)
{
}

void
UndoState::onMove(const Vector3SI32 &, const Vector2SI32 &)
{
}

void
UndoState::onRelease(const Vector3SI32 &)
{
    if (Project *project = m_stateControllerPtr->currentProject()) {
        project->handleUndoAction();
    }
}

IState::Type
UndoState::type() const NANOEM_DECL_NOEXCEPT
{
    return kTypeUndoState;
}

class RedoState NANOEM_DECL_SEALED : public BaseState {
public:
    RedoState(StateController *stateController, BaseApplicationService *application);
    ~RedoState() NANOEM_DECL_NOEXCEPT;

    void onPress(const Vector3SI32 & /* logicalScaleCursorPosition */) NANOEM_DECL_OVERRIDE;
    void onMove(
        const Vector3SI32 & /* logicalScaleCursorPosition */, const Vector2SI32 & /* delta */) NANOEM_DECL_OVERRIDE;
    void onRelease(const Vector3SI32 & /* logicalScaleCursorPosition */) NANOEM_DECL_OVERRIDE;
    Type type() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

RedoState::RedoState(StateController *stateController, BaseApplicationService *application)
    : BaseState(stateController, application)
{
}

RedoState::~RedoState() NANOEM_DECL_NOEXCEPT
{
}

void
RedoState::onPress(const Vector3SI32 &)
{
}

void
RedoState::onMove(const Vector3SI32 &, const Vector2SI32 &)
{
}

void
RedoState::onRelease(const Vector3SI32 &)
{
    if (Project *project = m_stateControllerPtr->currentProject()) {
        project->handleRedoAction();
    }
}

IState::Type
RedoState::type() const NANOEM_DECL_NOEXCEPT
{
    return kTypeRedoState;
}

class DrawUtil NANOEM_DECL_SEALED : private NonCopyable {
public:
    static Vector2SI32 deviceScaleCursorActiveBoneInViewport(const Project *project);
    static void drawBoneMoveHandle(IPrimitive2D *primitive, Model *model, bool isGrabbingHandle);
    static void drawBoneRotateHandle(IPrimitive2D *primitive, Model *model, bool isGrabbingHandle);
    static void drawActiveBonePoint(IPrimitive2D *primitive, const Vector4 &activeBoneColor, const Project *project);
    static void drawCameraLookAtPoint(IPrimitive2D *primitive, const ICamera *camera, const Project *project);
};

Vector2SI32
DrawUtil::deviceScaleCursorActiveBoneInViewport(const Project *project)
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

void
DrawUtil::drawBoneMoveHandle(IPrimitive2D *primitive, Model *activeModel, bool isGrabbingHandle)
{
    static const Vector3 kRed(1, 0, 0), kGreen(0, 1, 0);
    const Project *project = activeModel->project();
    const Model::AxisType type = activeModel->transformAxisType();
    const Vector2 center(deviceScaleCursorActiveBoneInViewport(project));
    const nanoem_f32_t length = project->deviceScaleCircleRadius() * 10.0f,
                       thickness = project->logicalScaleCircleRadius();
    const nanoem_model_bone_t *activeBone = activeModel->activeBone();
    activeModel->drawBoneConnections(primitive, activeBone);
    primitive->strokeLine(center, Vector2(center.x + length, center.y),
        Vector4(kRed, type == Model::kAxisX || !isGrabbingHandle ? 1.0 : 0.25), thickness);
    primitive->strokeLine(center, Vector2(center.x, center.y - length),
        Vector4(kGreen, type == Model::kAxisY || !isGrabbingHandle ? 1.0 : 0.25), thickness);
}

void
DrawUtil::drawBoneRotateHandle(IPrimitive2D *primitive, Model *activeModel, bool isGrabbingHandle)
{
    static const Vector3 kRed(1, 0, 0), kGreen(0, 1, 0), kBlue(0, 0, 1);
    const Project *project = activeModel->project();
    const Model::AxisType type = activeModel->transformAxisType();
    const Vector2 center(deviceScaleCursorActiveBoneInViewport(project));
    nanoem_f32_t radius = project->deviceScaleCircleRadius() * 7.5f,
                 thickness = project->logicalScaleCircleRadius() * 1.5f;
    const nanoem_f32_t kappa = 0.5522847498307933984022516322796f;
    const nanoem_f32_t x1 = center.x - radius;
    const nanoem_f32_t y1 = center.y - radius;
    const nanoem_f32_t x2 = center.x + radius;
    const nanoem_f32_t y2 = center.y + radius;
    activeModel->drawBoneConnections(primitive, activeModel->activeBone());
    primitive->strokeLine(Vector2(center.x - radius, center.y), Vector2(center.x + radius, center.y),
        Vector4(kBlue, type == Model::kAxisZ || !isGrabbingHandle ? 1.0 : 0.25), thickness);
    primitive->strokeLine(Vector2(center.x, center.y - radius), Vector2(center.x, center.y + radius),
        Vector4(kRed, type == Model::kAxisX || !isGrabbingHandle ? 1.0 : 0.25), thickness);
    const Vector4 green(kGreen, type == Model::kAxisY || !isGrabbingHandle ? 1.0 : 0.25);
    primitive->strokeCurve(Vector2(x1, center.y + 1), Vector2(x1, center.y - radius * kappa),
        Vector2(center.x - radius * kappa, y1), Vector2(center.x + 1, y1), green, thickness);
    primitive->strokeCurve(Vector2(center.x - 1, y1), Vector2(center.x + radius * kappa, y1),
        Vector2(x2, center.y - radius * kappa), Vector2(x2, center.y + 1), green, thickness);
    primitive->strokeCurve(Vector2(x2, center.y - 1), Vector2(x2, center.y + radius * kappa),
        Vector2(center.x + radius * kappa, y2), Vector2(center.x - 1, y2), green, thickness);
    primitive->strokeCurve(Vector2(center.x + 1, y2), Vector2(center.x - radius * kappa, y2),
        Vector2(x1, center.y + radius * kappa), Vector2(x1, center.y - 1), green, thickness);
}

void
DrawUtil::drawActiveBonePoint(IPrimitive2D *primitive, const Vector4 &activeBoneColor, const Project *project)
{
    const Vector2SI32 center(deviceScaleCursorActiveBoneInViewport(project));
    if (center.x != 0 && center.y != 0) {
        const nanoem_f32_t radius = project->deviceScaleCircleRadius(), extent = radius * 2;
        primitive->fillCircle(Vector4(center.x - radius, center.y - radius, extent, extent), activeBoneColor);
    }
}

void
DrawUtil::drawCameraLookAtPoint(IPrimitive2D *primitive, const ICamera *camera, const Project *project)
{
    const Vector2 center(camera->toDeviceScreenCoordinateInViewport(camera->boundLookAt()));
    const nanoem_f32_t circleRadius = project->deviceScaleCircleRadius();
    const nanoem_f32_t innerRadius = circleRadius * 0.65f;
    primitive->strokeCircle(
        Vector4(center.x - circleRadius, center.y - circleRadius, circleRadius * 2, circleRadius * 2),
        Vector4(1, 0, 0, 1), 2.0f);
    primitive->fillCircle(
        Vector4(center.x - innerRadius, center.y - innerRadius, innerRadius * 2, innerRadius * 2), Vector4(1, 0, 0, 1));
}

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
    static const Vector4 kColorRed(1, 0, 0, 1), kColorYellow(1, 1, 0, 1);
    if (Project *project = currentProject()) {
        if (m_state) {
            m_state->onDrawPrimitive2D(primitive);
        }
        const Vector2SI32 deviceScaleCursor(project->deviceScaleMovingCursorPosition());
        if (Model *activeModel = project->activeModel()) {
            Vector4 activeBoneColor(kColorRed);
            if (activeModel->isVisible()) {
                if (EnumUtils::isEnabled(Project::kDrawTypeBoneConnections, flags)) {
                    activeModel->drawAllBoneConnections(primitive, deviceScaleCursor);
                }
                if (EnumUtils::isEnabled(Project::kDrawTypeConstraintConnections, flags)) {
                    activeModel->drawConstraintConnections(primitive, deviceScaleCursor);
                }
                if (EnumUtils::isEnabled(Project::kDrawTypeConstraintHeatmaps, flags)) {
                    activeModel->drawConstraintsHeatMap(primitive);
                }
            }
            if (EnumUtils::isEnabled(Project::kDrawTypeBoneMoveHandle, flags)) {
                activeBoneColor = kColorYellow;
                const bool isGrabbingHandle = BaseState::isDraggingBoneState(m_state) && m_state->isGrabbingHandle();
                DrawUtil::drawBoneMoveHandle(primitive, activeModel, isGrabbingHandle);
            }
            if (EnumUtils::isEnabled(Project::kDrawTypeBoneRotateHandle, flags)) {
                activeBoneColor = kColorYellow;
                const bool isGrabbingHandle = BaseState::isDraggingBoneState(m_state) && m_state->isGrabbingHandle();
                DrawUtil::drawBoneRotateHandle(primitive, activeModel, isGrabbingHandle);
            }
            if (EnumUtils::isEnabled(Project::kDrawTypeActiveBone, flags)) {
                DrawUtil::drawActiveBonePoint(primitive, activeBoneColor, project);
            }
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

const IState *
StateController::state() const NANOEM_DECL_NOEXCEPT
{
    return m_state;
}

IState *
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
StateController::handlePointerScroll(const Vector3SI32 &logicalScaleCursorPosition, const Vector2SI32 &delta)
{
    if (Project *project = currentProject()) {
        if (intersectsViewportLayoutRect(project, logicalScaleCursorPosition) && !project->audioPlayer()->isPlaying() &&
            delta.y != 0) {
            ICamera *camera = project->activeCamera();
            camera->setDistance(
                camera->distance() + delta.y * BaseDraggingObjectState::scaleFactor(logicalScaleCursorPosition));
            camera->update();
            if (project->editingMode() != Project::kEditingModeSelect) {
                const Project::ModelList allModels(project->allModels());
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
StateController::handlePointerPress(const Vector3SI32 &logicalScaleCursorPosition, Project::CursorType type)
{
    if (Project *project = currentProject()) {
        switch (type) {
        case Project::kCursorTypeMouseLeft: {
            if (project->isPrimaryCursorTypeLeft()) {
                setPrimaryDraggingState(project, logicalScaleCursorPosition);
            }
            else {
                setSecondaryDraggingState(project, logicalScaleCursorPosition);
            }
            break;
        }
        case Project::kCursorTypeMouseMiddle: {
            setState(nanoem_new(DraggingMoveCameraLookAtState(this, m_applicationPtr)));
            break;
        }
        case Project::kCursorTypeMouseRight: {
            if (!project->isPrimaryCursorTypeLeft()) {
                setPrimaryDraggingState(project, logicalScaleCursorPosition);
            }
            else {
                setSecondaryDraggingState(project, logicalScaleCursorPosition);
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
            m_state->onPress(logicalScaleCursorPosition);
        }
        project->setLogicalPixelLastCursorPosition(type, logicalScaleCursorPosition, true);
        project->setCursorModifiers(static_cast<nanoem_u32_t>(logicalScaleCursorPosition.z));
    }
}

void
StateController::handlePointerMove(const Vector3SI32 &logicalScaleCursorPosition, const Vector2SI32 &delta)
{
    if (Project *project = currentProject()) {
        if (m_state) {
            m_state->onMove(logicalScaleCursorPosition, delta);
        }
        else {
            if (Model *model = project->activeModel()) {
                Model::AxisType axisType;
                const nanoem_model_bone_t *bonePtr =
                    DraggingBoneState::findHoverBone(logicalScaleCursorPosition, project, axisType);
                model->selection()->setHoveredBone(const_cast<nanoem_model_bone_t *>(bonePtr));
                model->setTransformAxisType(axisType);
            }
            project->setLogicalPixelMovingCursorPosition(logicalScaleCursorPosition);
            project->setCursorModifiers(static_cast<nanoem_u32_t>(logicalScaleCursorPosition.z));
        }
    }
}

void
StateController::handlePointerRelease(const Vector3SI32 &logicalScaleCursorPosition, Project::CursorType type)
{
    if (Project *project = currentProject()) {
        if (m_state) {
            m_state->onRelease(logicalScaleCursorPosition);
            setState(nullptr);
        }
        project->setLogicalPixelLastCursorPosition(type, logicalScaleCursorPosition, false);
        project->setCursorModifiers(static_cast<nanoem_u32_t>(logicalScaleCursorPosition.z));
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
    const Project *project, const Vector2SI32 &logicalScaleCursorPosition) const NANOEM_DECL_NOEXCEPT
{
    const Vector4SI32 rect(project->logicalScaleUniformedViewportLayoutRect());
    bool intersected = Inline::intersectsRectPoint(rect, logicalScaleCursorPosition) && project->isViewportHovered() &&
        !m_applicationPtr->hasModalDialog();
    return intersected;
}

void
StateController::setPrimaryDraggingState(Project *project, const Vector2SI32 &logicalScaleCursorPosition)
{
    if (intersectsViewportLayoutRect(project, logicalScaleCursorPosition)) {
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
                case IModelObjectSelection::kEditingTypeSoftBody:
                    state = nanoem_new(DraggingSoftBodySelectionState(this, m_applicationPtr));
                    break;
                case IModelObjectSelection::kEditingTypeVertex:
                    state = nanoem_new(DraggingVertexSelectionState(this, m_applicationPtr));
                    break;
                case IModelObjectSelection::kEditingTypeInfo:
                case IModelObjectSelection::kEditingTypeLabel:
                case IModelObjectSelection::kEditingTypeMorph:
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
StateController::setSecondaryDraggingState(Project *project, const Vector2SI32 &logicalScaleCursorPosition)
{
    if (intersectsViewportLayoutRect(project, logicalScaleCursorPosition)) {
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

nanoem_f32_t
BaseDraggingObjectState::scaleFactor(const Vector3SI32 &logicalScaleCursorPosition) NANOEM_DECL_NOEXCEPT
{
    nanoem_f32_t scaleFactor;
    if (EnumUtils::isEnabledT<int>(Project::kCursorModifierTypeControl, logicalScaleCursorPosition.z)) {
        scaleFactor = 0.1f;
    }
    else if (EnumUtils::isEnabledT<int>(Project::kCursorModifierTypeShift, logicalScaleCursorPosition.z)) {
        scaleFactor = 10.0f;
    }
    else {
        scaleFactor = 1.0f;
    }
    return scaleFactor;
}

} /* namespace nanoem */
