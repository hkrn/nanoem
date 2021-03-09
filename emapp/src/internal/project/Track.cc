/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/project/Track.h"

#include "emapp/Accessory.h"
#include "emapp/DirectionalLight.h"
#include "emapp/ITranslator.h"
#include "emapp/Model.h"
#include "emapp/PerspectiveCamera.h"
#include "emapp/ShadowCamera.h"
#include "emapp/StringUtils.h"
#include "emapp/model/Label.h"
#include "emapp/model/Morph.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace internal {
namespace project {

BaseTrack::BaseTrack(ITrack *parent)
    : m_parent(parent)
    , m_selected(false)
{
}

const ITrack *
BaseTrack::parent() const NANOEM_DECL_NOEXCEPT
{
    return m_parent;
}

ITrack *
BaseTrack::parent() NANOEM_DECL_NOEXCEPT
{
    return m_parent;
}

TrackList
BaseTrack::children() const
{
    return TrackList();
}

bool
BaseTrack::isVisible() const NANOEM_DECL_NOEXCEPT
{
    return true;
}

bool
BaseTrack::isExpandable() const NANOEM_DECL_NOEXCEPT
{
    return false;
}

bool
BaseTrack::isExpanded() const NANOEM_DECL_NOEXCEPT
{
    return false;
}
void
BaseTrack::setExpanded(bool /* value */)
{
}

bool
BaseTrack::isSelected() const NANOEM_DECL_NOEXCEPT
{
    return m_selected;
}

void
BaseTrack::setSelected(bool value)
{
    m_selected = value;
}

bool
BaseTrack::isFixed() const NANOEM_DECL_NOEXCEPT
{
    return false;
}

CameraTrack::CameraTrack(PerspectiveCamera *camera)
    : BaseTrack(nullptr)
    , m_camera(camera)
{
}

CameraTrack::~CameraTrack() NANOEM_DECL_NOEXCEPT
{
}

ITrack::Type
CameraTrack::type() const NANOEM_DECL_NOEXCEPT
{
    return kTypeCamera;
}

const void *
CameraTrack::opaque() const NANOEM_DECL_NOEXCEPT
{
    return m_camera;
}

void *
CameraTrack::opaque() NANOEM_DECL_NOEXCEPT
{
    return m_camera;
}

String
CameraTrack::name() const
{
    return nameConstString();
}

const char *
CameraTrack::nameConstString() const NANOEM_DECL_NOEXCEPT
{
    return m_camera->project()->translator()->translate("nanoem.project.track.camera");
}

bool
CameraTrack::isVisible() const NANOEM_DECL_NOEXCEPT
{
    return false;
}

bool
CameraTrack::isFixed() const NANOEM_DECL_NOEXCEPT
{
    return true;
}

LightTrack::LightTrack(DirectionalLight *light)
    : BaseTrack(nullptr)
    , m_light(light)
{
}

LightTrack::~LightTrack() NANOEM_DECL_NOEXCEPT
{
}

ITrack::Type
LightTrack::type() const NANOEM_DECL_NOEXCEPT
{
    return kTypeLight;
}

const void *
LightTrack::opaque() const NANOEM_DECL_NOEXCEPT
{
    return m_light;
}

void *
LightTrack::opaque() NANOEM_DECL_NOEXCEPT
{
    return m_light;
}

String
LightTrack::name() const
{
    return nameConstString();
}

const char *
LightTrack::nameConstString() const NANOEM_DECL_NOEXCEPT
{
    return m_light->project()->translator()->translate("nanoem.project.track.light");
}

bool
LightTrack::isVisible() const NANOEM_DECL_NOEXCEPT
{
    return false;
}

bool
LightTrack::isFixed() const NANOEM_DECL_NOEXCEPT
{
    return true;
}

SelfShadowTrack::SelfShadowTrack(ShadowCamera *shadowCamera)
    : BaseTrack(nullptr)
    , m_shadowCamera(shadowCamera)
{
}

SelfShadowTrack::~SelfShadowTrack() NANOEM_DECL_NOEXCEPT
{
}

ITrack::Type
SelfShadowTrack::type() const NANOEM_DECL_NOEXCEPT
{
    return kTypeSelfShadow;
}

const void *
SelfShadowTrack::opaque() const NANOEM_DECL_NOEXCEPT
{
    return m_shadowCamera;
}

void *
SelfShadowTrack::opaque() NANOEM_DECL_NOEXCEPT
{
    return m_shadowCamera;
}

String
SelfShadowTrack::name() const
{
    return nameConstString();
}

const char *
SelfShadowTrack::nameConstString() const NANOEM_DECL_NOEXCEPT
{
    return m_shadowCamera->project()->translator()->translate("nanoem.project.track.self-shadow");
}

bool
SelfShadowTrack::isVisible() const NANOEM_DECL_NOEXCEPT
{
    return false;
}

bool
SelfShadowTrack::isFixed() const NANOEM_DECL_NOEXCEPT
{
    return true;
}

AccessoryTrack::AccessoryTrack(Accessory *accessory)
    : BaseTrack(nullptr)
    , m_accessory(accessory)
{
}

AccessoryTrack::~AccessoryTrack() NANOEM_DECL_NOEXCEPT
{
}

ITrack::Type
AccessoryTrack::type() const NANOEM_DECL_NOEXCEPT
{
    return kTypeAccessory;
}

const void *
AccessoryTrack::opaque() const NANOEM_DECL_NOEXCEPT
{
    return m_accessory;
}

void *
AccessoryTrack::opaque() NANOEM_DECL_NOEXCEPT
{
    return m_accessory;
}

String
AccessoryTrack::name() const
{
    return m_accessory->name();
}

const char *
AccessoryTrack::nameConstString() const NANOEM_DECL_NOEXCEPT
{
    return m_accessory->nameConstString();
}

bool
AccessoryTrack::isVisible() const NANOEM_DECL_NOEXCEPT
{
    return true;
}

ModelTrack::ModelTrack(Model *model)
    : BaseTrack(nullptr)
    , m_model(model)
{
}

ModelTrack::~ModelTrack() NANOEM_DECL_NOEXCEPT
{
}

ITrack::Type
ModelTrack::type() const NANOEM_DECL_NOEXCEPT
{
    return kTypeModelRoot;
}

const void *
ModelTrack::opaque() const NANOEM_DECL_NOEXCEPT
{
    return m_model;
}

void *
ModelTrack::opaque() NANOEM_DECL_NOEXCEPT
{
    return m_model;
}

String
ModelTrack::name() const
{
    return nameConstString();
}

const char *
ModelTrack::nameConstString() const NANOEM_DECL_NOEXCEPT
{
    return m_model->project()->translator()->translate("nanoem.project.track.model");
}

bool
ModelTrack::isFixed() const NANOEM_DECL_NOEXCEPT
{
    return true;
}

BoneTrack::BoneTrack(const nanoem_model_bone_t *bone, ModelLabeledTrack *parent)
    : BaseTrack(parent)
    , m_bone(bone)
{
}

BoneTrack::~BoneTrack() NANOEM_DECL_NOEXCEPT
{
}

ITrack::Type
BoneTrack::type() const NANOEM_DECL_NOEXCEPT
{
    return kTypeModelBone;
}

const void *
BoneTrack::opaque() const NANOEM_DECL_NOEXCEPT
{
    return m_bone;
}

void *
BoneTrack::opaque() NANOEM_DECL_NOEXCEPT
{
    return nullptr;
}

String
BoneTrack::name() const
{
    return nameConstString();
}

const char *
BoneTrack::nameConstString() const NANOEM_DECL_NOEXCEPT
{
    return m_bone ? model::Bone::cast(m_bone)->nameConstString() : nullptr;
}

MorphTrack::MorphTrack(const nanoem_model_morph_t *morph, ModelLabeledTrack *parent)
    : BaseTrack(parent)
    , m_morph(morph)
{
}

MorphTrack::~MorphTrack() NANOEM_DECL_NOEXCEPT
{
}

ITrack::Type
MorphTrack::type() const NANOEM_DECL_NOEXCEPT
{
    return kTypeModelMorph;
}

const void *
MorphTrack::opaque() const NANOEM_DECL_NOEXCEPT
{
    return m_morph;
}

void *
MorphTrack::opaque() NANOEM_DECL_NOEXCEPT
{
    return nullptr;
}

String
MorphTrack::name() const
{
    return nameConstString();
}

const char *
MorphTrack::nameConstString() const NANOEM_DECL_NOEXCEPT
{
    return m_morph ? model::Morph::cast(m_morph)->nameConstString() : nullptr;
}

ModelLabeledTrack::ModelLabeledTrack(const nanoem_model_label_t *label)
    : BaseTrack(nullptr)
    , m_label(label)
    , m_name(model::Label::cast(label)->name())
    , m_visible(true)
    , m_expandable(true)
    , m_expanded(false)
{
    nanoem_rsize_t numItems;
    nanoem_model_label_item_t *const *items = nanoemModelLabelGetAllItemObjects(label, &numItems);
    if (nanoemModelLabelIsSpecial(label) != 0 && StringUtils::equalsIgnoreCase(m_name.c_str(), "Root")) {
        m_visible = m_expandable = false;
        if (numItems > 0) {
            const nanoem_model_label_item_t *item = items[0];
            if (const model::Bone *bone = model::Bone::cast(nanoemModelLabelItemGetBoneObject(item))) {
                m_name = bone->name();
            }
            else if (const model::Morph *morph = model::Morph::cast(nanoemModelLabelItemGetMorphObject(item))) {
                m_name = morph->name();
            }
        }
    }
    else {
        for (nanoem_rsize_t i = 0; i < numItems; i++) {
            const nanoem_model_label_item_t *item = items[i];
            if (const nanoem_model_bone_t *bone = nanoemModelLabelItemGetBoneObject(item)) {
                m_children.push_back(nanoem_new(BoneTrack(bone, this)));
            }
            else if (const nanoem_model_morph_t *morph = nanoemModelLabelItemGetMorphObject(item)) {
                m_children.push_back(nanoem_new(MorphTrack(morph, this)));
            }
        }
    }
}

ModelLabeledTrack::~ModelLabeledTrack() NANOEM_DECL_NOEXCEPT
{
    for (Project::TrackList::const_iterator it = m_children.begin(), end = m_children.end(); it != end; ++it) {
        nanoem_delete(*it);
    }
}

Project::TrackList
ModelLabeledTrack::children() const
{
    return m_children;
}

ITrack::Type
ModelLabeledTrack::type() const NANOEM_DECL_NOEXCEPT
{
    return kTypeModelLabel;
}

const void *
ModelLabeledTrack::opaque() const NANOEM_DECL_NOEXCEPT
{
    return m_label;
}

void *
ModelLabeledTrack::opaque() NANOEM_DECL_NOEXCEPT
{
    return nullptr;
}

String
ModelLabeledTrack::name() const
{
    return m_name;
}

const char *
ModelLabeledTrack::nameConstString() const NANOEM_DECL_NOEXCEPT
{
    return m_name.c_str();
}

bool
ModelLabeledTrack::isVisible() const NANOEM_DECL_NOEXCEPT
{
    return m_visible;
}

bool
ModelLabeledTrack::isFixed() const NANOEM_DECL_NOEXCEPT
{
    return true;
}

bool
ModelLabeledTrack::isExpandable() const NANOEM_DECL_NOEXCEPT
{
    return m_expandable;
}

bool
ModelLabeledTrack::isExpanded() const NANOEM_DECL_NOEXCEPT
{
    return m_expanded;
}

void
ModelLabeledTrack::setExpanded(bool value)
{
    m_expanded = value;
}

} /* namespace project */
} /* namespace internal */
} /* namespace nanoem */
