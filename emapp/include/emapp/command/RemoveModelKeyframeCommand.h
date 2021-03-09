/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COMMAND_REMOVEMODELKEYFRAMECOMMAND_H_
#define NANOEM_EMAPP_COMMAND_REMOVEMODELKEYFRAMECOMMAND_H_

#include "emapp/command/BaseModelKeyframeCommand.h"

namespace nanoem {
namespace command {

class RemoveModelKeyframeCommand NANOEM_DECL_SEALED : public BaseModelKeyframeCommand {
public:
    ~RemoveModelKeyframeCommand() NANOEM_DECL_NOEXCEPT;
    static undo_command_t *create(const void *messagePtr, const Model *model, Motion *motion);
    static undo_command_t *create(const Motion::ModelKeyframeList &keyframes, const Model *model, Motion *motion);

    void undo(Error &error);
    void redo(Error &error);
    const char *name() const NANOEM_DECL_NOEXCEPT;

private:
    static ModelKeyframeList toKeyframeList(
        const Motion::ModelKeyframeList &keyframes, const Model *model, const Motion *motion);
    RemoveModelKeyframeCommand(const Motion::ModelKeyframeList &keyframes, const Model *model, Motion *motion);

    void read(const void *messagePtr);
    void write(void *messagePtr);
    void release(void *messagePtr);
};

} /* namespace command */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COMMAND_REMOVEMODELKEYFRAMECOMMAND_H_ */
