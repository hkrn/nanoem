/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_PROJECT_NATIVE_H_
#define NANOEM_EMAPP_INTERNAL_PROJECT_NATIVE_H_

#include "emapp/Project.h"

namespace nanoem {

class IDrawable;
class Project;

namespace internal {
namespace project {

class Archive;

class Native NANOEM_DECL_SEALED : private NonCopyable {
public:
    struct OffscreenRenderTargetEffectAttachment {
        IDrawable *m_target;
        String m_name;
        String m_filePath;
        StringList m_includePaths;
    };
    enum FileType {
        kFileTypeFirstEnum,
        kFileTypeArchive = kFileTypeFirstEnum,
        kFileTypeData,
        kFileTypeMaxEnum,
    };
    typedef tinystl::vector<OffscreenRenderTargetEffectAttachment, TinySTLAllocator>
        OffscreenRenderTargetEffectAttachmentList;

    Native(Project *project);
    ~Native() NANOEM_DECL_NOEXCEPT;

    bool load(const nanoem_u8_t *data, size_t size, FileType type, Error &error, Project::IDiagnostics *diagsnotics);
    bool save(ByteArray &bytes, FileType type, Error &error);

    const Project::IncludeEffectSourceMap *findIncludeEffectSource(const IDrawable *drawable) const;
    Project::IncludeEffectSourceMap *findMutableIncludeEffectSource(const IDrawable *drawable);
    void getAllOffscreenRenderTargetEffectAttachments(OffscreenRenderTargetEffectAttachmentList &value) const;
    String findAnnotation(const String &name) const;
    void setAnnotation(const String &name, const String &value);
    nanoem_motion_format_type_t defaultSaveMotionFormat() const;
    void setDefaultSaveMotionFormat(nanoem_motion_format_type_t value);
    URI audioURI() const;
    URI videoURI() const;

private:
    struct Context;
    Context *m_context;
};

} /* namespace project */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_PROJECT_NATIVE_H_ */
