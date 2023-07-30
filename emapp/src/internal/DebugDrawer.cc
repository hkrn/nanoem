/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/DebugDrawer.h"

#include "emapp/ICamera.h"
#include "emapp/Project.h"
#include "emapp/StringUtils.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace internal {
namespace {
#include "emapp/private/shaders/debug_fs_glsl_core33.h"
#include "emapp/private/shaders/debug_fs_glsl_es3.h"
#include "emapp/private/shaders/debug_fs_msl_macos.h"
#include "emapp/private/shaders/debug_fs_spirv.h"
#include "emapp/private/shaders/debug_ps_dxbc.h"
#include "emapp/private/shaders/debug_vs_dxbc.h"
#include "emapp/private/shaders/debug_vs_glsl_core33.h"
#include "emapp/private/shaders/debug_vs_glsl_es3.h"
#include "emapp/private/shaders/debug_vs_msl_macos.h"
#include "emapp/private/shaders/debug_vs_spirv.h"
#include "emapp/private/shaders/pointed_debug_vs_msl_macos.h"
static const char *const kLabelPrefix = "@nanoem/DebugDrawer";
} /* namespace anonymous */

DebugDrawer::DebugDrawer(Project *project)
    : m_project(project)
{
    Inline::clearZeroMemory(m_bindings);
    m_shader = { SG_INVALID_ID };
    m_pointedShader = { SG_INVALID_ID };
}

DebugDrawer::~DebugDrawer() NANOEM_DECL_NOEXCEPT
{
    destroyAllPipelines(m_points);
    destroyAllPipelines(m_lines);
    destroyAllPipelines(m_glyphs);
    if (m_pointedShader.id != m_shader.id) {
        sg::destroy_shader(m_pointedShader);
        m_pointedShader = { SG_INVALID_ID };
    }
    sg::destroy_shader(m_shader);
    m_shader = { SG_INVALID_ID };
}

void
DebugDrawer::beginDraw()
{
    SG_PUSH_GROUP("internal::DebugDrawer::beginDraw");
    if (!m_buffers.empty()) {
        for (SGBufferList::const_iterator it = m_buffers.begin(), end = m_buffers.end(); it != end; ++it) {
            sg::destroy_buffer(*it);
        }
        m_buffers.clear();
    }
}

void
DebugDrawer::endDraw()
{
    SG_POP_GROUP();
}

dd::GlyphTextureHandle
DebugDrawer::createGlyphTexture(int width, int height, const void *pixels)
{
    sg_image_desc desc;
    Inline::clearZeroMemory(desc);
    desc.usage = SG_USAGE_IMMUTABLE;
    desc.width = width;
    desc.height = height;
    desc.pixel_format = SG_PIXELFORMAT_R8;
    desc.data.subimage[0][0].ptr = pixels;
    desc.data.subimage[0][0].size = width * height;
    char label[Inline::kMarkerStringLength];
    if (Inline::isDebugLabelEnabled()) {
        StringUtils::format(label, sizeof(label), "%s/Glyph", kLabelPrefix);
        desc.label = label;
    }
    sg_image image = sg::make_image(&desc);
    nanoem_assert(sg::query_image_state(image) == SG_RESOURCESTATE_VALID, "image must be valid");
    SG_LABEL_IMAGE(image, label);
    return reinterpret_cast<dd::GlyphTextureHandle>(image.id);
}

void
DebugDrawer::destroyGlyphTexture(dd::GlyphTextureHandle glyphTex)
{
    sg_image image = { static_cast<nanoem_u32_t>(reinterpret_cast<intptr_t>(glyphTex)) };
    sg::destroy_image(image);
}

void
DebugDrawer::drawPointList(const dd::DrawVertex *points, int count, bool depthEnabled)
{
    sg_pipeline pipeline;
    setupLinePipeline(depthEnabled, pipeline);
    sg_buffer buffer = createBuffer(points, count);
    draw(pipeline, buffer, count);
    m_buffers.push_back(buffer);
}

void
DebugDrawer::drawLineList(const dd::DrawVertex *lines, int count, bool depthEnabled)
{
    sg_pipeline pipeline;
    setupLinePipeline(depthEnabled, pipeline);
    sg_buffer buffer = createBuffer(lines, count);
    draw(pipeline, buffer, count);
    m_buffers.push_back(buffer);
}

