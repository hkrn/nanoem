/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_PROJECT_H_
#define NANOEM_EMAPP_PROJECT_H_

#include "emapp/FileUtils.h"
#include "emapp/IDrawable.h"
#include "emapp/IEffect.h"
#include "emapp/ITranslator.h"
#include "emapp/Motion.h"
#include "emapp/PhysicsEngine.h"
#include "emapp/PixelFormat.h"
#include "emapp/TimelineSegment.h"
#include "emapp/effect/OffscreenRenderTargetOption.h"
#include "emapp/model/BindPose.h"
#include "emapp/model/Bone.h"
#include "emapp/model/Morph.h"
#include "emapp/model/RigidBody.h"

#include "nanoem/ext/parson/parson.h"

struct Nanoem__Application__Command;

struct undo_command_t;
struct undo_stack_t;

namespace bx {
class HandleAlloc;
}

namespace nanoem {

class Accessory;
class AccessoryProgramBundle;
class DirectionalLight;
class Effect;
class Grid;
class IAudioPlayer;
class IBackgroundVideoRenderer;
class ICamera;
class ICancelPublisher;
class IDebugCapture;
class IEventPublisher;
class IFileManager;
class IImageView;
class ILight;
class IPrimitive2D;
class ITrack;
class ImageLoader;
class Model;
class ModelProgramBundle;
class PerspectiveCamera;
class Progress;
class ShadowCamera;
class UUID;

namespace effect {
struct GlobalUniform;
class RenderPassScope;
class RenderTargetColorImageContainer;
} /* namespace effect */

namespace model {
class Material;
class ISkinDeformer;
} /* namespace model */

namespace plugin {
class EffectPlugin;
} /* namespace plugin */

namespace internal {
class BlitPass;
class ClearPass;
class DebugDrawer;
} /* namespace internal */

class Project NANOEM_DECL_SEALED : private NonCopyable {
public:
    enum BinaryFormatType {
        kBinaryFormatFirstEnum,
        kBinaryFormatNative = kBinaryFormatFirstEnum,
        kBinaryFormatPMM,
        kBinaryFormatMaxEnum
    };
    enum CursorType {
        kCursorTypeFirstEnum,
        kCursorTypeMouseButton1 = kCursorTypeFirstEnum,
        kCursorTypeMouseButton2,
        kCursorTypeMouseButton3,
        kCursorTypeMouseButton4,
        kCursorTypeMouseButton5,
        kCursorTypeMouseButton6,
        kCursorTypeMouseButton7,
        kCursorTypeMouseButton8,
        kCursorTypeMouseLeft = kCursorTypeMouseButton1,
        kCursorTypeMouseRight = kCursorTypeMouseButton2,
        kCursorTypeMouseMiddle = kCursorTypeMouseButton3,
        kCursorTypeMouseUndo = kCursorTypeMouseButton4,
        kCursorTypeMouseRedo = kCursorTypeMouseButton5,
        kCursorTypeMaxEnum
    };
    enum CursorModifierType {
        kCursorModifierTypeShift = 0x1,
        kCursorModifierTypeControl = 0x2,
        kCursorModifierTypeAlt = 0x4,
        kCursorModifierTypeMaxEnum = 0x8
    };
    enum EditingMode {
        kEditingModeFirstEnum,
        kEditingModeNone = kEditingModeFirstEnum,
        kEditingModeSelect,
        kEditingModeMove,
        kEditingModeRotate,
        kEditingModeMaxEnum
    };
    enum FilePathMode {
        kFilePathModeFirstEnum,
        kFilePathModeAbsolute = kFilePathModeFirstEnum,
        kFilePathModeRelative,
        kFilePathModeMaxEnum,
    };
    enum RectangleType {
        kRectangleTypeFirstEnum,
        kRectangleTranslateX = kRectangleTypeFirstEnum,
        kRectangleTranslateY,
        kRectangleTranslateZ,
        kRectangleOrientateX,
        kRectangleOrientateY,
        kRectangleOrientateZ,
        kRectangleTransformCoordinateType,
        kRectangleCameraLookAt,
        kRectangleCameraZoom,
        kRectangleActualFPS,
        kRectangleTypeMaxEnum,
    };

    struct SaveState;
    struct IConfirmer {
        virtual ~IConfirmer() NANOEM_DECL_NOEXCEPT
        {
        }
        virtual void seek(nanoem_frame_index_t frameIndex, Project *project) = 0;
        virtual void play(Project *project) = 0;
        virtual void resume(Project *project) = 0;
    };
    struct ISkinDeformerFactory {
        virtual ~ISkinDeformerFactory() NANOEM_DECL_NOEXCEPT
        {
        }
        virtual model::ISkinDeformer *create(Model *model) = 0;
        virtual void begin() = 0;
        virtual void commit() = 0;
    };
    struct IRendererCapability {
        virtual ~IRendererCapability() NANOEM_DECL_NOEXCEPT
        {
        }
        virtual nanoem_u32_t suggestedSampleLevel(nanoem_u32_t value) const NANOEM_DECL_NOEXCEPT = 0;
        virtual bool supportsSampleLevel(nanoem_u32_t value) const NANOEM_DECL_NOEXCEPT = 0;
    };
    struct ISharedCancelPublisherRepository {
        virtual ~ISharedCancelPublisherRepository() NANOEM_DECL_NOEXCEPT
        {
        }
        virtual ICancelPublisher *cancelPublisher() = 0;
    };
    struct ISharedDebugCaptureRepository {
        virtual ~ISharedDebugCaptureRepository() NANOEM_DECL_NOEXCEPT
        {
        }
        virtual IDebugCapture *debugCapture() = 0;
    };
    struct ISharedResourceRepository {
        virtual ~ISharedResourceRepository() NANOEM_DECL_NOEXCEPT
        {
        }
        virtual AccessoryProgramBundle *accessoryProgramBundle() = 0;
        virtual ModelProgramBundle *modelProgramBundle() = 0;
        virtual effect::GlobalUniform *effectGlobalUniform() = 0;
        virtual const IImageView *toonImage(int value) = 0;
        virtual Vector4 toonColor(int value) = 0;
    };
    struct IUnicodeStringFactoryRepository {
        virtual ~IUnicodeStringFactoryRepository() NANOEM_DECL_NOEXCEPT
        {
        }
        virtual nanoem_unicode_string_factory_t *unicodeStringFactory() const NANOEM_DECL_NOEXCEPT = 0;
    };
    struct IDiagnostics {
        virtual void addNotFoundFileURI(const URI &fileURI) = 0;
        virtual void addDigestMismatchFileURI(const URI &fileURI) = 0;
    };

    typedef tinystl::pair<nanoem_frame_index_t, int> TransformPerformIndex;
    typedef tinystl::unordered_set<nanoem_rsize_t, TinySTLAllocator> ModelMaterialIndexSet;
    typedef tinystl::pair<nanoem_u16_t, ModelMaterialIndexSet> ModelMaterialIndexSetPair;
    typedef tinystl::vector<IDrawable *, TinySTLAllocator> DrawableList;
    typedef tinystl::vector<ITrack *, TinySTLAllocator> TrackList;
    typedef tinystl::vector<Model *, TinySTLAllocator> ModelList;
    typedef tinystl::vector<Accessory *, TinySTLAllocator> AccessoryList;
    typedef tinystl::vector<Motion *, TinySTLAllocator> MotionList;
    typedef tinystl::vector<Effect *, TinySTLAllocator> EffectList;
    typedef tinystl::unordered_set<Effect *, TinySTLAllocator> LoadedEffectSet;
    typedef tinystl::unordered_map<String, ByteArray, TinySTLAllocator> IncludeEffectSourceMap;
    typedef tinystl::unordered_map<IDrawable *, Motion *, TinySTLAllocator> MotionHashMap;
    typedef tinystl::unordered_map<nanoem_u32_t, String, TinySTLAllocator> SGHandleStringMap;

