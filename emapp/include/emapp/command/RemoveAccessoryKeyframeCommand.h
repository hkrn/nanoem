/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COMMAND_REMOVEACCESSORYKEYFRAMECOMMAND_H_
#define NANOEM_EMAPP_COMMAND_REMOVEACCESSORYKEYFRAMECOMMAND_H_

#include "emapp/command/BaseAccessoryKeyframeCommand.h"

namespace nanoem {
namespace command {

class RemoveAccessoryKeyframeCommand NANOEM_DECL_SEALED : public BaseAccessoryKeyframeCommand {
public:
    ~RemoveAccessoryKeyframeCommand() NANOEM_DECL_NOEXCEPT;
    static undo_command_t *create(const void *messagePtr, const Accessory *accessory, Motion *motion);
    static undo_command_t *create(
        const Motion::AccessoryKeyframeList &keyframes, const Accessory *accessory, Motion *motion);

    void undo(Error &error);
    void redo(Error &error);
    const char *name() const NANOEM_DECL_NOEXCEPT;

private:
    static AccessoryKeyframeList toKeyframeList(
        const Motion::AccessoryKeyframeList &keyframes, const Accessory *accessory, const Motion *motion);
    RemoveAccessoryKeyframeCommand(
        const Motion::AccessoryKeyframeList &keyframes, const Accessory *accessory, Motion *motion);

    void read(const void *messagePtr);
    void write(void *messagePtr);
    void release(void *messagePtr);
};

} /* namespace command */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COMMAND_REMOVEACCESSORYKEYFRAMECOMMAND_H_ */
