/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#ifndef NANOEM_EMAPP_INTERNAL_CAMERAVALUESTATE
#define NANOEM_EMAPP_INTERNAL_CAMERAVALUESTATE

#include "emapp/IVectorValueState.h"

namespace nanoem {

class ICamera;
class Project;

namespace internal {

class BaseCameraVectorValueState : public IVectorValueState, private NonCopyable {
public:
    BaseCameraVectorValueState(Project *project, ICamera *camera);
    ~BaseCameraVectorValueState();

    const Project *project() const NANOEM_DECL_NOEXCEPT;
    const ICamera *camera() const NANOEM_DECL_NOEXCEPT;

protected:
    void update();

    Project *m_project;
    ICamera *m_camera;
};

class CameraLookAtVectorValueState NANOEM_DECL_SEALED : public BaseCameraVectorValueState {
public:
    CameraLookAtVectorValueState(Project *project, ICamera *camera);
    ~CameraLookAtVectorValueState();

    nanoem_rsize_t numComponents() const NANOEM_DECL_OVERRIDE;
    const nanoem_f32_t *initialValue() const NANOEM_DECL_OVERRIDE;
    const nanoem_f32_t *currentValue() const NANOEM_DECL_OVERRIDE;
    void setValue(const nanoem_f32_t *value) NANOEM_DECL_OVERRIDE;
    void commit() NANOEM_DECL_OVERRIDE;
    void rollback() NANOEM_DECL_OVERRIDE;

private:
    const Vector3 m_initialLookAt;
    Vector3 m_currentLookAt;
};

class CameraAngleVectorValueState NANOEM_DECL_SEALED : public BaseCameraVectorValueState {
public:
    CameraAngleVectorValueState(Project *project, ICamera *camera);
    ~CameraAngleVectorValueState();

    nanoem_rsize_t numComponents() const NANOEM_DECL_OVERRIDE;
    const nanoem_f32_t *initialValue() const NANOEM_DECL_OVERRIDE;
    const nanoem_f32_t *currentValue() const NANOEM_DECL_OVERRIDE;
    void setValue(const nanoem_f32_t *value) NANOEM_DECL_OVERRIDE;
    void commit() NANOEM_DECL_OVERRIDE;
    void rollback() NANOEM_DECL_OVERRIDE;

private:
    const Vector3 m_initialAngle;
    Vector3 m_currentAngle;
};

class CameraFovVectorValueState NANOEM_DECL_SEALED : public BaseCameraVectorValueState {
public:
    CameraFovVectorValueState(Project *project, ICamera *camera);
    ~CameraFovVectorValueState();

    nanoem_rsize_t numComponents() const NANOEM_DECL_OVERRIDE;
    const nanoem_f32_t *initialValue() const NANOEM_DECL_OVERRIDE;
    const nanoem_f32_t *currentValue() const NANOEM_DECL_OVERRIDE;
    void setValue(const nanoem_f32_t *value) NANOEM_DECL_OVERRIDE;
    void commit() NANOEM_DECL_OVERRIDE;
    void rollback() NANOEM_DECL_OVERRIDE;

private:
    const nanoem_f32_t m_initialFov;
    nanoem_f32_t m_currentFov;
};

class CameraDistanceVectorValueState NANOEM_DECL_SEALED : public BaseCameraVectorValueState {
public:
    CameraDistanceVectorValueState(Project *project, ICamera *camera);
    ~CameraDistanceVectorValueState();

    nanoem_rsize_t numComponents() const NANOEM_DECL_OVERRIDE;
    const nanoem_f32_t *initialValue() const NANOEM_DECL_OVERRIDE;
    const nanoem_f32_t *currentValue() const NANOEM_DECL_OVERRIDE;
    void setValue(const nanoem_f32_t *value) NANOEM_DECL_OVERRIDE;
    void commit() NANOEM_DECL_OVERRIDE;
    void rollback() NANOEM_DECL_OVERRIDE;

private:
    const nanoem_f32_t m_initialDistance;
    nanoem_f32_t m_currentDistance;
};

} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_CAMERAVALUESTATE */
