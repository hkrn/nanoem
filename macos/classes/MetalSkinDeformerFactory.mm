/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#import "MetalSkinDeformerFactory.h"
#import "CocoaThreadedApplicationService.h"

#import <Metal/Metal.h>
#import <dispatch/dispatch.h>

#include "emapp/ICamera.h"
#include "emapp/Model.h"
#include "emapp/model/Vertex.h"
#include "emapp/private/CommonInclude.h"

#if defined(NANOEM_ENABLE_TBB)
#include "tbb/tbb.h"
#endif

namespace nanoem {
namespace macos {
namespace {

#include "emapp/private/shaders/model_skinning_cs_msl_macos.h"

struct ConstantData {
    nanoem_u32_t m_numVertices;
    nanoem_u32_t m_numMaxMorphItems;
    nanoem_f32_t m_edgeSize;
    nanoem_u32_t m_padding;
    static inline nanoem_u8_t *
    offset(id<MTLBuffer> buffer) noexcept
    {
        return static_cast<nanoem_u8_t *>(buffer.contents) + sizeof(ConstantData);
    }
};

struct BatchUpdateMatrixBufferRunner {
    static nanoem_rsize_t
    size(nanoem_rsize_t numBones) noexcept
    {
        return numBones * sizeof(bx::float4x4_t);
    }
    BatchUpdateMatrixBufferRunner(id<MTLBuffer> buffer, model::Bone *const *bones)
        : m_uberBuffer(buffer)
        , m_bones(bones)
        , m_matrices(reinterpret_cast<bx::float4x4_t *>(ConstantData::offset(buffer)))
    {
    }
    void
    execute(nanoem_rsize_t numBones, dispatch_queue_t queue)
    {
#ifdef NANOEM_ENABLE_TBB
        BX_UNUSED_1(queue);
        tbb::parallel_for(
            tbb::blocked_range<nanoem_rsize_t>(0, numBones), [this](const tbb::blocked_range<nanoem_rsize_t> &range) {
                for (nanoem_rsize_t i = range.begin(), end = range.end(); i != end; i++) {
                    const model::Bone *bone = m_bones[i];
                    const bx::float4x4_t m = bone->skinningTransformMatrix();
                    memcpy(&m_matrices[i], &m, sizeof(m_matrices[0]));
                }
            });
#else
        dispatch_apply_f(numBones, queue, this, [](void *data, size_t i) {
            auto self = static_cast<BatchUpdateMatrixBufferRunner *>(data);
            const model::Bone *bone = self->m_bones[i];
            const bx::float4x4_t m = bone->skinningTransformMatrix();
            memcpy(&self->m_matrices[i], &m, sizeof(self->m_matrices[0]));
        });
#endif
    }
    id<MTLBuffer> m_uberBuffer;
    model::Bone *const *m_bones;
    bx::float4x4_t *m_matrices;
};

struct BatchUpdateMorphWeightBufferRunner {
    static nanoem_rsize_t
    weightBufferSize(nanoem_rsize_t numMorphs) noexcept
    {
        return glm::max(numMorphs + 1, nanoem_rsize_t(1)) * sizeof(nanoem_f32_t);
    }
    static nanoem_rsize_t
    vertexBufferSize(nanoem_rsize_t numVertices, nanoem_rsize_t numMaxMorphItems) noexcept
    {
        return glm::max(numVertices, nanoem_rsize_t(1)) * glm::max(numMaxMorphItems, nanoem_rsize_t(1)) *
            sizeof(bx::simd128_t);
    }
    BatchUpdateMorphWeightBufferRunner(id<MTLBuffer> buffer, model::Morph *const *morphs, nanoem_rsize_t numBones)
        : m_uberBuffer(buffer)
        , m_morphs(morphs)
        , m_weights(reinterpret_cast<nanoem_f32_t *>(
              ConstantData::offset(buffer) + BatchUpdateMatrixBufferRunner::size(numBones)))
    {
    }
    void
    execute(nanoem_rsize_t numMorphs, dispatch_queue_t queue)
    {
#ifdef NANOEM_ENABLE_TBB
        BX_UNUSED_1(queue);
        tbb::parallel_for(
            tbb::blocked_range<nanoem_rsize_t>(0, numMorphs), [this](const tbb::blocked_range<nanoem_rsize_t> &range) {
                for (nanoem_rsize_t i = range.begin(), end = range.end(); i != end; i++) {
                    const model::Morph *morph = m_morphs[i];
                    m_weights[i + 1] = morph->weight();
                }
            });
#else
        dispatch_apply_f(numVertices, queue, this, [](void *data, size_t i) {
            auto self = static_cast<BatchUpdateVertexBufferRunner *>(data);
            const model::Morph *morph = m_morphs[i];
            m_weights[i] = morph->weight();
        });
#endif
    }
    id<MTLBuffer> m_uberBuffer;
    model::Morph *const *m_morphs;
    nanoem_f32_t *m_weights;
};

} /* namespace anonymous */

MetalSkinDeformerFactory::MetalSkinDeformerFactory(CocoaThreadedApplicationService *service)
    : m_device(service->metalDevice())
    , m_commandQueue(service->metalCommandQueue())
{
}

MetalSkinDeformerFactory::~MetalSkinDeformerFactory() noexcept
{
    m_commandBuffer = nil;
    m_commandQueue = nil;
    m_state = nil;
    m_device = nil;
    if (m_globalDeformerSema) {
        dispatch_semaphore_wait(m_globalDeformerSema, DISPATCH_TIME_FOREVER);
        m_globalDeformerSema = nullptr;
    }
    m_globalDeformerQueue = nullptr;
}

model::ISkinDeformer *
MetalSkinDeformerFactory::create(Model *model)
{
    model::ISkinDeformer *deformer = nullptr;
    if (m_state) {
        deformer = nanoem_new(Deformer(this, model));
    }
    else {
        SG_PUSH_GROUPF("MetalSkinDeformerFactory::create(name=%s)", model->nameConstString());
        NSError *error = nil;
        dispatch_data_t data = dispatch_data_create(g_nanoem_model_skinning_cs_msl_macos_data,
            g_nanoem_model_skinning_cs_msl_macos_size, nullptr, DISPATCH_DATA_DESTRUCTOR_DEFAULT);
        id<MTLLibrary> library = [m_device newLibraryWithData:data error:&error];
        m_function = [library newFunctionWithName:@"nanoemCSMain"];
        id<MTLComputePipelineState> state = [m_device newComputePipelineStateWithFunction:m_function error:&error];
        nanoem_assert(!error, "error must not be occured");
        if (!error && state.maxTotalThreadsPerThreadgroup > 0) {
#ifdef NDEBUG
#define GCD_LABEL(a) nullptr
#else
#define GCD_LABEL(a) a
#endif
            m_globalDeformerQueue = dispatch_queue_create(
                GCD_LABEL("com.github.nanoem.macos.MetalSkinDeformerFactory.m_globalDeformerQueue"), nullptr);
            m_globalDeformerSema = dispatch_semaphore_create(0);
            dispatch_semaphore_signal(m_globalDeformerSema);
#undef GCD_LABEL
            m_state = state;
            deformer = nanoem_new(Deformer(this, model));
        }
        SG_POP_GROUP();
    }
    return deformer;
}

void
MetalSkinDeformerFactory::pushDebugGroup(id<MTLCommandBuffer> commandBuffer, NSString *label)
{
#if defined(NANOEM_ENABLE_DEBUG_LABEL)
    if (@available(macOS 10.13, *)) {
        [commandBuffer pushDebugGroup:label];
    }
#else
    BX_UNUSED_2(commandBuffer, label);
#endif
}

void
MetalSkinDeformerFactory::popDebugGroup(id<MTLCommandBuffer> commandBuffer)
{
#if defined(NANOEM_ENABLE_DEBUG_LABEL)
    if (@available(macOS 10.13, *)) {
        [commandBuffer popDebugGroup];
    }
#else
    BX_UNUSED_1(commandBuffer);
#endif
}

void
MetalSkinDeformerFactory::begin()
{
    SG_PUSH_GROUP("MetalSkinDeformerFactory::begin");
    if (m_globalDeformerSema) {
        dispatch_semaphore_wait(m_globalDeformerSema, DISPATCH_TIME_FOREVER);
        m_commandBuffer = [m_commandQueue commandBuffer];
        pushDebugGroup(m_commandBuffer, @"MetalSkinDeformerFactory::begin");
    }
}

void
MetalSkinDeformerFactory::commit()
{
    if (m_commandBuffer) {
        popDebugGroup(m_commandBuffer);
        [m_commandBuffer addCompletedHandler:^(id<MTLCommandBuffer>) {
            dispatch_semaphore_signal(m_globalDeformerSema);
        }];
        [m_commandBuffer commit];
        m_commandBuffer = nil;
    }
    SG_POP_GROUP();
}

MetalSkinDeformerFactory::Deformer::Deformer(MetalSkinDeformerFactory *parent, Model *model)
    : m_parent(parent)
    , m_model(model)
{
}

MetalSkinDeformerFactory::Deformer::~Deformer() noexcept
{
    m_inputBuffer = nil;
    m_mutableUberBuffer = nil;
    m_immutableUberBuffer = nil;
}

sg_buffer
MetalSkinDeformerFactory::Deformer::create(const sg_buffer_desc &desc, int bufferIndex)
{
    SG_PUSH_GROUPF("MetalSkinDeformerFactory::Deformer::create(bufferIndex=%d)", bufferIndex);
    id<MTLDevice> device = m_parent->m_device;
    id<MTLBuffer> stagingInputBuffer = [device newBufferWithBytes:desc.data.ptr
                                                           length:desc.data.size
                                                          options:MTLResourceStorageModeManaged];
    m_inputBuffer = [device newBufferWithLength:desc.size options:MTLResourceStorageModePrivate];
    setLabel(m_inputBuffer, "InputBuffer");
    id<MTLCommandBuffer> commandBuffer = [m_parent->m_commandQueue commandBuffer];
    id<MTLBlitCommandEncoder> commandEncoder = [commandBuffer blitCommandEncoder];
    [commandEncoder copyFromBuffer:stagingInputBuffer
                      sourceOffset:0
                          toBuffer:m_inputBuffer
                 destinationOffset:0
                              size:desc.size];
    [commandEncoder endEncoding];
    [commandBuffer commit];
    id<MTLBuffer> outputBuffer = m_outputBuffers[bufferIndex] =
        [device newBufferWithLength:desc.size options:MTLResourceStorageModePrivate];
    setLabel(outputBuffer, bufferIndex == 0 ? "OutputBufferEven" : "OutputBufferOdd");
    sg_buffer_desc d(desc);
    d.mtl_buffers[0] = d.mtl_buffers[1] = (__bridge const void *) outputBuffer;
    d.data.ptr = nullptr;
    sg_buffer handle = sg::make_buffer(&d);
    SG_POP_GROUP();
    return handle;
}

void
MetalSkinDeformerFactory::Deformer::rebuildAllBones()
{
    m_bones.clear();
}

void
MetalSkinDeformerFactory::Deformer::destroy(sg_buffer value, int bufferIndex) noexcept
{
    sg::destroy_buffer(value);
    m_outputBuffers[bufferIndex] = nil;
    m_argumentBuffers[bufferIndex] = nil;
}

void
MetalSkinDeformerFactory::Deformer::execute(int bufferIndex)
{
    if (@available(macOS 10.13, *)) {
        SG_PUSH_GROUPF("MetalSkinDeformerFactory::Deformer::execute(bufferIndex=%d)", bufferIndex);
        nanoem_rsize_t numBones, numMorphs, numVertices;
        initializeUberBuffer(numBones, numMorphs, numVertices);
        updateMatrixBuffer(numBones);
        updateMorphBuffer(numBones, numMorphs);
        [m_mutableUberBuffer didModifyRange:NSMakeRange(0, m_mutableUberBuffer.length)];
        initializeArgumentBuffer(numBones, numVertices, bufferIndex);
        id<MTLComputePipelineState> state = m_parent->m_state;
        const MTLSize threadsPerGroup = MTLSizeMake(state.maxTotalThreadsPerThreadgroup, 1, 1),
                      numThreads = MTLSizeMake((numVertices / threadsPerGroup.width) + 1, 1, 1);
        id<MTLCommandBuffer> commandBuffer = m_parent->m_commandBuffer;
        NSString *label = [[NSString alloc] initWithFormat:@"MetalSkinDeformerFactory::Deformer::execute(name=%@)",
                                            [[NSString alloc] initWithUTF8String:m_model->canonicalNameConstString()]];
        pushDebugGroup(commandBuffer, label);
        id<MTLComputeCommandEncoder> commandEncoder = [commandBuffer computeCommandEncoder];
        [commandEncoder useResource:m_inputBuffer usage:MTLResourceUsageRead];
        [commandEncoder useResource:m_immutableUberBuffer usage:MTLResourceUsageRead];
        [commandEncoder useResource:m_mutableUberBuffer usage:MTLResourceUsageRead];
        [commandEncoder useResource:m_outputBuffers[bufferIndex] usage:MTLResourceUsageWrite];
        [commandEncoder setBuffer:m_argumentBuffers[bufferIndex] offset:0 atIndex:0];
        [commandEncoder setComputePipelineState:state];
        [commandEncoder dispatchThreadgroups:numThreads threadsPerThreadgroup:threadsPerGroup];
        [commandEncoder endEncoding];
        popDebugGroup(commandBuffer);
        SG_POP_GROUP();
    }
}

void
MetalSkinDeformerFactory::Deformer::setLabel(id<MTLResource> resource, const char *suffix)
{
#if defined(NANOEM_ENABLE_DEBUG_LABEL)
    NSString *name = [[NSString alloc] initWithUTF8String:m_model->canonicalNameConstString()];
    resource.label = [[NSString alloc] initWithFormat:@"Models/%@/Compute/%s", name, suffix];
#else
    BX_UNUSED_2(resource, suffix);
#endif
}

void
MetalSkinDeformerFactory::Deformer::initializeArgumentBuffer(
    nanoem_rsize_t numBones, nanoem_rsize_t numVertices, int bufferIndex)
{
    if (@available(macOS 10.13, *)) {
        id<MTLBuffer> argumentBuffer = m_argumentBuffers[bufferIndex];
        if (!argumentBuffer) {
            const nanoem_rsize_t constantDataOffset = 0, matrixBufferOffset = sizeof(ConstantData),
                                 morphWeightBufferOffset =
                                     matrixBufferOffset + BatchUpdateMatrixBufferRunner::size(numBones),
                                 vertexBufferOffset = 0,
                                 sdefOffset = vertexBufferOffset +
                BatchUpdateMorphWeightBufferRunner::vertexBufferSize(numVertices, m_numMaxMorphItems);
            id<MTLArgumentEncoder> argumentEncoder = [m_parent->m_function newArgumentEncoderWithBufferIndex:0];
            const NSUInteger encodedLength = argumentEncoder.encodedLength;
            argumentBuffer = [m_parent->m_device newBufferWithLength:encodedLength options:0];
            setLabel(argumentBuffer, bufferIndex == 0 ? "ArgumentBufferEven" : "ArgumentBufferOdd");
            [argumentEncoder setArgumentBuffer:argumentBuffer offset:0];
            /* same as SPIRV generated order */
            [argumentEncoder setBuffer:m_mutableUberBuffer offset:matrixBufferOffset atIndex:0];
            [argumentEncoder setBuffer:m_mutableUberBuffer offset:constantDataOffset atIndex:1];
            [argumentEncoder setBuffer:m_inputBuffer offset:0 atIndex:2];
            [argumentEncoder setBuffer:m_immutableUberBuffer offset:sdefOffset atIndex:3];
            [argumentEncoder setBuffer:m_immutableUberBuffer offset:0 atIndex:4];
            [argumentEncoder setBuffer:m_mutableUberBuffer offset:morphWeightBufferOffset atIndex:5];
            [argumentEncoder setBuffer:m_outputBuffers[bufferIndex] offset:0 atIndex:6];
            m_argumentBuffers[bufferIndex] = argumentBuffer;
        }
    }
}

void
MetalSkinDeformerFactory::Deformer::initializeUberBuffer(
    nanoem_rsize_t &numBones, nanoem_rsize_t &numMorphs, nanoem_rsize_t &numVertices)
{
    nanoemModelGetAllBoneObjects(m_model->data(), &numBones);
    numBones = glm::max(numBones, nanoem_rsize_t(1));
    auto vertices = nanoemModelGetAllVertexObjects(m_model->data(), &numVertices);
    auto morphs = nanoemModelGetAllMorphObjects(m_model->data(), &numMorphs);
    const nanoem_rsize_t matrixBufferLength = BatchUpdateMatrixBufferRunner::size(numBones),
                         morphWeightBufferLength = BatchUpdateMorphWeightBufferRunner::weightBufferSize(numMorphs);
    ConstantData constantData = {
        Inline::saturateInt32U(numVertices),
        Inline::saturateInt32U(m_numMaxMorphItems),
        m_model->edgeSize(),
        0,
    };
    if (!m_immutableUberBuffer) {
        typedef tinystl::pair<const nanoem_model_morph_t *, const nanoem_model_morph_vertex_t *> VertexMorphPair;
        typedef tinystl::vector<VertexMorphPair, TinySTLAllocator> MorphPairList;
        tinystl::vector<MorphPairList, TinySTLAllocator> vertex2morphs;
        {
            vertex2morphs.resize(numVertices);
            for (nanoem_rsize_t i = 0; i < numMorphs; i++) {
                const nanoem_model_morph_t *morphPtr = morphs[i];
                switch (nanoemModelMorphGetType(morphPtr)) {
                case NANOEM_MODEL_MORPH_TYPE_VERTEX: {
                    nanoem_rsize_t numItems;
                    auto items = nanoemModelMorphGetAllVertexMorphObjects(morphPtr, &numItems);
                    for (nanoem_rsize_t j = 0; j < numItems; j++) {
                        auto itemPtr = items[j];
                        auto vertexPtr = nanoemModelMorphVertexGetVertexObject(itemPtr);
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
            for (auto it = vertex2morphs.begin(), end = vertex2morphs.end(); it != end; ++it) {
                m_numMaxMorphItems = glm::max(m_numMaxMorphItems, it->size());
            }
            constantData.m_numMaxMorphItems = Inline::saturateInt32U(m_numMaxMorphItems);
        }
        const nanoem_rsize_t vertexBufferLength = numVertices * m_numMaxMorphItems * sizeof(bx::simd128_t),
                             sdefBufferLength = glm::max(numVertices, nanoem_rsize_t(1)) * sizeof(bx::simd128_t) * 3;
        nanoem_rsize_t bufferOffset = 0;
        ByteArray bytes(vertexBufferLength + sdefBufferLength);
        {
            bx::simd128_t *buffers = reinterpret_cast<bx::simd128_t *>(bytes.data());
            size_t offset = 0;
            for (auto it = vertex2morphs.begin(), end = vertex2morphs.end(); it != end; ++it) {
                bx::simd128_t *item = &buffers[offset * m_numMaxMorphItems];
                int i = 0;
                for (auto it2 = it->begin(), end2 = it->end(); it2 != end2; ++it2, ++i) {
                    auto position = nanoemModelMorphVertexGetPosition(it2->second);
                    /* reserve index zero for non morph weight */
                    int index = nanoemModelObjectGetIndex(nanoemModelMorphGetModelObject(it2->first)) + 1;
                    item[i] = bx::simd_ld(position[0], position[1], position[2], index);
                }
                offset++;
            }
            bufferOffset += vertexBufferLength;
        }
        {
            auto sdefPtr = reinterpret_cast<bx::simd128_t *>(bytes.data() + bufferOffset);
            for (nanoem_rsize_t i = 0; i < numVertices; i++) {
                const nanoem_model_vertex_t *vertexPtr = vertices[i];
                nanoem_rsize_t offset = i * 3;
                memcpy(&sdefPtr[offset + 0], nanoemModelVertexGetSdefC(vertexPtr), sizeof(*sdefPtr));
                memcpy(&sdefPtr[offset + 1], nanoemModelVertexGetSdefR0(vertexPtr), sizeof(*sdefPtr));
                memcpy(&sdefPtr[offset + 2], nanoemModelVertexGetSdefR1(vertexPtr), sizeof(*sdefPtr));
            }
        }
        id<MTLDevice> device = m_parent->m_device;
        id<MTLBuffer> stagingImmutableUbferBuffer = [device newBufferWithBytes:bytes.data()
                                                                        length:bytes.size()
                                                                       options:MTLResourceStorageModeManaged];
        m_immutableUberBuffer = [device newBufferWithLength:bytes.size() options:MTLResourceStorageModePrivate];
        setLabel(m_immutableUberBuffer, "ImmutableUberBuffer");
        id<MTLCommandBuffer> commandBuffer = m_parent->m_commandBuffer;
        id<MTLBlitCommandEncoder> commandEncoder = [commandBuffer blitCommandEncoder];
        [commandEncoder copyFromBuffer:stagingImmutableUbferBuffer
                          sourceOffset:0
                              toBuffer:m_immutableUberBuffer
                     destinationOffset:0
                                  size:bytes.size()];
        [commandEncoder endEncoding];
#if defined(NANOEM_ENABLE_DEBUG_LABEL)
        NSString *name = [[NSString alloc] initWithUTF8String:m_model->canonicalNameConstString()],
                 *prefix = [[NSString alloc] initWithFormat:@"Models/%@/Compute/ImmutableUberBuffer", name],
                 *vertexBufferName = [[NSString alloc] initWithFormat:@"%@/Vertices", prefix],
                 *sdefBufferName = [[NSString alloc] initWithFormat:@"%@/SDEF", prefix];
        bufferOffset = 0;
        [m_immutableUberBuffer addDebugMarker:vertexBufferName range:NSMakeRange(bufferOffset, vertexBufferLength)];
        bufferOffset += vertexBufferLength;
        [m_immutableUberBuffer addDebugMarker:sdefBufferName range:NSMakeRange(bufferOffset, sdefBufferLength)];
#endif
    }
    if (!m_mutableUberBuffer) {
        const nanoem_rsize_t length = sizeof(constantData) + matrixBufferLength + morphWeightBufferLength;
        m_mutableUberBuffer = [m_parent->m_device newBufferWithLength:length options:MTLResourceStorageModeManaged];
        setLabel(m_mutableUberBuffer, "MutableUberBuffer");
#if defined(NANOEM_ENABLE_DEBUG_LABEL)
        NSString *name = [[NSString alloc] initWithUTF8String:m_model->canonicalNameConstString()],
                 *prefix = [[NSString alloc] initWithFormat:@"Models/%@/Compute/MutableUberBuffer", name],
                 *constantBufferName = [[NSString alloc] initWithFormat:@"%@/Constant", prefix],
                 *morphWeightBufferName = [[NSString alloc] initWithFormat:@"%@/MorphWeights", prefix],
                 *matrixBufferName = [[NSString alloc] initWithFormat:@"%@/Matrices", prefix];
        nanoem_rsize_t bufferOffset = 0;
        [m_mutableUberBuffer addDebugMarker:constantBufferName range:NSMakeRange(bufferOffset, sizeof(constantData))];
        bufferOffset += sizeof(constantData);
        [m_mutableUberBuffer addDebugMarker:matrixBufferName range:NSMakeRange(bufferOffset, matrixBufferLength)];
        bufferOffset += matrixBufferLength;
        [m_mutableUberBuffer addDebugMarker:morphWeightBufferName
                                      range:NSMakeRange(bufferOffset, morphWeightBufferLength)];
#endif
    }
    memcpy(m_mutableUberBuffer.contents, &constantData, sizeof(constantData));
}

void
MetalSkinDeformerFactory::Deformer::updateMatrixBuffer(nanoem_rsize_t numBones)
{
    if (m_bones.empty()) {
        nanoem_rsize_t numObjects;
        nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_model->data(), &numObjects);
        if (numObjects > 0) {
            m_bones.resize(numObjects);
            for (nanoem_rsize_t i = 0; i < numObjects; i++) {
                m_bones[i] = model::Bone::cast(bones[i]);
            }
        }
        else {
            m_bones.push_back(m_model->sharedFallbackBone());
        }
    }
    BatchUpdateMatrixBufferRunner runner(m_mutableUberBuffer, m_bones.data());
    runner.execute(numBones, m_parent->m_globalDeformerQueue);
}

void
MetalSkinDeformerFactory::Deformer::updateMorphBuffer(nanoem_rsize_t numBones, nanoem_rsize_t numMorphs)
{
    if (m_morphs.empty()) {
        nanoem_rsize_t numObjects;
        auto morphs = nanoemModelGetAllMorphObjects(m_model->data(), &numObjects);
        if (numObjects > 0) {
            m_morphs.resize(numObjects);
            for (nanoem_rsize_t i = 0; i < numObjects; i++) {
                m_morphs[i] = model::Morph::cast(morphs[i]);
            }
        }
    }
    BatchUpdateMorphWeightBufferRunner runner(m_mutableUberBuffer, m_morphs.data(), numBones);
    runner.execute(numMorphs, m_parent->m_globalDeformerQueue);
}

} /* namespace macos */
} /* namespace nanoem */
