/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_PROJECT_ARCHIVE_H_
#define NANOEM_EMAPP_INTERNAL_PROJECT_ARCHIVE_H_

#include "emapp/Archiver.h"
#include "emapp/PluginFactory.h"
#include "emapp/Project.h"
#include "emapp/internal/project/Native.h"

namespace nanoem {

class Error;
class IDrawable;
class Motion;
class Project;
class Progress;
class URI;

namespace internal {

class Native;

namespace project {

class Archive NANOEM_DECL_SEALED : private NonCopyable {
public:
    static const char *const kManifestEntryPath;
    static const char *const kManifestCompatEntryPath;
    static const char *const kBGMEntryPath;

    Archive(Project *project, const URI &fileURI);
    ~Archive() NANOEM_DECL_NOEXCEPT;

    bool load(ISeekableReader *reader, Error &error);
    bool save(ISeekableWriter *writer, Error &error);

private:
    typedef tinystl::unordered_map<Motion *, IDrawable *, TinySTLAllocator> MotionDrawableMap;
    typedef tinystl::unordered_map<IDrawable *, String, TinySTLAllocator> DrawablePathMap;

    bool loadAudio(Native &native, Error &error);
    bool loadVideo(Native &native, Error &error);
    bool loadAllAccessories(const Archiver::EntryList &accessoryList, Error &error);
    bool loadAccessory(const String &entryPath, Error &error);
    bool loadAllModels(const Archiver::EntryList &modelList, Error &error);
    bool loadModel(const String &entryPath, Error &error);
    bool loadAllMotions(const Archiver::EntryList &motionList, Error &error);
    bool loadMotion(const Archiver::Entry &entry, Error &error);
    bool loadAllEffects(Native &native, Error &error);
    bool loadAllOffscreenEffectAttachments(Native &native, plugin::EffectPlugin *plugin, Error &error);
    bool loadOffscreenEffectAttachment(
        const Native::OffscreenRenderTargetEffectAttachment &attachment, plugin::EffectPlugin *plugin, Error &error);
    bool loadEffectFromSource(
        const URI &fileURI, IDrawable *drawable, plugin::EffectPlugin *plugin, Native &native, Error &error);
    bool loadEffectFromBinary(const URI &fileURI, IDrawable *drawable, Error &error);
    void aggregateAllIncludeEffectSources(
        const StringList &extensionList, Project::IncludeEffectSourceMap &includeEffectSources, Error &error);
    void addIncludeEffectSource(const IDrawable *drawable, const String &basePath, Native &native,
        PluginFactory::EffectPluginProxy &proxy, Error &error);
    bool saveAllAccessories(Native &native, StringSet &reservedNameSet, Error &error);
    bool saveAllModels(Native &native, StringSet &reservedNameSet, Error &error);
    bool saveAllMotions(Error &error);
    bool saveAudio(Error &error);
    bool saveVideo(Error &error);
    bool saveAllDrawableIncludeSources(IDrawable *drawable, Native &native, Error &error);

    const URI m_fileURI;
    Project *m_project;
    Archiver *m_archiver;
    Progress *m_progress;
    MotionDrawableMap m_accessoryMotionSet;
    MotionDrawableMap m_modelMotionSet;
    DrawablePathMap m_pathDrawables;
};

} /* namespace project */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_PROJECT_ARCHIVE_H_ */
