/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/Project.h"
#include "emapp/model/ISkinDeformer.h"

#include <dispatch/dispatch.h>

@protocol MTLBuffer;
@protocol MTLCommandBuffer;
@protocol MTLCommandQueue;
@protocol MTLComputePipelineState;
@protocol MTLDevice;
@protocol MTLFunction;
@protocol MTLResource;

namespace nanoem {

namespace model {
class Vertex;
}

namespace macos {

class CocoaThreadedApplicationService;

class MetalSkinDeformerFactory NANOEM_DECL_SEALED : public Project::ISkinDeformerFactory, private NonCopyable {
public:
    using BoneList = tinystl::vector<model::Bone *, TinySTLAllocator>;
    using MorphList = tinystl::vector<model::Morph *, TinySTLAllocator>;

    MetalSkinDeformerFactory(CocoaThreadedApplicationService *service);
    ~MetalSkinDeformerFactory() noexcept override;

    model::ISkinDeformer *create(Model *model) override;
    void begin() override;
    void commit() override;

private:
    static void pushDebugGroup(id<MTLCommandBuffer> commandBuffer, NSString *label);
    static void popDebugGroup(id<MTLCommandBuffer> commandBuffer);

    class Deformer NANOEM_DECL_SEALED : public model::ISkinDeformer, private NonCopyable {
    public:
        Deformer(MetalSkinDeformerFactory *parent, Model *model);
        ~Deformer() noexcept override;

        sg_buffer create(const sg_buffer_desc &desc, int bufferIndex) override;
        void rebuildAllBones() override;
        void destroy(sg_buffer value, int bufferIndex) noexcept override;
        void execute(int bufferIndex) override;

    private:
        void setLabel(id<MTLResource> resource, const char *suffix);
        void initializeArgumentBuffer(nanoem_rsize_t numBones, nanoem_rsize_t numVertices, int bufferIndex);
        void initializeUberBuffer(nanoem_rsize_t &numBones, nanoem_rsize_t &numMorphs, nanoem_rsize_t &numVertices);
        void updateMatrixBuffer(nanoem_rsize_t numBones);
        void updateMorphBuffer(nanoem_rsize_t numBones, nanoem_rsize_t numMorphs);

        MetalSkinDeformerFactory *m_parent;
        Model *m_model;
        id<MTLBuffer> m_argumentBuffers[2] = { nil, nil };
        id<MTLBuffer> m_outputBuffers[2] = { nil, nil };
        id<MTLBuffer> m_inputBuffer = nil;
        id<MTLBuffer> m_mutableUberBuffer = nil;
        id<MTLBuffer> m_immutableUberBuffer = nil;
        BoneList m_bones;
        MorphList m_morphs;
        nanoem_rsize_t m_numMaxMorphItems = 0;
    };

    id<MTLDevice> m_device = nil;
    id<MTLCommandQueue> m_commandQueue = nil;
    id<MTLCommandBuffer> m_commandBuffer = nil;
    id<MTLFunction> m_function = nil;
    id<MTLComputePipelineState> m_state = nil;
    dispatch_semaphore_t m_globalDeformerSema = nullptr;
    dispatch_queue_t m_globalDeformerQueue = nullptr;
};

} /* namespace macos */
} /* namespace nanoem */
