/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_GIZMOCONTROLLER_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_GIZMOCONTROLLER_H_

#include "emapp/Forward.h"
#include "imgui/imgui.h"

namespace nanoem {

class Model;
class Project;

namespace internal {
namespace imgui {

class GizmoController : private NonCopyable {
public:
    GizmoController();
    ~GizmoController();

    void begin();
    bool draw(ImDrawList *drawList, const ImVec2 &offset, const ImVec2 &size, Project *project);

private:
    void applyDeltaTransform(const Matrix4x4 &delta, Model *activeModel);
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_GIZMOCONTROLLER_H_ */
