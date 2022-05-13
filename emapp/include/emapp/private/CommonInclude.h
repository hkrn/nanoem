/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#ifndef NANOEM_EMAPP_PRIVATE_COMMONINCLUDE
#define NANOEM_EMAPP_PRIVATE_COMMONINCLUDE

#include "emapp/Forward.h"

/* bx */
#include "bx/os.h"

/* GLM */
#include "glm/gtc/type_ptr.hpp"
#ifndef NDEBUG
#include "glm/gtx/io.hpp"
#include "glm/gtx/string_cast.hpp"
#include <iostream>
#include <sstream>
#endif /* NDEBUG */

#if BX_PLATFORM_OSX
#include <mach/mach.h> /* for Inline::isDebuggerPresent */
#endif /* BX_PLATFORM_OSX */

#define nanoem_new(expr) BX_NEW(nanoem::g_emapp_allocator, expr)
#define nanoem_delete(expr) BX_DELETE(nanoem::g_emapp_allocator, expr)
#define nanoem_delete_safe(expr)                                                                                       \
    do {                                                                                                               \
        BX_DELETE(nanoem::g_emapp_allocator, expr);                                                                    \
        (expr) = nullptr;                                                                                              \
    } while (0)

typedef struct ProtobufCAllocator ProtobufCAllocator;

#if defined(NANOEM_ENABLE_DEBUG_LABEL)
#if defined(NANOEM_ENABLE_OPTICK)
#include "optick/src/optick.h"
#else
#define OPTICK_EVENT(...)
#define OPTICK_FRAME(name)
#endif /* NANOEM_ENABLE_OPTICK */
#define SG_PUSH_GROUP(label)                                                                                           \
    do {                                                                                                               \
        OPTICK_EVENT();                                                                                                \
        nanoem::sg::push_group(label);                                                                                 \
    } while (0)
#define SG_PUSH_GROUPF(format, ...)                                                                                    \
    do {                                                                                                               \
        OPTICK_EVENT();                                                                                                \
        nanoem::sg::push_group_format(format, __VA_ARGS__);                                                            \
    } while (0)
#define SG_POP_GROUP()                                                                                                 \
    do {                                                                                                               \
        nanoem::sg::pop_group();                                                                                       \
    } while (0)
#define SG_INSERT_MARKER(text)                                                                                         \
    do {                                                                                                               \
        nanoem::sg::insert_marker(text);                                                                               \
    } while (0)
#define SG_INSERT_MARKERF(format, ...)                                                                                 \
    do {                                                                                                               \
        nanoem::sg::insert_marker_format(format, __VA_ARGS__);                                                         \
    } while (0)
#define SG_LABEL_BUFFER(item, text) nanoem::sg::label_buffer((item), (text))
#define SG_LABEL_IMAGE(item, text) nanoem::sg::label_image((item), (text))
#define SG_LABEL_SHADER(item, text) nanoem::sg::label_shader((item), (text))
#define SG_LABEL_PASS(item, text) nanoem::sg::label_pass((item), (text))
#define SG_LABEL_PIPELINE(item, text) nanoem::sg::label_pipeline((item), (text))
#else /* NANOEM_ENABLE_DEBUG_LABEL */
#define SG_PUSH_GROUP(label) BX_UNUSED_1(label)
#define SG_PUSH_GROUPF(format, ...) BX_UNUSED_1(format)
#define SG_INSERT_MARKER(text) BX_UNUSED_1(text)
#define SG_INSERT_MARKERF(format, ...) BX_UNUSED(format, __VA_ARGS__)
#define SG_LABEL_BUFFER(item, label) BX_UNUSED_2(item, label)
#define SG_LABEL_IMAGE(item, label) BX_UNUSED_2(item, label)
#define SG_LABEL_SHADER(item, label) BX_UNUSED_2(item, label)
#define SG_LABEL_PASS(item, label) BX_UNUSED_2(item, label)
#define SG_LABEL_PIPELINE(item, label) BX_UNUSED_2(item, label)
#define SG_POP_GROUP() (void) 0
#define OPTICK_EVENT(...) (void) 0
#define OPTICK_FRAME(name) (void) 0
#endif /* NANOEM_ENABLE_DEBUG_LABEL */

NANOEM_DECL_API nanoem_unicode_string_factory_t *APIENTRY nanoemUnicodeStringFactoryCreateEXT(nanoem_status_t *status);
NANOEM_DECL_API void APIENTRY nanoemUnicodeStringFactoryDestroyEXT(nanoem_unicode_string_factory_t *factory);
NANOEM_DECL_API void APIENTRY nanoemUnicodeStringFactoryToUtf8OnStackEXT(nanoem_unicode_string_factory_t *factory,
    const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_u8_t *buffer, nanoem_rsize_t capacity,
    nanoem_status_t *status);
NANOEM_DECL_API nanoem_rsize_t APIENTRY nanoemUnicodeStringGetLength(const nanoem_unicode_string_t *string);

