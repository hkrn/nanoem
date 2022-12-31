/*
  Copyright (c) 2015-2023 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#include <d3dx9.h>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "./protoc/effect.pb-c.h"
#include "./protoc/effect_dx9bc.pb-c.h"
#include "emapp/sdk/Effect.h"

#include <minwinbase.h>
#include <winuser.h>

#if _MSC_VER < 1900
#define snprintf _snprintf
#endif

namespace {

typedef IDirect3D9 *(*pfn_Direct3DCreate9)(UINT);
typedef std::vector<std::pair<D3DSAMPLERSTATETYPE, UINT>> SamplerStateList;
struct TextureData {
    TextureData(int index, const std::string &name, const SamplerStateList &states)
        : m_index(index)
        , m_name(name)
        , m_states(states)
    {
    }
    int m_index;
    std::string m_name;
    SamplerStateList m_states;
};

// {4352041E-8FFB-4801-ABBB-B19ECE6F2F07}
static const GUID kTextureIdentifierGUID = { 0x4352041e, 0x8ffb, 0x4801,
    { 0xab, 0xbb, 0xb1, 0x9e, 0xce, 0x6f, 0x2f, 0x7 } };
static const char kSourceHeaderPrefix[] = "sampler _ps_s0 : register(ps,s0);\n"
                                          "sampler _ps_s1 : register(ps,s1);\n"
                                          "sampler _ps_s2 : register(ps,s2);\n"
                                          "sampler _ps_s3 : register(ps,s3);\n";
}

struct nanoem_application_plugin_effect_compiler_t {
    static const int kMaxSamplerStageCount = 16;
    template <typename T>
    static void
    release(T &ptr)
    {
        if (ptr) {
            ptr->Release();
            ptr = nullptr;
        }
    }
    static void
    replaceAll(std::string &str, const std::string &from, const std::string &to)
    {
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
    }
    static void
    toWideCharString(LPCSTR source, std::vector<wchar_t> &dest)
    {
        if (source) {
            const int length = int(strlen(source));
            dest.resize(MultiByteToWideChar(CP_UTF8, 0, source, length, 0, 0) + 1);
            MultiByteToWideChar(CP_UTF8, 0, source, length, dest.data(), int(dest.size()));
        }
    }
    struct FileIncluder : ID3DXInclude {
        FileIncluder(const std::wstring &basePath)
            : m_basePath(basePath)
        {
        }
        STDMETHOD(Open)
        (D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes) override
        {
            std::wstring path(m_basePath);
            path.append(L"/");
            std::vector<wchar_t> filename;
            toWideCharString(pFileName, filename);
            path.append(filename.data());
            HANDLE handle =
                CreateFileW(path.data(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
            HRESULT result = S_FALSE;
            if (handle != INVALID_HANDLE_VALUE) {
                std::vector<char> buffer;
                buffer.resize(GetFileSize(handle, 0));
                DWORD readSize = 0;
                if (ReadFile(handle, buffer.data(), DWORD(buffer.size()), &readSize, 0)) {
                    size_t size = buffer.size();
                    nanoem_u8_t *data = new nanoem_u8_t[size];
                    memcpy(data, buffer.data(), size);
                    *ppData = data;
                    *pBytes = UINT(size);
                    result = S_OK;
                }
                CloseHandle(handle);
            }
            return result;
        }
        STDMETHOD(Close)(LPCVOID pData) override
        {
            const nanoem_u8_t *data = static_cast<const nanoem_u8_t *>(pData);
            delete[] const_cast<nanoem_u8_t *>(data);
            return S_OK;
        }

        std::wstring m_basePath;
    };

    nanoem_application_plugin_effect_compiler_t()
        : m_d3d9(nullptr)
        , m_globalUIContext(nullptr)
        , m_device(nullptr)
        , m_pool(nullptr)
        , m_hwnd(nullptr)
        , m_flags(D3DXSHADER_NO_PRESHADER | D3DXSHADER_ENABLE_BACKWARDS_COMPATIBILITY)
    {
#ifndef NDEBUG
        m_flags |= D3DXSHADER_DEBUG | D3DXSHADER_OPTIMIZATION_LEVEL0;
#endif
        static const TCHAR kClassName[] = TEXT("nanoem_application_plugin_effect_compiler_t");
        HINSTANCE instance = GetModuleHandle(nullptr);
        WNDCLASSEX wnd;
        ZeroMemory(&wnd, sizeof(wnd));
        wnd.cbSize = sizeof(wnd);
        wnd.style = CS_CLASSDC;
        wnd.hInstance = instance;
        wnd.lpszClassName = kClassName;
        wnd.lpfnWndProc = DefWindowProc;
        RegisterClassEx(&wnd);
        m_hwnd =
            CreateWindow(kClassName, kClassName, WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, nullptr, nullptr, nullptr, nullptr);
        if (loadD3D9Library(m_d3d9)) {
            pfn_Direct3DCreate9 _Direct3DCreate9 =
                reinterpret_cast<pfn_Direct3DCreate9>(GetProcAddress(m_d3d9, "Direct3DCreate9"));
            m_globalUIContext = _Direct3DCreate9(D3D_SDK_VERSION);
            D3DPRESENT_PARAMETERS parameters;
            ZeroMemory(&parameters, sizeof(parameters));
            parameters.hDeviceWindow = m_hwnd;
            parameters.Windowed = true;
            parameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
            m_globalUIContext->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hwnd,
                D3DCREATE_HARDWARE_VERTEXPROCESSING, &parameters, &m_device);
            resetDefaultRenderStates();
            for (size_t i = 0; i < ARRAYSIZE(m_renderStates); i++) {
                DWORD value;
                D3DRENDERSTATETYPE key = static_cast<D3DRENDERSTATETYPE>(i);
                if (SUCCEEDED(m_device->GetRenderState(key, &value))) {
                    m_renderStates[key] = value;
                }
                else {
                    m_renderStates[key] = 0;
                }
            }
            for (int i = 0; i < ARRAYSIZE(m_samplerStates); i++) {
                DWORD value;
                D3DSAMPLERSTATETYPE key = static_cast<D3DSAMPLERSTATETYPE>(i);
                if (SUCCEEDED(m_device->GetSamplerState(0, key, &value))) {
                    m_samplerStates[key] = value;
                }
                else {
                    m_samplerStates[key] = 0;
                }
            }
            D3DXCreateEffectPool(&m_pool);
        }
        else {
            m_reason = "Cannot load d3d9.dll";
        }
    }
    ~nanoem_application_plugin_effect_compiler_t()
    {
        release(m_pool);
        release(m_device);
        release(m_globalUIContext);
        DestroyWindow(m_hwnd);
        FreeLibrary(m_d3d9);
    }

    int
    getOption(nanoem_u32_t key, void *value, nanoem_rsize_t * /* size */)
    {
        switch (key) {
        case NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_SHADER_VERSION:
            *static_cast<int *>(value) = 0;
            break;
        case NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_OPTIMIZATION:
            *static_cast<int *>(value) = (m_flags & D3DXSHADER_SKIPOPTIMIZATION) != 0 ? 0 : 1;
            break;
        case NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_VALIDATION:
            *static_cast<int *>(value) = (m_flags & D3DXSHADER_SKIPVALIDATION) != 0 ? 0 : 1;
            break;
        default:
            break;
        }
        return 0;
    }
    int
    setOption(nanoem_u32_t key, const void *value, nanoem_rsize_t /* size */)
    {
        switch (key) {
        case NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_SHADER_VERSION:
            break;
        case NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_OPTIMIZATION:
            if (*static_cast<const int *>(value)) {
                m_flags &= ~D3DXSHADER_SKIPOPTIMIZATION;
            }
            else {
                m_flags |= D3DXSHADER_SKIPOPTIMIZATION;
            }
            break;
        case NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_VALIDATION:
            if (*static_cast<const int *>(value)) {
                m_flags &= ~D3DXSHADER_SKIPVALIDATION;
            }
            else {
                m_flags |= D3DXSHADER_SKIPVALIDATION;
            }
            break;
        default:
            break;
        }
        return 0;
    }
    nanoem_u8_t *
    compile(const char *path, nanoem_rsize_t *outputSize)
    {
        ID3DXEffect *effect = nullptr;
        ID3DXBuffer *error = nullptr;
        nanoem_u8_t *data = nullptr;
        if (m_device) {
            std::vector<wchar_t> newPath;
            toWideCharString(path, newPath);
            HANDLE handle =
                CreateFileW(newPath.data(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (handle != INVALID_HANDLE_VALUE) {
                std::vector<char> buffer;
                buffer.resize(GetFileSize(handle, 0));
                DWORD readSize = 0;
                ReadFile(handle, buffer.data(), DWORD(buffer.size()), &readSize, 0);
                if (const wchar_t *p = wcsrchr(newPath.data(), L'/')) {
                    newPath[p - newPath.data()] = 0;
                }
                FileIncluder includer(newPath.data());
                data = internalCompile(buffer.data(), buffer.size(), outputSize, &includer);
                CloseHandle(handle);
            }
        }
        release(effect);
        release(error);
        return data;
    }
    nanoem_u8_t *
    compile(const char *source, nanoem_rsize_t length, nanoem_rsize_t *outputSize)
    {
        return internalCompile(source, length, outputSize, nullptr);
    }
    void
    addShaderSource(const char *path, const nanoem_u8_t *source, nanoem_rsize_t size)
    {
        UNREFERENCED_PARAMETER(path);
        UNREFERENCED_PARAMETER(source);
        UNREFERENCED_PARAMETER(size);
    }
    void
    destroy(nanoem_u8_t *data)
    {
        delete[] data;
    }

    const char *
    failureReason() const
    {
        return m_reason.c_str();
    }
    const char *
    recoverySuggestion() const
    {
        return nullptr;
    }

    static bool
    loadD3D9Library(HMODULE &handle)
    {
        wchar_t path[MAX_PATH];
        GetModuleFileNameW(nullptr, path, ARRAYSIZE(path));
        if (wchar_t *p = wcsrchr(path, L'\\')) {
            size_t size = ARRAYSIZE(path) - wcslen(path);
            wcscpy_s(p, size, L"\\plugins\\d3d9.dll");
        }
        handle = LoadLibraryW(path);
        return handle != nullptr;
    }
    static char *
    cloneString(const char *value)
    {
        char *s = nullptr;
        if (value) {
            size_t l = strlen(value);
            s = new char[l + 1];
            strcpy_s(s, l + 1, value);
        }
        return s;
    }
    nanoem_u8_t *
    internalCompile(const char *source, nanoem_rsize_t length, nanoem_rsize_t *outputSize, ID3DXInclude *includer)
    {
        ID3DXEffect *effect = nullptr;
        ID3DXBuffer *error = nullptr;
        nanoem_u8_t *data = nullptr;
        if (m_device) {
            m_reason.clear();
            std::string newSource;
#if 1
            newSource.append(kSourceHeaderPrefix);
            newSource.append(source, length);
            replaceAll(newSource, "vs_1_1", "vs_3_0");
            replaceAll(newSource, "ps_1_1", "ps_3_0");
            replaceAll(newSource, "vs_2_0", "vs_3_0");
            replaceAll(newSource, "ps_2_0", "ps_3_0");
            char from[32], to[32];
            for (int i = 12; i >= 4; i--) {
                snprintf(from, sizeof(from), "register(s%d)", i);
                snprintf(to, sizeof(to), "register(s%d)", i + 4);
                replaceAll(newSource, from, to);
            }
#else
            newSource.append(source, length);
#endif
            const D3DXMACRO macros[] = { { "_INDEX", "TEXCOORD7" }, { 0, 0 } };
            HRESULT result = D3DXCreateEffectEx(m_device, newSource.c_str(), UINT(newSource.size()), macros, includer,
                nullptr, m_flags, m_pool, &effect, &error);
            if (SUCCEEDED(result)) {
                data = generateMessage(effect, outputSize);
            }
            else if (error) {
                setErrorReason(error);
            }
            else {
                char buffer[BUFSIZ];
                snprintf(buffer, sizeof(buffer), "HRESULT = 0x%x\n", result);
                m_reason = buffer;
            }
        }
        release(effect);
        release(error);
        return data;
    }
    nanoem_u8_t *
    generateMessage(ID3DXEffect *effect, size_t *output_size)
    {
        nanoem_u8_t *data = nullptr;
        Nanoem__Effect__Effect *message = new Nanoem__Effect__Effect;
        nanoem__effect__effect__init(message);
        parseEffect(effect, message);
        size_t size = nanoem__effect__effect__get_packed_size(message);
        data = new nanoem_u8_t[size];
        *output_size = size;
        nanoem__effect__effect__pack(message, data);
        destroyEffect(message);
        return data;
    }
    void
    parseEffect(ID3DXEffect *effect, Nanoem__Effect__Effect *message)
    {
        std::vector<IDirect3DBaseTexture9 *> textures;
        D3DXEFFECT_DESC desc;
        effect->GetDesc(&desc);
        UINT size = desc.Parameters;
        if (size > 0) {
            message->n_parameters = size;
            message->parameters = new Nanoem__Effect__Parameter *[size];
            for (UINT i = 0; i < size; i++) {
                Nanoem__Effect__Parameter *parameter = new Nanoem__Effect__Parameter;
                message->parameters[i] = parameter;
                parseParameter(effect, i, parameter, textures);
            }
        }
        size = desc.Techniques;
        if (size > 0) {
            message->n_techniques = size;
            message->techniques = new Nanoem__Effect__Technique *[size];
            for (UINT i = 0; i < size; i++) {
                Nanoem__Effect__Technique *technique = new Nanoem__Effect__Technique;
                message->techniques[i] = technique;
                parseTechnique(effect, i, technique);
            }
        }
        for (auto it = textures.begin(), end = textures.end(); it != end; ++it) {
            IDirect3DBaseTexture9 *texture = *it;
            texture->Release();
        }
    }
    void
    destroyEffect(Nanoem__Effect__Effect *message)
    {
        for (size_t i = 0, size = message->n_techniques; i < size; i++) {
            destroyTechnique(message->techniques[i]);
        }
        delete[] message->techniques;
        for (size_t i = 0, size = message->n_parameters; i < size; i++) {
            destroyParameter(message->parameters[i]);
        }
        delete[] message->parameters;
        delete message;
    }
    void
    parseParameter(ID3DXEffect *effect, UINT index, Nanoem__Effect__Parameter *message,
        std::vector<IDirect3DBaseTexture9 *> &textures)
    {
        D3DXPARAMETER_DESC desc;
        D3DXHANDLE parameter = effect->GetParameter(0, index);
        effect->GetParameterDesc(parameter, &desc);
        nanoem__effect__parameter__init(message);
        message->class_case = NANOEM__EFFECT__PARAMETER__CLASS_CLASS_DX9BC;
        message->class_dx9bc = static_cast<Nanoem__Effect__Dx9bc__ParameterClass>(desc.Class);
        message->flags = desc.Flags;
        message->name = cloneString(desc.Name);
        message->num_columns = desc.Columns;
        message->num_elements = desc.Elements;
        message->num_rows = desc.Rows;
        message->semantic = cloneString(desc.Semantic);
        message->type_case = NANOEM__EFFECT__PARAMETER__TYPE_TYPE_DX9BC;
        message->type_dx9bc = static_cast<Nanoem__Effect__Dx9bc__ParameterType>(desc.Type);
        if (desc.Bytes > 0) {
            message->value.data = new nanoem_u8_t[desc.Bytes];
            message->value.len = desc.Bytes;
            effect->GetValue(parameter, message->value.data, UINT(message->value.len));
        }
        if (desc.Type >= D3DXPT_TEXTURE && desc.Type <= D3DXPT_TEXTURECUBE) {
            IDirect3DTexture9 *texture = nullptr;
            if (SUCCEEDED(D3DXCreateTexture(m_device, 1, 1, 1, 0, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT, &texture))) {
                texture->SetPrivateData(kTextureIdentifierGUID, desc.Name, DWORD(strlen(desc.Name) + 1), 0);
                effect->SetTexture(parameter, texture);
                textures.push_back(texture);
            }
        }
        UINT size = desc.Annotations;
        if (size > 0) {
            message->n_annotations = size;
            message->annotations = new Nanoem__Effect__Annotation *[size];
            for (UINT i = 0; i < size; i++) {
                Nanoem__Effect__Annotation *annotation = new Nanoem__Effect__Annotation;
                message->annotations[i] = annotation;
                parseAnnotation(effect, parameter, i, annotation);
            }
        }
    }
    void
    destroyParameter(Nanoem__Effect__Parameter *message)
    {
        delete[] message->name;
        delete[] message->semantic;
        delete[] message->value.data;
        for (size_t i = 0, size = message->n_annotations; i < size; i++) {
            destroyAnnotation(message->annotations[i]);
        }
        delete[] message->annotations;
        delete message;
    }
    void
    parseTechnique(ID3DXEffect *effect, UINT index, Nanoem__Effect__Technique *message)
    {
        D3DXTECHNIQUE_DESC desc;
        D3DXHANDLE technique = effect->GetTechnique(index);
        effect->GetTechniqueDesc(technique, &desc);
        nanoem__effect__technique__init(message);
        message->name = cloneString(desc.Name);
        UINT size = desc.Passes;
        if (size > 0) {
            message->n_passes = size;
            message->passes = new Nanoem__Effect__Pass *[size];
            effect->SetTechnique(technique);
            effect->Begin(nullptr, 0);
            for (UINT i = 0; i < size; i++) {
                Nanoem__Effect__Pass *pass = new Nanoem__Effect__Pass;
                message->passes[i] = pass;
                resetDefaultRenderStates();
                effect->BeginPass(i);
                parsePass(effect, technique, i, pass);
                effect->EndPass();
            }
            effect->End();
        }
        size = desc.Annotations;
        if (size > 0) {
            message->n_annotations = size;
            message->annotations = new Nanoem__Effect__Annotation *[size];
            for (UINT i = 0; i < size; i++) {
                Nanoem__Effect__Annotation *annotation = new Nanoem__Effect__Annotation;
                message->annotations[i] = annotation;
                parseAnnotation(effect, technique, i, annotation);
            }
        }
    }
    void
    destroyTechnique(Nanoem__Effect__Technique *message)
    {
        delete[] message->name;
        for (size_t i = 0, size = message->n_passes; i < size; i++) {
            destroyPass(message->passes[i]);
        }
        delete[] message->passes;
        for (size_t i = 0, size = message->n_annotations; i < size; i++) {
            destroyAnnotation(message->annotations[i]);
        }
        delete[] message->annotations;
        delete message;
    }
    void
    parsePass(ID3DXEffect *effect, D3DXHANDLE technique, UINT index, Nanoem__Effect__Pass *message)
    {
        D3DXPASS_DESC desc;
        D3DXHANDLE pass = effect->GetPass(technique, index);
        effect->GetPassDesc(pass, &desc);
        nanoem__effect__pass__init(message);
        message->name = cloneString(desc.Name);
        message->implementation_case = NANOEM__EFFECT__PASS__IMPLEMENTATION_IMPLEMENTATION_DX9BC;
        Nanoem__Effect__Dx9bc__Pass *impl = new Nanoem__Effect__Dx9bc__Pass;
        parseShaderPass(effect, pass, impl);
        message->implementation_dx9bc = impl;
        UINT size = desc.Annotations;
        if (size > 0) {
            message->n_annotations = size;
            message->annotations = new Nanoem__Effect__Annotation *[size];
            for (UINT i = 0; i < size; i++) {
                Nanoem__Effect__Annotation *annotation = new Nanoem__Effect__Annotation;
                message->annotations[i] = annotation;
                parseAnnotation(effect, pass, i, annotation);
            }
        }
    }
    void
    destroyPass(Nanoem__Effect__Pass *message)
    {
        delete[] message->name;
        destroyShaderPass(message->implementation_dx9bc);
        for (size_t i = 0, size = message->n_annotations; i < size; i++) {
            destroyAnnotation(message->annotations[i]);
        }
        delete[] message->annotations;
        delete message;
    }
    void
    parseShaderPass(ID3DXEffect *effect, D3DXHANDLE pass, Nanoem__Effect__Dx9bc__Pass *message)
    {
        D3DXPASS_DESC desc;
        effect->GetPassDesc(pass, &desc);
        nanoem__effect__dx9bc__pass__init(message);
        message->vertex_shader = new Nanoem__Effect__Dx9bc__Shader;
        parseShader(effect, desc.pVertexShaderFunction, message->vertex_shader);
        message->vertex_shader->type = NANOEM__EFFECT__DX9BC__SHADER_TYPE__ST_VERTEX;
        message->pixel_shader = new Nanoem__Effect__Dx9bc__Shader;
        parseShader(effect, desc.pPixelShaderFunction, message->pixel_shader);
        message->pixel_shader->type = NANOEM__EFFECT__DX9BC__SHADER_TYPE__ST_PIXEL;
        parseRenderStates(message);
        parseVertexShaderTextures(message);
        parsePixelShaderTextures(message);
    }
    void
    parseShader(ID3DXEffect *effect, const DWORD *shader, Nanoem__Effect__Dx9bc__Shader *message)
    {
        nanoem__effect__dx9bc__shader__init(message);
        UINT numConstants;
        ID3DXConstantTable *constantTable;
        size_t shaderSize = D3DXGetShaderSize(shader);
        message->major_version = D3DXGetShaderVersion(shader);
        ProtobufCBinaryData &assembly = message->assembly;
        assembly.data = new nanoem_u8_t[shaderSize];
        assembly.len = shaderSize;
        memcpy(assembly.data, shader, shaderSize);
        D3DXGetShaderConstantTable(shader, &constantTable);
        D3DXCONSTANTTABLE_DESC tableDesc;
        constantTable->GetDesc(&tableDesc);
        numConstants = tableDesc.Constants;
        size_t numSamplers = 0;
        message->samplers = new Nanoem__Effect__Dx9bc__Sampler *[kMaxSamplerStageCount];
        if (numConstants > 0) {
            message->n_constants = numConstants;
            message->constants = new Nanoem__Effect__Dx9bc__Constant *[numConstants];
            for (UINT i = 0; i < numConstants; i++) {
                D3DXCONSTANT_DESC desc;
                D3DXHANDLE handle = constantTable->GetConstant(nullptr, i);
                UINT count;
                constantTable->GetConstantDesc(handle, &desc, &count);
                Nanoem__Effect__Dx9bc__Constant *constant = new Nanoem__Effect__Dx9bc__Constant;
                nanoem__effect__dx9bc__constant__init(constant);
                message->constants[i] = constant;
                constant->default_value.len = desc.Bytes;
                constant->default_value.data = new nanoem_u8_t[desc.Bytes];
                if (const void *dataPtr = desc.DefaultValue) {
                    memcpy(constant->default_value.data, dataPtr, desc.Bytes);
                }
                else {
                    ZeroMemory(constant->default_value.data, desc.Bytes);
                }
                constant->class_ = static_cast<Nanoem__Effect__Dx9bc__ParameterClass>(desc.Class);
                constant->num_columns = desc.Columns;
                constant->num_elements = desc.Elements;
                constant->name = cloneString(desc.Name);
                constant->register_count = desc.RegisterCount;
                constant->register_index = desc.RegisterIndex;
                constant->register_set = static_cast<Nanoem__Effect__Dx9bc__RegisterSet>(desc.RegisterSet);
                constant->num_rows = desc.Rows;
                constant->struct_members = desc.StructMembers;
                constant->type = static_cast<Nanoem__Effect__Dx9bc__ParameterType>(desc.Type);
                if (constant->class_ == NANOEM__EFFECT__DX9BC__PARAMETER_CLASS__PC_OBJECT &&
                    constant->type >= NANOEM__EFFECT__DX9BC__PARAMETER_TYPE__PT_SAMPLER &&
                    constant->type <= NANOEM__EFFECT__DX9BC__PARAMETER_TYPE__PT_SAMPLERCUBE) {
                    Nanoem__Effect__Dx9bc__Sampler *sampler = new Nanoem__Effect__Dx9bc__Sampler;
                    nanoem__effect__dx9bc__sampler__init(sampler);
                    message->samplers[numSamplers++] = sampler;
                    sampler->name = cloneString(desc.Name);
                    sampler->index = desc.RegisterIndex;
                }
            }
        }
        message->n_samplers = numSamplers;
        constantTable->Release();
    }
    void
    parseRenderStates(Nanoem__Effect__Dx9bc__Pass *message)
    {
        std::vector<std::pair<D3DRENDERSTATETYPE, UINT>> renderStates;
        for (UINT i = 0; i <= D3DRS_BLENDOPALPHA; i++) {
            DWORD value;
            if (SUCCEEDED(m_device->GetRenderState(static_cast<D3DRENDERSTATETYPE>(i), &value)) &&
                value != m_renderStates[i]) {
                renderStates.push_back(std::make_pair(D3DRENDERSTATETYPE(i), UINT(value)));
            }
        }
        if (!renderStates.empty()) {
            size_t size = renderStates.size();
            message->n_render_states = size;
            message->render_states = new Nanoem__Effect__Dx9bc__RenderState *[size];
            for (size_t i = 0, numRenderStates = size; i < numRenderStates; i++) {
                Nanoem__Effect__Dx9bc__RenderState *state = new Nanoem__Effect__Dx9bc__RenderState;
                nanoem__effect__dx9bc__render_state__init(state);
                auto v = renderStates[i];
                state->key = v.first;
                state->value = v.second;
                message->render_states[i] = state;
            }
        }
    }
    void
    parseVertexShaderTextures(Nanoem__Effect__Dx9bc__Pass *message)
    {
        std::vector<TextureData> vertexTextureSamplers;
        for (UINT i = D3DVERTEXTEXTURESAMPLER0; i <= D3DVERTEXTEXTURESAMPLER3; i++) {
            IDirect3DBaseTexture9 *texture = nullptr;
            if (SUCCEEDED(m_device->GetTexture(i, &texture)) && texture) {
                char name[256];
                DWORD size = sizeof(name);
                if (SUCCEEDED(texture->GetPrivateData(kTextureIdentifierGUID, name, &size))) {
                    SamplerStateList states;
                    for (UINT j = D3DSAMP_ADDRESSU; j <= D3DSAMP_DMAPOFFSET; j++) {
                        DWORD value;
                        if (SUCCEEDED(m_device->GetSamplerState(i, static_cast<D3DSAMPLERSTATETYPE>(j), &value)) &&
                            m_samplerStates[j] != value) {
                            states.push_back(std::make_pair(D3DSAMPLERSTATETYPE(j), UINT(value)));
                        }
                    }
                    vertexTextureSamplers.push_back(TextureData(i - D3DVERTEXTEXTURESAMPLER0, name, states));
                }
                texture->Release();
            }
        }
        if (!vertexTextureSamplers.empty()) {
            message->n_vertex_textures = vertexTextureSamplers.size();
            message->vertex_textures = new Nanoem__Effect__Dx9bc__Texture *[message->n_vertex_textures];
            int vertexTextureIndex = 0;
            for (auto it = vertexTextureSamplers.cbegin(), end = vertexTextureSamplers.cend(); it != end; ++it) {
                Nanoem__Effect__Dx9bc__Texture *texture;
                auto item = *it;
                texture = message->vertex_textures[vertexTextureIndex++] = new Nanoem__Effect__Dx9bc__Texture;
                nanoem__effect__dx9bc__texture__init(texture);
                auto states = item.m_states;
                texture->name = cloneString(item.m_name.c_str());
                texture->sampler_index = item.m_index;
                texture->n_sampler_states = states.size();
                texture->sampler_states = new Nanoem__Effect__Dx9bc__SamplerState *[texture->n_sampler_states];
                for (size_t j = 0, numTextures = int(texture->n_sampler_states); j < numTextures; j++) {
                    Nanoem__Effect__Dx9bc__SamplerState *state;
                    auto stateValue = states[j];
                    state = texture->sampler_states[j] = new Nanoem__Effect__Dx9bc__SamplerState;
                    nanoem__effect__dx9bc__sampler_state__init(state);
                    state->key = stateValue.first;
                    state->value = stateValue.second;
                }
            }
        }
    }
    void
    parsePixelShaderTextures(Nanoem__Effect__Dx9bc__Pass *message)
    {
        std::vector<TextureData> textureSamplers;
        for (UINT i = 0; i < kMaxSamplerStageCount; i++) {
            IDirect3DBaseTexture9 *texture = nullptr;
            if (SUCCEEDED(m_device->GetTexture(i, &texture)) && texture) {
                char name[256];
                DWORD size = sizeof(name);
                if (SUCCEEDED(texture->GetPrivateData(kTextureIdentifierGUID, name, &size))) {
                    SamplerStateList states;
                    for (UINT j = D3DSAMP_ADDRESSU; j <= D3DSAMP_DMAPOFFSET; j++) {
                        DWORD value;
                        if (SUCCEEDED(m_device->GetSamplerState(i, static_cast<D3DSAMPLERSTATETYPE>(j), &value)) &&
                            m_samplerStates[j] != value) {
                            states.push_back(std::make_pair(D3DSAMPLERSTATETYPE(j), UINT(value)));
                        }
                    }
                    textureSamplers.push_back(TextureData(i, name, states));
                }
                texture->Release();
            }
        }
        if (!textureSamplers.empty()) {
            message->n_textures = textureSamplers.size();
            message->textures = new Nanoem__Effect__Dx9bc__Texture *[message->n_textures];
            int textureIndex = 0;
            for (auto it = textureSamplers.cbegin(), end = textureSamplers.cend(); it != end; ++it) {
                Nanoem__Effect__Dx9bc__Texture *texture;
                auto item = *it;
                texture = message->textures[textureIndex++] = new Nanoem__Effect__Dx9bc__Texture;
                nanoem__effect__dx9bc__texture__init(texture);
                auto states = item.m_states;
                texture->name = cloneString(item.m_name.c_str());
                texture->sampler_index = item.m_index;
                texture->n_sampler_states = states.size();
                texture->sampler_states = new Nanoem__Effect__Dx9bc__SamplerState *[texture->n_sampler_states];
                for (int j = 0, numTextures = int(texture->n_sampler_states); j < numTextures; j++) {
                    Nanoem__Effect__Dx9bc__SamplerState *state;
                    auto stateValue = states[j];
                    state = texture->sampler_states[j] = new Nanoem__Effect__Dx9bc__SamplerState;
                    nanoem__effect__dx9bc__sampler_state__init(state);
                    state->key = stateValue.first;
                    state->value = stateValue.second;
                }
            }
        }
    }
    void
    destroyShader(Nanoem__Effect__Dx9bc__Shader *message)
    {
        delete[] message->assembly.data;
        for (size_t i = 0, size = message->n_constants; i < size; i++) {
            Nanoem__Effect__Dx9bc__Constant *constant = message->constants[i];
            delete[] constant->default_value.data;
            delete[] constant->name;
            delete message->constants[i];
        }
        delete[] message->constants;
        for (size_t i = 0, size = message->n_samplers; i < size; i++) {
            delete[] message->samplers[i]->name;
            delete message->samplers[i];
        }
        delete[] message->samplers;
        delete message;
    }
    void
    destroyTexture(Nanoem__Effect__Dx9bc__Texture *message)
    {
        delete[] message->name;
        for (size_t i = 0, size = message->n_sampler_states; i < size; i++) {
            delete message->sampler_states[i];
        }
        delete[] message->sampler_states;
        delete message;
    }
    void
    destroyShaderPass(Nanoem__Effect__Dx9bc__Pass *message)
    {
        destroyShader(message->vertex_shader);
        destroyShader(message->pixel_shader);
        for (size_t i = 0, size = message->n_textures; i < size; i++) {
            destroyTexture(message->textures[i]);
        }
        delete[] message->textures;
        for (size_t i = 0, size = message->n_vertex_textures; i < size; i++) {
            destroyTexture(message->vertex_textures[i]);
        }
        delete[] message->vertex_textures;
        for (size_t i = 0, size = message->n_render_states; i < size; i++) {
            delete message->render_states[i];
        }
        delete[] message->render_states;
        delete message;
    }
    void
    parseAnnotation(ID3DXEffect *effect, D3DXHANDLE handle, UINT index, Nanoem__Effect__Annotation *message)
    {
        D3DXPARAMETER_DESC desc;
        D3DXHANDLE annotation = effect->GetAnnotation(handle, index);
        nanoem__effect__annotation__init(message);
        effect->GetParameterDesc(annotation, &desc);
        message->name = cloneString(desc.Name);
        switch (desc.Type) {
        case D3DXPT_BOOL: {
            if (desc.Class == D3DXPC_SCALAR) {
                BOOL value;
                effect->GetBool(annotation, &value);
                message->value_case = NANOEM__EFFECT__ANNOTATION__VALUE_BVAL;
                message->bval = value;
            }
            break;
        }
        case D3DXPT_INT: {
            if (desc.Class == D3DXPC_VECTOR) {
                INT values[4];
                effect->GetIntArray(annotation, values, ARRAYSIZE(values));
                message->value_case = NANOEM__EFFECT__ANNOTATION__VALUE_IVAL4;
                message->ival4 = new Nanoem__Effect__Vector4i;
                assignVector(message->ival4, values);
            }
            else if (desc.Class == D3DXPC_SCALAR) {
                INT value;
                effect->GetInt(annotation, &value);
                message->value_case = NANOEM__EFFECT__ANNOTATION__VALUE_IVAL;
                message->ival = value;
            }
            break;
        }
        case D3DXPT_FLOAT: {
            if (desc.Class == D3DXPC_VECTOR) {
                FLOAT values[4];
                effect->GetFloatArray(annotation, values, ARRAYSIZE(values));
                message->value_case = NANOEM__EFFECT__ANNOTATION__VALUE_FVAL4;
                message->fval4 = new Nanoem__Effect__Vector4f;
                assignVector(message->fval4, values);
            }
            else if (desc.Class == D3DXPC_SCALAR) {
                FLOAT value;
                effect->GetFloat(annotation, &value);
                message->value_case = NANOEM__EFFECT__ANNOTATION__VALUE_FVAL;
                message->fval = value;
            }
            break;
        }
        case D3DXPT_STRING: {
            LPCSTR value;
            effect->GetString(annotation, &value);
            message->value_case = NANOEM__EFFECT__ANNOTATION__VALUE_SVAL;
            message->sval = cloneString(value);
            break;
        }
        default:
            break;
        }
    }
    void
    destroyAnnotation(Nanoem__Effect__Annotation *message)
    {
        delete[] message->name;
        switch (message->value_case) {
        case NANOEM__EFFECT__ANNOTATION__VALUE_SVAL: {
            delete[] message->sval;
            break;
        }
        case NANOEM__EFFECT__ANNOTATION__VALUE_FVAL4: {
            delete message->fval4;
            break;
        }
        case NANOEM__EFFECT__ANNOTATION__VALUE_IVAL4: {
            delete message->ival4;
            break;
        }
        default:
            break;
        }
        delete message;
    }
    void
    assignVector(Nanoem__Effect__Vector4i *message, const INT *value)
    {
        nanoem__effect__vector4i__init(message);
        message->x = value[0];
        message->y = value[1];
        message->z = value[2];
        message->w = value[3];
    }
    void
    assignVector(Nanoem__Effect__Vector4f *message, const FLOAT *value)
    {
        nanoem__effect__vector4f__init(message);
        message->x = value[0];
        message->y = value[1];
        message->z = value[2];
        message->w = value[3];
    }
    void
    resetDefaultRenderStates()
    {
        m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        m_device->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
        m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVDESTALPHA);
        m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
        m_device->SetRenderState(D3DRS_ZENABLE, TRUE);
        m_device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    }
    void
    setErrorReason(ID3DXBuffer *error)
    {
        const char *reason = static_cast<const char *>(error->GetBufferPointer());
        m_reason = std::string(reason, error->GetBufferSize());
    }

    HMODULE m_d3d9;
    DWORD m_renderStates[D3DRS_BLENDOPALPHA];
    DWORD m_samplerStates[D3DSAMP_DMAPOFFSET];
    IDirect3D9 *m_globalUIContext;
    IDirect3DDevice9 *m_device;
    ID3DXEffectPool *m_pool;
    std::string m_reason;
    HWND m_hwnd;
    UINT m_flags;
};

