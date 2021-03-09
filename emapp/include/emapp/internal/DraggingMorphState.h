/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#ifndef NANOEM_EMAPP_PRIVATE_DRAGGINGMORPHSTATE
#define NANOEM_EMAPP_PRIVATE_DRAGGINGMORPHSTATE

#include "emapp/PhysicsEngine.h"
#include "emapp/model/BindPose.h"

namespace nanoem {

class Model;
class Project;

namespace internal {

class DraggingMorphSliderState NANOEM_DECL_SEALED : private NonCopyable {
public:
    DraggingMorphSliderState(const nanoem_model_morph_t *morph, Model *model, Project *project, nanoem_f32_t weight);
    ~DraggingMorphSliderState();

    void deform(nanoem_f32_t weight);
    void commit();

private:
    void setCameraLocked(bool value);

    const nanoem_f32_t m_lastMorphWeight;
    const nanoem_model_morph_t *m_lastActiveMorph;
    const nanoem_model_morph_t *m_targetMorph;
    Project *m_project;
    Model *m_model;
    model::BindPose m_lastBindPose;
    nanoem_u32_t m_lastEditingMode;
};

} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_PRIVATE_DRAGGINGMORPHSTATE */
