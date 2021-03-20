/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/Grid.h"

#include "emapp/Constants.h"
#include "emapp/IEventPublisher.h"
#include "emapp/PerspectiveCamera.h"
#include "emapp/Project.h"
#include "emapp/ResourceBundle.h"
#include "emapp/internal/LineDrawer.h"
#include "emapp/private/CommonInclude.h"

#include "bx/hash.h"

namespace nanoem {

Grid::Grid(Project *project)
    : m_project(project)
    , m_drawer(nullptr)
    , m_lineColor(0.5f)
    , m_cell(5.0f, 5.0f)
    , m_size(10.0f, 10.0f)
    , m_opacity(1.0f)
    , m_visible(false)
{
    m_drawer = nanoem_new(internal::LineDrawer(project));
}

Grid::~Grid() NANOEM_DECL_NOEXCEPT
{
    nanoem_delete_safe(m_drawer);
}

void
Grid::initialize()
{
    m_drawer->initialize();
    sg_buffer_desc vbd;
    Inline::clearZeroMemory(vbd);
    vbd.size = sizeof(sg::LineVertexUnit) * 0xfff;
    vbd.usage = SG_USAGE_STREAM;
    if (Inline::isDebugLabelEnabled()) {
        vbd.label = "@nanoem/GridPass/Vertices";
    }
    m_vertexBuffer = sg::make_buffer(&vbd);
    nanoem_assert(sg::query_buffer_state(m_vertexBuffer) == SG_RESOURCESTATE_VALID, "buffer must be valid");
    SG_LABEL_BUFFER(m_vertexBuffer, vbd.label);
}

void
Grid::destroy() NANOEM_DECL_NOEXCEPT
{
    SG_PUSH_GROUP("Grid::destroy");
    m_drawer->destroy();
    sg::destroy_buffer(m_vertexBuffer);
    SG_POP_GROUP();
}

void
Grid::draw(sg::PassBlock &pb)
{
    if (m_visible) {
        tinystl::vector<sg::LineVertexUnit, TinySTLAllocator> vertexList(numVertices());
        sg::LineVertexUnit *vertices = vertexList.data();
        int index = 0;
        generateXGrid(vertices, index);
        generateYGrid(vertices, index);
        generateUnit(vertices, Vector3(1, 0, 0), index);
        generateUnit(vertices, Vector3(0, 1, 0), index);
        generateUnit(vertices, Vector3(0, 0, 1), index);
        sg::update_buffer(
            m_vertexBuffer, vertexList.data(), Inline::saturateInt32(sizeof(*vertices) * vertexList.size()));
        const internal::LineDrawer::Option option(m_vertexBuffer, vertexList.size());
        m_drawer->draw(pb, option);
    }
}

Vector2
Grid::cell() const NANOEM_DECL_NOEXCEPT
{
    return m_cell;
}

void
Grid::setCell(const Vector2 &value)
{
    if (glm::distance(value, m_cell) > Constants::kEpsilon) {
        m_cell = glm::max(value, Vector2(0.01f));
        m_project->eventPublisher()->publishSetGridCellEvent(value);
    }
}

Vector2
Grid::size() const NANOEM_DECL_NOEXCEPT
{
    return m_size;
}

void
Grid::resize(const Vector2 &value)
{
    if (glm::distance(value, m_cell) > Constants::kEpsilon) {
        m_size = value;
        m_project->eventPublisher()->publishSetGridSizeEvent(value);
    }
}

nanoem_f32_t
Grid::opacity() const NANOEM_DECL_NOEXCEPT
{
    return m_opacity;
}

void
Grid::setOpacity(nanoem_f32_t value)
{
    m_opacity = glm::clamp(value, 0.0f, 1.0f);
}

bool
Grid::isVisible() const NANOEM_DECL_NOEXCEPT
{
    return m_visible;
}

void
Grid::setVisible(bool value)
{
    if (value != m_visible) {
        m_visible = value;
        m_project->eventPublisher()->publishToggleGridEnabledEvent(value);
    }
}

nanoem_u16_t
Grid::numVertices() const
{
    return nanoem_u16_t(m_size.x + m_size.y + 1) * 4 + 6;
}

void
Grid::generateXGrid(sg::LineVertexUnit *vertices, int &index)
{
    const Vector4U8 color(m_lineColor * Vector3(0xff), m_opacity * 0xff);
    for (int i = -int(m_size.x); i <= int(m_size.x); i++) {
        nanoem_f32_t height = m_size.y * m_cell.y;
        nanoem_f32_t x = i * m_cell.x;
        sg::LineVertexUnit &from = vertices[index];
        from.m_position = Vector3(x, 0, height);
        from.m_color = color;
        index++;
        sg::LineVertexUnit &to = vertices[index];
        to.m_position = Vector3(x, 0, i == 0 ? 0 : -height); /* prevent Z-axis flickering */
        to.m_color = color;
        index++;
    }
}

void
Grid::generateYGrid(sg::LineVertexUnit *vertices, int &index)
{
    const Vector4U8 color(m_lineColor * Vector3(0xff), m_opacity * 0xff);
    for (int i = -int(m_size.y); i <= int(m_size.y); i++) {
        nanoem_f32_t width = m_size.x * m_cell.x;
        nanoem_f32_t z = i * m_cell.y;
        sg::LineVertexUnit &from = vertices[index];
        from.m_position = Vector3(-width, 0, z);
        from.m_color = color;
        index++;
        sg::LineVertexUnit &to = vertices[index];
        to.m_position = Vector3(i == 0 ? 0 : width, 0, z); /* prevent X-axis flickering */
        to.m_color = color;
        index++;
    }
}

void
Grid::generateUnit(sg::LineVertexUnit *vertices, const Vector3 &color, int &index)
{
    const Vector4U8 c(color * Vector3(0xff), m_opacity * 0xff);
    nanoem_f32_t width = m_size.x * m_cell.x;
    sg::LineVertexUnit &from = vertices[index];
    from.m_position = Vector3(0, 0, 0);
    from.m_color = c;
    index++;
    sg::LineVertexUnit &to = vertices[index];
    to.m_position = Vector3(color * Vector3(width * (color.z > 0 ? -1 : 1)));
    to.m_color = c;
    index++;
}

} /* namespace nanoem */
