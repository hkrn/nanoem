/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_IMODALDIALOG_H_
#define NANOEM_EMAPP_IMODALDIALOG_H_

#include "emapp/Forward.h"

namespace nanoem {

class Project;

class IModalDialog {
public:
    enum ButtonType {
        kButtonTypeFirstEnum = 1 << 0,
        kButtonTypeOk = kButtonTypeFirstEnum,
        kButtonTypeCancel = 1 << 1,
        kButtonTypeSave = 1 << 2,
        kButtonTypeDiscard = 1 << 3,
        kButtonTypeMaxEnum
    };
    virtual ~IModalDialog() NANOEM_DECL_NOEXCEPT
    {
    }
    virtual const char *title() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void draw(const Project *project) = 0;
    virtual Vector4 desiredWindowSize(
        const Vector2UI16 &devicePixelWindowSize, nanoem_f32_t devicePxielRatio) const NANOEM_DECL_NOEXCEPT = 0;
    virtual nanoem_u32_t buttons() const NANOEM_DECL_NOEXCEPT = 0;
    virtual bool isButtonEnabled(nanoem_u32_t value) const NANOEM_DECL_NOEXCEPT = 0;
    virtual bool isCancelled() const NANOEM_DECL_NOEXCEPT = 0;
    virtual IModalDialog *onAccepted(Project *project) = 0;
    virtual IModalDialog *onCancelled(Project *project) = 0;
    virtual IModalDialog *onSaved(Project *project) = 0;
    virtual IModalDialog *onDiscarded(Project *project) = 0;
    virtual void setProgress(nanoem_f32_t value) = 0;
    virtual void setRowHeight(nanoem_f32_t value) = 0;
    virtual void setTitle(const char *value) = 0;
    virtual void setText(const char *value) = 0;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_IMODALDIALOG_H_ */
