/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COMMAND_ADDMORPHKEYFRAMECOMMAND_H_
#define NANOEM_EMAPP_COMMAND_ADDMORPHKEYFRAMECOMMAND_H_

#include "emapp/command/BaseMorphKeyframeCommand.h"

namespace nanoem {
namespace command {

class AddMorphKeyframeCommand NANOEM_DECL_SEALED : public BaseMorphKeyframeCommand {
public:
    ~AddMorphKeyframeCommand() NANOEM_DECL_NOEXCEPT;
    static undo_command_t *create(const void *messagePtr, const Model *model, Motion *motion);
    static undo_command_t *create(const Motion::MorphFrameIndexSetMap &morphs, const Model *model, Motion *motion);

    void undo(Error &error);
    void redo(Error &error);
    const char *name() const NANOEM_DECL_NOEXCEPT;

private:
    static MorphKeyframeList toKeyframeList(
        const Motion::MorphFrameIndexSetMap &morphs, const Model *model, const Motion *motion);
    AddMorphKeyframeCommand(const Motion::MorphFrameIndexSetMap &morphs, const Model *model, Motion *motion);

    void read(const void *messagePtr);
    void write(void *messagePtr);
    void release(void *messagePtr);
};

} /* namespace command */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COMMAND_ADDMORPHKEYFRAMECOMMAND_H_ */
