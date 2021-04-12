/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_APPLICATIONMENUBUILDER_H_
#define NANOEM_EMAPP_APPLICATIONMENUBUILDER_H_

#include "emapp/ThreadedApplicationClient.h"

namespace nanoem {

class Project;

class ApplicationMenuBuilder : private NonCopyable {
public:
    enum MenuItemType {
        kMenuItemTypeFirstEnum,
        kMenuItemTypeFileFirstEnum,
        kMenuItemTypeFileTitle = kMenuItemTypeFileFirstEnum,
        kMenuItemTypeFileNewProject,
        kMenuItemTypeFileOpenProject,
        kMenuItemTypeFileImportTitle,
        kMenuItemTypeFileImportModel,
        kMenuItemTypeFileImportAccessory,
        kMenuItemTypeFileImportModelPose,
        kMenuItemTypeFileImportMotionTitle,
        kMenuItemTypeFileImportModelMotion,
        kMenuItemTypeFileImportCameraMotion,
        kMenuItemTypeFileImportLightMotion,
        kMenuItemTypeFileImportAudioSource,
        kMenuItemTypeFileImportBackgroundVideo,
        kMenuItemTypeFileSaveProject,
        kMenuItemTypeFileSaveAsProject,
        kMenuItemTypeFileExportTitle,
        kMenuItemTypeFileExportModelPose,
        kMenuItemTypeFileExportModelMotion,
        kMenuItemTypeFileExportCameraMotion,
        kMenuItemTypeFileExportLightMotion,
        kMenuItemTypeFileExportModel,
        kMenuItemTypeFileExportImage,
        kMenuItemTypeFileExportVideo,
        kMenuItemTypeFileExit,
        kMenuItemTypeFileMaxEnum,
        kMenuItemTypeEditFirstEnum,
        kMenuItemTypeEditTitle = kMenuItemTypeEditFirstEnum,
        kMenuItemTypeEditUndo,
        kMenuItemTypeEditRedo,
        kMenuItemTypeEditCut,
        kMenuItemTypeEditCopy,
        kMenuItemTypeEditPaste,
        kMenuItemTypeEditBoneFirstEnum,
        kMenuItemTypeEditBoneTitle = kMenuItemTypeEditBoneFirstEnum,
        kMenuItemTypeEditBoneOpenParameterDialog,
        kMenuItemTypeEditBoneOpenCorrectionDialog,
        kMenuItemTypeEditBoneOpenBiasDialog,
        kMenuItemTypeEditBoneResetAngle,
        kMenuItemTypeEditBoneMaxEnum,
        kMenuItemTypeEditCameraFirstEnum,
        kMenuItemTypeEditCameraTitle = kMenuItemTypeEditCameraFirstEnum,
        kMenuItemTypeEditCameraOpenParameterDialog,
        kMenuItemTypeEditCameraOpenCorrectionDialog,
        kMenuItemTypeEditCameraResetAngle,
        kMenuItemTypeEditCameraMaxEnum,
        kMenuItemTypeEditMorphFirstEnum,
        kMenuItemTypeEditMorphTitle = kMenuItemTypeEditMorphFirstEnum,
        kMenuItemTypeEditMorphOpenCorrectionDialog,
        kMenuItemTypeEditMorphRemoveAllLipKeyframes,
        kMenuItemTypeEditMorphRemoveAllEyeKeyframes,
        kMenuItemTypeEditMorphRemoveAllEyebrowKeyframes,
        kMenuItemTypeEditMorphResetAllMorphs,
        kMenuItemTypeEditMorphRegisterAllMorphs,
        kMenuItemTypeEditMorphMaxEnum,
        kMenuItemTypeEditModelPluginTitle,
        kMenuItemTypeEditMotionPluginTitle,
        kMenuItemTypeEditOpenEffectParameterWindow,
        kMenuItemTypeEditOpenModelParameterWindow,
        kMenuItemTypeEditSelectAll,
        kMenuItemTypeEditPreference,
        kMenuItemTypeEditMaxEnum,
        kMenuItemTypeProjectFirstEnum,
        kMenuItemTypeProjectTitle = kMenuItemTypeProjectFirstEnum,
        kMenuItemTypeProjectPlay,
        kMenuItemTypeProjectStop,
        kMenuItemTypeProjectOpenViewportDialog,
        kMenuItemTypeProjectOpenDrawOrderDialog,
        kMenuItemTypeProjectOpenTransformOrderDialog,
        kMenuItemTypeProjectEnableLoop,
        kMenuItemTypeProjectEnableGrid,
        kMenuItemTypeProjectEnableGroundShadow,
        kMenuItemTypeProjectEnableEffect,
        kMenuItemTypeProjectEnableHighResolutionViewport,
        kMenuItemTypeProjectEnableComputeShaderSkinning,
        kMenuItemTypeProjectEnableVertexShaderSkinning,
        kMenuItemTypeProjectEnableMSAAx16,
        kMenuItemTypeProjectEnableMSAAx8,
        kMenuItemTypeProjectEnableMSAAx4,
        kMenuItemTypeProjectEnableMSAAx2,
        kMenuItemTypeProjectMSAATitle,
        kMenuItemTypeProjectDisableMSAA,
        kMenuItemTypeProjectPhysicsSimulationTitle,
        kMenuItemTypeProjectPhysicsSimulationEnableAnytime,
        kMenuItemTypeProjectPhysicsSimulationEnableTracing,
        kMenuItemTypeProjectPhysicsSimulationEnablePlaying,
        kMenuItemTypeProjectPhysicsSimulationDisable,
        kMenuItemTypeProjectPhysicsSimulationBakeAllMotions,
        kMenuItemTypeProjectPhysicsSimulationBakeAllMotionsWithIK,
        kMenuItemTypeProjectPhysicsSimulationConfiguration,
        kMenuItemTypeProjectPhysicsSimulationDebugDrawTitle,
        kMenuItemTypeProjectPhysicsSimulationEnableDrawingWireframe,
        kMenuItemTypeProjectPhysicsSimulationEnableDrawingAABB,
        kMenuItemTypeProjectPhysicsSimulationEnableDrawingContactPoints,
        kMenuItemTypeProjectPhysicsSimulationEnableDrawingConstraints,
        kMenuItemTypeProjectPhysicsSimulationEnableDrawingConstraintLimit,
        kMenuItemTypeProjectPreferredFPSTitle,
        kMenuItemTypeProjectPreferredMotionFPSUnlimited,
        kMenuItemTypeProjectPreferredMotionFPS60,
        kMenuItemTypeProjectPreferredMotionFPS30,
        kMenuItemTypeProjectClearAudioSource,
        kMenuItemTypeProjectClearBackgroundVideo,
        kMenuItemTypeProjectEnableFPSCounter,
        kMenuItemTypeProjectEnablePerformanceMonitor,
        kMenuItemTypeProjectMaxEnum,
        kMenuItemTypeCameraFirstEnum,
        kMenuItemTypeCameraTitle = kMenuItemTypeCameraFirstEnum,
        kMenuItemTypeCameraPresetTitle,
        kMenuItemTypeCameraPresetTop,
        kMenuItemTypeCameraPresetLeft,
        kMenuItemTypeCameraPresetRight,
        kMenuItemTypeCameraPresetBottom,
        kMenuItemTypeCameraPresetFront,
        kMenuItemTypeCameraPresetBack,
        kMenuItemTypeCameraRegisterKeyframe,
        kMenuItemTypeCameraRemoveAllSelectedKeyframes,
        kMenuItemTypeCameraReset,
        kMenuItemTypeCameraMaxEnum,
        kMenuItemTypeLightFirstEnum,
        kMenuItemTypeLightTitle = kMenuItemTypeLightFirstEnum,
        kMenuItemTypeLightSelfShadowTitle,
        kMenuItemTypeLightSelfShadowDisable,
        kMenuItemTypeLightSelfShadowEnableWithMode1,
        kMenuItemTypeLightSelfShadowEnableWithMode2,
        kMenuItemTypeLightSelfShadowRegisterKeyframe,
        kMenuItemTypeLightSelfShadowRemoveAllSelectedKeyframes,
        kMenuItemTypeLightSelfShadowReset,
        kMenuItemTypeLightRegisterKeyframe,
        kMenuItemTypeLightRemoveAllSelectedKeyframes,
        kMenuItemTypeLightReset,
        kMenuItemTypeLightMaxEnum,
        kMenuItemTypeModelFirstEnum,
        kMenuItemTypeModelTitle = kMenuItemTypeModelFirstEnum,
        kMenuItemTypeModelSelectTitle,
        kMenuItemTypeModelSelectBoneTitle,
        kMenuItemTypeModelSelectMorphTitle,
        kMenuItemTypeModelSelectMorphEyeTitle,
        kMenuItemTypeModelSelectMorphLipTitle,
        kMenuItemTypeModelSelectMorphEyebrowTitle,
        kMenuItemTypeModelSelectMorphOtherTitle,
        kMenuItemTypeModelSelectAllBoneKeyframesFromSelectedBoneSet,
        kMenuItemTypeModelExpandAllTracks,
        kMenuItemTypeModelCollapseAllTracks,
        kMenuItemTypeModelEditModeSelect,
        kMenuItemTypeModelEditModeRotate,
        kMenuItemTypeModelEditModeMove,
        kMenuItemTypeModelResetBoneAxisX,
        kMenuItemTypeModelResetBoneAxisY,
        kMenuItemTypeModelResetBoneAxisZ,
        kMenuItemTypeModelResetBoneOrientation,
        kMenuItemTypeModelMorphSetZero,
        kMenuItemTypeModelMorphSetHalf,
        kMenuItemTypeModelMorphSetOne,
        kMenuItemTypeModelEnableAddBlend,
        kMenuItemTypeModelEnableShadowMap,
        kMenuItemTypeModelEnableVisible,
        kMenuItemTypeModelRegisterKeyframe,
        kMenuItemTypeModelRemoveAllSelectedKeyframes,
        kMenuItemTypeModelPreferenceTitle,
        kMenuItemTypeModelPreferenceEnableComputeShaderSkinning,
        kMenuItemTypeModelPreferenceEnableShowAllBones,
        kMenuItemTypeModelPreferenceEnableShowRigidBodies,
        kMenuItemTypeModelPreferenceEnableShowJoints,
        kMenuItemTypeModelPreferenceEnableShowVertexFaces,
        kMenuItemTypeModelPreferenceEnableShowVertexPoints,
        kMenuItemTypeModelPreferenceEnableVertexShaderSkinning,
        kMenuItemTypeModelEdgeConfiguraiton,
        kMenuItemTypeModelReset,
        kMenuItemTypeModelDelete,
        kMenuItemTypeModelMaxEnum,
        kMenuItemTypeAccessoryFirstEnum,
        kMenuItemTypeAccessoryTitle = kMenuItemTypeAccessoryFirstEnum,
        kMenuItemTypeAccessorySelectTitle,
        kMenuItemTypeAccessoryEnableAddBlend,
        kMenuItemTypeAccessoryEnableShadow,
        kMenuItemTypeAccessoryEnableVisible,
        kMenuItemTypeAccessoryRegisterKeyframe,
        kMenuItemTypeAccessoryRemoveAllSelectedKeyframes,
        kMenuItemTypeAccessoryReset,
        kMenuItemTypeAccessoryDelete,
        kMenuItemTypeAccessoryMaxEnum,
        kMenuItemTypeMotionFirstEnum,
        kMenuItemTypeMotionTitle = kMenuItemTypeMotionFirstEnum,
        kMenuItemTypeMotionInsertEmptyTimelineFrame,
        kMenuItemTypeMotionRemoveTimelineFrame,
        kMenuItemTypeMotionReset,
        kMenuItemTypeMotionMaxEnum,
        kMenuItemTypeWindowTitle,
        kMenuItemTypeWindowMaximize,
        kMenuItemTypeWindowMinimize,
        kMenuItemTypeWindowRestore,
        kMenuItemTypeWindowFullscreen,
        kMenuItemTypeHelpTitle,
        kMenuItemTypeHelpOnline,
        kMenuItemTypeHelpAbout,
        kMenuItemTypeModelPluginExecute,
        kMenuItemTypeMotionPluginExecute,
        kMenuItemTypeMaxEnum,
    };
    enum MenuItemCheckedState {
        kMenuItemCheckedStateIndetermine = -1,
        kMenuItemCheckedStateFalse,
        kMenuItemCheckedStateTrue,
    };
    static const char *menuItemString(MenuItemType type) NANOEM_DECL_NOEXCEPT;
    static const char *stripMnemonic(char *buffer, size_t size, const char *text);
    static bool validateMenuItem(const Project *project, MenuItemType type, MenuItemCheckedState &state);

