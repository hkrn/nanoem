/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COMMAND_TRANSFORMBONECOMMAND_H_
#define NANOEM_EMAPP_COMMAND_TRANSFORMBONECOMMAND_H_

#include "emapp/command/BaseUndoCommand.h"

#include "emapp/model/BindPose.h"
#include "emapp/model/Bone.h"

namespace nanoem {

class Model;

namespace command {

class TransformBoneCommand NANOEM_DECL_SEALED : public BaseUndoCommand {
public:
    ~TransformBoneCommand() NANOEM_DECL_NOEXCEPT;
    static undo_command_t *create(const void *messagePtr, Model *model, Project *project);
    static undo_command_t *create(const model::BindPose &lastBindPose, const model::BindPose &currentBindPose,
        const model::Bone::List &targetBones, Model *model, Project *project);

    void undo(Error &error);
    void redo(Error &error);
    const char *name() const NANOEM_DECL_NOEXCEPT;

private:
    typedef tinystl::pair<const nanoem_model_bone_t *, bool> DirtyState;
    typedef tinystl::vector<DirtyState, nanoem::TinySTLAllocator> DirtyStateList;
    void execute(const model::BindPose &value, bool markAllDirty);
    static inline DirtyStateList createDirtyStates(const model::Bone::List &bones, const Model *model);
    TransformBoneCommand(const model::BindPose &lastBindPose, const model::BindPose &currentBindPose,
        const model::Bone::List &targetBones, Model *model, Project *project);

    void read(const void *messagePtr);
    void write(void *messagePtr);
    void release(void *messagePtr);

    const nanoem_frame_index_t m_localFrameIndex;
    Model *m_model;
    model::BindPose m_lastBindPose;
    model::BindPose m_currentBindPose;
    DirtyStateList m_dirtyStates;
};

} /* namespace command */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COMMAND_TRANSFORMBONECOMMAND_H_ */
