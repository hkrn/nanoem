/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_ALLOCATOR_H_
#define NANOEM_EMAPP_ALLOCATOR_H_

#include "emapp/Forward.h"

#include "nanodxm/nanodxm.h"
#include "nanoem/ext/parson/parson.h"
#include "nanomqo/nanomqo.h"
#include "protobuf-c/protobuf-c.h"
#include "undo/undo.h"

#include "bx/allocator.h"
#if defined(NANOEM_ENABLE_DEBUG_ALLOCATOR)
#include "bx/mutex.h"
#include <atomic>
#endif

namespace nanoem {

class Allocator NANOEM_DECL_SEALED : public bx::AllocatorI, private NonCopyable {
public:
    static void initialize();
    static void destroy();

    Allocator(const char *name);
    ~Allocator() NANOEM_DECL_NOEXCEPT;

private:
    struct AllocateLocation {
        typedef tinystl::stringT<AllocateLocation> String;
        static void *static_allocate(size_t bytes);
        static void static_deallocate(void *ptr, size_t /*bytes*/) NANOEM_DECL_NOEXCEPT;
        static void getStackTrace(String &value, const Allocator *allocator);
        AllocateLocation(const String &l, size_t s, const String &t);
        const String location;
        const size_t size;
        const String stacktrace;
    };
    typedef tinystl::unordered_map<void *, AllocateLocation, AllocateLocation> AllocateLocationMap;

    static Allocator g_instance_for_debugdraw;
    static Allocator g_instance_for_emapp;
    static Allocator g_instance_for_imgui;
    static Allocator g_instance_for_nanodxm;
    static Allocator g_instance_for_nanoem;
    static Allocator g_instance_for_nanomqo;
    static Allocator g_instance_for_par;
    static Allocator g_instance_for_parson;
    static Allocator g_instance_for_protobuf;
    static Allocator g_instance_for_sokol;
    static Allocator g_instance_for_stb;
    static Allocator g_instance_for_tinyobj;
    static Allocator g_instance_for_tinystl;
    static Allocator g_instance_for_undo;
    static ProtobufCAllocator g_pb_allocator;
    static nanodxm_global_allocator_t g_nanodxm_allocator;
    static nanoem_global_allocator_t g_nanoem_allocator;
    static nanomqo_global_allocator_t g_nanomqo_allocator;
    static undo_global_allocator_t g_undo_allocator;

    static void *allocateImGui(size_t size, void *userData);
    static void releaseImGui(void *ptr, void *userData) NANOEM_DECL_NOEXCEPT;
    static void *allocateJson(size_t size);
    static void releaseJson(void *ptr) NANOEM_DECL_NOEXCEPT;
    static void *allocate(void *opaque, size_t size, const char *filename, int line);
    static void *safeAllocate(void *opaque, size_t length, size_t size, const char *filename, int line);
    static void *reallocate(void *opaque, void *ptr, size_t size, const char *filename, int line);
    static void release(void *opaque, void *ptr, const char *filename, int line) NANOEM_DECL_NOEXCEPT;
    static void *allocate(void *opaque, size_t size);
    static void release(void *opaque, void *ptr) NANOEM_DECL_NOEXCEPT;

    void *realloc(void *ptr, size_t size, size_t align, const char *file, nanoem_u32_t line) NANOEM_DECL_OVERRIDE;

#if defined(NANOEM_ENABLE_DEBUG_ALLOCATOR)
    void
    incrementAllocationCount(size_t size)
    {
        if (size < 16384) {
            m_countAloocationSmall++;
        }
        if (size < 131072) {
            m_countAloocationMedium++;
        }
        if (size < 2097152) {
            m_countAloocationLarge++;
        }
        else {
            m_countAloocationHuge++;
        }
    }

    char m_nameBuffer[16];
    AllocateLocationMap m_allocatedAt;
    bx::Mutex m_allocatedAtLock;
    bx::Mutex m_peakAllocationLock;
    std::atomic<nanoem_u64_t> m_numAllocateCount;
    std::atomic<nanoem_u64_t> m_totalAllocateCount;
    std::atomic<nanoem_u64_t> m_totalReleaseCount;
    std::atomic<nanoem_u64_t> m_numAllocatedBytes;
    std::atomic<nanoem_u64_t> m_totalAllocatedBytes;
    std::atomic<nanoem_u64_t> m_countAloocationSmall;
    std::atomic<nanoem_u64_t> m_countAloocationMedium;
    std::atomic<nanoem_u64_t> m_countAloocationLarge;
    std::atomic<nanoem_u64_t> m_countAloocationHuge;
    nanoem_u64_t m_peakAllocateCount;
    nanoem_u64_t m_peakAllocatedBytes;
#endif /* NANOEM_ENABLE_DEBUG_ALLOCATOR */
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_ALLOCATOR_H_ */