void
DebugDrawer::drawGlyphList(const dd::DrawVertex *glyphs, int count, dd::GlyphTextureHandle glyphTex)
{
    sg_pipeline pipeline;
    m_bindings.fs.images[0].id = static_cast<nanoem_u32_t>(reinterpret_cast<intptr_t>(glyphTex));
    setupGlyphPipeline(pipeline);
    sg_buffer buffer = createBuffer(glyphs, count);
    draw(pipeline, buffer, count);
    m_buffers.push_back(buffer);
}

void
DebugDrawer::destroyAllPipelines(PipelineMap &values) NANOEM_DECL_NOEXCEPT
{
    for (PipelineMap::const_iterator it = values.begin(), end = values.end(); it != end; ++it) {
        sg::destroy_pipeline(it->second);
    }
    values.clear();
}

void
DebugDrawer::destroyBuffer(sg_buffer &buffer) NANOEM_DECL_NOEXCEPT
{
    sg::destroy_buffer(buffer);
    buffer = { SG_INVALID_ID };
}

nanoem_u32_t
DebugDrawer::hash(bool depthEnabled) const
{
    bx::HashMurmur2A hash;
    hash.begin(depthEnabled);
    hash.add(m_project->viewportPixelFormat());
    hash.add(m_project->sampleCount());
    return hash.end();
}

sg_buffer
DebugDrawer::createBuffer(const dd::DrawVertex *vertices, int count)
{
    sg_buffer_desc desc;
    Inline::clearZeroMemory(desc);
    desc.size = count * sizeof(vertices[0]);
    desc.data.ptr = vertices;
    desc.data.size = desc.size;
    desc.type = SG_BUFFERTYPE_VERTEXBUFFER;
    desc.usage = SG_USAGE_IMMUTABLE;
    char label[Inline::kMarkerStringLength];
    if (Inline::isDebugLabelEnabled()) {
        StringUtils::format(label, sizeof(label), "%s/Vertices", kLabelPrefix);
        desc.label = label;
    }
    sg_buffer buffer = sg::make_buffer(&desc);
    nanoem_assert(sg::query_buffer_state(buffer) == SG_RESOURCESTATE_VALID, "vertex buffer must be valid");
    SG_LABEL_BUFFER(buffer, label);
    return buffer;
}

void
DebugDrawer::setupPointPipeline(bool depthEnabled, sg_pipeline &pipelineRef)
{
    const nanoem_u32_t key = hash(depthEnabled);
    PipelineMap::const_iterator it = m_points.find(key);
    if (it != m_points.end()) {
        pipelineRef = it->second;
    }
    else {
        sg_pipeline_desc desc;
        initializeCommonPipelineDescription(desc);
        desc.shader = m_pointedShader;
        sg_depth_state &ds = desc.depth;
        ds.write_enabled = depthEnabled;
        ds.compare = SG_COMPAREFUNC_LESS_EQUAL;
        desc.primitive_type = SG_PRIMITIVETYPE_POINTS;
        Project::setAlphaBlendMode(desc.colors[0]);
        char label[Inline::kMarkerStringLength];
        if (Inline::isDebugLabelEnabled()) {
            StringUtils::format(
                label, sizeof(label), "%s/Points/%d", kLabelPrefix, Inline::saturateInt32(m_points.size()));
            desc.label = label;
        }
        else {
            *label = 0;
        }
        sg_pipeline pipeline = sg::make_pipeline(&desc);
        nanoem_assert(sg::query_pipeline_state(pipeline) == SG_RESOURCESTATE_VALID, "pipeline must be valid");
        m_project->setRenderPipelineName(pipeline, label);
        m_points.insert(tinystl::make_pair(key, pipeline));
        pipelineRef = pipeline;
    }
    m_bindings.fs.images[0] = m_project->sharedFallbackImage();
    m_bindings.fs.samplers[0] = m_project->sharedFallbackSampler();
}

