/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_GLFW_OPENGLTRANSFORMFEEDBACKSKINDEFORMERFACTORY_H_
#define NANOEM_EMAPP_GLFW_OPENGLTRANSFORMFEEDBACKSKINDEFORMERFACTORY_H_

#include "GL/gl3w.h"

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
    using BoneList = tinystl::vector<model::Bone *, TinySTLAllocator>;
    using VertexList = tinystl::vector<model::Vertex *, TinySTLAllocator>;

    OpenGLTransformFeedbackSkinDeformerFactory();
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
        static void initializeBufferObject(const void *data, int size, GLuint &object);
        static void initializeBufferObject(int size, GLuint &object);
        static void initializeTextureObject(int size, GLuint &object, Vector2SI32 &s);

        OpenGLTransformFeedbackSkinDeformerFactory *m_parent;
        Model *m_model;
        GLuint m_inputBuffer = 0;
        GLuint m_matrixTexture = 0;
        GLuint m_vertexDeltaTexture = 0;
        GLuint m_sdefTexture = 0;
        GLuint m_outputBuffers[2] = { 0, 0 };
        sg_buffer m_buffer = { SG_INVALID_ID };
        Vector2SI32 m_matrixTextureSize = Vector2SI32(0);
        Vector2SI32 m_vertexDeltaTextureSize = Vector2SI32(0);
        BoneList m_bones;
        VertexList m_vertexDeltas;
        ByteArray m_matrixTextureData;
        ByteArray m_vertexDeltaTextureData;
    };

    GLuint m_program = 0;
    GLint m_matrixTextureLocation = -1;
    GLint m_vertexTextureLocation = -1;
    GLint m_sdefTextureLocation = -1;
    GLint m_edgeScaleFactorLocation = -1;
};

} /* namespace glfw */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_GLFW_OPENGLTRANSFORMFEEDBACKSKINDEFORMERFACTORY_H_ */