namespace nanoem {

class Model;
extern bx::AllocatorI *g_bgfx_allocator;
extern bx::AllocatorI *g_emapp_allocator;
extern bx::AllocatorI *g_tinystl_allocator;
extern ProtobufCAllocator *g_protobufc_allocator;

} /* namespace nanoem */

namespace {

using namespace nanoem;

class Inline : private NonCopyable {
public:
    static const int kNameStackBufferSize = 128;
    static const int kLongNameStackBufferSize = 1024;
    static const int kReadingFileContentsBufferSize = 8192;
    static const int kMarkerStringLength = 256;

    static inline bool
    isDebugLabelEnabled() NANOEM_DECL_NOEXCEPT
    {
#if defined(NANOEM_ENABLE_DEBUG_LABEL)
        return true;
#else
        return false;
#endif
    }

    static inline void
    clearZeroMemory(void *data, size_t size) NANOEM_DECL_NOEXCEPT
    {
#if BX_PLATFORM_WINDOWS
        ZeroMemory(data, size);
#else
        memset(data, 0, size);
#endif
    }
    template <typename T>
    static inline void
    clearZeroMemory(T &data) NANOEM_DECL_NOEXCEPT
    {
        clearZeroMemory(&data, sizeof(data));
    }
    static inline bool
    intersectsRectPoint(const Vector4SI32 &rect, const Vector2SI32 &point) NANOEM_DECL_NOEXCEPT
    {
        return point.x >= rect.x && point.y >= rect.y && point.x <= rect.x + rect.z && point.y <= rect.y + rect.w;
    }
    static inline int
    saturateInt32(size_t value) NANOEM_DECL_NOEXCEPT
    {
        return static_cast<int>(nanoem_likely(value < INT32_MAX) ? value : INT32_MAX);
    }
    static inline nanoem_u32_t
    saturateInt32U(size_t value) NANOEM_DECL_NOEXCEPT
    {
        return static_cast<nanoem_u32_t>(nanoem_likely(value < UINT32_MAX) ? value : UINT32_MAX);
    }
    static inline nanoem_u32_t
    roundInt32(int value) NANOEM_DECL_NOEXCEPT
    {
        return static_cast<nanoem_u32_t>(nanoem_likely(value >= 0) ? value : 0);
    }
    static inline nanoem_f32_t
    fract(nanoem_f32_t value) NANOEM_DECL_NOEXCEPT
    {
        return (value < -1 || value > 1) ? glm::fract(value) : value;
    }
    static inline int
    readI24(const nanoem_u8_t *ptr) NANOEM_DECL_NOEXCEPT
    {
        int v = 0;
        if ((ptr[2] & 0x80) != 0) {
            v = (0xff << 24) | (ptr[2] << 16) | (ptr[1] << 8) | ptr[0];
        }
        else {
            v = (ptr[2] << 16) | (ptr[1] << 8) | ptr[0];
        }
        return v;
    }
    template <typename T>
    static inline void
    resolveSymbol(void *handle, const char *name, T &func)
    {
        func = reinterpret_cast<T>(bx::dlsym(handle, name));
    }
    template <typename T>
    static inline void
    resolveSymbol(void *handle, const char *name, T &func, bool &valid)
    {
        resolveSymbol(handle, name, func);
#ifndef NDEBUG
        if (nanoem_is_null(func)) {
            fprintf(stderr, "Symbol \"%s\" of %p is not found\n", name, handle);
        }
#endif
        valid &= nanoem_is_not_null(func);
    }
    static inline bool
    isDebuggerPresent()
    {
#ifndef NDEBUG
#if BX_PLATFORM_WINDOWS
        return ::IsDebuggerPresent() != 0;
#elif BX_PLATFORM_OSX
        mach_msg_type_number_t count = 0;
        exception_mask_t masks[EXC_TYPES_COUNT];
        mach_port_t ports[EXC_TYPES_COUNT];
        exception_behavior_t behaviors[EXC_TYPES_COUNT];
        thread_state_flavor_t flavors[EXC_TYPES_COUNT];
        exception_mask_t mask = EXC_MASK_ALL & ~(EXC_MASK_RESOURCE | EXC_MASK_GUARD);
        kern_return_t result =
            task_get_exception_ports(mach_task_self(), mask, masks, &count, ports, behaviors, flavors);
        bool presented = false;
        if (result == KERN_SUCCESS) {
            for (mach_msg_type_number_t i = 0; i < count; i++) {
                if (MACH_PORT_VALID(ports[i])) {
                    presented = true;
                    break;
                }
            }
        }
        return presented;
#else /* BX_PLATFORM_* */
        return false;
#endif /* BX_PLATFORM_* */
#else /* NDEBUG */
        return false;
#endif /* NDEBUG */
    }
};

} /* namespace anonymous */

#endif /* NANOEM_EMAPP_PRIVATE_COMMONINCLUDE */