    static const char *const kRedoLogFileExtension;
    static const char *const kArchivedNativeFormatFileExtension;
    static const char *const kFileSystemBasedNativeFormatFileExtension;
    static const char *const kPolygonMovieMakerFileExtension;
    static const char *const kViewportPrimaryName;
    static const char *const kViewportSecondaryName;
    static const nanoem_frame_index_t kMinimumBaseDuration;
    static const nanoem_frame_index_t kMaximumBaseDuration;
    static const nanoem_f32_t kDefaultCircleRadiusSize;

    static URI resolveArchiveURI(const URI &fileURI, const String &filename);
    static String resolveNameConfliction(const IDrawable *drawable, StringSet &reservedNameSet);
    static String resolveNameConfliction(const String &name, StringSet &reservedNameSet);
    static int countColorAttachments(const sg_pass_desc &pd) NANOEM_DECL_NOEXCEPT;
    static StringList loadableExtensions();
    static StringSet loadableExtensionsSet();
    static bool isArchiveURI(const URI &fileURI);
    static bool isLoadableExtension(const String &extension);
    static bool isLoadableExtension(const URI &fileURI);
    static void setAlphaBlendMode(sg_color_state &state) NANOEM_DECL_NOEXCEPT;
    static void setAddBlendMode(sg_color_state &state) NANOEM_DECL_NOEXCEPT;
    static void setStandardDepthStencilState(sg_depth_state &ds, sg_stencil_state &ss) NANOEM_DECL_NOEXCEPT;
    static void setShadowDepthStencilState(sg_depth_state &ds, sg_stencil_state &ss) NANOEM_DECL_NOEXCEPT;

    struct Injector {
        const IUnicodeStringFactoryRepository *m_unicodeStringFactoryRepository;
        const JSON_Value *m_applicationConfiguration;
        const char *m_dllPath;
        IAudioPlayer *m_audioPlayer;
        IBackgroundVideoRenderer *m_backgroundVideoRenderer;
        IConfirmer *m_confirmerPtr;
        IEventPublisher *m_eventPublisherPtr;
        IFileManager *m_fileManagerPtr;
        ISkinDeformerFactory *m_skinDeformerFactory;
        IPrimitive2D *m_primitive2DPtr;
        IRendererCapability *m_rendererCapability;
        ISharedCancelPublisherRepository *m_sharedCancelPublisherRepositoryPtr;
        ISharedDebugCaptureRepository *m_sharedDebugCaptureRepository;
        ISharedResourceRepository *m_sharedResourceRepositoryPtr;
        ITranslator *m_translatorPtr;
        Vector2UI16 m_windowSize;
        sg_pixel_format m_pixelFormat;
        nanoem_f32_t m_windowDevicePixelRatio;
        nanoem_f32_t m_viewportDevicePixelRatio;
        int m_preferredUndoCount;
    };
    Project(const Injector &injector);
    ~Project() NANOEM_DECL_NOEXCEPT;

    bool initialize(Error &error);
    void destroy() NANOEM_DECL_NOEXCEPT;
    void loadFromJSON(const JSON_Value *value);
    bool loadFromBinary(
        const nanoem_u8_t *data, size_t size, BinaryFormatType format, Error &error, IDiagnostics *diagnostics);
    bool loadFromBinary(const ByteArray &value, BinaryFormatType format, Error &error, IDiagnostics *diagnostics);
    bool loadFromArchive(ISeekableReader *reader, const URI &fileURI, Error &error);
    void saveAsJSON(JSON_Value *value);
    bool saveAsBinary(ByteArray &value, BinaryFormatType format, Error &error);
    bool saveAsBinary(ISeekableWriter *writer, BinaryFormatType format, nanoem_u8_t *checksum, Error &error);
    bool saveAsArchive(ISeekableWriter *writer, Error &error);
    bool loadEffectFromSource(const URI &baseURI, Effect *effect, URI &sourceURI, Progress &progress, Error &error);
    bool loadEffectFromSource(
        const URI &baseURI, Effect *effect, bool enableCache, URI &sourceURI, Progress &progress, Error &error);
    bool loadEffectFromBinary(const URI &fileURI, Effect *effect, Progress &progress, Error &error);

    const Accessory *findAccessoryByURI(const URI &fileURI) const NANOEM_DECL_NOEXCEPT;
    const Accessory *findAccessoryByFilename(const String &name) const NANOEM_DECL_NOEXCEPT;
    const Accessory *findAccessoryByName(const String &name) const NANOEM_DECL_NOEXCEPT;
    const Accessory *findAccessoryByHandle(const nanoem_u16_t handle) const NANOEM_DECL_NOEXCEPT;
    Accessory *findAccessoryByURI(const URI &fileURI) NANOEM_DECL_NOEXCEPT;
    Accessory *findAccessoryByFilename(const String &name) NANOEM_DECL_NOEXCEPT;
    Accessory *findAccessoryByName(const String &name) NANOEM_DECL_NOEXCEPT;
    Accessory *findAccessoryByHandle(const nanoem_u16_t handle) NANOEM_DECL_NOEXCEPT;
    const Model *findModelByURI(const URI &fileURI) const NANOEM_DECL_NOEXCEPT;
    const Model *findModelByFilename(const String &name) const NANOEM_DECL_NOEXCEPT;
    const Model *findModelByName(const String &name) const NANOEM_DECL_NOEXCEPT;
    const Model *findModelByHandle(const nanoem_u16_t handle) const NANOEM_DECL_NOEXCEPT;
    Model *findModelByURI(const URI &fileURI) NANOEM_DECL_NOEXCEPT;
    Model *findModelByFilename(const String &name) NANOEM_DECL_NOEXCEPT;
    Model *findModelByName(const String &name) NANOEM_DECL_NOEXCEPT;
    Model *findModelByHandle(const nanoem_u16_t handle) NANOEM_DECL_NOEXCEPT;
    const Motion *findMotionByHandle(const nanoem_u16_t handle) const NANOEM_DECL_NOEXCEPT;
    Motion *findMotionByHandle(const nanoem_u16_t handle) NANOEM_DECL_NOEXCEPT;
    const Effect *resolveEffect(const IDrawable *drawable) const NANOEM_DECL_NOEXCEPT;
    Effect *resolveEffect(IDrawable *drawable) NANOEM_DECL_NOEXCEPT;
    const nanoem_model_bone_t *resolveBone(const StringPair &value) const NANOEM_DECL_NOEXCEPT;
    bool containsMotion(const Motion *value) const NANOEM_DECL_NOEXCEPT;

    void newModel(Error &error);
    void addModel(Model *model);
    void addAccessory(Accessory *accessory);
    void convertAccessoryToModel(Accessory *accessory, Error &error);
    void performModelSkinDeformer(Model *model);
    bool loadAttachedDrawableEffect(IDrawable *drawable, Progress &progress, Error &error);
    bool reloadAllDrawableEffects(Progress &progress, Error &error);
    bool reloadDrawableEffect(Progress &progress, Error &error);
    bool reloadDrawableEffect(IDrawable *drawable, Progress &progress, Error &error);
    void setCameraMotion(Motion *motion);
    void setLightMotion(Motion *motion);
    void setSelfShadowMotion(Motion *motion);
    Motion *addAccessoryMotion(Motion *motion, Accessory *accessory);
    Motion *addModelMotion(Motion *motion, Model *model);
    void removeModel(Model *model);
    void removeAccessory(Accessory *accessory);
    void removeMotion(Motion *motion);
    URI resolveFileURI(const URI &fileURI) const;
    int findDrawableOrderIndex(const IDrawable *drawable) const NANOEM_DECL_NOEXCEPT;
    int findTransformOrderIndex(const Model *model) const NANOEM_DECL_NOEXCEPT;
    UUID generateUUID(const void *ptr) const NANOEM_DECL_NOEXCEPT;

