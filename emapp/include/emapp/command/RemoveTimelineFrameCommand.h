/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COMMAND_SHIFTALLKEYFRAMESFORWARDCOMMAND_H_
#define NANOEM_EMAPP_COMMAND_SHIFTALLKEYFRAMESFORWARDCOMMAND_H_

#include "emapp/command/BaseShiftingAllKeyframesCommand.h"

namespace nanoem {
namespace command {

class RemoveTimelineFrameCommand NANOEM_DECL_SEALED : public BaseShiftingAllKeyframesCommand {
public:
    ~RemoveTimelineFrameCommand() NANOEM_DECL_NOEXCEPT;
    static undo_command_t *create(const void *messagePtr, Project *project);
    static undo_command_t *create(Motion *motion, nanoem_frame_index_t frameIndex, nanoem_u32_t types);

    void undo(Error &error);
    void redo(Error &error);
    const char *name() const NANOEM_DECL_NOEXCEPT;

private:
    RemoveTimelineFrameCommand(Project *project);
    RemoveTimelineFrameCommand(Motion *motion, nanoem_frame_index_t frameIndex, nanoem_u32_t types);

    void read(const void *messagePtr);
    void write(void *messagePtr);
    void release(void *messagePtr);
};

} /* namespace command */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COMMAND_SHIFTALLKEYFRAMESFORWARDCOMMAND_H_ */
