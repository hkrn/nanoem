/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_BASEPASS_H_
#define NANOEM_EMAPP_INTERNAL_BASEPASS_H_

#include "emapp/Project.h"

namespace nanoem {

struct PixelFormat;
class Project;

namespace internal {

class BasePass : private NonCopyable {
public:
    BasePass(Project *project);
    virtual ~BasePass() NANOEM_DECL_NOEXCEPT;

    void destroy() NANOEM_DECL_NOEXCEPT;

protected:
    void setupPipeline(const PixelFormat &format, sg_pipeline &pipelineRef);
    virtual void setupShaderDescription(sg_shader_desc &desc) = 0;
    virtual void setupPipelineDescription(sg_pipeline_desc &desc) = 0;
    virtual const char *name() const NANOEM_DECL_NOEXCEPT = 0;

    Project *m_project;
    sg_bindings m_bindings;

private:
    sg_shader m_shader;
    PipelineMap m_pipelines;
};

} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_BLITTER_H_ */
