/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/ModelKeyframeSelector.h"

#include "emapp/IMotionKeyframeSelection.h"
#include "emapp/ITrack.h"
#include "emapp/Model.h"
#include "emapp/Project.h"
#include "emapp/internal/ModelObjectSelection.h"
#include "emapp/private/CommonInclude.h"

#include "imgui/imgui.h"

namespace nanoem {
namespace internal {
namespace imgui {

bool
ModelKeyframeSelector::callback(void *userData, int index, const char **out) NANOEM_DECL_NOEXCEPT
{
    const ModelKeyframeSelector *self = static_cast<const ModelKeyframeSelector *>(userData);
    return self->select(index, out);
}

ModelKeyframeSelector::ModelKeyframeSelector(Project *project)
    : m_rootTrack(nullptr)
    , m_project(project)
{
    const TrackList *tracks = project->allTracks();
    for (Project::TrackList::const_iterator it = tracks->begin(), end = tracks->end(); it != end; ++it) {
        const ITrack *track = *it;
        if (track->isExpandable()) {
            const Project::TrackList children(track->children());
            for (Project::TrackList::const_iterator it2 = children.begin(), end2 = children.end(); it2 != end2; ++it2) {
                const ITrack *child = *it2;
                const ITrack::Type type = child->type();
                if (type == ITrack::kTypeModelBone) {
                    m_bones.push_back(static_cast<const nanoem_model_bone_t *>(child->opaque()));
                }
                else if (type == ITrack::kTypeModelMorph) {
                    m_morphs.push_back(static_cast<const nanoem_model_morph_t *>(child->opaque()));
                }
            }
        }
        else if (track->type() == ITrack::kTypeModelLabel) {
            m_rootTrack = track;
        }
    }
}

bool
ModelKeyframeSelector::combo(int *index)
{
    return ImGui::Combo("##timeline.select.model", index, callback, this, count());
}

void
ModelKeyframeSelector::handleAction(const TimelineSegment &segment, int index)
{
    Model *activeModel = m_project->activeModel();
    Motion *motion = m_project->resolveMotion(activeModel);
    IMotionKeyframeSelection *selection = motion->selection();
    switch (static_cast<SelectType>(index)) {
    case kSelectTypeAll: {
        selectRootTrack(segment, selection);
        selection->addModelKeyframes(segment.m_from, segment.m_to);
        for (model::Bone::List::const_iterator it = m_bones.begin(), end = m_bones.end(); it != end; ++it) {
            const nanoem_model_bone_t *bonePtr = *it;
            selection->addBoneKeyframes(bonePtr, segment.m_from, segment.m_to);
        }
        for (model::Morph::List::const_iterator it = m_morphs.begin(), end = m_morphs.end(); it != end; ++it) {
            const nanoem_model_morph_t *morphPtr = *it;
            selection->addMorphKeyframes(morphPtr, segment.m_from, segment.m_to);
        }
        break;
    }
    case kSelectTypeLabelRoot: {
        selectRootTrack(segment, selection);
        break;
    }
    case kSelectTypeModel: {
        selection->addModelKeyframes(segment.m_from, segment.m_to);
        break;
    }
    case kSelectTypeBones: {
        const model::Bone::Set *boneSet = activeModel->selection()->allBoneSet();
        for (model::Bone::Set::const_iterator it = boneSet->begin(), end = boneSet->end(); it != end; ++it) {
            const nanoem_model_bone_t *bonePtr = *it;
            selection->addBoneKeyframes(bonePtr, segment.m_from, segment.m_to);
        }
        break;
    }
    case kSelectTypeMorphs: {
        break;
    }
    case kSelectTypeAllMorphs: {
        for (model::Morph::List::const_iterator it = m_morphs.begin(), end = m_morphs.end(); it != end; ++it) {
            const nanoem_model_morph_t *morphPtr = *it;
            selection->addMorphKeyframes(morphPtr, segment.m_from, segment.m_to);
        }
        break;
    }
    default:
        if (index < count()) {
            int base = Inline::saturateInt32(m_morphs.size() + kSelectTypeMaxEnum);
            if (!m_bones.empty() && index >= base) {
                const nanoem_model_bone_t *bonePtr = m_bones[index - base];
                selection->addBoneKeyframes(bonePtr, segment.m_from, segment.m_to);
            }
            else if (!m_morphs.empty()) {
                const nanoem_model_morph_t *morphPtr = m_morphs[index - kSelectTypeMaxEnum];
                selection->addMorphKeyframes(morphPtr, segment.m_from, segment.m_to);
            }
        }
        break;
    }
}

bool
ModelKeyframeSelector::select(int index, const char **out) const NANOEM_DECL_NOEXCEPT
{
    const ITranslator *translator = m_project->translator();
    switch (static_cast<SelectType>(index)) {
    case kSelectTypeAll: {
        *out = translator->translate("nanoem.project.track.select-all");
        break;
    }
    case kSelectTypeLabelRoot: {
        *out = m_rootTrack ? m_rootTrack->nameConstString() : translator->translate("nanoem.project.track.none");
        break;
    }
    case kSelectTypeModel: {
        *out = translator->translate("nanoem.project.track.model");
        break;
    }
    case kSelectTypeBones: {
        *out = translator->translate("nanoem.project.track.selected-bones");
        break;
    }
    case kSelectTypeMorphs: {
        *out = translator->translate("nanoem.project.track.selected-morphs");
        break;
    }
    case kSelectTypeAllMorphs: {
        *out = translator->translate("nanoem.project.track.all-morphs");
        break;
    }
    default:
        if (index < count()) {
            int base = Inline::saturateInt32(m_morphs.size() + kSelectTypeMaxEnum);
            if (!m_bones.empty() && index >= base) {
                int offset = index - base;
                if (const model::Bone *bone = model::Bone::cast(m_bones[offset])) {
                    *out = bone->nameConstString();
                }
                else {
                    *out = "(Unknown)";
                }
            }
            else if (!m_morphs.empty()) {
                int offset = index - kSelectTypeMaxEnum;
                if (const model::Morph *morph = model::Morph::cast(m_morphs[offset])) {
                    *out = morph->nameConstString();
                }
                else {
                    *out = "(Unknown)";
                }
            }
        }
        else {
            *out = "(Unknown)";
        }
        break;
    }
    return true;
}

int
ModelKeyframeSelector::count() const NANOEM_DECL_NOEXCEPT
{
    return Inline::saturateInt32(m_bones.size() + m_morphs.size() + kSelectTypeMaxEnum);
}

void
ModelKeyframeSelector::selectRootTrack(const TimelineSegment &segment, IMotionKeyframeSelection *selection)
{
    if (const ITrack *rootTrack = m_rootTrack) {
        const nanoem_model_label_t *labelPtr = static_cast<const nanoem_model_label_t *>(rootTrack->opaque());
        nanoem_rsize_t numLabels;
        nanoem_model_label_item_t *const *items = nanoemModelLabelGetAllItemObjects(labelPtr, &numLabels);
        if (items && numLabels > 0) {
            const nanoem_model_label_item_t *itemPtr = items[0];
            if (const nanoem_model_bone_t *bonePtr = nanoemModelLabelItemGetBoneObject(itemPtr)) {
                selection->addBoneKeyframes(bonePtr, segment.m_from, segment.m_to);
            }
            else if (const nanoem_model_morph_t *morphPtr = nanoemModelLabelItemGetMorphObject(itemPtr)) {
                selection->addMorphKeyframes(morphPtr, segment.m_from, segment.m_to);
            }
        }
    }
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
