/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "D3D11SkinDeformerFactory.h"

#include <d3d11.h>

#include "COMInline.h"
#include "Win32ThreadedApplicationService.h"

#include "emapp/ICamera.h"
#include "emapp/Model.h"
#include "emapp/StringUtils.h"
#include "emapp/model/Vertex.h"
#include "emapp/private/CommonInclude.h"

#if defined(NANOEM_ENABLE_TBB)
#include "tbb/tbb.h"
#endif /* NANOEM_ENABLE_TBB */

namespace nanoem {
namespace win32 {
namespace {

#include "emapp/private/shaders/model_skinning_cs_dxbc.h"

struct BatchUpdateMatrixBufferRunner {
    BatchUpdateMatrixBufferRunner(ID3D11DeviceContext *context, ID3D11Buffer *matrixBuffer, model::Bone *const *bones)
        : m_context(context)
        , m_matrixBuffer(matrixBuffer)
        , m_bones(bones)
    {
        m_context->AddRef();
        m_matrixBuffer->AddRef();
    }
    ~BatchUpdateMatrixBufferRunner()
    {
        m_matrixBuffer->Release();
        m_context->Release();
    }
    void
    execute(nanoem_rsize_t numBones)
    {
        D3D11_MAPPED_SUBRESOURCE sd = {};
        if (m_context->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &sd) == S_OK) {
            bx::float4x4_t *matrices = static_cast<bx::float4x4_t *>(sd.pData);
#if defined(NANOEM_ENABLE_TBB)
            tbb::parallel_for(tbb::blocked_range<nanoem_rsize_t>(0, numBones),
                [this, matrices](const tbb::blocked_range<nanoem_rsize_t> &range) {
                    for (nanoem_rsize_t i = range.begin(), end = range.end(); i != end; i++) {
                        const model::Bone *bone = m_bones[i];
                        const bx::float4x4_t m = bone->skinningTransformMatrix();
                        memcpy(&matrices[i], &m, sizeof(matrices[0]));
                    }
                });
#else
            for (nanoem_rsize_t i = 0; i < numBones; i++) {
                const model::Bone *bone = m_bones[i];
                const bx::float4x4_t m = bone->skinningTransformMatrix();
                memcpy(&matrices[i], &m, sizeof(matrices[0]));
            }
#endif /* NANOEM_ENABLE_TBB */
            m_context->Unmap(m_matrixBuffer, 0);
        }
    }
    ID3D11DeviceContext *m_context;
    ID3D11Buffer *m_matrixBuffer;
    model::Bone *const *m_bones;
};

struct BatchUpdateMorphWeightBufferRunner {
    BatchUpdateMorphWeightBufferRunner(ID3D11DeviceContext *context, ID3D11Buffer *buffer, model::Morph *const *morphs)
        : m_context(context)
        , m_buffer(buffer)
        , m_morphs(morphs)
    {
        m_context->AddRef();
        m_buffer->AddRef();
    }
    ~BatchUpdateMorphWeightBufferRunner()
    {
        m_buffer->Release();
        m_context->Release();
    }
    void
    execute(nanoem_rsize_t numMorphs)
    {
        D3D11_MAPPED_SUBRESOURCE sd = {};
        if (m_context->Map(m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &sd) == S_OK) {
            auto weights = reinterpret_cast<nanoem_f32_t *>(sd.pData);
#if defined(NANOEM_ENABLE_TBB)
            tbb::parallel_for(tbb::blocked_range<nanoem_rsize_t>(0, numMorphs),
                [this, weights](const tbb::blocked_range<nanoem_rsize_t> &range) {
                    for (nanoem_rsize_t i = range.begin(), end = range.end(); i != end; i++) {
                        const model::Morph *morph = m_morphs[i];
                        weights[i + 1] = morph->weight();
                    }
                });
#else
            for (nanoem_rsize_t i = 0; i < numMorphs; i++) {
                const model::Morph *morph = m_morphs[i];
                weights[i] = morph->weight();
            }
#endif /* NANOEM_ENABLE_TBB */
            m_context->Unmap(m_buffer, 0);
        }
    }
    ID3D11DeviceContext *m_context;
    ID3D11Buffer *m_buffer;
    model::Morph *const *m_morphs;
};

} /* namespace anonymous */

D3D11SkinDeformerFactory::D3D11SkinDeformerFactory(ID3D11Device *device, ID3D11DeviceContext *context)
    : m_device(device)
    , m_context(context)
{
    m_device->AddRef();
    m_context->AddRef();
}

D3D11SkinDeformerFactory::~D3D11SkinDeformerFactory()
{
    COMInline::safeRelease(m_shader);
    m_device->Release();
    m_context->Release();
}

model::ISkinDeformer *
D3D11SkinDeformerFactory::create(Model *model)
{
    if (!m_shader) {
        Error error;
        COMInline::wrapCall(m_device->CreateComputeShader(g_nanoem_model_skinning_ps_dxbc_data,
                                g_nanoem_model_skinning_ps_dxbc_size, nullptr, &m_shader),
            error);
        nanoem_assert(!error.hasReason(), "must not be error occurred");
    }
    return nanoem_new(Deformer(this, model));
}

void
D3D11SkinDeformerFactory::begin()
{
}

void
D3D11SkinDeformerFactory::commit()
{
}

D3D11SkinDeformerFactory::Deformer::Deformer(D3D11SkinDeformerFactory *parent, Model *model)
    : m_parent(parent)
    , m_model(model)
{
}

D3D11SkinDeformerFactory::Deformer::~Deformer()
{
    COMInline::safeRelease(m_inputBuffer);
    COMInline::safeRelease(m_inputBufferView);
    COMInline::safeRelease(m_outputBuffers[0]);
    COMInline::safeRelease(m_outputBuffersView[0]);
    COMInline::safeRelease(m_outputBuffers[1]);
    COMInline::safeRelease(m_outputBuffersView[1]);
    COMInline::safeRelease(m_matrixBuffer);
    COMInline::safeRelease(m_matrixBufferView);
    COMInline::safeRelease(m_vertexBuffer);
    COMInline::safeRelease(m_vertexBufferView);
    COMInline::safeRelease(m_sdefBuffer);
    COMInline::safeRelease(m_sdefBufferView);
}

sg_buffer
D3D11SkinDeformerFactory::Deformer::create(const sg_buffer_desc &desc, int bufferIndex)
{
    Error error;
    if (!m_inputBufferView) {
        createInputBuffer(desc, error);
    }
    if (!m_outputBuffersView[bufferIndex]) {
        createOutputBuffer(desc, bufferIndex, error);
    }
    if (!m_sdefBufferView) {
        createSdefBuffer(error);
    }
    if (!m_vertexBufferView) {
        createVertexBuffer(error);
    }
    sg_buffer_desc d(desc);
    d.d3d11_buffer = (const void *) m_outputBuffers[bufferIndex];
    d.data.ptr = nullptr;
    sg_buffer buffer = m_buffer = sg::make_buffer(&d);
    return buffer;
}

void
D3D11SkinDeformerFactory::Deformer::rebuildAllBones()
{
    m_bones.clear();
}

void
D3D11SkinDeformerFactory::Deformer::destroy(sg_buffer value, int bufferIndex) noexcept
{
    sg::destroy_buffer(value);
    COMInline::safeRelease(m_outputBuffers[bufferIndex]);
    COMInline::safeRelease(m_outputBuffersView[bufferIndex]);
}

void
D3D11SkinDeformerFactory::Deformer::execute(int bufferIndex)
{
    SG_PUSH_GROUP("D3D11SkinDeformerFactory::Deformer::execute");
    Error error;
    nanoem_rsize_t numVertices;
    nanoemModelGetAllVertexObjects(m_model->data(), &numVertices);
    updateMatrixBuffer(error);
    updateMorphWeightBuffer(error);
    {
        struct {
            uint32_t m_numVertices;
            uint32_t m_numMaxMorphItems;
            float m_edgeScaleFactor;
            uint32_t m_padding;
        } data = { Inline::saturateInt32U(numVertices), Inline::saturateInt32U(m_numMaxMorphItems), m_model->edgeSize(),
            0 };
        if (!m_constantBuffer) {
            ID3D11Device *device = m_parent->m_device;
            D3D11_BUFFER_DESC bufferDesc = {};
            bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bufferDesc.ByteWidth = sizeof(data);
            bufferDesc.Usage = D3D11_USAGE_DEFAULT;
            COMInline::wrapCall(device->CreateBuffer(&bufferDesc, nullptr, &m_constantBuffer), error);
        }
        ID3D11DeviceContext *context = m_parent->m_context;
        if (m_constantBuffer) {
            context->UpdateSubresource(m_constantBuffer, 0, nullptr, &data, 0, 0);
        }
    }
    nanoem_assert(!error.hasReason(), "must not be error occurred");
    ID3D11DeviceContext *context = m_parent->m_context;
    context->CSSetShader(m_parent->m_shader, nullptr, 0);
    context->CSSetConstantBuffers(0, 1, &m_constantBuffer);
    context->CSSetShaderResources(0, 1, &m_inputBufferView);
    context->CSSetShaderResources(1, 1, &m_matrixBufferView);
    context->CSSetShaderResources(2, 1, &m_vertexBufferView);
    context->CSSetShaderResources(3, 1, &m_sdefBufferView);
    context->CSSetShaderResources(4, 1, &m_morphWeightBufferView);
    context->CSSetUnorderedAccessViews(0, 1, &m_outputBuffersView[bufferIndex], nullptr);
    UINT numThreads = UINT(numVertices / 256) + 1;
    context->Dispatch(numThreads, 1, 1);
    ID3D11UnorderedAccessView *nullUAV = nullptr;
    context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
    SG_POP_GROUP();
}

void
D3D11SkinDeformerFactory::Deformer::setLabel(ID3D11DeviceChild *child, const char *suffix, Error &error)
{
    nanoem_parameter_assert(child, "must NOT be NULL");
#if defined(SOKOL_DEBUG) && SOKOL_DEBUG
    char buffer[Inline::kLongNameStackBufferSize];
    bx::snprintf(buffer, sizeof(buffer), "Models/%s/Compute/%s", m_model->canonicalNameConstString(), suffix);
    MutableWideString ws;
    StringUtils::getWideCharString(buffer, ws);
    const UINT length = Inline::saturateInt32U(ws.size() * sizeof(ws[0]));
    COMInline::wrapCall(child->SetPrivateData(WKPDID_D3DDebugObjectNameW, length, ws.data()), error);
#else
    BX_UNUSED_3(child, suffix, error);
#endif
}

void
D3D11SkinDeformerFactory::Deformer::setLabel(ID3D11Resource *resource, const char *suffix, Error &error)
{
    nanoem_parameter_assert(resource, "must NOT be NULL");
#if defined(SOKOL_DEBUG) && SOKOL_DEBUG
    char buffer[Inline::kLongNameStackBufferSize];
    bx::snprintf(buffer, sizeof(buffer), "Models/%s/Compute/%s", m_model->canonicalNameConstString(), suffix);
    MutableWideString ws;
    StringUtils::getWideCharString(buffer, ws);
    const UINT length = Inline::saturateInt32U(ws.size() * sizeof(ws[0]));
    COMInline::wrapCall(resource->SetPrivateData(WKPDID_D3DDebugObjectNameW, length, ws.data()), error);
#else
    BX_UNUSED_3(resource, suffix, error);
#endif
}

void
D3D11SkinDeformerFactory::Deformer::createInputBuffer(const sg_buffer_desc &desc, Error &error)
{
    ID3D11Device *device = m_parent->m_device;
    nanoem_rsize_t numItems = desc.size / sizeof(Model::VertexUnit);
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.StructureByteStride = sizeof(Model::VertexUnit);
    bufferDesc.ByteWidth = Inline::saturateInt32U(desc.size);
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    D3D11_SUBRESOURCE_DATA res = {};
    res.pSysMem = desc.data.ptr;
    COMInline::wrapCall(device->CreateBuffer(&bufferDesc, &res, &m_inputBuffer), error);
    if (m_inputBuffer) {
        setLabel(m_inputBuffer, "InputBuffer", error);
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.Buffer.NumElements = Inline::saturateInt32U(numItems);
        COMInline::wrapCall(device->CreateShaderResourceView(m_inputBuffer, &srvDesc, &m_inputBufferView), error);
        if (m_inputBufferView) {
            setLabel(m_inputBufferView, "InputBufferView", error);
        }
    }
}

void
D3D11SkinDeformerFactory::Deformer::createOutputBuffer(const sg_buffer_desc &desc, int bufferIndex, Error &error)
{
    ID3D11Device *device = m_parent->m_device;
    ID3D11Buffer *&buffer = m_outputBuffers[bufferIndex];
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_UNORDERED_ACCESS;
    bufferDesc.ByteWidth = Inline::saturateInt32U(desc.size);
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
    COMInline::wrapCall(device->CreateBuffer(&bufferDesc, nullptr, &buffer), error);
    if (buffer) {
        D3D11_UNORDERED_ACCESS_VIEW_DESC ud = {};
        ud.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        ud.Format = DXGI_FORMAT_R32_TYPELESS;
        ud.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
        ud.Buffer.NumElements = bufferDesc.ByteWidth / 4;
        ID3D11UnorderedAccessView *&bufferView = m_outputBuffersView[bufferIndex];
        COMInline::wrapCall(device->CreateUnorderedAccessView(buffer, &ud, &bufferView), error);
        if (bufferView) {
            setLabel(bufferView, bufferIndex == 0 ? "OutputBufferView/Even" : "OutputBufferView/Odd", error);
        }
    }
}

void
D3D11SkinDeformerFactory::Deformer::updateMatrixBuffer(Error &error)
{
    if (m_bones.empty()) {
        nanoem_rsize_t numBones;
        auto bones = nanoemModelGetAllBoneObjects(m_model->data(), &numBones);
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
        createMatrixBuffer(numBones, error);
    }
    BatchUpdateMatrixBufferRunner runner(m_parent->m_context, m_matrixBuffer, m_bones.data());
    runner.execute(m_bones.size());
}

void
D3D11SkinDeformerFactory::Deformer::updateMorphWeightBuffer(Error &error)
{
    nanoem_rsize_t numMorphs;
    auto morphs = nanoemModelGetAllMorphObjects(m_model->data(), &numMorphs);
    if (m_morphs.empty()) {
        m_morphs.resize(numMorphs);
        for (nanoem_rsize_t i = 0; i < numMorphs; i++) {
            m_morphs[i] = model::Morph::cast(morphs[i]);
        }
        createMorphWeightBuffer(numMorphs, error);
    }
    BatchUpdateMorphWeightBufferRunner runner(m_parent->m_context, m_morphWeightBuffer, m_morphs.data());
    runner.execute(numMorphs);
}

void
D3D11SkinDeformerFactory::Deformer::createMatrixBuffer(nanoem_rsize_t numBones, Error &error)
{
    ID3D11Device *device = m_parent->m_device;
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.StructureByteStride = sizeof(bx::float4x4_t);
    bufferDesc.ByteWidth = Inline::saturateInt32U(numBones * bufferDesc.StructureByteStride);
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    COMInline::wrapCall(device->CreateBuffer(&bufferDesc, nullptr, &m_matrixBuffer), error);
    if (m_matrixBuffer) {
        setLabel(m_matrixBuffer, "MatrixBuffer", error);
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.Buffer.NumElements = Inline::saturateInt32U(numBones);
        COMInline::wrapCall(device->CreateShaderResourceView(m_matrixBuffer, &srvDesc, &m_matrixBufferView), error);
        if (m_matrixBufferView) {
            setLabel(m_matrixBufferView, "MatrixBufferView", error);
        }
    }
}

void
D3D11SkinDeformerFactory::Deformer::createMorphWeightBuffer(nanoem_rsize_t numMorphs, Error &error)
{
    ID3D11Device *device = m_parent->m_device;
    D3D11_BUFFER_DESC bufferDesc = {};
    const UINT numElements = Inline::saturateInt32U(numMorphs + 1);
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.StructureByteStride = sizeof(nanoem_f32_t);
    bufferDesc.ByteWidth = Inline::saturateInt32U(nanoem_rsize_t(numElements) * bufferDesc.StructureByteStride);
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    COMInline::wrapCall(device->CreateBuffer(&bufferDesc, nullptr, &m_morphWeightBuffer), error);
    if (m_morphWeightBuffer) {
        setLabel(m_morphWeightBuffer, "MorphWeightBuffer", error);
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.Buffer.NumElements = numElements;
        COMInline::wrapCall(
            device->CreateShaderResourceView(m_morphWeightBuffer, &srvDesc, &m_morphWeightBufferView), error);
        if (m_morphWeightBufferView) {
            setLabel(m_morphWeightBufferView, "MorphWeightBuffer", error);
        }
    }
}

void
D3D11SkinDeformerFactory::Deformer::createVertexBuffer(Error &error)
{
    typedef tinystl::pair<const nanoem_model_morph_t *, const nanoem_model_morph_vertex_t *> VertexMorphPair;
    typedef tinystl::vector<VertexMorphPair, TinySTLAllocator> MorphPairList;
    tinystl::vector<MorphPairList, TinySTLAllocator> vertex2morphs;
    nanoem_rsize_t numVertices;
    auto vetices = nanoemModelGetAllVertexObjects(m_model->data(), &numVertices);
    {
        nanoem_rsize_t numMorphs;
        auto morphs = nanoemModelGetAllMorphObjects(m_model->data(), &numMorphs);
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
    }
    ID3D11Device *device = m_parent->m_device;
    D3D11_BUFFER_DESC bufferDesc = {};
    const UINT numElements = glm::max(Inline::saturateInt32U(numVertices * m_numMaxMorphItems), 1u);
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.StructureByteStride = sizeof(bx::simd128_t);
    bufferDesc.ByteWidth = Inline::saturateInt32U(nanoem_rsize_t(numElements) * bufferDesc.StructureByteStride);
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    ByteArray bytes(bufferDesc.ByteWidth);
    auto buffers = reinterpret_cast<bx::simd128_t *>(bytes.data());
    size_t offset = 0;
    for (auto it = vertex2morphs.begin(), end = vertex2morphs.end(); it != end; ++it) {
        bx::simd128_t *item = &buffers[offset * m_numMaxMorphItems];
        int i = 0;
        for (auto it2 = it->begin(), end2 = it->end(); it2 != end2; ++it2, ++i) {
            auto position = nanoemModelMorphVertexGetPosition(it2->second);
            /* reserve index zero for non morph weight */
            int index = nanoemModelObjectGetIndex(nanoemModelMorphGetModelObject(it2->first)) + 1;
            item[i] = bx::simd_ld(position[0], position[1], position[2], float(index));
        }
        offset++;
    }
    const D3D11_SUBRESOURCE_DATA res = { bytes.data(), {} };
    COMInline::wrapCall(device->CreateBuffer(&bufferDesc, &res, &m_vertexBuffer), error);
    if (m_vertexBuffer) {
        setLabel(m_vertexBuffer, "VertexBuffer", error);
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.Buffer.NumElements = numElements;
        COMInline::wrapCall(device->CreateShaderResourceView(m_vertexBuffer, &srvDesc, &m_vertexBufferView), error);
        if (m_vertexBufferView) {
            setLabel(m_vertexBufferView, "VertexBufferView", error);
        }
    }
}

void
D3D11SkinDeformerFactory::Deformer::createSdefBuffer(Error &error)
{
    nanoem_rsize_t numVertices;
    auto vertices = nanoemModelGetAllVertexObjects(m_model->data(), &numVertices);
    ID3D11Device *device = m_parent->m_device;
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.StructureByteStride = sizeof(bx::simd128_t) * 3;
    bufferDesc.ByteWidth = Inline::saturateInt32U(numVertices * bufferDesc.StructureByteStride);
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    ByteArray bytes(bufferDesc.ByteWidth);
    bx::simd128_t *sdefPtr = reinterpret_cast<bx::simd128_t *>(bytes.data());
    for (nanoem_rsize_t i = 0; i < numVertices; i++) {
        const nanoem_model_vertex_t *vertex = vertices[i];
        nanoem_rsize_t offset = i * 3;
        memcpy(&sdefPtr[offset + 0], nanoemModelVertexGetSdefC(vertex), sizeof(*sdefPtr));
        memcpy(&sdefPtr[offset + 1], nanoemModelVertexGetSdefR0(vertex), sizeof(*sdefPtr));
        memcpy(&sdefPtr[offset + 2], nanoemModelVertexGetSdefR1(vertex), sizeof(*sdefPtr));
    }
    const D3D11_SUBRESOURCE_DATA res = { bytes.data(), {} };
    COMInline::wrapCall(device->CreateBuffer(&bufferDesc, &res, &m_sdefBuffer), error);
    if (m_sdefBuffer) {
        setLabel(m_sdefBuffer, "SdefBuffer", error);
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.Buffer.NumElements = Inline::saturateInt32U(numVertices);
        COMInline::wrapCall(device->CreateShaderResourceView(m_sdefBuffer, &srvDesc, &m_sdefBufferView), error);
        if (m_sdefBufferView) {
            setLabel(m_sdefBufferView, "SdefBufferView", error);
        }
    }
}

} /* namespace win32 */
} /* namespace nanoem */
