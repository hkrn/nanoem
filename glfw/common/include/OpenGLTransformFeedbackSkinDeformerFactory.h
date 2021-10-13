/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_GLFW_OPENGLTRANSFORMFEEDBACKSKINDEFORMERFACTORY_H_
#define NANOEM_EMAPP_GLFW_OPENGLTRANSFORMFEEDBACKSKINDEFORMERFACTORY_H_

#include "emapp/Project.h"
#include "emapp/model/ISkinDeformer.h"

namespace nanoem {

class Model;

namespace model {
class Bone;
class Vertex;
}

namespace glfw {

class OpenGLTransformFeedbackSkinDeformerFactory : public Project::ISkinDeformerFactory, private NonCopyable {
public:
    typedef void (*ProcAddress)(void);
    typedef ProcAddress (*PFN_GetProcAddress)(const char *);
    typedef tinystl::vector<model::Bone *, TinySTLAllocator> BoneList;
    typedef tinystl::vector<model::Morph *, TinySTLAllocator> MorphList;

    OpenGLTransformFeedbackSkinDeformerFactory(PFN_GetProcAddress func);
    ~OpenGLTransformFeedbackSkinDeformerFactory() override;

    model::ISkinDeformer *create(Model *model) override;
    void begin() override;
    void commit() override;

private:
    class Deformer : public model::ISkinDeformer, private NonCopyable {
    public:
        Deformer(OpenGLTransformFeedbackSkinDeformerFactory *parent, Model *model);
        ~Deformer() override;

        sg_buffer create(const sg_buffer_desc &desc, int bufferIndex) override;
        void rebuildAllBones() override;
        void destroy(sg_buffer value, int bufferIndex) noexcept override;
        void execute(int bufferIndex) override;

    private:
        static nanoem_rsize_t alignBufferSize(nanoem_rsize_t value) NANOEM_DECL_NOEXCEPT;
        static void reserveBufferWithAlignedSize(ByteArray &bytes);
        static void initializeBufferObject(const void *data, nanoem_rsize_t size, nanoem_u32_t &object);
        static void initializeBufferObject(nanoem_rsize_t size, nanoem_u32_t &object);
        static void initializeTextureObject(nanoem_rsize_t size, nanoem_u32_t &object, Vector2SI32 &s);
        static void initializeTextureObject(const ByteArray &bytes, nanoem_u32_t &object, Vector2SI32 &s);
        static void updateTextureObject(const ByteArray &bytes, const Vector2SI32 &s, nanoem_u32_t object);
        static void activateTextureUniform(nanoem_u32_t location, nanoem_u32_t object, int offset);
        static void destroyBufferObject(nanoem_u32_t &object) NANOEM_DECL_NOEXCEPT;
        static void destroyTextureObject(nanoem_u32_t &object) NANOEM_DECL_NOEXCEPT;

        void createInputBuffer(const sg_buffer_desc &desc);
        void createOutputBuffer(const sg_buffer_desc &desc, int bufferIndex);
        void updateMatrixBuffer();
        void updateMorphWeightBuffer();
        void createMatrixBuffer();
        void createMorphWeightBuffer();
        void createVertexBuffer();
        void createSdefBuffer();
        void setDebugLabel(nanoem_u32_t object, nanoem_u32_t type, const char *suffix);

        OpenGLTransformFeedbackSkinDeformerFactory *m_parent;
        Model *m_model;
        nanoem_u32_t m_inputBufferObject;
        nanoem_u32_t m_outputBufferObjects[2];
        nanoem_u32_t m_matrixTextureObject;
        nanoem_u32_t m_morphWeightTextureObject;
        nanoem_u32_t m_sdefTextureObject;
        nanoem_u32_t m_vertexTextureObject;
        sg_buffer m_buffer;
        Vector2SI32 m_matrixTextureSize;
        Vector2SI32 m_morphWeightTextureSize;
        BoneList m_bones;
        MorphList m_morphs;
        ByteArray m_matrixBufferData;
        ByteArray m_morphWeightBufferData;
        nanoem_rsize_t m_numMaxMorphItems;
    };

    nanoem_u32_t m_program;
    nanoem_i32_t m_matrixTextureLocation;
    nanoem_i32_t m_morphWeightTextureLocation;
    nanoem_i32_t m_sdefTextureLocation;
    nanoem_i32_t m_vertexTextureLocation;
    nanoem_i32_t m_argumentUniformLocation;
};

} /* namespace glfw */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_GLFW_OPENGLTRANSFORMFEEDBACKSKINDEFORMERFACTORY_H_ */
