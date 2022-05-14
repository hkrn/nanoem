/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/Allocator.h"

#include "imgui/imgui.h"

#include "bx/allocator.h"
#include "bx/debug.h"
#include "bx/string.h"
#include "tinystl/allocator.h"

#if defined(NANOEM_ENABLE_MIMALLOC)
#include "mimalloc.h"
#endif

#define BX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT 8
#if BX_PLATFORM_WINDOWS && BX_COMPILER_MSVC
#if defined(NANOEM_ALLOCATOR_BACKTRACE)
#pragma comment(lib, "dbghelp.lib")
#include <DbgHelp.h>
#endif
#include <crtdbg.h>
#include <stdio.h>
#elif !BX_PLATFORM_WINDOWS
#define SecureZeroMemory(ptr, size) memset((ptr), 0, (size));
#endif
#if defined(NANOEM_ALLOCATOR_BACKTRACE) && (BX_PLATFORM_BSD || BX_PLATFORM_OSX || BX_PLATFORM_LINUX)
#include <execinfo.h>
#endif

#if defined(NANOEM_ENABLE_DEBUG_ALLOCATOR)
#include "bx/mutex.h"
#include <algorithm>
#include <atomic>
#endif

namespace nanoem {

bx::AllocatorI *g_dd_allocator = nullptr;
bx::AllocatorI *g_emapp_allocator = nullptr;
bx::AllocatorI *g_par_allocator = nullptr;
bx::AllocatorI *g_sokol_allocator = nullptr;
bx::AllocatorI *g_stb_allocator = nullptr;
bx::AllocatorI *g_tinyobj_allocator = nullptr;
bx::AllocatorI *g_tinystl_allocator = nullptr;
ProtobufCAllocator *g_protobufc_allocator = nullptr;

void *
Allocator::AllocateLocation::static_allocate(size_t bytes)
{
    return operator new(bytes);
}

void
Allocator::AllocateLocation::static_deallocate(void *ptr, size_t /*bytes*/) NANOEM_DECL_NOEXCEPT
{
    operator delete(ptr);
}

void
Allocator::AllocateLocation::getStackTrace(String &value, const Allocator *allocator)
{
#if defined(NANOEM_ALLOCATOR_BACKTRACE) && BX_PLATFORM_WINDOWS
    BX_UNUSED_1(allocator);
    VOID *stack[64];
    PSYMBOL_INFO symbol;
    HANDLE process = GetCurrentProcess();
    SymInitialize(process, nullptr, TRUE);
    WORD frames = CaptureStackBackTrace(0, ARRAYSIZE(stack), stack, nullptr);
    symbol = static_cast<SYMBOL_INFO *>(operator new(sizeof(*symbol) + 256));
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(*symbol);
    char buffer[256];
    for (WORD i = 2; i < frames; i++) {
        SymFromAddr(process, reinterpret_cast<DWORD64>(stack[i]), 0, symbol);
        bx::snprintf(buffer, sizeof(buffer), "%d: %s: - 0x%0X\n", frames - i - 1, symbol->Name, symbol->Address);
        value.append(buffer);
    }
    operator delete(symbol);
#elif defined(NANOEM_ALLOCATOR_BACKTRACE) && (BX_PLATFORM_BSD || BX_PLATFORM_OSX || BX_PLATFORM_LINUX)
    BX_UNUSED_1(allocator);
    void *stack[64];
    int frames = ::backtrace(stack, BX_COUNTOF(stack));
    char **strs = ::backtrace_symbols(stack, frames);
    char buffer[256];
    for (int i = 0; i < frames; i++) {
        bx::snprintf(buffer, sizeof(buffer), "%s\n", strs[i]);
        value.append(buffer);
    }
    ::free(strs);
#else
    BX_UNUSED_2(value, allocator);
#endif
}

Allocator::AllocateLocation::AllocateLocation(const String &l, size_t s, const String &t)
    : location(l)
    , size(s)
    , stacktrace(t)
{
}

void
Allocator::initialize()
{
    g_dd_allocator = &g_instance_for_debugdraw;
    g_emapp_allocator = &g_instance_for_emapp;
    g_par_allocator = &g_instance_for_par;
    g_sokol_allocator = &g_instance_for_sokol;
    g_stb_allocator = &g_instance_for_stb;
    g_protobufc_allocator = &g_pb_allocator;
    g_tinyobj_allocator = &g_instance_for_tinyobj;
    g_tinystl_allocator = &g_instance_for_tinystl;
    json_set_allocation_functions(allocateJson, releaseJson);
    nanodxmGlobalSetCustomAllocator(&g_nanodxm_allocator);
    nanoemGlobalSetCustomAllocator(&g_nanoem_allocator);
    nanomqoGlobalSetCustomAllocator(&g_nanomqo_allocator);
    undoGlobalSetCustomAllocator(&g_undo_allocator);
    ImGui::SetAllocatorFunctions(allocateImGui, releaseImGui);
}

void
Allocator::destroy()
{
}

void *
Allocator::allocateImGui(size_t size, void * /* userData */)
{
    return BX_ALLOC(&g_instance_for_imgui, size);
}

void
Allocator::releaseImGui(void *ptr, void * /* userData */) NANOEM_DECL_NOEXCEPT
{
    return BX_FREE(&g_instance_for_imgui, ptr);
}

void *
Allocator::allocateJson(size_t size)
{
    return BX_ALLOC(&g_instance_for_parson, size);
}

void
Allocator::releaseJson(void *ptr) NANOEM_DECL_NOEXCEPT
{
    return BX_FREE(&g_instance_for_parson, ptr);
}

void *
Allocator::allocate(void *opaque, size_t size, const char *filename, int line)
{
    return bx::alloc(static_cast<bx::AllocatorI *>(opaque), size, 0, filename, nanoem_u32_t(line));
}

void *
Allocator::safeAllocate(void *opaque, size_t length, size_t size, const char *filename, int line)
{
    const size_t totalBytes = length * size;
    void *ptr = bx::alloc(static_cast<bx::AllocatorI *>(opaque), totalBytes, 0, filename, nanoem_u32_t(line));
    SecureZeroMemory(ptr, totalBytes);
    return ptr;
}

void *
Allocator::reallocate(void *opaque, void *ptr, size_t size, const char *filename, int line)
{
    return bx::realloc(static_cast<bx::AllocatorI *>(opaque), ptr, size, 0, filename, nanoem_u32_t(line));
}

void
Allocator::release(void *opaque, void *ptr, const char *filename, int line) NANOEM_DECL_NOEXCEPT
{
    bx::free(static_cast<bx::AllocatorI *>(opaque), ptr, 0, filename, nanoem_u32_t(line));
}

void *
Allocator::allocate(void *opaque, size_t size)
{
    return BX_ALLOC(static_cast<bx::AllocatorI *>(opaque), size);
}

void
Allocator::release(void *opaque, void *ptr) NANOEM_DECL_NOEXCEPT
{
    BX_FREE(static_cast<bx::AllocatorI *>(opaque), ptr);
}

Allocator::Allocator(const char *name)
#ifdef NANOEM_ENABLE_DEBUG_ALLOCATOR
    : m_numAllocateCount(0)
    , m_totalAllocateCount(0)
    , m_totalReleaseCount(0)
    , m_numAllocatedBytes(0)
    , m_totalAllocatedBytes(0)
    , m_countAloocationSmall(0)
    , m_countAloocationMedium(0)
    , m_countAloocationLarge(0)
    , m_countAloocationHuge(0)
    , m_peakAllocateCount(0)
    , m_peakAllocatedBytes(0)
{
    bx::strCopy(m_nameBuffer, sizeof(m_nameBuffer), name);
#else
{
    BX_UNUSED_1(name);
#endif
}

Allocator::~Allocator() NANOEM_DECL_NOEXCEPT
{
#ifdef NANOEM_ENABLE_DEBUG_ALLOCATOR
    char peakAllocatedBytesBuffer[16], totalAllocatedBytesBuffer[16], message[1024];
    bx::prettify(peakAllocatedBytesBuffer, sizeof(peakAllocatedBytesBuffer), nanoem_u64_t(m_peakAllocatedBytes));
    bx::prettify(totalAllocatedBytesBuffer, sizeof(totalAllocatedBytesBuffer), nanoem_u64_t(m_totalAllocatedBytes));
    if (m_numAllocateCount > 0) {
        bx::snprintf(message, sizeof(message), "LEAKED(%s): %d:%d:%d (%s:%s) => %d", m_nameBuffer, m_peakAllocateCount,
            m_totalAllocateCount.load(), m_totalReleaseCount.load(), peakAllocatedBytesBuffer,
            totalAllocatedBytesBuffer, m_numAllocateCount.load());
        bx::debugOutput(message);
    }
    else if (m_totalAllocateCount > 0) {
        bx::snprintf(message, sizeof(message), "NORMAL(%s): %d:%d (%s:%s)", m_nameBuffer, m_peakAllocateCount,
            m_totalAllocateCount.load(), peakAllocatedBytesBuffer, totalAllocatedBytesBuffer);
        bx::debugOutput(message);
    }
    typedef tinystl::unordered_map<AllocateLocation::String, tinystl::pair<AllocateLocation, int>, AllocateLocation>
        FilterMap;
    FilterMap filter;
    for (AllocateLocationMap::const_iterator it = m_allocatedAt.begin(), end = m_allocatedAt.end(); it != end; ++it) {
        const AllocateLocation::String &location = it->second.location;
        FilterMap::iterator it2 = filter.find(location);
        if (it2 != filter.end()) {
            it2->second.second++;
        }
        else {
            filter.insert(tinystl::make_pair(location, tinystl::make_pair(it->second, 1)));
        }
    }
    for (FilterMap::const_iterator it = filter.begin(), end = filter.end(); it != end; ++it) {
        if (it->second.second > 0) {
            bx::snprintf(message, sizeof(message), "%s => %d\n", it->first.c_str(), it->second.second);
            const AllocateLocation::String &stacktrace = it->second.first.stacktrace;
            if (!stacktrace.empty()) {
                bx::snprintf(message, sizeof(message), "%s", stacktrace.c_str());
            }
            bx::debugOutput(message);
        }
    }
    size_t total = m_countAloocationSmall + m_countAloocationMedium + m_countAloocationLarge + m_countAloocationHuge;
    if (total > 0) {
        nanoem_f64_t totalF = static_cast<nanoem_f64_t>(total);
        if (m_countAloocationSmall > 0) {
            bx::snprintf(message, sizeof(message), "Small: %.2f%%", (m_countAloocationSmall / totalF) * 100);
            bx::debugOutput(message);
        }
        if (m_countAloocationMedium > 0) {
            bx::snprintf(message, sizeof(message), "Medium: %.2f%%", (m_countAloocationMedium / totalF) * 100);
            bx::debugOutput(message);
        }
        if (m_countAloocationLarge > 0) {
            bx::snprintf(message, sizeof(message), "Large: %.2f%%", (m_countAloocationLarge / totalF) * 100);
            bx::debugOutput(message);
        }
        if (m_countAloocationHuge > 0) {
            bx::snprintf(message, sizeof(message), "Huge: %.2f%%", (m_countAloocationHuge / totalF) * 100);
            bx::debugOutput(message);
        }
    }
#endif
}

void *
Allocator::realloc(void *ptr, size_t size, size_t align, const char *file, nanoem_u32_t line)
{
    if (0 == size) {
        if (nullptr != ptr) {
#ifdef NANOEM_ENABLE_DEBUG_ALLOCATOR
            m_numAllocateCount--;
            m_totalReleaseCount++;
            incrementAllocationCount(size);
            {
                bx::MutexScope scope(m_allocatedAtLock);
                BX_UNUSED_1(scope);
                AllocateLocationMap::iterator it = m_allocatedAt.find(ptr);
                nanoem_assert(it != m_allocatedAt.end(), "must be found");
                nanoem_assert(m_numAllocatedBytes >= it->second.size, "must be greater than allocated size");
                m_numAllocatedBytes -= it->second.size;
                m_allocatedAt.erase(it);
            }
#endif /* NANOEM_ENABLE_DEBUG_ALLOCATOR */
#if BX_COMPILER_MSVC && defined(_DEBUG)
            if (BX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT >= align) {
                _free_dbg(ptr, _NORMAL_BLOCK);
                return nullptr;
            }
            _aligned_free_dbg(ptr);
#elif defined(NANOEM_ENABLE_MIMALLOC)
            BX_UNUSED(file, line);
            mi_free(ptr);
#else
            if (BX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT >= align) {
                ::free(ptr);
                return nullptr;
            }
#if BX_COMPILER_MSVC
            BX_UNUSED(file, line);
            _aligned_free(ptr);
#else
            bx::alignedFree(this, ptr, align, file, line);
#endif /* BX_COMPILER_MSVC */
#endif /* BX_COMPILER_MSVC */
        }
        return nullptr;
    }
    else if (nullptr == ptr) {
#ifdef NANOEM_ENABLE_DEBUG_ALLOCATOR
        m_numAllocateCount++;
        m_totalAllocateCount++;
        m_totalAllocatedBytes += size;
        m_numAllocatedBytes += size;
        incrementAllocationCount(size);
        {
            bx::MutexScope scope(m_peakAllocationLock);
            BX_UNUSED_1(scope);
            m_peakAllocateCount = std::max(m_numAllocateCount.load(), m_peakAllocateCount);
            m_peakAllocatedBytes =
                m_numAllocatedBytes > m_peakAllocatedBytes ? m_numAllocatedBytes.load() : m_peakAllocatedBytes;
        }
#endif /* NANOEM_ENABLE_DEBUG_ALLOCATOR */
#if BX_COMPILER_MSVC && defined(_DEBUG)
        if (BX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT >= align) {
            ptr = _malloc_dbg(size, _NORMAL_BLOCK, file, line);
        }
        else {
            ptr = _aligned_malloc_dbg(size, align, file, line);
        }
#elif defined(NANOEM_ENABLE_MIMALLOC)
        BX_UNUSED_1(align);
        ptr = mi_malloc(size);
#else
        if (BX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT >= align) {
            ptr = ::malloc(size);
        }
        else {
#if BX_COMPILER_MSVC
            BX_UNUSED(file, line);
            ptr = _aligned_malloc(size, align);
#else
            ptr = bx::alignedAlloc(this, size, align, file, line);
#endif /* BX_COMPILER_MSVC */
        }
#endif /* BX_COMPILER_MSVC */
#ifdef NANOEM_ENABLE_DEBUG_ALLOCATOR
        AllocateLocation::String location, stacktrace;
        AllocateLocation::getStackTrace(stacktrace, this);
        bx::stringPrintf(location, "%s(%d)", file, line);
        {
            bx::MutexScope scope(m_allocatedAtLock);
            BX_UNUSED_1(scope);
            m_allocatedAt.insert(tinystl::make_pair(ptr, AllocateLocation(location, size, stacktrace)));
        }
#endif
        return ptr;
    }
    else {
#ifdef NANOEM_ENABLE_DEBUG_ALLOCATOR
        m_totalAllocatedBytes += size;
        {
            bx::MutexScope scope(m_allocatedAtLock);
            BX_UNUSED_1(scope);
            AllocateLocationMap::iterator it = m_allocatedAt.find(ptr);
            nanoem_assert(it != m_allocatedAt.end(), "must be found");
            nanoem_assert(m_numAllocatedBytes >= it->second.size, "must be greater than allocated size");
            m_numAllocatedBytes -= it->second.size;
            m_allocatedAt.erase(it);
        }
#endif /* NANOEM_ENABLE_DEBUG_ALLOCATOR */
        void *newPtr = 0;
#if BX_COMPILER_MSVC && defined(_DEBUG)
        if (BX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT >= align) {
            newPtr = _realloc_dbg(ptr, size, _NORMAL_BLOCK, file, line);
        }
        else {
            newPtr = _aligned_realloc_dbg(ptr, size, align, file, line);
        }
#elif defined(NANOEM_ENABLE_MIMALLOC)
        newPtr = mi_realloc(ptr, size);
#else
        if (BX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT >= align) {
            newPtr = ::realloc(ptr, size);
        }
        else {
#if BX_COMPILER_MSVC
            BX_UNUSED(file, line);
            newPtr = _aligned_realloc(ptr, size, align);
#else
            newPtr = bx::alignedRealloc(this, ptr, size, align, file, line);
#endif /* BX_COMPILER_MSVC */
        }
#endif /* BX_COMPILER_MSVC */
#ifdef NANOEM_ENABLE_DEBUG_ALLOCATOR
        AllocateLocation::String location, stacktrace;
        AllocateLocation::getStackTrace(stacktrace, this);
        bx::stringPrintf(location, "%s(%d)", file, line);
        m_numAllocatedBytes += size;
        {
            bx::MutexScope scope(m_peakAllocationLock);
            BX_UNUSED_1(scope);
            m_peakAllocatedBytes =
                m_numAllocatedBytes > m_peakAllocatedBytes ? m_numAllocatedBytes.load() : m_peakAllocatedBytes;
        }
        {
            bx::MutexScope scope(m_allocatedAtLock);
            BX_UNUSED_1(scope);
            m_allocatedAt.insert(tinystl::make_pair(newPtr, AllocateLocation(location, size, stacktrace)));
        }
#endif /* NANOEM_ENABLE_DEBUG_ALLOCATOR */
        return newPtr;
    }
}

Allocator Allocator::g_instance_for_debugdraw("debugdraw");
Allocator Allocator::g_instance_for_emapp("emapp");
Allocator Allocator::g_instance_for_imgui("ImGui");
Allocator Allocator::g_instance_for_nanodxm("nanodxm");
Allocator Allocator::g_instance_for_nanoem("nanoem");
Allocator Allocator::g_instance_for_nanomqo("nanomqo");
Allocator Allocator::g_instance_for_par("par");
Allocator Allocator::g_instance_for_parson("parson");
Allocator Allocator::g_instance_for_protobuf("protobuf-c");
Allocator Allocator::g_instance_for_sokol("sokol");
Allocator Allocator::g_instance_for_stb("stb");
Allocator Allocator::g_instance_for_tinyobj("tinyobjc");
Allocator Allocator::g_instance_for_tinystl("tinystl");
Allocator Allocator::g_instance_for_undo("undo");

ProtobufCAllocator Allocator::g_pb_allocator = {
    allocate,
    release,
    &g_instance_for_protobuf,
};

nanodxm_global_allocator_t Allocator::g_nanodxm_allocator = { &g_instance_for_nanodxm, allocate, safeAllocate,
    reallocate, release };

nanoem_global_allocator_t Allocator::g_nanoem_allocator = { &g_instance_for_nanoem, allocate, safeAllocate, reallocate,
    release };

nanomqo_global_allocator_t Allocator::g_nanomqo_allocator = { &g_instance_for_nanomqo, allocate, safeAllocate,
    reallocate, release };

undo_global_allocator_t Allocator::g_undo_allocator = { &g_instance_for_undo, allocate, safeAllocate, reallocate,
    release };

} /* namespace nanoem */
