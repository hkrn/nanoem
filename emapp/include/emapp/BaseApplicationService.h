/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_BASEAPPLICATIONSERVICE_H_
#define NANOEM_EMAPP_BASEAPPLICATIONSERVICE_H_

#include "emapp/IFileManager.h"
#include "emapp/Project.h"

struct nk_context;
struct undo_command_t;

typedef struct _Nanoem__Application__Command Nanoem__Application__Command;
typedef struct _Nanoem__Application__Event Nanoem__Application__Event;

namespace bx {
class CommandLine;
} /* namespace bx */

namespace nanoem {

class BaseApplicationClient;
class DefaultFileManager;
class IAudioPlayer;
class IBackgroundVideoRenderer;
class IEventPublisher;
class IFileManager;
class IModalDialog;
class IPrimitive2D;
class IProjectHolder;
class IVideoRecorder;
class Image;
class Project;
class StateController;

union sentry_value_u {
    nanoem_u64_t _bits;
    nanoem_f64_t _double;
};
typedef union sentry_value_u sentry_value_t;
typedef sentry_value_t(APIENTRY *PFN_sentry_value_new_breadcrumb)(const char *, const char *);
typedef sentry_value_t(APIENTRY *PFN_sentry_value_new_int32)(nanoem_i32_t);
typedef sentry_value_t(APIENTRY *PFN_sentry_value_new_double)(nanoem_f64_t);
typedef sentry_value_t(APIENTRY *PFN_sentry_value_new_string)(const char *);
typedef sentry_value_t(APIENTRY *PFN_sentry_value_new_object)();
typedef void(APIENTRY *PFN_sentry_value_set_by_key)(sentry_value_t, const char *, sentry_value_t);
typedef void(APIENTRY *PFN_sentry_add_breadcrumb)(sentry_value_t);
typedef void(APIENTRY *PFN_sentry_set_extra)(const char *, sentry_value_t);
typedef void(APIENTRY *PFN_sentry_set_tag)(const char *, const char *);

extern PFN_sentry_value_new_breadcrumb sentry_value_new_breadcrumb;
extern PFN_sentry_value_new_int32 sentry_value_new_int32;
extern PFN_sentry_value_new_double sentry_value_new_double;
extern PFN_sentry_value_new_string sentry_value_new_string;
extern PFN_sentry_value_new_object sentry_value_new_object;
extern PFN_sentry_value_set_by_key sentry_value_set_by_key;
extern PFN_sentry_add_breadcrumb sentry_add_breadcrumb;
extern PFN_sentry_set_extra sentry_set_extra;
extern PFN_sentry_set_tag sentry_set_tag;

struct sentry_envelope_s;
typedef struct sentry_envelope_s sentry_envelope_t;
typedef void(APIENTRY *PFN_sentry_envelope_free)(sentry_envelope_t *);
typedef char *(APIENTRY *PFN_sentry_envelope_serialize)(const sentry_envelope_t *, size_t *);
typedef void(APIENTRY *PFN_sentry_free)(void *);
extern PFN_sentry_envelope_free sentry_envelope_free;
extern PFN_sentry_envelope_serialize sentry_envelope_serialize;
extern PFN_sentry_free sentry_free;

namespace internal {
class CapturingPassState;
class IUIWindow;
} /* namespace internal */

namespace plugin {
class ModelIOPlugin;
class MotionIOPlugin;
} /* namespace plguin */

class BaseApplicationService : private NonCopyable {
public:
    static const char *const kOrganizationDomain;
    static const char *const kRendererOpenGL;
    static const char *const kRendererDirectX;
    static const char *const kRendererMetal;
    enum KeyType {
        kKeyType_UNKNOWN = -1,
        kKeyType_FIRST = 32,
        kKeyType_SPACE = kKeyType_FIRST,
        kKeyType_APOSTROPHE = 39,
        kKeyType_COMMA = 44,
        kKeyType_MINUS = 45,
        kKeyType_PERIOD = 46,
        kKeyType_SLASH = 47,
        kKeyType_0 = 48,
        kKeyType_1 = 49,
        kKeyType_2 = 50,
        kKeyType_3 = 51,
        kKeyType_4 = 52,
        kKeyType_5 = 53,
        kKeyType_6 = 54,
        kKeyType_7 = 55,
        kKeyType_8 = 56,
        kKeyType_9 = 57,
        kKeyType_SEMICOLON = 59,
        kKeyType_EQUAL = 61,
        kKeyType_A = 65,
        kKeyType_B = 66,
        kKeyType_C = 67,
        kKeyType_D = 68,
        kKeyType_E = 69,
        kKeyType_F = 70,
        kKeyType_G = 71,
        kKeyType_H = 72,
        kKeyType_I = 73,
        kKeyType_J = 74,
        kKeyType_K = 75,
        kKeyType_L = 76,
        kKeyType_M = 77,
        kKeyType_N = 78,
        kKeyType_O = 79,
        kKeyType_P = 80,
        kKeyType_Q = 81,
        kKeyType_R = 82,
        kKeyType_S = 83,
        kKeyType_T = 84,
        kKeyType_U = 85,
        kKeyType_V = 86,
        kKeyType_W = 87,
        kKeyType_X = 88,
        kKeyType_Y = 89,
        kKeyType_Z = 90,
        kKeyType_LEFT_BRACKET = 91,
        kKeyType_BACKSLASH = 92,
        kKeyType_RIGHT_BRACKET = 93,
        kKeyType_GRAVE_ACCENT = 96,
        kKeyType_WORLD_1 = 161,
        kKeyType_WORLD_2 = 162,
        kKeyType_ESCAPE = 256,
        kKeyType_ENTER = 257,
        kKeyType_TAB = 258,
        kKeyType_BACKSPACE = 259,
        kKeyType_INSERT = 260,
        kKeyType_DELETE = 261,
        kKeyType_RIGHT = 262,
        kKeyType_LEFT = 263,
        kKeyType_DOWN = 264,
        kKeyType_UP = 265,
        kKeyType_PAGE_UP = 266,
        kKeyType_PAGE_DOWN = 267,
        kKeyType_HOME = 268,
        kKeyType_END = 269,
        kKeyType_CAPS_LOCK = 280,
        kKeyType_SCROLL_LOCK = 281,
        kKeyType_NUM_LOCK = 282,
        kKeyType_PRINT_SCREEN = 283,
        kKeyType_PAUSE = 284,
        kKeyType_F1 = 290,
        kKeyType_F2 = 291,
        kKeyType_F3 = 292,
        kKeyType_F4 = 293,
        kKeyType_F5 = 294,
        kKeyType_F6 = 295,
        kKeyType_F7 = 296,
        kKeyType_F8 = 297,
        kKeyType_F9 = 298,
        kKeyType_F10 = 299,
        kKeyType_F11 = 300,
        kKeyType_F12 = 301,
        kKeyType_F13 = 302,
        kKeyType_F14 = 303,
        kKeyType_F15 = 304,
        kKeyType_F16 = 305,
        kKeyType_F17 = 306,
        kKeyType_F18 = 307,
        kKeyType_F19 = 308,
        kKeyType_F20 = 309,
        kKeyType_F21 = 310,
        kKeyType_F22 = 311,
        kKeyType_F23 = 312,
        kKeyType_F24 = 313,
        kKeyType_F25 = 314,
        kKeyType_KP_0 = 320,
        kKeyType_KP_1 = 321,
        kKeyType_KP_2 = 322,
        kKeyType_KP_3 = 323,
        kKeyType_KP_4 = 324,
        kKeyType_KP_5 = 325,
        kKeyType_KP_6 = 326,
        kKeyType_KP_7 = 327,
        kKeyType_KP_8 = 328,
        kKeyType_KP_9 = 329,
        kKeyType_KP_DECIMAL = 330,
        kKeyType_KP_DIVIDE = 331,
        kKeyType_KP_MULTIPLY = 332,
        kKeyType_KP_SUBTRACT = 333,
        kKeyType_KP_ADD = 334,
        kKeyType_KP_ENTER = 335,
        kKeyType_KP_EQUAL = 336,
        kKeyType_LEFT_SHIFT = 340,
        kKeyType_LEFT_CONTROL = 341,
        kKeyType_LEFT_ALT = 342,
        kKeyType_LEFT_SUPER = 343,
        kKeyType_RIGHT_SHIFT = 344,
        kKeyType_RIGHT_CONTROL = 345,
        kKeyType_RIGHT_ALT = 346,
        kKeyType_RIGHT_SUPER = 347,
        kKeyType_MENU = 348,
        kKeyType_MAX,
    };
    struct SentryDescription {
        typedef sentry_value_t (*PFN_mask_string_function)(const char *);
        typedef void (*PFN_transport_send_envelope)(sentry_envelope_t *, void *);
        const char *m_dllFilePath;
        const char *m_dsn;
        const char *m_handlerFilePath;
        const char *m_databaseDirectoryPath;
        const char *m_deviceModelName;
        const char *m_localeName;
        const char *m_rendererName;
        const char *m_clientUUID;
        bool m_isModelEditingEnabled;
        PFN_mask_string_function m_maskString;
        PFN_transport_send_envelope m_transportSendEnvelope;
        void *m_transportUserData;
    };