    ApplicationMenuBuilder(BaseApplicationClient *client, bool enableModelEditing);
    virtual ~ApplicationMenuBuilder() NANOEM_DECL_NOEXCEPT;

    void build();
    void startModalDialog();
    void stopModalDialog();

protected:
    typedef struct MainMenuBarHandle *MainMenuBarHandle;
    typedef struct MenuBarHandle *MenuBarHandle;
    typedef struct MenuItemHandle *MenuItemHandle;
    typedef tinystl::unordered_map<MenuItemType, MenuItemHandle, TinySTLAllocator> MenuItemMap;
    typedef tinystl::unordered_map<MenuItemType, bool, TinySTLAllocator> EnabledMenuItemMap;

    virtual void createAllMenus() = 0;
    virtual MenuBarHandle createMenuBar() = 0;
    virtual MenuBarHandle createMenuBar(MenuItemType type) = 0;
    virtual MenuItemHandle createMenuItem(MainMenuBarHandle menu) = 0;
    virtual MenuItemHandle appendMenuItem(MenuBarHandle menu, MenuItemType type) = 0;
    virtual void createSelectAccessoryMenuItem(MenuBarHandle menu, nanoem_u16_t handle, const char *name) = 0;
    virtual void createSelectModelMenuItem(MenuBarHandle menu, nanoem_u16_t handle, const char *name) = 0;
    virtual void createSelectBoneMenuItem(MenuBarHandle menu, const char *name) = 0;
    virtual void createSelectMorphMenuItem(
        MenuBarHandle menu, nanoem_model_morph_category_t category, const char *name) = 0;
    virtual void createPluginMenuItem(
        MenuBarHandle menu, MenuItemType type, nanoem_u16_t handle, const String &name, const StringList &items) = 0;
    virtual void updateAllSelectDrawableItems(MenuBarHandle menu, nanoem_u16_t handle) = 0;
    virtual void removeMenuItemById(MenuBarHandle menu, int index) = 0;

