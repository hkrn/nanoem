/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COMMAND_ADDLIGHTKEYFRAMECOMMAND_H_
#define NANOEM_EMAPP_COMMAND_ADDLIGHTKEYFRAMECOMMAND_H_

#include "emapp/command/BaseLightKeyframeCommand.h"

namespace nanoem {
namespace command {

class AddLightKeyframeCommand NANOEM_DECL_SEALED : public BaseLightKeyframeCommand {
public:
    ~AddLightKeyframeCommand() NANOEM_DECL_NOEXCEPT;
    static undo_command_t *create(const void *messagePtr, const ILight *light, Motion *motion);
    static undo_command_t *create(
        const ILight *light, const Motion::FrameIndexList &targetFrameIndices, Motion *motion);

    void undo(Error &error);
    void redo(Error &error);
    const char *name() const NANOEM_DECL_NOEXCEPT;

private:
    static LightKeyframeList toKeyframeList(
        const ILight *light, const Motion::FrameIndexList &frameIndices, const Motion *motion);
    AddLightKeyframeCommand(const ILight *light, const Motion::FrameIndexList &targetFrameIndices, Motion *motion);

    void read(const void *messagePtr);
    void write(void *messagePtr);
    void release(void *messagePtr);
};

} /* namespace command */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COMMAND_ADDLIGHTKEYFRAMECOMMAND_H_ */
