/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_ISTATE_H_
#define NANOEM_EMAPP_ISTATE_H_

#include "emapp/Forward.h"

namespace nanoem {

class Error;
class IPrimitive2D;

class IState {
public:
    enum DrawType {
        kDrawTypeActiveBone = 0x1,
        kDrawTypeBoneConnections = 0x2,
        kDrawTypeConstraintConnections = 0x4,
        kDrawTypeConstraintHeatmaps = 0x8,
        kDrawTypeBoneTooltip = 0x10,
        kDrawTypeBoneMoveHandle = 0x20,
        kDrawTypeBoneRotateHandle = 0x40,
        kDrawTypeBoneLocalAxes = 0x80,
        kDrawTypeCameraLookAt = 0x100,
        kDrawTypeVertexWeightPainter = 0x200,
        kDrawTypeMaxEnum = 0x400,
    };
    enum Type {
        kTypeFirstEnum,
        kTypeUndoState = kTypeFirstEnum,
        kTypeRedoState,
        kTypeDraggingBoneTranslateActiveBoneState,
        kTypeDraggingBoneOrientateActiveBoneState,
        kTypeDraggingBoneAxisAlignedTranslateActiveBoneState,
        kTypeDraggingBoneAxisAlignedOrientateActiveBoneState,
        kTypeDraggingBoneDirectionalOrientateActiveBoneState,
        kTypeDraggingCameraLookAtState,
        kTypeDraggingCameraZoomState,
        kTypeDraggingCameraAxisAlignedTranslateCameraState,
        kTypeDraggingCameraAxisAlignedOrientateCameraState,
        kTypeDraggingMoveCameraLookAtState,
        kTypeDraggingBoxSelectedBoneState,
        kTypeDraggingBoneSelectionState,
        kTypeDraggingFaceSelectionState,
        kTypeDraggingJointSelectionState,
        kTypeDraggingMaterialSelectionState,
        kTypeDraggingRigidBodySelectionState,
        kTypeDraggingSoftBodySelectionState,
        kTypeDraggingVertexSelectionState,
        kTypeCreatingParentBoneState,
        kTypeCreatingTargetBoneState,
        kTypePaintVertexWeightState,
        kTypeMaxEnum,
    };
    virtual ~IState() NANOEM_DECL_NOEXCEPT
    {
    }
    virtual void onPress(const Vector3SI32 &logicalScaleCursorPosition, Error &error) = 0;
    virtual void onMove(const Vector3SI32 &logicalScaleCursorPosition, const Vector2SI32 &delta) = 0;
    virtual void onRelease(const Vector3SI32 &logicalScaleCursorPosition, Error &error) = 0;
    virtual void onDrawPrimitive2D(IPrimitive2D *primitive) = 0;
    virtual Type type() const NANOEM_DECL_NOEXCEPT = 0;
    virtual bool isGrabbingHandle() const NANOEM_DECL_NOEXCEPT = 0;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_ISTATE_H_ */
