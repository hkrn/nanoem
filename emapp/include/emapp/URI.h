/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_URI_H_
#define NANOEM_EMAPP_URI_H_

#include "emapp/Forward.h"

namespace nanoem {

class URI NANOEM_DECL_SEALED {
public:
    static String stringByDeletingTrailingPathSeparator(const String &value);
    static String stringByDeletingLastPathComponent(const String &value);
    static String stringByDeletingPathExtension(const String &value);
    static String lastPathComponent(const String &value);
    static const char *lastPathComponentConstString(const String &value);
    static String pathExtension(const String &value);
    static URI createFromFilePath(const String &path);
    static URI createFromFilePath(const String &path, const String &fragment);

    explicit URI();
    URI(const URI &value);
    ~URI() NANOEM_DECL_NOEXCEPT;

    String absolutePath() const;
    const char *absolutePathConstString() const NANOEM_DECL_NOEXCEPT;
    String fragment() const;
    const char *fragmentConstString() const NANOEM_DECL_NOEXCEPT;
    String absolutePathByDeletingLastPathComponent() const;
    String absolutePathByDeletingPathExtension() const;
    String lastPathComponent() const;
    const char *lastPathComponentConstString() const NANOEM_DECL_NOEXCEPT;
    String pathExtension() const;
    bool isEmpty() const NANOEM_DECL_NOEXCEPT;
    bool hasFragment() const;
    bool equalsTo(const URI &other) const NANOEM_DECL_NOEXCEPT;
    bool equalsToAbsolutePath(const String &other) const NANOEM_DECL_NOEXCEPT;
    bool equalsToAbsolutePathConstString(const char *other) const NANOEM_DECL_NOEXCEPT;
    bool equalsToFilenameConstString(const char *other) const NANOEM_DECL_NOEXCEPT;

private:
    URI(const String &path, const String &fragment);

    String m_absolutePath;
    String m_fragment;
};

typedef tinystl::vector<URI, TinySTLAllocator> URIList;
typedef tinystl::pair<String, URI> FileEntity;
typedef tinystl::vector<FileEntity, TinySTLAllocator> FileEntityList;
typedef tinystl::unordered_map<String, URI, TinySTLAllocator> FileEntityMap;

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_URI_H_ */
