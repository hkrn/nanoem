/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

/* insert dummy functions */
#if !defined(NANOEM_WIN32_HAS_OPENGL) && defined(_WIN32) && defined(_M_ARM64)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
int ChoosePixelFormat(HDC hdc, const PIXELFORMATDESCRIPTOR* ppfd) { return 0; }
int DescribePixelFormat(HDC hdc, int iPixelFormat, UINT  nBytes, LPPIXELFORMATDESCRIPTOR ppfd) { return 0; }
BOOL SetPixelFormat(HDC hdc, int format, const PIXELFORMATDESCRIPTOR* ppfd) { return 0; }
BOOL SwapBuffers(HDC Arg1) { return 0; }
#endif /* NANOEM_WIN32_HAS_OPENGL */

#define SOKOL_IMPL
#define SOKOL_AUDIO_IMPL
#include "sokol/sokol_app.h"
#include "sokol/sokol_audio.h"