void
DebugDrawer::setupLinePipeline(bool depthEnabled, sg_pipeline &pipelineRef)
{
    const nanoem_u32_t key = hash(depthEnabled);
    PipelineMap::const_iterator it = m_lines.find(key);
    if (it != m_lines.end()) {
        pipelineRef = it->second;
    }
    else {
        sg_pipeline_desc desc;
        initializeCommonPipelineDescription(desc);
        desc.shader = m_shader;
        sg_depth_state &ds = desc.depth;
        if (depthEnabled) {
            ds.write_enabled = depthEnabled;
            ds.compare = SG_COMPAREFUNC_LESS_EQUAL;
        }
        desc.primitive_type = SG_PRIMITIVETYPE_LINES;
        char label[Inline::kMarkerStringLength];
        if (Inline::isDebugLabelEnabled()) {
            StringUtils::format(
                label, sizeof(label), "%s/Lines/%d", kLabelPrefix, Inline::saturateInt32(m_lines.size()));
            desc.label = label;
        }
        else {
            *label = 0;
        }
        sg_pipeline pipeline = sg::make_pipeline(&desc);
        nanoem_assert(sg::query_pipeline_state(pipeline) == SG_RESOURCESTATE_VALID, "pipeline must be valid");
        m_project->setRenderPipelineName(pipeline, label);
        m_lines.insert(tinystl::make_pair(key, pipeline));
        pipelineRef = pipeline;
    }
    m_bindings.fs.images[0] = m_project->sharedFallbackImage();
    m_bindings.fs.samplers[0] = m_project->sharedFallbackSampler();
}

void
DebugDrawer::setupGlyphPipeline(sg_pipeline &pipelineRef)
{
    const nanoem_u32_t key = hash(false);
    PipelineMap::const_iterator it = m_glyphs.find(key);
    if (it != m_glyphs.end()) {
        pipelineRef = it->second;
    }
    else {
        sg_pipeline_desc desc;
        initializeCommonPipelineDescription(desc);
        desc.shader = m_shader;
        desc.primitive_type = SG_PRIMITIVETYPE_TRIANGLES;
        char label[Inline::kMarkerStringLength];
        if (Inline::isDebugLabelEnabled()) {
            StringUtils::format(
                label, sizeof(label), "%s/Glyphs/%d", kLabelPrefix, Inline::saturateInt32(m_glyphs.size()));
            desc.label = label;
        }
        else {
            *label = 0;
        }
        sg_pipeline pipeline = sg::make_pipeline(&desc);
        nanoem_assert(sg::query_pipeline_state(pipeline) == SG_RESOURCESTATE_VALID, "pipeline must be valid");
        m_project->setRenderPipelineName(pipeline, label);
        m_glyphs.insert(tinystl::make_pair(key, pipeline));
        pipelineRef = pipeline;
    }
}

void
DebugDrawer::initializeCommonPipelineDescription(sg_pipeline_desc &desc)
{
    setupShader();
    Inline::clearZeroMemory(desc);
    desc.index_type = SG_INDEXTYPE_NONE;
    desc.colors[0].pixel_format = m_project->viewportPixelFormat();
    desc.sample_count = m_project->sampleCount();
    Project::setAlphaBlendMode(desc.colors[0]);
    sg_vertex_layout_state &ld = desc.layout;
    ld.buffers[0].stride = sizeof(dd::DrawVertex);
    ld.attrs[0] = sg_vertex_attr_state { 0, 0, SG_VERTEXFORMAT_FLOAT3 };
    ld.attrs[1] = sg_vertex_attr_state { 0, 0, SG_VERTEXFORMAT_FLOAT3 };
    ld.attrs[2] = sg_vertex_attr_state { 0, 8, SG_VERTEXFORMAT_FLOAT2 };
    ld.attrs[3] = sg_vertex_attr_state { 0, 0, SG_VERTEXFORMAT_FLOAT2 };
    ld.attrs[4] = sg_vertex_attr_state { 0, 0, SG_VERTEXFORMAT_FLOAT2 };
    ld.attrs[5] = sg_vertex_attr_state { 0, 0, SG_VERTEXFORMAT_FLOAT2 };
    ld.attrs[6] = sg_vertex_attr_state { 0, 0, SG_VERTEXFORMAT_FLOAT2 };
    ld.attrs[7] = sg_vertex_attr_state { 0, 12, SG_VERTEXFORMAT_FLOAT3 };
}

