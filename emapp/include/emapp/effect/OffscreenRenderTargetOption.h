/*
  Copyright (c) 2015-2023 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#pragma once
#ifndef NANOEM_EMAPP_EFFECT_OFFSCREENRENDERTARGETOPTION_H_
#define NANOEM_EMAPP_EFFECT_OFFSCREENRENDERTARGETOPTION_H_

#include "emapp/Forward.h"

namespace nanoem {
namespace effect {

struct OffscreenRenderTargetOption {
    OffscreenRenderTargetOption(
        const String &name, const String &description, const Vector4 &clearColor, nanoem_f32_t clearDepth);
    ~OffscreenRenderTargetOption() NANOEM_DECL_NOEXCEPT;

    void getClearPassAction(sg_pass_action &pa) const NANOEM_DECL_NOEXCEPT;
    void getPassDescription(sg_pass_desc &pd) const NANOEM_DECL_NOEXCEPT;

    const String m_name;
    const String m_description;
    const Vector4 m_clearColor;
    const nanoem_f32_t m_clearDepth;
    sg_image m_colorImage;
    sg_image m_depthStencilImage;
    sg_image m_resolveImage;
    sg_image_desc m_colorImageDescription;
    sg_image_desc m_depthStencilImageDescription;
    StringPairList m_conditions;
    int m_sharedImageReferenceCount;
};
typedef tinystl::vector<OffscreenRenderTargetOption, TinySTLAllocator> OffscreenRenderTargetOptionList;
typedef tinystl::unordered_map<String, OffscreenRenderTargetOption, TinySTLAllocator> OffscreenRenderTargetOptionMap;

} /* namespace effect */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_EFFECT_OFFSCREENRENDERTARGETOPTION_H_ */
