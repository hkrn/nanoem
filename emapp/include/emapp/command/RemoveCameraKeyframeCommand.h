/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COMMAND_REMOVECAMERAKEYFRAMECOMMAND_H_
#define NANOEM_EMAPP_COMMAND_REMOVECAMERAKEYFRAMECOMMAND_H_

#include "emapp/command/BaseCameraKeyframeCommand.h"

namespace nanoem {
namespace command {

class RemoveCameraKeyframeCommand NANOEM_DECL_SEALED : public BaseCameraKeyframeCommand {
public:
    ~RemoveCameraKeyframeCommand() NANOEM_DECL_NOEXCEPT;
    static undo_command_t *create(const void *messagePtr, const ICamera *camera, Motion *motion);
    static undo_command_t *create(const Motion::CameraKeyframeList &keyframes, const ICamera *camera, Motion *motion);

    void undo(Error &error);
    void redo(Error &error);
    const char *name() const NANOEM_DECL_NOEXCEPT;

private:
    static CameraKeyframeList toKeyframeList(const Motion::CameraKeyframeList &keyframes, const Motion *motion);
    RemoveCameraKeyframeCommand(const Motion::CameraKeyframeList &keyframes, const ICamera *camera, Motion *motion);

    void read(const void *messagePtr);
    void write(void *messagePtr);
    void release(void *messagePtr);
};

} /* namespace command */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COMMAND_REMOVECAMERAKEYFRAMECOMMAND_H_ */
