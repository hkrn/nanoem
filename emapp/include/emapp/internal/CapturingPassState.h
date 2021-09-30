/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_CAPTURINGPASSSTATE_H_
#define NANOEM_EMAPP_INTERNAL_CAPTURINGPASSSTATE_H_

#include "emapp/Error.h"
#include "emapp/ModalDialogFactory.h"
#include "emapp/Project.h"
#include "emapp/URI.h"

#include "bx/mutex.h"

namespace nanoem {

class BaseApplicationService;
class IFileManager;
class IVideoRecorder;
class StateController;

namespace plugin {
class DecoderPlugin;
class EncoderPlugin;
} /* namespace plugin */

namespace internal {

class CapturingPassState : private NonCopyable {
public:
    static const char kExportResolutionKey[];
    static const char kExportSampleLevelKey[];

    template <typename TItem>
    static inline StringList
    getStringList(const tinystl::vector<tinystl::pair<TItem, String>, TinySTLAllocator> &items)
    {
        StringList strings;
        for (typename tinystl::vector<tinystl::pair<TItem, String>, TinySTLAllocator>::const_iterator
                 it = items.begin(),
                 end = items.end();
             it != end; ++it) {
            strings.push_back(it->second);
        }
        return strings;
    }

    CapturingPassState(StateController *stateControllerPtr, Project *project);
    virtual ~CapturingPassState() NANOEM_DECL_NOEXCEPT;

    virtual bool start(Error &error) = 0;
    virtual IModalDialog *createDialog() = 0;
    virtual bool capture(Project *project, Error &error) = 0;
    virtual void cancel() = 0;

    virtual void save(Project *project);
    virtual void restore(Project *project);

    bool transitDestruction(Project *project);
    void setOutputImageSize(const Vector2UI16 &value);
    void setPixelFormat(sg_pixel_format value);
    void setStartFrameIndex(nanoem_frame_index_t value);
    void setSampleLevel(nanoem_u32_t value);
    void setViewportAspectRatioEnabled(bool value);
    void setPreventFrameMisalighmentEnabled(bool value);
    URI fileURI() const;
    void setFileURI(const URI &value);

protected:
    class BaseModalDialog;
    class ImageBlitter;
    enum StateTransition {
        kFirstEnum,
        kNone = kFirstEnum,
        kInitialized,
        kConfigured,
        kReady,
        kBlitted,
        kFinished,
        kDestroyReady,
        kCancelled,
        kRestored,
        kDestroyed,
        kMaxEnum,
    };

    bool prepareCapturingViewport(Project *project);
    void readPassImage();
    void blitOutputPass();
    void incrementAsyncCount();
    void decrementAsyncCount();
    bool canBlitTransition();
    virtual void destroy();

    StateController *stateController();
    const ByteArray &frameImageData() const;
    nanoem_u8_t *mutableFrameImageDataPtr();
    bx::Mutex &mutex();
    sg_image_desc outputImageDescription() const;
    StateTransition stateTransition();
    void setStateTransition(StateTransition value);
    sg_pass outputPass() const;
    sg_buffer frameStagingBuffer() const;
    nanoem_frame_index_t startFrameIndex() const;
    nanoem_frame_index_t lastVideoPTS() const;
    void setLastVideoPTS(nanoem_frame_index_t value);
    bool hasSaveState() const;

private:
    URI m_fileURI;
    StateController *m_stateControllerPtr;
    ImageBlitter *m_blitter;
    Project *m_project;
    Project::SaveState *m_saveState;
    StateTransition m_state;
    bx::Mutex m_mutex;
    ByteArray m_frameImageData;
    sg_buffer m_frameStagingBuffer;
    Vector2UI16 m_lastLogicalScaleUniformedViewportImageSize;
    sg_pass m_outputPass;
    sg_pass_desc m_outputPassDescription;
    sg_image_desc m_outputImageDescription;
    nanoem_f32_t m_lastViewportDevicePixelRatio;
    nanoem_u32_t m_lastPreferredMotionFPS;
    nanoem_u32_t m_lastSampleLevel;
    nanoem_u32_t m_sampleLevel;
    nanoem_frame_index_t m_lastVideoPTS;
    nanoem_frame_index_t m_startFrameIndex;
    volatile int m_asyncCount;
    int m_blittedCount;
    bool m_viewportAspectRatioEnabled;
    bool m_displaySyncDisabled;
    bool m_preventFrameMisalighmentEnabled;
};

class CapturingPassAsImageState NANOEM_DECL_SEALED : public CapturingPassState {
public:
    CapturingPassAsImageState(StateController *stateControllerPtr, Project *project);
    ~CapturingPassAsImageState() NANOEM_DECL_NOEXCEPT_OVERRIDE;