    sg_image backgroundImageHandle();
    bool hasBackgroundImageHandle() const NANOEM_DECL_NOEXCEPT;
    Vector2UI16 windowSize() const NANOEM_DECL_NOEXCEPT;
    void resizeWindowSize(const Vector2UI16 &value);
    void resizeUniformedViewportLayout(const Vector4UI16 &value);
    void resizeUniformedViewportImage(const Vector2UI16 &value);
    void saveState(SaveState *&state);
    void restoreState(const SaveState *state, bool forceSeek);
    void destroyState(SaveState *&state);
    void writeRedoMessage();
    void writeRedoMessage(const Nanoem__Application__Command *command, Error &error);
    void setWritingRedoMessageDisabled(bool value);
    Vector4UI16 queryDevicePixelRectangle(RectangleType type, const Vector2UI16 &offset) const NANOEM_DECL_NOEXCEPT;
    Vector4UI16 queryLogicalPixelRectangle(RectangleType type, const Vector2UI16 &offset) const NANOEM_DECL_NOEXCEPT;
    bool intersectsTransformHandle(const Vector2SI32 &position, RectangleType &type) const NANOEM_DECL_NOEXCEPT;
    void clearAudioSource(Error &error);
    void clearBackgroundVideo();

    void play();
    void stop();
    void pause(bool force);
    void resume(bool force);
    void togglePlaying();
    void restart(nanoem_frame_index_t frameIndex);
    void restart();
    void seek(nanoem_frame_index_t frameIndex, bool forceSeek);
    void seek(nanoem_frame_index_t frameIndex, nanoem_f32_t amount, bool forceSeek);
    void update();
    bool resetAllPasses();
    void pushUndo(undo_command_t *command);
    bool canSeek() const NANOEM_DECL_NOEXCEPT;

    void resetPhysicsSimulation();
    void resetAllModelEdges();
    void performPhysicsSimulationOnce();
    void synchronizeAllMotions(
        nanoem_frame_index_t frameIndex, nanoem_f32_t amount, PhysicsEngine::SimulationTimingType timing);
    void setRenderPassName(sg_pass pass, const char *value);
    void setRenderPipelineName(sg_pipeline pipeline, const char *value);
    sg_image sharedFallbackImage() const NANOEM_DECL_NOEXCEPT;
    sg::PassBlock::IDrawQueue *sharedBatchDrawQueue() NANOEM_DECL_NOEXCEPT;
    sg::PassBlock::IDrawQueue *sharedSerialDrawQueue() NANOEM_DECL_NOEXCEPT;
    ImageLoader *sharedImageLoader();
    internal::BlitPass *sharedImageBlitter();
    internal::DebugDrawer *sharedDebugDrawer();

    void drawAllOffscreenRenderTargets();
    void drawShadowMap();
    void drawViewport();
    void flushAllCommandBuffers();

    sg_pass beginRenderPass(sg_pass pass);
    void blitRenderPass(sg::PassBlock::IDrawQueue *drawQueue, sg_pass destRenderPass, sg_pass sourceRenderPass);
    void clearRenderPass(
        sg::PassBlock::IDrawQueue *drawQueue, sg_pass pass, const sg_pass_action &action, const PixelFormat &format);
    PixelFormat findRenderPassPixelFormat(sg_pass value, int sampleCount) const NANOEM_DECL_NOEXCEPT;
    PixelFormat currentRenderPassPixelFormat() const NANOEM_DECL_NOEXCEPT;
    const char *findRenderPassName(sg_pass value) const NANOEM_DECL_NOEXCEPT;
    const char *findRenderPassName(sg_pass value, const char *fallbackName) const NANOEM_DECL_NOEXCEPT;
    const char *findRenderPipelineName(sg_pipeline value) const NANOEM_DECL_NOEXCEPT;
    const char *findRenderPipelineName(sg_pipeline value, const char *fallbackName) const NANOEM_DECL_NOEXCEPT;
    bool getOriginOffscreenRenderPassColorImageDescription(
        sg_pass_desc &pd, sg_image_desc &id) const NANOEM_DECL_NOEXCEPT;
    bool getCurrentRenderPassColorImageDescription(sg_pass_desc &pd, sg_image_desc &id) const NANOEM_DECL_NOEXCEPT;
    bool getScriptExternalRenderPassColorImageDescription(
        sg_pass_desc &pd, sg_image_desc &id) const NANOEM_DECL_NOEXCEPT;
    bool getRenderPassColorImageDescription(
        sg_pass pass, sg_pass_desc &pd, sg_image_desc &id) const NANOEM_DECL_NOEXCEPT;
    void getViewportRenderPassColorImageDescription(sg_pass_desc &pd, sg_image_desc &id) const NANOEM_DECL_NOEXCEPT;
    bool getOriginOffscreenRenderPassDepthImageDescription(
        sg_pass_desc &pd, sg_image_desc &id) const NANOEM_DECL_NOEXCEPT;
    bool getCurrentRenderPassDepthImageDescription(sg_pass_desc &pd, sg_image_desc &id) const NANOEM_DECL_NOEXCEPT;
    void getViewportRenderPassDepthImageDescription(sg_pass_desc &pd, sg_image_desc &id) const NANOEM_DECL_NOEXCEPT;
    bool isRenderPassOutputSame(sg_pass pass, const sg_pass_desc &pd) const NANOEM_DECL_NOEXCEPT;

    void attachActiveEffect(IDrawable *drawable, Effect *effect, Progress &progress, Error &error);
    void attachActiveEffect(IDrawable *drawable, Effect *effect, const IncludeEffectSourceMap &includeEffectSources,
        Progress &progress, Error &error);
    void attachEffectToSelectedDrawable(Effect *effect, Error &error);
    void attachModelMaterialEffect(model::Material *material, Effect *effect);
    void setOffscreenPassiveRenderTargetEffect(const String &name, IDrawable *drawable, Effect *targetEffect);

    void setRedoDrawable(nanoem_u32_t key, IDrawable *value);
    Accessory *resolveRedoAccessory(nanoem_u32_t key);
    Model *resolveRedoModel(nanoem_u32_t key);
    void clearAllRedoHandles();

    void expandAllTracks();
    void collapseAllTracks();
    void rebuildAllTracks();
    void selectAllKeyframes(nanoem_u32_t flags);
    void selectAllKeyframes(Model *model, nanoem_u32_t flags);
    void copyAllSelectedKeyframes(Error &error);
    void copyAllSelectedKeyframes(Model *model, Error &error);
    void pasteAllSelectedKeyframes(nanoem_frame_index_t frameIndex, Error &error);
    void pasteAllSelectedKeyframes(Model *model, nanoem_frame_index_t frameIndex, Error &error);
    void symmetricPasteAllSelectedKeyframes(nanoem_frame_index_t frameIndex, Error &error);
    void symmetricPasteAllSelectedKeyframes(Model *model, nanoem_frame_index_t frameIndex, Error &error);
    void selectAllKeyframesInColumn();
    void selectAllKeyframesInColumn(Model *model);
    void selectAllMotionKeyframesIn(const TimelineSegment &range);
    void selectAllMotionKeyframesIn(const TimelineSegment &range, Model *model);
    void clearMotionClipboard();
    void copyAllSelectedKeyframeInterpolations();
    void pasteAllSelectedKeyframeInterpolations(nanoem_frame_index_t frameIndex);
    bool hasSelectedKeyframeInterpolations() const NANOEM_DECL_NOEXCEPT;
    void makeAllSelectedKeyframeInterpolationsLinear(Error &error);
    void makeAllSelectedKeyframeInterpolationsLinear(Model *model, Error &error);
    void copyAllSelectedBones(Error &error);
    void copyAllSelectedBones(Model *model, Error &error);
    void pasteAllSelectedBones(Error &error);
    void pasteAllSelectedBones(Model *model, Error &error);
    void symmetricPasteAllSelectedBones(Error &error);
    void symmetricPasteAllSelectedBones(Model *model, Error &error);
    void selectAllBoneKeyframesFromSelectedBoneSet(Model *model);
    bool isModelClipboardEmpty() const NANOEM_DECL_NOEXCEPT;
    bool isMotionClipboardEmpty() const NANOEM_DECL_NOEXCEPT;
    bool hasKeyframeSelection() const NANOEM_DECL_NOEXCEPT;

