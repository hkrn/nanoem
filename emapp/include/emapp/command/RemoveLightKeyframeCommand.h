/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COMMAND_REMOVELIGHTKEYFRAMECOMMAND_H_
#define NANOEM_EMAPP_COMMAND_REMOVELIGHTKEYFRAMECOMMAND_H_

#include "emapp/command/BaseLightKeyframeCommand.h"

namespace nanoem {
namespace command {

class RemoveLightKeyframeCommand NANOEM_DECL_SEALED : public BaseLightKeyframeCommand {
public:
    ~RemoveLightKeyframeCommand() NANOEM_DECL_NOEXCEPT;
    static undo_command_t *create(const void *messagePtr, const ILight *light, Motion *motion);
    static undo_command_t *create(const Motion::LightKeyframeList &keyframes, const ILight *light, Motion *motion);

    void undo(Error &error);
    void redo(Error &error);
    const char *name() const NANOEM_DECL_NOEXCEPT;

private:
    static LightKeyframeList toKeyframeList(const Motion::LightKeyframeList &keyframes, const Motion *motion);
    RemoveLightKeyframeCommand(const Motion::LightKeyframeList &keyframes, const ILight *light, Motion *motion);

    void read(const void *messagePtr);
    void write(void *messagePtr);
    void release(void *messagePtr);
};

} /* namespace command */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COMMAND_REMOVELIGHTKEYFRAMECOMMAND_H_ */