namespace {
static _CrtMemState s_state;
}

void APIENTRY
nanoemApplicationPluginEffectCompilerInitialize(void)
{
    _CrtMemCheckpoint(&s_state);
}

nanoem_application_plugin_effect_compiler_t *APIENTRY
nanoemApplicationPluginEffectCompilerCreate()
{
    return new nanoem_application_plugin_effect_compiler_t();
}

int APIENTRY
nanoemApplicationPluginEffectCompilerGetOption(
    nanoem_application_plugin_effect_compiler_t *plugin, nanoem_u32_t key, void *value, nanoem_rsize_t *size)
{
    return nanoem_is_not_null(plugin) ? plugin->getOption(key, value, size) : 0;
}

int APIENTRY
nanoemApplicationPluginEffectCompilerSetOption(
    nanoem_application_plugin_effect_compiler_t *plugin, nanoem_u32_t key, const void *value, nanoem_rsize_t size)
{
    return nanoem_is_not_null(plugin) ? plugin->setOption(key, value, size) : 0;
}

const char *const *APIENTRY
nanoemApplicationPluginEffectCompilerGetAvailableExtensions(
    nanoem_application_plugin_effect_compiler_t * /* plugin */, nanoem_rsize_t *num_extensions)
{
    static const char *kExtensions[] = { "fx", "fxsub" };
    *num_extensions = sizeof(kExtensions) / sizeof(kExtensions[0]);
    return kExtensions;
}

