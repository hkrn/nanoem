/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COMMANDREGISTRATOR_H_
#define NANOEM_EMAPP_COMMANDREGISTRATOR_H_

#include "emapp/Motion.h"
#include "emapp/TimelineSegment.h"

struct undo_command_t;

namespace nanoem {

class Accessory;
class Error;
class ICamera;
class ILight;
class Model;
class Progress;
class Project;

class CommandRegistrator NANOEM_DECL_SEALED : private NonCopyable {
public:
    CommandRegistrator(Project *project);
    ~CommandRegistrator() NANOEM_DECL_NOEXCEPT;

    void registerAddAccessoryKeyframesCommand(
        const Motion::FrameIndexList &frameIndices, Accessory *accessory, Motion *motion);
    void registerAddAccessoryKeyframesCommandByCurrentLocalFrameIndex(Accessory *accessory);
    void registerAddBoneKeyframesCommand(const Motion::BoneFrameIndexSetMap &bones, Model *model, Motion *motion);
    void registerAddBoneKeyframesCommandBySelectedBoneSet(Model *model);
    void registerAddCameraKeyframesCommand(
        const Motion::FrameIndexList &frameIndices, ICamera *globalCamera, Motion *motion);
    void registerAddCameraKeyframesCommandByCurrentLocalFrameIndex();
    void registerAddLightKeyframesCommand(
        const Motion::FrameIndexList &frameIndices, ILight *globalLight, Motion *motion);
    void registerAddLightKeyframesCommandByCurrentLocalFrameIndex();
    void registerAddModelKeyframesCommand(const Motion::FrameIndexList &frameIndices, Model *model, Motion *motion);
    void registerAddModelKeyframesCommandByCurrentLocalFrameIndex();
    void registerAddModelKeyframesCommandByCurrentLocalFrameIndex(Model *model);
    void registerAddMorphKeyframesCommand(const Motion::MorphFrameIndexSetMap &morphs, Model *model, Motion *motion);
    void registerAddMorphKeyframesCommandByAllMorphs();
    void registerAddMorphKeyframesCommandByAllMorphs(Model *model);
    void registerAddMorphKeyframesCommandByActiveMorph(Model *model, nanoem_model_morph_category_t category);
    void registerAddSelfShadowKeyframesCommand(
        const Motion::FrameIndexList &frameIndices, ShadowCamera *shadowCamera, Motion *motion);
    void registerAddSelfShadowKeyframesCommandByCurrentLocalFrameIndex();
    void registerRemoveAccessoryKeyframesCommand(
        const Motion::FrameIndexList &frameIndices, Accessory *accessory, Motion *motion);
    void registerRemoveAccessoryKeyframesCommandByCurrentLocalFrameIndex(Accessory *accessory);
    void registerRemoveBoneKeyframesCommand(const Motion::BoneFrameIndexSetMap &bones, Model *model, Motion *motion);
    void registerRemoveBoneKeyframesCommandByCurrentLocalFrameIndex(Model *model);
    void registerRemoveCameraKeyframesCommand(
        const Motion::FrameIndexList &frameIndices, ICamera *globalCamera, Motion *motion);
    void registerRemoveCameraKeyframesCommandByCurrentLocalFrameIndex();
    void registerRemoveLightKeyframesCommand(
        const Motion::FrameIndexList &frameIndices, ILight *globalLight, Motion *motion);
    void registerRemoveLightKeyframesCommandByCurrentLocalFrameIndex();
    void registerRemoveModelKeyframesCommand(const Motion::FrameIndexList &frameIndices, Model *model, Motion *motion);
    void registerRemoveModelKeyframesCommandByCurrentLocalFrameIndex();
    void registerRemoveModelKeyframesCommandByCurrentLocalFrameIndex(Model *model);
    void registerRemoveMorphKeyframesCommand(const Motion::MorphFrameIndexSetMap &morphs, Model *model, Motion *motion);
    void registerRemoveMorphKeyframesCommandByCurrentLocalFrameIndex(Model *model);
    void registerRemoveSelfShadowKeyframesCommand(
        const Motion::FrameIndexList &frameIndices, ShadowCamera *shadowCamera, Motion *motion);
    void registerRemoveSelfShadowKeyframesCommandByCurrentLocalFrameIndex();
    void registerInsertEmptyTimelineFrameCommand();
    void registerMoveAllSelectedKeyframesCommand(int delta, Error &error);
    void registerRemoveTimelineFrameCommand();
    void registerCorrectAllSelectedBoneKeyframesCommand(Model *model, const Motion::CorrectionVectorFactor &translation,
        const Motion::CorrectionVectorFactor &orientation, Error &error);
    void registerCorrectAllSelectedCameraKeyframesCommand(const Motion::CorrectionVectorFactor &lookAt,
        const Motion::CorrectionVectorFactor &angle, const Motion::CorrectionScalarFactor &distance, Error &error);
    void registerCorrectAllSelectedMorphKeyframesCommand(
        Model *model, const Motion::CorrectionScalarFactor &weight, Error &error);
    void registerScaleAllMotionKeyframesInCommand(
        const TimelineSegment &range, nanoem_f32_t scaleFactor, nanoem_u32_t flags, Error &error);
    void registerRemoveAllSelectedKeyframesCommand();
    void registerRemoveAllSelectedKeyframesCommand(Model *model);
    void registerRemoveAllLipMorphKeyframesCommand();
    void registerRemoveAllLipMorphKeyframesCommand(Model *model);
    void registerRemoveAllEyeMorphKeyframesCommand();
    void registerRemoveAllEyeMorphKeyframesCommand(Model *model);
    void registerRemoveAllEyebrowMorphKeyframesCommand();
    void registerRemoveAllEyebrowMorphKeyframesCommand(Model *model);
    void registerInitializeMotionCommand(Error &error);
    void registerBakeAllModelMotionsCommand(bool enableBakingConstraint, Error &error);

private:
    typedef tinystl::vector<undo_command_t *, TinySTLAllocator> UndoCommandList;

    void internalRegisterRemoveAllMorphKeyframesCommand(Model *model, nanoem_model_morph_category_t category);
    bool internalMoveAllSelectedKeyframes(int delta, UndoCommandList &commands, Error &error);
    bool internalMoveAllSelectedAccessoryKeyframes(
        int delta, Motion::SortDirectionType direction, UndoCommandList &commands, Error &error);
    bool internalMoveAllSelectedCameraKeyframes(
        int delta, Motion::SortDirectionType direction, UndoCommandList &commands, Error &error);
    bool internalMoveAllSelectedLightKeyframes(
        int delta, Motion::SortDirectionType direction, UndoCommandList &commands, Error &error);
    bool internalMoveAllSelectedModelKeyframes(
        int delta, Motion::SortDirectionType direction, UndoCommandList &commands, Error &error);
    bool internalMoveAllSelectedSelfShadowKeyframes(
        int delta, Motion::SortDirectionType direction, UndoCommandList &commands, Error &error);
    bool canRegisterMotionCommand() const NANOEM_DECL_NOEXCEPT;

    Project *m_project;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_BEZIERCURVE_H_ */
