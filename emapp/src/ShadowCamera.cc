/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/ShadowCamera.h"

#include "emapp/Constants.h"
#include "emapp/ICamera.h"
#include "emapp/IEventPublisher.h"
#include "emapp/ILight.h"
#include "emapp/PixelFormat.h"
#include "emapp/Project.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {

const char *const ShadowCamera::kPassName = "@nanoem/ShadowCamera/Pass";
const char *const ShadowCamera::kColorImageName = "@nanoem/ShadowCamera/ColorImage";
const char *const ShadowCamera::kDepthImageName = "@nanoem/ShadowCamera/DepthImage";
const nanoem_f32_t ShadowCamera::kMaximumDistance = 10000.0f;
const nanoem_f32_t ShadowCamera::kMinimumDistance = 0.0f;
const nanoem_f32_t ShadowCamera::kInitialDistance = 8875.0f;
const int ShadowCamera::kInitialTextureSize = 2048;

ShadowCamera::ShadowCamera(Project *project)
    : m_project(project)
    , m_textureSize(kInitialTextureSize)
    , m_coverageMode(kCoverageModeType1)
    , m_distance(kInitialDistance)
    , m_enabled(false)
    , m_dirty(false)
{
    m_shadowPass = m_fallbackPass = { SG_INVALID_ID };
    m_sampler = { SG_INVALID_ID };
    Inline::clearZeroMemory(m_shadowPassDesc);
    Inline::clearZeroMemory(m_fallbackPassDesc);
    if (Inline::isDebugLabelEnabled()) {
        m_shadowPassDesc.label = kPassName;
    }
}

ShadowCamera::~ShadowCamera() NANOEM_DECL_NOEXCEPT
{
}

void
ShadowCamera::initialize()
{
    SG_PUSH_GROUP("ShadowCamera::initialize");
    sg_image_desc id;
    Inline::clearZeroMemory(id);
    id.width = id.height = 1;
    id.pixel_format = SG_PIXELFORMAT_RGBA8;
    id.render_target = true;
    sg_image &colorImage = m_fallbackPassDesc.color_attachments[0].image;
    if (!sg::is_valid(colorImage)) {
        if (Inline::isDebugLabelEnabled()) {
            id.label = "@nanoem/ShadowCamera/FallbackColorImage";
        }
        colorImage = sg::make_image(&id);
        nanoem_assert(sg::query_image_state(colorImage) == SG_RESOURCESTATE_VALID, "color image must be valid");
        SG_LABEL_IMAGE(colorImage, kColorImageName);
    }
    id.pixel_format = SG_PIXELFORMAT_DEPTH;
    sg_image &depthImage = m_fallbackPassDesc.depth_stencil_attachment.image;
    if (!sg::is_valid(depthImage)) {
        if (Inline::isDebugLabelEnabled()) {
            id.label = "@nanoem/ShadowCamera/FallbackDepthImage";
        }
        depthImage = sg::make_image(&id);
        nanoem_assert(sg::query_image_state(depthImage) == SG_RESOURCESTATE_VALID, "depth image must be valid");
        SG_LABEL_IMAGE(depthImage, kDepthImageName);
    }
    if (!sg::is_valid(m_fallbackPass)) {
        if (Inline::isDebugLabelEnabled()) {
            m_fallbackPassDesc.label = "@nanoem/ShadowCamera/FallbackPass";
        }
        m_fallbackPass = sg::make_pass(&m_fallbackPassDesc);
        nanoem_assert(sg::query_pass_state(m_fallbackPass) == SG_RESOURCESTATE_VALID, "pass must be valid");
    }
    if (!sg::is_valid(m_sampler)) {
        sg_sampler_desc sd;
        Inline::clearZeroMemory(sd);
        sd.mag_filter = sd.min_filter = SG_FILTER_NEAREST;
        if (Inline::isDebugLabelEnabled()) {
            sd.label = "@nanoem/ShadowCamera/FallbackPass";
        }
        m_sampler = sg::make_sampler(&sd);
        nanoem_assert(sg::query_sampler_state(m_sampler) == SG_RESOURCESTATE_VALID, "pass must be valid");
    }
    SG_POP_GROUP();
}