    void toggleTransformCoordinateType();
    void handleUndoAction();
    void handleRedoAction();
    void handleCopyAction(Error &error);
    void handleCutAction(Error &error);
    void handlePasteAction(Error &error);
    bool canCut() const NANOEM_DECL_NOEXCEPT;
    bool canUndo() const NANOEM_DECL_NOEXCEPT;
    bool canRedo() const NANOEM_DECL_NOEXCEPT;
    bool isDirty() const NANOEM_DECL_NOEXCEPT;
    bool isPlaying() const NANOEM_DECL_NOEXCEPT;
    bool isPaused() const NANOEM_DECL_NOEXCEPT;

    nanoem_frame_index_t duration(nanoem_frame_index_t baseDuration) const NANOEM_DECL_NOEXCEPT;
    nanoem_frame_index_t duration() const NANOEM_DECL_NOEXCEPT;
    nanoem_frame_index_t baseDuration() const NANOEM_DECL_NOEXCEPT;
    nanoem_frame_index_t currentLocalFrameIndex() const NANOEM_DECL_NOEXCEPT;
    nanoem_frame_index_t elapsedLocalFrameIndex() const NANOEM_DECL_NOEXCEPT;
    void setBaseDuration(nanoem_frame_index_t value);
    void setBaseDuration(const IAudioPlayer *audio);
    TimelineSegment playingSegment() const NANOEM_DECL_NOEXCEPT;
    void setPlayingSegment(const TimelineSegment &value);
    TimelineSegment selectionSegment() const NANOEM_DECL_NOEXCEPT;
    void setSelectionSegment(const TimelineSegment &value);
    nanoem_f32_t motionFPSScaleFactor() const NANOEM_DECL_NOEXCEPT;
    const DrawableList *drawableOrderList() const NANOEM_DECL_NOEXCEPT;
    void setDrawableOrderList(const DrawableList &value);
    const ModelList *transformOrderList() const NANOEM_DECL_NOEXCEPT;
    void setTransformOrderList(const ModelList &value);
    const ModelList *allModels() const NANOEM_DECL_NOEXCEPT;
    const AccessoryList *allAccessories() const NANOEM_DECL_NOEXCEPT;
    const MotionList *allMotions() const NANOEM_DECL_NOEXCEPT;
    const TrackList *allTracks() const NANOEM_DECL_NOEXCEPT;
    TrackList allFixedTracks() const;
    SGHandleStringMap allViewNames() const;
    Model *createModel();
    void destroyModel(Model *model);
    Motion *createMotion();
    void destroyMotion(Motion *motion);
    ICamera *createCamera();
    Accessory *createAccessory();
    void destroyAccessory(Accessory *accessory);
    Effect *createEffect();
    Effect *findEffect(const URI &fileURI);
    const Effect *upcastEffect(const IEffect *value) const NANOEM_DECL_NOEXCEPT;
    Effect *upcastEffect(IEffect *value) NANOEM_DECL_NOEXCEPT;
    void destroyEffect(Effect *effect);
    void destroyDrawableEffect(IDrawable *drawable);

    const JSON_Value *applicationConfiguration() const NANOEM_DECL_NOEXCEPT;
    const ITranslator *translator() const NANOEM_DECL_NOEXCEPT;
    nanoem_unicode_string_factory_t *unicodeStringFactory() const NANOEM_DECL_NOEXCEPT;
    IBackgroundVideoRenderer *backgroundVideoRenderer() NANOEM_DECL_NOEXCEPT;
    IConfirmer *confirmer() NANOEM_DECL_NOEXCEPT;
    IFileManager *fileManager() NANOEM_DECL_NOEXCEPT;
    IEventPublisher *eventPublisher() NANOEM_DECL_NOEXCEPT;
    IPrimitive2D *primitive2D() NANOEM_DECL_NOEXCEPT;
    ISharedCancelPublisherRepository *sharedCancelPublisherRepository() NANOEM_DECL_NOEXCEPT;
    ISharedDebugCaptureRepository *sharedDebugCaptureRepository() NANOEM_DECL_NOEXCEPT;
    ISharedResourceRepository *sharedResourceRepository() NANOEM_DECL_NOEXCEPT;

