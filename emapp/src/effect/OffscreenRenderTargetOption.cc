/*
  Copyright (c) 2015-2021 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#include "emapp/effect/OffscreenRenderTargetOption.h"

#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace effect {

OffscreenRenderTargetOption::OffscreenRenderTargetOption(
    const String &name, const String &description, const Vector4 &clearColor, nanoem_f32_t clearDepth)
    : m_name(name)
    , m_description(description)
    , m_clearColor(clearColor)
    , m_clearDepth(clearDepth)
    , m_sharedImageReferenceCount(0)
{
    Inline::clearZeroMemory(m_colorImageDescription);
    Inline::clearZeroMemory(m_depthStencilImageDescription);
    m_colorImage = m_depthStencilImage = { SG_INVALID_ID };
}

OffscreenRenderTargetOption::~OffscreenRenderTargetOption() NANOEM_DECL_NOEXCEPT
{
}

void
OffscreenRenderTargetOption::getPassAction(sg_pass_action &pa) const NANOEM_DECL_NOEXCEPT
{
    Inline::clearZeroMemory(pa);
    pa.colors[0].action = pa.depth.action = SG_ACTION_CLEAR;
    memcpy(&pa.colors[0].value, glm::value_ptr(m_clearColor), sizeof(pa.colors[0].value));
    pa.depth.value = m_clearDepth;
}

void
OffscreenRenderTargetOption::getPassDescription(sg_pass_desc &pd) const NANOEM_DECL_NOEXCEPT
{
    Inline::clearZeroMemory(pd);
    pd.color_attachments[0].image = m_colorImage;
    pd.depth_stencil_attachment.image = m_depthStencilImage;
}

} /* namespace effect */
} /* namespace nanoem */