void
ShadowCamera::clear()
{
    sg_pass_action action;
    sg::PassBlock::initializeClearAction(action);
    memcpy(&action.colors[0].clear_value, glm::value_ptr(Vector4(1)), sizeof(action.colors[0].clear_value));
    action.depth.clear_value = 1;
    const PixelFormat format(m_project->findRenderPassPixelFormat(passHandle(), 1));
    m_project->clearRenderPass(m_project->sharedBatchDrawQueue(), passHandle(), action, format);
}

void
ShadowCamera::resize(const Vector2UI16 &size)
{
    if (size != m_textureSize) {
        m_textureSize = glm::max(size, Vector2UI16(256));
        update();
    }
}

void
ShadowCamera::reset() NANOEM_DECL_NOEXCEPT
{
    setDistance(kInitialDistance);
    setCoverageMode(kCoverageModeType1);
}

void
ShadowCamera::update()
{
    if (m_enabled) {
        SG_PUSH_GROUPF("ShadowCamera::update(width=%d, height=%d)", m_textureSize.x, m_textureSize.y);
        PixelFormat format;
        sg_image_desc id;
        Inline::clearZeroMemory(id);
        id.width = m_textureSize.x;
        id.height = m_textureSize.y;
        id.pixel_format = SG_PIXELFORMAT_R32F;
        id.render_target = true;
        format.setColorPixelFormat(SG_PIXELFORMAT_R32F, 0);
        sg_image &colorImage = m_shadowPassDesc.color_attachments[0].image;
        sg::destroy_image(colorImage);
        if (Inline::isDebugLabelEnabled()) {
            id.label = kColorImageName;
        }
        colorImage = sg::make_image(&id);
        nanoem_assert(sg::query_image_state(colorImage) == SG_RESOURCESTATE_VALID, "color image must be valid");
        SG_LABEL_IMAGE(colorImage, kColorImageName);
        id.pixel_format = SG_PIXELFORMAT_DEPTH;
        format.setDepthPixelFormat(SG_PIXELFORMAT_DEPTH);
        sg_image &depthImage = m_shadowPassDesc.depth_stencil_attachment.image;
        sg::destroy_image(depthImage);
        if (Inline::isDebugLabelEnabled()) {
            id.label = kDepthImageName;
        }
        depthImage = sg::make_image(&id);
        nanoem_assert(sg::query_image_state(depthImage) == SG_RESOURCESTATE_VALID, "color image must be valid");
        SG_LABEL_IMAGE(depthImage, kDepthImageName);
        sg::destroy_pass(m_shadowPass);
        format.setNumColorAttachemnts(1);
        format.setNumSamples(1);
        m_shadowPass = m_project->registerRenderPass(m_shadowPassDesc, format);
        m_project->setRenderPassName(m_shadowPass, kPassName);
        SG_POP_GROUP();
    }
}

void
ShadowCamera::destroy() NANOEM_DECL_NOEXCEPT
{
    SG_PUSH_GROUP("ShadowCamera::destroy");
    invalidate();
    sg_image &fallbackColorImage = m_fallbackPassDesc.color_attachments[0].image;
    sg::destroy_image(fallbackColorImage);
    fallbackColorImage = { SG_INVALID_ID };
    sg_image &fallbackDepthImage = m_fallbackPassDesc.depth_stencil_attachment.image;
    sg::destroy_image(fallbackDepthImage);
    fallbackDepthImage = { SG_INVALID_ID };
    sg::destroy_pass(m_fallbackPass);
    m_fallbackPass = { SG_INVALID_ID };
    SG_POP_GROUP();
}

sg_pass
ShadowCamera::passHandle() const NANOEM_DECL_NOEXCEPT
{
    return sg::is_valid(m_shadowPass) ? m_shadowPass : m_fallbackPass;
}

sg_image
ShadowCamera::colorImageHandle() const NANOEM_DECL_NOEXCEPT
{
    return sg::is_valid(m_shadowPass) ? m_shadowPassDesc.color_attachments[0].image
                                      : m_fallbackPassDesc.color_attachments[0].image;
}

sg_image
ShadowCamera::depthImageHandle() const NANOEM_DECL_NOEXCEPT
{
    return sg::is_valid(m_shadowPass) ? m_shadowPassDesc.depth_stencil_attachment.image
                                      : m_fallbackPassDesc.depth_stencil_attachment.image;
}

sg_sampler
ShadowCamera::samplerHandle() const NANOEM_DECL_NOEXCEPT
{
    return m_sampler;
}

Vector2UI16
ShadowCamera::imageSize() const NANOEM_DECL_NOEXCEPT
{
    return m_textureSize;
}