nanoem_u8_t *APIENTRY
nanoemApplicationPluginEffectCompilerCreateBinaryFromFile(
    nanoem_application_plugin_effect_compiler_t *plugin, const char *path, nanoem_rsize_t *size)
{
    return nanoem_is_not_null(plugin) ? plugin->compile(path, size) : NULL;
}

nanoem_u8_t *APIENTRY
nanoemApplicationPluginEffectCompilerCreateBinaryFromMemory(nanoem_application_plugin_effect_compiler_t *plugin,
    const char *source, nanoem_rsize_t length, nanoem_rsize_t *size)
{
    return nanoem_is_not_null(plugin) ? plugin->compile(source, length, size) : NULL;
}

void APIENTRY
nanoemApplicationPluginEffectCompilerAddShaderSource(nanoem_application_plugin_effect_compiler_t *plugin,
    const char *path, const nanoem_u8_t *source, nanoem_rsize_t size)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->addShaderSource(path, source, size);
    }
}

const char *APIENTRY
nanoemApplicationPluginEffectCompilerGetFailureReason(const nanoem_application_plugin_effect_compiler_t *plugin)
{
    return nanoem_is_not_null(plugin) ? plugin->failureReason() : NULL;
}

const char *APIENTRY
nanoemApplicationPluginEffectCompilerGetRecoverySuggestion(const nanoem_application_plugin_effect_compiler_t *plugin)
{
    return nanoem_is_not_null(plugin) ? plugin->recoverySuggestion() : NULL;
}

void APIENTRY
nanoemApplicationPluginEffectCompilerDestroyBinary(
    nanoem_application_plugin_effect_compiler_t *plugin, nanoem_u8_t *data, nanoem_rsize_t /* size */)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->destroy(data);
    }
}

void APIENTRY
nanoemApplicationPluginEffectCompilerDestroy(nanoem_application_plugin_effect_compiler_t *plugin)
{
    delete plugin;
}

void APIENTRY
nanoemApplicationPluginEffectCompilerTerminate(void)
{
    _CrtMemState state, result;
    UNREFERENCED_PARAMETER(state);
    UNREFERENCED_PARAMETER(result);
    _CrtMemCheckpoint(&state);
    if (_CrtMemDifference(&result, &s_state, &state)) {
        _CrtMemDumpStatistics(&result);
    }
}
