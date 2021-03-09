/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COMMAND_ADDSELFSHADOWKEYFRAMECOMMAND_H_
#define NANOEM_EMAPP_COMMAND_ADDSELFSHADOWKEYFRAMECOMMAND_H_

#include "emapp/command/BaseSelfShadowKeyframeCommand.h"

namespace nanoem {
namespace command {

class AddSelfShadowKeyframeCommand NANOEM_DECL_SEALED : public BaseSelfShadowKeyframeCommand {
public:
    ~AddSelfShadowKeyframeCommand() NANOEM_DECL_NOEXCEPT;
    static undo_command_t *create(const void *messagePtr, const ShadowCamera *shadow, Motion *motion);
    static undo_command_t *create(
        const ShadowCamera *shadow, const Motion::FrameIndexList &targetFrameIndices, Motion *motion);

    void undo(Error &error);
    void redo(Error &error);
    const char *name() const NANOEM_DECL_NOEXCEPT;

private:
    static SelfShadowKeyframeList toKeyframeList(
        const ShadowCamera *shadow, const Motion::FrameIndexList &frameIndices, const Motion *motion);
    AddSelfShadowKeyframeCommand(
        const ShadowCamera *shadow, const Motion::FrameIndexList &targetFrameIndices, Motion *motion);

    void read(const void *messagePtr);
    void write(void *messagePtr);
    void release(void *messagePtr);
};

} /* namespace command */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COMMAND_ADDSELFSHADOWKEYFRAMECOMMAND_H_ */