void
ShadowCamera::getViewProjection(Matrix4x4 &view, Matrix4x4 &projection) const NANOEM_DECL_NOEXCEPT
{
    getViewMatrix(view);
    getProjectionMatrix(projection);
}

void
ShadowCamera::getCropMatrix(Matrix4x4 &crop) const NANOEM_DECL_NOEXCEPT
{
    const sg_backend backend = sg::query_backend();
    if (backend == SG_BACKEND_GLCORE33 || backend == SG_BACKEND_GLES3) {
        static const Matrix4x4 &kCropMatrixOpenGL =
            glm::scale(glm::translate(Constants::kIdentity, Vector3(0.5f)), Vector3(0.5f));
        crop = kCropMatrixOpenGL;
    }
    else {
        static const Matrix4x4 &kCropMatrixD3D =
            glm::scale(glm::translate(Constants::kIdentity, Vector3(0.5f)), Vector3(0.5f, -0.5f, 0.5f));
        crop = kCropMatrixD3D;
    }
}

const Project *
ShadowCamera::project() const NANOEM_DECL_NOEXCEPT
{
    return m_project;
}

Project *
ShadowCamera::project() NANOEM_DECL_NOEXCEPT
{
    return m_project;
}

nanoem_f32_t
ShadowCamera::distance() const NANOEM_DECL_NOEXCEPT
{
    return m_distance;
}

void
ShadowCamera::setDistance(nanoem_f32_t value)
{
    if (value != m_distance) {
        m_distance = glm::clamp(value, kMinimumDistance, kMaximumDistance);
        setDirty(true);
        m_project->eventPublisher()->publishSetShadowMapDistanceEvent(value);
    }
}

ShadowCamera::CoverageModeType
ShadowCamera::coverageMode() const NANOEM_DECL_NOEXCEPT
{
    return m_coverageMode;
}

void
ShadowCamera::setCoverageMode(CoverageModeType value)
{
    if (value != m_coverageMode) {
        switch (value) {
        case kCoverageModeTypeNone:
        case kCoverageModeType1:
        case kCoverageModeType2:
            m_coverageMode = value;
            break;
        default:
            m_coverageMode = kCoverageModeTypeNone;
            break;
        }
        setDirty(true);
        m_project->eventPublisher()->publishSetShadowMapModeEvent(value);
    }
}

bool
ShadowCamera::isEnabled() const NANOEM_DECL_NOEXCEPT
{
    return m_enabled;
}

void
ShadowCamera::setEnabled(bool value)
{
    if (value != m_enabled) {
        m_enabled = value;
        value ? update() : invalidate();
        setDirty(true);
        m_project->eventPublisher()->publishToggleShadowMapEnabledEvent(value);
    }
}

bool
ShadowCamera::isDirty() const NANOEM_DECL_NOEXCEPT
{
    return m_dirty;
}

void
ShadowCamera::setDirty(bool value)
{
    m_dirty = value;
}

void
ShadowCamera::getViewMatrix(Matrix4x4 &view) const NANOEM_DECL_NOEXCEPT
{
    const ICamera *camera = m_project->globalCamera();
    const ILight *light = m_project->globalLight();
    const Vector3 cameraDirection(camera->direction());
    const Vector3 lightDirection(glm::normalize(light->direction()));
    const Vector3 rawLightViewX(glm::cross(cameraDirection, lightDirection));
    nanoem_f32_t viewLength = glm::length(rawLightViewX);
    if (glm::abs(viewLength) < Constants::kEpsilon) {
        viewLength = 1.0f;
    }
    const Vector3 lightViewX(glm::normalize(rawLightViewX / viewLength));
    const Vector3 lightViewY(glm::cross(lightDirection, lightViewX));
    const Vector3 lightViewOrigin(camera->position() + light->direction() * Vector3(-50));
    const Matrix3x3 lightViewMatrix3(lightViewX, lightViewY, lightDirection);
    view = glm::translate(Matrix4x4(glm::transpose(lightViewMatrix3)), -lightViewOrigin);
}

