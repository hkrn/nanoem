/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_LINEDRAWER_H_
#define NANOEM_EMAPP_INTERNAL_LINEDRAWER_H_

#include "emapp/Forward.h"

namespace nanoem {

class Project;

namespace internal {

class LineDrawer NANOEM_DECL_SEALED : private NonCopyable {
public:
    struct Option {
        Option(sg_buffer vertexBuffer, size_t numIndices);
        sg::PassBlock::IDrawQueue *m_drawQueue;
        sg_pass m_pass;
        sg_buffer m_vertexBuffer;
        sg_buffer m_indexBuffer;
        sg_primitive_type m_primitiveType;
        sg_index_type m_indexType;
        size_t m_offset;
        size_t m_numIndices;
        Matrix4x4 m_worldTransform;
        Vector4 m_color;
        bool m_enableBlendMode;
        bool m_enableDepthTest;
    };
    LineDrawer(Project *project);
    ~LineDrawer() NANOEM_DECL_NOEXCEPT;

    void initialize();
    void draw(sg::PassBlock &pb, const Option &option);
    void drawPass(const Option &option);
    void destroy() NANOEM_DECL_NOEXCEPT;

private:
    Project *m_project;
    sg_bindings m_bindings;
    PipelineMap m_pipelines;
    sg_shader m_shader;
    sg_shader m_pointedShader;
};

} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_LINEDRAWER_H_ */