    const Model *activeModel() const NANOEM_DECL_NOEXCEPT;
    Model *activeModel() NANOEM_DECL_NOEXCEPT;
    const Model *lastActiveModel() const NANOEM_DECL_NOEXCEPT;
    Model *lastActiveModel() NANOEM_DECL_NOEXCEPT;
    void setActiveModel(Model *value);
    const Accessory *activeAccessory() const NANOEM_DECL_NOEXCEPT;
    Accessory *activeAccessory() NANOEM_DECL_NOEXCEPT;
    void setActiveAccessory(Accessory *value);
    const ICamera *activeCamera() const NANOEM_DECL_NOEXCEPT;
    ICamera *activeCamera() NANOEM_DECL_NOEXCEPT;
    const ICamera *globalCamera() const NANOEM_DECL_NOEXCEPT;
    ICamera *globalCamera() NANOEM_DECL_NOEXCEPT;
    const ILight *activeLight() const NANOEM_DECL_NOEXCEPT;
    ILight *activeLight() NANOEM_DECL_NOEXCEPT;
    const ILight *globalLight() const NANOEM_DECL_NOEXCEPT;
    ILight *globalLight() NANOEM_DECL_NOEXCEPT;
    const ShadowCamera *shadowCamera() const NANOEM_DECL_NOEXCEPT;
    ShadowCamera *shadowCamera() NANOEM_DECL_NOEXCEPT;
    const Grid *grid() const NANOEM_DECL_NOEXCEPT;
    Grid *grid() NANOEM_DECL_NOEXCEPT;
    const IAudioPlayer *audioPlayer() const NANOEM_DECL_NOEXCEPT;
    IAudioPlayer *audioPlayer() NANOEM_DECL_NOEXCEPT;
    ISkinDeformerFactory *skinDeformerFactory() NANOEM_DECL_NOEXCEPT;
    const Motion *resolveMotion(const IDrawable *drawable) const NANOEM_DECL_NOEXCEPT;
    Motion *resolveMotion(IDrawable *drawable) NANOEM_DECL_NOEXCEPT;
    const IDrawable *resolveDrawable(const Motion *motion) const NANOEM_DECL_NOEXCEPT;
    IDrawable *resolveDrawable(Motion *motion) NANOEM_DECL_NOEXCEPT;
    const Motion *cameraMotion() const NANOEM_DECL_NOEXCEPT;
    Motion *cameraMotion() NANOEM_DECL_NOEXCEPT;
    const Motion *lightMotion() const NANOEM_DECL_NOEXCEPT;
    Motion *lightMotion() NANOEM_DECL_NOEXCEPT;
    const Motion *selfShadowMotion() const NANOEM_DECL_NOEXCEPT;
    Motion *selfShadowMotion() NANOEM_DECL_NOEXCEPT;
    const undo_stack_t *activeUndoStack() const NANOEM_DECL_NOEXCEPT;
    undo_stack_t *activeUndoStack() NANOEM_DECL_NOEXCEPT;
    const undo_stack_t *undoStack() const NANOEM_DECL_NOEXCEPT;
    undo_stack_t *undoStack() NANOEM_DECL_NOEXCEPT;
    URI fileURI() const;
    void setFileURI(const URI &value);
    FileUtils::TransientPath transientPath() const;
    bool hasTransientPath() const NANOEM_DECL_NOEXCEPT;
    void setTransientPath(const FileUtils::TransientPath &value);
    bool containsIndexOfMaterialToAttachEffect(
        nanoem_u16_t handle, nanoem_rsize_t materialIndex) const NANOEM_DECL_NOEXCEPT;
    void addIndexOfMaterialToAttachEffect(nanoem_u16_t handle, nanoem_rsize_t materialIndex);
    void removeIndexOfMaterialToAttachEffect(nanoem_u16_t handle, nanoem_rsize_t materialIndex);
    void clearAllIndicesOfMaterialToAttachEffect(nanoem_u16_t handle);
    URI redoFileURI() const;
    void setRedoFileURI(const URI &value);
    Vector4SI32 backgroundVideoRect() const NANOEM_DECL_NOEXCEPT;
    void setBackgroundVideoRect(const Vector4SI32 &value);
    Vector2SI32 deviceScaleMovingCursorPosition() const NANOEM_DECL_NOEXCEPT;
    Vector2SI32 logicalScaleMovingCursorPosition() const NANOEM_DECL_NOEXCEPT;
    void setLogicalPixelMovingCursorPosition(const Vector2SI32 &value);
    nanoem_u32_t cursorModifiers() const NANOEM_DECL_NOEXCEPT;
    void setCursorModifiers(nanoem_u32_t value);
    Vector4SI32 deviceScaleLastCursorPosition(CursorType type) const NANOEM_DECL_NOEXCEPT;
    Vector4SI32 logicalScaleLastCursorPosition(CursorType type) const NANOEM_DECL_NOEXCEPT;
    bool isCursorPressed(CursorType type) const NANOEM_DECL_NOEXCEPT;
    void setLogicalPixelLastCursorPosition(CursorType type, const Vector2SI32 &value, bool pressed);
    Vector2SI32 lastScrollDelta() const NANOEM_DECL_NOEXCEPT;
    void setLastScrollDelta(const Vector2SI32 &value);
    Vector4 logicalScaleUniformedViewportImageRect() const NANOEM_DECL_NOEXCEPT;
    Vector4 deviceScaleUniformedViewportImageRect() const NANOEM_DECL_NOEXCEPT;
    Vector4UI16 logicalScaleUniformedViewportLayoutRect() const NANOEM_DECL_NOEXCEPT;
    Vector4UI16 deviceScaleUniformedViewportLayoutRect() const NANOEM_DECL_NOEXCEPT;
    Vector2UI16 logicalScaleUniformedViewportImageSize() const NANOEM_DECL_NOEXCEPT;
    Vector2UI16 deviceScaleUniformedViewportImageSize() const NANOEM_DECL_NOEXCEPT;
    Vector2UI16 logicalViewportPadding() const NANOEM_DECL_NOEXCEPT;
    void setLogicalViewportPadding(const Vector2UI16 &value);
    Vector2SI32 resolveLogicalCursorPositionInViewport(const Vector2SI32 &value) const NANOEM_DECL_NOEXCEPT;
    Vector4 viewportBackgroundColor() const NANOEM_DECL_NOEXCEPT;
    void setViewportBackgroundColor(const Vector4 &value);
    nanoem_f64_t currentUptimeSeconds() const NANOEM_DECL_NOEXCEPT;
    nanoem_f64_t elapsedUptimeSeconds() const NANOEM_DECL_NOEXCEPT;
    void setUptimeSeconds(nanoem_f64_t value);
    nanoem_f32_t deviceScaleViewportScaleFactor() const NANOEM_DECL_NOEXCEPT;
    nanoem_f32_t windowDevicePixelRatio() const NANOEM_DECL_NOEXCEPT;
    nanoem_f32_t pendingWindowDevicePixelRatio() const NANOEM_DECL_NOEXCEPT;
    void setWindowDevicePixelRatio(nanoem_f32_t value);
    nanoem_f32_t viewportDevicePixelRatio() const NANOEM_DECL_NOEXCEPT;
    void setViewportDevicePixelRatio(nanoem_f32_t value);
    nanoem_f32_t logicalScaleCircleRadius() const NANOEM_DECL_NOEXCEPT;
    nanoem_f32_t deviceScaleCircleRadius() const NANOEM_DECL_NOEXCEPT;
    void setCircleRadius(nanoem_f32_t value);
    const PhysicsEngine *physicsEngine() const NANOEM_DECL_NOEXCEPT;
    PhysicsEngine *physicsEngine();
    nanoem_motion_bone_keyframe_interpolation_type_t boneKeyframeInterpolationType() const NANOEM_DECL_NOEXCEPT;
    void setBoneKeyframeInterpolationType(nanoem_motion_bone_keyframe_interpolation_type_t value);
    nanoem_motion_camera_keyframe_interpolation_type_t cameraKeyframeInterpolationType() const NANOEM_DECL_NOEXCEPT;
    void setCameraKeyframeInterpolationType(nanoem_motion_camera_keyframe_interpolation_type_t value);
    IDrawable::DrawType drawType() const NANOEM_DECL_NOEXCEPT;
    void setDrawType(IDrawable::DrawType value);
    sg_pass registerRenderPass(const sg_pass_desc &desc, const PixelFormat &format);
    sg_pass registerOffscreenRenderPass(const Effect *ownerEffect, const effect::OffscreenRenderTargetOption &option);
    void overrideOffscreenRenderPass(const sg_pass_desc &desc, const PixelFormat &format);
    sg_pass currentRenderPass() const NANOEM_DECL_NOEXCEPT;
    sg_pass currentOffscreenRenderPass() const NANOEM_DECL_NOEXCEPT;
    sg_pass lastDrawnRenderPass() const NANOEM_DECL_NOEXCEPT;
    sg_pass scriptExternalRenderPass() const NANOEM_DECL_NOEXCEPT;
    bool isCurrentRenderPassActive() const NANOEM_DECL_NOEXCEPT;
    bool isOffscreenRenderPassActive() const NANOEM_DECL_NOEXCEPT;
    bool isRenderPassViewport() const NANOEM_DECL_NOEXCEPT;
    void setCurrentRenderPass(sg_pass value);
    void resetCurrentRenderPass();
    void setScriptExternalRenderPass(sg_pass value, const Vector4 &clearColor, nanoem_f32_t clearDepth);
    void resetScriptExternalRenderPass();
    void createAllOffscreenRenderTargets(Effect *ownerEffect, const IncludeEffectSourceMap &includeEffectSources,
        bool enableEffectPlugin, bool enableSourceCache, Progress &progress, Error &error);
    void attachDrawableOffscreenEffects(IDrawable *drawable);
    void releaseAllOffscreenRenderTarget(Effect *ownerEffect);
    void getAllOffscreenRenderTargetEffects(const IEffect *ownerEffect, LoadedEffectSet &allRenderTargetEffects) const;
    Vector2UI16 deviceScaleViewportPrimaryImageSize() const NANOEM_DECL_NOEXCEPT;
    sg_pass viewportPrimaryPass() const NANOEM_DECL_NOEXCEPT;
    sg_image viewportPrimaryImage() const NANOEM_DECL_NOEXCEPT;
    sg_image viewportSecondaryImage() const NANOEM_DECL_NOEXCEPT;
    sg_image context2DImage() const NANOEM_DECL_NOEXCEPT;
    sg_pass context2DPass() const NANOEM_DECL_NOEXCEPT;
    bool containsDrawableToAttachOffscreenRenderTargetEffect(
        const String &key, IDrawable *value) const NANOEM_DECL_NOEXCEPT;
    void addDrawableToAttachOffscreenRenderTargetEffect(const String &key, IDrawable *value);
    void removeDrawableToAttachOffscreenRenderTargetEffect(const String &key, IDrawable *value);
    void clearAllDrawablesToAttachOffscreenRenderTargetEffect(const String &key);
    effect::RenderTargetColorImageContainer *findSharedRenderTargetImageContainer(
        const String &name, const IEffect *parent) const;
    effect::RenderTargetColorImageContainer *findSharedRenderTargetImageContainer(
        sg_image handle, const IEffect *parent) const NANOEM_DECL_NOEXCEPT;
    effect::RenderPassScope *offscreenRenderPassScope() NANOEM_DECL_NOEXCEPT;
    int countSharedRenderTargetImageContainer(const String &name, const IEffect *parent) const;
    void setSharedRenderTargetImageContainer(
        const String &name, const IEffect *parent, effect::RenderTargetColorImageContainer *container);
    void removeAllSharedRenderTargetImageContainers(const IEffect *parent);
    const ITrack *selectedTrack() const NANOEM_DECL_NOEXCEPT;
    ITrack *selectedTrack() NANOEM_DECL_NOEXCEPT;
    void setSelectedTrack(ITrack *value);
    EditingMode editingMode() const NANOEM_DECL_NOEXCEPT;
    void setEditingMode(EditingMode value);
    FilePathMode filePathMode() const NANOEM_DECL_NOEXCEPT;
    void setFilePathMode(FilePathMode value);
    bool isPhysicsSimulationEnabled() const NANOEM_DECL_NOEXCEPT;
    void setPhysicsSimulationMode(PhysicsEngine::SimulationModeType value);
    ITranslator::LanguageType language() const NANOEM_DECL_NOEXCEPT;
    nanoem_language_type_t castLanguage() const NANOEM_DECL_NOEXCEPT;
    void setLanguage(ITranslator::LanguageType value);
    Vector2UI16 shadowMapSize() const NANOEM_DECL_NOEXCEPT;
    void setShadowMapSize(const Vector2UI16 &value);
    Vector2UI16 viewportImageSize() const NANOEM_DECL_NOEXCEPT;
    void setViewportImageSize(const Vector2UI16 &value);
    sg_pixel_format viewportPixelFormat() const NANOEM_DECL_NOEXCEPT;
    void setViewportPixelFormat(sg_pixel_format value);
    TransformPerformIndex transformPerformedAt() const NANOEM_DECL_NOEXCEPT;
    void setTransformPerformedAt(const TransformPerformIndex &value);
    void updateTransformPerformedAt(const TransformPerformIndex &value);
    void resetTransformPerformedAt();
    nanoem_f32_t timeStepFactor() const NANOEM_DECL_NOEXCEPT;
    void setTimeStepFactor(nanoem_f32_t value);
    nanoem_f32_t backgroundVideoScaleFactor() const NANOEM_DECL_NOEXCEPT;
    void setBackgroundVideoScaleFactor(nanoem_f32_t value);
    nanoem_f32_t physicsSimulationTimeStep() const NANOEM_DECL_NOEXCEPT;
    nanoem_u32_t baseFPS() const NANOEM_DECL_NOEXCEPT;
    nanoem_u32_t preferredMotionFPS() const NANOEM_DECL_NOEXCEPT;
    nanoem_f32_t invertedPreferredMotionFPS() const NANOEM_DECL_NOEXCEPT;
    void setPreferredMotionFPS(nanoem_u32_t value, bool unlimited);
    nanoem_u32_t preferredEditingFPS() const NANOEM_DECL_NOEXCEPT;
    void setPreferredEditingFPS(nanoem_u32_t value);
    nanoem_u32_t actualFPS() const NANOEM_DECL_NOEXCEPT;
    void setActualFPS(nanoem_u32_t value);
    void setPhysicsSimulationEngineDebugFlags(nanoem_u32_t value);
    nanoem_u32_t coordinationSystem() const NANOEM_DECL_NOEXCEPT;
    void setCoordinationSystem(nanoem_u32_t value);
    int maxAnisotropyValue() const NANOEM_DECL_NOEXCEPT;
    int sampleCount() const NANOEM_DECL_NOEXCEPT;
    nanoem_u32_t sampleLevel() const NANOEM_DECL_NOEXCEPT;
    void setSampleLevel(nanoem_u32_t value);
    bool isResetAllPassesPending() const NANOEM_DECL_NOEXCEPT;
    bool isDisplaySyncDisabled() const NANOEM_DECL_NOEXCEPT;
    bool isHiddenBoneBoundsRigidBodyDisabled() const NANOEM_DECL_NOEXCEPT;
    void setHiddenBoneBoundsRigidBodyDisabled(bool value);
    bool isLoopEnabled() const NANOEM_DECL_NOEXCEPT;
    void setLoopEnabled(bool value);
    bool isCameraShared() const NANOEM_DECL_NOEXCEPT;
    void setCameraShared(bool value);
    bool isGroundShadowEnabled() const NANOEM_DECL_NOEXCEPT;
    void setGroundShadowEnabled(bool value);
    bool isShadowMapEnabled() const NANOEM_DECL_NOEXCEPT;
    void setShadowMapEnabled(bool value);
    bool isMultipleBoneSelectionEnabled() const NANOEM_DECL_NOEXCEPT;
    void setMultipleBoneSelectionEnabled(bool value);
    bool isTransformHandleVisible() const NANOEM_DECL_NOEXCEPT;
    void setTransformHandleVisible(bool value);
    bool isBezierCurveAjustmentEnabled() const NANOEM_DECL_NOEXCEPT;
    void setBezierCurveAdjustmentEnabled(bool value);
    bool isMotionMergeEnabled() const NANOEM_DECL_NOEXCEPT;
    void setMotionMergeEnabled(bool value);
    bool isEffectPluginEnabled() const NANOEM_DECL_NOEXCEPT;
    void setEffectPluginEnabled(bool value);
    bool isCompiledEffectCacheEnabled() const NANOEM_DECL_NOEXCEPT;
    void setCompiledEffectCacheEnabled(bool value);
    bool isViewportCaptured() const NANOEM_DECL_NOEXCEPT;
    void setViewportCaptured(bool value);
    bool isViewportHovered() const NANOEM_DECL_NOEXCEPT;
    void setViewportHovered(bool value);
    bool isViewportWindowDetached() const NANOEM_DECL_NOEXCEPT;
    void setViewportWindowDetached(bool value);
    bool isPrimaryCursorTypeLeft() const NANOEM_DECL_NOEXCEPT;
    void setPrimaryCursorTypeLeft(bool value);
    bool isPlayingAudioPartEnabled() const NANOEM_DECL_NOEXCEPT;
    void setPlayingAudioPartEnabled(bool value);
    bool isViewportWithTransparentEnabled() const NANOEM_DECL_NOEXCEPT;
    void setViewportWithTransparentEnabled(bool value);
    bool isConfirmSeekEnabled(nanoem_mutable_motion_keyframe_type_t type) const NANOEM_DECL_NOEXCEPT;
    void setConfirmSeekEnabled(nanoem_mutable_motion_keyframe_type_t type, bool value);
    bool isCancelRequested() const NANOEM_DECL_NOEXCEPT;
    void setCancelRequested(bool value);
    bool isUniformedViewportImageSizeEnabled() const NANOEM_DECL_NOEXCEPT;
    void setUniformedViewportImageSizeEnabled(bool value);
    bool isFPSCounterEnabled() const NANOEM_DECL_NOEXCEPT;
    void setFPSCounterEnabled(bool value);
    bool isPerformanceMonitorEnabled() const NANOEM_DECL_NOEXCEPT;
    void setPerformanceMonitorEnabled(bool value);
    bool hasInputTextFocus() const NANOEM_DECL_NOEXCEPT;
    void setInputTextFocus(bool value);
    bool isPhysicsSimulationForBoneKeyframeEnabled() const NANOEM_DECL_NOEXCEPT;
    void setPhysicsSimulationForBoneKeyframeEnabled(bool value);
    bool isAnisotropyEnabled() const NANOEM_DECL_NOEXCEPT;
    void setAnisotropyEnabled(bool value);
    bool isMipmapEnabled() const NANOEM_DECL_NOEXCEPT;
    void setMipmapEnabled(bool value);
    bool isPowerSavingEnabled() const NANOEM_DECL_NOEXCEPT;
    void setPowerSavingEnabled(bool value);
    bool isModelEditingEnabled() const NANOEM_DECL_NOEXCEPT;
    void setModelEditingEnabled(bool value);
    bool isActive() const NANOEM_DECL_NOEXCEPT;
    void setActive(bool value);

private:
    struct DrawQueue;
    struct BatchDrawQueue;
    struct SerialDrawQueue;
    struct Pass {
        Pass(Project *project, const char *name);
        void update(const Vector2UI16 &size);
        void getDescription(sg_pass_desc &pd) const NANOEM_DECL_NOEXCEPT;
        void destroy();
        Project *m_project;
        String m_name;
        sg_pass m_handle;
        sg_image m_colorImage;
        sg_image m_depthImage;
    };
    struct FPSUnit {
        FPSUnit() NANOEM_DECL_NOEXCEPT;
        ~FPSUnit() NANOEM_DECL_NOEXCEPT;
        bool operator!=(nanoem_u32_t value) const NANOEM_DECL_NOEXCEPT;
        void operator=(nanoem_u32_t value) NANOEM_DECL_NOEXCEPT;
        nanoem_u32_t m_value;
        nanoem_f32_t m_scaleFactor;
        nanoem_f32_t m_invertedValue;
        nanoem_f32_t m_invertedScaleFactor;
    };
    struct RenderPassBundle {
        RenderPassBundle();
        ~RenderPassBundle() NANOEM_DECL_NOEXCEPT;
        sg_pass m_handle;
        sg_image m_colorImage;
        sg_image m_depthImage;
        sg_pass_desc m_desciption;
        PixelFormat m_format;
    };
    struct SharedRenderTargetImageContainer {
        const IEffect *m_parent;
        effect::RenderTargetColorImageContainer *m_container;
        int m_count;
    };
    struct OffscreenRenderTargetCondition {
        String m_pattern;
        Effect *m_passiveEffect;
        bool m_hidden;
        bool m_none;
    };

