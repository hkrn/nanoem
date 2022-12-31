/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_PROJECT_TRACK_H_
#define NANOEM_EMAPP_INTERNAL_PROJECT_TRACK_H_

#include "emapp/ITrack.h"
#include "emapp/Project.h"

namespace nanoem {
namespace internal {
namespace project {

class BaseTrack : public ITrack, private NonCopyable {
public:
    BaseTrack(ITrack *parent);

    const ITrack *parent() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    ITrack *parent() NANOEM_DECL_NOEXCEPT_OVERRIDE;
    TrackList children() const NANOEM_DECL_OVERRIDE;
    bool isVisible() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool isExpandable() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool isExpanded() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setExpanded(bool value) NANOEM_DECL_OVERRIDE;
    bool isSelected() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setSelected(bool value) NANOEM_DECL_OVERRIDE;
    bool isFixed() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    ITrack *m_parent;
    bool m_selected;
};

class CameraTrack NANOEM_DECL_SEALED : public BaseTrack {
public:
    CameraTrack(PerspectiveCamera *camera);
    ~CameraTrack() NANOEM_DECL_NOEXCEPT;

    Type type() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    const void *opaque() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void *opaque() NANOEM_DECL_NOEXCEPT_OVERRIDE;
    String name() const NANOEM_DECL_OVERRIDE;
    const char *nameConstString() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool isVisible() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool isFixed() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    PerspectiveCamera *m_camera;
};

class LightTrack NANOEM_DECL_SEALED : public BaseTrack {
public:
    LightTrack(DirectionalLight *light);
    ~LightTrack() NANOEM_DECL_NOEXCEPT;

    Type type() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    const void *opaque() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void *opaque() NANOEM_DECL_NOEXCEPT_OVERRIDE;
    String name() const NANOEM_DECL_OVERRIDE;
    const char *nameConstString() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool isVisible() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool isFixed() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    DirectionalLight *m_light;
};

class SelfShadowTrack NANOEM_DECL_SEALED : public BaseTrack {
public:
    SelfShadowTrack(ShadowCamera *shadowCamera);
    ~SelfShadowTrack() NANOEM_DECL_NOEXCEPT;

    Type type() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    const void *opaque() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void *opaque() NANOEM_DECL_NOEXCEPT_OVERRIDE;
    String name() const NANOEM_DECL_OVERRIDE;
    const char *nameConstString() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool isVisible() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool isFixed() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    ShadowCamera *m_shadowCamera;
};

class AccessoryTrack NANOEM_DECL_SEALED : public BaseTrack {
public:
    AccessoryTrack(Accessory *accessory);
    ~AccessoryTrack() NANOEM_DECL_NOEXCEPT;

    Type type() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    const void *opaque() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void *opaque() NANOEM_DECL_NOEXCEPT_OVERRIDE;
    String name() const NANOEM_DECL_OVERRIDE;
    const char *nameConstString() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool isVisible() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    Accessory *m_accessory;
};

class ModelTrack NANOEM_DECL_SEALED : public BaseTrack {
public:
    ModelTrack(Model *model);
    ~ModelTrack() NANOEM_DECL_NOEXCEPT;

    Type type() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    const void *opaque() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void *opaque() NANOEM_DECL_NOEXCEPT_OVERRIDE;
    String name() const NANOEM_DECL_OVERRIDE;
    const char *nameConstString() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool isFixed() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    Model *m_model;
};

class ModelLabeledTrack;

class BoneTrack NANOEM_DECL_SEALED : public BaseTrack {
public:
    BoneTrack(const nanoem_model_bone_t *bone, ModelLabeledTrack *parent);
    ~BoneTrack() NANOEM_DECL_NOEXCEPT;

    Type type() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    const void *opaque() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void *opaque() NANOEM_DECL_NOEXCEPT_OVERRIDE;
    String name() const NANOEM_DECL_OVERRIDE;
    const char *nameConstString() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    const nanoem_model_bone_t *m_bone;
};

class MorphTrack NANOEM_DECL_SEALED : public BaseTrack {
public:
    MorphTrack(const nanoem_model_morph_t *morph, ModelLabeledTrack *parent);
    ~MorphTrack() NANOEM_DECL_NOEXCEPT;

    Type type() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    const void *opaque() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void *opaque() NANOEM_DECL_NOEXCEPT_OVERRIDE;
    String name() const NANOEM_DECL_OVERRIDE;
    const char *nameConstString() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    const nanoem_model_morph_t *m_morph;
};

class ModelLabeledTrack NANOEM_DECL_SEALED : public BaseTrack {
public:
    ModelLabeledTrack(const nanoem_model_label_t *label);
    ~ModelLabeledTrack() NANOEM_DECL_NOEXCEPT;

    TrackList children() const NANOEM_DECL_OVERRIDE;
    Type type() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    const void *opaque() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void *opaque() NANOEM_DECL_NOEXCEPT_OVERRIDE;
    String name() const NANOEM_DECL_OVERRIDE;
    const char *nameConstString() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool isVisible() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool isFixed() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool isExpandable() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool isExpanded() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setExpanded(bool value) NANOEM_DECL_OVERRIDE;

private:
    const nanoem_model_label_t *m_label;
    TrackList m_children;
    String m_name;
    bool m_visible;
    bool m_expandable;
    bool m_expanded;
};

} /* namespace project */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_PROJECT_TRACK_H_ */
