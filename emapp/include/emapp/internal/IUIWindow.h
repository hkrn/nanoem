/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#ifndef NANOEM_EMAPP_INTERNAL_IUIWINDOW
#define NANOEM_EMAPP_INTERNAL_IUIWINDOW

#include "emapp/BaseApplicationService.h"

namespace nanoem {

class Accessory;
class Model;
class IModalDialog;
class IPrimitive2D;
class IState;
class Project;

namespace plugin {
class ModelIOPlugin;
class MotionIOPlugin;
}

namespace internal {

class IUIWindow {
public:
    virtual ~IUIWindow() NANOEM_DECL_NOEXCEPT
    {
    }

    virtual void initialize(nanoem_f32_t devicePixelRatio) = 0;
    virtual void reset() = 0;
    virtual void destroy() NANOEM_DECL_NOEXCEPT = 0;
    virtual void setFontPointSize(nanoem_f32_t pointSize) = 0;
    virtual void setKeyPressed(BaseApplicationService::KeyType key) = 0;
    virtual void setKeyReleased(BaseApplicationService::KeyType key) = 0;
    virtual void setUnicodePressed(nanoem_u32_t key) = 0;
    virtual void setCurrentCPUPercentage(nanoem_f32_t value) = 0;
    virtual void setCurrentMemoryBytes(nanoem_u64_t value) = 0;
    virtual void setMaxMemoryBytes(nanoem_u64_t value) = 0;
    virtual void setScreenCursorPress(int type, int x, int y, int modifiers) = 0;
    virtual void setScreenCursorMove(int x, int y, int modifiers) = 0;
    virtual void setScreenCursorRelease(int type, int x, int y, int modifiers) = 0;

    virtual void openBoneParametersDialog(Project *project) = 0;
    virtual void openBoneCorrectionDialog(Project *project) = 0;
    virtual void openBoneBiasDialog(Project *project) = 0;
    virtual void openCameraParametersDialog(Project *project) = 0;
    virtual void openCameraCorrectionDialog(Project *project) = 0;
    virtual void openMorphCorrectionDialog(Project *project) = 0;
    virtual void openViewportDialog(Project *project) = 0;
    virtual void openModelDrawOrderDialog(Project *project) = 0;
    virtual void openModelTransformOrderDialog(Project *project) = 0;
    virtual void openPhysicsEngineDialog(Project *project) = 0;
    virtual void openModelAllObjectsDialog() = 0;
    virtual void openModelEdgeDialog(Project *project) = 0;
    virtual void openScaleAllSelectedKeyframesDialog() = 0;
    virtual void openEffectParameterDialog(Project *project) = 0;
    virtual void openModelParameterDialog(Project *project) = 0;
    virtual void openPreferenceDialog() = 0;
    virtual bool openModelIOPluginDialog(Project *project, plugin::ModelIOPlugin *plugin, const ByteArray &input,
        const ByteArray &layout, int functionIndex) = 0;
    virtual bool openMotionIOPluginDialog(Project *project, plugin::MotionIOPlugin *plugin, const ByteArray &input,
        const ByteArray &layout, int functionIndex) = 0;

    virtual void resizeDevicePixelWindowSize(const Vector2UI16 &value) = 0;
    virtual void setDevicePixelRatio(float value) = 0;
    virtual void drawAll2DPrimitives(Project *project, Project::IViewportOverlay *overlay, nanoem_u32_t flags) = 0;
    virtual void drawAllWindows(Project *project, const IState *state, nanoem_u32_t flags) = 0;

    virtual const IModalDialog *currentModalDialog() const NANOEM_DECL_NOEXCEPT = 0;
    virtual IModalDialog *currentModalDialog() NANOEM_DECL_NOEXCEPT = 0;
    virtual void addModalDialog(IModalDialog *value) = 0;
    virtual void clearAllModalDialogs() = 0;
    virtual bool hasModalDialog() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void swapModalDialog(IModalDialog *value) = 0;
    virtual bool isVisible() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setVisible(bool value) = 0;

    virtual IPrimitive2D *primitiveContext() NANOEM_DECL_NOEXCEPT = 0;
};

} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IUIWINDOW */
