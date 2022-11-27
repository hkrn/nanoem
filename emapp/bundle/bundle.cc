/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/Forward.h"

#include "bx/allocator.h"

/* ImGui */
#include "imgui/imgui.cpp"
#include "imgui/imgui_demo.cpp"
#include "imgui/imgui_draw.cpp"
#include "imgui/imgui_tables.cpp"
#include "imgui/imgui_widgets.cpp"

/* ImGuiFileDialog */
#if defined(NANOEM_ENABLE_IMGUI_FILE_DIALOG)
#define USE_IMGUI_TABLES
#define USE_BOOKMARK
#define createDirButtonString u8"\uf067"
#define okButtonString u8"\uf00c OK"
#define cancelButtonString u8"\uf00d Cancel"
#define resetButtonString u8"\uf064"
#define drivesButtonString u8"\uf0a0"
#define searchString u8"\uf002"
#define dirEntryString u8"\uf07b"
#define linkEntryString u8"\uf1c9"
#define fileEntryString u8"\uf15b"
#define OverWriteDialogConfirmButtonString u8"\uf00c Confirm"
#define OverWriteDialogCancelButtonString u8"\uf00d Cancel "
#define USE_CUSTOM_SORTING_ICON
#define tableHeaderAscendingIcon u8"\uf077"
#define tableHeaderDescendingIcon u8"\uf078"

nanoem_pragma_diagnostics_push()
nanoem_pragma_diagnostics_ignore_clang_gcc("-Wunused-parameter")
#include "imguifiledialog/ImGuiFileDialog.cpp"
nanoem_pragma_diagnostics_pop()
#endif /* NANOEM_ENABLE_IMGUI_FILE_DIALOG */

/* ImGuizmo */
/* workaround of near/far keywords on win32 */
#undef near
#undef far
#include "imguizmo/ImGuizmo.cpp"

namespace nanoem {
extern bx::AllocatorI *g_dd_allocator;
extern bx::AllocatorI *g_par_allocator;
extern bx::AllocatorI *g_sokol_allocator;
extern bx::AllocatorI *g_stb_allocator;
extern bx::AllocatorI *g_tinyobj_allocator;
#include "sha256.c"
}

using namespace nanoem;

inline sg_trace_hooks
sg_install_trace_hooks(const sg_trace_hooks* trace_hooks)
{
    return sg::install_trace_hooks(trace_hooks);
}

inline sg_backend
sg_query_backend(void)
{
    return sg::query_backend();
}

inline sg_desc
sg_query_desc(void)
{
    return sg::query_desc();
}

inline sg_features
sg_query_features(void)
{
    return sg::query_features();
}

inline sg_image_info
sg_query_image_info(sg_image img)
{
    return sg::query_image_info(img);
}

inline sg_pipeline_info
sg_query_pipeline_info(sg_pipeline pip)
{
    return sg::query_pipeline_info(pip);
}

inline sg_pass_info
sg_query_pass_info(sg_pass pass)
{
    return sg::query_pass_info(pass);
}

inline sg_limits
sg_query_limits(void)
{
    return sg::query_limits();
}

inline sg_buffer_info
sg_query_buffer_info(sg_buffer buf)
{
    return sg::query_buffer_info(buf);
}

inline sg_pixelformat_info
sg_query_pixelformat(sg_pixel_format fmt)
{
    return sg::query_pixelformat(fmt);
}

inline sg_resource_state
sg_query_buffer_state(sg_buffer buf)
{
    return sg::query_buffer_state(buf);
}

inline sg_resource_state
sg_query_image_state(sg_image img)
{
    return sg::query_image_state(img);
}

inline sg_resource_state
sg_query_pass_state(sg_pass pass)
{
    return sg::query_pass_state(pass);
}

inline sg_resource_state
sg_query_pipeline_state(sg_pipeline pip)
{
    return sg::query_pipeline_state(pip);
}

inline sg_resource_state
sg_query_shader_state(sg_shader shd)
{
    return sg::query_shader_state(shd);
}

inline sg_shader_info
sg_query_shader_info(sg_shader shd)
{
    return sg::query_shader_info(shd);
}

#define SOKOL_GFX_INCLUDED
#define SOKOL_GFX_IMGUI_IMPL
#include "sokol/util/sokol_gfx_imgui.h"

#if BX_PLATFORM_WINDOWS
#include <Windows.h>
#else
#define SecureZeroMemory(ptr, size) memset((ptr), 0, (size));
#endif

extern "C" void *
__par_malloc(size_t size, const char *file, int line)
{
    return bx::alloc(g_par_allocator, size, 0, file, line);
}
extern "C" void *
__par_calloc(size_t size, size_t count, const char *file, int line)
{
    const size_t allocateSize = size * count;
    void *ptr = __par_malloc(allocateSize, file, line);
    SecureZeroMemory(ptr, allocateSize);
    return ptr;
}
extern "C" void *
__par_realloc(void *ptr, size_t size, const char *file, int line)
{
    return bx::realloc(g_par_allocator, ptr, size, 0, file, line);
}
extern "C" void
__par_free(void *ptr, const char *file, int line)
{
    bx::free(g_par_allocator, ptr, 0, file, line);
}

extern "C" void *
__stb_malloc(size_t size, const char *file, int line)
{
    return bx::alloc(g_stb_allocator, size, 0, file, line);
}
extern "C" void *
__stb_realloc(void *ptr, size_t size, const char *file, int line)
{
    return bx::realloc(g_stb_allocator, ptr, size, 0, file, line);
}
extern "C" void
__stb_free(void *ptr, const char *file, int line)
{
    bx::free(g_stb_allocator, ptr, 0, file, line);
}

extern "C" void *
__dd_malloc(size_t size, const char *file, int line)
{
    return bx::alloc(g_dd_allocator, size, 0, file, line);
}
extern "C" void
__dd_free(void *ptr, const char *file, int line)
{
    bx::free(g_dd_allocator, ptr, 0, file, line);
}

extern "C" void *
__tinyobj_malloc(size_t size, const char *file, int line)
{
    return bx::alloc(g_tinyobj_allocator, size, 0, file, line);
}
extern "C" void *
__tinyobj_calloc(size_t size, size_t count, const char *file, int line)
{
    const size_t allocateSize = size * count;
    void *ptr = __tinyobj_malloc(allocateSize, file, line);
    SecureZeroMemory(ptr, allocateSize);
    return ptr;
}
extern "C" void *
__tinyobj_realloc(void *ptr, size_t size, const char *file, int line)
{
    return bx::realloc(g_tinyobj_allocator, ptr, size, 0, file, line);
}
extern "C" void
__tinyobj_free(void *ptr, const char *file, int line)
{
    bx::free(g_tinyobj_allocator, ptr, 0, file, line);
}

/* Debug Draw */
#ifdef NDEBUG
#define DEBUG_DRAW_OVERFLOWED(message) (void) 0
#define DD_MALLOC(size) __dd_malloc((size), nullptr, 0)
#define DD_MFREE(ptr) __dd_free((ptr), nullptr, 0)
#else
#define DD_MALLOC(size) __dd_malloc((size), __FILE__, __LINE__)
#define DD_MFREE(ptr) __dd_free((ptr), __FILE__, __LINE__)
#endif /* NDEBUG */
#define DEBUG_DRAW_IMPLEMENTATION
#include "debug-draw/debug_draw.hpp"
