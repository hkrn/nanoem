#if defined(__MINGW32__)
#include <guiddef.h>
const GUID WKPDID_D3DDebugObjectNameW = { 0x4cca5fd8, 0x921f, 0x42c8, { 0x85, 0x66, 0x70, 0xca, 0xf2, 0xa9, 0xb7, 0x41 } };
#endif

#define SOKOL_IMPL
#define SOKOL_D3D11
#include "./sokol_delegate.c"

#include <d3d11_1.h>

static ID3DUserDefinedAnnotation *g_annotation = 0;

static wchar_t *
appendSuffix(const char *text, size_t length, const wchar_t *suffix, int *size)
{
    size_t count = length * sizeof(wchar_t);
    size_t actual_size = (count + wcslen(suffix)) * sizeof(wchar_t);
    wchar_t *name = (wchar_t *)malloc(actual_size + sizeof(wchar_t));
    name[MultiByteToWideChar(CP_UTF8, 0, text, length, name, count)] = 0;
    wcscat(name, suffix);
    *size = actual_size;
    return name;
}

static void
sgx_read_image_core(const _sg_image_t *image, void *data, size_t size)
{
    ID3D11Texture2D *src_tex = image->d3d11.tex2d;
    D3D11_TEXTURE2D_DESC desc;
    memset(&desc, 0, sizeof(desc));
    desc.Usage = D3D11_USAGE_STAGING;
    desc.Width = image->cmn.width;
    desc.Format = image->d3d11.format;
    desc.Height = image->cmn.height;
    desc.ArraySize = 1;
    desc.MipLevels = 1;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    ID3D11Texture2D *stg_tex;
    _sg.d3d11.dev->lpVtbl->CreateTexture2D(_sg.d3d11.dev, &desc, 0, &stg_tex);
    ID3D11Resource *src_res, *dst_res;
    src_tex->lpVtbl->QueryInterface(src_tex, &IID_ID3D11Resource, (void **) &src_res);
    stg_tex->lpVtbl->QueryInterface(stg_tex, &IID_ID3D11Resource, (void **) &dst_res);
    _sg.d3d11.ctx->lpVtbl->CopyResource(_sg.d3d11.ctx, dst_res, src_res);
    D3D11_MAPPED_SUBRESOURCE src = { 0, 0, 0 };
    _sg.d3d11.ctx->lpVtbl->Map(_sg.d3d11.ctx, dst_res, 0, D3D11_MAP_READ, 0, &src);
    if (src.pData) {
        memcpy(data, src.pData, size);
    }
    _sg.d3d11.ctx->lpVtbl->Unmap(_sg.d3d11.ctx, dst_res, 0);
    src_res->lpVtbl->Release(src_res);
    dst_res->lpVtbl->Release(dst_res);
    stg_tex->lpVtbl->Release(stg_tex);
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

SGX_API_DECL void APIENTRY
sgx_label_buffer(sg_buffer buffer, const char *text)
{
    _sg_buffer_t *ptr = _sg_lookup_buffer(&_sg.pools, buffer.id);
    if (ptr && text) {
        int length = strlen(text), count = MultiByteToWideChar(CP_UTF8, 0, text, length, 0, 0), size = count * sizeof(wchar_t);
        wchar_t *name = (wchar_t *) malloc(size);
        if (name) {
            MultiByteToWideChar(CP_UTF8, 0, text, length, name, count);
            ID3D11Buffer *buffer = ptr->d3d11.buf;
            buffer->lpVtbl->SetPrivateData(buffer, &WKPDID_D3DDebugObjectNameW, 0, NULL);
            buffer->lpVtbl->SetPrivateData(buffer, &WKPDID_D3DDebugObjectNameW, size, name);
            free(name);
        }
    }
}

SGX_API_DECL void APIENTRY
sgx_label_image(sg_image image, const char *text)
{
    _sg_image_t *ptr = _sg_lookup_image(&_sg.pools, image.id);
    if (ptr && text) {
        int new_size, length = strlen(text), count = MultiByteToWideChar(CP_UTF8, 0, text, length, 0, 0), size = count * sizeof(wchar_t);
        wchar_t *name = NULL;
        ID3D11Texture2D *tex2d = ptr->d3d11.tex2d;
        if (tex2d) {
            name = appendSuffix(text, length, L"/Texture2D", &new_size);
            if (name) {
                tex2d->lpVtbl->SetPrivateData(tex2d, &WKPDID_D3DDebugObjectNameW, 0, NULL);
                tex2d->lpVtbl->SetPrivateData(tex2d, &WKPDID_D3DDebugObjectNameW, new_size, name);
                free(name);
            }
        }
        ID3D11Texture3D *tex3d = ptr->d3d11.tex3d;
        if (tex3d) {
            name = appendSuffix(text, length, L"/Texture3D", &new_size);
            if (name) {
                tex3d->lpVtbl->SetPrivateData(tex3d, &WKPDID_D3DDebugObjectNameW, 0, NULL);
                tex3d->lpVtbl->SetPrivateData(tex3d, &WKPDID_D3DDebugObjectNameW, new_size, name);
                free(name);
            }
        }
        ID3D11Texture2D *texds = ptr->d3d11.texds;
        if (texds) {
            name = appendSuffix(text, length, L"/DepthStencil", &new_size);
            if (name) {
                texds->lpVtbl->SetPrivateData(texds, &WKPDID_D3DDebugObjectNameW, 0, NULL);
                texds->lpVtbl->SetPrivateData(texds, &WKPDID_D3DDebugObjectNameW, new_size, name);
                free(name);
            }
        }
        ID3D11Texture2D *texmsaa = ptr->d3d11.texmsaa;
        if (texmsaa) {
            name = appendSuffix(text, length, L"/MSAA", &new_size);
            if (name) {
                texmsaa->lpVtbl->SetPrivateData(texmsaa, &WKPDID_D3DDebugObjectNameW, 0, NULL);
                texmsaa->lpVtbl->SetPrivateData(texmsaa, &WKPDID_D3DDebugObjectNameW, new_size, name);
                free(name);
            }
        }
        ID3D11SamplerState *smp = ptr->d3d11.smp;
        if (smp) {
            name = appendSuffix(text, length, L"/SamplerState", &new_size);
            if (name) {
                smp->lpVtbl->SetPrivateData(smp, &WKPDID_D3DDebugObjectNameW, 0, NULL);
                smp->lpVtbl->SetPrivateData(smp, &WKPDID_D3DDebugObjectNameW, new_size, name);
                free(name);
            }
        }
        ID3D11ShaderResourceView *srv = ptr->d3d11.srv;
        if (srv) {
            name = appendSuffix(text, length, L"/ShaderResourceView", &new_size);
            if (name) {
                srv->lpVtbl->SetPrivateData(srv, &WKPDID_D3DDebugObjectNameW, 0, NULL);
                srv->lpVtbl->SetPrivateData(srv, &WKPDID_D3DDebugObjectNameW, new_size, name);
                free(name);
            }
        }
    }
}

SGX_API_DECL void APIENTRY
sgx_label_shader(sg_shader shader, const char *text)
{
    _sg_shader_t *ptr = _sg_lookup_shader(&_sg.pools, shader.id);
    if (ptr && text) {
        wchar_t *name = NULL;
        int length = strlen(text), count = MultiByteToWideChar(CP_UTF8, 0, text, length, 0, 0), size = count * sizeof(wchar_t);
        ID3D11PixelShader *fs = ptr->d3d11.fs;
        if (fs) {
            int new_size;
            name = appendSuffix(text, length, L"/PixelShader", &new_size);
            if (name) {
                fs->lpVtbl->SetPrivateData(fs, &WKPDID_D3DDebugObjectNameW, 0, NULL);
                fs->lpVtbl->SetPrivateData(fs, &WKPDID_D3DDebugObjectNameW, new_size, name);
                free(name);
            }
        }
        ID3D11VertexShader *vs = ptr->d3d11.vs;
        if (vs) {
            int new_size;
            name = appendSuffix(text, length, L"/VertexShader", &new_size);
            if (name) {
                vs->lpVtbl->SetPrivateData(vs, &WKPDID_D3DDebugObjectNameW, 0, NULL);
                vs->lpVtbl->SetPrivateData(vs, &WKPDID_D3DDebugObjectNameW, new_size, name);
                free(name);
            }
        }
        for (int i = 0; i < SG_NUM_SHADER_STAGES; i++) {
            const _sg_shader_stage_t *stage = &ptr->cmn.stage[i];
            _sg_d3d11_shader_stage_t *stage_d3d11 = &ptr->d3d11.stage[i];
            for (int j = 0, num_uniform_blocks = stage->num_uniform_blocks; j < num_uniform_blocks; j++) {
                ID3D11Buffer *buffer = stage_d3d11->cbufs[j];
                if (buffer) {
                    const wchar_t *suffix = i == SG_SHADERSTAGE_VS ? L"/VSConstantBuffer" : L"/PSConstantBuffer";
                    int new_size;
                    name = appendSuffix(text, length,  suffix, &new_size);
                    if (name) {
                        buffer->lpVtbl->SetPrivateData(buffer, &WKPDID_D3DDebugObjectNameW, 0, NULL);
                        buffer->lpVtbl->SetPrivateData(buffer, &WKPDID_D3DDebugObjectNameW, new_size, name);
                        free(name);
                    }
                }
            }
        }
    }
}

SGX_API_DECL void APIENTRY
sgx_label_pass(sg_pass pass, const char *text)
{
    _sg_pass_t *ptr = _sg_lookup_pass(&_sg.pools, pass.id);
    if (ptr && text) {
        int length = strlen(text), count = MultiByteToWideChar(CP_UTF8, 0, text, length, 0, 0), size = count * sizeof(wchar_t);
        wchar_t *name = (wchar_t *)malloc(size);
        if (name) {
            MultiByteToWideChar(CP_UTF8, 0, text, length, name, count);
            for (int i = 0; i < SG_MAX_COLOR_ATTACHMENTS; i++) {
                ID3D11RenderTargetView *color_view = ptr->d3d11.color_atts[i].rtv;
                if (color_view) {
                    wchar_t suffix[16];
                    int new_size;
                    swprintf_s(suffix, ARRAYSIZE(suffix), L"/ColorView/%d", i);
                    name = appendSuffix(text, length, suffix, &new_size);
                    if (name) {
                        color_view->lpVtbl->SetPrivateData(color_view, &WKPDID_D3DDebugObjectNameW, 0, NULL);
                        color_view->lpVtbl->SetPrivateData(color_view, &WKPDID_D3DDebugObjectNameW, new_size, name);
                        free(name);
                    }
                }
            }
            ID3D11RenderTargetView *depth_view = ptr->d3d11.ds_att.dsv;
            if (depth_view) {
                int new_size;
                name = appendSuffix(text, length, L"/DepthStencilView", &new_size);
                if (name) {
                    depth_view->lpVtbl->SetPrivateData(depth_view, &WKPDID_D3DDebugObjectNameW, 0, NULL);
                    depth_view->lpVtbl->SetPrivateData(depth_view, &WKPDID_D3DDebugObjectNameW, new_size, name);
                    free(name);
                }
            }
        }
    }
}

SGX_API_DECL void APIENTRY
sgx_label_pipeline(sg_pipeline pipeline, const char *text)
{
    _sg_pipeline_t *ptr = _sg_lookup_pipeline(&_sg.pools, pipeline.id);
    if (ptr && text) {
        wchar_t *name = 0;
        int length = strlen(text), count = MultiByteToWideChar(CP_UTF8, 0, text, length, 0, 0), size = count * sizeof(wchar_t);
        ID3D11BlendState *bs = ptr->d3d11.bs;
        if (bs) {
            int new_size;
            name = appendSuffix(text, length, L"/BlendState", &new_size);
            if (name) {
                bs->lpVtbl->SetPrivateData(bs, &WKPDID_D3DDebugObjectNameW, 0, NULL);
                bs->lpVtbl->SetPrivateData(bs, &WKPDID_D3DDebugObjectNameW, new_size, name);
                free(name);
            }
        }
        ID3D11DepthStencilState *dss = ptr->d3d11.dss;
        if (ptr->d3d11.dss) {
            int new_size;
            name = appendSuffix(text, length, L"/DepthStencilState", &new_size);
            dss->lpVtbl->SetPrivateData(dss, &WKPDID_D3DDebugObjectNameW, 0, NULL);
            dss->lpVtbl->SetPrivateData(dss, &WKPDID_D3DDebugObjectNameW, new_size, name);
            free(name);
        }
        ID3D11InputLayout *il = ptr->d3d11.il;
        if (ptr->d3d11.il) {
            int new_size;
            name = appendSuffix(text, length, L"/InputLayout", &new_size);
            if (name) {
                il->lpVtbl->SetPrivateData(il, &WKPDID_D3DDebugObjectNameW, 0, NULL);
                il->lpVtbl->SetPrivateData(il, &WKPDID_D3DDebugObjectNameW, new_size, name);
                free(name);
            }
        }
        ID3D11RasterizerState *rs = ptr->d3d11.rs;
        if (rs) {
            int new_size;
            name = appendSuffix(text, length, L"/RasterizerState", &new_size);
            if (name) {
                rs->lpVtbl->SetPrivateData(rs, &WKPDID_D3DDebugObjectNameW, 0, NULL);
                rs->lpVtbl->SetPrivateData(rs, &WKPDID_D3DDebugObjectNameW, new_size, name);
                free(name);
            }
        }
    }
}

SGX_API_DECL void APIENTRY
sgx_push_group(const char *text)
{
    if (!g_annotation && _sg.d3d11.ctx) {
        _sg.d3d11.ctx->lpVtbl->QueryInterface(_sg.d3d11.ctx, &IID_ID3DUserDefinedAnnotation, &g_annotation);
    }
    if (g_annotation && text) {
        int length = strlen(text), count = MultiByteToWideChar(CP_UTF8, 0, text, length, 0, 0);
        wchar_t *name = (wchar_t *)malloc((count + 1) * sizeof(wchar_t));
        if (name) {
            name[MultiByteToWideChar(CP_UTF8, 0, text, length, name, count)] = 0;
            g_annotation->lpVtbl->BeginEvent(g_annotation, name);
            sg_push_debug_group(name);
            free(name);
        }
    }
}

SGX_API_DECL void APIENTRY
sgx_pop_group(void)
{
    if (g_annotation) {
        g_annotation->lpVtbl->EndEvent(g_annotation);
        sg_pop_debug_group();
    }
}

SGX_API_DECL void APIENTRY
sgx_read_image(sg_image image, sg_buffer buffer, void *data, size_t size)
{
    _SOKOL_UNUSED(buffer);
    const _sg_pass_t *ptr = _sg_lookup_image(&_sg.pools, image.id);
    if (ptr) {
        sgx_read_image_core(ptr, data, size);
    }
}

SGX_API_DECL void APIENTRY
sgx_read_pass(sg_pass pass, sg_buffer buffer, void *data, size_t size)
{
    _SOKOL_UNUSED(buffer);
    const _sg_pass_t *ptr = _sg_lookup_pass(&_sg.pools, pass.id);
    if (ptr) {
        const _sg_image_t *image = _sg_lookup_image(&_sg.pools, ptr->cmn.color_atts[0].image_id.id);
        sgx_read_image_core(image, data, size);
    }
}

SGX_API_DECL intptr_t
sgx_get_native_pass_handle(sg_pass pass)
{
    _SOKOL_UNUSED(pass);
    return 0;
}

SGX_API_DECL void * APIENTRY
sgx_map_buffer(sg_buffer buffer)
{
    _sg_buffer_t *ptr = _sg_lookup_buffer(&_sg.pools, buffer.id);
    D3D11_MAPPED_SUBRESOURCE src = { 0, 0, 0 };
    if (ptr) {
        ID3D11Resource *res;
        ptr->d3d11.buf->lpVtbl->QueryInterface(ptr->d3d11.buf, &IID_ID3D11Resource, (void **) &res);
        _sg.d3d11.ctx->lpVtbl->Map(_sg.d3d11.ctx, res, 0, D3D11_MAP_WRITE_DISCARD, 0, &src);
        res->lpVtbl->Release(res);
    }
    return src.pData;
}

SGX_API_DECL void APIENTRY
sgx_unmap_buffer(sg_buffer buffer, void *address)
{
    _sg_buffer_t *ptr = _sg_lookup_buffer(&_sg.pools, buffer.id);
    if (ptr && address) {
        ID3D11Resource *res;
        ptr->d3d11.buf->lpVtbl->QueryInterface(ptr->d3d11.buf, &IID_ID3D11Resource, (void**) &res);
        _sg.d3d11.ctx->lpVtbl->Unmap(_sg.d3d11.ctx, ptr->d3d11.buf, 0);
        res->lpVtbl->Release(res);
    }
}

SGX_API_DECL void APIENTRY
sgx_insert_marker(const char *text)
{
    if (!g_annotation && _sg.d3d11.ctx) {
        _sg.d3d11.ctx->lpVtbl->QueryInterface(_sg.d3d11.ctx, &IID_ID3DUserDefinedAnnotation, &g_annotation);
    }
    if (g_annotation && text) {
        int length = strlen(text), count = MultiByteToWideChar(CP_UTF8, 0, text, length, 0, 0);
        wchar_t *name = (wchar_t *)malloc((count + 1) * sizeof(wchar_t));
        if (name) {
            name[MultiByteToWideChar(CP_UTF8, 0, text, length, name, count)] = 0;
            g_annotation->lpVtbl->SetMarker(g_annotation, name);
            free(name);
        }
    }
}

SGX_API_DECL void APIENTRY
sgx_bootstrap(void)
{
}
