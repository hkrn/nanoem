/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include <stddef.h>
#if defined(_MSC_VER) && _MSC_VER <= 1800
#define inline __inline
#endif

/* nanodxm */
#include "nanodxm/nanodxm.c"

/* nanomqo */
#include "nanomqo/nanomqo.c"

/* undo */
#define UNDO_STACK_MAX_NUM 1024
#include "undo/undo.c"

/* par */
extern void *__par_malloc(size_t size, const char *file, int line);
extern void *__par_calloc(size_t size, size_t count, const char *file, int line);
extern void *__par_realloc(void *ptr, size_t size, const char *file, int line);
extern void __par_free(void *ptr, const char *file, int line);
#define PAR_SHAPES_IMPLEMENTATION
#ifndef NDEBUG
#define PAR_MALLOC(T, size) ((T *) __par_malloc((size) * sizeof(T), __FILE__, __LINE__))
#define PAR_CALLOC(T, size) ((T *) __par_calloc((size) * sizeof(T), 1, __FILE__, __LINE__))
#define PAR_REALLOC(T, p, size) ((T *) __par_realloc((p), (size) * sizeof(T), __FILE__, __LINE__))
#define PAR_FREE(p) __par_free((p), __FILE__, __LINE__)
#else
#define PAR_MALLOC(T, size) ((T *) __par_malloc((size) * sizeof(T), 0, 0))
#define PAR_CALLOC(T, size) ((T *) __par_calloc((size) * sizeof(T), 1, 0, 0))
#define PAR_REALLOC(T, p, size) ((T *) __par_realloc((p), (size) * sizeof(T), 0, 0))
#define PAR_FREE(p) __par_free((p), 0, 0)
#endif
#if defined(_MSC_VER)
__pragma(warning(push))
__pragma(warning(disable:4244))
__pragma(warning(disable:4305))
#endif /* _MSC_VER */
#include "par/par_shapes.h"
#if defined(_MSC_VER)
__pragma(warning(pop))
#endif /* _MSC_VER */

/* add "__" prefix to prevent confliction of stb.h */
extern void *__stb_malloc(size_t size, const char *file, int line);
extern void *__stb_realloc(void *ptr, size_t size, const char *file, int line);
extern void __stb_free(void *ptr, const char *file, int line);

/* stb */
#define STB_SPRINTF_IMPLEMENTATION
#include "stb/stb_sprintf.h"

/* wildcardcmp */
#include "wildcardcmp.c"

/* sokol_time */
#define SOKOL_IMPL
#include "sokol/sokol_time.h"
