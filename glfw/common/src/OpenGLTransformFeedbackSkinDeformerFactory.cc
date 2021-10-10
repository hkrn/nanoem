/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "OpenGLTransformFeedbackSkinDeformerFactory.h"

#include "emapp/Error.h"
#include "emapp/ICamera.h"
#include "emapp/Model.h"
#include "emapp/StringUtils.h"
#include "emapp/private/CommonInclude.h"

#if defined(NANOEM_ENABLE_TBB)
#include "tbb/tbb.h"
#endif /* NANOEM_ENABLE_TBB */

namespace nanoem {
namespace glfw {
namespace {

#include "emapp/private/shaders/model_skinning_tf_vs_glsl_es3.h"

static const char *kTransformFeedbackInput[] = {
    "a_position",
    "a_normal",
    "a_texcoord",
    "a_edge",
    "a_uva1",
    "a_uva2",
    "a_uva3",
    "a_uva4",
    "a_weights",
    "a_indices",
    "a_info",
};
static const size_t kTransformFeedbackInputStride = BX_COUNTOF(kTransformFeedbackInput) * 16;
static const char *kTransformFeedbackOutput[] = {
    "v_position",
    "v_normal",
    "v_texcoord",
    "v_edge",
    "v_uva1",
    "v_uva2",
    "v_uva3",
    "v_uva4",
    "v_weights",
    "v_indices",
    "v_info",
};

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

OpenGLTransformFeedbackSkinDeformerFactory::OpenGLTransformFeedbackSkinDeformerFactory()
    : m_program(0)
    , m_matrixTextureLocation(-1)
    , m_morphWeightTextureLocation(-1)
    , m_sdefTextureLocation(-1)
    , m_vertexTextureLocation(-1)
    , m_argumentUniformLocation(-1)
{
}

OpenGLTransformFeedbackSkinDeformerFactory::~OpenGLTransformFeedbackSkinDeformerFactory()
{
    if (m_program) {
        glDeleteProgram(m_program);
        m_program = 0;
    }
}

model::ISkinDeformer *
OpenGLTransformFeedbackSkinDeformerFactory::create(Model *model)
{
    model::ISkinDeformer *deformer = nullptr;
    if (m_program) {
        deformer = nanoem_new(Deformer(this, model));
    }
    else {
        GLuint vss = glCreateShader(GL_VERTEX_SHADER);
        const char *const vsSource = reinterpret_cast<const char *const>(g_nanoem_model_skinning_tf_fs_glsl_es3_data);
        glShaderSource(vss, 1, &vsSource, nullptr);
        glCompileShader(vss);
        GLuint fss = glCreateShader(GL_FRAGMENT_SHADER);
        const char *const fsSource = R"(#version 300 es
precision highp float;
out vec4 o_color;

void main()
{
  o_color = vec4(1.0);
}
                                   )";
        glShaderSource(fss, 1, &fsSource, nullptr);
        glCompileShader(fss);
        GLint program = glCreateProgram();
        glAttachShader(program, vss);
        glAttachShader(program, fss);
        for (size_t i = 0; i < BX_COUNTOF(kTransformFeedbackInput); i++) {
            glBindAttribLocation(program, Inline::saturateInt32U(i), kTransformFeedbackInput[i]);
        }
        glTransformFeedbackVaryings(
            program, BX_COUNTOF(kTransformFeedbackOutput), kTransformFeedbackOutput, GL_INTERLEAVED_ATTRIBS);
        glLinkProgram(program);
        m_argumentUniformLocation = glGetUniformLocation(program, "_Global");
        m_matrixTextureLocation = glGetUniformLocation(program, "SPIRV_Cross_Combinedu_matricesTextureSPIRV_Cross_DummySampler");
        m_morphWeightTextureLocation = glGetUniformLocation(program, "SPIRV_Cross_Combinedu_morphWeightTextureSPIRV_Cross_DummySampler");
        m_sdefTextureLocation = glGetUniformLocation(program, "SPIRV_Cross_Combinedu_sdefTextureSPIRV_Cross_DummySampler");
        m_vertexTextureLocation = glGetUniformLocation(program, "SPIRV_Cross_Combinedu_verticesTextureSPIRV_Cross_DummySampler");
        GLint result;
        glGetProgramiv(program, GL_LINK_STATUS, &result);
        if (result) {
            deformer = nanoem_new(Deformer(this, model));
            m_program = program;
        }
        else {
            glDeleteProgram(program);
        }
        glDeleteShader(vss);
        glDeleteShader(fss);
    }
    return deformer;
}

void
OpenGLTransformFeedbackSkinDeformerFactory::begin()
{
}

void
OpenGLTransformFeedbackSkinDeformerFactory::commit()
{
}

OpenGLTransformFeedbackSkinDeformerFactory::Deformer::Deformer(
    OpenGLTransformFeedbackSkinDeformerFactory *parent, Model *model)
    : m_parent(parent)
    , m_model(model)
    , m_inputBufferObject(0)
    , m_matrixTextureObject(0)
    , m_morphWeightTextureObject(0)
    , m_sdefTextureObject(0)
    , m_vertexTextureObject(0)
    , m_matrixTextureSize(0)
    , m_morphWeightTextureSize(0)
    , m_numMaxMorphItems(0)
{
    sg_buffer m_buffer = { SG_INVALID_ID };
    Inline::clearZeroMemory(m_outputBufferObjects);
}

OpenGLTransformFeedbackSkinDeformerFactory::Deformer::~Deformer()
{
    destroyBufferObject(m_inputBufferObject);
    destroyTextureObject(m_matrixTextureObject);
    destroyTextureObject(m_morphWeightTextureObject);
    destroyTextureObject(m_sdefTextureObject);
    destroyTextureObject(m_vertexTextureObject);
    glDeleteBuffers(2, m_outputBufferObjects);
    m_outputBufferObjects[0] = m_outputBufferObjects[1] = 0;
}

sg_buffer
OpenGLTransformFeedbackSkinDeformerFactory::Deformer::create(const sg_buffer_desc &desc, int bufferIndex)
{
    int size = Inline::saturateInt32(desc.data.size);
    if (m_inputBufferObject == 0) {
        createInputBuffer(desc);
    }
    if (m_outputBufferObjects[bufferIndex] == 0) {
        createOutputBuffer(desc, bufferIndex);
    }
    if (m_vertexTextureObject == 0) {
        createVertexBuffer();
    }
    if (m_sdefTextureObject == 0) {
        createSdefBuffer();
    }
    sg_buffer_desc d(desc);
    d.gl_buffers[0] = d.gl_buffers[1] = m_outputBufferObjects[bufferIndex];
    d.data.ptr = nullptr;
    sg_buffer buffer = m_buffer = sg::make_buffer(&d);
    return buffer;
}

void
OpenGLTransformFeedbackSkinDeformerFactory::Deformer::rebuildAllBones()
{
    m_bones.clear();
}

void
OpenGLTransformFeedbackSkinDeformerFactory::Deformer::destroy(sg_buffer value, int bufferIndex) noexcept
{
    sg::destroy_buffer(value);
    GLuint &outputBuffer = m_outputBufferObjects[bufferIndex];
    glDeleteBuffers(1, &outputBuffer);
    outputBuffer = 0;
}

void
OpenGLTransformFeedbackSkinDeformerFactory::Deformer::execute(int bufferIndex)
{
    nanoem_rsize_t numVertices;
    nanoemModelGetAllVertexObjects(m_model->data(), &numVertices);
    updateMatrixBuffer();
    updateMorphWeightBuffer();
    glUseProgram(m_parent->m_program);
    activateTextureUniform(m_parent->m_matrixTextureLocation, m_matrixTextureObject, 0);
    activateTextureUniform(m_parent->m_morphWeightTextureLocation, m_morphWeightTextureObject, 1);
    activateTextureUniform(m_parent->m_sdefTextureLocation, m_sdefTextureObject, 2);
    activateTextureUniform(m_parent->m_vertexTextureLocation, m_vertexTextureObject, 3);
    glUniform4f(m_parent->m_argumentUniformLocation, m_model->edgeSize(), float(m_numMaxMorphItems), 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, m_inputBufferObject);
    for (size_t i = 0; i < BX_COUNTOF(kTransformFeedbackInput); i++) {
        const void *offset = reinterpret_cast<const void *>(i * sizeof(bx::simd128_t));
        const nanoem_u32_t index = Inline::saturateInt32U(i);
        glEnableVertexAttribArray(index);
        glVertexAttribPointer(index, 4, GL_FLOAT, GL_FALSE, kTransformFeedbackInputStride, offset);
    }
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_outputBufferObjects[bufferIndex]);
    glEnable(GL_RASTERIZER_DISCARD);
    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, GLsizei(numVertices));
    glEndTransformFeedback();
    glDisable(GL_RASTERIZER_DISCARD);
    glUseProgram(0);
    sg::reset_state_cache();
}

