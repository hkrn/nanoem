/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_CLEANER_H_
#define NANOEM_EMAPP_INTERNAL_CLEANER_H_

#include "emapp/internal/BasePass.h"

namespace nanoem {

class Project;

namespace internal {

class ClearPass NANOEM_DECL_SEALED : public BasePass {
public:
    ClearPass(Project *project);
    ~ClearPass() NANOEM_DECL_NOEXCEPT;

    void clear(
        sg::PassBlock::IDrawQueue *drawQueue, sg_pass pass, const sg_pass_action &action, const PixelFormat &format);

private:
    void draw(sg::PassBlock::IDrawQueue *drawQueue, sg_pipeline pipeline, sg_pass pass, const sg_pass_action &action);
    void setupVertexBuffer();
    void setupShaderDescription(sg_shader_desc &desc) NANOEM_DECL_OVERRIDE;
    void setupPipelineDescription(sg_pipeline_desc &desc) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_CLEANER_H_ */
