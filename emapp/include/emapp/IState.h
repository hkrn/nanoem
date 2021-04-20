/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_ISTATE_H_
#define NANOEM_EMAPP_ISTATE_H_

#include "emapp/Forward.h"

namespace nanoem {

class IState {
public:
    enum Type {
        kTypeFirstEnum,
        kTypeUndoState = kTypeFirstEnum,
        kTypeRedoState,
        kTypeDraggingBoneTranslateActiveBoneState,
        kTypeDraggingBoneOrientateActiveBoneState,
        kTypeDraggingBoneAxisAlignedTranslateActiveBoneState,
        kTypeDraggingBoneAxisAlignedOrientateActiveBoneState,
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
        kTypeCreatingChildBoneState,
        kTypeMaxEnum,
    };
    virtual ~IState() NANOEM_DECL_NOEXCEPT
    {
    }
    virtual void onPress(const Vector3SI32 &logicalScaleCursorPosition) = 0;
    virtual void onMove(const Vector3SI32 &logicalScaleCursorPosition, const Vector2SI32 &delta) = 0;
    virtual void onRelease(const Vector3SI32 &logicalScaleCursorPosition) = 0;
    virtual void onDrawPrimitive2D(IPrimitive2D *primitive) = 0;
    virtual Type type() const NANOEM_DECL_NOEXCEPT = 0;
    virtual bool isGrabbingHandle() const NANOEM_DECL_NOEXCEPT = 0;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_ISTATE_H_ */