    virtual void appendMenuSeparator(MenuBarHandle menu) = 0;
    virtual void clearAllMenuItems(MenuBarHandle menu) = 0;
    virtual void setParentMenu(MenuItemHandle parent, MenuBarHandle menu) = 0;
    virtual void setMenuItemEnabled(MenuItemHandle item, bool value) = 0;
    virtual void setMenuItemChecked(MenuItemHandle item, bool value) = 0;
    virtual bool isMenuItemEnabled(MenuItemHandle item) const NANOEM_DECL_NOEXCEPT = 0;
    virtual bool isMenuItemChecked(MenuItemHandle item) const NANOEM_DECL_NOEXCEPT = 0;

    void initialize();
    void togglePlaying(bool value);
    void updateActiveModelMenu(nanoem_u16_t handle);
    void createFileMenu(MainMenuBarHandle mainMenu);
    void createEditMenu(MainMenuBarHandle bar);
    void createEditMotionMenu(MenuBarHandle editMenu);
    void createEditBoneMenu(MenuBarHandle editMenu);
    void createEditCameraMenu(MenuBarHandle editMenu);
    void createEditMorphMenu(MenuBarHandle editMenu);
    void createEditModelPluginMenu(MenuBarHandle editMenu);
    void createEditMotionPluginMenu(MenuBarHandle editMenu);
    void createProjectMSAAMenu(MenuBarHandle projectMenu);
    void createProjectPhysicsSimulationMenu(MenuBarHandle projectMenu);
    void createProjectPreferredMotionFPSMenu(MenuBarHandle projectMenu);
    void createProjectPhysicsSimulationDebugDrawMenu(MenuBarHandle physicsSimulationMenu);
    void createCameraMenu(MainMenuBarHandle bar);
    void createLightMenu(MainMenuBarHandle bar);
    void createModelMenu(MainMenuBarHandle bar);
    void createModelBoneMenu(MenuBarHandle modelMenu);
    void createModelMorphMenu(MenuBarHandle modelMenu);
    void createModelPreferenceMenu(MenuBarHandle modelMenu);
    void createAccessoryMenu(MainMenuBarHandle bar);
    void createProjectMenu(MainMenuBarHandle bar);
    void setMenuItemEnable(MenuItemType first, MenuItemType last, bool value);
    bool isMenuItemEnabled(MenuItemType type) const;
    void setMenuItemEnable(MenuItemType type, bool value);
    bool isMenuItemChecked(MenuItemType type) const;
    void setMenuItemChecked(MenuItemType type, bool value);
    void setAllAccessoryMenuItemsEnabled(bool value);
    void setAllModelMenuItemsEnabled(bool value);