    typedef tinystl::unordered_set<IDrawable *, TinySTLAllocator> DrawableSet;
    typedef tinystl::unordered_map<nanoem_u16_t, Accessory *, TinySTLAllocator> AccessoryHandleMap;
    typedef tinystl::unordered_map<nanoem_u16_t, Model *, TinySTLAllocator> ModelHandleMap;
    typedef tinystl::unordered_map<nanoem_u16_t, Motion *, TinySTLAllocator> MotionHandleMap;
    typedef tinystl::unordered_map<IEffect::ScriptOrderType, DrawableSet, TinySTLAllocator> EffectOrderSet;
    typedef tinystl::unordered_map<const IDrawable *, int, TinySTLAllocator> TechniqueOffsetMap;
    typedef tinystl::unordered_map<nanoem_u32_t, RenderPassBundle, TinySTLAllocator> RenderPassBundleMap;
    typedef tinystl::unordered_map<nanoem_u32_t, RenderPassBundle *, TinySTLAllocator> HashedRenderPassBundleMap;
    typedef tinystl::vector<OffscreenRenderTargetCondition, TinySTLAllocator> OffscreenRenderTargetConditionList;
    typedef tinystl::unordered_map<String, OffscreenRenderTargetConditionList, TinySTLAllocator>
        NamedOffscreenRenderTargetConditionListMap;
    typedef tinystl::unordered_map<Effect *, NamedOffscreenRenderTargetConditionListMap, TinySTLAllocator>
        OffscreenRenderTargetConditionListMap;
    typedef tinystl::unordered_map<const IEffect *, LoadedEffectSet, TinySTLAllocator>
        OffscreenRenderTargetEffectSetMap;
    typedef tinystl::unordered_map<String, SharedRenderTargetImageContainer, TinySTLAllocator>
        SharedRenderTargetImageContainerMap;
    typedef tinystl::unordered_map<Motion *, IDrawable *, TinySTLAllocator> DrawableMotionSet;
    typedef tinystl::unordered_map<nanoem_u32_t, nanoem_u16_t, TinySTLAllocator> RedoObjectHandleMap;
    typedef tinystl::vector<const effect::OffscreenRenderTargetOption *, TinySTLAllocator>
        SortedOffscreenRenderTargetOptionList;
    typedef tinystl::unordered_map<String, tinystl::pair<Effect *, int>, TinySTLAllocator> EffectReferenceMap;
    typedef tinystl::pair<String, DrawableSet> OffscreenRenderTargetDrawableSet;

