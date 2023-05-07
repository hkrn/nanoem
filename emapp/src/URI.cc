/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/URI.h"

#include "emapp/StringUtils.h"

#include "bx/hash.h"

namespace nanoem {

String
URI::stringByDeletingTrailingPathSeparator(const String &value)
{
    if (*(value.c_str() + value.size() - 1) == '/') {
        return String(value.c_str(), value.size() - 1);
    }
    return value;
}

String
URI::stringByDeletingLastPathComponent(const String &value)
{
    if (const char *p = strrchr(value.c_str(), '/')) {
        return String(value.c_str(), size_t(p - value.c_str()));
    }
    return String();
}

String
URI::stringByDeletingPathExtension(const String &value)
{
    if (const char *p = strrchr(value.c_str(), '/')) {
        if (const char *q = strrchr(p + 1, '.')) {
            return String(value.c_str(), size_t(q - value.c_str()));
        }
    }
    else if (const char *p2 = strrchr(value.c_str(), '.')) {
        return String(value.c_str(), size_t(p2 - value.c_str()));
    }
    return value;
}

String
URI::pathExtension(const String &value)
{
    const String path(lastPathComponent(value));
    if (const char *p = strrchr(path.c_str(), '.')) {
        return String(p + 1);
    }
    return String();
}

String
URI::lastPathComponent(const String &value)
{
    return lastPathComponentConstString(value);
}

const char *
URI::lastPathComponentConstString(const String &value)
{
    const char *p = strrchr(value.c_str(), '/');
    return p ? p + 1 : value.c_str();
}

URI
URI::createFromFilePath(const String &path)
{
    return URI(path, String());
}

URI
URI::createFromFilePath(const String &path, const String &fragment)
{
    return URI(path, fragment);
}

URI::URI()
{
}

URI::URI(const URI &value)
    : m_absolutePath(value.m_absolutePath)
    , m_fragment(value.m_fragment)
{
}

URI::~URI() NANOEM_DECL_NOEXCEPT
{
}

String
URI::absolutePath() const
{
    return m_absolutePath;
}

const char *
URI::absolutePathConstString() const NANOEM_DECL_NOEXCEPT
{
    return m_absolutePath.c_str();
}

String
URI::fragment() const
{
    return m_fragment;
}

const char *
URI::fragmentConstString() const NANOEM_DECL_NOEXCEPT
{
    return m_fragment.c_str();
}

String
URI::absolutePathByDeletingLastPathComponent() const
{
    return stringByDeletingLastPathComponent(m_absolutePath);
}

String
URI::absolutePathByDeletingPathExtension() const
{
    return stringByDeletingPathExtension(m_absolutePath);
}

String
URI::lastPathComponent() const
{
    return lastPathComponent(m_absolutePath);
}

const char *
URI::lastPathComponentConstString() const NANOEM_DECL_NOEXCEPT
{
    return lastPathComponentConstString(m_absolutePath);
}

String
URI::pathExtension() const
{
    return pathExtension(m_absolutePath);
}

nanoem_u32_t
URI::hash() const NANOEM_DECL_NOEXCEPT
{
    bx::HashMurmur2A hasher;
    hasher.begin();
    hasher.add(m_absolutePath.c_str(), m_absolutePath.size());
    hasher.add(m_fragment.c_str(), m_fragment.size());
    return hasher.end();
}

bool
URI::isEmpty() const NANOEM_DECL_NOEXCEPT
{
    return m_absolutePath.empty() && m_fragment.empty();
}

bool
URI::hasFragment() const
{
    return !m_fragment.empty();
}

bool
URI::equalsTo(const URI &other) const NANOEM_DECL_NOEXCEPT
{
    return m_absolutePath == other.m_absolutePath && m_fragment == other.m_fragment;
}

bool
URI::equalsToAbsolutePath(const String &other) const NANOEM_DECL_NOEXCEPT
{
    return m_absolutePath == other;
}

bool
URI::equalsToAbsolutePathConstString(const char *other) const NANOEM_DECL_NOEXCEPT
{
    return StringUtils::equals(m_absolutePath.c_str(), other);
}

bool
URI::equalsToFilenameConstString(const char *other) const NANOEM_DECL_NOEXCEPT
{
    const char *p = lastPathComponentConstString(m_absolutePath);
    return p ? StringUtils::equalsIgnoreCase(p, other) : false;
}

bool
URI::operator==(const URI &value) const NANOEM_DECL_NOEXCEPT
{
    return StringUtils::equals(absolutePathConstString(), value.absolutePathConstString()) &&
        StringUtils::equals(fragmentConstString(), value.fragmentConstString());
}

URI::URI(const String &path, const String &fragment)
    : m_absolutePath(path)
    , m_fragment(fragment)
{
}

} /* namespace nanoem */
