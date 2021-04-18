/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/ResourceBundle.h"

#include "emapp/ImageLoader.h"
#include "emapp/Project.h"
#include "emapp/private/CommonInclude.h"

#include "bimg/decode.h"
#include "glm/gtc/type_ptr.hpp"

namespace {

using namespace nanoem;

#include "emapp/private/resources/toons/toon01.h"
#include "emapp/private/resources/toons/toon02.h"
#include "emapp/private/resources/toons/toon03.h"
#include "emapp/private/resources/toons/toon04.h"
#include "emapp/private/resources/toons/toon05.h"
#include "emapp/private/resources/toons/toon06.h"
#include "emapp/private/resources/toons/toon07.h"
#include "emapp/private/resources/toons/toon08.h"
#include "emapp/private/resources/toons/toon09.h"
#include "emapp/private/resources/toons/toon10.h"

void
loadSharedTexture(const nanoem_u8_t *data, size_t size, int index, Image *&imageRef, ByteArray &bytes)
{
    nanoem_parameter_assert(data, "must not be NULL");
    nanoem_parameter_assert(size > 0, "must not be NULL");
    bx::Error err;
    if (bimg::ImageContainer *container =
            bimg::imageParse(g_bimg_allocator, data, nanoem_u32_t(size), bimg::TextureFormat::RGBA8, &err)) {
        String filename;
        bx::stringPrintf(filename, "@nanoem/SharedToonTexture/%d", index);
        sg_image_desc desc;
        Inline::clearZeroMemory(desc);
        desc.width = container->m_width;
        desc.height = container->m_height;
        desc.mag_filter = desc.min_filter = SG_FILTER_NEAREST;
        desc.wrap_u = desc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
        const nanoem_u8_t *dataPtr = static_cast<const nanoem_u8_t *>(container->m_data);
        nanoem_rsize_t dataSize = container->m_size;
        bytes.assign(dataPtr, dataPtr + dataSize);
        desc.num_mipmaps = 1;
        imageRef->setFilename(filename);
        imageRef->setDescription(desc);
        if (Inline::isDebugLabelEnabled()) {
            imageRef->setLabel(imageRef->filenameConstString());
        }
        imageRef->setOriginData(bytes.data(), dataSize);
        imageRef->setFileExist(true);
        imageRef->create();
        SG_LABEL_IMAGE(imageRef->handle(), filename.c_str());
        bimg::imageFree(container);
    }
}

Vector4
loadSharedColor(const nanoem_u8_t *data, size_t size)
{
    nanoem_parameter_assert(data, "must not be NULL");
    nanoem_parameter_assert(size > 0, "must not be NULL");
    Vector4 color(0);
    bx::Error err;
    if (bimg::ImageContainer *container =
            bimg::imageParse(g_bimg_allocator, data, nanoem_u32_t(size), bimg::TextureFormat::RGBA8, &err)) {
        const nanoem_u8_t *ptr = static_cast<const nanoem_u8_t *>(container->m_data);
        /* fetch left-bottom corner pixel */
        const nanoem_rsize_t offset = nanoem_rsize_t(container->m_height - 1) * container->m_width * 4;
        color = glm::make_vec4(ptr + offset);
        bimg::imageFree(container);
    }
    return color;
}

} /* namespace anonymous */

namespace nanoem {
namespace resources {

void
initializeSharedToonTextures(Image **textures, ByteArray *data)
{
    nanoem_parameter_assert(textures, "must not be NULL");
    loadSharedTexture(toon01_data, toon01_size, 1, textures[0], data[0]);
    loadSharedTexture(toon02_data, toon02_size, 2, textures[1], data[1]);
    loadSharedTexture(toon03_data, toon03_size, 3, textures[2], data[2]);
    loadSharedTexture(toon04_data, toon04_size, 4, textures[3], data[3]);
    loadSharedTexture(toon05_data, toon05_size, 5, textures[4], data[4]);
    loadSharedTexture(toon06_data, toon06_size, 6, textures[5], data[5]);
    loadSharedTexture(toon07_data, toon07_size, 7, textures[6], data[6]);
    loadSharedTexture(toon08_data, toon08_size, 8, textures[7], data[7]);
    loadSharedTexture(toon09_data, toon09_size, 9, textures[8], data[8]);
    loadSharedTexture(toon10_data, toon10_size, 10, textures[9], data[9]);
}

void
initializeSharedToonColors(Vector4 *colors)
{
    colors[0] = loadSharedColor(toon01_data, toon01_size);
    colors[1] = loadSharedColor(toon02_data, toon02_size);
    colors[2] = loadSharedColor(toon03_data, toon03_size);
    colors[3] = loadSharedColor(toon04_data, toon04_size);
    colors[4] = loadSharedColor(toon05_data, toon05_size);
    colors[5] = loadSharedColor(toon06_data, toon06_size);
    colors[6] = loadSharedColor(toon07_data, toon07_size);
    colors[7] = loadSharedColor(toon08_data, toon08_size);
    colors[8] = loadSharedColor(toon09_data, toon09_size);
    colors[9] = loadSharedColor(toon10_data, toon10_size);
}

} /* namespace resources */
} /* namespace nanoem */
