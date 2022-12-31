/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COMMAND_ADDCAMERAKEYFRAMECOMMAND_H_
#define NANOEM_EMAPP_COMMAND_ADDCAMERAKEYFRAMECOMMAND_H_

#include "emapp/command/BaseCameraKeyframeCommand.h"

namespace nanoem {
namespace command {

class AddCameraKeyframeCommand NANOEM_DECL_SEALED : public BaseCameraKeyframeCommand {
public:
    ~AddCameraKeyframeCommand() NANOEM_DECL_NOEXCEPT;
    static undo_command_t *create(const void *messagePtr, const ICamera *camera, Motion *motion);
    static undo_command_t *create(
        const ICamera *camera, const Motion::FrameIndexList &targetFrameIndices, Motion *motion);

    void undo(Error &error);
    void redo(Error &error);
    const char *name() const NANOEM_DECL_NOEXCEPT;

private:
    static CameraKeyframeList toKeyframeList(
        const ICamera *camera, const Motion::FrameIndexList &frameIndices, const Motion *motion);
    AddCameraKeyframeCommand(const ICamera *camera, const Motion::FrameIndexList &targetFrameIndices, Motion *motion);

    void read(const void *messagePtr);
    void write(void *messagePtr);
    void release(void *messagePtr);
};

} /* namespace command */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COMMAND_ADDCAMERAKEYFRAMECOMMAND_H_ */
