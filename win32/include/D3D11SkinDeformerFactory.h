/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/Project.h"
#include "emapp/model/ISkinDeformer.h"

struct ID3D11Buffer;
struct ID3D11ComputeShader;
struct ID3D11Device;
struct ID3D11DeviceChild;
struct ID3D11DeviceContext;
struct ID3D11Resource;
struct ID3D11ShaderResourceView;
struct ID3D11UnorderedAccessView;

namespace nanoem {

namespace model {
class Vertex;
}

namespace win32 {

class Win32ThreadedApplicationService;

class D3D11SkinDeformerFactory : public Project::ISkinDeformerFactory, private NonCopyable {
public:
    D3D11SkinDeformerFactory(ID3D11Device *device, ID3D11DeviceContext *context);
    ~D3D11SkinDeformerFactory() override;

    model::ISkinDeformer *create(Model *model) override;
    void begin() override;
    void commit() override;

private:
    using BoneList = tinystl::vector<model::Bone *, TinySTLAllocator>;
    using MorphList = tinystl::vector<model::Morph *, TinySTLAllocator>;

    class Deformer : public model::ISkinDeformer, private NonCopyable {
    public:
        Deformer(D3D11SkinDeformerFactory *parent, Model *model);
        ~Deformer() override;

        sg_buffer create(const sg_buffer_desc &desc, int bufferIndex) override;
        void rebuildAllBones() override;
        void destroy(sg_buffer value, int bufferIndex) noexcept override;
        void execute(int bufferIndex) override;

    private:
        void setLabel(ID3D11DeviceChild *child, const char *suffix, Error &error);
        void setLabel(ID3D11Resource *resource, const char *suffix, Error &error);
        void createInputBuffer(const sg_buffer_desc &desc, Error &error);
        void createOutputBuffer(const sg_buffer_desc &desc, int bufferIndex, Error &error);
        void updateMatrixBuffer(Error &error);
        void updateMorphWeightBuffer(Error &error);
        void createMatrixBuffer(nanoem_rsize_t numBones, Error &error);
        void createMorphWeightBuffer(nanoem_rsize_t numMorphs, Error &error);
        void createVertexBuffer(Error &error);
        void createSdefBuffer(Error &error);

        D3D11SkinDeformerFactory *m_parent;
        Model *m_model;
        sg_buffer m_buffer = { SG_INVALID_ID };
        ID3D11Buffer *m_inputBuffer = nullptr;
        ID3D11ShaderResourceView *m_inputBufferView = nullptr;
        ID3D11Buffer *m_outputBuffers[2] = { nullptr, nullptr };
        ID3D11UnorderedAccessView *m_outputBuffersView[2] = { nullptr, nullptr };
        ID3D11Buffer *m_matrixBuffer = nullptr;
        ID3D11ShaderResourceView *m_matrixBufferView = nullptr;
        ID3D11Buffer *m_vertexBuffer = nullptr;
        ID3D11ShaderResourceView *m_vertexBufferView = nullptr;
        ID3D11Buffer *m_sdefBuffer = nullptr;
        ID3D11ShaderResourceView *m_sdefBufferView = nullptr;
        ID3D11Buffer *m_morphWeightBuffer = nullptr;
        ID3D11ShaderResourceView *m_morphWeightBufferView = nullptr;
        ID3D11Buffer *m_constantBuffer = nullptr;
        BoneList m_bones;
        MorphList m_morphs;
        nanoem_rsize_t m_numMaxMorphItems = 0;
    };

    ID3D11Device *m_device = nullptr;
    ID3D11DeviceContext *m_context = nullptr;
    ID3D11ComputeShader *m_shader = nullptr;
};

} /* namespace win32 */
} /* namespace nanoem */
