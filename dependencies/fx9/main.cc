#include "fx9/Compiler.h"
#include "fx9/Lexer.h"
#include "fx9/Parser.h"

#include <cctype>
#include <memory>
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
// #include <d3dcommon.h>
#include <d3dcompiler.h>
#endif

#include "GLFW/glfw3.h"
#if defined(__APPLE__)
#include "mtlpp/mtlpp.hpp"
#include <dispatch/dispatch.h>
#endif

#include "effect.pb-c.h"
#include "protobuf-c/protobuf-c.h"

using namespace glslang;
using namespace fx9;

namespace {

typedef void (*PFN_glDebugProc)(GLenum, GLenum, GLuint, GLenum, GLsizei, const char *, const void *);

typedef const uint8_t *(*PFN_glGetString)(GLenum);
typedef void (*PFN_glDebugMessageCallback)(PFN_glDebugProc, const void *);
typedef uint32_t (*PFN_glCreateShader)(GLenum);
typedef void (*PFN_glShaderSource)(GLuint, int, const char **, int *);
typedef void (*PFN_glCompileShader)(GLuint);
typedef void (*PFN_glGetShaderiv)(GLuint, GLenum, int *);
typedef void (*PFN_glGetShaderInfoLog)(GLuint, int, int *, char *);
typedef void (*PFN_glDeleteShader)(GLuint);
PFN_glGetString _glGetString = nullptr;
PFN_glDebugMessageCallback _glDebugMessageCallback = nullptr;
PFN_glCreateShader _glCreateShader = nullptr;
PFN_glShaderSource _glShaderSoruce = nullptr;
PFN_glCompileShader _glCompileShader = nullptr;
PFN_glGetShaderiv _glGetShaderiv = nullptr;
PFN_glGetShaderInfoLog _glGetShaderInfoLog = nullptr;
PFN_glDeleteShader _glDeleteShader = nullptr;

typedef uint32_t (*PFN_glCreateProgram)();
typedef void (*PFN_glAttachShader)(GLuint, GLuint);
typedef void (*PFN_glLinkProgram)(GLuint);
typedef void (*PFN_glGetProgramiv)(GLuint, GLenum, int *);
typedef void (*PFN_glGetProgramInfoLog)(GLuint, int, int *, char *);
typedef void (*PFN_glDeleteProgram)(GLuint);
PFN_glCreateProgram _glCreateProgram = nullptr;
PFN_glAttachShader _glAttachShader = nullptr;
PFN_glLinkProgram _glLinkProgram = nullptr;
PFN_glGetProgramiv _glGetProgramiv = nullptr;
PFN_glGetProgramInfoLog _glGetProgramInfoLog = nullptr;
PFN_glDeleteProgram _glDeleteProgram = nullptr;

struct Main {
    Main(EProfile profile, EShMessages messages, FILE *out)
        : m_profile(profile)
        , m_messages(messages)
        , m_version(150)
        , m_compiler(new Compiler(profile, messages))
        , m_out(out)
    {
        m_compiler->setDefineMacro("FX9", "1");
        m_compiler->setDefineMacro("NANOEM", "1");
    }
    ~Main()
    {
        if (m_out != stdout) {
            fclose(m_out);
        }
    }

#if 0
    uint32_t
    createShader(uint32_t type, const Fx9__Effect__Dx9ms__Shader *shader, FILE *out)
    {
        const char *codeBody = shader->code;
        int codeSize = int(strlen(codeBody));
        uint32_t id = 0;
        // fprintf(out, "[SHADER]\n%s\n", codeBody);
        if (codeSize > 0) {
            id = _glCreateShader(type);
            _glShaderSoruce(id, 1, &codeBody, &codeSize);
            _glCompileShader(id);
            int result = 0;
            _glGetShaderiv(id, 0x8B81, &result);
            if (result == 0) {
                int logSize = 0;
                _glGetShaderiv(id, 0x8B84, &logSize);
                std::vector<char> logMessage(logSize);
                _glGetShaderInfoLog(id, logSize, &logSize, logMessage.data());
#if defined(_WIN32)
                std::vector<wchar_t> wideMessage(logSize + 1);
                std::vector<wchar_t> wideCodeBody(codeSize + 1);
                wideMessage[MultiByteToWideChar(CP_UTF8, 0, logMessage.data(), logSize, wideMessage.data(), logSize)] =
                    0;
                wideCodeBody[MultiByteToWideChar(CP_UTF8, 0, codeBody, codeSize, wideCodeBody.data(), codeSize)] = 0;
                // fwprintf(out, L"[SHADER]\n%s\n[body]\n%s\n", wideMessage.data(), wideCodeBody.data());
                fwprintf(out, L"[SHADER]\n%s\n", wideMessage.data());
#else
                // fprintf(out, "[SHADER]\n%s\n[body]\n%s\n", logMessage.data(), codeBody);
                fprintf(out, "[SHADER]\n%s\n", logMessage.data());
#endif
                _glDeleteShader(id);
                id = 0;
            }
        }
        if (m_verbose) {
            fputs(shader->code, stdout);
        }
        return id;
    }
    void
    createShader(const Fx9__Effect__Hlsl__Shader *shader, const char *profile)
    {
#if defined(_WIN32)
#define D3D10_SHADER_DEBUG (1 << 0)
#define D3D10_SHADER_SKIP_VALIDATION (1 << 1)
#define D3D10_SHADER_SKIP_OPTIMIZATION (1 << 2)
#define D3D10_SHADER_PACK_MATRIX_ROW_MAJOR (1 << 3)
#define D3D10_SHADER_PACK_MATRIX_COLUMN_MAJOR (1 << 4)
#define D3D10_SHADER_PARTIAL_PRECISION (1 << 5)
#define D3D10_SHADER_FORCE_VS_SOFTWARE_NO_OPT (1 << 6)
#define D3D10_SHADER_FORCE_PS_SOFTWARE_NO_OPT (1 << 7)
#define D3D10_SHADER_NO_PRESHADER (1 << 8)
#define D3D10_SHADER_AVOID_FLOW_CONTROL (1 << 9)
#define D3D10_SHADER_PREFER_FLOW_CONTROL (1 << 10)
#define D3D10_SHADER_ENABLE_STRICTNESS (1 << 11)
#define D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY (1 << 12)
#define D3D10_SHADER_IEEE_STRICTNESS (1 << 13)
#define D3D10_SHADER_WARNINGS_ARE_ERRORS (1 << 18)
#define D3D10_SHADER_RESOURCES_MAY_ALIAS (1 << 19)
#define D3D10_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES (1 << 20)
#define D3D10_ALL_RESOURCES_BOUND (1 << 21)
// optimization level flags
#define D3D10_SHADER_OPTIMIZATION_LEVEL0 (1 << 14)
#define D3D10_SHADER_OPTIMIZATION_LEVEL1 0
#define D3D10_SHADER_OPTIMIZATION_LEVEL2 ((1 << 14) | (1 << 15))
#define D3D10_SHADER_OPTIMIZATION_LEVEL3 (1 << 15)
// Force root signature flags. (Passed in Flags2)
#define D3D10_SHADER_FLAGS2_FORCE_ROOT_SIGNATURE_LATEST 0
#define D3D10_SHADER_FLAGS2_FORCE_ROOT_SIGNATURE_1_0 (1 << 4)
#define D3D10_SHADER_FLAGS2_FORCE_ROOT_SIGNATURE_1_1 (1 << 5)
        if (0) {
            const char *codeBody = shader->code;
            int codeSize = int(strlen(codeBody));
            std::vector<wchar_t> wideCodeBody(codeSize + 1);
            wideCodeBody[MultiByteToWideChar(CP_UTF8, 0, codeBody, codeSize, wideCodeBody.data(), codeSize)] = 0;
            fwprintf(m_out, L"%s\n", wideCodeBody.data());
        }
        if (HMODULE d3dc = LoadLibraryA("D3DCompiler_47.dll")) {
            typedef HRESULT (*pfn_D3DCompile)(LPCVOID pSrcData, SIZE_T SrcDataSize, LPCSTR pSourceName,
                CONST D3D_SHADER_MACRO * pDefines, LPD3DINCLUDE pInclude, LPCSTR pEntrypoint, LPCSTR pTarget,
                UINT Flags1, UINT Flags2, LPD3DBLOB * ppCode, LPD3DBLOB * ppErrorMsgs);
            if (pfn_D3DCompile D3DCompile = reinterpret_cast<pfn_D3DCompile>(GetProcAddress(d3dc, "D3DCompile"))) {
                UINT flags = 0 | D3D10_SHADER_DEBUG;
                ID3DBlob *assembly, *error;
                HRESULT result = D3DCompile(shader->code, strlen(shader->code), "", nullptr, nullptr, "main", profile,
                    flags, 0, &assembly, &error);
                if (SUCCEEDED(result)) {
                    /*
                    FILE *fp = fopen("output.bin", "wb");
                    fwrite(assembly->GetBufferPointer(), assembly->GetBufferSize(), 1, fp);
                    fclose(fp);
                    */
                    int length = int(strlen(shader->code));
                    std::vector<wchar_t> wideMessage(length + 1);
                    wideMessage[MultiByteToWideChar(CP_UTF8, 0, shader->code, length, wideMessage.data(), length)] = 0;
                    // fwprintf(m_out, L"source:\n%s\n", wideMessage.data());
                    // fwprintf(m_out, L"buffer: %lld\n", assembly->GetBufferSize());
                    typedef HRESULT (*pfn_D3DStripShader)(
                        LPCVOID pShaderBytecode, SIZE_T BytecodeLength, UINT uStripFlags, LPD3DBLOB * ppStrippedBlob);
                    if (pfn_D3DStripShader D3DStripShader =
                            reinterpret_cast<pfn_D3DStripShader>(GetProcAddress(d3dc, "D3DStripShader"))) {
                        enum D3DCOMPILER_STRIP_FLAGS {
                            D3DCOMPILER_STRIP_REFLECTION_DATA = 1,
                            D3DCOMPILER_STRIP_DEBUG_INFO = 2,
                            D3DCOMPILER_STRIP_TEST_BLOBS = 4,
                            D3DCOMPILER_STRIP_FORCE_DWORD = 0x7fffffff,
                        };
                        ID3DBlob *stripped;
                        flags = 0 | D3DCOMPILER_STRIP_REFLECTION_DATA | D3DCOMPILER_STRIP_DEBUG_INFO |
                            D3DCOMPILER_STRIP_TEST_BLOBS;
                        result =
                            D3DStripShader(assembly->GetBufferPointer(), assembly->GetBufferSize(), flags, &stripped);
                        if (SUCCEEDED(result)) {
                            // fwprintf(m_out, L"stripped: %lld\n", stripped->GetBufferSize());
                        }
                    }
                    typedef HRESULT (*pfn_D3DReflect)(
                        LPCVOID pSrcData, SIZE_T SrcDataSize, REFIID pInterface, void **ppReflector);
                    if (pfn_D3DReflect D3DReflect =
                            reinterpret_cast<pfn_D3DReflect>(GetProcAddress(d3dc, "D3DReflect"))) {
                        const GUID uuid = { 0x8d536ca1, 0x0cca, 0x4956,
                            { 0xa8, 0x37, 0x78, 0x69, 0x63, 0x75, 0x55, 0x84 } };
                        ID3D11ShaderReflection *reflect = NULL;
                        result = D3DReflect(assembly->GetBufferPointer(), assembly->GetBufferSize(), uuid,
                            reinterpret_cast<void **>(&reflect));
                        if (SUCCEEDED(result)) {
                            D3D11_SHADER_DESC desc;
                            reflect->GetDesc(&desc);
                            fprintf(stderr, "Creator: %s 0x%08x\n", desc.Creator, desc.Version);
                            fprintf(stderr, "Num constant buffers: %d\n", desc.ConstantBuffers);
                            fprintf(stderr, "Input:\n");
                            if (strcmp(profile, "vs_5_0") == 0) {
                                for (uint32_t ii = 0; ii < desc.InputParameters; ++ii) {
                                    D3D11_SIGNATURE_PARAMETER_DESC spd;
                                    reflect->GetInputParameterDesc(ii, &spd);
                                    fprintf(stderr, "\t%2d: %s%d, vt %d, ct %d, mask %x, reg %d", ii, spd.SemanticName,
                                        spd.SemanticIndex, spd.SystemValueType, spd.ComponentType, spd.Mask,
                                        spd.Register);
                                }
                            }
                            fprintf(stderr, "Output:\n");
                            for (uint32_t ii = 0; ii < desc.OutputParameters; ++ii) {
                                D3D11_SIGNATURE_PARAMETER_DESC spd;
                                reflect->GetOutputParameterDesc(ii, &spd);
                                fprintf(stderr, "\t%2d: name=%s%d, system=%d, component=%d\n", ii, spd.SemanticName,
                                    spd.SemanticIndex, spd.SystemValueType, spd.ComponentType);
                            }
                            for (uint32_t ii = 0, num = desc.ConstantBuffers; ii < num; ++ii) {
                                ID3D11ShaderReflectionConstantBuffer *cbuffer = reflect->GetConstantBufferByIndex(ii);
                                D3D11_SHADER_BUFFER_DESC bufferDesc;
                                HRESULT hr = cbuffer->GetDesc(&bufferDesc);
                                // size_t _size = (uint16_t)bufferDesc.Size;
                                if (SUCCEEDED(hr)) {
                                    fprintf(stderr, "%s, type=%d, variables=%d, size=%d\n", bufferDesc.Name,
                                        bufferDesc.Type, bufferDesc.Variables, bufferDesc.Size);
                                    for (uint32_t jj = 0; jj < bufferDesc.Variables; ++jj) {
                                        ID3D11ShaderReflectionVariable *var = cbuffer->GetVariableByIndex(jj);
                                        ID3D11ShaderReflectionType *type = var->GetType();
                                        D3D11_SHADER_VARIABLE_DESC varDesc;
                                        hr = var->GetDesc(&varDesc);
                                        if (SUCCEEDED(hr)) {
                                            D3D11_SHADER_TYPE_DESC constDesc;
                                            hr = type->GetDesc(&constDesc);
                                            if (SUCCEEDED(hr) && 0 != (varDesc.uFlags & D3D_SVF_USED)) {
                                                fprintf(stderr, "\t%s, %d, size %d, flags 0x%08x\n", varDesc.Name,
                                                    varDesc.StartOffset, varDesc.Size, varDesc.uFlags);
                                            }
                                        }
                                    }
                                }
                            }
                            fprintf(stderr, "Bound:\n");
                            for (uint32_t ii = 0; ii < desc.BoundResources; ++ii) {
                                D3D11_SHADER_INPUT_BIND_DESC bindDesc;
                                HRESULT hr = reflect->GetResourceBindingDesc(ii, &bindDesc);
                                if (SUCCEEDED(hr) && D3D_SIT_SAMPLER == bindDesc.Type) {
                                    fprintf(stderr, "\t%s, type=%d, point=%d, count=%d\n", bindDesc.Name, bindDesc.Type,
                                        bindDesc.BindPoint, bindDesc.BindCount);
                                }
                            }
                            reflect->Release();
                        }
                    }
#if 0
                    typedef HRESULT (*pfn_D3DGetDebugInfo)(
                      LPCVOID pSrcData,
                      SIZE_T SrcDataSize,
                      LPD3DBLOB *ppDebugInfo
                    );
                    if (pfn_D3DGetDebugInfo D3DGetDebugInfo = reinterpret_cast<pfn_D3DGetDebugInfo>(GetProcAddress(d3dc, "D3DGetDebugInfo"))) {
                        ID3DBlob *debug;
                        result = D3DGetDebugInfo(assembly->GetBufferPointer(), assembly->GetBufferSize(), &debug);
                        if (SUCCEEDED(result)) {
                            size_t length = debug->GetBufferSize();
                            std::vector<wchar_t> wideMessage(length + 1);
                            wideMessage[MultiByteToWideChar(CP_UTF8, 0, static_cast<const char *>(debug->GetBufferPointer()), int(length), wideMessage.data(), int(length))] = 0;
                            fwprintf(m_out, L"debug: %s\n", wideMessage.data());
                        }
                        else {
                            fwprintf(m_out, L"error(debug): 0x%x\n", result);
                        }
                    }
                    typedef HRESULT (*pfn_D3DDisassemble)(
                      LPCVOID pSrcData,
                      SIZE_T SrcDataSize,
                      UINT Flags,
                      LPD3DBLOB *szComments,
                      LPD3DBLOB *ppDisassembly
                    );
                    if (pfn_D3DDisassemble D3DDisassemble = reinterpret_cast<pfn_D3DDisassemble>(GetProcAddress(d3dc, "D3DDisassemble"))) {
                        ID3DBlob *comment;
                        ID3DBlob *disassembly;
                        result = D3DDisassemble(assembly->GetBufferPointer(), assembly->GetBufferSize(), 0, &comment, &disassembly);
                        if (SUCCEEDED(result)) {
                            size_t length = disassembly->GetBufferSize();
                            std::vector<wchar_t> wideMessage(length + 1);
                            wideMessage[MultiByteToWideChar(CP_UTF8, 0, static_cast<const char *>(disassembly->GetBufferPointer()), int(length), wideMessage.data(), int(length))] = 0;
                            fwprintf(m_out, L"disassembly: %s\n", wideMessage.data());
                        }
                        else {
                            fwprintf(m_out, L"error(disassembly): 0x%x\n", result);
                        }
                    }
#endif
                }
                else {
                    size_t length = error->GetBufferSize();
                    std::vector<wchar_t> wideErrorMessage(length + 1);
                    wideErrorMessage[MultiByteToWideChar(CP_UTF8, 0,
                        static_cast<const char *>(error->GetBufferPointer()), int(length), wideErrorMessage.data(),
                        int(length))] = 0;
                    fwprintf(m_out, L"error(0x%x): %s\n", result, wideErrorMessage.data());
                }
            }
            FreeLibrary(d3dc);
        }
#else
        if (m_verbose) {
            fputs(shader->code, stdout);
        }
#endif
    }
    void
    createShader(const Fx9__Effect__Msl__Shader *shader)
    {
#if defined(__APPLE__)
        static mtlpp::Device device = mtlpp::Device::CreateSystemDefaultDevice();
        {
            dispatch_semaphore_t sema = dispatch_semaphore_create(0);
            mtlpp::CompileOptions options;
            options.SetFastMathEnabled(true);
            device.NewLibrary(
                shader->code, options, [this, sema, shader](const mtlpp::Library &library, const ns::Error &error) {
                    if (library) {
                        if (m_verbose) {
                            fprintf(stdout, "SUCCESS: %p:%s\n", library.GetPtr(),
                                error ? error.GetLocalizedDescription().GetCStr() : nullptr);
                        }
                    }
                    else {
                        if (m_verbose) {
                            fprintf(stdout, "ERROR: %s:%s\n", error.GetLocalizedDescription().GetCStr(), shader->code);
                        }
                        else {
                            fprintf(stdout, "ERROR: %s\n", error.GetLocalizedDescription().GetCStr());
                        }
                    }
                    dispatch_semaphore_signal(sema);
                });
            dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
            dispatch_release(sema);
        }
        if (m_verbose) {
            fputs(shader->code, stdout);
        }
#elif defined(_WIN32)
        const char *codeBody = shader->code;
        int codeSize = int(strlen(codeBody));
        std::vector<wchar_t> wideCodeBody(codeSize + 1);
        wideCodeBody[MultiByteToWideChar(CP_UTF8, 0, codeBody, codeSize, wideCodeBody.data(), codeSize)] = 0;
        fwprintf(m_out, L"[SHADER]\n%s\n", wideCodeBody.data());
#endif
    }
    void
    createEffectPass(const Fx9__Effect__Dx9ms__Pass *impl)
    {
        uint32_t vsh = createShader(0x8B31, impl->vertex_shader, m_out);
        uint32_t fsh = createShader(0x8B30, impl->pixel_shader, m_out);
        if (vsh != 0 && fsh != 0) {
            uint32_t program = _glCreateProgram();
            _glAttachShader(program, vsh);
            _glAttachShader(program, fsh);
            _glLinkProgram(program);
            int result = 0;
            _glGetProgramiv(program, 0x8B82, &result);
            if (result == 0) {
                int logSize = 0;
                _glGetProgramiv(program, 0x8B84, &logSize);
                std::vector<char> logMessage(logSize);
                _glGetProgramInfoLog(program, logSize, &logSize, logMessage.data());
#if defined(_WIN32)
                std::vector<wchar_t> wideMessage(logSize);
                wideMessage[MultiByteToWideChar(CP_UTF8, 0, logMessage.data(), logSize, wideMessage.data(), logSize)] =
                    fwprintf(m_out, L"[PROGRAM]\n%s\n", wideMessage.data());
#else
                fprintf(m_out, "[PROGRAM]\n%s\n", logMessage.data());
                fprintf(m_out, "[VSH]\n%s\n", impl->vertex_shader->code);
                fprintf(m_out, "[FSH]\n%s\n", impl->pixel_shader->code);
#endif
            }
            _glDeleteProgram(program);
        }
        _glDeleteShader(vsh);
        _glDeleteShader(fsh);
    }
    void
    createEffectPass(const Fx9__Effect__Hlsl__Pass *impl)
    {
        createShader(impl->vertex_shader, "vs_5_0");
        createShader(impl->pixel_shader, "ps_5_0");
    }
    void
    createEffectPass(const Fx9__Effect__Msl__Pass *impl)
    {
        createShader(impl->vertex_shader);
        createShader(impl->pixel_shader);
    }
    void
    createEffectPass(const Fx9__Effect__Spirv__Pass * /* impl */)
    {
    }
#endif
    void
    createEffectShader(
        const Fx9__Effect__Shader *shader, const Fx9__Effect__Pass *pass, const Fx9__Effect__Technique *technique)
    {
        switch (shader->body_case) {
        case FX9__EFFECT__SHADER__BODY_GLSL: {
            if (m_verbose) {
                log("// %s/%s\n%s\n", technique->name, pass->name, shader->glsl);
            }
            break;
        }
        case FX9__EFFECT__SHADER__BODY_HLSL: {
            if (m_verbose) {
                log("// %s/%s\n%s\n", technique->name, pass->name, shader->hlsl);
            }
            break;
        }
        case FX9__EFFECT__SHADER__BODY_MSL: {
            if (m_verbose) {
                log("// %s/%s\n%s\n", technique->name, pass->name, shader->msl);
            }
            break;
        }
        case FX9__EFFECT__SHADER__BODY_SPIRV:
            break;
        case FX9__EFFECT__SHADER__BODY__NOT_SET:
        case _FX9__EFFECT__SHADER__BODY_IS_INT_SIZE:
        default:
            break;
        }
    }
    void
    compileEffectTechnique(const Fx9__Effect__Technique *technique)
    {
        for (size_t i = 0, numPasses = technique->n_passes; i < numPasses; i++) {
            const auto *pass = technique->passes[i];
            createEffectShader(pass->vertex_shader, pass, technique);
            createEffectShader(pass->pixel_shader, pass, technique);
        }
    }
    void
    compileEffectProduct(const Compiler::EffectProduct &effectProduct)
    {
        const auto &message = effectProduct.message;
        auto *effect = fx9__effect__effect__unpack(nullptr, message.size(), message.data());
        for (size_t i = 0, numTechniques = effect->n_techniques; i < numTechniques; i++) {
            compileEffectTechnique(effect->techniques[i]);
        }
        for (size_t i = 0, numParameters = effect->n_parameters; i < numParameters; i++) {
            const auto *parameter = effect->parameters[i];
            std::string semantic(parameter->semantic);
            std::transform(semantic.begin(), semantic.end(), semantic.begin(), [](char c) { return std::toupper(c); });
            if (!semantic.empty()) {
                auto &it = m_semanticAnnotationMap[semantic][parameter->name];
                for (size_t j = 0, numAnnotations = parameter->n_annotations; j < numAnnotations; j++) {
                    const auto *annotation = parameter->annotations[j];
                    if (annotation->value_case == FX9__EFFECT__ANNOTATION__VALUE_SVAL) {
                        it[annotation->name] = annotation->sval;
                    }
                }
            }
        }
        fx9__effect__effect__free_unpacked(effect, nullptr);
    }
    void
    buildEffectProduct(const char *path)
    {
        Compiler::EffectProduct effect;
        bool performCompile = false;
#if defined(_WIN32)
        auto converter = [](const std::string &from, std::vector<wchar_t> &to) {
            if (!from.empty()) {
                to.resize(from.size() + 1);
                to[MultiByteToWideChar(CP_UTF8, 0, from.c_str(), int(from.size()), to.data(), int(from.size()))] = 0;
            }
        };
        std::string newPath(path);
        std::replace(newPath.begin(), newPath.end(), '\\', '/');
        wchar_t widePath[MAX_PATH];
        widePath[MultiByteToWideChar(CP_UTF8, 0, newPath.c_str(), int(newPath.size()), widePath, ARRAYSIZE(widePath))] =
            0;
        if (m_compiler->compile(newPath.c_str(), effect)) {
            performCompile = effect.hasAllPassCompiled();
            const wchar_t *icon = 0;
            if (performCompile) {
                icon = L"\u2705";
            }
            else if (effect.isEmpty()) {
                icon = L"\u2753";
            }
            else {
                icon = L"\u26a0";
            }
            fwprintf(m_out, L"%s\t%s (%zu:%zu)\n", icon, widePath, effect.numPasses, effect.numCompiledPasses);
            if (!performCompile && !effect.isEmpty()) {
                std::vector<wchar_t> infoMessage, translatorMessage, optimizerMessage, validatorMessage;
                converter(effect.sink.info, infoMessage);
                converter(effect.sink.translator, translatorMessage);
                converter(effect.sink.optimizer, optimizerMessage);
                converter(effect.sink.validator, validatorMessage);
                fwprintf(m_out, L"[INFO]\n%s\n[TRANSLATOR]\n%s\n[OPTIMIZER]\n%s\n[VALIDATOR]\n%s\n", infoMessage.data(),
                    translatorMessage.data(), optimizerMessage.data(), validatorMessage.data());
            }
        }
        else if (effect.hasAllPassCompiled()) {
            std::vector<wchar_t> infoMessage, translatorMessage, optimizerMessage, validatorMessage;
            converter(effect.sink.info, infoMessage);
            converter(effect.sink.translator, translatorMessage);
            converter(effect.sink.optimizer, optimizerMessage);
            converter(effect.sink.validator, validatorMessage);
            fwprintf(m_out, L"\u2757\t%s (%zu:%zu)\n[INFO]\n%s\n[TRANSLATOR]\n%s\n[OPTIMIZER]\n%s\n[VALIDATOR]\n%s\n",
                widePath, effect.numPasses, effect.numCompiledPasses, infoMessage.data(), translatorMessage.data(),
                optimizerMessage.data(), validatorMessage.data());
        }
        else if (!effect.sink.isEmpty()) {
            std::vector<wchar_t> infoMessage, translatorMessage, optimizerMessage, validatorMessage;
            converter(effect.sink.info, infoMessage);
            converter(effect.sink.translator, translatorMessage);
            converter(effect.sink.optimizer, optimizerMessage);
            converter(effect.sink.validator, validatorMessage);
            fwprintf(m_out, L"\u274c\t%s (%zu:%zu)\n[INFO]\n%s\n[TRANSLATOR]\n%s\n[OPTIMIZER]\n%s\n[VALIDATOR]\n%s\n",
                widePath, effect.numPasses, effect.numCompiledPasses, infoMessage.data(), translatorMessage.data(),
                optimizerMessage.data(), validatorMessage.data());
        }
#else
        if (m_compiler->compile(path, effect)) {
            performCompile = effect.hasAllPassCompiled();
            const char *icon = 0;
            if (performCompile) {
                icon = "\u2705";
            }
            else if (effect.isEmpty() && effect.sink.isEmpty()) {
                icon = "\u2753";
            }
            else {
                icon = "\u26a0";
            }
            fprintf(m_out, "%s\t%s (%zu:%zu)\n", icon, path, effect.numPasses, effect.numCompiledPasses);
            if (!effect.sink.isEmpty()) {
                fprintf(m_out, "[INFO]\n%s\n[TRANSLATOR]\n%s\n[OPTIMIZER]\n%s\n[VALIDATOR]\n%s\n",
                    effect.sink.info.c_str(), effect.sink.translator.c_str(), effect.sink.optimizer.c_str(),
                    effect.sink.validator.c_str());
            }
        }
        else if (effect.hasAnyCompiledPass()) {
            fprintf(m_out, "\u2757\t%s (%zu:%zu)\n[INFO]\n%s\n[TRANSLATOR]\n%s\n[OPTIMIZER]\n%s\n[VALIDATOR]\n%s\n",
                path, effect.numPasses, effect.numCompiledPasses, effect.sink.info.c_str(),
                effect.sink.translator.c_str(), effect.sink.optimizer.c_str(), effect.sink.validator.c_str());
        }
        else if (!effect.sink.isEmpty()) {
            fprintf(m_out, "\u274c\t%s (%zu:%zu)\n[INFO]\n%s\n[TRANSLATOR]\n%s\n[OPTIMIZER]\n%s\n[VALIDATOR]\n%s\n",
                path, effect.numPasses, effect.numCompiledPasses, effect.sink.info.c_str(),
                effect.sink.translator.c_str(), effect.sink.optimizer.c_str(), effect.sink.validator.c_str());
        }
        fflush(m_out);
#endif
        if (performCompile) {
            compileEffectProduct(effect);
            if (auto e = fx9__effect__effect__unpack(nullptr, effect.message.size(), effect.message.data())) {
                auto parse = [this, path](const char *ptr, const char *p) {
                    auto trim = [](const char *p, const char *q) {
                        while (isspace(*p)) {
                            p++;
                        }
                        const char *ptr = q - 1;
                        while (ptr >= p) {
                            if (isspace(*ptr)) {
                                q--;
                            }
                            else {
                                break;
                            }
                            ptr--;
                        }
                        return std::string(p, q);
                    };
                    if (const char *q = strchr(ptr, '=')) {
                        std::string key(trim(ptr, q)), value(trim(q + 1, p));
                        if (value != "hide" && value != "none") {
                            std::string s(path);
                            const char *l = strrchr(s.c_str(), '/');
                            if (l != nullptr) {
                                std::string path(s.c_str(), l);
                                path.push_back('/');
                                path.append(value.c_str());
                                buildEffectProduct(path.c_str());
                            }
                        }
                    }
                };
                for (size_t i = 0, numParameters = e->n_parameters; i < numParameters; i++) {
                    auto parameter = e->parameters[i];
                    if (strcmp(parameter->semantic, "OFFSCREENRENDERTARGET") == 0) {
                        for (size_t j = 0, numAnnotations = parameter->n_annotations; j < numAnnotations; j++) {
                            auto annotation = parameter->annotations[j];
                            if (strcmp(annotation->name, "DefaultEffect") == 0) {
                                const char *ptr = annotation->sval;
                                while (const char *p = strchr(ptr, ';')) {
                                    parse(ptr, p);
                                    ptr = p + 1;
                                }
                                parse(ptr, annotation->sval + strlen(annotation->sval));
                            }
                        }
                    }
                }
                fx9__effect__effect__free_unpacked(e, nullptr);
            }
            if (false) {
                std::string effectPath(path);
                effectPath.append(".bin");
                if (FILE *fp = fopen(effectPath.c_str(), "wb")) {
                    fwrite(effect.message.data(), effect.message.size(), 1, fp);
                    fclose(fp);
                }
            }
        }
    }
    void
    dump()
    {
#if 0
        for (const auto &it : m_semanticAnnotationMap) {
            fprintf(stderr, "%s\n", it.first.c_str());
            for (const auto &it2 : it.second) {
                fprintf(stderr, "  %s\n", it2.first.c_str());
                for (const auto &it3 : it2.second) {
                    fprintf(stderr, "    %s:%s\n", it3.first.c_str(), it3.second.c_str());
                }
            }
        }
#elif 1
        std::map<std::string, int> map;
        for (const auto &it : m_semanticAnnotationMap) {
            map.insert(std::make_pair(it.first, it.second.size()));
        }
        for (const auto &it : map) {
            fprintf(stderr, "%s:%d\n", it.first.c_str(), it.second);
        }
#endif
    }
    void
    log(const char *format, ...)
    {
        va_list ap;
        va_start(ap, format);
#if defined(_WIN32)
        int size = vsnprintf(nullptr, 0, format, ap);
        char *bytes = new char[size + 1];
        vsnprintf(bytes, size + 1, format, ap);
        bytes[size] = 0;
        int size2 = MultiByteToWideChar(CP_UTF8, 0, bytes, size, nullptr, 0);
        wchar_t *buffer = new wchar_t[size2 + 1];
        MultiByteToWideChar(CP_UTF8, 0, bytes, size, buffer, size2);
        buffer[size2] = 0;
        fputws(buffer, m_out);
        delete[] bytes;
        delete[] buffer;
#else
        vfprintf(m_out, format, ap);
#endif
        va_end(ap);
    }

