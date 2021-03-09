/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "OpenGLComputeShaderSkinDeformerFactory.h"

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

#include "emapp/private/shaders/model_skinning_cs_glsl_core43.h"
#include "emapp/private/shaders/model_skinning_cs_glsl_es31.h"

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

OpenGLComputeShaderSkinDeformerFactory::OpenGLComputeShaderSkinDeformerFactory()
{
}

OpenGLComputeShaderSkinDeformerFactory::~OpenGLComputeShaderSkinDeformerFactory()
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
{
}

OpenGLComputeShaderSkinDeformerFactory::Deformer::~Deformer()
{
    glDeleteBuffers(1, &m_inputBuffer);
    m_inputBuffer = 0;
    glDeleteBuffers(1, &m_matrixBuffer);
    m_matrixBuffer = 0;
    glDeleteBuffers(1, &m_vertexDeltaBuffer);
    m_vertexDeltaBuffer = 0;
    glDeleteBuffers(1, &m_sdefBuffer);
    m_sdefBuffer = 0;
    glDeleteBuffers(2, m_outputBuffers);
    m_outputBuffers[0] = m_outputBuffers[1] = 0;
}

sg_buffer
OpenGLComputeShaderSkinDeformerFactory::Deformer::create(const sg_buffer_desc &desc, int bufferIndex)
{
    Error error;
    nanoem_rsize_t numBones, numVertices;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_model->data(), &numBones);
    nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(m_model->data(), &numVertices);
    initializeBufferObject(desc.data.ptr, Inline::saturateInt32(desc.data.size), m_inputBuffer);
    const nanoem_rsize_t matrixBufferSize = sizeof(bx::simd128_t) * numBones * 4;
    initializeShaderStorageBufferObject(Inline::saturateInt32(matrixBufferSize), m_matrixBuffer);
    m_matrixBufferData.resize(matrixBufferSize);
    const nanoem_rsize_t vertexBufferSize = sizeof(bx::simd128_t) * numVertices;
    initializeShaderStorageBufferObject(Inline::saturateInt32(vertexBufferSize), m_vertexDeltaBuffer);
    initializeShaderStorageBufferObject(48, m_argumentBuffer);
    m_vertexDeltaBufferData.resize(vertexBufferSize);
    initializeBufferObject(desc.size, m_outputBuffers[bufferIndex]);
    if (m_bones.empty()) {
        m_bones.resize(numBones);
        for (nanoem_rsize_t i = 0; i < numBones; i++) {
            m_bones[i] = model::Bone::cast(bones[i]);
        }
    }
    if (m_vertexDeltas.empty()) {
        m_vertexDeltas.resize(numVertices);
        glGenBuffers(1, &m_sdefBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_sdefBuffer);
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
        glBufferData(GL_SHADER_STORAGE_BUFFER, bytes.size(), bytes.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
#if defined(SOKOL_DEBUG)
    char markerPrefix[Inline::kMarkerStringLength], markerBuffer[Inline::kMarkerStringLength];
    StringUtils::format(markerPrefix, sizeof(markerPrefix), "Models/%s/Compute", m_model->canonicalNameConstString());
    StringUtils::format(markerBuffer, sizeof(markerBuffer), "%s/InputBuffer", markerPrefix);
    glObjectLabel(GL_BUFFER, m_inputBuffer, -1, markerBuffer);
    StringUtils::format(markerBuffer, sizeof(markerBuffer), "%s/MatrixBuffer", markerPrefix);
    glObjectLabel(GL_BUFFER, m_matrixBuffer, -1, markerBuffer);
    StringUtils::format(markerBuffer, sizeof(markerBuffer), "%s/VertexBuffer", markerPrefix);
    glObjectLabel(GL_BUFFER, m_vertexDeltaBuffer, -1, markerBuffer);
    StringUtils::format(markerBuffer, sizeof(markerBuffer), "%s/SdefBuffer", markerPrefix);
    glObjectLabel(GL_BUFFER, m_sdefBuffer, -1, markerBuffer);
    StringUtils::format(markerBuffer, sizeof(markerBuffer), "%s/Arguments", markerPrefix);
    glObjectLabel(GL_BUFFER, m_argumentBuffer, -1, markerBuffer);
#endif
    sg_buffer_desc d(desc);
    d.gl_buffers[0] = d.gl_buffers[1] = m_outputBuffers[bufferIndex];
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
OpenGLComputeShaderSkinDeformerFactory::Deformer::destroy(sg_buffer value, int bufferIndex) noexcept
{
    sg::destroy_buffer(value);
    GLuint &outputBuffer = m_outputBuffers[bufferIndex];
    glDeleteBuffers(1, &outputBuffer);
    outputBuffer = 0;
}

void
OpenGLComputeShaderSkinDeformerFactory::Deformer::execute(int bufferIndex)
{
    glUseProgram(m_parent->m_program);
    {
        struct Argument {
            nanoem_u32_t m_numVertices;
            nanoem_f32_t m_edgeScaleFactor;
            nanoem_u8_t m_padding[8];
        } args = {};
        args.m_numVertices = Inline::saturateInt32U(m_vertexDeltas.size()),
        args.m_edgeScaleFactor = m_model->edgeSize();
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_argumentBuffer);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(args), &args);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_argumentBuffer);
    }
    {
        BatchUpdateMatrixBufferRunner runner(&m_matrixBufferData, m_bones.data());
        runner.execute(m_bones.size());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_matrixBuffer);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_matrixBufferData.size(), m_matrixBufferData.data());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_matrixBuffer);
    }
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_inputBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_sdefBuffer);
    {
        BatchUpdateVertexDeltaBufferRunner runner(&m_vertexDeltaBufferData, m_vertexDeltas.data());
        runner.execute(m_vertexDeltas.size());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_vertexDeltaBuffer);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_vertexDeltaBufferData.size(), m_vertexDeltaBufferData.data());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_vertexDeltaBuffer);
    }
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_outputBuffers[bufferIndex]);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    const nanoem_u32_t numGroups = (Inline::saturateInt32U(m_vertexDeltas.size()) / 256) + 1;
    glDispatchCompute(numGroups, 1, 1);
    sg::reset_state_cache();
}

void
OpenGLComputeShaderSkinDeformerFactory::Deformer::initializeBufferObject(const void *data, int size, GLuint &object)
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
OpenGLComputeShaderSkinDeformerFactory::Deformer::initializeBufferObject(int size, GLuint &object)
{
    if (!object) {
        glGenBuffers(1, &object);
        glBindBuffer(GL_ARRAY_BUFFER, object);
        glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STREAM_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void
OpenGLComputeShaderSkinDeformerFactory::Deformer::initializeShaderStorageBufferObject(int size, GLuint &object)
{
    if (!object) {
        glGenBuffers(1, &object);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, object);
        glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_STREAM_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
}

} /* namespace glfw */
} /* namespace nanoem */
