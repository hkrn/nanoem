/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_LISTUTILS_H_
#define NANOEM_EMAPP_LISTUTILS_H_

#include "emapp/Forward.h"

namespace nanoem {

class ListUtils NANOEM_DECL_SEALED : private NonCopyable {
public:
    template <typename T>
    static inline tinystl::vector<T, TinySTLAllocator>
    toListFromSet(const tinystl::unordered_set<T, TinySTLAllocator> &set)
    {
        tinystl::vector<T, TinySTLAllocator> list;
        for (typename tinystl::unordered_set<T, TinySTLAllocator>::const_iterator it = set.begin(), end = set.end();
             it != end; ++it) {
            list.push_back(*it);
        }
        return list;
    }
    template <typename T>
    static inline tinystl::unordered_set<T, TinySTLAllocator>
    toSetFromList(const tinystl::vector<T, TinySTLAllocator> &list)
    {
        tinystl::unordered_set<T, TinySTLAllocator> set;
        for (typename tinystl::vector<T, TinySTLAllocator>::const_iterator it = list.begin(), end = list.end();
             it != end; ++it) {
            set.insert(*it);
        }
        return set;
    }
    template <typename T>
    static inline bool
    contains(const T &item, const tinystl::vector<T, TinySTLAllocator> &list) NANOEM_DECL_NOEXCEPT
    {
        bool found = false;
        for (typename tinystl::vector<T, TinySTLAllocator>::const_iterator it = list.begin(), end = list.end();
             it != end; ++it) {
            found = *it == item;
            if (found) {
                break;
            }
        }
        return found;
    }
    template <typename T>
    static inline int
    indexOf(const T &item, const tinystl::vector<T, TinySTLAllocator> &list) NANOEM_DECL_NOEXCEPT
    {
        int index = -1, offset = 0;
        for (typename tinystl::vector<T, TinySTLAllocator>::const_iterator it = list.begin(), end = list.end();
             it != end; ++it) {
            if (*it == item) {
                index = offset;
                break;
            }
            offset++;
        }
        return index;
    }
    template <typename T>
    static inline bool
    removeItem(const T &item, tinystl::vector<T, TinySTLAllocator> &list) NANOEM_DECL_NOEXCEPT
    {
        bool found = false;
        for (typename tinystl::vector<T, TinySTLAllocator>::iterator it = list.begin(), end = list.end(); it != end;
             ++it) {
            found = *it == item;
            if (found) {
                list.erase_unordered(it);
                break;
            }
        }
        return found;
    }
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_LISTUTILS_H_ */