nanoem_rsize_t 
OpenGLTransformFeedbackSkinDeformerFactory::Deformer::alignBufferSize(nanoem_rsize_t value) NANOEM_DECL_NOEXCEPT
{
    const nanoem_rsize_t logicalSize = value / 16;
    const int stride = static_cast<int>(glm::ceil(glm::sqrt(logicalSize * 1.0f)));
    nanoem_rsize_t actualSize = value;
    if (stride * stride > logicalSize) {
        actualSize = stride * stride * 16;
    }
    return actualSize;
}

void
OpenGLTransformFeedbackSkinDeformerFactory::Deformer::reserveBufferWithAlignedSize(ByteArray &bytes)
{
    bytes.reserve(alignBufferSize(bytes.size()));
}

void
OpenGLTransformFeedbackSkinDeformerFactory::Deformer::initializeBufferObject(
    const void *data, nanoem_rsize_t size, GLuint &object)
{
    if (!object) {
        glGenBuffers(1, &object);
        glBindBuffer(GL_ARRAY_BUFFER, object);
        glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STATIC_READ);
        glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void
OpenGLTransformFeedbackSkinDeformerFactory::Deformer::initializeBufferObject(nanoem_rsize_t size, GLuint &object)
{
    if (!object) {
        glGenBuffers(1, &object);
        glBindBuffer(GL_ARRAY_BUFFER, object);
        glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STREAM_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void
OpenGLTransformFeedbackSkinDeformerFactory::Deformer::initializeTextureObject(nanoem_rsize_t size, GLuint &object, Vector2SI32 &s)
{
    if (!object) {
        const int stride = static_cast<int>(glm::ceil(glm::sqrt(size / sizeof(bx::simd128_t) * 1.0f)));
        s = Vector2SI32(stride, stride);
        glGenTextures(1, &object);
        glBindTexture(GL_TEXTURE_2D, object);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, s.x, s.y, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void
OpenGLTransformFeedbackSkinDeformerFactory::Deformer::initializeTextureObject(
    const ByteArray &bytes, nanoem_u32_t &object, Vector2SI32 &s)
{
    if (!object) {
        const nanoem_rsize_t logicalSize = bytes.size() / 16;
        const int stride = static_cast<int>(glm::ceil(glm::sqrt(logicalSize * 1.0f)));
        s = Vector2SI32(stride, stride);
        glGenTextures(1, &object);
        glBindTexture(GL_TEXTURE_2D, object);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, s.x, s.y, 0, GL_RGBA, GL_FLOAT, bytes.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void
OpenGLTransformFeedbackSkinDeformerFactory::Deformer::updateTextureObject(
    const ByteArray &bytes, const Vector2SI32 &s, nanoem_u32_t object)
{
    glBindTexture(GL_TEXTURE_2D, object);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, s.x, s.y, GL_RGBA, GL_FLOAT, bytes.data());
    glBindTexture(GL_TEXTURE_2D, 0);
}

void
OpenGLTransformFeedbackSkinDeformerFactory::Deformer::activateTextureUniform(
    nanoem_u32_t location, nanoem_u32_t object, int offset)
{
    glActiveTexture(GL_TEXTURE0 + offset);
    glBindTexture(GL_TEXTURE_2D, object);
    glUniform1i(location, offset);
}

void
OpenGLTransformFeedbackSkinDeformerFactory::Deformer::destroyBufferObject(nanoem_u32_t &object) NANOEM_DECL_NOEXCEPT
{
    if (object != 0) {
        glDeleteBuffers(1, &object);
        object = 0;
    }
}

void
OpenGLTransformFeedbackSkinDeformerFactory::Deformer::destroyTextureObject(nanoem_u32_t &object) NANOEM_DECL_NOEXCEPT
{
    if (object != 0) {
        glDeleteTextures(1, &object);
        object = 0;
    }
}

void
OpenGLTransformFeedbackSkinDeformerFactory::Deformer::createInputBuffer(const sg_buffer_desc &desc)
{
    initializeBufferObject(desc.data.ptr, Inline::saturateInt32(desc.data.size), m_inputBufferObject);
    setDebugLabel(m_inputBufferObject, GL_BUFFER, "InputBuffer");
}

void
OpenGLTransformFeedbackSkinDeformerFactory::Deformer::createOutputBuffer(const sg_buffer_desc &desc, int bufferIndex)
{
    char suffix[16] = { 0 };
    StringUtils::format(suffix, sizeof(suffix), "OutputBuffer/%d", bufferIndex);
    initializeBufferObject(Inline::saturateInt32(desc.size), m_outputBufferObjects[bufferIndex]);
    setDebugLabel(m_outputBufferObjects[bufferIndex], GL_BUFFER, suffix);
}

void
OpenGLTransformFeedbackSkinDeformerFactory::Deformer::updateMatrixBuffer()
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
        reserveBufferWithAlignedSize(m_matrixBufferData);
        createMatrixBuffer();
    }
    BatchUpdateMatrixBufferRunner runner(&m_matrixBufferData, m_bones.data());
    runner.execute(m_bones.size());
    updateTextureObject(m_matrixBufferData, m_matrixTextureSize, m_matrixTextureObject);
}

void
OpenGLTransformFeedbackSkinDeformerFactory::Deformer::updateMorphWeightBuffer()
{
    if (m_morphs.empty()) {
        nanoem_rsize_t numMorphs;
        nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(m_model->data(), &numMorphs);
        m_morphs.resize(numMorphs);
        for (nanoem_rsize_t i = 0; i < numMorphs; i++) {
            m_morphs[i] = model::Morph::cast(morphs[i]);
        }
        m_morphWeightBufferData.resize(((numMorphs + 1) / 4) * sizeof(bx::simd128_t));
        reserveBufferWithAlignedSize(m_morphWeightBufferData);
        createMorphWeightBuffer();
    }
    BatchUpdateMorphWeightBufferRunner runner(&m_morphWeightBufferData, m_morphs.data());
    runner.execute(m_morphs.size());
    updateTextureObject(m_morphWeightBufferData, m_morphWeightTextureSize, m_morphWeightTextureObject);
}

void
OpenGLTransformFeedbackSkinDeformerFactory::Deformer::createMatrixBuffer()
{
    const nanoem_rsize_t bufferSize = m_matrixBufferData.size();
    initializeTextureObject(bufferSize, m_matrixTextureObject, m_matrixTextureSize);
    setDebugLabel(m_matrixTextureObject, GL_TEXTURE, "MatrixTexture");
}

void
OpenGLTransformFeedbackSkinDeformerFactory::Deformer::createMorphWeightBuffer()
{
    const nanoem_rsize_t bufferSize = m_morphWeightBufferData.size();
    initializeTextureObject(bufferSize, m_morphWeightTextureObject, m_morphWeightTextureSize);
    setDebugLabel(m_morphWeightTextureObject, GL_TEXTURE, "MorphWeightTexture");
}

void
OpenGLTransformFeedbackSkinDeformerFactory::Deformer::createVertexBuffer()
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
    const nanoem_rsize_t bufferSize = glm::max(numVertices * m_numMaxMorphItems, nanoem_rsize_t(1)) * sizeof(bx::simd128_t);
    nanoem_rsize_t offset = 0;
    ByteArray vertexBufferData(bufferSize);
    reserveBufferWithAlignedSize(vertexBufferData);
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
    Vector2SI32 size;
    initializeTextureObject(vertexBufferData, m_vertexTextureObject, size);
    setDebugLabel(m_vertexTextureObject, GL_TEXTURE, "VertexTexture");
}

void
OpenGLTransformFeedbackSkinDeformerFactory::Deformer::createSdefBuffer()
{
    nanoem_rsize_t numVertices;
    nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(m_model->data(), &numVertices);
    ByteArray sdefBufferData(numVertices * sizeof(bx::simd128_t) * 3);
    reserveBufferWithAlignedSize(sdefBufferData);
    bx::simd128_t *sdefBufferPtr = reinterpret_cast<bx::simd128_t *>(sdefBufferData.data());
    for (nanoem_rsize_t i = 0; i < numVertices; i++) {
        const nanoem_model_vertex_t *vertexPtr = vertices[i];
        const nanoem_rsize_t offset = i * 3;
        memcpy(&sdefBufferPtr[offset + 0], nanoemModelVertexGetSdefC(vertexPtr), sizeof(*sdefBufferPtr));
        memcpy(&sdefBufferPtr[offset + 1], nanoemModelVertexGetSdefR0(vertexPtr), sizeof(*sdefBufferPtr));
        memcpy(&sdefBufferPtr[offset + 2], nanoemModelVertexGetSdefR1(vertexPtr), sizeof(*sdefBufferPtr));
    }
    Vector2SI32 size;
    initializeTextureObject(sdefBufferData, m_sdefTextureObject, size);
    setDebugLabel(m_sdefTextureObject, GL_TEXTURE, "SdefTexture");
}

void
OpenGLTransformFeedbackSkinDeformerFactory::Deformer::setDebugLabel(nanoem_u32_t object, nanoem_u32_t type, const char *suffix)
{
#if defined(SOKOL_DEBUG)
    char label[Inline::kMarkerStringLength];
    const char *name = m_model->canonicalNameConstString();
    StringUtils::format(label, sizeof(label), "Models/%s/TransformFeedback/%s", name, suffix);
    glObjectLabel(type, object, -1, label);
#else
    BX_UNUSED_2(object, suffix);
#endif /* SOKOL_DEBUG */
}

} /* namespace glfw */
} /* namespace nanoem */
