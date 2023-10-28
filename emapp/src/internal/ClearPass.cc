/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/ClearPass.h"

#include "emapp/Project.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace internal {
namespace {
#include "emapp/private/shaders/clear_fs_glsl_core33.h"
#include "emapp/private/shaders/clear_fs_glsl_es3.h"
#include "emapp/private/shaders/clear_fs_msl_macos.h"
#include "emapp/private/shaders/clear_fs_spirv.h"
#include "emapp/private/shaders/clear_fs_wgsl.h"
#include "emapp/private/shaders/clear_ps_dxbc.h"
#include "emapp/private/shaders/clear_vs_dxbc.h"
#include "emapp/private/shaders/clear_vs_glsl_core33.h"
#include "emapp/private/shaders/clear_vs_glsl_es3.h"
#include "emapp/private/shaders/clear_vs_msl_macos.h"
#include "emapp/private/shaders/clear_vs_spirv.h"
#include "emapp/private/shaders/clear_vs_wgsl.h"
} /* namespace anonymous */

ClearPass::ClearPass(Project *project)
    : BasePass(project)
{
}

ClearPass::~ClearPass() NANOEM_DECL_NOEXCEPT
{
}

void
ClearPass::clear(
    sg::PassBlock::IDrawQueue *drawQueue, sg_pass pass, const sg_pass_action &action, const PixelFormat &format)
{
    setupVertexBuffer();
    sg_pipeline pipeline = { SG_INVALID_ID };
    setupPipeline(format, pipeline);
    draw(drawQueue, pipeline, pass, action);
}

void
ClearPass::draw(sg::PassBlock::IDrawQueue *drawQueue, sg_pipeline pipeline, sg_pass pass, const sg_pass_action &action)
{
    SG_INSERT_MARKERF("internal::ClearPass::draw(id=%d, name=%s)", pass.id, m_project->findRenderPassName(pass));
    sg::PassBlock pb(drawQueue, pass, action);
    pb.applyPipelineBindings(pipeline, m_bindings);
    pb.draw(0, 4);
}

void
ClearPass::setupVertexBuffer()
{
    sg_buffer &vb = m_bindings.vertex_buffers[0];
    if (!sg::is_valid(vb)) {
        sg_buffer_desc vbd;
        Inline::clearZeroMemory(vbd);
        sg::QuadVertexUnit vertices[4];
        sg::QuadVertexUnit::generateQuadTriStrip(vertices);
        vbd.size = sizeof(vertices);
        vbd.data.ptr = vertices;
        vbd.data.size = vbd.size;
        if (Inline::isDebugLabelEnabled()) {
            vbd.label = "@nanoem/ClearPass/Vertices";
        }
        vb = sg::make_buffer(&vbd);
        nanoem_assert(sg::query_buffer_state(vb) == SG_RESOURCESTATE_VALID, "vertex buffer must be valid");
        SG_LABEL_BUFFER(vb, vbd.label);
    }
}

void
ClearPass::setupShaderDescription(sg_shader_desc &desc)
{
    const sg_backend backend = sg::query_backend();
    if (backend == SG_BACKEND_D3D11) {
        desc.fs.bytecode.ptr = g_nanoem_clear_ps_dxbc_data;
        desc.fs.bytecode.size = g_nanoem_clear_ps_dxbc_size;
        desc.vs.bytecode.ptr = g_nanoem_clear_vs_dxbc_data;
        desc.vs.bytecode.size = g_nanoem_clear_vs_dxbc_size;
    }
    else if (sg::is_backend_metal(backend)) {
        desc.vs.bytecode.ptr = g_nanoem_clear_vs_msl_macos_data;
        desc.vs.bytecode.size = g_nanoem_clear_vs_msl_macos_size;
        desc.fs.bytecode.ptr = g_nanoem_clear_fs_msl_macos_data;
        desc.fs.bytecode.size = g_nanoem_clear_fs_msl_macos_size;
    }
    else if (backend == SG_BACKEND_WGPU) {
        desc.fs.source = reinterpret_cast<const char *>(g_nanoem_clear_fs_wgsl_data);
        desc.vs.source = reinterpret_cast<const char *>(g_nanoem_clear_vs_wgsl_data);
    }
    else if (backend == SG_BACKEND_GLCORE33) {
        desc.fs.source = reinterpret_cast<const char *>(g_nanoem_clear_fs_glsl_core33_data);
        desc.vs.source = reinterpret_cast<const char *>(g_nanoem_clear_vs_glsl_core33_data);
    }
    else if (backend == SG_BACKEND_GLES3) {
        desc.fs.source = reinterpret_cast<const char *>(g_nanoem_clear_fs_glsl_es3_data);
        desc.vs.source = reinterpret_cast<const char *>(g_nanoem_clear_vs_glsl_es3_data);
    }
    desc.vs.entry = "nanoemVSMain";
    desc.fs.entry = "nanoemPSMain";
    desc.attrs[0] = sg_shader_attr_desc { "a_position", "SV_POSITION", 0 };
}

void
ClearPass::setupPipelineDescription(sg_pipeline_desc &desc)
{
    desc.primitive_type = SG_PRIMITIVETYPE_TRIANGLE_STRIP;
    desc.cull_mode = SG_CULLMODE_BACK;
    desc.colors[0].write_mask = SG_COLORMASK_NONE;
    sg_vertex_layout_state &ld = desc.layout;
    ld.buffers[0].stride = sizeof(sg::QuadVertexUnit);
    ld.attrs[0] = sg_vertex_attr_state { 0, offsetof(sg::QuadVertexUnit, m_position), SG_VERTEXFORMAT_FLOAT2 };
}

const char *
ClearPass::name() const NANOEM_DECL_NOEXCEPT
{
    return "ClearPass";
}

} /* namespace internal */
} /* namespace nanoem */
