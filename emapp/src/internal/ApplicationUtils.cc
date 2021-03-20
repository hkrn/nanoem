/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/ApplicationUtils.h"

#include "emapp/Model.h"
#include "emapp/StringUtils.h"
#include "emapp/private/CommonInclude.h"
#include "sokol/sokol_time.h"

#include "../src/protoc/application.pb-c.h"

namespace nanoem {
namespace internal {

nanoem_u64_t
ApplicationUtils::timestamp() NANOEM_DECL_NOEXCEPT
{
    return stm_now();
}

void
ApplicationUtils::fillAllMorphItems(const MorphMap &morphMap, nanoem_model_morph_category_t category, char **&items,
    size_t &numItems, MutableStringList &morphNameList)
{
    MorphMap::const_iterator it = morphMap.find(category);
    if (it != morphMap.end()) {
        numItems = it->second.size();
        morphNameList.resize(numItems);
        items = new char *[numItems];
        size_t index = 0;
        for (model::Morph::List::const_iterator it2 = it->second.begin(), end2 = it->second.end(); it2 != end2;
             ++it2, ++index) {
            if (const model::Morph *morph = model::Morph::cast(*it2)) {
                items[index] = StringUtils::cloneString(morph->nameConstString(), morphNameList[index]);
            }
        }
    }
}

Nanoem__Application__URI *
ApplicationUtils::assignURI(
    Nanoem__Application__URI *uri, MutableString &absolutePath, MutableString &fragment, const URI &fileURI)
{
    StringUtils::copyString(fileURI.absolutePath(), absolutePath);
    StringUtils::copyString(fileURI.fragment(), fragment);
    nanoem__application__uri__init(uri);
    uri->absolute_path = absolutePath.data();
    uri->fragment = fragment.data();
    return uri;
}

void
ApplicationUtils::fillPluginMessage(const PluginFactory::BaseIOPluginProxy &proxy, Nanoem__Application__Plugin *plugin,
    MutableString &name, MutableString &description, MutableStringList &functions, nanoem_rsize_t offset)
{
    nanoem__application__plugin__init(plugin);
    plugin->name = StringUtils::cloneString(proxy.name().c_str(), name);
    plugin->description = StringUtils::cloneString(proxy.description().c_str(), description);
    int numFunctions = proxy.countAllFunctions();
    if (numFunctions > 0) {
        plugin->n_functions = numFunctions;
        plugin->functions = new char *[numFunctions];
        for (int i = 0; i < numFunctions; i++) {
            plugin->functions[i] = StringUtils::cloneString(proxy.functionName(i).c_str(), functions[offset + i]);
        }
    }
}

void
ApplicationUtils::allocateStringList(
    const StringList &input, char **&output, size_t &length, MutableStringList &stringList)
{
    length = input.size();
    output = new char *[length];
    stringList.resize(length);
    for (nanoem_rsize_t i = 0; i < length; i++) {
        output[i] = StringUtils::cloneString(input[i].c_str(), stringList[i]);
    }
}

void
ApplicationUtils::freeStringList(char **&output) NANOEM_DECL_NOEXCEPT
{
    delete[] output;
}

Vector4U8
ApplicationUtils::toU8V(const Nanoem__Common__Interpolation *i) NANOEM_DECL_NOEXCEPT
{
    return Vector4U8(i->x0, i->y0, i->x1, i->y1);
}

void
ApplicationUtils::allocateURIList(const URIList &values, Nanoem__Application__URI **&uris, size_t &numURIs,
    MutableStringList &absolutePathList, MutableStringList &fragmentList)
{
    numURIs = values.size();
    uris = new Nanoem__Application__URI *[numURIs];
    absolutePathList.resize(numURIs);
    fragmentList.resize(numURIs);
    for (size_t i = 0; i < numURIs; i++) {
        const URI &fileURI = values[i];
        Nanoem__Application__URI *uri = uris[i] = nanoem_new(Nanoem__Application__URI);
        nanoem__application__uri__init(uri);
        uri->absolute_path = StringUtils::cloneString(fileURI.absolutePathConstString(), absolutePathList[i]);
        uri->fragment = StringUtils::cloneString(fileURI.fragmentConstString(), fragmentList[i]);
    }
}

void
ApplicationUtils::freeURIList(Nanoem__Application__URI **uris, size_t numURIs) NANOEM_DECL_NOEXCEPT
{
    for (size_t i = 0; i < numURIs; i++) {
        Nanoem__Application__URI *uri = uris[i];
        nanoem_delete(uri);
    }
    delete[] uris;
}

Nanoem__Common__Interpolation *
ApplicationUtils::assignInteprolation(Nanoem__Common__Interpolation &i, const Vector4U8 &value) NANOEM_DECL_NOEXCEPT
{
    i = NANOEM__COMMON__INTERPOLATION__INIT;
    i.x0 = value.x;
    i.y0 = value.y;
    i.x1 = value.z;
    i.y1 = value.w;
    return &i;
}

void
ApplicationUtils::constructPluginList(const Nanoem__Application__Plugin *const *items, nanoem_rsize_t numItems,
    BaseApplicationClient::PluginItemList &plugins)
{
    plugins.reserve(numItems);
    for (nanoem_rsize_t i = 0; i < numItems; i++) {
        const Nanoem__Application__Plugin *item = items[i];
        BaseApplicationClient::PluginItem plugin;
        plugin.m_name = item->name;
        plugin.m_description = item->description;
        for (nanoem_rsize_t j = 0, numFunctions = item->n_functions; j < numFunctions; j++) {
            plugin.m_functions.push_back(item->functions[j]);
        }
        plugins.push_back(plugin);
    }
}

} /* namespace internal */
} /* namespace nanoem */