    BaseApplicationClient *m_client;
    MenuItemMap m_menuItems;
    EnabledMenuItemMap m_wasEnabledItemMap;
    MenuBarHandle m_fileMenu;
    MenuBarHandle m_importMenu;
    MenuBarHandle m_exportMenu;
    MenuBarHandle m_editMenu;
    MenuBarHandle m_editBoneMenu;
    MenuBarHandle m_editCameraMenu;
    MenuBarHandle m_editMorphMenu;
    MenuBarHandle m_editModelPluginMenu;
    MenuBarHandle m_editMotionPluginMenu;
    MenuBarHandle m_projectMenu;
    MenuBarHandle m_msaaMenu;
    MenuBarHandle m_physicsSimulationMenu;
    MenuBarHandle m_physicsSimulationDebugDrawMenu;
    MenuBarHandle m_preferredMotionFPSMenu;
    MenuBarHandle m_cameraMenu;
    MenuBarHandle m_cameraPresetMenu;
    MenuBarHandle m_lightMenu;
    MenuBarHandle m_selfShadowMenu;
    MenuBarHandle m_modelMenu;
    MenuBarHandle m_modelPreferenceMenu;
    MenuBarHandle m_selectModelMenu;
    MenuBarHandle m_selectBoneMenu;
    MenuBarHandle m_selectMorphMenu[NANOEM_MODEL_MORPH_CATEGORY_MAX_ENUM];
    MenuBarHandle m_accessoryMenu;
    MenuBarHandle m_motionMenu;
    MenuBarHandle m_selectAccessoryMenu;
    MenuBarHandle m_windowMenu;
    MenuBarHandle m_helpMenu;

private:
    static MenuItemCheckedState convertCheckedState(bool value);
    static void handleUndoEvent(void *userData, bool canUndo, bool canRedo);
    static void handleRedoEvent(void *userData, bool canRedo, bool canUndo);
    static void handleUndoChangeEvent(void *userData);
    static void handleAddAccessoryEvent(void *userData, nanoem_u16_t handle, const char *name);
    static void handleAddModelEvent(void *userData, nanoem_u16_t handle, const char *name);
    static void handleSetActiveAccessoryEvent(void *userData, nanoem_u16_t handle, const char *name);
    static void handleSetActiveModelEvent(void *userData, nanoem_u16_t handle, const char *name);
    static void handleRemoveAccessoryEvent(void *userData, nanoem_u16_t handle, const char *name);
    static void handleRemoveModelEvent(void *userData, nanoem_u16_t handle, const char *name);
    static void handlePlayEvent(void *userData, nanoem_frame_index_t duration, nanoem_frame_index_t localFrameIndex);
    static void handleStopEvent(void *userData, nanoem_frame_index_t duration, nanoem_frame_index_t localFrameIndex);
    static void handlePauseEvent(void *userData, nanoem_frame_index_t duration, nanoem_frame_index_t localFrameIndex);
    static void handleResumeEvent(void *userData, nanoem_frame_index_t duration, nanoem_frame_index_t localFrameIndex);
    static void handleActiveModelAddBlendEnabledEvent(void *userData, bool value);
    static void handleToggleActiveModelShadowMapEnabled(void *userData, bool value);
    static void handleToggleActiveModelShowAllBones(void *userData, bool value);
    static void handleToggleActiveModelShowAllRigidBodiesEvent(void *userData, bool value);
    static void handleToggleActiveModelShowAllVertexFacesEvent(void *userData, bool value);
    static void handleToggleActiveModelShowAllVertexPointsEvent(void *userData, bool value);
    static void handleToggleActiveModelVisibleEvent(void *userData, bool value);
    static void handleToggleGridEnabledEvent(void *userData, bool value);
    static void handleToggleProjectEffectEnabledEvent(void *userData, bool value);
    static void handleToggleProjectGroundShadowEnabledEvent(void *userData, bool value);
    static void handleSetPhysicsSimulationModeEvent(void *userData, nanoem_u32_t value);
    static void handleSetPreferredMotionFPSEvent(void *userData, nanoem_u32_t value, bool unlimited);
    static void handleSetProjectSampleLevelEvent(void *userData, nanoem_u32_t value);
    static void handleSetShadowMapModeEvent(void *userData, nanoem_u32_t value);
    static void handleAddModalDialogEvent(void *userData);
    static void handleClearModalDialogEvent(void *userData);
    static void handleSetLanguageEvent(void *userData, nanoem_u32_t value);
    static void handleToggleProjectPlayingWithLoopEvent(void *userData, bool value);
    static void handleToggleActiveAccessoryAddBlendEnabledEvent(void *userData, bool value);
    static void handleToggleActiveAccessoryShadowEnabledEvent(void *userData, bool value);
    static void handleToggleActiveAccessoryVisibleEvent(void *userData, bool value);
    static void handleGetAllModelBonesRequest(void *userData, nanoem_u16_t handle, const StringList &values);
    static void handleGetAllModelMorphsRequest(void *userData, nanoem_u16_t handle, const StringList &eyes,
        const StringList &eyebrows, const StringList &lips, const StringList &others);
    static void handleSetupProjectEvent(void *userData, const Vector2 &windowSize, nanoem_f32_t windowDevicePixelRatio,
        nanoem_f32_t viewportDevicePixelRatio);
    static void handleSetEditingModeEvent(void *userData, nanoem_u32_t value);
    static void handleShadowMapModeEvent(void *userData, nanoem_u32_t value);
    static void rebuildAllAccessoryNames(void *userData, const ThreadedApplicationClient::DrawableItemList &items);
    static void rebuildAllModelNames(void *userData, const ThreadedApplicationClient::DrawableItemList &items);
    static void handleCompleteLoadingAllModelPluginsEvent(
        void *userData, const ThreadedApplicationClient::PluginItemList &items);
    static void handleCompleteLoadingAllMotionPluginsEvent(
        void *userData, const ThreadedApplicationClient::PluginItemList &items);
    static void handleCanCopyEvent(void *userData, bool value);
    static void handleCanPasteEvent(void *userData, bool value);
    static void handleSetWindowPixelRatioEvent(void *userData, nanoem_f32_t value);
    static void handleSetViewportPixelRatioEvent(void *userData, nanoem_f32_t value);
    static void handleToggleModelEditingEnabledEvent(void *userData, bool value);

    void addMorphMenuItems(const StringList &morphs, nanoem_model_morph_category_t category);
    void togglAllMutatingActionMenuItems(bool value);

    tinystl::pair<nanoem_u16_t, bool> m_lazyActiveAccessoryHandle;
    tinystl::pair<nanoem_u16_t, bool> m_lazyActiveModelHandle;
    bool m_enableModelEditing;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_APPLICATIONMENUBUILDER_H_ */