    static Vector4UI16 internalQueryRectangle(RectangleType type, const Vector4UI16 &viewportRect,
        const Vector2UI16 &offset, nanoem_f32_t deviceScaleRatio) NANOEM_DECL_NOEXCEPT;
    static Vector2UI16 uniformedViewportImageSize(
        const Vector2UI16 &viewportLayoutSize, const Vector2UI16 &viewportImageSize) NANOEM_DECL_NOEXCEPT;
    static void adjustViewportImageRect(const Vector4 viewportLayoutRect, const Vector2 &viewportImageSize,
        Vector4 &viewportImageRect) NANOEM_DECL_NOEXCEPT;
    static bool matchDrawableEffect(const IDrawable *drawable, const Effect *ownerEffect, const String &expr);
    static void symmetricLocalTransformBone(
        const String &name, const Vector3 &translation, const Quaternion &orientation, Model *model);

    void addEffectOrderSet(IDrawable *drawable);
    void addEffectOrderSet(IDrawable *drawable, IEffect::ScriptOrderType order);
    void removeEffectOrderSet(IDrawable *drawable);
    void internalSetDrawableActiveEffect(IDrawable *drawable, Effect *effect,
        const IncludeEffectSourceMap &includeEffectSources, bool enableEffectPlugin, bool enableSourceCache,
        Progress &progress, Error &error);
    void applyAllOffscreenRenderTargetEffectsToDrawable(IDrawable *drawable);
    void applyAllDrawablesToOffscreenRenderTargetEffect(IDrawable *ownerDrawable, Effect *ownerEffect);
    void applyDrawableToOffscreenRenderTargetEffect(IDrawable *drawable, Effect *ownerEffect);
    void loadOffscreenRenderTargetEffect(Effect *ownerEffect, const IncludeEffectSourceMap &includeEffectSources,
        const StringPair &condition, bool enableEffectPlugin, bool enableSourceCache,
        OffscreenRenderTargetConditionList &newConditions, Progress &progress, Error &error);
    bool loadOffscreenRenderTargetEffectFromEffectSourceMap(const Effect *ownerEffect,
        const IncludeEffectSourceMap &includeEffectSources, const StringPair &condition, bool enableEffectPlugin,
        OffscreenRenderTargetConditionList &newConditions, Progress &progress, Error &error);
    bool loadOffscreenRenderTargetEffectFromByteArray(Effect *targetEffect, const URI &fileURI,
        const StringPair &condition, const ByteArray &bytes, OffscreenRenderTargetConditionList &newConditionsoid,
        Progress &progress, Error &error);
    void cancelRenderOffscreenRenderTarget(Effect *ownerEffect);

    void internalPasteAllSelectedKeyframes(Model *model, nanoem_frame_index_t frameIndex, bool symmetric, Error &error);
    void internalPasteAllSelectedBones(Model *model, bool symmetric, Error &error);
    void internalSeek(nanoem_frame_index_t frameIndex);
    void internalSeek(nanoem_frame_index_t frameIndex, nanoem_f32_t amount, nanoem_f32_t delta);
    void destroyDetachedEffect(Effect *effect);
    void synchronizeCamera(nanoem_frame_index_t frameIndex, nanoem_f32_t amount);
    void synchronizeLight(nanoem_frame_index_t frameIndex, nanoem_f32_t amount);
    void synchronizeSelfShadow(nanoem_frame_index_t frameIndex);
    void markAllModelsDirty();
    void internalPerformPhysicsSimulation(nanoem_f32_t delta);
    void removeDrawable(IDrawable *drawable);
    void internalResizeUniformedViewportImage(const Vector2UI16 &value);
    void internalResetAllRenderTargets(const Vector2UI16 &size);
    void resetViewportPassFormatAndDescription();
    sg_pass registerRenderPass(const sg_pass_desc &desc, const PixelFormat &format, RenderPassBundle *&descPtrRef);
    void resetOffscreenRenderTarget(const Effect *ownerEffect, const effect::OffscreenRenderTargetOption &option);
    void drawOffscreenRenderTarget(Effect *ownerEffect);
    void drawObjectToOffscreenRenderTarget(IDrawable *drawable, Effect *ownerEffect, const String &name);
    void drawAllEffectsDependsOnScriptExternal();
    void getAllOffscreenRenderTargetOptions(const Effect *ownerEffect, effect::OffscreenRenderTargetOptionList &value,
        SortedOffscreenRenderTargetOptionList &sorted) const;
    bool hasAnyDependsOnScriptExternalEffect() const NANOEM_DECL_NOEXCEPT;
    void clearViewportPrimaryPass();
    void drawBackgroundVideo();
    void drawGrid();
    void drawViewport(IEffect::ScriptOrderType order, IDrawable::DrawType type);
    void blitRenderPass(sg::PassBlock::IDrawQueue *drawQueue, sg_pass destRenderPass, sg_pass sourceRenderPass,
        internal::BlitPass *blitter);
    void createFallbackImage();
    void preparePlaying();
    void prepareStopping(bool forceSeek);
    bool loadAttachedDrawableEffect(IDrawable *drawable, bool enableSourceCache, Progress &progress, Error &error);
    URI resolveSourceEffectCachePath(const URI &fileURI);
    bool findSourceEffectCache(const URI &fileURI, ByteArray &cache, Error &error);
    void setSourceEffectCache(const URI &fileURI, const ByteArray &cache, Error &error);
    void addLoadedEffectSet(Effect *value);
    void setOffscreenRenderPassScope(effect::RenderPassScope *value);
    bool continuesPlaying();

