/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_DEBUGDRAWER_H_
#define NANOEM_EMAPP_INTERNAL_DEBUGDRAWER_H_

#include "debug-draw/debug_draw.hpp"
#include "emapp/Forward.h"

namespace nanoem {

class Project;

namespace internal {

class DebugDrawer NANOEM_DECL_SEALED : public dd::RenderInterface, private NonCopyable {
public:
    DebugDrawer(Project *project);
    ~DebugDrawer() NANOEM_DECL_NOEXCEPT_OVERRIDE;

    void beginDraw() NANOEM_DECL_OVERRIDE;
    void endDraw() NANOEM_DECL_OVERRIDE;

    dd::GlyphTextureHandle createGlyphTexture(int width, int height, const void *pixels) NANOEM_DECL_OVERRIDE;
    void destroyGlyphTexture(dd::GlyphTextureHandle glyphTex) NANOEM_DECL_OVERRIDE;
    void drawPointList(const dd::DrawVertex *points, int count, bool depthEnabled) NANOEM_DECL_OVERRIDE;
    void drawLineList(const dd::DrawVertex *lines, int count, bool depthEnabled) NANOEM_DECL_OVERRIDE;
    void drawGlyphList(const dd::DrawVertex *glyphs, int count, dd::GlyphTextureHandle glyphTex) NANOEM_DECL_OVERRIDE;

private:
    typedef tinystl::vector<sg_buffer, TinySTLAllocator> SGBufferList;
    static void destroyAllPipelines(PipelineMap &values) NANOEM_DECL_NOEXCEPT;
    static void destroyBuffer(sg_buffer &buffer) NANOEM_DECL_NOEXCEPT;

    nanoem_u32_t hash(bool depthEnabled) const;
    sg_buffer createBuffer(const dd::DrawVertex *vertices, int count);
    void setupPointPipeline(bool depthEnabled, sg_pipeline &pieplineRef);
    void setupLinePipeline(bool depthEnabled, sg_pipeline &pieplineRef);
    void setupGlyphPipeline(sg_pipeline &pieplineRef);
    void initializeCommonPipelineDescription(sg_pipeline_desc &desc);
    void setupShader();
    void draw(sg_pipeline pipeline, sg_buffer buffer, int count);

    Project *m_project;
    SGBufferList m_buffers;
    sg_bindings m_bindings;
    sg_shader m_shader;
    sg_shader m_pointedShader;
    PipelineMap m_points;
    PipelineMap m_lines;
    PipelineMap m_glyphs;
};

} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_BLITPASS_H_ */
