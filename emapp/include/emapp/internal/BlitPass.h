/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_BLITPASS_H_
#define NANOEM_EMAPP_INTERNAL_BLITPASS_H_

#include "emapp/internal/BasePass.h"

namespace nanoem {

class Project;

namespace internal {

class BlitPass NANOEM_DECL_SEALED : public BasePass {
public:
    BlitPass(Project *project, bool flipY);
    ~BlitPass() NANOEM_DECL_NOEXCEPT;

    void destroy() NANOEM_DECL_NOEXCEPT_OVERRIDE;

    void blit(sg::PassBlock::IDrawQueue *drawQueue, const sg::NamedPass &dest, const sg::NamedImage &source,
        const Vector4 &rect, const PixelFormat &format);
    void blit(sg::PassBlock::IDrawQueue *drawQueue, const sg::NamedPass &dest, const sg::NamedImage &source,
        const Vector4 &rect, const PixelFormat &format, const Vector4 &viewport);
    void markAsDirty();

private:
    void draw(sg::PassBlock::IDrawQueue *drawQueue, sg_pipeline pipeline, sg_pass dest, sg_image source);
    void setupVertexBuffer(const Vector4 &rect);
    void setupShaderDescription(sg_shader_desc &desc) NANOEM_DECL_OVERRIDE;
    void setupPipelineDescription(sg_pipeline_desc &desc) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

    sg::QuadVertexUnit m_vertices[4];
    sg_sampler m_sampler;
    bool m_dirty;
};

} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_BLITPASS_H_ */
