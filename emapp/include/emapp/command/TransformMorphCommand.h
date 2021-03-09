/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COMMAND_TRANSFORMMORPHCOMMAND_H_
#define NANOEM_EMAPP_COMMAND_TRANSFORMMORPHCOMMAND_H_

#include "emapp/command/BaseUndoCommand.h"
#include "emapp/model/BindPose.h"

namespace nanoem {

class Model;

namespace command {

class TransformMorphCommand NANOEM_DECL_SEALED : public BaseUndoCommand {
public:
    typedef tinystl::pair<const nanoem_model_morph_t *, tinystl::pair<nanoem_f32_t, nanoem_f32_t>> WeightState;
    typedef tinystl::vector<WeightState, nanoem::TinySTLAllocator> WeightStateList;

    ~TransformMorphCommand() NANOEM_DECL_NOEXCEPT;
    static undo_command_t *create(const void *messagePtr, Model *model, Project *project);
    static undo_command_t *create(
        const WeightStateList &weightStates, const model::BindPose &currentBindPose, Model *model, Project *project);

    void undo(Error &error);
    void redo(Error &error);
    const char *name() const NANOEM_DECL_NOEXCEPT;

private:
    void execute(bool undo);
    TransformMorphCommand(
        const WeightStateList &weightStates, const model::BindPose &currentBindPose, Model *model, Project *project);

    void read(const void *messagePtr);
    void write(void *messagePtr);
    void release(void *messagePtr);

    const nanoem_frame_index_t m_localFrameIndex;
    WeightStateList m_weightStates;
    Model *m_model;
    model::BindPose m_currentBindPose;
};

} /* namespace command */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COMMAND_TRANSFORMMORPHCOMMAND_H_ */