void
ShadowCamera::getProjectionMatrix(Matrix4x4 &projection) const NANOEM_DECL_NOEXCEPT
{
    const ICamera *camera = m_project->globalCamera();
    const ILight *light = m_project->globalLight();
    const Vector3 cameraDirection(camera->direction());
    const Vector3 lightDirection(glm::normalize(light->direction()));
    nanoem_f32_t distance = (10000.0f - m_distance) / 100000.0f;
    nanoem_f32_t angle = glm::abs(glm::dot(cameraDirection, lightDirection));
#if 1
    nanoem_f32_t c0, c1, c2;
    const nanoem_f32_t halfDistance = distance * 0.5, distanceX0_15 = distance * 0.15;
    if (coverageMode() == kCoverageModeType2) {
        const nanoem_f32_t distanceX3 = distance * 3;
        projection = Matrix4x4(
            distanceX3, 0, 0, 0, 0, distanceX3, 1.5 * distance, distanceX3, 0, 0, distanceX0_15, 0, 0, -1, 0, 1);
        c0 = 3.0;
        c1 = -4.7;
        c2 = 1.8;
    }
    else {
        const nanoem_f32_t distanceX2 = distance * 2;
        projection =
            Matrix4x4(distanceX2, 0, 0, 0, 0, distanceX2, halfDistance, distance, 0, 0, distanceX0_15, 0, 0, -1, 0, 1);
        c0 = 1.0;
        c1 = -1.3;
        c2 = 0.4;
    }
    if (angle > 0.9) {
        const nanoem_f32_t oneMinusAngle = 1 - angle;
        projection = Matrix4x4(distance, 0, 0, 0, 0, distance, halfDistance * oneMinusAngle, distance * oneMinusAngle,
            0, 0, distanceX0_15, 0, 0, angle - 1, 0, 1);
    }
    else if (angle > 0.8) {
        float t = 10 * (angle - 0.8);
        projection[0][0] = glm::mix(projection[0][0], distance, t);
        projection[1][1] = glm::mix(projection[1][1], distance, t);
        projection[1][3] = distance * (c0 + c1 * t + c2 * t * t);
        projection[1][2] = 0.5 * projection[1][3];
        projection[3][1] = glm::mix(-1.0, -0.1, t);
    }
#else
    if (angle > 0.9f) {
        nanoem_f32_t d2 = distance * (1 - angle);
        nanoem_f32_t d015 = 0.15f * distance;
        projection = Matrix4x4(Vector4(distance, 0, 0, 0), Vector4(0, distance, 0.5f * d2, d2), Vector4(0, 0, d015, 0),
            Vector4(0, angle - 1, 0, 1));
    }
    else {
        Vector3 c;
        nanoem_f32_t d015 = 0.15f * distance;
        switch (coverageMode()) {
        case kCoverageModeType2: {
            nanoem_f32_t d30 = 3.0f * distance;
            nanoem_f32_t d15 = 1.5f * distance;
            projection = Matrix4x4(
                Vector4(d30, 0, 0, 0), Vector4(0, d30, d15, d30), Vector4(0, 0, d015, 0), Vector4(0, -1, 0, 1));
            c = Vector3(3.0f, -4.7f, 1.8f);
            break;
        }
        case kCoverageModeType1:
        default: {
            nanoem_f32_t d20 = 2.0f * distance;
            nanoem_f32_t d05 = 0.5f * distance;
            projection = Matrix4x4(
                Vector4(d20, 0, 0, 0), Vector4(0, d20, d05, distance), Vector4(0, 0, d015, 0), Vector4(0, -1, 0, 1));
            c = Vector3(1.0f, -1.3f, 0.4f);
            break;
        }
        }
        if (angle > 0.8f) {
            nanoem_f32_t t = 10 * (angle - 0.8f);
            projection[0][0] = glm::mix(projection[0][0], distance, t);
            projection[1][1] = glm::mix(projection[1][1], distance, t);
            projection[1][3] = distance * (c.x + c.y * t + c.z * t * t);
            projection[1][2] = 0.5f * projection[3][1];
            projection[3][1] = glm::mix(-1.0f, 0.1f, t);
        }
    }
#endif
}

void
ShadowCamera::invalidate()
{
    sg_image &shadowMapColorImage = m_shadowPassDesc.color_attachments[0].image;
    sg::destroy_image(shadowMapColorImage);
    shadowMapColorImage = { SG_INVALID_ID };
    sg_image &shadowMapDepthImage = m_shadowPassDesc.depth_stencil_attachment.image;
    sg::destroy_image(shadowMapDepthImage);
    shadowMapDepthImage = { SG_INVALID_ID };
    sg::destroy_pass(m_shadowPass);
    m_shadowPass = { SG_INVALID_ID };
}

} /* namespace nanoem */
