#import <Metal/Metal.h>

#define SOKOL_IMPL
#include "./sokol_delegate.c"

static void
sgx_read_image_core(const _sg_image_t *image_ptr, sg_buffer buffer, void *data, size_t size)
{
    uint32_t texture_pool_index, buffer_pool_index;
    _sg_buffer_t *buffer_ptr = _sg_lookup_buffer(&_sg.pools, buffer.id);
    if (buffer_ptr) {
        buffer_pool_index = buffer_ptr->mtl.buf[buffer_ptr->cmn.active_slot];
        if (image_ptr->mtl.depth_tex != _SG_MTL_INVALID_SLOT_INDEX) {
            texture_pool_index = image_ptr->mtl.depth_tex;
        }
        else {
            texture_pool_index = image_ptr->mtl.tex[image_ptr->cmn.active_slot];
        }
        __unsafe_unretained id<MTLTexture> source = _sg_mtl_id(texture_pool_index);
        __unsafe_unretained id<MTLBuffer> dest = _sg_mtl_id(buffer_pool_index);
        if (source && dest) {
            const int width = image_ptr->cmn.width, height = image_ptr->cmn.height;
            id<MTLCommandBuffer> mtl_command_buffer = [_sg.mtl.cmd_queue commandBuffer];
            id<MTLBlitCommandEncoder> mtl_blitter = [mtl_command_buffer blitCommandEncoder];
            size_t stride = size / height;
            [mtl_blitter copyFromTexture:source
                         sourceSlice:0
                         sourceLevel:0
                         sourceOrigin:MTLOriginMake(0, 0, 0)
                         sourceSize:MTLSizeMake(width, height, 1)
                         toBuffer:dest
                         destinationOffset:0
                         destinationBytesPerRow:stride
                         destinationBytesPerImage:size];
            [mtl_blitter synchronizeResource:dest];
            [mtl_blitter endEncoding];
            [mtl_command_buffer commit];
            [mtl_command_buffer waitUntilCompleted];
            memcpy(data, dest.contents, size);
        }
    }
}

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

SGX_API_DECL void
sgx_label_buffer(sg_buffer buffer, const char *text)
{
    _sg_buffer_t *ptr = _sg_lookup_buffer(&_sg.pools, buffer.id);
    if (ptr && text) {
        uint32_t pool_index = ptr->mtl.buf[ptr->cmn.active_slot];
        __unsafe_unretained id<MTLBuffer> mtl_buf = _sg_mtl_id(pool_index);
        NSString *label = [[NSString alloc] initWithUTF8String:text];
        mtl_buf.label = label;
    }
}

SGX_API_DECL void
sgx_label_image(sg_image image, const char *text)
{
    _sg_image_t *ptr = _sg_lookup_image(&_sg.pools, image.id);
    if (ptr && text) {
        uint32_t pool_index;
        if (ptr->mtl.msaa_tex != _SG_MTL_INVALID_SLOT_INDEX) {
            pool_index = ptr->mtl.msaa_tex;
        }
        else if (ptr->mtl.depth_tex != _SG_MTL_INVALID_SLOT_INDEX) {
            pool_index = ptr->mtl.depth_tex;
        }
        else {
            pool_index = ptr->mtl.tex[ptr->cmn.active_slot];
        }
        if (pool_index != _SG_MTL_INVALID_SLOT_INDEX) {
            __unsafe_unretained id<MTLTexture> mtl_tex = _sg_mtl_id(pool_index);
            NSString *label = [[NSString alloc] initWithUTF8String:text];
            mtl_tex.label = label;
        }
    }
}

SGX_API_DECL void
sgx_label_shader(sg_shader shader, const char *text)
{
    _sg_shader_t *ptr = _sg_lookup_shader(&_sg.pools, shader.id);
    if (ptr && text) {
        uint32_t pool_index_vs = ptr->mtl.stage[SG_SHADERSTAGE_VS].mtl_func;
        uint32_t pool_index_fs = ptr->mtl.stage[SG_SHADERSTAGE_FS].mtl_func;
        if (pool_index_vs != _SG_MTL_INVALID_SLOT_INDEX && pool_index_fs != _SG_MTL_INVALID_SLOT_INDEX) {
            NSString *label = [[NSString alloc] initWithUTF8String:text];
            __unsafe_unretained id<MTLFunction> mtl_func_vert = _sg_mtl_id(pool_index_vs);
            mtl_func_vert.label = label;
            __unsafe_unretained id<MTLFunction> mtl_func_frag = _sg_mtl_id(pool_index_fs);
            mtl_func_frag.label = label;
        }
    }
}

SGX_API_DECL void
sgx_label_pass(sg_pass pass, const char *text)
{
    _SOKOL_UNUSED(pass);
    if (_sg.mtl.cmd_encoder && text) {
        NSString *label = [[NSString alloc] initWithUTF8String:text];
        _sg.mtl.cmd_encoder.label = label;
    }
}

SGX_API_DECL void
sgx_label_pipeline(sg_pipeline pipeline, const char *text)
{
    _sg_pipeline_t *ptr = _sg_lookup_pipeline(&_sg.pools, pipeline.id);
    if (ptr && text) {
#if 0
        uint32_t pool_index = ptr->mtl.rps;
        if (pool_index != _SG_MTL_INVALID_SLOT_INDEX) {
            __unsafe_unretained id<MTLRenderPipelineState> mtl_rps = _sg_mtl_id(pool_index);
            NSString *label = [[NSString alloc] initWithUTF8String:text];
            mtl_rps.label = label;
        }
        pool_index = ptr->mtl.dss;
        if (pool_index != _SG_MTL_INVALID_SLOT_INDEX) {
            __unsafe_unretained id<MTLDepthStencilState> mtl_dss = _sg_mtl_id(pool_index);
            NSString *label = [[NSString alloc] initWithUTF8String:text];
            mtl_dss.label = label;
        }
#endif
    }
}

