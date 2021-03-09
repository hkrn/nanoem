/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_GRID_H_
#define NANOEM_EMAPP_GRID_H_

#include "emapp/Forward.h"

namespace nanoem {

class Project;
namespace internal {
class LineDrawer;
}

class Grid NANOEM_DECL_SEALED : private NonCopyable {
public:
    Grid(Project *project);
    ~Grid() NANOEM_DECL_NOEXCEPT;

    void initialize();
    void destroy() NANOEM_DECL_NOEXCEPT;
    void draw(sg::PassBlock &pb);

    Vector2 cell() const NANOEM_DECL_NOEXCEPT;
    void setCell(const Vector2 &value);
    Vector2 size() const NANOEM_DECL_NOEXCEPT;
    void resize(const Vector2 &value);
    nanoem_f32_t opacity() const NANOEM_DECL_NOEXCEPT;
    void setOpacity(nanoem_f32_t value);
    bool isVisible() const NANOEM_DECL_NOEXCEPT;
    void setVisible(bool value);

private:
    nanoem_u16_t numVertices() const;
    void generateXGrid(sg::LineVertexUnit *vertices, int &index);
    void generateYGrid(sg::LineVertexUnit *vertices, int &index);
    void generateUnit(sg::LineVertexUnit *vertices, const Vector3 &color, int &index);

    Project *m_project;
    internal::LineDrawer *m_drawer;
    sg_buffer m_vertexBuffer;
    Vector3 m_lineColor;
    Vector2 m_cell;
    Vector2 m_size;
    nanoem_f32_t m_opacity;
    bool m_visible;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_GRID_H_ */
