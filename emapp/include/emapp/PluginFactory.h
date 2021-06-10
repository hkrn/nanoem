/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_PLUGINFACTORY_H_
#define NANOEM_EMAPP_PLUGINFACTORY_H_

#include "emapp/Error.h"
#include "emapp/IEventPublisher.h"
#include "emapp/Motion.h"

struct undo_command_t;

namespace nanoem {

class IAudioPlayer;
class Error;
class Project;
class URI;

namespace plugin {
class DecoderPlugin;
class EffectPlugin;
class EncoderPlugin;
class ModelIOPlugin;
class MotionIOPlugin;
} /* namespace plugin */

class PluginFactory NANOEM_DECL_SEALED : private NonCopyable {
public:
    class DecoderPluginProxy NANOEM_DECL_SEALED : private NonCopyable {
    public:
        DecoderPluginProxy(plugin::DecoderPlugin *plugin);
        ~DecoderPluginProxy() NANOEM_DECL_NOEXCEPT;

        bool loadAudio(const URI &fileURI, nanoem_u32_t fps, Error &error);
        bool loadVideo(const URI &fileURI, nanoem_u32_t fps, Error &error);
        bool decodeAudioFrame(nanoem_frame_index_t frameIndex, ByteArray &bytes, Error &error);
        bool decodeVideoFrame(nanoem_frame_index_t frameIndex, ByteArray &bytes, Error &error);

        nanoem_u32_t numBits() const NANOEM_DECL_NOEXCEPT;
        nanoem_u32_t numChannels() const NANOEM_DECL_NOEXCEPT;
        nanoem_u32_t frequency() const NANOEM_DECL_NOEXCEPT;
        nanoem_u32_t width() const NANOEM_DECL_NOEXCEPT;
        nanoem_u32_t height() const NANOEM_DECL_NOEXCEPT;
        nanoem_frame_index_t duration() const NANOEM_DECL_NOEXCEPT;
        StringList availableDecodingAudioExtensions() const;
        StringList availableDecodingVideoExtensions() const;
        Error error() const;

    private:
        plugin::DecoderPlugin *m_plugin;
    };
    class EncoderPluginProxy NANOEM_DECL_SEALED : private NonCopyable {
    public:
        EncoderPluginProxy(plugin::EncoderPlugin *plugin);
        ~EncoderPluginProxy() NANOEM_DECL_NOEXCEPT;

        void setSize(const glm::ivec2 &value, Error &error);
        bool getUIWindowLayout(ByteArray &value, Error &error);
        void setUIComponentLayout(const char *id, const ByteArray &value, bool &reloadLayout, Error &error);
        void cancel(Error &error);

        StringList availableEncodingVideoFormatExtensions() const;
        String name() const;
        Error error() const;

    private:
        plugin::EncoderPlugin *m_plugin;
    };
    class EffectPluginProxy NANOEM_DECL_SEALED : private NonCopyable {
    public:
        EffectPluginProxy(plugin::EffectPlugin *plugin);
        ~EffectPluginProxy() NANOEM_DECL_NOEXCEPT;

        bool compile(const URI &fileURI, ByteArray &output);
        bool compile(const String &source, ByteArray &output);
        void addIncludeSource(const String &path, const ByteArray &bytes);
        StringList availableExtensions() const;
        Error error() const;
        void setShaderVersion(int value, Error &error);
        void setMipmapEnabled(bool value, Error &error);

    private:
        plugin::EffectPlugin *m_plugin;
    };
    class BaseIOPluginProxy : private NonCopyable {
    public:
        virtual ~BaseIOPluginProxy() NANOEM_DECL_NOEXCEPT
        {
        }
        virtual String name() const = 0;
        virtual String description() const = 0;
        virtual String version() const = 0;
        virtual void setLanguage(int value) = 0;
        virtual int countAllFunctions() const NANOEM_DECL_NOEXCEPT = 0;
        virtual String functionName(int value) const = 0;
        virtual void setFunction(int value, Error &error) = 0;
        virtual void setProjectDescription(Project *project, Error &error) = 0;
        virtual void setInputData(const ByteArray &bytes, Error &error) = 0;
        virtual bool getUIWindowLayout(ByteArray &value, Error &error) = 0;
        virtual void setUIComponentLayout(const char *id, const ByteArray &value, bool &reloadLayout, Error &error) = 0;
    };
    class ModelIOPluginProxy NANOEM_DECL_SEALED : public BaseIOPluginProxy {
    public:
        typedef tinystl::vector<int, TinySTLAllocator> IntList;

        ModelIOPluginProxy(plugin::ModelIOPlugin *plugin);
        ~ModelIOPluginProxy() NANOEM_DECL_NOEXCEPT;

