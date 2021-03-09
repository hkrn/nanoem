/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_EFFECTPARAMETERDIALOG_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_EFFECTPARAMETERDIALOG_H_

#include "emapp/StringUtils.h"
#include "emapp/effect/Common.h"
#include "emapp/internal/imgui/BaseNonModalDialogWindow.h"

namespace nanoem {
namespace internal {
namespace imgui {

struct EffectParameterDialog : BaseNonModalDialogWindow {
    static const char *const kIdentifier;
    static const nanoem_f32_t kMinimumWindowWidth;
    typedef tinystl::vector<const String *, TinySTLAllocator> StringPtrList;
    struct StringPtrListSort {
        static int
        sort(const void *left, const void *right)
        {
            const String *l = *static_cast<String *const *>(left);
            const String *r = *static_cast<String *const *>(right);
            return StringUtils::compare(l->c_str(), r->c_str());
        }
        void
        execute(StringPtrList &names)
        {
            qsort(names.data(), names.size(), sizeof(names[0]), sort);
        }
    };

    static void handleLoadingModelEffectSetting(const URI &fileURI, Project *project, void *userData);
    static void handleSaveingModelEffectSetting(const URI &fileURI, Project *project, void *userData);

    EffectParameterDialog(BaseApplicationService *applicationPtr, ImGuiWindow *parent);

    bool draw(Project *project);
    void layoutAllOffscreenRenderTargets(Project *project);
    void layoutOffscreenMainRenderTargetAttachments(Project *project, nanoem_f32_t maxTextWidth);
    void layoutAllOffscreenRenderTargetAttachments(Project *project, const Effect *ownerEffect,
        const effect::OffscreenRenderTargetOption &option, nanoem_f32_t maxTextWidth);
    void layoutDefaultOffscreenRenderTargetAttachment(Project *project, const String &offscreenOwnerName);
    void layoutOffscreenRenderTargetAttachment(
        Project *project, IDrawable *drawable, const String &offscreenOwnerName, nanoem_f32_t maxTextWidth);
    void layoutAllModelMaterialEffectAttachments(Project *project);
    void layoutModelMaterialAttachment(Model *model, Project *project);
    void layoutAllParameters(Project *project, Effect *effect, bool &needsReload);
    void layoutRenderTargetImage(sg_image handle, const Vector2UI16 &size, const String &name, const String &desc);
    void layoutParameterEditor(const effect::UIWidgetParameter *it);
    void layoutUniformBufferDetail(const String &name, const ByteArray &bytes);
    void queryFileDialog(IFileManager::DialogType type, Project *project);

    ImGuiWindow *m_parent;
    int m_activeOffscreenRenderTargetIndex;
    int m_activeModelTargetIndex;
    int m_activeParameterIndex;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_EFFECTPARAMETERDIALOG_H_ */
