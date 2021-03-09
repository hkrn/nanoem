/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#ifndef NANOEM_EMAPP_INTERNAL_ACCESSORYVALUESTATE
#define NANOEM_EMAPP_INTERNAL_ACCESSORYVALUESTATE

#include "emapp/IVectorValueState.h"

namespace nanoem {

class Accessory;

namespace internal {

class BaseAccessoryVectorValueState : public IVectorValueState, private NonCopyable {
public:
    BaseAccessoryVectorValueState(Accessory *accessory);
    ~BaseAccessoryVectorValueState();

protected:
    Accessory *m_accessory;
};

class AccessoryTranslationVectorValueState NANOEM_DECL_SEALED : public BaseAccessoryVectorValueState {
public:
    AccessoryTranslationVectorValueState(Accessory *accessory);
    ~AccessoryTranslationVectorValueState();

    nanoem_rsize_t numComponents() const NANOEM_DECL_OVERRIDE;
    const nanoem_f32_t *initialValue() const NANOEM_DECL_OVERRIDE;
    const nanoem_f32_t *currentValue() const NANOEM_DECL_OVERRIDE;
    void setValue(const nanoem_f32_t *value) NANOEM_DECL_OVERRIDE;
    void commit() NANOEM_DECL_OVERRIDE;
    void rollback() NANOEM_DECL_OVERRIDE;

private:
    const Vector3 m_initiallTranslation;
    Vector3 m_currentTranslation;
};

class AccessoryOrientationVectorValueState NANOEM_DECL_SEALED : public BaseAccessoryVectorValueState {
public:
    AccessoryOrientationVectorValueState(Accessory *accessory);
    ~AccessoryOrientationVectorValueState();

    nanoem_rsize_t numComponents() const NANOEM_DECL_OVERRIDE;
    const nanoem_f32_t *initialValue() const NANOEM_DECL_OVERRIDE;
    const nanoem_f32_t *currentValue() const NANOEM_DECL_OVERRIDE;
    void setValue(const nanoem_f32_t *value) NANOEM_DECL_OVERRIDE;
    void commit() NANOEM_DECL_OVERRIDE;
    void rollback() NANOEM_DECL_OVERRIDE;

private:
    const Vector3 m_initialOrientation;
    Vector3 m_currentOrientation;
};

class AccessoryOpacityValueState NANOEM_DECL_SEALED : public BaseAccessoryVectorValueState {
public:
    AccessoryOpacityValueState(Accessory *accessory);
    ~AccessoryOpacityValueState();

    nanoem_rsize_t numComponents() const NANOEM_DECL_OVERRIDE;
    const nanoem_f32_t *initialValue() const NANOEM_DECL_OVERRIDE;
    const nanoem_f32_t *currentValue() const NANOEM_DECL_OVERRIDE;
    void setValue(const nanoem_f32_t *value) NANOEM_DECL_OVERRIDE;
    void commit() NANOEM_DECL_OVERRIDE;
    void rollback() NANOEM_DECL_OVERRIDE;

private:
    const nanoem_f32_t m_initialOpacity;
    nanoem_f32_t m_currentOpacity;
};

class AccessoryScaleFactorValueState NANOEM_DECL_SEALED : public BaseAccessoryVectorValueState {
public:
    AccessoryScaleFactorValueState(Accessory *accessory);
    ~AccessoryScaleFactorValueState();

    nanoem_rsize_t numComponents() const NANOEM_DECL_OVERRIDE;
    const nanoem_f32_t *initialValue() const NANOEM_DECL_OVERRIDE;
    const nanoem_f32_t *currentValue() const NANOEM_DECL_OVERRIDE;
    void setValue(const nanoem_f32_t *value) NANOEM_DECL_OVERRIDE;
    void commit() NANOEM_DECL_OVERRIDE;
    void rollback() NANOEM_DECL_OVERRIDE;

private:
    const nanoem_f32_t m_initialScaleFactor;
    nanoem_f32_t m_currentScaleFactor;
};

} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_ACCESSORYVALUESTATE */