        String name() const NANOEM_DECL_OVERRIDE;
        String description() const NANOEM_DECL_OVERRIDE;
        String version() const NANOEM_DECL_OVERRIDE;
        void setLanguage(int value) NANOEM_DECL_OVERRIDE;
        int countAllFunctions() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
        String functionName(int value) const NANOEM_DECL_OVERRIDE;
        void setFunction(int value, Error &error) NANOEM_DECL_OVERRIDE;
        void setProjectDescription(Project *project, Error &error) NANOEM_DECL_OVERRIDE;
        void setInputData(const ByteArray &bytes, Error &error) NANOEM_DECL_OVERRIDE;
        bool getUIWindowLayout(ByteArray &value, Error &error) NANOEM_DECL_OVERRIDE;
        void setUIComponentLayout(
            const char *id, const ByteArray &value, bool &reloadLayout, Error &error) NANOEM_DECL_OVERRIDE;

        void setBackupEnabled(bool value);
        void setupSelection(const Model *model, Error &error);
        void setupEditingMask(const Model *model, Error &error);
        void setAllSelectedVertexObjectIndices(const IntList &value, Error &error);
        void setAllSelectedMaterialObjectIndices(const IntList &value, Error &error);
        void setAllSelectedBoneObjectIndices(const IntList &value, Error &error);
        void setAllSelectedMorphObjectIndices(const IntList &value, Error &error);
        void setAllSelectedLabelObjectIndices(const IntList &value, Error &error);
        void setAllSelectedRigidBodyObjectIndices(const IntList &value, Error &error);
        void setAllSelectedJointObjectIndices(const IntList &value, Error &error);
        void execute(int functionIndex, const ByteArray &input, Model *model, Project *project, Error &error);

    private:
        plugin::ModelIOPlugin *m_plugin;
        bool m_enableBackup;
    };
    class MotionIOPluginProxy NANOEM_DECL_SEALED : public BaseIOPluginProxy {
    public:
        MotionIOPluginProxy(plugin::MotionIOPlugin *plugin);
        ~MotionIOPluginProxy() NANOEM_DECL_NOEXCEPT;

        void setLanguage(int value) NANOEM_DECL_OVERRIDE;
        String name() const NANOEM_DECL_OVERRIDE;
        String description() const NANOEM_DECL_OVERRIDE;
        String version() const NANOEM_DECL_OVERRIDE;
        int countAllFunctions() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
        String functionName(int value) const NANOEM_DECL_OVERRIDE;
        void setFunction(int value, Error &error) NANOEM_DECL_OVERRIDE;
        void setProjectDescription(Project *project, Error &error) NANOEM_DECL_OVERRIDE;
        void setInputData(const ByteArray &bytes, Error &error) NANOEM_DECL_OVERRIDE;
        bool getUIWindowLayout(ByteArray &value, Error &error) NANOEM_DECL_OVERRIDE;
        void setUIComponentLayout(
            const char *id, const ByteArray &value, bool &reloadLayout, Error &error) NANOEM_DECL_OVERRIDE;

        void setupAccessorySelection(const Motion *activeMotion, Error &error);
        void setupModelSelection(const Motion *motion, const Model *model, Error &error);
        void setAllNamedSelectedBoneKeyframes(const String &name, const Motion::FrameIndexList &indices, Error &error);
        void setAllNamedSelectedMorphKeyframes(const String &name, const Motion::FrameIndexList &indices, Error &error);
        void setAllSelectedAccessoryKeyframes(const Motion::FrameIndexList &indices, Error &error);
        void setAllSelectedCameraKeyframes(const Motion::FrameIndexList &indices, Error &error);
        void setAllSelectedLightKeyframes(const Motion::FrameIndexList &indices, Error &error);
        void setAllSelectedModelKeyframes(const Motion::FrameIndexList &indices, Error &error);
        void setAllSelectedSelfShadowKeyframes(const Motion::FrameIndexList &indices, Error &error);
        void execute(int functionIndex, const ByteArray &input, Project *project, Error &error);

    private:
        undo_command_t *execute(
            const ByteArray &input, Motion *motion, const Model *model, nanoem_u32_t flags, Error &error);

        plugin::MotionIOPlugin *m_plugin;
    };

    static plugin::EncoderPlugin *createEncoderPlugin(const URI &fileURI, IEventPublisher *publisher);
    static plugin::DecoderPlugin *createDecoderPlugin(const URI &fileURI, IEventPublisher *publisher);
    static plugin::EffectPlugin *createEffectPlugin(const URI &fileURI, IEventPublisher *publisher, Error &error);
    static plugin::ModelIOPlugin *createModelIOPlugin(const URI &fileURI, IEventPublisher *publisher);
    static plugin::MotionIOPlugin *createMotionIOPlugin(const URI &fileURI, IEventPublisher *publisher);
    static void destroyDecoderPlugin(plugin::DecoderPlugin *plugin);
    static void destroyEncoderPlugin(plugin::EncoderPlugin *plugin);
    static void destroyEffectPlugin(plugin::EffectPlugin *plugin);
    static void destroyModelIOPlugin(plugin::ModelIOPlugin *plugin);
    static void destroyMotionIOPlugin(plugin::MotionIOPlugin *plugin);
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_PLUGINFACTORY_H_ */
