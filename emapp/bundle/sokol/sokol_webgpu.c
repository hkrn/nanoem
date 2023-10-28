#define SOKOL_WGPU
#define SOKOL_IMPL
#include "./sokol_delegate.c"

#include <stdlib.h>

SGX_API_DECL void APIENTRY
sgx_setup(const sg_desc *desc)
{
    sg_setup(desc);
}

SGX_API_DECL void APIENTRY
sgx_shutdown(void)
{
    sg_shutdown();
}

SGX_API_DECL void APIENTRY
sgx_label_buffer(sg_buffer buffer, const char *text)
{
    _SOKOL_UNUSED(buffer);
    _SOKOL_UNUSED(text);
}

SGX_API_DECL void APIENTRY
sgx_label_image(sg_image image, const char *text)
{
    _SOKOL_UNUSED(image);
    _SOKOL_UNUSED(text);
}

SGX_API_DECL void APIENTRY
sgx_label_shader(sg_shader shader, const char *text)
{
    _SOKOL_UNUSED(shader);
    _SOKOL_UNUSED(text);
}

SGX_API_DECL void APIENTRY
sgx_label_pass(sg_pass pass, const char *text)
{
    _SOKOL_UNUSED(pass);
    _SOKOL_UNUSED(text);
}

SGX_API_DECL void APIENTRY
sgx_label_pipeline(sg_pipeline pipeline, const char *text)
{
    _SOKOL_UNUSED(pipeline);
    _SOKOL_UNUSED(text);
}

SGX_API_DECL void APIENTRY
sgx_push_group(const char *text)
{
    WGPUCommandEncoder encoder = _sg.wgpu.cmd_enc;
    if (encoder) {
        wgpuCommandEncoderPushDebugGroup(encoder, text);
    }
}

SGX_API_DECL void APIENTRY
sgx_pop_group(void)
{
    WGPUCommandEncoder encoder = _sg.wgpu.cmd_enc;
    if (encoder) {
        wgpuCommandEncoderPopDebugGroup(encoder);
    }
}

SGX_API_DECL void APIENTRY
sgx_read_image(sg_image image, void *data, size_t size)
{
#if 0
    _sg_image_t *img = _sg_lookup_image(&_sg.pools, image.id);
    if (img) {
        const WGPUCommandEncoderDescriptor cmd_buf_desc = {
            NULL,
            NULL
        };
        WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(_sg.wgpu.dev, &cmd_buf_desc);
        const WGPUBufferDescriptor buf_desc = {
            NULL,
            NULL,
            WGPUBufferUsage_CopyDst,
            size
        };
        WGPUBuffer buffer = wgpuDeviceCreateBuffer(_sg.wgpu.dev, &buf_desc);
        const WGPUExtent3D extent = { img->cmn.width, img->cmn.height, img->cmn.depth };
        const WGPUTextureCopyView src = {
            NULL,
            img->wgpu.tex,
            1,
            1,
            { 0, 0, 0 }
        };
        const WGPUBufferCopyView dst = {
            NULL,
            buffer,
            0,
            size / img->cmn.height,
            img->cmn.height,
            (size / img->cmn.height) * img->cmn.width,
            img->cmn.height
        };
        wgpuCommandEncoderCopyTextureToBuffer(encoder, &src, &dst, &extent);
        wgpuBufferMapReadAsync(buffer, 0, 0);
        WGPUCommandBuffer cmd_buf = wgpuCommandEncoderFinish(encoder, 0);
        wgpuQueueSubmit(_sg.wgpu.queue, 1, &cmd_buf);
        wgpuCommandBufferRelease(cmd_buf);
        wgpuCommandEncoderRelease(encoder);
    }
#else
#endif
    _SOKOL_UNUSED(image);
    _SOKOL_UNUSED(data);
    _SOKOL_UNUSED(size);
}

SGX_API_DECL void APIENTRY
sgx_read_pass(sg_pass pass, void *data, size_t size)
{
    _SOKOL_UNUSED(pass);
    _SOKOL_UNUSED(data);
    _SOKOL_UNUSED(size);
}

SGX_API_DECL void *APIENTRY
sgx_map_buffer(sg_buffer buffer)
{
#if 0
    _sg_buffer_t *buf = _sg_lookup_buffer(&_sg.pools, buffer.id);
    if (buf) {
        wgpuBufferMapWriteAsync(buf->wgpu.buf, NULL, NULL);
    }
#else
    _SOKOL_UNUSED(buffer);
#endif
    return 0;
}

SGX_API_DECL void APIENTRY
sgx_unmap_buffer(sg_buffer buffer, void *address)
{
#if 0
    _sg_buffer_t *buf = _sg_lookup_buffer(&_sg.pools, buffer.id);
    if (buf) {
        wgpuBufferUnmap(buf->wgpu.buf);
    }
#else
    _SOKOL_UNUSED(buffer);
#endif
    _SOKOL_UNUSED(address);
}

SGX_API_DECL void APIENTRY
sgx_insert_marker(const char *text)
{
    WGPUCommandEncoder encoder = _sg.wgpu.cmd_enc;
    if (encoder) {
        wgpuCommandEncoderInsertDebugMarker(encoder, text);
    }
}