    const EProfile m_profile;
    const EShMessages m_messages;
    const int m_version;
    std::unique_ptr<Compiler> m_compiler;
    std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, std::string>>>
        m_semanticAnnotationMap;
    FILE *m_out = nullptr;
    bool m_verbose = false;
};

static void
debugCallback(
    GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, const void *userParam)
{
    char buffer[4096];
    snprintf(buffer, sizeof(buffer), "source=0x%x type=0x%x id=%d severity=0x%x message=%s", source, type, id, severity,
        message);
    if (severity != 0x9146 && type != 0x8251) {
#if defined(_WIN32)
        OutputDebugStringA(buffer);
        OutputDebugStringA("\n");
#else
        fprintf(stderr, "%s\n", buffer);
#endif
    }
}
}

int
main(int argc, char **argv)
{
    glfwInit();
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(1, 1, __FILE__, nullptr, nullptr);
    glfwMakeContextCurrent(window);
    _glGetString = reinterpret_cast<PFN_glGetString>(glfwGetProcAddress("glGetString"));
    _glDebugMessageCallback =
        reinterpret_cast<PFN_glDebugMessageCallback>(glfwGetProcAddress("glDebugMessageCallback"));
    _glCreateShader = reinterpret_cast<PFN_glCreateShader>(glfwGetProcAddress("glCreateShader"));
    _glShaderSoruce = reinterpret_cast<PFN_glShaderSource>(glfwGetProcAddress("glShaderSource"));
    _glCompileShader = reinterpret_cast<PFN_glCompileShader>(glfwGetProcAddress("glCompileShader"));
    _glGetShaderiv = reinterpret_cast<PFN_glGetShaderiv>(glfwGetProcAddress("glGetShaderiv"));
    _glGetShaderInfoLog = reinterpret_cast<PFN_glGetShaderInfoLog>(glfwGetProcAddress("glGetShaderInfoLog"));
    _glDeleteShader = reinterpret_cast<PFN_glDeleteShader>(glfwGetProcAddress("glDeleteShader"));
    _glCreateProgram = reinterpret_cast<PFN_glCreateProgram>(glfwGetProcAddress("glCreateProgram"));
    _glAttachShader = reinterpret_cast<PFN_glAttachShader>(glfwGetProcAddress("glAttachShader"));
    _glLinkProgram = reinterpret_cast<PFN_glLinkProgram>(glfwGetProcAddress("glLinkProgram"));
    _glGetProgramiv = reinterpret_cast<PFN_glGetProgramiv>(glfwGetProcAddress("glGetProgramiv"));
    _glGetProgramInfoLog = reinterpret_cast<PFN_glGetProgramInfoLog>(glfwGetProcAddress("glGetProgramInfoLog"));
    _glDeleteProgram = reinterpret_cast<PFN_glDeleteProgram>(glfwGetProcAddress("glDeleteProgram"));
    fprintf(stderr, "GL_VENDOR:   %s\n", _glGetString(0x1F00));
    fprintf(stderr, "GL_RENDERER: %s\n", _glGetString(0x1F01));
    fprintf(stderr, "GL_VERSION:  %s\n", _glGetString(0x1F02));
    if (_glDebugMessageCallback) {
        _glDebugMessageCallback(debugCallback, nullptr);
    }
    Compiler::initialize();
    {
        FILE *out = 0;
#if defined(_WIN32)
        out = _wfopen(L"output.txt", L"wb");
        fputc(0xff, out);
        fputc(0xfe, out);
#else
        out = stdout;
#endif
        Main main(ECoreProfile, EShMsgDefault, out);
        bool executed = false;
        if (argc > 1) {
            for (int i = 1; i < argc; i++) {
                const char *arg = argv[i];
                if (strcmp(arg, "--optimization") == 0 || strcmp(arg, "--optimize") == 0) {
                    main.m_compiler->setOptimizeEnabled(true);
                    fprintf(stderr, "Optimization Enabled\n");
                }
                else if (strcmp(arg, "--validation") == 0 || strcmp(arg, "--validate") == 0) {
                    main.m_compiler->setValidationEnabled(true);
                    fprintf(stderr, "Validation Enabled\n");
                }
                else if (strcmp(arg, "--essl") == 0) {
                    main.m_compiler->setTargetLanguage(Compiler::kLanguageTypeESSL);
                }
                else if (strcmp(arg, "--glsl") == 0) {
                    main.m_compiler->setTargetLanguage(Compiler::kLanguageTypeGLSL);
                }
                else if (strcmp(arg, "--hlsl") == 0) {
                    main.m_compiler->setTargetLanguage(Compiler::kLanguageTypeHLSL);
                }
                else if (strcmp(arg, "--msl") == 0) {
                    main.m_compiler->setTargetLanguage(Compiler::kLanguageTypeMSL);
                }
                else if (strcmp(arg, "--spirv") == 0) {
                    main.m_compiler->setTargetLanguage(Compiler::kLanguageTypeSPIRV);
                }
                else if (strcmp(arg, "--verbose") == 0) {
                    main.m_verbose = true;
                }
                else if (const char *p = strrchr(arg, '.')) {
                    if (strcmp(p, ".txt") == 0) {
                        char buffer[1024];
                        if (FILE *fp = fopen(arg, "r")) {
                            while (fgets(buffer, sizeof(buffer), fp)) {
                                if (*buffer != '#') {
                                    buffer[strlen(buffer) - 1] = 0;
                                    main.buildEffectProduct(buffer);
                                }
                            }
                            fclose(fp);
                            executed = true;
                        }
                    }
                    else if (strcmp(p, ".fx") == 0 || strcmp(p, ".fxsub") == 0) {
                        main.buildEffectProduct(arg);
                        executed = true;
                    }
                }
            }
        }
        if (!executed) {
            main.buildEffectProduct("test.fx");
        }
    }
    Compiler::terminate();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
