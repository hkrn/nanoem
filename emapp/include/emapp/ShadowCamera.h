/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_SHADOWCAMERA_H_
#define NANOEM_EMAPP_SHADOWCAMERA_H_

#include "Forward.h"

namespace nanoem {

class Project;

class ShadowCamera NANOEM_DECL_SEALED : private NonCopyable {
public:
    static const char *const kPassName;
    static const char *const kColorImageName;
    static const char *const kDepthImageName;
    static const nanoem_f32_t kMaximumDistance;
    static const nanoem_f32_t kMinimumDistance;
    static const nanoem_f32_t kInitialDistance;
    static const int kInitialTextureSize;
    enum CoverageModeType {
        kCoverageModeTypeFirstEnum,
        kCoverageModeTypeNone = kCoverageModeTypeFirstEnum,
        kCoverageModeType1,
        kCoverageModeType2,
        kCoverageModeTypeMaxEnum
    };

    ShadowCamera(Project *m_project);
    ~ShadowCamera() NANOEM_DECL_NOEXCEPT;

    void initialize();
    void clear();
    void resize(const Vector2UI16 &size);
    void reset() NANOEM_DECL_NOEXCEPT;
    void update();
    void destroy() NANOEM_DECL_NOEXCEPT;

    sg_pass passHandle() const NANOEM_DECL_NOEXCEPT;
    sg_image colorImageHandle() const NANOEM_DECL_NOEXCEPT;
    sg_image depthImageHandle() const NANOEM_DECL_NOEXCEPT;
    sg_sampler samplerHandle() const NANOEM_DECL_NOEXCEPT;
    Vector2UI16 imageSize() const NANOEM_DECL_NOEXCEPT;
    void getViewProjection(Matrix4x4 &view, Matrix4x4 &projection) const NANOEM_DECL_NOEXCEPT;
    void getCropMatrix(Matrix4x4 &crop) const NANOEM_DECL_NOEXCEPT;

    const Project *project() const NANOEM_DECL_NOEXCEPT;
    Project *project() NANOEM_DECL_NOEXCEPT;
    nanoem_f32_t distance() const NANOEM_DECL_NOEXCEPT;
    void setDistance(nanoem_f32_t value);
    CoverageModeType coverageMode() const NANOEM_DECL_NOEXCEPT;
    void setCoverageMode(CoverageModeType value);
    bool isEnabled() const NANOEM_DECL_NOEXCEPT;
    void setEnabled(bool value);
    bool isDirty() const NANOEM_DECL_NOEXCEPT;
    void setDirty(bool value);

private:
    void getViewMatrix(Matrix4x4 &view) const NANOEM_DECL_NOEXCEPT;
    void getProjectionMatrix(Matrix4x4 &projection) const NANOEM_DECL_NOEXCEPT;
    void invalidate();

    Project *m_project;
    sg_pass m_shadowPass;
    sg_pass_desc m_shadowPassDesc;
    sg_pass m_fallbackPass;
    sg_pass_desc m_fallbackPassDesc;
    sg_sampler m_sampler;
    Vector2UI16 m_textureSize;
    CoverageModeType m_coverageMode;
    nanoem_f32_t m_distance;
    bool m_enabled;
    bool m_dirty;
};
}

#endif /* NANOEM_EMAPP_SHADOWCAMERA_H_ */
