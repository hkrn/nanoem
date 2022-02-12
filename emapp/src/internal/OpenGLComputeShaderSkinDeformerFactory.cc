/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/OpenGLComputeShaderSkinDeformerFactory.h"

#include "emapp/Error.h"
#include "emapp/ICamera.h"
#include "emapp/Model.h"
#include "emapp/StringUtils.h"
#include "emapp/private/CommonInclude.h"

#if defined(NANOEM_ENABLE_TBB)
#include "tbb/tbb.h"
#endif /* NANOEM_ENABLE_TBB */

namespace nanoem {
namespace internal {
namespace {

#define APIENTRYP APIENTRY *
typedef char GLchar;
typedef int GLint;
typedef unsigned int GLuint;
typedef unsigned int GLenum;
typedef int GLsizei;
typedef ptrdiff_t GLintptr;
typedef ptrdiff_t GLsizeiptr;

typedef void(APIENTRYP PFNGLATTACHSHADERPROC)(GLuint program, GLuint shader);
typedef void(APIENTRYP PFNGLBINDBUFFERPROC)(GLenum target, GLuint buffer);
typedef void(APIENTRYP PFNGLBINDBUFFERBASEPROC)(GLenum target, GLuint index, GLuint buffer);
typedef void(APIENTRYP PFNGLBUFFERDATAPROC)(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void(APIENTRYP PFNGLBUFFERSUBDATAPROC)(GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
typedef void(APIENTRYP PFNGLCOMPILESHADERPROC)(GLuint shader);
typedef void(APIENTRYP PFNGLDISPATCHCOMPUTEPROC)(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
typedef GLuint(APIENTRYP PFNGLCREATEPROGRAMPROC)(void);
typedef GLuint(APIENTRYP PFNGLCREATESHADERPROC)(GLenum type);
typedef void(APIENTRYP PFNGLDELETEBUFFERSPROC)(GLsizei n, const GLuint *buffers);
typedef void(APIENTRYP PFNGLDELETEPROGRAMPROC)(GLuint program);
typedef void(APIENTRYP PFNGLDELETESHADERPROC)(GLuint shader);
typedef void(APIENTRYP PFNGLGENBUFFERSPROC)(GLsizei n, GLuint *buffers);
typedef void(APIENTRYP PFNGLGETPROGRAMIVPROC)(GLuint program, GLenum pname, GLint *params);
typedef void(APIENTRYP PFNGLGETSHADERIVPROC)(GLuint shader, GLenum pname, GLint *params);
typedef void(APIENTRYP PFNGLLINKPROGRAMPROC)(GLuint program);
typedef void(APIENTRYP PFNGLOBJECTLABELPROC)(GLenum identifier, GLuint name, GLsizei length, const GLchar *label);
typedef void(APIENTRYP PFNGLSHADERSOURCEPROC)(
    GLuint shader, GLsizei count, const GLchar *const *string, const GLint *length);
typedef void(APIENTRYP PFNGLUSEPROGRAMPROC)(GLuint program);

static const GLenum GL_ARRAY_BUFFER = 0x8892;
static const GLenum GL_BUFFER = 0x82E0;
static const GLenum GL_COMPUTE_SHADER = 0x91B9;
static const GLenum GL_COMPILE_STATUS = 0x8B81;
static const GLenum GL_LINK_STATUS = 0x8B82;
static const GLenum GL_SHADER_STORAGE_BUFFER = 0x90D2;
static const GLenum GL_STATIC_READ = 0x88E5;
static const GLenum GL_STREAM_DRAW = 0x88E0;
static const GLenum GL_STREAM_READ = 0x88E1;
static const GLenum GL_UNIFORM_BUFFER = 0x8A11;

PFNGLATTACHSHADERPROC glAttachShader = nullptr;
PFNGLBINDBUFFERPROC glBindBuffer = nullptr;
PFNGLBINDBUFFERBASEPROC glBindBufferBase = nullptr;
PFNGLBUFFERDATAPROC glBufferData = nullptr;
PFNGLBUFFERSUBDATAPROC glBufferSubData = nullptr;
PFNGLCOMPILESHADERPROC glCompileShader = nullptr;
PFNGLCREATEPROGRAMPROC glCreateProgram = nullptr;
PFNGLCREATESHADERPROC glCreateShader = nullptr;
PFNGLDELETEBUFFERSPROC glDeleteBuffers = nullptr;
PFNGLDELETEPROGRAMPROC glDeleteProgram = nullptr;
PFNGLDELETESHADERPROC glDeleteShader = nullptr;
PFNGLDISPATCHCOMPUTEPROC glDispatchCompute = nullptr;
PFNGLGENBUFFERSPROC glGenBuffers = nullptr;
PFNGLGETPROGRAMIVPROC glGetProgramiv = nullptr;
PFNGLGETSHADERIVPROC glGetShaderiv = nullptr;
PFNGLLINKPROGRAMPROC glLinkProgram = nullptr;
PFNGLOBJECTLABELPROC glObjectLabel = nullptr;
PFNGLSHADERSOURCEPROC glShaderSource = nullptr;
PFNGLUSEPROGRAMPROC glUseProgram = nullptr;

#include "emapp/private/shaders/model_skinning_cs_glsl_core43.h"
#include "emapp/private/shaders/model_skinning_cs_glsl_es31.h"

struct BatchUpdateMatrixBufferRunner {
    BatchUpdateMatrixBufferRunner(ByteArray *matricesBuffer, model::Bone *const *bones)
        : m_matricesBuffer(matricesBuffer)
        , m_bones(bones)
    {
    }
    ~BatchUpdateMatrixBufferRunner()
    {
    }

    void
    execute(nanoem_rsize_t numBones)
    {
        bx::float4x4_t *matrices = reinterpret_cast<bx::float4x4_t *>(m_matricesBuffer->data());
        nanoem_parameter_assert(
            m_matricesBuffer->size() >= numBones * sizeof(*matrices), "must be bigger than capacity");
#if defined(NANOEM_ENABLE_TBB)
        tbb::parallel_for(tbb::blocked_range<nanoem_rsize_t>(0, numBones),
            [this, matrices](const tbb::blocked_range<nanoem_rsize_t> &range) {
                for (nanoem_rsize_t i = range.begin(), end = range.end(); i != end; i++) {
                    const model::Bone *bone = m_bones[i];
                    const bx::float4x4_t m = bone->skinningTransformMatrix();
                    memcpy(&matrices[i], &m, sizeof(*matrices));
                }
            });
#else
        for (nanoem_rsize_t i = 0; i < numBones; i++) {
            const model::Bone *bone = m_bones[i];
            const bx::float4x4_t m = bone->skinningTransformMatrix();
            memcpy(&matrices[i], &m, sizeof(matrices[0]));
        }
#endif /* NANOEM_ENABLE_TBB */
    }

    ByteArray *m_matricesBuffer;
    model::Bone *const *m_bones;
};

struct BatchUpdateMorphWeightBufferRunner {
    BatchUpdateMorphWeightBufferRunner(ByteArray *morphWeightBuffer, model::Morph *const *morphs)
        : m_morphWeightBuffer(morphWeightBuffer)
        , m_morphs(morphs)
    {
    }
    ~BatchUpdateMorphWeightBufferRunner()
    {
    }

    void
    execute(nanoem_rsize_t numMorphs)
    {
        nanoem_f32_t *weights = reinterpret_cast<nanoem_f32_t *>(m_morphWeightBuffer->data());
#if defined(NANOEM_ENABLE_TBB)
        tbb::parallel_for(tbb::blocked_range<nanoem_rsize_t>(0, numMorphs),
            [this, weights](const tbb::blocked_range<nanoem_rsize_t> &range) {
                for (nanoem_rsize_t i = range.begin(), end = range.end(); i != end; i++) {
                    model::Morph *morph = m_morphs[i];
                    weights[i + 1] = morph->weight();
                }
            });
#else
        for (nanoem_rsize_t i = 0; i < numMorphs; i++) {
            model::Morph *morph = m_morphs[i];
            weights[i + 1] = morph->weight();
        }
#endif /* NANOEM_ENABLE_TBB */
    }

    ByteArray *m_morphWeightBuffer;
    model::Morph *const *m_morphs;
};

} /* namespace anonymous */

OpenGLComputeShaderSkinDeformerFactory::OpenGLComputeShaderSkinDeformerFactory(PFN_GetProcAddress func)
    : m_program(0)
{
    glAttachShader = reinterpret_cast<PFNGLATTACHSHADERPROC>(func("glAttachShader"));
    glBindBuffer = reinterpret_cast<PFNGLBINDBUFFERPROC>(func("glBindBuffer"));
    glBindBufferBase = reinterpret_cast<PFNGLBINDBUFFERBASEPROC>(func("glBindBufferBase"));
    glBufferData = reinterpret_cast<PFNGLBUFFERDATAPROC>(func("glBufferData"));
    glBufferSubData = reinterpret_cast<PFNGLBUFFERSUBDATAPROC>(func("glBufferSubData"));
    glCompileShader = reinterpret_cast<PFNGLCOMPILESHADERPROC>(func("glCompileShader"));
    glCreateProgram = reinterpret_cast<PFNGLCREATEPROGRAMPROC>(func("glCreateProgram"));
    glCreateShader = reinterpret_cast<PFNGLCREATESHADERPROC>(func("glCreateShader"));
    glDeleteBuffers = reinterpret_cast<PFNGLDELETEBUFFERSPROC>(func("glDeleteBuffers"));
    glDeleteProgram = reinterpret_cast<PFNGLDELETEPROGRAMPROC>(func("glDeleteProgram"));
    glDeleteShader = reinterpret_cast<PFNGLDELETESHADERPROC>(func("glDeleteShader"));
    glDispatchCompute = reinterpret_cast<PFNGLDISPATCHCOMPUTEPROC>(func("glDispatchCompute"));
    glGenBuffers = reinterpret_cast<PFNGLGENBUFFERSPROC>(func("glGenBuffers"));
    glGetProgramiv = reinterpret_cast<PFNGLGETPROGRAMIVPROC>(func("glGetProgramiv"));
    glGetShaderiv = reinterpret_cast<PFNGLGETSHADERIVPROC>(func("glGetShaderiv"));
    glLinkProgram = reinterpret_cast<PFNGLLINKPROGRAMPROC>(func("glLinkProgram"));
    glObjectLabel = reinterpret_cast<PFNGLOBJECTLABELPROC>(func("glObjectLabel"));
    glShaderSource = reinterpret_cast<PFNGLSHADERSOURCEPROC>(func("glShaderSource"));
    glUseProgram = reinterpret_cast<PFNGLUSEPROGRAMPROC>(func("glUseProgram"));
}

OpenGLComputeShaderSkinDeformerFactory::~OpenGLComputeShaderSkinDeformerFactory() NANOEM_DECL_NOEXCEPT
{
    if (m_program) {
        glDeleteProgram(m_program);
        m_program = 0;
    }
}

model::ISkinDeformer *
OpenGLComputeShaderSkinDeformerFactory::create(Model *model)
{
    model::ISkinDeformer *deformer = nullptr;
    if (m_program) {
        deformer = nanoem_new(Deformer(this, model));
    }
    else {
        GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
        if (sg::query_backend() == SG_BACKEND_GLES3) {
            const char *const source = reinterpret_cast<const char *const>(g_nanoem_model_skinning_cs_glsl_es31_data);
            glShaderSource(shader, 1, &source, nullptr);
        }
        else {
            const char *const source = reinterpret_cast<const char *const>(g_nanoem_model_skinning_cs_glsl_core43_data);
            glShaderSource(shader, 1, &source, nullptr);
        }
        glCompileShader(shader);
        GLint result;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
        if (result) {
            GLint program = glCreateProgram();
            glAttachShader(program, shader);
            glLinkProgram(program);
            glGetProgramiv(program, GL_LINK_STATUS, &result);
            if (result) {
                deformer = nanoem_new(Deformer(this, model));
                m_program = program;
            }
            else {
                glDeleteProgram(program);
            }
        }
        glDeleteShader(shader);
    }
    return deformer;
}

void
OpenGLComputeShaderSkinDeformerFactory::begin()
{
}

void
OpenGLComputeShaderSkinDeformerFactory::commit()
{
}

OpenGLComputeShaderSkinDeformerFactory::Deformer::Deformer(OpenGLComputeShaderSkinDeformerFactory *parent, Model *model)
    : m_parent(parent)
    , m_model(model)
    , m_inputBufferObject(0)
    , m_matrixBufferObject(0)
    , m_morphWeightBufferObject(0)
    , m_vertexBufferObject(0)
    , m_sdefBufferObject(0)
    , m_argumentBufferObject(0)
    , m_numMaxMorphItems(0)
{
    m_buffer = { SG_INVALID_ID };
    Inline::clearZeroMemory(m_outputBufferObjects);
}

OpenGLComputeShaderSkinDeformerFactory::Deformer::~Deformer() NANOEM_DECL_NOEXCEPT
{
    destroyBufferObject(m_inputBufferObject);
    destroyBufferObject(m_matrixBufferObject);
    destroyBufferObject(m_morphWeightBufferObject);
    destroyBufferObject(m_sdefBufferObject);
    glDeleteBuffers(2, m_outputBufferObjects);
    m_outputBufferObjects[0] = m_outputBufferObjects[1] = 0;
}

sg_buffer
OpenGLComputeShaderSkinDeformerFactory::Deformer::create(const sg_buffer_desc &desc, int bufferIndex)
{
    if (m_argumentBufferObject == 0) {
        initializeShaderStorageBufferObject(48, m_argumentBufferObject);
        setDebugLabel(m_argumentBufferObject, "ArgumentBuffer");
    }
    if (m_inputBufferObject == 0) {
        createInputBuffer(desc);
    }
    if (m_outputBufferObjects[bufferIndex] == 0) {
        createOutputBuffer(desc, bufferIndex);
    }
    if (m_vertexBufferObject == 0) {
        createVertexBuffer();
    }
    if (m_sdefBufferObject == 0) {
        createSdefBuffer();
    }
    sg_buffer_desc d(desc);
    d.gl_buffers[0] = d.gl_buffers[1] = m_outputBufferObjects[bufferIndex];
    d.data.ptr = nullptr;
    sg_buffer buffer = m_buffer = sg::make_buffer(&d);
    return buffer;
}

void
OpenGLComputeShaderSkinDeformerFactory::Deformer::rebuildAllBones()
{
    m_bones.clear();
}

void
OpenGLComputeShaderSkinDeformerFactory::Deformer::destroy(sg_buffer value, int bufferIndex) NANOEM_DECL_NOEXCEPT
{
    sg::destroy_buffer(value);
    destroyBufferObject(m_outputBufferObjects[bufferIndex]);
}

void
OpenGLComputeShaderSkinDeformerFactory::Deformer::execute(int bufferIndex)
{
    nanoem_rsize_t numVertices;
    nanoemModelGetAllVertexObjects(m_model->data(), &numVertices);
    updateArgumentBuffer(numVertices);
    updateMatrixBuffer();
    updateMorphWeightBuffer();
    glUseProgram(m_parent->m_program);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_matrixBufferObject);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_morphWeightBufferObject);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_inputBufferObject);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_sdefBufferObject);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_vertexBufferObject);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, m_outputBufferObjects[bufferIndex]);
    const nanoem_u32_t numGroups = (Inline::saturateInt32U(numVertices) / 256) + 1;
    glDispatchCompute(numGroups, 1, 1);
    glUseProgram(0);
}

