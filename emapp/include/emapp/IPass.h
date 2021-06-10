/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_IPASS_H_
#define NANOEM_EMAPP_IPASS_H_

#include "emapp/Forward.h"

struct nanodxm_material_t;
struct nanoem_model_material_t;

namespace nanoem {

class Accessory;
class ICamera;
class IDrawable;
class ILight;
class Model;
class Project;
class ShadowCamera;

class IPass : private NonCopyable {
public:
    struct Buffer {
        Buffer(nanoem_rsize_t numIndices, nanoem_rsize_t offset, bool depthEnabled)
            : m_numIndices(int(glm::min(numIndices, size_t(0x7fffffff))))
            , m_offset(int(glm::min(offset, size_t(0x7fffffff))))
            , m_depthEnabled(depthEnabled)
        {
            m_vertexBuffer = { SG_INVALID_ID };
            m_indexBuffer = { SG_INVALID_ID };
        }
        const int m_numIndices;
        int m_offset;
        sg_buffer m_vertexBuffer;
        sg_buffer m_indexBuffer;
        bool m_depthEnabled;
    };
    virtual ~IPass() NANOEM_DECL_NOEXCEPT
    {
    }
    virtual void setGlobalParameters(const IDrawable *drawable, const Project *project) = 0;
    virtual void setCameraParameters(const ICamera *camera, const Matrix4x4 &world) = 0;
    virtual void setLightParameters(const ILight *light, bool adjustment) = 0;
    virtual void setAllAccessoryParameters(const Accessory *model, const Project *project) = 0;
    virtual void setAllModelParameters(const Model *model, const Project *project) = 0;
    virtual void setMaterialParameters(const nanoem_model_material_t *materialPtr) = 0;
    virtual void setMaterialParameters(const nanodxm_material_t *materialPtr) = 0;
    virtual void setEdgeParameters(const nanoem_model_material_t *materialPtr, nanoem_f32_t edgeSize) = 0;
    virtual void setGroundShadowParameters(const ILight *light, const ICamera *camera, const Matrix4x4 &world) = 0;
    virtual void setShadowMapParameters(const ShadowCamera *shadowCamera, const Matrix4x4 &world) = 0;
    virtual void execute(const IDrawable *drawable, const Buffer &buffer) = 0;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_IPASS_H_ */
