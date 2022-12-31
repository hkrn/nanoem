/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#ifndef NANOEM_SHADER_MACROS_HLSL_
#define NANOEM_SHADER_MACROS_HLSL_

#if defined(GLSLANG)
#define GLSLANG_ANNOTATION(a) a
#else
#define GLSLANG_ANNOTATION(a)
#endif

#ifndef VK_DESCRIPTOR_SET_SAMPLER
#define VK_DESCRIPTOR_SET_SAMPLER 0
#endif /* VK_DESCRIPTOR_SET_SAMPLER */

#ifndef VK_DESCRIPTOR_SET_TEXTURE
#define VK_DESCRIPTOR_SET_TEXTURE 0
#endif /* VK_DESCRIPTOR_SET_TEXTURE */

#ifndef VK_DESCRIPTOR_SET_UNIFORM
#define VK_DESCRIPTOR_SET_UNIFORM 0
#endif /* VK_DESCRIPTOR_SET_UNIFORM */

#ifndef VK_DESCRIPTOR_SET_BUFFER
#define VK_DESCRIPTOR_SET_BUFFER 0
#endif /* VK_DESCRIPTOR_SET_BUFFER */

#endif /* NANOEM_SHADER_MACROS_HLSL_ */
