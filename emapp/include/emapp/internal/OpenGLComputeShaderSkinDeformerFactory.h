/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_OPENGLCOMPUTESHADERSKINDEFORMERFACTORY_H_
#define NANOEM_EMAPP_INTERNAL_OPENGLCOMPUTESHADERSKINDEFORMERFACTORY_H_

#include "emapp/Project.h"
#include "emapp/model/ISkinDeformer.h"

namespace nanoem {

class Model;

namespace model {
class Bone;
class Vertex;
} /* namespace model */

namespace internal {

class OpenGLComputeShaderSkinDeformerFactory NANOEM_DECL_SEALED : public Project::ISkinDeformerFactory,
                                                                  private NonCopyable {
public:
    typedef void(APIENTRY *ProcAddress)(void);
    typedef ProcAddress(APIENTRY *PFN_GetProcAddress)(const char *);
    typedef tinystl::vector<model::Bone *, TinySTLAllocator> BoneList;
    typedef tinystl::vector<model::Morph *, TinySTLAllocator> MorphList;

    OpenGLComputeShaderSkinDeformerFactory(PFN_GetProcAddress func);
    ~OpenGLComputeShaderSkinDeformerFactory() NANOEM_DECL_NOEXCEPT_OVERRIDE;

    model::ISkinDeformer *create(Model *model) NANOEM_DECL_OVERRIDE;
    void begin() NANOEM_DECL_OVERRIDE;
    void commit() NANOEM_DECL_OVERRIDE;

private:
    class Deformer : public model::ISkinDeformer, private NonCopyable {
    public:
        Deformer(OpenGLComputeShaderSkinDeformerFactory *parent, Model *model);
        ~Deformer() NANOEM_DECL_NOEXCEPT_OVERRIDE;

        sg_buffer create(const sg_buffer_desc &desc, int bufferIndex) NANOEM_DECL_OVERRIDE;
        void rebuildAllBones() NANOEM_DECL_OVERRIDE;
        void destroy(sg_buffer value, int bufferIndex) NANOEM_DECL_NOEXCEPT_OVERRIDE;
        void execute(int bufferIndex) NANOEM_DECL_OVERRIDE;

    private:
        static void initializeBufferObject(const void *data, nanoem_rsize_t size, nanoem_u32_t &object);
        static void initializeBufferObject(nanoem_rsize_t size, nanoem_u32_t &object);
        static void initializeShaderStorageBufferObject(nanoem_rsize_t size, nanoem_u32_t &object);
        static void initializeShaderStorageBufferObject(const ByteArray &bytes, nanoem_u32_t &object);
        static void updateBufferObject(const ByteArray &bytes, nanoem_u32_t object);
        static void destroyBufferObject(nanoem_u32_t &object) NANOEM_DECL_NOEXCEPT;

        void createInputBuffer(const sg_buffer_desc &desc);
        void createOutputBuffer(const sg_buffer_desc &desc, int bufferIndex);
        void updateArgumentBuffer(nanoem_rsize_t numVertices);
        void updateMatrixBuffer();
        void updateMorphWeightBuffer();
        void createMatrixBuffer();
        void createMorphWeightBuffer();
        void createVertexBuffer();
        void createSdefBuffer();
        void setDebugLabel(nanoem_u32_t object, const char *suffix);

        OpenGLComputeShaderSkinDeformerFactory *m_parent;
        Model *m_model;
        sg_buffer m_buffer;
        BoneList m_bones;
        MorphList m_morphs;
        ByteArray m_matrixBufferData;
        ByteArray m_morphWeightBufferData;
        nanoem_u32_t m_inputBufferObject;
        nanoem_u32_t m_matrixBufferObject;
        nanoem_u32_t m_morphWeightBufferObject;
        nanoem_u32_t m_vertexBufferObject;
        nanoem_u32_t m_sdefBufferObject;
        nanoem_u32_t m_argumentBufferObject;
        nanoem_u32_t m_outputBufferObjects[2];
        nanoem_rsize_t m_numMaxMorphItems;
    };

    nanoem_u32_t m_program = 0;
};

} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_OPENGLCOMPUTESHADERSKINDEFORMERFACTORY_H_ */
