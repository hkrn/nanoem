/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "OpenGLTransformFeedbackSkinDeformerFactory.h"

#include "emapp/Error.h"
#include "emapp/ICamera.h"
#include "emapp/Model.h"
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
    BatchUpdateMatrixBufferRunner(ByteArray *bytes, model::Bone *const *bones)
        : m_bytes(bytes)
        , m_bones(bones)
    {
    }
    ~BatchUpdateMatrixBufferRunner()
    {
    }

    void
    execute(nanoem_rsize_t numBones)
    {
        bx::float4x4_t *matrices = reinterpret_cast<bx::float4x4_t *>(m_bytes->data());
        nanoem_parameter_assert(m_bytes->size() >= numBones * sizeof(*matrices), "must be bigger than capacity");
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

    ByteArray *m_bytes;
    model::Bone *const *m_bones;
};

struct BatchUpdateVertexDeltaBufferRunner {
    BatchUpdateVertexDeltaBufferRunner(ByteArray *bytes, model::Vertex *const *vertices)
        : m_bytes(bytes)
        , m_vertexDeltas(vertices)
    {
    }
    ~BatchUpdateVertexDeltaBufferRunner()
    {
    }

    void
    execute(nanoem_rsize_t numVertices)
    {
        bx::simd128_t *vertexDeltas = reinterpret_cast<bx::simd128_t *>(m_bytes->data());
#if defined(NANOEM_ENABLE_TBB)
        tbb::parallel_for(tbb::blocked_range<nanoem_rsize_t>(0, numVertices),
            [this, vertexDeltas](const tbb::blocked_range<nanoem_rsize_t> &range) {
                for (nanoem_rsize_t i = range.begin(), end = range.end(); i != end; i++) {
                    model::Vertex *vertex = m_vertexDeltas[i];
                    vertexDeltas[i] = vertex->m_simd.m_delta;
                    vertex->reset();
                }
            });
#else
        for (nanoem_rsize_t i = 0; i < numVertices; i++) {
            model::Vertex *vertex = m_vertexDeltas[i];
            vertexDeltas[i] = vertex->m_simd.m_delta;
            vertex->reset();
        }
#endif /* NANOEM_ENABLE_TBB */
    }

    ByteArray *m_bytes;
    model::Vertex *const *m_vertexDeltas;
};

} /* namespace anonymous */

OpenGLTransformFeedbackSkinDeformerFactory::OpenGLTransformFeedbackSkinDeformerFactory()
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
        m_edgeScaleFactorLocation = glGetUniformLocation(program, "_648");
#if 0
        m_matrixTextureLocation =
            glGetUniformLocation(program, "SPIRV_Cross_Combinedu_matricesTextureSPIRV_Cross_DummySampler");
        m_vertexTextureLocation =
            glGetUniformLocation(program, "SPIRV_Cross_Combinedu_verticesTextureSPIRV_Cross_DummySampler");
        m_sdefTextureLocation =
            glGetUniformLocation(program, "SPIRV_Cross_Combinedu_sdefTextureSPIRV_Cross_DummySampler");
#else
        m_vertexTextureLocation = glGetUniformLocation(program, "SPIRV_Cross_CombinedSPIRV_Cross_DummySampler");
        m_matrixTextureLocation = glGetUniformLocation(program, "SPIRV_Cross_CombinedSPIRV_Cross_DummySampler_1");
        m_sdefTextureLocation = glGetUniformLocation(program, "SPIRV_Cross_CombinedSPIRV_Cross_DummySampler_2");
#endif
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
{
}

OpenGLTransformFeedbackSkinDeformerFactory::Deformer::~Deformer()
{
    glDeleteBuffers(1, &m_inputBuffer);
    m_inputBuffer = 0;
    glDeleteTextures(1, &m_matrixTexture);
    m_matrixTexture = 0;
    glDeleteTextures(1, &m_vertexDeltaTexture);
    m_vertexDeltaTexture = 0;
    glDeleteTextures(1, &m_sdefTexture);
    m_sdefTexture = 0;
    glDeleteBuffers(2, m_outputBuffers);
    m_outputBuffers[0] = m_outputBuffers[1] = 0;
}