void
OpenGLComputeShaderSkinDeformerFactory::Deformer::initializeBufferObject(
    const void *data, nanoem_rsize_t size, nanoem_u32_t &object)
{
    if (!object) {
        const int innerSize = Inline::saturateInt32(size);
        glGenBuffers(1, &object);
        glBindBuffer(GL_ARRAY_BUFFER, object);
        glBufferData(GL_ARRAY_BUFFER, innerSize, nullptr, GL_STATIC_READ);
        glBufferSubData(GL_ARRAY_BUFFER, 0, innerSize, data);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void
OpenGLComputeShaderSkinDeformerFactory::Deformer::initializeBufferObject(nanoem_rsize_t size, nanoem_u32_t &object)
{
    if (!object) {
        glGenBuffers(1, &object);
        glBindBuffer(GL_ARRAY_BUFFER, object);
        glBufferData(GL_ARRAY_BUFFER, Inline::saturateInt32(size), nullptr, GL_STREAM_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void
OpenGLComputeShaderSkinDeformerFactory::Deformer::initializeShaderStorageBufferObject(
    nanoem_rsize_t size, nanoem_u32_t &object)
{
    if (!object) {
        glGenBuffers(1, &object);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, object);
        glBufferData(GL_SHADER_STORAGE_BUFFER, Inline::saturateInt32(size), nullptr, GL_STREAM_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
}

void
OpenGLComputeShaderSkinDeformerFactory::Deformer::initializeShaderStorageBufferObject(
    const ByteArray &bytes, nanoem_u32_t &object)
{
    if (!object) {
        glGenBuffers(1, &object);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, object);
        glBufferData(GL_SHADER_STORAGE_BUFFER, Inline::saturateInt32(bytes.size()), bytes.data(), GL_STATIC_READ);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
}

void
OpenGLComputeShaderSkinDeformerFactory::Deformer::updateBufferObject(const ByteArray &bytes, nanoem_u32_t object)
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, object);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, Inline::saturateInt32(bytes.size()), bytes.data());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void
OpenGLComputeShaderSkinDeformerFactory::Deformer::destroyBufferObject(nanoem_u32_t &object) NANOEM_DECL_NOEXCEPT
{
    if (object != 0) {
        glDeleteBuffers(1, &object);
        object = 0;
    }
}

void
OpenGLComputeShaderSkinDeformerFactory::Deformer::createInputBuffer(const sg_buffer_desc &desc)
{
    initializeBufferObject(desc.data.ptr, Inline::saturateInt32(desc.data.size), m_inputBufferObject);
    setDebugLabel(m_inputBufferObject, "InputBuffer");
}

void
OpenGLComputeShaderSkinDeformerFactory::Deformer::createOutputBuffer(const sg_buffer_desc &desc, int bufferIndex)
{
    char suffix[16] = { 0 };
    StringUtils::format(suffix, sizeof(suffix), "OutputBuffer/%d", bufferIndex);
    initializeBufferObject(Inline::saturateInt32(desc.size), m_outputBufferObjects[bufferIndex]);
    setDebugLabel(m_outputBufferObjects[bufferIndex], suffix);
}

void
OpenGLComputeShaderSkinDeformerFactory::Deformer::updateArgumentBuffer(nanoem_rsize_t numVertices)
{
    struct Argument {
        nanoem_u32_t m_numVertices;
        nanoem_u32_t m_numMaxMorphItems;
        nanoem_f32_t m_edgeScaleFactor;
        nanoem_u32_t m_padding;
    } args = {
        Inline::saturateInt32U(numVertices),
        Inline::saturateInt32U(m_numMaxMorphItems),
        m_model->edgeSize(),
        0,
    };
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_argumentBufferObject);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(args), &args);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_argumentBufferObject);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void
OpenGLComputeShaderSkinDeformerFactory::Deformer::updateMatrixBuffer()
{
    if (m_bones.empty()) {
        nanoem_rsize_t numBones;
        nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_model->data(), &numBones);
        if (numBones > 0) {
            m_bones.resize(numBones);
            for (nanoem_rsize_t i = 0; i < numBones; i++) {
                m_bones[i] = model::Bone::cast(bones[i]);
            }
        }
        else {
            m_bones.push_back(m_model->sharedFallbackBone());
            numBones = 1;
        }
        m_matrixBufferData.resize(numBones * sizeof(bx::float4x4_t));
        createMatrixBuffer();
    }
    BatchUpdateMatrixBufferRunner runner(&m_matrixBufferData, m_bones.data());
    runner.execute(m_bones.size());
    updateBufferObject(m_matrixBufferData, m_matrixBufferObject);
}

void
OpenGLComputeShaderSkinDeformerFactory::Deformer::updateMorphWeightBuffer()
{
    if (m_morphs.empty()) {
        nanoem_rsize_t numMorphs;
        nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(m_model->data(), &numMorphs);
        m_morphs.resize(numMorphs);
        for (nanoem_rsize_t i = 0; i < numMorphs; i++) {
            m_morphs[i] = model::Morph::cast(morphs[i]);
        }
        m_morphWeightBufferData.resize((numMorphs + 1) * sizeof(nanoem_f32_t));
        createMorphWeightBuffer();
    }
    BatchUpdateMorphWeightBufferRunner runner(&m_morphWeightBufferData, m_morphs.data());
    runner.execute(m_morphs.size());
    updateBufferObject(m_morphWeightBufferData, m_morphWeightBufferObject);
}

void
OpenGLComputeShaderSkinDeformerFactory::Deformer::createMatrixBuffer()
{
    const nanoem_rsize_t bufferSize = m_matrixBufferData.size();
    initializeShaderStorageBufferObject(Inline::saturateInt32(bufferSize), m_matrixBufferObject);
    setDebugLabel(m_matrixBufferObject, "MatrixBuffer");
}

void
OpenGLComputeShaderSkinDeformerFactory::Deformer::createMorphWeightBuffer()
{
    const nanoem_rsize_t bufferSize = m_morphWeightBufferData.size();
    initializeShaderStorageBufferObject(Inline::saturateInt32(bufferSize), m_morphWeightBufferObject);
    setDebugLabel(m_morphWeightBufferObject, "MorphWeightBuffer");
}

void
OpenGLComputeShaderSkinDeformerFactory::Deformer::createVertexBuffer()
{
    typedef tinystl::pair<const nanoem_model_morph_t *, const nanoem_model_morph_vertex_t *> VertexMorphPair;
    typedef tinystl::vector<VertexMorphPair, TinySTLAllocator> MorphPairList;
    tinystl::vector<MorphPairList, TinySTLAllocator> vertex2morphs;
    nanoem_rsize_t numVertices;
    nanoemModelGetAllVertexObjects(m_model->data(), &numVertices);
    {
        nanoem_rsize_t numMorphs;
        nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(m_model->data(), &numMorphs);
        vertex2morphs.resize(numVertices);
        for (nanoem_rsize_t i = 0; i < numMorphs; i++) {
            const nanoem_model_morph_t *morphPtr = morphs[i];
            switch (nanoemModelMorphGetType(morphPtr)) {
            case NANOEM_MODEL_MORPH_TYPE_VERTEX: {
                nanoem_rsize_t numItems;
                nanoem_model_morph_vertex_t *const *items =
                    nanoemModelMorphGetAllVertexMorphObjects(morphPtr, &numItems);
                for (nanoem_rsize_t j = 0; j < numItems; j++) {
                    const nanoem_model_morph_vertex_t *itemPtr = items[j];
                    const nanoem_model_vertex_t *vertexPtr = nanoemModelMorphVertexGetVertexObject(itemPtr);
                    int index = nanoemModelObjectGetIndex(nanoemModelVertexGetModelObject(vertexPtr));
                    vertex2morphs[index].push_back(
                        tinystl::make_pair(morphPtr, static_cast<const nanoem_model_morph_vertex_t *>(itemPtr)));
                }
                break;
            }
            case NANOEM_MODEL_MORPH_TYPE_UVA1:
            case NANOEM_MODEL_MORPH_TYPE_UVA2:
            case NANOEM_MODEL_MORPH_TYPE_UVA3:
            case NANOEM_MODEL_MORPH_TYPE_UVA4: {
                break;
            }
            default:
                break;
            }
        }
        for (tinystl::vector<MorphPairList, TinySTLAllocator>::const_iterator it = vertex2morphs.begin(),
                                                                              end = vertex2morphs.end();
             it != end; ++it) {
            m_numMaxMorphItems = glm::max(m_numMaxMorphItems, it->size());
        }
    }
    const nanoem_rsize_t bufferSize =
        glm::max(numVertices * m_numMaxMorphItems, nanoem_rsize_t(1)) * sizeof(bx::simd128_t);
    nanoem_rsize_t offset = 0;
    ByteArray vertexBufferData(bufferSize);
    bx::simd128_t *buffers = reinterpret_cast<bx::simd128_t *>(vertexBufferData.data());
    for (tinystl::vector<MorphPairList, TinySTLAllocator>::const_iterator it = vertex2morphs.begin(),
                                                                          end = vertex2morphs.end();
         it != end; ++it) {
        bx::simd128_t *item = &buffers[offset * m_numMaxMorphItems];
        int i = 0;
        for (MorphPairList::const_iterator it2 = it->begin(), end2 = it->end(); it2 != end2; ++it2, ++i) {
            const nanoem_f32_t *position = nanoemModelMorphVertexGetPosition(it2->second);
            /* reserve index zero for non morph weight */
            int index = nanoemModelObjectGetIndex(nanoemModelMorphGetModelObject(it2->first)) + 1;
            item[i] = bx::simd_ld(position[0], position[1], position[2], float(index));
        }
        offset++;
    }
    initializeShaderStorageBufferObject(vertexBufferData, m_vertexBufferObject);
    setDebugLabel(m_vertexBufferObject, "VertexBuffer");
}

void
OpenGLComputeShaderSkinDeformerFactory::Deformer::createSdefBuffer()
{
    nanoem_rsize_t numVertices;
    nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(m_model->data(), &numVertices);
    ByteArray sdefBuffer(numVertices * sizeof(bx::simd128_t) * 3);
    bx::simd128_t *sdefBufferPtr = reinterpret_cast<bx::simd128_t *>(sdefBuffer.data());
    for (nanoem_rsize_t i = 0; i < numVertices; i++) {
        const nanoem_model_vertex_t *vertexPtr = vertices[i];
        const nanoem_rsize_t offset = i * 3;
        memcpy(&sdefBufferPtr[offset + 0], nanoemModelVertexGetSdefC(vertexPtr), sizeof(*sdefBufferPtr));
        memcpy(&sdefBufferPtr[offset + 1], nanoemModelVertexGetSdefR0(vertexPtr), sizeof(*sdefBufferPtr));
        memcpy(&sdefBufferPtr[offset + 2], nanoemModelVertexGetSdefR1(vertexPtr), sizeof(*sdefBufferPtr));
    }
    initializeShaderStorageBufferObject(sdefBuffer, m_sdefBufferObject);
    setDebugLabel(m_sdefBufferObject, "SdefBuffer");
}

void
OpenGLComputeShaderSkinDeformerFactory::Deformer::setDebugLabel(nanoem_u32_t object, const char *suffix)
{
#if defined(SOKOL_DEBUG) && SOKOL_DEBUG
    char label[Inline::kMarkerStringLength];
    const char *name = m_model->canonicalNameConstString();
    StringUtils::format(label, sizeof(label), "Models/%s/Compute/%s", name, suffix);
    glObjectLabel(GL_BUFFER, object, -1, label);
#else
    BX_UNUSED_2(object, suffix);
#endif /* SOKOL_DEBUG */
}

} /* namespace internal */
} /* namespace nanoem */