    static Vector2UI16 minimumRequiredWindowSize() NANOEM_DECL_NOEXCEPT;
    static void setup() NANOEM_DECL_NOEXCEPT;
    static void *openSentryDll(const SentryDescription &desc);
    static void closeSentryDll(void *handle);

    BaseApplicationService(const JSON_Value *root);
    virtual ~BaseApplicationService() NANOEM_DECL_NOEXCEPT;

    void initialize(nanoem_f32_t windowDevicePixelRatio, nanoem_f32_t viewportDevicePixelRatio);
    void destroy();

    Project *createProject(const Vector2UI16 &logicalPixelWindowSize, sg_pixel_format pixelFormat,
        nanoem_f32_t windowDevicePixelRatio, nanoem_f32_t viewportDevicePixelRatio, const char *dllPath);
    void destroyProject(Project *project);
    void draw(Project *project, Project::IViewportOverlay *overlay);

    const char *translateMenuItem(nanoem_u32_t type) const NANOEM_DECL_NOEXCEPT;
    bool dispatchMenuItemAction(Project *project, nanoem_u32_t type, Error &error);
    void setKeyPressed(KeyType key);
    void setKeyReleased(KeyType key);
    void setUnicodePressed(nanoem_u32_t value);
    void dispatchCommandMessage(const nanoem_u8_t *data, size_t size, Project *project, bool forceUndo);
    void sendLoadingAllModelIOPluginsEventMessage(int language);
    void sendLoadingAllMotionIOPluginsEventMessage(int language);

