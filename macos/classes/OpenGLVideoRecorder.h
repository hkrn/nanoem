/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#ifndef NANOEM_EMAPP_MACOS_OPENGLVIDEORECORDER_H_
#define NANOEM_EMAPP_MACOS_OPENGLVIDEORECORDER_H_

#import "BaseVideoRecorder.h"

@class NSOpenGLContext;

namespace nanoem {
namespace macos {

class OpenGLVideoRecorder final : public BaseVideoRecorder {
public:
    OpenGLVideoRecorder(Project *projectPtr, NSOpenGLContext *context, void *func);
    ~OpenGLVideoRecorder() noexcept;

    bool capture(nanoem_frame_index_t frameIndex) override;

private:
    typedef intptr_t (*PFN_sgx_get_native_pass_handle)(sg_pass);
    void updateDepthImage(IOSurfaceRef surface);

    PFN_sgx_get_native_pass_handle _sgx_get_native_pass_handle = nullptr;
    NSOpenGLContext *m_context = nil;
    GLuint m_texture = 0;
};

} /* namespace macos */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_MACOS_OPENGLVIDEORECORDER_H_ */
