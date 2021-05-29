/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_MODEL_IGIZMO_H_
#define NANOEM_EMAPP_MODEL_IGIZMO_H_

#include "emapp/Forward.h"

namespace nanoem {
namespace model {

class IGizmo : private NonCopyable {
public:
    enum OperationType {
        kOperationTypeFirstEnum,
        kOperationTypeTranslate = kOperationTypeFirstEnum,
        kOperationTypeRotate,
        kOperationTypeScale,
        kOperationTypeMaxEnum
    };
    enum TransformCoordinateType {
        kTransformCoordinateTypeFirstEnum,
        kTransformCoordinateTypeGlobal = kTransformCoordinateTypeFirstEnum,
        kTransformCoordinateTypeLocal,
        kTransformCoordinateTypeMaxEnum
    };
    virtual ~IGizmo() NANOEM_DECL_NOEXCEPT
    {
    }

    virtual Matrix4x4 pivotMatrix() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setPivotMatrix(const Matrix4x4 &value) = 0;
    virtual TransformCoordinateType transformCoordinateType() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setTransformCoordinateType(TransformCoordinateType value) = 0;
    virtual OperationType operationType() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setOperationType(OperationType value) = 0;
};

} /* namespace model */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_MODEL_IGIZMO_H_ */
