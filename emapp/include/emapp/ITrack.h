/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_ITRACK_H_
#define NANOEM_EMAPP_ITRACK_H_

#include "emapp/Forward.h"

namespace nanoem {

class ITrack;
typedef tinystl::vector<ITrack *, TinySTLAllocator> TrackList;

class ITrack {
public:
    enum Type {
        kTypeFirstEnum,
        kTypeCamera = kTypeFirstEnum,
        kTypeLight,
        kTypeSelfShadow,
        kTypeGravity,
        kTypeAccessory,
        kTypeModelRoot,
        kTypeModelLabel,
        kTypeModelBone,
        kTypeModelMorph,
        kTypeMaxEnum
    };
    virtual ~ITrack() NANOEM_DECL_NOEXCEPT
    {
    }
    virtual const ITrack *parent() const NANOEM_DECL_NOEXCEPT = 0;
    virtual ITrack *parent() NANOEM_DECL_NOEXCEPT = 0;
    virtual TrackList children() const = 0;
    virtual Type type() const NANOEM_DECL_NOEXCEPT = 0;
    virtual const void *opaque() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void *opaque() NANOEM_DECL_NOEXCEPT = 0;
    virtual String name() const = 0;
    virtual const char *nameConstString() const NANOEM_DECL_NOEXCEPT = 0;
    virtual bool isVisible() const NANOEM_DECL_NOEXCEPT = 0;
    virtual bool isExpandable() const NANOEM_DECL_NOEXCEPT = 0;
    virtual bool isExpanded() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setExpanded(bool value) = 0;
    virtual bool isSelected() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setSelected(bool value) = 0;
    virtual bool isFixed() const NANOEM_DECL_NOEXCEPT = 0;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_ITRACK_H_ */
