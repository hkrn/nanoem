/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/Model.h"
#include "emapp/internal/DraggingMorphState.h"

using namespace nanoem;
using namespace nanoem::internal;
using namespace test;

TEST_CASE("model_dragging_morph", "[emapp][model]")
{
    TestScope scope;
    {
        ProjectPtr o = scope.createProject();
        Project *project = o->withRecoverable();
        Model *activeModel = o->createModel();
        project->addModel(activeModel);
        project->setActiveModel(activeModel);
        nanoem_rsize_t numMorphs;
        nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(activeModel->data(), &numMorphs);
        const nanoem_model_morph_t *activeMorph = morphs[0];
        std::unique_ptr<DraggingMorphSliderState> state(
            new DraggingMorphSliderState(activeMorph, activeModel, project, 0.0f));
        state->deform(0.42f);
        SECTION("commit")
        {
            state->commit();
            REQUIRE(model::Morph::cast(activeMorph)->weight() == Approx(0.42f));
        }
        SECTION("undo")
        {
            state->commit();
            project->handleUndoAction();
            REQUIRE(model::Morph::cast(activeMorph)->weight() == Approx(0.0f));
        }
        project->handleRedoAction();
        SECTION("undo and redo")
        {
            state->commit();
            project->handleUndoAction();
            project->handleRedoAction();
            REQUIRE(model::Morph::cast(activeMorph)->weight() == Approx(0.42f));
        }
        SECTION("recovery")
        {
            state->commit();
            project->handleUndoAction();
            project->handleRedoAction();
            ProjectPtr o2 = scope.createProject();
            Project *project2 = o2.get()->m_project;
            scope.recover(project2);
            nanoem_model_morph_t *const *morphs2 =
                nanoemModelGetAllMorphObjects(project2->activeModel()->data(), &numMorphs);
            const nanoem_model_morph_t *activeMorph2 = morphs2[0];
            REQUIRE(model::Morph::cast(activeMorph2)->weight() == Approx(0.42f));
        }
    }
}
