/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_IEFFECT_H_
#define NANOEM_EMAPP_IEFFECT_H_

#include "emapp/IPass.h"
#include "emapp/URI.h"

namespace nanoem {

class ITechnique;

class IEffect {
public:
    enum ScriptClassType {
        kScriptClassTypeFirstEnum,
        kScriptClassTypeObject = kScriptClassTypeFirstEnum,
        kScriptClassTypeScene,
        kScriptClassTypeSceneObject,
        kScriptClassTypeMaxEnum
    };
    enum ScriptOrderType {
        kScriptOrderTypeFirstEnum,
        kScriptOrderTypeDependsOnScriptExternal = kScriptOrderTypeFirstEnum,
        kScriptOrderTypePreProcess,
        kScriptOrderTypeStandard,
        kScriptOrderTypePostProcess,
        kScriptOrderTypeMaxEnum
    };
    struct ImageResourceParameter {
        ImageResourceParameter(
            const String &name, const URI &fileURI, const String &filename, const sg_image_desc &desc, bool shared)
            : m_fileURI(fileURI)
            , m_name(name)
            , m_filename(filename)
            , m_desc(desc)
            , m_shared(shared)
        {
        }
        ImageResourceParameter(const ImageResourceParameter &parameter)
            : m_fileURI(parameter.m_fileURI)
            , m_name(parameter.m_name)
            , m_filename(parameter.m_filename)
            , m_desc(parameter.m_desc)
            , m_shared(parameter.m_shared)
        {
        }
        ~ImageResourceParameter()
        {
        }
        const URI m_fileURI;
        const String m_name;
        const String m_filename;
        sg_image_desc m_desc;
        bool m_shared;
    };

    typedef tinystl::vector<ImageResourceParameter, TinySTLAllocator> ImageResourceList;

    virtual ~IEffect() NANOEM_DECL_NOEXCEPT
    {
    }
    virtual ITechnique *findTechnique(const String &passType, const nanodxm_material_t *material,
        nanoem_rsize_t materialIndex, nanoem_rsize_t numMaterials, Accessory *accessory) = 0;
    virtual ITechnique *findTechnique(const String &passType, const nanoem_model_material_t *material,
        nanoem_rsize_t materialIndex, nanoem_rsize_t numMaterials, Model *model) = 0;
    virtual void createImageResource(const void *ptr, size_t size, const ImageResourceParameter &parameter) = 0;
    virtual void setAllParameterObjects(
        const nanoem_motion_effect_parameter_t *const *parameters, nanoem_rsize_t numParameters) = 0;
    virtual void setAllParameterObjects(const nanoem_motion_effect_parameter_t *const *fromParameters,
        nanoem_rsize_t numFromParameters, const nanoem_motion_effect_parameter_t *const *toParameters,
        nanoem_rsize_t numToParameters, nanoem_f32_t coefficient) = 0;
    virtual ImageResourceList allImageResources() const = 0;
    virtual ScriptClassType scriptClass() const NANOEM_DECL_NOEXCEPT = 0;
    virtual ScriptOrderType scriptOrder() const NANOEM_DECL_NOEXCEPT = 0;
    virtual bool hasScriptExternal() const NANOEM_DECL_NOEXCEPT = 0;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_IEFFECT_H_ */
