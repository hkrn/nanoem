/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/LineDrawer.h"

#include "emapp/ICamera.h"
#include "emapp/PixelFormat.h"
#include "emapp/Project.h"
#include "emapp/StringUtils.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace internal {
namespace {
#include "emapp/private/shaders/grid_fs_glsl_core33.h"
#include "emapp/private/shaders/grid_fs_glsl_es3.h"
#include "emapp/private/shaders/grid_fs_msl_macos.h"
#include "emapp/private/shaders/grid_ps_dxbc.h"
#include "emapp/private/shaders/grid_vs_dxbc.h"
#include "emapp/private/shaders/grid_vs_glsl_core33.h"
#include "emapp/private/shaders/grid_vs_glsl_es3.h"
#include "emapp/private/shaders/grid_vs_msl_macos.h"
#include "emapp/private/shaders/pointed_grid_vs_msl_macos.h"

struct Uniform {
    Matrix4x4 m_viewProjection;
    Vector4 m_color;
};

} /* namespace anonymous */

LineDrawer::Option::Option(sg_buffer vertexBuffer, size_t numIndices)
    : m_drawQueue(nullptr)
    , m_vertexBuffer(vertexBuffer)
    , m_primitiveType(SG_PRIMITIVETYPE_LINES)
    , m_indexType(SG_INDEXTYPE_NONE)
    , m_offset(0)
    , m_numIndices(numIndices)
    , m_worldTransform(1)
    , m_color(1.0f)
    , m_enableBlendMode(false)
    , m_enableDepthTest(false)
{
    m_pass = { SG_INVALID_ID };
    m_indexBuffer = { SG_INVALID_ID };
}

LineDrawer::LineDrawer(Project *project)
    : m_project(project)
{
    Inline::clearZeroMemory(m_bindings);
    m_shader = { SG_INVALID_ID };
    m_pointedShader = { SG_INVALID_ID };
}

LineDrawer::~LineDrawer() NANOEM_DECL_NOEXCEPT
{
}

