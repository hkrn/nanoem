/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_GLFW_OPENGLCOMPUTESHADERSKINDEFORMERFACTORY_H_
#define NANOEM_EMAPP_GLFW_OPENGLCOMPUTESHADERSKINDEFORMERFACTORY_H_

#include "emapp/Project.h"
#include "emapp/model/ISkinDeformer.h"

namespace nanoem {

class Model;

namespace model {
class Bone;
class Vertex;
}

namespace glfw {

class OpenGLComputeShaderSkinDeformerFactory : public Project::ISkinDeformerFactory, private NonCopyable {
public:
    using ProcAddress = void (*)(void);
    using PFN_GetProcAddress = ProcAddress (*)(const char *);
    using BoneList = tinystl::vector<model::Bone *, TinySTLAllocator>;
    using MorphList = tinystl::vector<model::Morph *, TinySTLAllocator>;

    OpenGLComputeShaderSkinDeformerFactory(PFN_GetProcAddress func);
    ~OpenGLComputeShaderSkinDeformerFactory() noexcept override;

    model::ISkinDeformer *create(Model *model) override;
    void begin() override;
    void commit() override;

private:
    class Deformer : public model::ISkinDeformer, private NonCopyable {
    public:
        Deformer(OpenGLComputeShaderSkinDeformerFactory *parent, Model *model);
        ~Deformer() noexcept override;

        sg_buffer create(const sg_buffer_desc &desc, int bufferIndex) override;
        void rebuildAllBones() override;
        void destroy(sg_buffer value, int bufferIndex) noexcept override;
        void execute(int bufferIndex) override;

    private:
        static void initializeBufferObject(const void *data, int size, nanoem_u32_t &object);
        static void initializeBufferObject(int size, nanoem_u32_t &object);
        static void initializeShaderStorageBufferObject(int size, nanoem_u32_t &object);
        static void initializeShaderStorageBufferObject(const ByteArray &bytes, nanoem_u32_t &object);
        static void updateBufferObject(const ByteArray &bytes, nanoem_u32_t object);
        static void destroyBufferObject(nanoem_u32_t &object) noexcept;

        void createInputBuffer(const sg_buffer_desc &desc, Error &error);
        void createOutputBuffer(const sg_buffer_desc &desc, int bufferIndex, Error &error);
        void updateMatrixBuffer(Error &error);
        void updateMorphWeightBuffer(Error &error);
        void createMatrixBuffer(Error &error);
        void createMorphWeightBuffer(Error &error);
        void createVertexBuffer(Error &error);
        void createSdefBuffer(Error &error);
        void setDebugLabel(nanoem_u32_t object, const char *suffix);

        OpenGLComputeShaderSkinDeformerFactory *m_parent;
        Model *m_model;
        sg_buffer m_buffer = { SG_INVALID_ID };
        BoneList m_bones;
        MorphList m_morphs;
        ByteArray m_matrixBufferData;
        ByteArray m_morphWeightBufferData;
        nanoem_u32_t m_inputBufferObject = 0;
        nanoem_u32_t m_matrixBufferObject = 0;
        nanoem_u32_t m_morphWeightBufferObject = 0;
        nanoem_u32_t m_vertexBufferObject = 0;
        nanoem_u32_t m_sdefBufferObject = 0;
        nanoem_u32_t m_argumentBufferObject = 0;
        nanoem_u32_t m_outputBufferObjects[2] = { 0, 0 };
        nanoem_rsize_t m_numMaxMorphItems = 0;
    };

    nanoem_u32_t m_program = 0;
};

} /* namespace glfw */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_GLFW_OPENGLCOMPUTESHADERSKINDEFORMERFACTORY_H_ */