    IPrimitive2D *primitiveContext() NANOEM_DECL_NOEXCEPT;
    const IModalDialog *currentModalDialog() const NANOEM_DECL_NOEXCEPT;
    IModalDialog *currentModalDialog() NANOEM_DECL_NOEXCEPT;
    void addModalDialog(IModalDialog *value);
    void clearAllModalDialog();
    bool hasModalDialog() const NANOEM_DECL_NOEXCEPT;

    virtual sg_pixel_format defaultPassPixelFormat() const NANOEM_DECL_NOEXCEPT;
    virtual void beginDefaultPass(
        nanoem_u32_t windowID, const sg_pass_action &pa, int width, int height, int &sampleCount);
    virtual void endDefaultPass();
    virtual BaseApplicationClient *menubarApplicationClient();
    virtual IVideoRecorder *createVideoRecorder();
    virtual void destroyVideoRecorder(IVideoRecorder *recorder);
    virtual bool hasVideoRecorder() const NANOEM_DECL_NOEXCEPT;
    virtual bool isRendererAvailable(const char *value) const NANOEM_DECL_NOEXCEPT;

    const IProjectHolder *projectHolder() const NANOEM_DECL_NOEXCEPT;
    IProjectHolder *projectHolder() NANOEM_DECL_NOEXCEPT;
    DefaultFileManager *defaultFileManager() NANOEM_DECL_NOEXCEPT;