void
DebugDrawer::setupShader()
{
    if (!sg::is_valid(m_shader)) {
        sg_shader_desc desc;
        Inline::clearZeroMemory(desc);
        const sg_backend backend = sg::query_backend();
        if (backend == SG_BACKEND_D3D11) {
            desc.vs.bytecode.ptr = g_nanoem_debug_vs_dxbc_data;
            desc.vs.bytecode.size = g_nanoem_debug_vs_dxbc_size;
            desc.fs.bytecode.ptr = g_nanoem_debug_ps_dxbc_data;
            desc.fs.bytecode.size = g_nanoem_debug_ps_dxbc_size;
        }
        else if (sg::is_backend_metal(backend)) {
            desc.vs.bytecode.ptr = g_nanoem_debug_vs_msl_macos_data;
            desc.vs.bytecode.size = g_nanoem_debug_vs_msl_macos_size;
            desc.fs.bytecode.ptr = g_nanoem_debug_fs_msl_macos_data;
            desc.fs.bytecode.size = g_nanoem_debug_fs_msl_macos_size;
        }
        else if (backend == SG_BACKEND_GLCORE33) {
            desc.vs.source = reinterpret_cast<const char *>(g_nanoem_debug_vs_glsl_core33_data);
            desc.fs.source = reinterpret_cast<const char *>(g_nanoem_debug_fs_glsl_core33_data);
        }
        else if (backend == SG_BACKEND_GLES3) {
            desc.vs.source = reinterpret_cast<const char *>(g_nanoem_debug_vs_glsl_es3_data);
            desc.fs.source = reinterpret_cast<const char *>(g_nanoem_debug_fs_glsl_es3_data);
        }
        desc.vs.entry = "nanoemVSMain";
        desc.vs.uniform_blocks[0].size = sizeof(Matrix4x4);
        desc.fs.images[0] = sg_shader_image_desc { true, false, SG_IMAGETYPE_2D, SG_IMAGESAMPLETYPE_FLOAT };
        desc.fs.samplers[0] = sg_shader_sampler_desc { true, SG_SAMPLERTYPE_SAMPLE };
#if defined(NANOEM_ENABLE_SHADER_OPTIMIZED)
        desc.vs.uniform_blocks[0].uniforms[0] = sg_shader_uniform_desc { "_30", SG_UNIFORMTYPE_MAT4, 1 };
        desc.fs.image_sampler_pairs[0] = sg_shader_image_sampler_pair_desc { true, 0, 0, "SPIRV_Cross_Combined" };
#else
        desc.vs.uniform_blocks[0].uniforms[0] = sg_shader_uniform_desc { "ui_parameters_t", SG_UNIFORMTYPE_MAT4, 1 };
        desc.fs.image_sampler_pairs[0] = sg_shader_image_sampler_pair_desc { true, 0, 0, "SPIRV_Cross_Combined" };
#endif /* NANOEM_ENABLE_SHADER_OPTIMIZED */
        desc.fs.entry = "nanoemPSMain";
        desc.attrs[0] = sg_shader_attr_desc { "a_position", "SV_POSITION", 0 };
        desc.attrs[1] = sg_shader_attr_desc { "a_normal", "NORMAL", 0 };
        desc.attrs[2] = sg_shader_attr_desc { "a_texcoord0", "TEXCOORD", 0 };
        desc.attrs[3] = sg_shader_attr_desc { "a_texcoord1", "TEXCOORD", 1 };
        desc.attrs[4] = sg_shader_attr_desc { "a_texcoord2", "TEXCOORD", 2 };
        desc.attrs[5] = sg_shader_attr_desc { "a_texcoord3", "TEXCOORD", 3 };
        desc.attrs[6] = sg_shader_attr_desc { "a_texcoord4", "TEXCOORD", 4 };
        desc.attrs[7] = sg_shader_attr_desc { "a_color0", "COLOR", 0 };
        m_shader = sg::make_shader(&desc);
        if (sg::is_backend_metal(backend)) {
            desc.vs.bytecode.ptr = g_nanoem_pointed_debug_vs_msl_macos_data;
            desc.vs.bytecode.size = g_nanoem_pointed_debug_vs_msl_macos_size;
            m_pointedShader = sg::make_shader(&desc);
            nanoem_assert(sg::query_shader_state(m_pointedShader) == SG_RESOURCESTATE_VALID, "shader must be valid");
            SG_LABEL_SHADER(m_pointedShader, desc.label);
        }
        else {
            m_pointedShader = m_shader;
        }
    }
}

void
DebugDrawer::draw(sg_pipeline pipeline, sg_buffer buffer, int count)
{
    sg_pass pass = m_project->viewportPrimaryPass();
    if (sg::is_valid(pass)) {
        sg_pass_action action;
        sg::PassBlock::initializeLoadStoreAction(action);
        sg::PassBlock pb(m_project->sharedBatchDrawQueue(), pass, action);
        Matrix4x4 view, projection;
        m_project->activeCamera()->getViewTransform(view, projection);
        m_bindings.vertex_buffers[0] = buffer;
        pb.applyPipelineBindings(pipeline, m_bindings);
        pb.applyUniformBlock(SG_SHADERSTAGE_VS, glm::value_ptr(projection * view), sizeof(view));
        pb.draw(0, count);
    }
}

} /* namespace internal */
} /* namespace nanoem */
