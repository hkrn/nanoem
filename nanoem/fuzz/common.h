#include <stdint.h>

#include "nanoem/nanoem.h"
#include "nanoem/ext/document.h"
#include "nanoem/ext/motion.h"
#include "nanoem/ext/mutable.h"

#if defined(NANOEM_ENABLE_ICU)
#include "nanoem/ext/icu.h"
#define nanoemUnicodeStringFactoryCreateEXT nanoemUnicodeStringFactoryCreateICU
#define nanoemUnicodeStringFactoryDestroyEXT nanoemUnicodeStringFactoryDestroyICU
#define nanoemUnicodeStringFactoryToUtf8OnStackEXT nanoemUnicodeStringFactoryToUtf8OnStackICU
#elif defined(NANOEM_ENABLE_MBWC)
#include "nanoem/ext/mbwc.h"
#define nanoemUnicodeStringFactoryCreateEXT nanoemUnicodeStringFactoryCreateMBWC
#define nanoemUnicodeStringFactoryDestroyEXT nanoemUnicodeStringFactoryDestroyMBWC
#define nanoemUnicodeStringFactoryToUtf8OnStackEXT nanoemUnicodeStringFactoryToUtf8OnStackMBWC
#elif defined(__APPLE__)
#include "nanoem/ext/cfstring.h"
#if defined(NANOEM_ENABLE_CFSTRING)
#define nanoemUnicodeStringFactoryCreateEXT nanoemUnicodeStringFactoryCreateCF
#define nanoemUnicodeStringFactoryDestroyEXT nanoemUnicodeStringFactoryDestroyCF
#define nanoemUnicodeStringFactoryToUtf8OnStackEXT nanoemUnicodeStringFactoryToUtf8OnStackCF
#endif
#else

static nanoem_unicode_string_factory_t *
nanoemUnicodeStringFactoryCreateEXT(void)
{
    nanoem_unicode_string_factory_t *factory = nanoemUnicodeStringFactoryCreate();
    return factory;
}

static void
nanoemUnicodeStringFactoryDestroyEXT(nanoem_unicode_string_factory_t *factory)
{
    nanoemUnicodeStringFactoryDestroy(factory);
}

static char *
nanoemUnicodeStringFactoryToUtf8EXT(nanoem_unicode_string_factory_t *factory, const nanoem_unicode_string_t *string,
    nanoem_rsize_t *length, nanoem_status_t *status)
{
    nanoem_mark_unused(factory);
    nanoem_mark_unused(string);
    *length = 0;
    *status = NANOEM_STATUS_SUCCESS;
    return NULL;
}

static void
nanoemUnicodeStringFactoryToUtf8OnStackEXT(nanoem_unicode_string_factory_t *factory,
    const nanoem_unicode_string_t *string, nanoem_rsize_t *length, char *buffer, size_t capacity,
    nanoem_status_t *status)
{
    char *s = nanoemUnicodeStringFactoryGetByteArray(factory, string, length, status);
    strncpy(buffer, s, capacity);
    nanoemUnicodeStringFactoryDestroyByteArray(factory, s);
}

#endif /* NANOEM_ENABLE_EXT */
