/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/BasePass.h"

#include "emapp/PixelFormat.h"
#include "emapp/Project.h"
#include "emapp/StringUtils.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {

class Project;

namespace internal {

BasePass::BasePass(Project *project)
    : m_project(project)
{
    Inline::clearZeroMemory(m_bindings);
    m_shader = { SG_INVALID_ID };
}

BasePass::~BasePass() NANOEM_DECL_NOEXCEPT
{
}

void
BasePass::destroy() NANOEM_DECL_NOEXCEPT
{
    for (PipelineMap::const_iterator it = m_pipelines.begin(), end = m_pipelines.end(); it != end; ++it) {
        sg::destroy_pipeline(it->second);
    }
    m_pipelines.clear();
    sg::destroy_shader(m_shader);
    m_shader = { SG_INVALID_ID };
    sg::destroy_buffer(m_bindings.vertex_buffers[0]);
    Inline::clearZeroMemory(m_bindings);
}

void
BasePass::setupPipeline(const PixelFormat &format, sg_pipeline &pipelineRef)
{
    const nanoem_u32_t key = format.hash();
    PipelineMap::const_iterator it = m_pipelines.find(key);
    if (it != m_pipelines.end()) {
        pipelineRef = it->second;
    }
    else {
        if (!sg::is_valid(m_shader)) {
            sg_shader_desc sd;
            Inline::clearZeroMemory(sd);
            setupShaderDescription(sd);
            if (Inline::isDebugLabelEnabled()) {
                sd.label = "@nanoem/BasePass";
            }
            m_shader = sg::make_shader(&sd);
            nanoem_assert(sg::query_shader_state(m_shader) == SG_RESOURCESTATE_VALID, "shader must be valid");
            SG_LABEL_SHADER(m_shader, sd.label);
        }
        sg_pipeline_desc pd;
        Inline::clearZeroMemory(pd);
        setupPipelineDescription(pd);
        pd.color_count = format.numColorAttachments();
        for (int i = 0, numColorAttachments = pd.color_count; i < numColorAttachments; i++) {
            pd.colors[i].pixel_format = format.colorPixelFormat(i);
        }
        pd.depth.pixel_format = format.depthPixelFormat();
        pd.sample_count = format.numSamples();
        pd.shader = m_shader;
        char label[Inline::kMarkerStringLength];
        if (Inline::isDebugLabelEnabled()) {
            StringUtils::format(
                label, sizeof(label), "@nanoem/%s/%d", name(), Inline::saturateInt32(m_pipelines.size()));
            pd.label = label;
        }
        else {
            *label = 0;
        }
        sg_pipeline pipeline = sg::make_pipeline(&pd);
        nanoem_assert(sg::query_pipeline_state(pipeline) == SG_RESOURCESTATE_VALID, "pipeline must be valid");
        m_project->setRenderPipelineName(pipeline, label);
        m_pipelines.insert(tinystl::make_pair(key, pipeline));
        pipelineRef = pipeline;
    }
}

} /* namespace internal */
} /* namespace nanoem */