sg_buffer
OpenGLTransformFeedbackSkinDeformerFactory::Deformer::create(const sg_buffer_desc &desc, int bufferIndex)
{
    Error error;
    nanoem_rsize_t numBones, numVertices;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_model->data(), &numBones);
    nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(m_model->data(), &numVertices);
    int size = Inline::saturateInt32(desc.data.size);
    initializeBufferObject(desc.data.ptr, size, m_inputBuffer);
    initializeTextureObject(Inline::saturateInt32(numBones * 4), m_matrixTexture, m_matrixTextureSize);
    m_matrixTextureData.resize(sizeof(bx::simd128_t) * m_matrixTextureSize.x * m_matrixTextureSize.y);
    initializeTextureObject(Inline::saturateInt32(numVertices), m_vertexDeltaTexture, m_vertexDeltaTextureSize);
    m_vertexDeltaTextureData.resize(sizeof(bx::simd128_t) * m_vertexDeltaTextureSize.x * m_vertexDeltaTextureSize.y);
    initializeBufferObject(size, m_outputBuffers[bufferIndex]);
    if (m_bones.empty()) {
        m_bones.resize(numBones);
        for (nanoem_rsize_t i = 0; i < numBones; i++) {
            m_bones[i] = model::Bone::cast(bones[i]);
        }
    }
    if (m_vertexDeltas.empty()) {
        m_vertexDeltas.resize(numVertices);
        int stride = static_cast<int>(glm::ceil(glm::sqrt(numVertices * 3.0f)));
        glGenTextures(1, &m_sdefTexture);
        glBindTexture(GL_TEXTURE_2D, m_sdefTexture);
        ByteArray bytes(numVertices * sizeof(bx::simd128_t) * 3);
        auto sdefPtr = reinterpret_cast<bx::simd128_t *>(bytes.data());
        for (nanoem_rsize_t i = 0; i < numVertices; i++) {
            const nanoem_model_vertex_t *vertexPtr = vertices[i];
            model::Vertex *vertex = model::Vertex::cast(vertexPtr);
            const nanoem_rsize_t offset = i * 3;
            m_vertexDeltas[i] = vertex;
            memcpy(&sdefPtr[offset + 0], nanoemModelVertexGetSdefC(vertexPtr), sizeof(*sdefPtr));
            memcpy(&sdefPtr[offset + 1], nanoemModelVertexGetSdefR0(vertexPtr), sizeof(*sdefPtr));
            memcpy(&sdefPtr[offset + 2], nanoemModelVertexGetSdefR1(vertexPtr), sizeof(*sdefPtr));
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, stride, stride, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    sg_buffer_desc d(desc);
    d.gl_buffers[0] = d.gl_buffers[1] = m_outputBuffers[bufferIndex];
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
    GLuint &outputBuffer = m_outputBuffers[bufferIndex];
    glDeleteBuffers(1, &outputBuffer);
    outputBuffer = 0;
}

void
OpenGLTransformFeedbackSkinDeformerFactory::Deformer::execute(int bufferIndex)
{
    glUseProgram(m_parent->m_program);
    glUniform4f(m_parent->m_edgeScaleFactorLocation, m_model->edgeSize(), 0, 0, 0);
    glActiveTexture(GL_TEXTURE0);
    {
        BatchUpdateMatrixBufferRunner runner(&m_matrixTextureData, m_bones.data());
        runner.execute(m_bones.size());
        glBindTexture(GL_TEXTURE_2D, m_matrixTexture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_matrixTextureSize.x, m_matrixTextureSize.y, GL_RGBA, GL_FLOAT,
            m_matrixTextureData.data());
        glUniform1i(m_parent->m_matrixTextureLocation, 0);
    }
    glActiveTexture(GL_TEXTURE1);
    {
        BatchUpdateVertexDeltaBufferRunner runner(&m_vertexDeltaTextureData, m_vertexDeltas.data());
        runner.execute(m_vertexDeltas.size());
        glBindTexture(GL_TEXTURE_2D, m_vertexDeltaTexture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_vertexDeltaTextureSize.x, m_vertexDeltaTextureSize.y, GL_RGBA,
            GL_FLOAT, m_vertexDeltaTextureData.data());
        glUniform1i(m_parent->m_vertexTextureLocation, 1);
    }
    glBindBuffer(GL_ARRAY_BUFFER, m_inputBuffer);
    for (size_t i = 0; i < BX_COUNTOF(kTransformFeedbackInput); i++) {
        const void *offset = reinterpret_cast<const void *>(i * sizeof(bx::simd128_t));
        const nanoem_u32_t index = Inline::saturateInt32U(i);
        glEnableVertexAttribArray(index);
        glVertexAttribPointer(index, 4, GL_FLOAT, GL_FALSE, kTransformFeedbackInputStride, offset);
    }
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_outputBuffers[bufferIndex]);
    glEnable(GL_RASTERIZER_DISCARD);
    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, Inline::saturateInt32(m_vertexDeltas.size()));
    glEndTransformFeedback();
    glDisable(GL_RASTERIZER_DISCARD);
    sg::reset_state_cache();
}

void
OpenGLTransformFeedbackSkinDeformerFactory::Deformer::initializeBufferObject(const void *data, int size, GLuint &object)
{
    if (!object) {
        glGenBuffers(1, &object);
        glBindBuffer(GL_ARRAY_BUFFER, object);
        glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void
OpenGLTransformFeedbackSkinDeformerFactory::Deformer::initializeBufferObject(int size, GLuint &object)
{
    if (!object) {
        glGenBuffers(1, &object);
        glBindBuffer(GL_ARRAY_BUFFER, object);
        glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STREAM_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void
OpenGLTransformFeedbackSkinDeformerFactory::Deformer::initializeTextureObject(int size, GLuint &object, Vector2SI32 &s)
{
    if (!object) {
        const int stride = static_cast<int>(glm::ceil(glm::sqrt(size * 1.0f)));
        s = Vector2SI32(stride, stride);
        glGenTextures(1, &object);
        glBindTexture(GL_TEXTURE_2D, object);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, s.x, s.y, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

} /* namespace glfw */
} /* namespace nanoem */