SGX_API_DECL void
sgx_push_group(const char *text)
{

    if (@available(macOS 10.13, *)) {
        if (_sg.mtl.cmd_buffer && text) {
            NSString *label = [[NSString alloc] initWithUTF8String:text];
            [_sg.mtl.cmd_buffer pushDebugGroup:label];
            sg_push_debug_group(text);
        }
    }
}

SGX_API_DECL void
sgx_pop_group(void)
{
    if (@available(macOS 10.13, *)) {
        if (_sg.mtl.cmd_buffer) {
            [_sg.mtl.cmd_buffer popDebugGroup];
            sg_pop_debug_group();
        }
    }
}

SGX_API_DECL void
sgx_read_image(sg_image image, sg_buffer buffer, void *data, size_t size)
{
    const _sg_image_t *ptr = _sg_lookup_image(&_sg.pools, image.id);
    if (ptr) {
        sgx_read_image_core(ptr, buffer, data, size);
    }
}

SGX_API_DECL void
sgx_read_pass(sg_pass pass, sg_buffer buffer, void *data, size_t size)
{
    _sg_pass_t *ptr = _sg_lookup_pass(&_sg.pools, pass.id);
    if (ptr) {
        const _sg_image_t *image = _sg_lookup_image(&_sg.pools, ptr->cmn.color_atts[0].image_id.id);
        if (image) {
            sgx_read_image_core(image, buffer, data, size);
        }
    }
}

typedef void (*sgx_read_pass_async_callback)(const void *, size_t, void *);
SGX_API_DECL void
sgx_read_pass_async(sg_pass pass, sg_buffer buffer, sgx_read_pass_async_callback callback, void *opaque)
{
    _sg_pass_t *ptr = _sg_lookup_pass(&_sg.pools, pass.id);
    if (ptr) {
        uint32_t texture_pool_index, buffer_pool_index;
        const _sg_image_t *image_ptr = _sg_lookup_image(&_sg.pools, ptr->cmn.color_atts[0].image_id.id);
        _sg_buffer_t *buffer_ptr = _sg_lookup_buffer(&_sg.pools, buffer.id);
        if (image_ptr && buffer_ptr) {
            buffer_pool_index = buffer_ptr->mtl.buf[buffer_ptr->cmn.active_slot];
            if (image_ptr->mtl.depth_tex != _SG_MTL_INVALID_SLOT_INDEX) {
                texture_pool_index = image_ptr->mtl.depth_tex;
            }
            else {
                texture_pool_index = image_ptr->mtl.tex[image_ptr->cmn.active_slot];
            }
            __unsafe_unretained id<MTLBuffer> dest = _sg_mtl_id(buffer_pool_index);
            __unsafe_unretained id<MTLTexture> source = _sg_mtl_id(texture_pool_index);
            if (source && dest) {
                const int width = image_ptr->cmn.width, height = image_ptr->cmn.height;
                id<MTLCommandBuffer> cmd_buffer = [_sg.mtl.cmd_queue commandBuffer];
                id<MTLBlitCommandEncoder> mtl_blitter = [cmd_buffer blitCommandEncoder];
                size_t size = dest.length, stride = size / height;
                [mtl_blitter copyFromTexture:source
                             sourceSlice:0
                             sourceLevel:0
                             sourceOrigin:MTLOriginMake(0, 0, 0)
                             sourceSize:MTLSizeMake(width, height, 1)
                             toBuffer:dest
                             destinationOffset:0
                             destinationBytesPerRow:stride
                             destinationBytesPerImage:size];
                [mtl_blitter endEncoding];
                [cmd_buffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
                    _SOKOL_UNUSED(buffer);
                    callback(dest.contents, size, opaque);
                }];
                [cmd_buffer commit];
            }
        }
    }
}

SGX_API_DECL void *
sgx_map_buffer(sg_buffer buffer)
{
    _sg_buffer_t *ptr = _sg_lookup_buffer(&_sg.pools, buffer.id);
    void *address = 0;
    if (ptr) {
        uint32_t pool_index = ptr->mtl.buf[ptr->cmn.active_slot];
        if (pool_index != _SG_MTL_INVALID_SLOT_INDEX) {
            __unsafe_unretained id<MTLBuffer> mtl_buf = _sg_mtl_id(pool_index);
            address = mtl_buf.contents;
        }
    }
    return 0;
}

SGX_API_DECL void
sgx_unmap_buffer(sg_buffer buffer, void *address)
{
    _sg_buffer_t *ptr = _sg_lookup_buffer(&_sg.pools, buffer.id);
    if (ptr && address) {
        uint32_t pool_index = ptr->mtl.buf[ptr->cmn.active_slot];
        if (pool_index != _SG_MTL_INVALID_SLOT_INDEX) {
            __unsafe_unretained id<MTLBuffer> mtl_buf = _sg_mtl_id(pool_index);
            [mtl_buf didModifyRange:NSMakeRange(0, mtl_buf.length)];
        }
    }
}

SGX_API_DECL void
sgx_insert_marker(const char *text)
{
    if (_sg.mtl.cmd_encoder && text) {
        NSString *label = [[NSString alloc] initWithUTF8String:text];
        [_sg.mtl.cmd_encoder insertDebugSignpost:label];
    }
}

SGX_API_DECL void *
sgx_mtl_cmd_queue(void)
{
    return (__bridge void *) _sg.mtl.cmd_queue;
}
