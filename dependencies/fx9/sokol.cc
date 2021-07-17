#include "fx9/Compiler.h"
#include "fx9/Lexer.h"
#include "fx9/Parser.h"

#include <memory>
#include <stdio.h>

#include "sokol_app.h"
#include "sokol_gfx.h"

#include "effect.pb-c.h"
#include "protobuf-c/protobuf-c.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#if defined(SOKOL_D3D11)
#pragma comment(lib, "d3dcompiler.lib")
#include <d3d11.h>
#include <d3dcompiler.h>
#endif

using namespace glslang;
using namespace fx9;

namespace {

struct Main {
    template <typename T>
    static inline void
    zerofill(T &data)
    {
        memset(&data, 0, sizeof(data));
    }

    Main(EProfile profile, EShMessages messages, FILE *out)
        : m_profile(profile)
        , m_messages(messages)
        , m_version(150)
        , m_compiler(new Compiler(profile, messages))
        , m_out(out)
        , m_verbose(false)
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

    bool
    createAllShaderImages(const Fx9__Effect__Shader *shaderPtr, sg_shader_image_desc *images)
    {
        const size_t numSamplers = shaderPtr->n_samplers;
        bool succeeded = numSamplers <= SG_MAX_SHADERSTAGE_IMAGES;
        if (succeeded) {
            bool found = false;
            for (int i = SG_MAX_SHADERSTAGE_IMAGES - 1; i >= 0; i--) {
                const sg_image_type type = images[i].image_type;
                if (!found && type != _SG_IMAGETYPE_DEFAULT) {
                    found = true;
                }
                else if (found && type == _SG_IMAGETYPE_DEFAULT) {
                    static char buffer[16];
                    snprintf(buffer, sizeof(buffer), "s%d", i);
                    images[i] = sg_shader_image_desc { buffer, SG_IMAGETYPE_2D };
                    // fprintf(stderr, "%d:(%s)\n", i, buffer);
                }
            }
        }
        return succeeded;
    }
    void
    setShader(const Fx9__Effect__Shader *shader, sg_shader_stage_desc &desc)
    {
        switch (shader->body_case) {
        case FX9__EFFECT__SHADER__BODY_GLSL:
            desc.source = shader->glsl;
            break;
        case FX9__EFFECT__SHADER__BODY_HLSL:
            desc.source = shader->hlsl;
            break;
        case FX9__EFFECT__SHADER__BODY_MSL:
            desc.source = shader->msl;
            break;
        case FX9__EFFECT__SHADER__BODY_SPIRV:
            desc.bytecode.ptr = shader->spirv.data;
            desc.bytecode.size = shader->spirv.len;
            break;
        default:
            break;
        }
        if (desc.source && m_verbose) {
            log("%s\n", desc.source);
        }
    }
    void
    compileEffectProduct(const Compiler::EffectProduct &effectProduct)
    {
        const auto &message = effectProduct.message;
        auto *effect = fx9__effect__effect__unpack(nullptr, message.size(), message.data());
        for (size_t i = 0, numTechniques = effect->n_techniques; i < numTechniques; i++) {
            const auto *technique = effect->techniques[i];
            for (size_t j = 0, numPasses = technique->n_passes; j < numPasses; j++) {
                const auto *pass = technique->passes[j];
                sg_shader_desc sd;
                zerofill(sd);
                sd.fs.entry = sd.vs.entry = "fx9_metal_main";
                if (createAllShaderImages(pass->pixel_shader, sd.fs.images) &&
                    createAllShaderImages(pass->vertex_shader, sd.vs.images)) {
                    setShader(pass->pixel_shader, sd.fs);
                    setShader(pass->vertex_shader, sd.vs);
                    createPass(pass->name, sd);
                }
                else {
                    log("Cannot compile pass \"%s\" due to exceed of image slots (%d:%d)\n", pass->name,
                        pass->vertex_shader->n_samplers, pass->pixel_shader->n_samplers);
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
        std::string newPath(path), translator, optimizer;
        std::replace(newPath.begin(), newPath.end(), '\\', '/');
        auto takeAllErrorMessageLogs = [&translator, &optimizer, &effect]() {
            for (const auto &item : effect.sink.translator) {
                translator.append(item);
                translator.append("\n");
            }
            for (const auto &item : effect.sink.optimizer) {
                optimizer.append(item);
                optimizer.append("\n");
            }
        };
        if (m_compiler->compile(newPath.c_str(), effect)) {
            performCompile = effect.hasAllPassCompiled();
            const char *icon = 0;
            if (performCompile) {
                static const uint8_t kWhiteCheckMarkIcon[] = { 0xE2, 0x9C, 0x85, 0x0 };
                icon = reinterpret_cast<const char *>(kWhiteCheckMarkIcon); // \u2705
            }
            else if (effect.isEmpty()) {
                static const uint8_t kQuestionIcon[] = { 0xE2, 0x9D, 0x93, 0x0 };
                icon = reinterpret_cast<const char *>(kQuestionIcon); // \u2753
            }
            else {
                static const uint8_t kWarningIcon[] = { 0xE2, 0x9A, 0xA0, 0x0 };
                icon = reinterpret_cast<const char *>(kWarningIcon); // \u26a0
            }
            log("%s\t%s (%zu:%zu)\n", icon, path, effect.numPasses, effect.numCompiledPasses);
            if (!performCompile && !effect.isEmpty()) {
                takeAllErrorMessageLogs();
                log("[INFO]\n%s\n[TRANSLATOR]\n%s\n[OPTIMIZER]\n%s\n[VALIDATOR]\n%s\n", effect.sink.info.c_str(),
                    translator.c_str(), optimizer.c_str(), effect.sink.validator.c_str());
            }
        }
        else if (effect.hasAnyCompiledPass()) {
            static const uint8_t kExclamationIcon[] = { 0xE2, 0x9D, 0x97, 0x0 }; // \u2757
            takeAllErrorMessageLogs();
            log("%s\t%s (%zu:%zu)\n[INFO]\n%s\n[TRANSLATOR]\n%s\n[OPTIMIZER]\n%s\n[VALIDATOR]\n%s\n", kExclamationIcon,
                path, effect.numPasses, effect.numCompiledPasses, effect.sink.info.c_str(),
                translator.c_str(), optimizer.c_str(), effect.sink.validator.c_str());
        }
        else if (!effect.sink.isEmpty()) {
            static const uint8_t kCrossMarkIcon[] = { 0xE2, 0x9D, 0x8C, 0x0 }; // \u274c
            takeAllErrorMessageLogs();
            log("%s\t%s (%zu:%zu)\n[INFO]\n%s\n[TRANSLATOR]\n%s\n[OPTIMIZER]\n%s\n[VALIDATOR]\n%s\n", kCrossMarkIcon,
                path, effect.numPasses, effect.numCompiledPasses, effect.sink.info.c_str(),
                translator.c_str(), optimizer.c_str(), effect.sink.validator.c_str());
        }
        fflush(m_out);
        if (performCompile) {
            compileEffectProduct(effect);
            if (auto e = fx9__effect__effect__unpack(nullptr, effect.message.size(), effect.message.data())) {
                auto parse = [this, &newPath](const char *ptr, const char *p) {
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
                            const char *l = strrchr(newPath.c_str(), '/');
                            std::string path(newPath.c_str(), l);
                            path.push_back('/');
                            path.append(value.c_str());
                            buildEffectProduct(path.c_str());
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
    createPass(const char *name, sg_shader_desc &sd)
    {
        sd.attrs[0] = sg_shader_attr_desc { "a_position", "SV_POSITION", 0 };
        sd.attrs[1] = sg_shader_attr_desc { "a_normal", "NORMAL", 0 };
        sd.attrs[2] = sg_shader_attr_desc { "a_texcoord0", "TEXCOORD", 0 };
        sd.attrs[3] = sg_shader_attr_desc { "a_texcoord1", "TEXCOORD", 1 };
        sd.attrs[4] = sg_shader_attr_desc { "a_texcoord2", "TEXCOORD", 2 };
        sd.attrs[5] = sg_shader_attr_desc { "a_texcoord3", "TEXCOORD", 3 };
        sd.attrs[6] = sg_shader_attr_desc { "a_texcoord4", "TEXCOORD", 4 };
        sd.attrs[7] = sg_shader_attr_desc { "a_color", "COLOR", 0 };
        sg_shader sh = sg_make_shader(&sd);
        if (sh.id != SG_INVALID_ID) {
            if (sg_query_shader_state(sh) == SG_RESOURCESTATE_VALID) {
                sg_pipeline_desc pd;
                zerofill(pd);
                pd.shader = sh;
                sg_layout_desc &ld = pd.layout;
                ld.buffers[0].stride = 128;
                ld.attrs[0] = sg_vertex_attr_desc { 0, 0, SG_VERTEXFORMAT_FLOAT4 };
                ld.attrs[1] = sg_vertex_attr_desc { 0, 16, SG_VERTEXFORMAT_FLOAT4 };
                ld.attrs[2] = sg_vertex_attr_desc { 0, 32, SG_VERTEXFORMAT_FLOAT4 };
                ld.attrs[3] = sg_vertex_attr_desc { 0, 48, SG_VERTEXFORMAT_FLOAT4 };
                ld.attrs[4] = sg_vertex_attr_desc { 0, 64, SG_VERTEXFORMAT_FLOAT4 };
                ld.attrs[5] = sg_vertex_attr_desc { 0, 80, SG_VERTEXFORMAT_FLOAT4 };
                ld.attrs[6] = sg_vertex_attr_desc { 0, 96, SG_VERTEXFORMAT_FLOAT4 };
                ld.attrs[7] = sg_vertex_attr_desc { 0, 112, SG_VERTEXFORMAT_FLOAT4 };
                sg_pipeline ph = sg_make_pipeline(&pd);
                if (ph.id != SG_INVALID_ID) {
                    if (sg_query_pipeline_state(ph) == SG_RESOURCESTATE_VALID) {
                        sg_bindings bindings;
                        zerofill(bindings);
                        sg_buffer_desc vbd;
                        zerofill(vbd);
                        vbd.size = ld.buffers[0].stride;
                        vbd.usage = SG_USAGE_DYNAMIC;
                        sg_buffer vbh = sg_make_buffer(&vbd);
                        bindings.vertex_buffers[0] = vbh;
                        sg_image_desc id;
                        zerofill(id);
                        id.width = 1;
                        id.height = 1;
                        id.pixel_format = SG_PIXELFORMAT_RGBA8;
                        uint32_t pixel = 0;
                        id.data.subimage[0][0].ptr = &pixel;
                        id.data.subimage[0][0].size = sizeof(pixel);
                        sg_image ih = sg_make_image(&id);
                        for (int i = 0; i < SG_MAX_SHADERSTAGE_IMAGES; i++) {
                            if (sd.fs.images[i].name) {
                                bindings.fs_images[i] = ih;
                            }
                            if (sd.vs.images[i].name) {
                                bindings.vs_images[i] = ih;
                            }
                        }
                        sg_pass_action pa;
                        zerofill(pa);
                        sg_begin_default_pass(&pa, 1, 1);
                        sg_apply_pipeline(ph);
                        sg_apply_bindings(&bindings);
                        sg_end_pass();
                        sg_draw(0, 0, 1);
                        sg_commit();
                        sg_destroy_buffer(vbh);
                        sg_destroy_image(ih);
                    }
                    sg_destroy_pipeline(ph);
                }
                else {
                    log("Cannot create the pipeline: %s\n", name);
                }
            }
            sg_destroy_shader(sh);
        }
        else {
            log("Cannot compile the shader: %s\n", name);
        }
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
    FILE *m_out;
    bool m_verbose;
};

} /* namespace anonymous */

sapp_desc
sokol_main(int argc, char **argv)
{
    sapp_desc desc;
    Main::zerofill(desc);
    desc.init_cb = []() {
        Compiler::initialize();
        sg_desc desc;
        Main::zerofill(desc);
        desc.context.metal.device = sapp_metal_get_device();
        desc.context.metal.drawable_cb = sapp_metal_get_drawable;
        desc.context.metal.renderpass_descriptor_cb = sapp_metal_get_renderpass_descriptor;
        desc.context.d3d11.depth_stencil_view_cb = sapp_d3d11_get_depth_stencil_view;
        desc.context.d3d11.device = sapp_d3d11_get_device();
        desc.context.d3d11.device_context = sapp_d3d11_get_device_context();
        desc.context.d3d11.render_target_view_cb = sapp_d3d11_get_render_target_view;
        sg_setup(&desc);
    };
    static int s_argc = argc;
    static char **s_argv = argv;
    desc.frame_cb = []() {
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
        if (s_argc > 1) {
            main.m_compiler->setValidationEnabled(true);
#if defined(SOKOL_METAL)
            main.m_compiler->setTargetLanguage(Compiler::kLanguageTypeMSL);
            main.m_compiler->setValidationEnabled(false);
            main.m_compiler->setOptimizeEnabled(true);
#elif defined(SOKOL_D3D11)
            main.m_compiler->setTargetLanguage(Compiler::kLanguageTypeHLSL);
            main.m_compiler->setValidationEnabled(false);
#endif
            for (int i = 1; i < s_argc; i++) {
                const char *arg = s_argv[i];
                if (strcmp(arg, "--optimization") == 0 || strcmp(arg, "--optimize") == 0) {
                    main.m_compiler->setOptimizeEnabled(true);
                    fprintf(stderr, "Optimization Enabled\n");
                }
                else if (strcmp(arg, "--essl") == 0) {
                    main.m_compiler->setTargetLanguage(Compiler::kLanguageTypeESSL);
                }
                else if (strcmp(arg, "--glsl") == 0) {
                    main.m_compiler->setTargetLanguage(Compiler::kLanguageTypeGLSL);
                }
                else if (strcmp(arg, "--hlsl") == 0) {
                    main.m_compiler->setTargetLanguage(Compiler::kLanguageTypeHLSL);
                    main.m_compiler->setValidationEnabled(false);
                }
                else if (strcmp(arg, "--msl") == 0) {
                    main.m_compiler->setTargetLanguage(Compiler::kLanguageTypeMSL);
                    main.m_compiler->setValidationEnabled(false);
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
                                size_t length = strlen(buffer);
                                if (length > 0 && *buffer != '#') {
                                    buffer[length - 1] = 0;
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
        Compiler::terminate();
        sg_shutdown();
        exit(0);
    };
    desc.cleanup_cb = []() {
        Compiler::terminate();
        sg_shutdown();
    };
    desc.width = desc.height = 1;
    return desc;
}
