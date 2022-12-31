/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COMMAND_ADDBONEKEYFRAMECOMMAND_H_
#define NANOEM_EMAPP_COMMAND_ADDBONEKEYFRAMECOMMAND_H_

#include "emapp/command/BaseBoneKeyframeCommand.h"

namespace nanoem {
namespace command {

class AddBoneKeyframeCommand NANOEM_DECL_SEALED : public BaseBoneKeyframeCommand {
public:
    ~AddBoneKeyframeCommand() NANOEM_DECL_NOEXCEPT;
    static undo_command_t *create(const void *messagePtr, const Model *model, Motion *motion);
    static undo_command_t *create(const Motion::BoneFrameIndexSetMap &bones, const Model *model, Motion *motion);

    void undo(Error &error);
    void redo(Error &error);
    const char *name() const NANOEM_DECL_NOEXCEPT;

private:
    typedef tinystl::vector<tinystl::pair<const nanoem_model_bone_t *, Motion::KeyframeBound>, nanoem::TinySTLAllocator>
        BoneList;

    static BoneKeyframeList toKeyframeList(const BoneList &bones, const Model *model, const Motion *motion);
    static BoneKeyframe toKeyframe(const nanoem_model_bone_t *bonePtr, const Motion::KeyframeBound &bound,
        const Motion *motion, nanoem_unicode_string_factory_t *factory, bool enableBezierCurveAdjustment,
        bool enablePhysicsSimulation);
    AddBoneKeyframeCommand(const BoneList &bones, const Model *model, Motion *motion);

    void read(const void *messagePtr);
    void write(void *messagePtr);
    void release(void *messagePtr);
};

} /* namespace command */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COMMAND_ADDBONEKEYFRAMECOMMAND_H_ */
