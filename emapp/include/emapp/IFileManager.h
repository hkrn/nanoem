/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_IFILEMANAGER_H_
#define NANOEM_EMAPP_IFILEMANAGER_H_

#include "emapp/URI.h"

namespace nanoem {

namespace plugin {
class DecoderPlugin;
class EffectPlugin;
class EncoderPlugin;
} /* namespace */

class Accessory;
class Effect;
class Error;
class Model;
class Project;

class IFileManager {
public:
    struct QueryFileDialogCallbacks {
        typedef void (*AcceptCallback)(const URI &fileURI, Project *project, Error &error, void *opaque);
        typedef void (*CancelCallback)(Project *project, void *opaque);
        typedef void (*DestroyCallback)(void *userData);
        void *m_opaque;
        AcceptCallback m_accept;
        CancelCallback m_cancel;
        DestroyCallback m_destory;
    };
    typedef tinystl::vector<plugin::DecoderPlugin *, TinySTLAllocator> DecoderPluginList;
    typedef tinystl::vector<plugin::EncoderPlugin *, TinySTLAllocator> EncoderPluginList;

    enum DialogType {
        kDialogTypeFirstEnum,
        kDialogTypeOpenProject = kDialogTypeFirstEnum,
        kDialogTypeOpenModelFile,
        kDialogTypeOpenModelMotionFile,
        kDialogTypeOpenCameraMotionFile,
        kDialogTypeOpenLightMotionFile,
        kDialogTypeOpenAudioFile,
        kDialogTypeOpenVideoFile,
        kDialogTypeOpenEffectFile,
        kDialogTypeSaveProjectFile,
        kDialogTypeSaveModelFile,
        kDialogTypeSaveCameraMotionFile,
        kDialogTypeSaveLightMotionFile,
        kDialogTypeSaveModelMotionFile,
        kDialogTypeLoadModelFile,
        kDialogTypeUserCallback,
        kDialogTypeMaxEnum
    };

    virtual ~IFileManager() NANOEM_DECL_NOEXCEPT
    {
    }

    virtual bool loadFromFile(const URI &fileURI, DialogType type, Project *project, Error &error) = 0;
    virtual bool loadFromFileWithHandle(
        const URI &fileURI, nanoem_u16_t handle, DialogType type, Project *project, Error &error) = 0;
    virtual bool loadFromFileWithModalDialog(const URI &fileURI, DialogType type, Project *project, Error &error) = 0;
    virtual bool saveAsFile(const URI &fileURI, DialogType type, Project *project, Error &error) = 0;
    virtual bool hasTransientQueryFileDialogCallback() const NANOEM_DECL_NOEXCEPT = 0;
    virtual void setTransientQueryFileDialogCallback(QueryFileDialogCallbacks callbacks) = 0;
    virtual void resetTransientQueryFileDialogCallback() = 0;

    virtual URI sharedSourceEffectCacheDirectory() = 0;
    virtual plugin::EffectPlugin *sharedEffectPlugin() = 0;

    virtual bool loadAudioFile(const URI &fileURI, Project *project, Error &error) = 0;
    virtual bool loadVideoFile(const URI &fileURI, Project *project, Error &error) = 0;
    virtual DecoderPluginList resolveAudioDecoderPluginList(const char *extension) const = 0;
    virtual DecoderPluginList resolveVideoDecoderPluginList(const char *extension) const = 0;
    virtual EncoderPluginList allVideoEncoderPluginList() const = 0;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_IFILEMANAGER_H_ */