void
LineDrawer::initialize()
{
    sg_shader_desc sd;
    Inline::clearZeroMemory(sd);
    sd.vs.uniform_blocks[0].size = sizeof(Uniform);
#if defined(NANOEM_ENABLE_SHADER_OPTIMIZED)
    sd.vs.uniform_blocks[0].uniforms[0] = sg_shader_uniform_desc { "_30", SG_UNIFORMTYPE_FLOAT4, 5 };
#else
    sd.vs.uniform_blocks[0].uniforms[0] = sg_shader_uniform_desc { "grid_parameters_t", SG_UNIFORMTYPE_FLOAT4, 5 };
#endif /* NANOEM_ENABLE_SHADER_OPTIMIZED */
    const sg_backend backend = sg::query_backend();
    if (backend == SG_BACKEND_D3D11) {
        sd.fs.bytecode.ptr = g_nanoem_grid_ps_dxbc_data;
        sd.fs.bytecode.size = g_nanoem_grid_ps_dxbc_size;
        sd.vs.bytecode.ptr = g_nanoem_grid_vs_dxbc_data;
        sd.vs.bytecode.size = g_nanoem_grid_vs_dxbc_size;
    }
    else if (sg::is_backend_metal(backend)) {
        sd.fs.bytecode.ptr = g_nanoem_grid_fs_msl_macos_data;
        sd.fs.bytecode.size = g_nanoem_grid_fs_msl_macos_size;
        sd.vs.bytecode.ptr = g_nanoem_grid_vs_msl_macos_data;
        sd.vs.bytecode.size = g_nanoem_grid_vs_msl_macos_size;
    }
    else if (backend == SG_BACKEND_GLCORE33) {
        sd.fs.source = reinterpret_cast<const char *>(g_nanoem_grid_fs_glsl_core33_data);
        sd.vs.source = reinterpret_cast<const char *>(g_nanoem_grid_vs_glsl_core33_data);
    }
    else if (backend == SG_BACKEND_GLES3) {
        sd.fs.source = reinterpret_cast<const char *>(g_nanoem_grid_fs_glsl_es3_data);
        sd.vs.source = reinterpret_cast<const char *>(g_nanoem_grid_vs_glsl_es3_data);
    }
    sd.vs.entry = "nanoemVSMain";
    sd.fs.entry = "nanoemPSMain";
    sd.attrs[0] = sg_shader_attr_desc { "a_position", "SV_POSITION", 0 };
    sd.attrs[1] = sg_shader_attr_desc { "a_normal", "NORMAL", 0 };
    sd.attrs[2] = sg_shader_attr_desc { "a_texcoord0", "TEXCOORD", 0 };
    sd.attrs[3] = sg_shader_attr_desc { "a_texcoord1", "TEXCOORD", 1 };
    sd.attrs[4] = sg_shader_attr_desc { "a_texcoord2", "TEXCOORD", 2 };
    sd.attrs[5] = sg_shader_attr_desc { "a_texcoord3", "TEXCOORD", 3 };
    sd.attrs[6] = sg_shader_attr_desc { "a_texcoord4", "TEXCOORD", 4 };
    sd.attrs[7] = sg_shader_attr_desc { "a_color0", "COLOR", 0 };
    if (Inline::isDebugLabelEnabled()) {
        sd.label = "@nanoem/LineDrawer";
    }
    m_shader = sg::make_shader(&sd);
    nanoem_assert(sg::query_shader_state(m_shader) == SG_RESOURCESTATE_VALID, "shader must be valid");
    SG_LABEL_SHADER(m_shader, sd.label);
    if (sg::is_backend_metal(backend)) {
        sd.vs.bytecode.ptr = g_nanoem_pointed_grid_vs_msl_macos_data;
        sd.vs.bytecode.size = g_nanoem_pointed_grid_vs_msl_macos_size;
        m_pointedShader = sg::make_shader(&sd);
        nanoem_assert(sg::query_shader_state(m_pointedShader) == SG_RESOURCESTATE_VALID, "shader must be valid");
        SG_LABEL_SHADER(m_pointedShader, sd.label);
    }
    else {
        m_pointedShader = m_shader;
    }
}

