/*
  Copyright (c) 2015-2021 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#pragma once
#ifndef NANOEM_EMAPP_EFFECT_ANIMATEDIMAGECONTAINER_H_
#define NANOEM_EMAPP_EFFECT_ANIMATEDIMAGECONTAINER_H_

#include "emapp/Forward.h"

namespace nanoem {

namespace image {
class APNG;
} /* namespace image */

namespace effect {

class AnimatedImageContainer NANOEM_DECL_SEALED {
public:
    AnimatedImageContainer(
        const String &name, const String &seekVariable, image::APNG *image, nanoem_f32_t offset, nanoem_f32_t speed);
    ~AnimatedImageContainer() NANOEM_DECL_NOEXCEPT;

    void create();
    void update(nanoem_f32_t seconds);
    void markDirty();
    void destroy() NANOEM_DECL_NOEXCEPT;

    const char *seekVariableNameConstString() const NANOEM_DECL_NOEXCEPT;
    sg_image colorImage() const NANOEM_DECL_NOEXCEPT;
    nanoem_f32_t offset() const NANOEM_DECL_NOEXCEPT;
    nanoem_f32_t speed() const NANOEM_DECL_NOEXCEPT;

private:
    const String m_name;
    const String m_seekVariable;
    image::APNG *m_data;
    sg_image m_colorImage;
    sg_image_desc m_colorImageDescription;
    nanoem_f32_t m_offset;
    nanoem_f32_t m_speed;
    bool m_dirty;
};

} /* namespace effect */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_EFFECT_ANIMATEDIMAGECONTAINER_H_ */
