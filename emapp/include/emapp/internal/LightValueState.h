/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#ifndef NANOEM_EMAPP_INTERNAL_LIGHTVALUESTATE
#define NANOEM_EMAPP_INTERNAL_LIGHTVALUESTATE

#include "emapp/IVectorValueState.h"

namespace nanoem {

class ILight;
class Project;

namespace internal {

class BaseLightVectorValueState : public IVectorValueState, private NonCopyable {
public:
    BaseLightVectorValueState(Project *project, ILight *light);
    ~BaseLightVectorValueState();

protected:
    Project *m_project;
    ILight *m_light;
};

class LightColorVectorValueState NANOEM_DECL_SEALED : public BaseLightVectorValueState {
public:
    LightColorVectorValueState(Project *project, ILight *light);
    ~LightColorVectorValueState();

    nanoem_rsize_t numComponents() const NANOEM_DECL_OVERRIDE;
    const nanoem_f32_t *initialValue() const NANOEM_DECL_OVERRIDE;
    const nanoem_f32_t *currentValue() const NANOEM_DECL_OVERRIDE;
    void setValue(const nanoem_f32_t *value) NANOEM_DECL_OVERRIDE;
    void commit() NANOEM_DECL_OVERRIDE;
    void rollback() NANOEM_DECL_OVERRIDE;

private:
    Vector3 m_currentColor;
    Vector3 m_initialColor;
};

class LightDirectionVectorValueState NANOEM_DECL_SEALED : public BaseLightVectorValueState {
public:
    LightDirectionVectorValueState(Project *project, ILight *light);
    ~LightDirectionVectorValueState();

    nanoem_rsize_t numComponents() const NANOEM_DECL_OVERRIDE;
    const nanoem_f32_t *initialValue() const NANOEM_DECL_OVERRIDE;
    const nanoem_f32_t *currentValue() const NANOEM_DECL_OVERRIDE;
    void setValue(const nanoem_f32_t *value) NANOEM_DECL_OVERRIDE;
    void commit() NANOEM_DECL_OVERRIDE;
    void rollback() NANOEM_DECL_OVERRIDE;

private:
    Vector3 m_currentDirection;
    Vector3 m_initialDirection;
};

} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_LIGHTVALUESTATE */