    const ITranslator *translator() const NANOEM_DECL_NOEXCEPT;
    ITranslator *translator() NANOEM_DECL_NOEXCEPT;
    void setTranslator(ITranslator *value);
    IEventPublisher *eventPublisher() NANOEM_DECL_NOEXCEPT;
    void setEventPublisher(IEventPublisher *value);
    const IFileManager *fileManager() const NANOEM_DECL_NOEXCEPT;
    IFileManager *fileManager() NANOEM_DECL_NOEXCEPT;
    void setFileManager(IFileManager *value);
    const JSON_Value *applicationConfiguration() const NANOEM_DECL_NOEXCEPT;
    JSON_Value *applicationPendingChangeConfiguration() NANOEM_DECL_NOEXCEPT;
    nanoem_u32_t defaultAuxFlags() const NANOEM_DECL_NOEXCEPT;
    void setDefaultAuxFlags(nanoem_u32_t value);
    bool isUIVisible() const NANOEM_DECL_NOEXCEPT;
    void setUIVisible(bool value);

protected:
    virtual void sendEventMessage(const Nanoem__Application__Event *event) = 0;

    virtual IAudioPlayer *createAudioPlayer();
    virtual IBackgroundVideoRenderer *createBackgroundVideoRenderer();
    virtual ICancelPublisher *createCancelPublisher();
    virtual IDebugCapture *createDebugCapture();
    virtual Project::IRendererCapability *createRendererCapability();
    virtual Project::ISkinDeformerFactory *createSkinDeformerFactory();
    virtual bool loadFromFile(const URI &fileURI, Project *project, IFileManager::DialogType type, Error &error);
    virtual bool saveAsFile(const URI &fileURI, Project *project, IFileManager::DialogType type, Error &error);
    virtual bool handleCommandMessage(Nanoem__Application__Command *command, Project *project, Error &error);
    virtual void handleSetupGraphicsEngine(sg_desc &desc);
    virtual void handleInitializeApplication();
    virtual void handleNewProject();
    virtual void handleSuspendProject();
    virtual void handleResumeProject();
    virtual void handleDestructApplication();
    virtual void handleTerminateApplication();
    virtual void postEmptyApplicationEvent();
    virtual void beginDrawContext();
    virtual void endDrawContext();
    virtual void presentDefaultPass(const Project *project);
    virtual void createDefaultRenderTarget(const Vector2UI16 &devicePixelWindowSize);
    virtual void resizeDefaultRenderTarget(const Vector2UI16 &devicePixelWindowSize, const Project *project);
    virtual void destroyDefaultRenderTarget();

    Project::ISharedCancelPublisherRepository *sharedCancelPublisherRepository() NANOEM_DECL_NOEXCEPT;
    Project::ISharedDebugCaptureRepository *sharedDebugCaptureRepository() NANOEM_DECL_NOEXCEPT;
    Project::ISharedResourceRepository *sharedResourceRepository() NANOEM_DECL_NOEXCEPT;
    nanoem_rsize_t sizeofEventMessage(const Nanoem__Application__Event *command) NANOEM_DECL_NOEXCEPT;
    void packEventMessage(const Nanoem__Application__Event *command, nanoem_u8_t *data);
    void startCapture(Project *project, const URI &fileURI, Error &error);
    void stopCapture(Project *project);
    void setProject(Project *project);
    void *resolveDllProcAddress(const char *name);
    void drawDefaultPass();

private:
    typedef tinystl::unordered_set<uint32_t, TinySTLAllocator> HandledSGXMessageSet;
    class EventPublisher;
    class Confirmer : public Project::IConfirmer {
    public:
        Confirmer(BaseApplicationService *applicationPtr);

        void seek(nanoem_frame_index_t frameIndex, Project *project) NANOEM_DECL_OVERRIDE;
        void play(Project *project) NANOEM_DECL_OVERRIDE;
        void resume(Project *project) NANOEM_DECL_OVERRIDE;

    private:
        static IModalDialog *handleSeek(void *userData, Project *project);
        static IModalDialog *handlePlay(void *userData, Project *project);
        static IModalDialog *handleResume(void *userData, Project *project);

        BaseApplicationService *m_parent;
    };
    class SharedCancelPublisherRepository : public Project::ISharedCancelPublisherRepository {
    public:
        SharedCancelPublisherRepository(BaseApplicationService *service);
        ~SharedCancelPublisherRepository() NANOEM_DECL_NOEXCEPT_OVERRIDE;

        ICancelPublisher *cancelPublisher() NANOEM_DECL_OVERRIDE;

    private:
        BaseApplicationService *m_service;
        ICancelPublisher *m_cancelPublisher;
    };
    class SharedDebugCaptureRepository : public Project::ISharedDebugCaptureRepository {
    public:
        SharedDebugCaptureRepository(BaseApplicationService *service);
        ~SharedDebugCaptureRepository() NANOEM_DECL_NOEXCEPT_OVERRIDE;

