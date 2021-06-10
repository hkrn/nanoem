/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_STATECONTROLLER_H_
#define NANOEM_EMAPP_STATECONTROLLER_H_

#include "emapp/Forward.h"
#include "emapp/IProjectHolder.h"
#include "emapp/ModalDialogFactory.h"
#include "emapp/Project.h"

namespace nanoem {

class BaseApplicationService;
class IFileManager;
class IModelObjectSelection;
class IState;

class StateController NANOEM_DECL_SEALED : public IProjectHolder, private NonCopyable {
public:
    StateController(BaseApplicationService *application, IFileManager *delegate);
    ~StateController() NANOEM_DECL_NOEXCEPT;

    const Project *currentProject() const NANOEM_DECL_OVERRIDE;
    Project *currentProject() NANOEM_DECL_OVERRIDE;

    BaseApplicationService *application() NANOEM_DECL_NOEXCEPT;
    IFileManager *fileManager() NANOEM_DECL_NOEXCEPT;
    const IState *currentState() const NANOEM_DECL_NOEXCEPT;
    IState *currentState() NANOEM_DECL_NOEXCEPT;
    void setCurrentState(IState *state);
    void consumeDefaultPass();

    void handlePointerScroll(const Vector3SI32 &logicalCursorPosition, const Vector2SI32 &delta);
    void handlePointerPress(const Vector3SI32 &logicalCursorPosition, Project::CursorType type);
    void handlePointerMove(const Vector3SI32 &logicalCursorPosition, const Vector2SI32 &delta);
    void handlePointerRelease(const Vector3SI32 &logicalCursorPosition, Project::CursorType type);

    Project *createProject();
    void newProject();
    void newProject(const Vector2UI16 &windowSize, const char *dllPath, sg_pixel_format pixelFormat,
        nanoem_f32_t windowDevicePixelRatio, nanoem_f32_t viewportDevicePixelRatio, nanoem_u32_t fps);
    void setProject(Project *newProject);
    void toggleEditingMode(Project::EditingMode value);

    nanoem_u64_t globalFrameIndex() const NANOEM_DECL_NOEXCEPT;
    nanoem_rsize_t boneIndex() const NANOEM_DECL_NOEXCEPT;
    void setBoneIndex(nanoem_rsize_t value);

private:
    bool intersectsViewportLayoutRect(
        const Project *project, const Vector2SI32 &logicalCursorPosition) const NANOEM_DECL_NOEXCEPT;
    void setPrimaryDraggingState(Project *project, const Vector2SI32 &logicalCursorPosition);
    void setSecondaryDraggingState(Project *project, const Vector2SI32 &logicalCursorPosition);
    IState *draggingBoneSelectionState(const IModelObjectSelection *selection);
    IState *draggingModelObjectState(const IModelObjectSelection *selection);

    typedef tinystl::vector<Project *, TinySTLAllocator> QueuedProjectList;
    BaseApplicationService *m_applicationPtr;
    IFileManager *m_fileManager;
    IState *m_state;
    tinystl::pair<Project *, QueuedProjectList> m_project;
    Project::EditingMode m_lastEditingMode;
    String m_dllPath;
    nanoem_rsize_t m_boneIndex;
    tinystl::pair<nanoem_u64_t, nanoem_u64_t> m_frameIndex;
    tinystl::pair<nanoem_u64_t, nanoem_u64_t> m_elapsedTime;
};

} /* namespace nanoem */

#endif /* NANOEM_STATECONTROLLER_H_ */