    const IUnicodeStringFactoryRepository *m_unicodeStringFactoryRepository;
    const JSON_Value *m_applicationConfiguration;
    IBackgroundVideoRenderer *m_backgroundVideoRenderer;
    IConfirmer *m_confirmer;
    IFileManager *m_fileManager;
    IEventPublisher *m_eventPublisher;
    IPrimitive2D *m_primitive2D;
    IRendererCapability *m_rendererCapability;
    ISharedCancelPublisherRepository *m_sharedCancelPublisherRepository;
    ISharedDebugCaptureRepository *m_sharedDebugCaptureRepository;
    ISharedResourceRepository *m_sharedResourceRepository;
    ITranslator *m_translator;
    ImageLoader *m_sharedImageLoader;
    DrawableList m_drawableOrderList;
    ModelList m_transformModelOrderList;
    tinystl::pair<Model *, Model *> m_activeModelPairPtr;
    Accessory *m_activeAccessoryPtr;
    IAudioPlayer *m_audioPlayer;
    ISkinDeformerFactory *m_skinDeformerFactory;
    PhysicsEngine *m_physicsEngine;
    PerspectiveCamera *m_camera;
    DirectionalLight *m_light;
    Grid *m_grid;
    Motion *m_cameraMotionPtr;
    Motion *m_lightMotionPtr;
    Motion *m_selfShadowMotionPtr;
    ShadowCamera *m_shadowCamera;
    undo_stack_t *m_undoStack;
    ModelList m_allModelPtrs;
    AccessoryList m_allAccessoryPtrs;
    MotionList m_allMotions;
    MotionHashMap m_drawable2MotionPtrs;
    TrackList m_allTracks;
    ITrack *m_selectedTrack;
    SaveState *m_lastSaveState;
    DrawQueue *m_drawQueue;
    BatchDrawQueue *m_batchDrawQueue;
    SerialDrawQueue *m_serialDrawQueue;
    effect::RenderPassScope *m_offscreenRenderPassScope;
    internal::BlitPass *m_viewportPassBlitter;
    internal::BlitPass *m_renderPassBlitter;
    internal::BlitPass *m_sharedImageBlitter;
    internal::ClearPass *m_renderPassCleaner;
    internal::DebugDrawer *m_sharedDebugDrawer;
    tinystl::pair<sg_pixel_format, sg_pixel_format> m_viewportPixelFormat;
    model::BindPose m_lastBindPose;
    model::RigidBody::VisualizationClause m_rigidBodyVisualizationClause;
    IDrawable::DrawType m_drawType;
    tinystl::pair<URI, FileUtils::TransientPath> m_fileURI;
    URI m_redoFileURI;
    OffscreenRenderTargetDrawableSet m_drawablesToAttachOffscreenRenderTargetEffect;
    sg_pass m_currentRenderPass;
    sg_pass m_lastDrawnRenderPass;
    sg_pass m_currentOffscreenRenderPass;
    sg_pass m_originOffscreenRenderPass;
    sg_pass m_scriptExternalRenderPass;
    SharedRenderTargetImageContainerMap m_sharedRenderTargetImageContainers;
    EditingMode m_editingMode;
    FilePathMode m_filePathMode;
    TimelineSegment m_playingSegment;
    TimelineSegment m_selectionSegment;
    nanoem_frame_index_t m_baseDuration;
    ITranslator::LanguageType m_language;
    tinystl::pair<Vector4UI16, Vector4UI16> m_uniformViewportLayoutRect;
    tinystl::pair<Vector2UI16, Vector2UI16> m_uniformViewportImageSize;
    Vector4SI32 m_backgroundVideoRect;
    Vector4SI32 m_boneSelectionRect;
    Vector4SI32 m_logicalScaleCursorPositions[kCursorTypeMaxEnum];
    Vector2SI32 m_logicalScaleMovingCursorPosition;
    Vector2SI32 m_scrollDelta;
    Vector2UI16 m_windowSize;
    Vector2UI16 m_viewportImageSize;
    Vector2UI16 m_viewportPadding;
    Vector4 m_viewportBackgroundColor;
    OffscreenRenderTargetConditionListMap m_allOffscreenRenderTargets;
    OffscreenRenderTargetEffectSetMap m_allOffscreenRenderTargetEffectSets;
    sg_image m_fallbackImage;
    bx::HandleAlloc *m_objectHandleAllocator;
    AccessoryHandleMap m_accessoryHandleMap;
    ModelHandleMap m_modelHandleMap;
    MotionHandleMap m_motionHandleMap;
    RenderPassBundleMap m_renderPassBundleMap;
    HashedRenderPassBundleMap m_hashedRenderPassBundleMap;
    RedoObjectHandleMap m_redoObjectHandles;
    SGHandleStringMap m_renderPassStringMap;
    SGHandleStringMap m_renderPipelineStringMap;
    Pass m_viewportPrimaryPass;
    Pass m_viewportSecondaryPass;
    Pass m_context2DPass;
    tinystl::pair<sg_image, Vector2UI16> m_backgroundImage;
    FPSUnit m_preferredMotionFPS;
    nanoem_u32_t m_editingFPS;
    nanoem_motion_bone_keyframe_interpolation_type_t m_boneInterpolationType;
    nanoem_motion_camera_keyframe_interpolation_type_t m_cameraInterpolationType;
    ByteArray m_modelClipboard;
    ByteArray m_motionClipboard;
    EffectOrderSet m_effectOrderSet;
    EffectReferenceMap m_effectReferences;
    LoadedEffectSet m_loadedEffectSet;
    DrawableList m_dependsOnScriptExternal;
    TransformPerformIndex m_transformPerformedAt;
    ModelMaterialIndexSetPair m_indicesOfMaterialToAttachEffect;
    tinystl::pair<nanoem_f32_t, nanoem_f32_t> m_windowDevicePixelRatio;
    tinystl::pair<nanoem_f32_t, nanoem_f32_t> m_viewportDevicePixelRatio;
    tinystl::pair<nanoem_f64_t, nanoem_f64_t> m_uptime;
    tinystl::pair<nanoem_frame_index_t, nanoem_frame_index_t> m_localFrameIndex;
    nanoem_f32_t m_timeStepFactor;
    nanoem_f32_t m_backgroundVideoScaleFactor;
    nanoem_f32_t m_circleRadius;
    tinystl::pair<nanoem_u32_t, nanoem_u32_t> m_sampleLevel;
    nanoem_u64_t m_stateFlags;
    nanoem_u64_t m_confirmSeekFlags;
    nanoem_u32_t m_lastPhysicsDebugFlags;
    nanoem_u32_t m_coordinationSystem;
    nanoem_u32_t m_cursorModifiers;
    nanoem_u32_t m_actualFPS;
    nanoem_u32_t m_actionSequence;
    bool m_active;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_PROJECT_H_ */