void
LineDrawer::draw(sg::PassBlock &pb, const Option &option)
{
    const PixelFormat &format = m_project->currentRenderPassPixelFormat();
    bx::HashMurmur2A hash;
    hash.begin(0);
    format.addHash(hash);
    hash.add(option.m_primitiveType);
    hash.add(option.m_indexType);
    sg_pipeline pipeline;
    nanoem_u32_t key = hash.end();
    PipelineMap::const_iterator it = m_pipelines.find(key);
    if (it != m_pipelines.end()) {
        pipeline = it->second;
    }
    else {
        sg_pipeline_desc pd;
        Inline::clearZeroMemory(pd);
        pd.shader = option.m_primitiveType == SG_PRIMITIVETYPE_POINTS ? m_pointedShader : m_shader;
        pd.colors[0].pixel_format = format.colorPixelFormat(0);
        pd.depth.pixel_format = format.depthPixelFormat();
        pd.color_count = format.numColorAttachments();
        if (option.m_enableBlendMode) {
            Project::setAlphaBlendMode(pd.colors[0]);
        }
        if (option.m_enableDepthTest) {
            pd.depth.write_enabled = true;
            pd.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
        }
        pd.sample_count = format.numSamples();
        sg_layout_desc &ld = pd.layout;
        ld.buffers[0].stride = sizeof(sg::LineVertexUnit);
        ld.attrs[0] = sg_vertex_attr_desc { 0, offsetof(sg::LineVertexUnit, m_position), SG_VERTEXFORMAT_FLOAT3 };
        ld.attrs[1] = sg_vertex_attr_desc { 0, offsetof(sg::LineVertexUnit, m_position), SG_VERTEXFORMAT_FLOAT3 };
        ld.attrs[2] = sg_vertex_attr_desc { 0, offsetof(sg::LineVertexUnit, m_position), SG_VERTEXFORMAT_FLOAT2 };
        ld.attrs[3] = sg_vertex_attr_desc { 0, offsetof(sg::LineVertexUnit, m_position), SG_VERTEXFORMAT_FLOAT2 };
        ld.attrs[4] = sg_vertex_attr_desc { 0, offsetof(sg::LineVertexUnit, m_position), SG_VERTEXFORMAT_FLOAT2 };
        ld.attrs[5] = sg_vertex_attr_desc { 0, offsetof(sg::LineVertexUnit, m_position), SG_VERTEXFORMAT_FLOAT2 };
        ld.attrs[6] = sg_vertex_attr_desc { 0, offsetof(sg::LineVertexUnit, m_position), SG_VERTEXFORMAT_FLOAT2 };
        ld.attrs[7] = sg_vertex_attr_desc { 0, offsetof(sg::LineVertexUnit, m_color), SG_VERTEXFORMAT_UBYTE4N };
        pd.primitive_type = option.m_primitiveType;
        pd.index_type = option.m_indexType;
        char label[Inline::kMarkerStringLength];
        if (Inline::isDebugLabelEnabled()) {
            StringUtils::format(
                label, sizeof(label), "@nanoem/LineDrawer/%d", Inline::saturateInt32(m_pipelines.size()));
            pd.label = label;
        }
        else {
            *label = 0;
        }
        pipeline = sg::make_pipeline(&pd);
        nanoem_assert(sg::query_pipeline_state(pipeline) == SG_RESOURCESTATE_VALID, "pipeline must be valid");
        m_project->setRenderPipelineName(pipeline, label);
        m_pipelines.insert(tinystl::make_pair(key, pipeline));
    }
    m_bindings.vertex_buffers[0] = option.m_vertexBuffer;
    m_bindings.index_buffer = option.m_indexBuffer;
    pb.applyPipelineBindings(pipeline, m_bindings);
    Matrix4x4 view, projection;
    Uniform uniform;
    m_project->activeCamera()->getViewTransform(view, projection);
    uniform.m_viewProjection = projection * view * option.m_worldTransform;
    uniform.m_color = option.m_color;
    pb.applyUniformBlock(SG_SHADERSTAGE_VS, &uniform, sizeof(uniform));
    pb.draw(Inline::saturateInt32(option.m_offset), Inline::saturateInt32(option.m_numIndices));
}

void
LineDrawer::drawPass(const Option &option)
{
    sg_pass_action action;
    Inline::clearZeroMemory(action);
    action.colors[0].action = SG_ACTION_LOAD;
    action.depth.action = action.stencil.action = SG_ACTION_LOAD;
    const sg_pass basePass = sg::is_valid(option.m_pass) ? option.m_pass : m_project->currentRenderPass(),
                  pass = m_project->beginRenderPass(basePass);
    sg::PassBlock::IDrawQueue *drawQueue = option.m_drawQueue ? option.m_drawQueue : m_project->sharedBatchDrawQueue();
    sg::PassBlock pb(drawQueue, pass, action);
    draw(pb, option);
}

void
LineDrawer::destroy() NANOEM_DECL_NOEXCEPT
{
    for (PipelineMap::const_iterator it = m_pipelines.begin(), end = m_pipelines.end(); it != end; ++it) {
        sg::destroy_pipeline(it->second);
    }
    m_pipelines.clear();
    if (m_pointedShader.id != m_shader.id) {
        sg::destroy_shader(m_pointedShader);
        m_pointedShader = { SG_INVALID_ID };
    }
    sg::destroy_shader(m_shader);
    m_shader = { SG_INVALID_ID };
}

} /* namespace internal */
} /* namespace nanoem */