    bool start(Error &error) NANOEM_DECL_OVERRIDE;
    IModalDialog *createDialog() NANOEM_DECL_OVERRIDE;
    bool capture(Project *project, Error &error) NANOEM_DECL_OVERRIDE;
    void cancel() NANOEM_DECL_OVERRIDE;

    void addExportImageDialog(Project *project);

private:
    class ModalDialog;

    static IModalDialog *handleCancelExportingImage(void *userData, Project *project);
};

class CapturingPassAsVideoState NANOEM_DECL_SEALED : public CapturingPassState {
public:
    static const char kExportVideoEnableNativeKey[];
    static const char kExportAudioCodecKey[];
    static const char kExportVideoCodecKey[];
    static const char kExportVideoFormatKey[];
    static const char kExportVideoProfileKey[];
    static const char kExportVideoRangeKey[];

    CapturingPassAsVideoState(StateController *stateControllerPtr, Project *project);
    ~CapturingPassAsVideoState() NANOEM_DECL_NOEXCEPT_OVERRIDE;

    bool start(Error &error) NANOEM_DECL_OVERRIDE;
    IModalDialog *createDialog() NANOEM_DECL_OVERRIDE;
    bool capture(Project *project, Error &error) NANOEM_DECL_OVERRIDE;
    void cancel() NANOEM_DECL_OVERRIDE;
    void restore(Project *project) NANOEM_DECL_OVERRIDE;

    void setEncoderPlugin(plugin::EncoderPlugin *value);
    void setEndFrameIndex(nanoem_frame_index_t value);
    void addExportVideoDialog(Project *project);

private:
    class ModalDialog;

    static IModalDialog *handleCancelExportingVideo(void *userData, Project * /* project */);
    static void calculateFrameIndex(
        nanoem_f32_t deltaScaleFactor, nanoem_f32_t &amount, nanoem_frame_index_t &frameIndex);

    nanoem_frame_index_t duration() const NANOEM_DECL_NOEXCEPT;
    void handleCaptureViaVideoRecorder(Project *project, nanoem_frame_index_t frameIndex, nanoem_frame_index_t audioPTS,
        nanoem_frame_index_t videoPTS, nanoem_frame_index_t durationFrameIndices, nanoem_f32_t deltaScaleFactor);
    void handleCaptureViaEncoderPlugin(Project *project, nanoem_frame_index_t frameIndex, nanoem_frame_index_t audioPTS,
        nanoem_frame_index_t videoPTS, nanoem_frame_index_t durationFrameIndices, nanoem_f32_t deltaScaleFactor,
        Error &error);
    bool encodeVideoFrame(
        const ByteArray &frameData, nanoem_frame_index_t audioPTS, nanoem_frame_index_t videoPTS, Error &error);
    void seekAndProgress(Project *project, nanoem_frame_index_t frameIndex, nanoem_frame_index_t durationFrameIndices);
    void finishEncoding();
    void stopEncoding(Error &error);
    void destroy() NANOEM_DECL_OVERRIDE;

    plugin::EncoderPlugin *m_encoderPluginPtr;
    IVideoRecorder *m_videoRecorder;
    IVideoRecorder *m_destroyingVideoRecorder;
    nanoem_frame_index_t m_endFrameIndex;
    nanoem_f32_t m_amount;
};

} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_INTERNAL_CAPTURINGPASSSTATE_H_ */