        IDebugCapture *debugCapture() NANOEM_DECL_OVERRIDE;

    private:
        BaseApplicationService *m_service;
        IDebugCapture *m_debugCapture;
    };
    class SharedResourceRepository : public Project::ISharedResourceRepository {
    public:
        SharedResourceRepository();
        ~SharedResourceRepository() NANOEM_DECL_NOEXCEPT_OVERRIDE;

        void destroy();

        AccessoryProgramBundle *accessoryProgramBundle() NANOEM_DECL_OVERRIDE;
        ModelProgramBundle *modelProgramBundle() NANOEM_DECL_OVERRIDE;
        effect::GlobalUniform *effectGlobalUniform() NANOEM_DECL_OVERRIDE;
        const IImageView *toonImage(int value) NANOEM_DECL_OVERRIDE;
        Vector4 toonColor(int value) NANOEM_DECL_OVERRIDE;

    private:
        AccessoryProgramBundle *m_accessoryProgramBundle;
        ModelProgramBundle *m_modelProgramBundle;
        effect::GlobalUniform *m_effectGlobalUniform;
        Image *m_sharedToonImages[10];
        ByteArray m_sharedToonImageData[10];
        Vector4 m_sharedToonColors[10];
        bool m_sharedToonImagesInitialized;
        bool m_sharedToonColorsInitialized;
    };
    class UnicodeStringFactoryRepository : public Project::IUnicodeStringFactoryRepository {
    public:
        UnicodeStringFactoryRepository();
        ~UnicodeStringFactoryRepository() NANOEM_DECL_NOEXCEPT_OVERRIDE;

        nanoem_unicode_string_factory_t *unicodeStringFactory() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

    private:
        nanoem_unicode_string_factory_t *m_factory;
    };

    static IModalDialog *handleSaveOnNewProject(void *userData, Project *project);
    static IModalDialog *handleDiscardOnNewProject(void *userData, Project *project);
    static IModalDialog *handleSaveOnOpenProject(void *userData, Project *project);
    static IModalDialog *handleDiscardOnOpenProject(void *userData, Project *project);
    static IModalDialog *handleSaveOnExitApplication(void *userData, Project *project);
    static IModalDialog *handleDiscardOnExitApplication(void *userData, Project *project);
    static IModalDialog *handleCancelRecordingVideo(void *userData, Project *project);
    static void *allocateSGXMemory(void *opaque, size_t size, const char *file, int line);
    static void releaseSGXMemory(void *opaque, void *ptr, const char *file, int line) NANOEM_DECL_NOEXCEPT;
    static void handleSGXMessage(void *opaque, const char *message, const char *file, int line);
    static void writeRedoMessage(Nanoem__Application__Command *command, Project *project, Error &error);
    static void performRedo(undo_command_t *commandPtr, undo_stack_t *stack, undo_command_t *&commandPtrRef);

    void sendQueryEventMessage(Nanoem__Application__Event *event, const Nanoem__Application__Command *command);
    void sendSaveAfterConfirmEventMessage();
    void sendDiscardAfterConfirmEventMessage();

    const JSON_Value *m_applicationConfiguration;
    DefaultFileManager *m_defaultFileManager;
    EventPublisher *m_eventPublisher;
    StateController *m_stateController;
    IFileManager *m_fileManagerPtr;
    IEventPublisher *m_eventPublisherPtr;
    ITranslator *m_translatorPtr;
    JSON_Value *m_applicationPendingChangeConfiguration;
    internal::CapturingPassState *m_capturingPassState;
    internal::IUIWindow *m_window;
    Confirmer m_confirmer;
    HandledSGXMessageSet m_handledSGXMessages;
    SharedCancelPublisherRepository m_sharedCancelPublisherRepository;
    SharedDebugCaptureRepository m_sharedDebugCaptureRepository;
    SharedResourceRepository m_sharedResourceRepository;
    UnicodeStringFactoryRepository m_unicodeStringFactoryRepository;
    nanoem_u32_t m_defaultAuxFlags;
    sg_context m_context;
    void *m_dllHandle;
    bool m_initialized;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_BASEAPPLICATIONSERVICE_H_ */
