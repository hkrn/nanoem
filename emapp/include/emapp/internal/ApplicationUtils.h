/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_APPLICATIONUTILS
#define NANOEM_EMAPP_INTERNAL_APPLICATIONUTILS

#include "emapp/BaseApplicationClient.h"
#include "emapp/PluginFactory.h"
#include "emapp/model/Morph.h"

struct _Nanoem__Common__Interpolation;
struct _Nanoem__Application__Plugin;
struct _Nanoem__Application__URI;
typedef _Nanoem__Common__Interpolation Nanoem__Common__Interpolation;
typedef _Nanoem__Application__Plugin Nanoem__Application__Plugin;
typedef _Nanoem__Application__URI Nanoem__Application__URI;

namespace nanoem {

class URI;
typedef tinystl::unordered_map<nanoem_model_morph_category_t, model::Morph::List, TinySTLAllocator> MorphMap;

namespace internal {

class ApplicationUtils NANOEM_DECL_SEALED : private NonCopyable {
public:
    static nanoem_u64_t timestamp() NANOEM_DECL_NOEXCEPT;
    static void fillAllMorphItems(const MorphMap &morphMap, nanoem_model_morph_category_t category, char **&items,
        size_t &numItems, MutableStringList &morphNameList);
    static Nanoem__Application__URI *assignURI(
        Nanoem__Application__URI *uri, MutableString &absolutePath, MutableString &fragment, const URI &fileURI);
    static void fillPluginMessage(const PluginFactory::BaseIOPluginProxy &proxy, Nanoem__Application__Plugin *plugin,
        MutableString &name, MutableString &description, MutableStringList &functions, nanoem_rsize_t offset);
    static void allocateStringList(
        const StringList &input, char **&output, size_t &length, MutableStringList &stringList);
    static void freeStringList(char **&output) NANOEM_DECL_NOEXCEPT;
    static glm::u8vec4 toU8V(const Nanoem__Common__Interpolation *i) NANOEM_DECL_NOEXCEPT;
    static void allocateURIList(const URIList &values, Nanoem__Application__URI **&uris, size_t &numURIs,
        MutableStringList &absolutePathList, MutableStringList &fragmentList);
    static void freeURIList(Nanoem__Application__URI **uris, size_t numURIs) NANOEM_DECL_NOEXCEPT;
    static Nanoem__Common__Interpolation *assignInteprolation(
        Nanoem__Common__Interpolation &i, const glm::u8vec4 &value) NANOEM_DECL_NOEXCEPT;
    static void constructPluginList(const Nanoem__Application__Plugin *const *items, nanoem_rsize_t numItems,
        BaseApplicationClient::PluginItemList &plugins);
};

} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_APPLICATIONUTILS */
