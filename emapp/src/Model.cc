/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/Model.h"

#include "emapp/Archiver.h"
#include "emapp/Color.h"
#include "emapp/Effect.h"
#include "emapp/EnumUtils.h"
#include "emapp/Error.h"
#include "emapp/FileUtils.h"
#include "emapp/ICamera.h"
#include "emapp/IEventPublisher.h"
#include "emapp/IFileManager.h"
#include "emapp/ILight.h"
#include "emapp/IPrimitive2D.h"
#include "emapp/ImageLoader.h"
#include "emapp/ListUtils.h"
#include "emapp/ModelProgramBundle.h"
#include "emapp/Motion.h"
#include "emapp/PerspectiveCamera.h"
#include "emapp/PhysicsEngine.h"
#include "emapp/Progress.h"
#include "emapp/Project.h"
#include "emapp/ResourceBundle.h"
#include "emapp/StringUtils.h"
#include "emapp/command/TransformBoneCommand.h"
#include "emapp/command/TransformMorphCommand.h"
#include "emapp/internal/LineDrawer.h"
#include "emapp/internal/ModelObjectSelection.h"
#include "emapp/model/BindPose.h"
#include "emapp/model/Exporter.h"
#include "emapp/model/ISkinDeformer.h"
#include "emapp/model/Importer.h"
#include "emapp/model/Joint.h"
#include "emapp/model/Label.h"
#include "emapp/model/RigidBody.h"
#include "emapp/model/SoftBody.h"
#include "emapp/model/Vertex.h"
#include "emapp/private/CommonInclude.h"

#include "glm/gtx/dual_quaternion.hpp"
#include "glm/gtx/matrix_interpolation.hpp"
#include "glm/gtx/quaternion.hpp"

#include "./CommandMessage.inl"
#include "nanoem/ext/converter.h"
#include "sokol/sokol_time.h"
#include "undo/undo.h"

#include "emapp/internal/DebugDrawer.h"

#include <glm/gtc/matrix_inverse.hpp>

#if defined(NANOEM_ENABLE_TBB)
#include "tbb/tbb.h"
#endif /* NANOEM_ENABLE_TBB */
#if defined(__APPLE__)
#include <dispatch/dispatch.h>
#endif /* __APPLE__ */

namespace nanoem {
namespace {

const int kMaxBoneUniforms = 55;

enum PrivateStateFlags {
    kPrivateStateVisible = 1 << 1,
    kPrivateStateUploaded = 1 << 2,
    kPrivateStateVertexShaderSkinning = 1 << 3,
    kPrivateStateShowAllBones = 1 << 4,
    kPrivateStateShowAllVertexPoints = 1 << 5,
    kPrivateStateShowAllVertexFaces = 1 << 6,
    kPrivateStateShowAllRigidBodies = 1 << 7,
    kPrivateStateShowAllRigidBodiesColorByShape = 1 << 8,
    kPrivateStateComputeShaderSkinning = 1 << 9,
    kPrivateStateMorphWeightFocus = 1 << 10,
    kPrivateStateEnableShadow = 1 << 11,
    kPrivateStateEnableShadowMap = 1 << 12,
    kPrivateStateEnableAddBlend = 1 << 13,
    kPrivateStateShowAllJoints = 1 << 14,
    kPrivateStateDirty = 1 << 15,
    kPrivateStateDirtyStagingBuffer = 1 << 16,
    kPrivateStateDirtyMorph = 1 << 17,
    kPrivateStatePhysicsSimulation = 1 << 18,
    kPrivateStateEnableGroundShadow = 1 << 19,
    kPrivateStateReserved = 1 << 31,
};
static const nanoem_u32_t kPrivateStateInitialValue = kPrivateStatePhysicsSimulation | kPrivateStateEnableGroundShadow;

} /* namespace anonymous */

const Matrix4x4 Model::kInitialWorldMatrix = Constants::kIdentity;

struct Model::LoadingImageItem {
    LoadingImageItem(const URI &fileURI, const String &filename, sg_wrap wrap, nanoem_u32_t flags)
        : m_fileURI(fileURI)
        , m_filename(filename)
        , m_wrap(wrap)
        , m_flags(flags)
    {
    }
    URI m_fileURI;
    String m_filename;
    sg_wrap m_wrap;
    nanoem_u32_t m_flags;
};

Model::VertexUnit::VertexUnit() NANOEM_DECL_NOEXCEPT : m_position(bx::simd_zero()),
                                                       m_normal(bx::simd_zero()),
                                                       m_texcoord(bx::simd_zero()),
                                                       m_edge(bx::simd_zero()),
                                                       m_weights(bx::simd_zero()),
                                                       m_indices(bx::simd_zero()),
                                                       m_info(bx::simd_zero())
{
    m_uva[0] = bx::simd_zero();
    m_uva[1] = bx::simd_zero();
    m_uva[2] = bx::simd_zero();
    m_uva[3] = bx::simd_zero();
}

Model::VertexUnit::~VertexUnit() NANOEM_DECL_NOEXCEPT
{
}

void
Model::VertexUnit::setUVA(const model::Vertex *vertex) NANOEM_DECL_NOEXCEPT
{
    m_uva[0] = bx::simd_add(vertex->m_simd.m_originUVA[0], vertex->m_simd.m_deltaUVA[1]);
    m_uva[1] = bx::simd_add(vertex->m_simd.m_originUVA[1], vertex->m_simd.m_deltaUVA[2]);
    m_uva[2] = bx::simd_add(vertex->m_simd.m_originUVA[2], vertex->m_simd.m_deltaUVA[3]);
    m_uva[3] = bx::simd_add(vertex->m_simd.m_originUVA[3], vertex->m_simd.m_deltaUVA[4]);
}

void
Model::VertexUnit::performSkinning(nanoem_f32_t edgeSizeFactor, const model::Vertex *vertex) NANOEM_DECL_NOEXCEPT
{
    if (!vertex->hasSoftBody()) {
        performSkinningByType(vertex, &m_position, &m_normal);
    }
    m_edge = bx::simd_madd(m_normal, bx::simd_splat(bx::simd_x(vertex->m_simd.m_info) * edgeSizeFactor), m_position);
    m_texcoord = bx::simd_add(vertex->m_simd.m_texcoord, vertex->m_simd.m_deltaUVA[0]);
    setUVA(vertex);
}

void
Model::VertexUnit::setWeightColor(const model::Bone *bone, const model::Vertex *vertex) NANOEM_DECL_NOEXCEPT
{
    performSkinningByType(vertex, &m_position, &m_normal);
    Vector3 color(Constants::kZeroV3);
    for (nanoem_rsize_t i = 0; i < 4; i++) {
        const model::Bone *item = vertex->bone(i);
        if (item && item == bone) {
            nanoem_f32_t component = 0;
            switch (i) {
            case 0:
                component = bx::simd_x(vertex->m_simd.m_weights);
                break;
            case 1:
                component = bx::simd_y(vertex->m_simd.m_weights);
                break;
            case 2:
                component = bx::simd_z(vertex->m_simd.m_weights);
                break;
            case 3:
                component = bx::simd_w(vertex->m_simd.m_weights);
                break;
            }
            color = Color::jet(component);
            break;
        }
    }
    m_info = bx::simd_ld(glm::value_ptr(Vector4(color, 1)));
    setUVA(vertex);
}

void
Model::VertexUnit::prepareSkinning(
    const model::Material::BoneIndexHashMap *indexHashMap, const model::Vertex *vertex) NANOEM_DECL_NOEXCEPT
{
    m_position = bx::simd_add(m_position, vertex->m_simd.m_delta);
    if (indexHashMap) {
        model::Material::BoneIndexHashMap::const_iterator it = indexHashMap->find(vertex->material());
        if (it != indexHashMap->end() && !vertex->isSkinningEnabled()) {
            const Int32HashMap &hashMap = it->second;
            nanoem_f32_t values[4];
            for (int i = 0; i < 4; i++) {
                int vertexBoneIndex = nanoemModelObjectGetIndex(
                    nanoemModelBoneGetModelObject(nanoemModelVertexGetBoneObject(vertex->data(), i)));
                Int32HashMap::const_iterator it2 = hashMap.find(vertexBoneIndex);
                values[i] = nanoem_f32_t(it2 != hashMap.end() ? it2->second : -1);
            }
            m_indices = bx::simd_ld(values);
        }
        else {
            performSkinningByType(vertex, &m_position, &m_normal);
        }
    }
    else {
        m_indices = vertex->m_simd.m_indices;
    }
    m_texcoord = bx::simd_add(m_texcoord, vertex->m_simd.m_deltaUVA[0]);
    m_uva[0] = bx::simd_add(vertex->m_simd.m_originUVA[0], vertex->m_simd.m_deltaUVA[1]);
    m_uva[1] = bx::simd_add(vertex->m_simd.m_originUVA[1], vertex->m_simd.m_deltaUVA[2]);
    m_uva[2] = bx::simd_add(vertex->m_simd.m_originUVA[2], vertex->m_simd.m_deltaUVA[3]);
    m_uva[3] = bx::simd_add(vertex->m_simd.m_originUVA[3], vertex->m_simd.m_deltaUVA[4]);
    m_weights = vertex->m_simd.m_weights;
    m_info = vertex->m_simd.m_info;
    setUVA(vertex);
}

bx::simd128_t
Model::VertexUnit::swizzleWeight(const model::Vertex *vertex, nanoem_rsize_t index) NANOEM_DECL_NOEXCEPT
{
    bx::simd128_t weight;
    switch (index) {
    case 0:
        weight = bx::simd_swiz_xxxx(vertex->m_simd.m_weights);
        break;
    case 1:
        weight = bx::simd_swiz_yyyy(vertex->m_simd.m_weights);
        break;
    case 2:
        weight = bx::simd_swiz_zzzz(vertex->m_simd.m_weights);
        break;
    case 3:
        weight = bx::simd_swiz_wwww(vertex->m_simd.m_weights);
        break;
    default:
        weight = bx::simd_zero();
        break;
    }
    return weight;
}

void
Model::VertexUnit::performSkinningBdef1(const model::Vertex *vertex, const bx::simd128_t op, const bx::simd128_t on,
    bx::simd128_t *p, bx::simd128_t *n) NANOEM_DECL_NOEXCEPT
{
    const model::Bone *bone = vertex->bone(0);
    const bx::float4x4_t stm0 = bone->skinningTransformMatrix();
    const bx::float4x4_t ntm0 = bone->normalTransformMatrix();
    bx::simd_st(p, bx::simd_mul_xyz1(op, &stm0));
    bx::simd_st(n, bx::simd_mul_xyz1(on, &ntm0));
}

void
Model::VertexUnit::performSkinningBdef2(const model::Vertex *vertex, const bx::simd128_t op, const bx::simd128_t on,
    bx::simd128_t *p, bx::simd128_t *n) NANOEM_DECL_NOEXCEPT
{
    const bx::simd128_t weight = bx::simd_swiz_xxxx(vertex->m_simd.m_weights);
    if (bx::simd_test_all_x(bx::simd_cmpeq(weight, bx::simd_zero()))) {
        const model::Bone *bone1 = vertex->bone(1);
        const bx::float4x4_t stm0 = bone1->skinningTransformMatrix();
        const bx::float4x4_t ntm0 = bone1->normalTransformMatrix();
        bx::simd_st(p, bx::simd_mul_xyz1(op, &stm0));
        bx::simd_st(n, bx::simd_mul_xyz1(on, &ntm0));
    }
    else if (bx::simd_test_all_x(bx::simd_cmpeq(weight, bx::simd_splat(1.0f)))) {
        const model::Bone *bone0 = vertex->bone(0);
        const bx::float4x4_t stm1 = bone0->skinningTransformMatrix();
        const bx::float4x4_t ntm1 = bone0->normalTransformMatrix();
        bx::simd_st(p, bx::simd_mul_xyz1(op, &stm1));
        bx::simd_st(n, bx::simd_mul_xyz1(on, &ntm1));
    }
    else {
        const model::Bone *bone0 = vertex->bone(0);
        const model::Bone *bone1 = vertex->bone(1);
        const bx::float4x4_t stm0 = bone0->skinningTransformMatrix();
        const bx::float4x4_t stm1 = bone1->skinningTransformMatrix();
        const bx::float4x4_t ntm0 = bone0->normalTransformMatrix();
        const bx::float4x4_t ntm1 = bone1->normalTransformMatrix();
        const bx::simd128_t p0 = bx::simd_mul_xyz1(op, &stm0);
        const bx::simd128_t p1 = bx::simd_mul_xyz1(op, &stm1);
        const bx::simd128_t n0 = bx::simd_mul_xyz1(on, &ntm0);
        const bx::simd128_t n1 = bx::simd_mul_xyz1(on, &ntm1);
        bx::simd_st(p, bx::simd_lerp_ni(p1, p0, weight));
        bx::simd_st(n, bx::simd_lerp_ni(n1, n0, weight));
    }
}

void
Model::VertexUnit::performSkinningBdef4(const model::Vertex *vertex, const bx::simd128_t op, const bx::simd128_t on,
    nanoem_rsize_t i, bx::simd128_t *p, bx::simd128_t *n) NANOEM_DECL_NOEXCEPT
{
    const model::Bone *bone = vertex->bone(i);
    const bx::simd128_t weight = swizzleWeight(vertex, i);
    const bx::float4x4_t stm = bone->skinningTransformMatrix();
    const bx::float4x4_t ntm = bone->normalTransformMatrix();
    const bx::simd128_t p0 = bx::simd_mul_xyz1(op, &stm);
    const bx::simd128_t n0 = bx::simd_mul_xyz1(on, &ntm);
    bx::simd_st(p, bx::simd_madd_ni(p0, weight, bx::simd_ld(p)));
    bx::simd_st(n, bx::simd_madd_ni(n0, weight, bx::simd_ld(n)));
}

void
Model::VertexUnit::performSkinningBdef4(const model::Vertex *vertex, const bx::simd128_t op, const bx::simd128_t on,
    bx::simd128_t *p, bx::simd128_t *n) NANOEM_DECL_NOEXCEPT
{
    bx::simd_st(p, bx::simd_zero());
    bx::simd_st(n, bx::simd_zero());
    performSkinningBdef4(vertex, op, on, 0, p, n);
    performSkinningBdef4(vertex, op, on, 1, p, n);
    performSkinningBdef4(vertex, op, on, 2, p, n);
    performSkinningBdef4(vertex, op, on, 3, p, n);
}

void
Model::VertexUnit::performSkinningQdef(const model::Vertex *vertex, const bx::simd128_t op, const bx::simd128_t on,
    nanoem_rsize_t i, bx::simd128_t *p, bx::simd128_t *n) NANOEM_DECL_NOEXCEPT
{
#if GLM_VERSION >= 960
    const model::Bone *u = vertex->bone(i);
    const bx::simd128_t weight = swizzleWeight(vertex, i);
    const Matrix4x4 pdq(glm::mat3x4_cast(glm::dualquat(u->localOrientation(), u->localTranslation())));
    const Matrix4x4 ndq(glm::mat3x4_cast(glm::dualquat(u->localOrientation(), Constants::kZeroV3)));
    const bx::float4x4_t *pdq1 = reinterpret_cast<const bx::float4x4_t *>(glm::value_ptr(pdq));
    const bx::float4x4_t *ndq1 = reinterpret_cast<const bx::float4x4_t *>(glm::value_ptr(ndq));
    bx::simd_st(p, bx::simd_madd_ni(bx::simd_mul_xyz1(op, pdq1), weight, bx::simd_ld(p)));
    bx::simd_st(n, bx::simd_madd_ni(bx::simd_mul_xyz1(on, ndq1), weight, bx::simd_ld(n)));
#else
    performSkinningBdef4(vertex, op, on, i, p, n);
#endif
}

void
Model::VertexUnit::performSkinningQdef(const model::Vertex *vertex, const bx::simd128_t op, const bx::simd128_t on,
    bx::simd128_t *p, bx::simd128_t *n) NANOEM_DECL_NOEXCEPT
{
    bx::simd_st(p, bx::simd_zero());
    bx::simd_st(n, bx::simd_zero());
    performSkinningQdef(vertex, op, on, 0, p, n);
    performSkinningQdef(vertex, op, on, 1, p, n);
    performSkinningQdef(vertex, op, on, 2, p, n);
    performSkinningQdef(vertex, op, on, 3, p, n);
}

void
Model::VertexUnit::performSkinningSdef(const model::Vertex *vertex, const bx::simd128_t op, const bx::simd128_t on,
    bx::simd128_t *p, bx::simd128_t *n) NANOEM_DECL_NOEXCEPT
{
#if defined(NANOEM_ENABLE_SDEF)
    const model::Bone *bone0 = vertex->bone(0);
    const model::Bone *bone1 = vertex->bone(1);
    const nanoem_model_vertex_t *vertexPtr = vertex->data();
    const bx::simd128_t weight = vertex->m_simd.m_weights;
    const bx::simd128_t sdefC = bx::simd_ld(nanoemModelVertexGetSdefC(vertexPtr));
    const bx::simd128_t sdefR0 = bx::simd_ld(nanoemModelVertexGetSdefR0(vertexPtr));
    const bx::simd128_t sdefR1 = bx::simd_ld(nanoemModelVertexGetSdefR1(vertexPtr));
    const bx::simd128_t w0 = bx::simd_swiz_xxxx(weight);
    const bx::simd128_t w1 = bx::simd_swiz_yyyy(weight);
    const bx::simd128_t sdefI = bx::simd_add(bx::simd_mul(sdefR0, w0), bx::simd_mul(sdefR1, w1));
    const bx::simd128_t sdefR0N = bx::simd_add(sdefC, bx::simd_sub(sdefR0, sdefI));
    const bx::simd128_t sdefR1N = bx::simd_add(sdefC, bx::simd_sub(sdefR1, sdefI));
    const bx::float4x4_t stm0 = bone0->skinningTransformMatrix();
    const bx::float4x4_t stm1 = bone1->skinningTransformMatrix();
    const bx::simd128_t r0 = bx::simd_mul_xyz1(sdefR0N, &stm0);
    const bx::simd128_t r1 = bx::simd_mul_xyz1(sdefR1N, &stm1);
    const bx::simd128_t c0 = bx::simd_mul_xyz1(sdefC, &stm0);
    const bx::simd128_t c1 = bx::simd_mul_xyz1(sdefC, &stm1);
    const bx::simd128_t delta = bx::simd_add(bx::simd_mul(bx::simd_sub(bx::simd_add(r0, c0), sdefC), w0),
        bx::simd_mul(bx::simd_sub(bx::simd_add(r1, c1), sdefC), w1));
    const bx::simd128_t t = bx::simd_mul(bx::simd_add(sdefC, delta), bx::simd_splat(0.5f));
    const Quaternion q0(glm::quat_cast(bone0->skinningTransform())), q1(glm::quat_cast(bone1->skinningTransform()));
    const Matrix4x4 m0(glm::mat4_cast(glm::slerp(q0, q1, bx::simd_x(w1))));
    const bx::float4x4_t m1 = *reinterpret_cast<const bx::float4x4_t *>(&m0);
    bx::simd_st(p, bx::simd_add(bx::simd_mul_xyz1(bx::simd_sub(op, sdefC), &m1), t));
    bx::simd_st(n, bx::simd_mul_xyz1(on, &m1));
#else
    performSkinningBdef2(vertex, op, on, p, n);
#endif /* NANOEM_ENABLE_SDEF */
}

void
Model::VertexUnit::performSkinningByType(
    const model::Vertex *vertex, bx::simd128_t *p, bx::simd128_t *n) NANOEM_DECL_NOEXCEPT
{
    typedef void (*pfn_performSkinning)(
        const model::Vertex *, const bx::simd128_t, const bx::simd128_t, bx::simd128_t *, bx::simd128_t *);
    static pfn_performSkinning s_funcs[] = { performSkinningBdef1, performSkinningBdef2, performSkinningBdef4,
        performSkinningSdef, performSkinningQdef };
    nanoem_model_vertex_type_t type = nanoemModelVertexGetType(vertex->data());
    s_funcs[type](vertex, bx::simd_add(vertex->m_simd.m_origin, vertex->m_simd.m_delta), vertex->m_simd.m_normal, p, n);
}

Model::ImportSetting::ImportSetting(const URI &fileURI)
    : m_fileURI(fileURI)
    , m_transform(1)
    , m_fileType(kFileTypeNone)
{
    const String &pathExtension = fileURI.pathExtension();
    if (pathExtension == String("obj")) {
        m_fileType = kFileTypeWaveFrontObj;
    }
    else if (pathExtension == String("x")) {
        m_fileType = kFileTypeDirectX;
    }
    else if (pathExtension == String("mqo")) {
        m_fileType = kFileTypeMetasequoia;
    }
    m_name[NANOEM_LANGUAGE_TYPE_JAPANESE] = m_name[NANOEM_LANGUAGE_TYPE_ENGLISH] = fileURI.lastPathComponent();
    String comment;
    String filename(URI::stringByDeletingPathExtension(fileURI.lastPathComponent()));
    String path(fileURI.absolutePathByDeletingLastPathComponent());
    path.append("/");
    path.append(filename.c_str());
    path.append(".txt");
    FileReaderScope scope(nullptr);
    Error error;
    if (scope.open(URI::createFromFilePath(path), error)) {
        ByteArray bytes;
        FileUtils::read(scope, bytes, error);
        const char *text = reinterpret_cast<const char *>(bytes.data());
        comment.append(text, text + bytes.size());
    }
    else {
        StringUtils::format(comment, "This comment can be replaced with %s.txt in the same folder of %s",
            filename.c_str(), fileURI.lastPathComponent().c_str());
    }
    m_comment[NANOEM_LANGUAGE_TYPE_JAPANESE] = m_comment[NANOEM_LANGUAGE_TYPE_ENGLISH] = comment;
}

Model::ImportSetting::~ImportSetting() NANOEM_DECL_NOEXCEPT
{
}

Model::ExportSetting::ExportSetting()
    : m_transform(1)
{
}

Model::ExportSetting::~ExportSetting() NANOEM_DECL_NOEXCEPT
{
}

Model::ParallelSkinningTaskData::ParallelSkinningTaskData(
    Model *model, const IDrawable::DrawType type, nanoem_f32_t edgeSizeFactor)
    : m_model(model)
    , m_drawType(type)
    , m_edgeSizeScaleFactor(edgeSizeFactor)
    , m_boneIndices(0)
    , m_output(0)
    , m_materials(nullptr)
    , m_vertices(nullptr)
    , m_numVertices(0)
{
    const nanoem_model_t *opaque = model->data();
    nanoem_rsize_t numMaterials;
    m_materials = nanoemModelGetAllMaterialObjects(opaque, &numMaterials);
    m_vertices = nanoemModelGetAllVertexObjects(opaque, &m_numVertices);
}

Model::ParallelSkinningTaskData::~ParallelSkinningTaskData() NANOEM_DECL_NOEXCEPT
{
}

Model::DrawArrayBuffer::DrawArrayBuffer()
{
    m_buffer = { SG_INVALID_ID };
}

Model::DrawArrayBuffer::~DrawArrayBuffer() NANOEM_DECL_NOEXCEPT
{
}

void
Model::DrawArrayBuffer::destroy()
{
    SG_INSERT_MARKERF("Model::DrawArrayBuffer::destroy(vertex=%d", m_buffer.id);
    sg::destroy_buffer(m_buffer);
    m_buffer = { SG_INVALID_ID };
}

Model::DrawIndexedBuffer::DrawIndexedBuffer()
{
    m_vertexBuffer = m_indexBuffer = { SG_INVALID_ID };
}

Model::DrawIndexedBuffer::~DrawIndexedBuffer() NANOEM_DECL_NOEXCEPT
{
}

nanoem_u32_t
Model::DrawIndexedBuffer::fillShape(const par_shapes_mesh *shape, const Vector4 &color)
{
    nanoem_u32_t numVertices = Inline::saturateInt32U(shape->npoints);
    if (m_color != color && sg::is_valid(m_vertexBuffer)) {
        sg::destroy_buffer(m_vertexBuffer);
        m_vertexBuffer = { SG_INVALID_ID };
        m_color = color;
    }
    if (!sg::is_valid(m_vertexBuffer)) {
        size_t size = sizeof(m_vertices[0]) * numVertices;
        m_vertices.resize(numVertices);
        const Vector4U8 &normalizedColor = color * Vector4(0xff);
        for (nanoem_u32_t i = 0; i < numVertices; i++) {
            sg::LineVertexUnit &unit = m_vertices[i];
            unit.m_position = Vector4(glm::make_vec3(&shape->points[i * 3]), 1);
            unit.m_color = normalizedColor;
        }
        sg_buffer_desc desc;
        Inline::clearZeroMemory(desc);
        desc.data.ptr = m_vertices.data();
        desc.data.size = desc.size = size;
        desc.usage = SG_USAGE_IMMUTABLE;
        m_vertexBuffer = sg::make_buffer(&desc);
        nanoem_assert(sg::query_buffer_state(m_vertexBuffer) == SG_RESOURCESTATE_VALID, "vertex buffer must be valid");
    }
    if (!sg::is_valid(m_indexBuffer)) {
        nanoem_u32_t numIndices = Inline::saturateInt32U(shape->ntriangles) * 3;
        size_t size = sizeof(m_indices[0]) * numIndices;
        m_indices.resize(numIndices);
        for (nanoem_u32_t i = 0; i < numIndices; i++) {
            m_indices[i] = shape->triangles[i];
        }
        sg_buffer_desc desc;
        Inline::clearZeroMemory(desc);
        desc.data.ptr = m_indices.data();
        desc.data.size = desc.size = size;
        desc.usage = SG_USAGE_IMMUTABLE;
        desc.type = SG_BUFFERTYPE_INDEXBUFFER;
        m_indexBuffer = sg::make_buffer(&desc);
        nanoem_assert(sg::query_buffer_state(m_indexBuffer) == SG_RESOURCESTATE_VALID, "index buffer must be valid");
    }
    return numVertices;
}

void
Model::DrawIndexedBuffer::destroy()
{
    SG_INSERT_MARKERF("Model::DrawIndexedBuffer::destroy(vertex=%d, index=%d)", m_vertexBuffer.id, m_indexBuffer.id);
    if (sg::is_valid(m_vertexBuffer)) {
        sg::destroy_buffer(m_vertexBuffer);
        m_vertexBuffer = { SG_INVALID_ID };
    }
    if (sg::is_valid(m_indexBuffer)) {
        sg::destroy_buffer(m_indexBuffer);
        m_indexBuffer = { SG_INVALID_ID };
    }
}

StringList
Model::loadableExtensions()
{
    static const String kLoadableModelExtensions[] = { String("pmd"), String("pmx"), String() };
    return StringList(
        &kLoadableModelExtensions[0], &kLoadableModelExtensions[BX_COUNTOF(kLoadableModelExtensions) - 1]);
}

StringSet
Model::loadableExtensionsSet()
{
    return ListUtils::toSetFromList<String>(loadableExtensions());
}

bool
Model::isLoadableExtension(const String &extension)
{
    return FileUtils::isLoadableExtension(extension, loadableExtensionsSet());
}

bool
Model::isLoadableExtension(const URI &fileURI)
{
    return isLoadableExtension(fileURI.pathExtension());
}

void
Model::setStandardPipelineDescription(sg_pipeline_desc &desc)
{
    setCommonPipelineDescription(desc);
    sg_layout_desc &ld = desc.layout;
    ld.attrs[0] = sg_vertex_attr_desc { 0, offsetof(VertexUnit, m_position), SG_VERTEXFORMAT_FLOAT3 };
    ld.attrs[7] = sg_vertex_attr_desc { 0, offsetof(VertexUnit, m_info), SG_VERTEXFORMAT_FLOAT4 };
}

void
Model::setEdgePipelineDescription(sg_pipeline_desc &desc)
{
    setCommonPipelineDescription(desc);
    sg_layout_desc &ld = desc.layout;
    ld.attrs[0] = sg_vertex_attr_desc { 0, offsetof(VertexUnit, m_edge), SG_VERTEXFORMAT_FLOAT3 };
    ld.attrs[7] = sg_vertex_attr_desc { 0, offsetof(VertexUnit, m_info), SG_VERTEXFORMAT_FLOAT4 };
}

Model::Model(Project *project, nanoem_u16_t handle)
    : m_handle(handle)
    , m_project(project)
    , m_camera(nullptr)
    , m_selection(nullptr)
    , m_drawer(nullptr)
    , m_skinDeformer(nullptr)
    , m_opaque(nullptr)
    , m_undoStack(nullptr)
    , m_activeConstraintPtr(nullptr)
    , m_activeMaterialPtr(nullptr)
    , m_activeBonePairPtr(nullptr, nullptr)
    , m_activeEffectPtrPair(nullptr, nullptr)
    , m_screenImage(nullptr)
    , m_sharedFallbackBone(nullptr)
    , m_userData(nullptr, nullptr)
    , m_pivotMatrix(0)
    , m_edgeColor(0, 0, 0, 1)
    , m_gizmoOperationType(kGizmoOperationTypeTranslate)
    , m_gizmoTransformCoordinateType(kTransformCoordinateTypeLocal)
    , m_transformAxisType(kAxisTypeMaxEnum)
    , m_transformCoordinateType(kTransformCoordinateTypeLocal)
    , m_states(kPrivateStateInitialValue)
    , m_edgeSizeScaleFactor(1.0f)
    , m_opacity(1.0f)
    , m_dispatchParallelTaskQueue(nullptr)
    , m_countVertexSkinningNeeded(0)
    , m_stageVertexBufferIndex(0)
{
    nanoem_assert(m_project, "must not be nullptr");
    Inline::clearZeroMemory(m_activeMorphPtr);
    m_vertexBuffers[0] = m_vertexBuffers[1] = m_indexBuffer = { SG_INVALID_ID };
    m_drawAllPoints.m_buffer = m_drawAllLines.m_vertexBuffer = m_drawAllLines.m_indexBuffer = { SG_INVALID_ID };
    m_activeEffectPtrPair.first = m_project->sharedResourceRepository()->modelProgramBundle();
    m_activeEffectPtrPair.second = nullptr;
    m_selection = nanoem_new(internal::ModelObjectSelection(this));
    m_undoStack = undoStackCreateWithSoftLimit(undoStackGetSoftLimit(m_project->undoStack()));
    m_camera = project->createCamera();
    const ICamera *camera = project->globalCamera();
    m_camera->setAngle(camera->angle());
    m_camera->setDistance(camera->distance());
    m_camera->setFov(camera->fov());
    m_camera->setLookAt(camera->lookAt());
    m_opaque = nanoemModelCreate(project->unicodeStringFactory(), nullptr);
    m_camera->update();
    setShadowMapEnabled(true);
}

Model::~Model() NANOEM_DECL_NOEXCEPT
{
    nanoem_delete_safe(m_camera);
    nanoem_delete_safe(m_drawer);
    nanoem_delete_safe(m_skinDeformer);
    nanoem_delete_safe(m_selection);
    nanoem_delete_safe(m_screenImage);
    undoStackDestroy(m_undoStack);
    m_undoStack = nullptr;
    nanoemModelDestroy(m_opaque);
    m_activeBonePairPtr.first = m_activeBonePairPtr.second = nullptr;
    m_activeEffectPtrPair.first = nullptr;
    m_activeEffectPtrPair.second = nullptr;
    m_activeMaterialPtr = nullptr;
    m_edgeSizeScaleFactor = 0.0f;
    m_opacity = 0.0f;
    m_countVertexSkinningNeeded = 0;
    m_opaque = nullptr;
    m_project = nullptr;
}

bool
Model::load(const nanoem_u8_t *bytes, size_t length, Error &error)
{
    nanoem_parameter_assert(bytes, "must not be nullptr");
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_buffer_t *buffer = nanoemBufferCreate(bytes, length, &status);
    nanoemModelLoadFromBuffer(m_opaque, buffer, &status);
    nanoemBufferDestroy(buffer);
    bool succeeded = status == NANOEM_STATUS_SUCCESS;
    if (succeeded) {
        if (nanoemModelGetFormatType(m_opaque) == NANOEM_MODEL_FORMAT_TYPE_PMD_1_0) {
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            nanoem_model_converter_t *converter = nanoemModelConverterCreate(m_opaque, &status);
            nanoem_mutable_model_t *model =
                nanoemModelConverterExecute(converter, NANOEM_MODEL_FORMAT_TYPE_PMX_2_0, &status);
            nanoem_model_t *previousOpaqueData = m_opaque;
            m_opaque = nanoemMutableModelGetOriginObjectReference(model);
            nanoemModelDestroy(previousOpaqueData);
            nanoemMutableModelDestroy(model);
            nanoemModelConverterDestroy(converter);
        }
        nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
        nanoem_language_type_t language = m_project->castLanguage();
        StringUtils::getUtf8String(nanoemModelGetName(m_opaque, language), factory, m_name);
        StringUtils::getUtf8String(nanoemModelGetComment(m_opaque, language), factory, m_comment);
        StringUtils::getUtf8String(
            nanoemModelGetName(m_opaque, NANOEM_LANGUAGE_TYPE_FIRST_ENUM), factory, m_canonicalName);
        if (m_name.empty()) {
            m_name = m_canonicalName;
        }
    }
    else {
        char message[Error::kMaxReasonLength];
        StringUtils::format(message, sizeof(message), "Cannot load the model: %s",
            Error::convertStatusToMessage(status, m_project->translator()));
        error = Error(message, status, Error::kDomainTypeNanoem);
    }
    return succeeded;
}

bool
Model::load(const ByteArray &bytes, Error &error)
{
    return load(bytes.data(), bytes.size(), error);
}

bool
Model::load(const nanoem_u8_t *bytes, size_t length, const ImportSetting &setting, Error &error)
{
    model::Importer importer(this);
    return importer.execute(bytes, length, setting, error);
}

bool
Model::load(const ByteArray &bytes, const ImportSetting &setting, Error &error)
{
    return load(bytes.data(), bytes.size(), setting, error);
}

bool
Model::loadPose(const nanoem_u8_t *bytes, size_t length, Error &error)
{
    model::BindPose pose;
    bool result = pose.load(this, bytes, length, error);
    if (result) {
        performAllMorphsDeform(false);
    }
    return result;
}

bool
Model::loadPose(const ByteArray &bytes, Error &error)
{
    return loadPose(bytes.data(), bytes.size(), error);
}

bool
Model::loadArchive(const String &entryPoint, const Archiver &archiver, Error &error)
{
    Archiver::Entry entry;
    bool succeeded = false;
    if (archiver.findEntry(entryPoint, entry, error)) {
        ByteArray bytes;
        succeeded = archiver.extract(entry, bytes, error) && !bytes.empty() && load(bytes, error);
    }
    return succeeded;
}

bool
Model::loadArchive(const String &entryPoint, ISeekableReader *reader, Error &error)
{
    bool succeeded = false;
    if (!entryPoint.empty()) {
        Archiver archiver(reader);
        if (archiver.open(error)) {
            succeeded = loadArchive(entryPoint, archiver, error);
            archiver.close(error);
        }
    }
    return succeeded;
}

bool
Model::loadArchive(ISeekableReader *reader, Progress &progress, Error &error)
{
    Archiver archiver(reader);
    bool succeeded = false;
    if (archiver.open(error)) {
        const Archiver::EntryList &entries = archiver.allEntries(error);
        for (Archiver::EntryList::const_iterator it = entries.begin(), end = entries.end(); it != end; ++it) {
            const Archiver::Entry &entry = *it;
            const String &path = entry.m_path;
            const char *p = strrchr(path.c_str(), '.');
            if (p && isLoadableExtension(p + 1)) {
                archiver.close(error);
                succeeded = loadArchive(path, reader, error) && uploadArchive(reader, progress, error);
                break;
            }
        }
    }
    return succeeded;
}

bool
Model::save(IWriter *writer, Error &error) const
{
    nanoem_parameter_assert(writer, "must NOT be nullptr");
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_buffer_t *mutableBuffer = nanoemMutableBufferCreate(&status);
    nanoem_mutable_model_t *mutableModel = nanoemMutableModelCreateAsReference(m_opaque, &status);
    nanoemMutableModelSaveToBuffer(mutableModel, mutableBuffer, &status);
    bool succeeded = false;
    if (status == NANOEM_STATUS_SUCCESS) {
        nanoem_buffer_t *buffer = nanoemMutableBufferCreateBufferObject(mutableBuffer, &status);
        FileUtils::write(writer, buffer, error);
        nanoemBufferDestroy(buffer);
        succeeded = !error.hasReason();
    }
    else {
        char message[Error::kMaxReasonLength];
        StringUtils::format(message, sizeof(message), "Cannot save the model: %s",
            Error::convertStatusToMessage(status, m_project->translator()));
        error = Error(message, status, Error::kDomainTypeNanoem);
    }
    nanoemMutableModelDestroy(mutableModel);
    nanoemMutableBufferDestroy(mutableBuffer);
    return succeeded;
}

bool
Model::save(ByteArray &bytes, Error &error) const
{
    MemoryWriter writer(&bytes);
    return save(&writer, error);
}

bool
Model::save(IWriter *writer, const ExportSetting &setting, Error &error) const
{
    model::Exporter exporter(this);
    return exporter.execute(writer, setting, error);
}

bool
Model::save(ByteArray &bytes, const ExportSetting &setting, Error &error) const
{
    MemoryWriter writer(&bytes);
    return save(&writer, setting, error);
}

bool
Model::savePose(IWriter *writer, Error &error)
{
    model::BindPose pose;
    return pose.save(this, writer, error);
}

bool
Model::savePose(ByteArray &bytes, Error &error)
{
    MemoryWriter writer(&bytes);
    return savePose(&writer, error);
}

bool
Model::saveArchive(const String &prefix, Archiver &archiver, Error &error)
{
    Archiver::Entry entry;
    ByteArray bytes;
    bool succeeded = save(bytes, error);
    if (succeeded) {
        String newFileName(prefix);
        newFileName.append(filename().c_str());
        entry.m_path = newFileName;
        if (archiver.addEntry(entry, bytes, error)) {
            FileEntityMap allAttachments;
            for (FileEntityMap::const_iterator it = m_imageURIs.begin(), end = m_imageURIs.end(); it != end; ++it) {
                allAttachments.insert(tinystl::make_pair(it->first, it->second));
            }
            for (FileEntityMap::const_iterator it = m_attachmentURIs.begin(), end = m_attachmentURIs.end(); it != end;
                 ++it) {
                allAttachments.insert(tinystl::make_pair(it->first, it->second));
            }
            if (const Effect *effect = m_project->resolveEffect(this)) {
                effect->attachAllResources(allAttachments);
                for (OffscreenPassiveRenderTargetEffectMap::const_iterator
                         it = m_offscreenPassiveRenderTargetEffects.begin(),
                         end = m_offscreenPassiveRenderTargetEffects.end();
                     it != end; ++it) {
                    const OffscreenPassiveRenderTargetEffect &effect = it->second;
                    if (const Effect *passiveEffect = m_project->upcastEffect(effect.m_passiveEffect)) {
                        passiveEffect->attachAllResources(allAttachments);
                    }
                }
            }
            succeeded &= saveAllAttachments(prefix, allAttachments, archiver, error);
        }
        else {
            succeeded = false;
        }
    }
    return succeeded;
}

bool
Model::saveArchive(IFileWriter *writer, Error &error)
{
    nanoem_parameter_assert(writer, "must NOT be nullptr");
    bool succeeded = true;
    Archiver archiver(writer);
    if (archiver.open(error)) {
        String basePath(canonicalName());
        basePath.append("/");
        succeeded = saveArchive(basePath, archiver, error);
        archiver.close(error);
    }
    else {
        succeeded = false;
    }
    return succeeded;
}

nanoem_u32_t
Model::createAllImages()
{
    const bool flip = !sg::query_features().origin_top_left;
    Project::ISharedResourceRepository *repository = m_project->sharedResourceRepository();
    nanoem_rsize_t numMaterials;
    nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(m_opaque, &numMaterials);
    nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
    clearAllLoadingImageItems();
    for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
        const nanoem_model_material_t *materialPtr = materials[i];
        model::Material *material = model::Material::cast(materialPtr);
        if (const nanoem_model_texture_t *diffuseTexture = nanoemModelMaterialGetDiffuseTextureObject(materialPtr)) {
            const nanoem_unicode_string_t *path = nanoemModelTextureGetPath(diffuseTexture);
            nanoem_rsize_t length;
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            nanoem_u8_t utf8Path[16];
            nanoemUnicodeStringFactoryToUtf8OnStackEXT(factory, path, &length, utf8Path, sizeof(utf8Path), &status);
            if (ImageLoader::isScreenBMP(reinterpret_cast<const char *>(utf8Path))) {
                m_screenImage = nanoem_new(Image);
                m_screenImage->m_filename = Project::kViewportSecondaryName;
                m_screenImage->m_handle = m_project->viewportSecondaryImage();
                Inline::clearZeroMemory(m_screenImage->m_description);
                material->setDiffuseImage(m_screenImage);
            }
            else {
                const nanoem_u32_t flags = ImageLoader::kFlagsEnableMipmap | ImageLoader::kFlagsFallbackWhiteOpaque;
                material->setDiffuseImage(createImage(path, SG_WRAP_REPEAT, flags));
            }
        }
        if (const nanoem_model_texture_t *sphereTexture = nanoemModelMaterialGetSphereMapTextureObject(materialPtr)) {
            const nanoem_unicode_string_t *path = nanoemModelTextureGetPath(sphereTexture);
            uint32_t flags = nanoemModelMaterialGetSphereMapTextureType(materialPtr) ==
                    NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_ADD
                ? ImageLoader::kFlagsFallbackBlackOpaque
                : ImageLoader::kFlagsFallbackWhiteOpaque;
            if (flip) {
                flags |= ImageLoader::kFlagsEnableFlipY;
            }
            material->setSphereMapImage(createImage(path, SG_WRAP_CLAMP_TO_EDGE, flags));
        }
        if (nanoemModelMaterialIsToonShared(materialPtr)) {
            const int index = nanoemModelMaterialGetToonTextureIndex(materialPtr);
            material->setToonImage(repository->toonImage(index));
            material->setToonColor(repository->toonColor(index) / Vector4(0xff));
        }
        else if (const nanoem_model_texture_t *toonTexture = nanoemModelMaterialGetToonTextureObject(materialPtr)) {
            const nanoem_unicode_string_t *path = nanoemModelTextureGetPath(toonTexture);
            material->setToonImage(createImage(path, SG_WRAP_CLAMP_TO_EDGE, ImageLoader::kFlagsFallbackWhiteOpaque));
        }
    }
    return Inline::saturateInt32U(m_loadingImageItems.size());
}

void
Model::setupAllBindings()
{
    nanoem_rsize_t numObjects, numVertices, numVertexIndices;
    model::RigidBody::Resolver resolver;
    String objectName;
    nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
    nanoem_language_type_t language = m_project->castLanguage();
    nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(m_opaque, &numObjects);
    numVertices = numObjects;
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        nanoem_model_vertex_t *vertexPtr = vertices[i];
        model::Vertex *vertex = model::Vertex::create();
        vertex->bind(vertexPtr);
    }
    nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(m_opaque, &numObjects);
    const nanoem_u32_t *indices = nanoemModelGetAllVertexIndices(m_opaque, &numVertexIndices);
    nanoem_rsize_t indexOffset = 0, numIndices;
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        nanoem_model_material_t *materialPtr = materials[i];
        model::Material *material = model::Material::create(m_project->sharedFallbackImage());
        material->bind(materialPtr);
        material->resetLanguage(materialPtr, factory, language);
        numIndices = nanoemModelMaterialGetNumVertexIndices(materialPtr);
        for (nanoem_rsize_t j = indexOffset, offsetTo = indexOffset + numIndices; j < offsetTo; j++) {
            const nanoem_u32_t vertexIndex = indices[j];
            if (model::Vertex *vertex = model::Vertex::cast(vertices[vertexIndex])) {
                vertex->setMaterial(materialPtr);
            }
        }
        indexOffset += numIndices;
    }
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_opaque, &numObjects);
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        nanoem_model_bone_t *bonePtr = bones[i];
        model::Bone *bone = model::Bone::create();
        bone->bind(bonePtr);
        bone->resetLanguage(bonePtr, factory, language);
        if (const nanoem_model_constraint_t *constraint = nanoemModelBoneGetConstraintObject(bonePtr)) {
            bindConstraint(const_cast<nanoem_model_constraint_t *>(constraint));
        }
        if (nanoemModelBoneHasInherentOrientation(bonePtr) || nanoemModelBoneHasInherentTranslation(bonePtr)) {
            const nanoem_model_bone_t *parentBone = nanoemModelBoneGetInherentParentBoneObject(bonePtr);
            m_inherentBones[parentBone].insert(bonePtr);
        }
        for (nanoem_rsize_t j = NANOEM_LANGUAGE_TYPE_FIRST_ENUM; j < NANOEM_LANGUAGE_TYPE_MAX_ENUM; j++) {
            StringUtils::getUtf8String(
                nanoemModelBoneGetName(bonePtr, static_cast<nanoem_language_type_t>(j)), factory, objectName);
            m_bones.insert(tinystl::make_pair(objectName, static_cast<const nanoem_model_bone_t *>(bonePtr)));
        }
    }
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        nanoem_model_bone_t *bonePtr = bones[i];
        if (const nanoem_model_bone_t *parentBone = nanoemModelBoneGetParentBoneObject(bonePtr)) {
            m_parentBoneTree[parentBone].push_back(bonePtr);
        }
    }
    if (numObjects > 0) {
        const nanoem_model_bone_t *firstBone = bones[0];
        m_selection->toggleSelectAndActiveBone(firstBone, false);
    }
    nanoem_model_constraint_t *const *constraints = nanoemModelGetAllConstraintObjects(m_opaque, &numObjects);
    if (numObjects > 0 && constraints) {
        for (nanoem_rsize_t i = 0; i < numObjects; i++) {
            nanoem_model_constraint_t *constraintPtr = constraints[i];
            bindConstraint(constraintPtr);
            const nanoem_model_bone_t *targetBonePtr = nanoemModelConstraintGetTargetBoneObject(constraintPtr);
            const nanoem_model_constraint_t *constConstraintPtr = constraintPtr;
            m_constraints.insert(tinystl::make_pair(targetBonePtr, constConstraintPtr));
        }
    }
    else {
        nanoemModelGetAllBoneObjects(m_opaque, &numObjects);
        for (nanoem_rsize_t i = 0; i < numObjects; i++) {
            const nanoem_model_bone_t *targetBonePtr = bones[i];
            if (const nanoem_model_constraint_t *constraintPtr = nanoemModelBoneGetConstraintObject(targetBonePtr)) {
                m_constraints.insert(tinystl::make_pair(targetBonePtr, constraintPtr));
            }
        }
    }
    nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(m_opaque, &numObjects);
    model::Bone::Set boneSet;
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        nanoem_model_morph_t *morphPtr = morphs[i];
        model::Morph *morph = model::Morph::create();
        morph->bind(morphPtr);
        morph->resetLanguage(morphPtr, factory, language);
        if (nanoemModelMorphGetType(morphPtr) == NANOEM_MODEL_MORPH_TYPE_VERTEX) {
            nanoem_rsize_t numMorphVertices;
            nanoem_model_morph_vertex_t *const *morphVertices =
                nanoemModelMorphGetAllVertexMorphObjects(morphPtr, &numMorphVertices);
            for (nanoem_rsize_t j = 0; j < numMorphVertices; j++) {
                const nanoem_model_vertex_t *vertex = nanoemModelMorphVertexGetVertexObject(morphVertices[j]);
                boneSet.insert(nanoemModelVertexGetBoneObject(vertex, 0));
                boneSet.insert(nanoemModelVertexGetBoneObject(vertex, 1));
                boneSet.insert(nanoemModelVertexGetBoneObject(vertex, 2));
                boneSet.insert(nanoemModelVertexGetBoneObject(vertex, 3));
            }
        }
        nanoem_model_morph_category_t category = nanoemModelMorphGetCategory(morphPtr);
        if (!activeMorph(category)) {
            setActiveMorph(category, morphPtr);
        }
        for (nanoem_rsize_t j = NANOEM_LANGUAGE_TYPE_FIRST_ENUM; j < NANOEM_LANGUAGE_TYPE_MAX_ENUM; j++) {
            StringUtils::getUtf8String(
                nanoemModelMorphGetName(morphPtr, static_cast<nanoem_language_type_t>(j)), factory, objectName);
            m_morphs.insert(tinystl::make_pair(objectName, static_cast<const nanoem_model_morph_t *>(morphPtr)));
        }
    }
    nanoem_model_label_t *const *labels = nanoemModelGetAllLabelObjects(m_opaque, &numObjects);
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        nanoem_model_label_t *labelPtr = labels[i];
        model::Label *label = model::Label::create();
        label->bind(labelPtr);
        label->resetLanguage(labelPtr, factory, language);
    }
    PhysicsEngine *physics = m_project->physicsEngine();
    nanoem_model_rigid_body_t *const *rigidBodies = nanoemModelGetAllRigidBodyObjects(m_opaque, &numObjects);
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        nanoem_model_rigid_body_t *rigidBodyPtr = rigidBodies[i];
        model::RigidBody *rigidBody = model::RigidBody::create();
        bool isDynamic = nanoemModelRigidBodyGetTransformType(rigidBodyPtr) !=
            NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_BONE_TO_SIMULATION;
        bool isMorph = isDynamic && boneSet.find(nanoemModelRigidBodyGetBoneObject(rigidBodyPtr)) != boneSet.end();
        rigidBody->bind(rigidBodyPtr, physics, isMorph, resolver);
        rigidBody->resetLanguage(rigidBodyPtr, factory, language);
        rigidBody->initializeTransformFeedback(rigidBodyPtr);
    }
    nanoem_model_joint_t *const *joints = nanoemModelGetAllJointObjects(m_opaque, &numObjects);
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        nanoem_model_joint_t *jointPtr = joints[i];
        model::Joint *joint = model::Joint::create();
        joint->bind(jointPtr, physics, resolver);
        joint->resetLanguage(jointPtr, factory, language);
    }
#if defined(NANOEM_ENABLE_SOFTBODY)
    nanoem_model_soft_body_t *const *softBodies = nanoemModelGetAllSoftBodyObjects(m_opaque, &numObjects);
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        nanoem_model_soft_body_t *softBodyPtr = softBodies[i];
        model::SoftBody *softBody = model::SoftBody::create();
        softBody->bind(softBodyPtr, physics, resolver);
        softBody->resetLanguage(softBodyPtr, factory, language);
    }
#endif /* NANOEM_ENABLE_SOFTBODY */
    for (nanoem_rsize_t i = 0; i < numVertices; i++) {
        nanoem_model_vertex_t *vertexPtr = vertices[i];
        model::Vertex *vertex = model::Vertex::cast(vertexPtr);
        vertex->setupBoneBinding(vertexPtr, this);
    }
    splitBonesPerMaterial(m_boneIndexHashes);
}

void
Model::upload()
{
    SG_PUSH_GROUPF("Model::upload(name=%s)", canonicalNameConstString());
    model::Material::IndexHashMap materialIndexHash;
    createVertexIndexBuffers();
    setActiveEffect(m_project->sharedResourceRepository()->modelProgramBundle());
#if defined(__APPLE__)
#ifdef NDEBUG
#define GCD_LABEL(a) nullptr
#else
#define GCD_LABEL(a) a
#endif /* NDEBUG */
    if (dispatch_queue_t queue = static_cast<dispatch_queue_t>(m_dispatchParallelTaskQueue)) {
        dispatch_release(queue);
    }
    m_dispatchParallelTaskQueue =
        dispatch_queue_create(GCD_LABEL("com.github.nanoem.gcd.model"), DISPATCH_QUEUE_CONCURRENT);
#undef GCD_LABEL
#endif /* __APPLE__ */
    EnumUtils::setEnabled(kPrivateStateUploaded, m_states, true);
    setDirty(true);
    SG_POP_GROUP();
}

void
Model::uploadArchive(const Archiver &archiver, Progress &progress, Error &error)
{
    SG_PUSH_GROUPF("Model::uploadArchive(name=%s)", canonicalNameConstString());
    ByteArray bytes;
    Archiver::Entry entry;
    ImageLoader *imageLoader = m_project->sharedImageLoader();
    for (LoadingImageItemList::const_iterator it = m_loadingImageItems.begin(), end = m_loadingImageItems.end();
         it != end; ++it) {
        const LoadingImageItem *item = *it;
        const URI &fileURI = item->m_fileURI;
        const String &filename = fileURI.fragment();
        if (!progress.tryLoadingItem(fileURI)) {
            error = Error::cancelled();
            break;
        }
        else if (archiver.findEntry(filename, entry, error) && archiver.extract(entry, bytes, error)) {
            imageLoader->decode(bytes, item->m_filename, this, item->m_wrap, item->m_flags, error);
        }
        else {
            sg_image_desc desc;
            if (EnumUtils::isEnabled(item->m_flags, ImageLoader::kFlagsFallbackWhiteOpaque)) {
                ImageLoader::fill1x1WhitePixelImage(desc);
            }
            else if (EnumUtils::isEnabled(item->m_flags, ImageLoader::kFlagsFallbackBlackOpaque)) {
                ImageLoader::fill1x1BlackPixelImage(desc);
            }
            else {
                ImageLoader::fill1x1TransparentPixelImage(desc);
            }
            uploadImage(item->m_filename, desc);
        }
    }
    clearAllLoadingImageItems();
    SG_POP_GROUP();
}

bool
Model::uploadArchive(ISeekableReader *reader, Progress &progress, Error &error)
{
    Archiver archiver(reader);
    bool succeeded = false;
    if (archiver.open(error)) {
        setupAllBindings();
        createAllImages();
        upload();
        uploadArchive(archiver, progress, error);
        archiver.close(error);
        succeeded = !error.hasReason();
    }
    return succeeded;
}

void
Model::loadAllImages(Progress &progress, Error &error)
{
    SG_PUSH_GROUPF("Model::loadAllImages(name=%s)", canonicalNameConstString());
    ImageLoader *imageLoader = m_project->sharedImageLoader();
    for (LoadingImageItemList::const_iterator it = m_loadingImageItems.begin(), end = m_loadingImageItems.end();
         it != end; ++it) {
        const LoadingImageItem *item = *it;
        const URI &fileURI = item->m_fileURI;
        if (!progress.tryLoadingItem(fileURI)) {
            error = Error::cancelled();
            break;
        }
        else if (!imageLoader->load(fileURI, this, item->m_wrap, item->m_flags, error)) {
            sg_image_desc desc;
            if (EnumUtils::isEnabled(item->m_flags, ImageLoader::kFlagsFallbackWhiteOpaque)) {
                ImageLoader::fill1x1WhitePixelImage(desc);
            }
            else if (EnumUtils::isEnabled(item->m_flags, ImageLoader::kFlagsFallbackBlackOpaque)) {
                ImageLoader::fill1x1BlackPixelImage(desc);
            }
            else {
                ImageLoader::fill1x1TransparentPixelImage(desc);
            }
            uploadImage(item->m_filename, desc);
        }
        progress.increment();
    }
    clearAllLoadingImageItems();
    SG_POP_GROUP();
}

void
Model::readLoadCommandMessage(const Nanoem__Application__Command *action)
{
    const Nanoem__Application__RedoLoadModelCommand *command = action->redo_load_model;
    const nanoem_rsize_t numBones = command->n_bones;
    m_redoBoneNames.resize(numBones);
    for (nanoem_rsize_t i = 0; i < numBones; i++) {
        const Nanoem__Application__Bone *bone = command->bones[i];
        if ((size_t) bone->index < numBones) {
            m_redoBoneNames[bone->index] = bone->name;
        }
    }
    const nanoem_rsize_t numMorphs = command->n_morphs;
    m_redoMorphNames.resize(numMorphs);
    for (nanoem_rsize_t i = 0; i < numMorphs; i++) {
        const Nanoem__Application__Morph *morph = command->morphs[i];
        if ((size_t) morph->index < numMorphs) {
            m_redoMorphNames[morph->index] = morph->name;
        }
    }
}

void
Model::writeLoadCommandMessage(Error &error)
{
    Nanoem__Application__RedoLoadModelCommand command = NANOEM__APPLICATION__REDO_LOAD_MODEL_COMMAND__INIT;
    MutableString pathString, fragmentString, nameString, nameBuffer;
    command.content_case = NANOEM__APPLICATION__REDO_LOAD_MODEL_COMMAND__CONTENT_FILE_URI;
    Nanoem__Application__URI uri = NANOEM__APPLICATION__URI__INIT;
    uri.absolute_path = StringUtils::cloneString(m_fileURI.absolutePathConstString(), pathString);
    uri.fragment = StringUtils::cloneString(m_fileURI.fragmentConstString(), fragmentString);
    command.file_uri = &uri;
    command.model_handle = m_handle;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_rsize_t numBones, numMorphs, length;
    nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_opaque, &numBones);
    command.n_bones = numBones;
    command.bones = new Nanoem__Application__Bone *[numBones];
    for (nanoem_rsize_t i = 0; i < numBones; i++) {
        Nanoem__Application__Bone *b = nanoem_new(Nanoem__Application__Bone);
        nanoem__application__bone__init(b);
        const nanoem_model_bone_t *bone = bones[i];
        b->index = nanoemModelObjectGetIndex(nanoemModelBoneGetModelObject(bone));
        b->name = reinterpret_cast<char *>(nanoemUnicodeStringFactoryGetByteArrayEncoding(factory,
            nanoemModelBoneGetName(bone, NANOEM_LANGUAGE_TYPE_FIRST_ENUM), &length, NANOEM_CODEC_TYPE_UTF8, &status));
        command.bones[i] = b;
    }
    nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(m_opaque, &numMorphs);
    command.n_morphs = numMorphs;
    command.morphs = new Nanoem__Application__Morph *[numMorphs];
    for (nanoem_rsize_t i = 0; i < numMorphs; i++) {
        Nanoem__Application__Morph *m = nanoem_new(Nanoem__Application__Morph);
        nanoem__application__morph__init(m);
        const nanoem_model_morph_t *morph = morphs[i];
        m->index = nanoemModelObjectGetIndex(nanoemModelMorphGetModelObject(morph));
        m->name = reinterpret_cast<char *>(nanoemUnicodeStringFactoryGetByteArrayEncoding(factory,
            nanoemModelMorphGetName(morph, NANOEM_LANGUAGE_TYPE_FIRST_ENUM), &length, NANOEM_CODEC_TYPE_UTF8, &status));
        command.morphs[i] = m;
    }
    command.name = StringUtils::cloneString(nameConstString(), nameString);
    Nanoem__Application__Command action = NANOEM__APPLICATION__COMMAND__INIT;
    action.timestamp = stm_now();
    action.type_case = NANOEM__APPLICATION__COMMAND__TYPE_REDO_LOAD_MODEL;
    action.redo_load_model = &command;
    m_project->writeRedoMessage(&action, error);
    for (nanoem_rsize_t i = 0; i < numBones; i++) {
        Nanoem__Application__Bone *b = command.bones[i];
        nanoemUnicodeStringFactoryDestroyByteArray(factory, reinterpret_cast<nanoem_u8_t *>(b->name));
        nanoem_delete(b);
    }
    delete[] command.bones;
    for (nanoem_rsize_t i = 0; i < numMorphs; i++) {
        Nanoem__Application__Morph *m = command.morphs[i];
        nanoemUnicodeStringFactoryDestroyByteArray(factory, reinterpret_cast<nanoem_u8_t *>(m->name));
        nanoem_delete(m);
    }
    delete[] command.morphs;
}

void
Model::writeDeleteCommandMessage(Error &error)
{
    Nanoem__Application__RedoDeleteModelCommand command = NANOEM__APPLICATION__REDO_DELETE_MODEL_COMMAND__INIT;
    command.model_handle = m_handle;
    Nanoem__Application__Command action = NANOEM__APPLICATION__COMMAND__INIT;
    action.timestamp = stm_now();
    action.redo_delete_model = &command;
    action.type_case = NANOEM__APPLICATION__COMMAND__TYPE_REDO_DELETE_MODEL;
    m_project->writeRedoMessage(&action, error);
}

void
Model::clear()
{
    internalClear();
    m_activeBonePairPtr.first = m_activeBonePairPtr.second = nullptr;
    m_activeConstraintPtr = nullptr;
    m_activeEffectPtrPair.first = m_activeEffectPtrPair.second = nullptr;
    m_activeMaterialPtr = nullptr;
    Inline::clearZeroMemory(m_activeMorphPtr);
    m_boneIndexHashes.clear();
    m_bones.clear();
    m_constraints.clear();
    m_inherentBones.clear();
    m_morphs.clear();
    m_redoBoneNames.clear();
    m_redoMorphNames.clear();
    m_outsideParents.clear();
    m_constraintJointBones.clear();
    m_constraintEffectorBones.clear();
    m_boneBoundRigidBodies.clear();
    m_parentBoneTree.clear();
    m_boundingBox.reset();
    m_name = m_comment = m_canonicalName = String();
    nanoemModelDestroy(m_opaque);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_opaque = nanoemModelCreate(m_project->unicodeStringFactory(), &status);
}

void
Model::destroy()
{
    SG_PUSH_GROUPF("Model::destroy(name=%s", canonicalNameConstString());
    setVisible(false);
    undoStackClear(m_undoStack);
    nanoem_delete_safe(m_sharedFallbackBone);
    m_states = 0;
    if (UserDataDestructor destructor = m_userData.second) {
        destructor(m_userData.first, this);
    }
#if defined(__APPLE__)
    if (dispatch_queue_t queue = static_cast<dispatch_queue_t>(m_dispatchParallelTaskQueue)) {
        m_dispatchParallelTaskQueue = 0;
        dispatch_release(queue);
    }
#endif /* __APPLE__ */
    internalClear();
    SG_POP_GROUP();
}

void
Model::synchronizeMotion(const Motion *motion, nanoem_frame_index_t frameIndex, nanoem_f32_t amount,
    PhysicsEngine::SimulationTimingType timing)
{
    const nanoem_motion_model_keyframe_t *keyframe = nullptr;
    bool visible = true;
    if (motion && timing == PhysicsEngine::kSimulationTimingBefore) {
        keyframe = motion->findModelKeyframe(frameIndex);
        if (keyframe) {
            setEdgeColor(glm::make_vec4(nanoemMotionModelKeyframeGetEdgeColor(keyframe)));
            setEdgeSizeScaleFactor(nanoemMotionModelKeyframeGetEdgeScaleFactor(keyframe));
            if (IEffect *effect = activeEffect()) {
                nanoem_rsize_t numParameters;
                nanoem_motion_effect_parameter_t *const *parameters =
                    nanoemMotionModelKeyframeGetAllEffectParameterObjects(keyframe, &numParameters);
                effect->setAllParameterObjects(parameters, numParameters);
            }
        }
        else {
            nanoem_motion_model_keyframe_t *prevKeyframe, *nextKeyframe;
            nanoemMotionSearchClosestModelKeyframes(motion->data(), frameIndex, &prevKeyframe, &nextKeyframe);
            if (prevKeyframe && nextKeyframe) {
                keyframe = prevKeyframe;
                const nanoem_f32_t &coef = Motion::coefficient(prevKeyframe, nextKeyframe, frameIndex);
                setEdgeColor(glm::mix(glm::make_vec4(nanoemMotionModelKeyframeGetEdgeColor(prevKeyframe)),
                    glm::make_vec4(nanoemMotionModelKeyframeGetEdgeColor(nextKeyframe)), coef));
                setEdgeSizeScaleFactor(glm::mix(nanoemMotionModelKeyframeGetEdgeScaleFactor(prevKeyframe),
                    nanoemMotionModelKeyframeGetEdgeScaleFactor(nextKeyframe), coef));
                if (IEffect *effect = activeEffect()) {
                    nanoem_rsize_t numFromParameters, numToParameters;
                    nanoem_motion_effect_parameter_t *const *fromParameters =
                        nanoemMotionModelKeyframeGetAllEffectParameterObjects(prevKeyframe, &numFromParameters);
                    nanoem_motion_effect_parameter_t *const *toParameters =
                        nanoemMotionModelKeyframeGetAllEffectParameterObjects(nextKeyframe, &numToParameters);
                    effect->setAllParameterObjects(
                        fromParameters, numFromParameters, toParameters, numToParameters, coef);
                }
            }
        }
        if (keyframe) {
            visible = nanoemMotionModelKeyframeIsVisible(keyframe) != 0;
            setPhysicsSimulationEnabled(nanoemMotionModelKeyframeIsPhysicsSimulationEnabled(keyframe) != 0);
            setVisible(visible);
            synchronizeAllConstraintStates(keyframe);
            synchronizeAllOutsideParents(keyframe);
        }
        else {
            visible = isVisible();
        }
    }
    if (motion && visible) {
        if (timing == PhysicsEngine::kSimulationTimingBefore) {
            m_boundingBox.reset();
            resetAllMaterials();
            resetAllBoneLocalTransform();
            synchronizeMorphMotion(motion, frameIndex, amount);
            synchronizeBoneMotion(motion, frameIndex, amount, timing);
            solveAllConstraints();
            synchronizeAllRigidBodyKinematics(motion, frameIndex);
            synchronizeAllRigidBodiesTransformFeedbackToSimulation();
        }
        else if (timing == PhysicsEngine::kSimulationTimingAfter) {
            synchronizeBoneMotion(motion, frameIndex, amount, timing);
        }
    }
}

void
Model::synchronizeAllRigidBodiesTransformFeedbackFromSimulation(PhysicsEngine::RigidBodyFollowBoneType followType)
{
    nanoem_rsize_t numRigidBodies;
    nanoem_model_rigid_body_t *const *rigidBodies = nanoemModelGetAllRigidBodyObjects(m_opaque, &numRigidBodies);
    for (nanoem_rsize_t i = 0; i < numRigidBodies; i++) {
        const nanoem_model_rigid_body_t *rigidBodyPtr = rigidBodies[i];
        if (model::RigidBody *rigidBody = model::RigidBody::cast(rigidBodyPtr)) {
            rigidBody->synchronizeTransformFeedbackFromSimulation(rigidBodyPtr, followType);
        }
    }
}

void
Model::synchronizeAllRigidBodiesTransformFeedbackToSimulation()
{
    nanoem_rsize_t numRigidBodies;
    nanoem_model_rigid_body_t *const *rigidBodies = nanoemModelGetAllRigidBodyObjects(m_opaque, &numRigidBodies);
    for (nanoem_rsize_t i = 0; i < numRigidBodies; i++) {
        const nanoem_model_rigid_body_t *rigidBodyPtr = rigidBodies[i];
        if (model::RigidBody *rigidBody = model::RigidBody::cast(rigidBodyPtr)) {
            rigidBody->applyAllForces(rigidBodyPtr);
            rigidBody->synchronizeTransformFeedbackToSimulation(rigidBodyPtr);
        }
    }
}

void
Model::performAllBonesTransform()
{
    applyAllBonesTransform(PhysicsEngine::kSimulationTimingBefore);
    solveAllConstraints();
    PhysicsEngine *engine = m_project->physicsEngine();
    if (engine->mode() == PhysicsEngine::kSimulationModeEnableAnytime) {
        synchronizeAllRigidBodiesTransformFeedbackToSimulation();
        engine->stepSimulation(m_project->physicsSimulationTimeStep());
        synchronizeAllRigidBodiesTransformFeedbackFromSimulation(PhysicsEngine::kRigidBodyFollowBoneSkip);
    }
    applyAllBonesTransform(PhysicsEngine::kSimulationTimingAfter);
    markStagingVertexBufferDirty();
    if (m_camera->followingType() != ICamera::kFollowingTypeNone) {
        m_camera->update();
    }
}

void
Model::performAllMorphsDeform(bool resetAll)
{
    if (resetAll) {
        nanoem_rsize_t numObjects, numChildren;
        nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(m_opaque, &numObjects);
        const Motion *motion = m_project->resolveMotion(this);
        nanoem_frame_index_t currentFrameIndex = m_project->currentLocalFrameIndex();
        model::Morph::Set activeMorphs;
        for (int i = NANOEM_MODEL_MORPH_CATEGORY_FIRST_ENUM; i < NANOEM_MODEL_MORPH_CATEGORY_MAX_ENUM; i++) {
            activeMorphs.insert(activeMorph(static_cast<nanoem_model_morph_category_t>(i)));
        }
        for (nanoem_rsize_t i = 0; i < numObjects; i++) {
            const nanoem_model_morph_t *morphPtr = morphs[i];
            switch (nanoemModelMorphGetType(morphPtr)) {
            case NANOEM_MODEL_MORPH_TYPE_BONE: {
                const nanoem_model_morph_bone_t *const *children =
                    nanoemModelMorphGetAllBoneMorphObjects(morphPtr, &numChildren);
                for (nanoem_rsize_t i = 0; i < numChildren; i++) {
                    const nanoem_model_morph_bone_t *child = children[i];
                    const nanoem_model_bone_t *targetBonePtr = nanoemModelMorphBoneGetBoneObject(child);
                    if (model::Bone *bone = model::Bone::cast(targetBonePtr)) {
                        const nanoem_model_rigid_body_t *rigidBodyPtr = nullptr;
                        BoneBoundRigidBodyMap::const_iterator it = m_boneBoundRigidBodies.find(targetBonePtr);
                        if (it != m_boneBoundRigidBodies.end()) {
                            rigidBodyPtr = it->second;
                        }
                        bone->resetMorphTransform();
                        bone->synchronizeMotion(motion, targetBonePtr, rigidBodyPtr, currentFrameIndex, 0);
                    }
                }
                break;
            }
            case NANOEM_MODEL_MORPH_TYPE_FLIP: {
                const nanoem_model_morph_flip_t *const *children =
                    nanoemModelMorphGetAllFlipMorphObjects(morphPtr, &numChildren);
                for (nanoem_rsize_t i = 0; i < numChildren; i++) {
                    const nanoem_model_morph_flip_t *child = children[i];
                    const nanoem_model_morph_t *targetMorphPtr = nanoemModelMorphFlipGetMorphObject(child);
                    if (activeMorphs.find(targetMorphPtr) == activeMorphs.end()) {
                        if (model::Morph *morph = model::Morph::cast(targetMorphPtr)) {
                            const nanoem_unicode_string_t *name =
                                nanoemModelMorphGetName(targetMorphPtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
                            morph->synchronizeMotion(motion, name, currentFrameIndex, 0);
                        }
                    }
                }
                break;
            }
            case NANOEM_MODEL_MORPH_TYPE_GROUP: {
                const nanoem_model_morph_group_t *const *children =
                    nanoemModelMorphGetAllGroupMorphObjects(morphPtr, &numChildren);
                for (nanoem_rsize_t i = 0; i < numChildren; i++) {
                    const nanoem_model_morph_group_t *child = children[i];
                    const nanoem_model_morph_t *targetMorphPtr = nanoemModelMorphGroupGetMorphObject(child);
                    if (activeMorphs.find(targetMorphPtr) == activeMorphs.end()) {
                        if (model::Morph *morph = model::Morph::cast(targetMorphPtr)) {
                            const nanoem_unicode_string_t *name =
                                nanoemModelMorphGetName(targetMorphPtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
                            morph->synchronizeMotion(motion, name, currentFrameIndex, 0);
                        }
                    }
                }
                break;
            }
            case NANOEM_MODEL_MORPH_TYPE_MATERIAL: {
                const nanoem_model_morph_material_t *const *children =
                    nanoemModelMorphGetAllMaterialMorphObjects(morphPtr, &numChildren);
                for (nanoem_rsize_t i = 0; i < numChildren; i++) {
                    const nanoem_model_morph_material_t *child = children[i];
                    const nanoem_model_material_t *targetMaterialPtr = nanoemModelMorphMaterialGetMaterialObject(child);
                    if (model::Material *material = model::Material::cast(targetMaterialPtr)) {
                        material->reset(targetMaterialPtr);
                    }
                }
                break;
            }
            default:
                break;
            }
        }
    }
    deformAllMorphs(true);
    performAllBonesTransform();
}

void
Model::deformAllMorphs(bool checkDirty)
{
    nanoem_rsize_t numObjects;
    nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(data(), &numObjects);
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        const nanoem_model_morph_t *morphPtr = morphs[i];
        predeformMorph(morphPtr);
    }
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        const nanoem_model_morph_t *morphPtr = morphs[i];
        deformMorph(morphPtr, checkDirty);
    }
}

void
Model::markStagingVertexBufferDirty()
{
    EnumUtils::setEnabled(kPrivateStateDirtyStagingBuffer, m_states, true);
}

void
Model::updateStagingVertexBuffer()
{
    if (EnumUtils::isEnabled(kPrivateStateDirtyStagingBuffer, m_states)) {
        sg_buffer stagingVertexBuffer = m_vertexBuffers[m_stageVertexBufferIndex];
        if (sg::is_valid(stagingVertexBuffer)) {
            SG_PUSH_GROUPF("Model::updateStagingVertexBuffer(name=%s)", canonicalNameConstString());
            if (m_skinDeformer) {
                m_skinDeformer->execute(m_stageVertexBufferIndex);
            }
            else if (nanoem_u8_t *ptr = static_cast<nanoem_u8_t *>(sg::map_buffer(stagingVertexBuffer))) {
                internalUpdateStagingVertexBuffer(ptr, m_vertexBufferData.size() / sizeof(VertexUnit));
                sg::unmap_buffer(stagingVertexBuffer, ptr);
            }
            else if (!m_vertexBufferData.empty()) {
                const size_t size = m_vertexBufferData.size();
                internalUpdateStagingVertexBuffer(m_vertexBufferData.data(), size / sizeof(VertexUnit));
                sg::update_buffer(stagingVertexBuffer, m_vertexBufferData.data(), Inline::saturateInt32(size));
            }
            m_stageVertexBufferIndex = 1 - m_stageVertexBufferIndex;
            SG_POP_GROUP();
        }
        EnumUtils::setEnabled(kPrivateStateDirtyStagingBuffer | kPrivateStateDirtyMorph, m_states, false);
    }
}

void
Model::registerUpdateActiveBoneTransformCommand(const Vector3 &translation, const Quaternion &orientation)
{
    model::Bone::Set bones;
    model::BindPose lastBindPose, currentBindPose;
    const nanoem_model_bone_t *activeBonePtr = activeBone();
    bones.insert(activeBonePtr);
    saveBindPose(lastBindPose);
    if (model::Bone *bone = model::Bone::cast(activeBonePtr)) {
        bone->setLocalUserTranslation(translation);
        bone->setLocalUserOrientation(orientation);
        saveBindPose(currentBindPose);
        pushUndo(command::TransformBoneCommand::create(
            lastBindPose, currentBindPose, ListUtils::toListFromSet(bones), this, m_project));
    }
}

void
Model::registerResetBoneSetTransformCommand(
    const model::Bone::Set &boneSet, const model::BindPose &lastBindPose, ResetType type)
{
    if (!boneSet.empty()) {
        nanoem_rsize_t numObjects;
        model::BindPose currentBindPose;
        nanoem_model_bone_t *const *bones = nanoemModelGetAllOrderedBoneObjects(data(), &numObjects);
        currentBindPose.m_parameters.resize(numObjects);
        for (nanoem_rsize_t i = 0; i < numObjects; i++) {
            const nanoem_model_bone_t *bone = bones[i];
            model::BindPose::Parameter &p = currentBindPose.m_parameters[i];
            p.save(bone);
            if (boneSet.find(bone) != boneSet.end()) {
                switch (type) {
                case kResetTypeTranslationAxisX:
                case kResetTypeTranslationAxisY:
                case kResetTypeTranslationAxisZ: {
                    const int offset = static_cast<int>(type) - static_cast<int>(kResetTypeTranslationAxisX);
                    p.m_localUserTranslation[offset] = 0;
                    break;
                }
                case kResetTypeOrientation: {
                    p.m_localUserOrientation = Constants::kZeroQ;
                    break;
                }
                case kResetTypeOrientationAngleX:
                case kResetTypeOrientationAngleY:
                case kResetTypeOrientationAngleZ: {
                    const int offset = static_cast<int>(type) - static_cast<int>(kResetTypeOrientationAngleX);
                    Vector3 angle(glm::eulerAngles(p.m_localUserOrientation));
                    angle[offset] = 0;
                    p.m_localUserOrientation = angle;
                    break;
                }
                default:
                    break;
                }
            }
        }
        pushUndo(command::TransformBoneCommand::create(
            lastBindPose, currentBindPose, ListUtils::toListFromSet(boneSet), this, m_project));
    }
}

void
Model::registerResetActiveBoneTransformCommand(ResetType type)
{
    model::Bone::Set bones;
    model::BindPose lastBindPose;
    bones.insert(activeBone());
    saveBindPose(lastBindPose);
    registerResetBoneSetTransformCommand(bones, lastBindPose, type);
}

void
Model::registerResetMorphSetWeightsCommand(const model::Morph::Set &morphSet)
{
    if (!morphSet.empty()) {
        command::TransformMorphCommand::WeightStateList weightStates;
        for (model::Morph::Set::const_iterator it = morphSet.begin(), end = morphSet.end(); it != end; ++it) {
            const nanoem_model_morph_t *morphPtr = *it;
            if (const model::Morph *morph = model::Morph::cast(morphPtr)) {
                weightStates.push_back(tinystl::make_pair(morphPtr, tinystl::make_pair(0.0f, morph->weight())));
            }
        }
        model::BindPose currentBindPose;
        saveBindPose(currentBindPose);
        pushUndo(command::TransformMorphCommand::create(weightStates, currentBindPose, this, m_project));
    }
}

void
Model::registerResetAllMorphWeightsCommand()
{
    model::Morph::Set morphSet;
    nanoem_rsize_t numMorphs;
    nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(data(), &numMorphs);
    for (nanoem_rsize_t i = 0; i < numMorphs; i++) {
        morphSet.insert(morphs[i]);
    }
    registerResetMorphSetWeightsCommand(morphSet);
}

void
Model::resetLanguage()
{
    nanoem_rsize_t numObjects;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_opaque, &numObjects);
    nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
    nanoem_language_type_t language = m_project->castLanguage();
    StringUtils::getUtf8String(nanoemModelGetName(m_opaque, language), factory, m_name);
    StringUtils::getUtf8String(nanoemModelGetComment(m_opaque, language), factory, m_comment);
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        const nanoem_model_bone_t *bonePtr = bones[i];
        if (model::Bone *bone = model::Bone::cast(bonePtr)) {
            bone->resetLanguage(bonePtr, factory, language);
            if (const nanoem_model_constraint_t *constraintPtr = nanoemModelBoneGetConstraintObject(bonePtr)) {
                model::Constraint *constraint = model::Constraint::cast(constraintPtr);
                constraint->resetLanguage(constraintPtr, factory, language);
            }
        }
    }
    nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(m_opaque, &numObjects);
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        const nanoem_model_morph_t *morphPtr = morphs[i];
        if (model::Morph *morph = model::Morph::cast(morphPtr)) {
            morph->resetLanguage(morphPtr, factory, language);
        }
    }
    nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(m_opaque, &numObjects);
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        const nanoem_model_material_t *materialPtr = materials[i];
        if (model::Material *material = model::Material::cast(materialPtr)) {
            material->resetLanguage(materialPtr, factory, language);
        }
    }
    nanoem_model_label_t *const *labels = nanoemModelGetAllLabelObjects(m_opaque, &numObjects);
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        const nanoem_model_label_t *labelPtr = labels[i];
        if (model::Label *label = model::Label::cast(labelPtr)) {
            label->resetLanguage(labelPtr, factory, language);
        }
    }
    nanoem_model_constraint_t *const *constraints = nanoemModelGetAllConstraintObjects(m_opaque, &numObjects);
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        const nanoem_model_constraint_t *constraintPtr = constraints[i];
        if (model::Constraint *constraint = model::Constraint::cast(constraintPtr)) {
            constraint->resetLanguage(constraintPtr, factory, language);
        }
    }
    nanoem_model_rigid_body_t *const *rigidBodies = nanoemModelGetAllRigidBodyObjects(m_opaque, &numObjects);
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        const nanoem_model_rigid_body_t *rigidBodyPtr = rigidBodies[i];
        if (model::RigidBody *rigidBody = model::RigidBody::cast(rigidBodyPtr)) {
            rigidBody->resetLanguage(rigidBodyPtr, factory, language);
        }
    }
    nanoem_model_joint_t *const *joints = nanoemModelGetAllJointObjects(m_opaque, &numObjects);
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        const nanoem_model_joint_t *jointPtr = joints[i];
        if (model::Joint *joint = model::Joint::cast(jointPtr)) {
            joint->resetLanguage(jointPtr, factory, language);
        }
    }
}

void
Model::removeBone(const nanoem_model_bone_t *value)
{
    String s;
    const nanoem_unicode_string_t *name = nanoemModelBoneGetName(value, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
    StringUtils::getUtf8String(name, m_project->unicodeStringFactory(), s);
    removeBone(s);
}

void
Model::removeBone(const String &value)
{
    BoneHashMap::const_iterator it = m_bones.find(value);
    if (it != m_bones.end()) {
        const nanoem_model_bone_t *bonePtr = it->second;
        m_bones.erase(it);
        if (const nanoem_model_constraint_t *constraintPtr = nanoemModelBoneGetConstraintObject(bonePtr)) {
            nanoem_rsize_t numJoints;
            nanoem_model_constraint_joint_t *const *joints =
                nanoemModelConstraintGetAllJointObjects(constraintPtr, &numJoints);
            for (nanoem_rsize_t i = 0; i < numJoints; i++) {
                const nanoem_model_constraint_joint_t *joint = joints[i];
                ResolveConstraintJointParentMap::const_iterator it2 =
                    m_constraintJointBones.find(nanoemModelConstraintJointGetBoneObject(joint));
                if (it2 != m_constraintJointBones.end()) {
                    m_constraintJointBones.erase(it2);
                }
            }
            m_constraintEffectorBones.erase(nanoemModelConstraintGetEffectorBoneObject(constraintPtr));
        }
        ConstraintMap::const_iterator it2 = m_constraints.find(bonePtr);
        if (it2 != m_constraints.end()) {
            m_constraints.erase(it2);
        }
        BoneBoundRigidBodyMap::const_iterator it5 = m_boneBoundRigidBodies.find(bonePtr);
        if (it5 != m_boneBoundRigidBodies.end()) {
            m_boneBoundRigidBodies.erase(it5);
        }
        if (m_skinDeformer) {
            m_skinDeformer->rebuildAllBones();
        }
    }
}

void
Model::removeMorph(const nanoem_model_morph_t *value)
{
    String s;
    const nanoem_unicode_string_t *name = nanoemModelMorphGetName(value, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
    StringUtils::getUtf8String(name, m_project->unicodeStringFactory(), s);
    removeMorph(s);
}

void
Model::removeMorph(const String &value)
{
    MorphHashMap::const_iterator it = m_morphs.find(value);
    if (it != m_morphs.end()) {
        m_morphs.erase(it);
    }
}

void
Model::pushUndo(undo_command_t *command)
{
    nanoem_assert(!m_project->isPlaying(), "must not be called while playing");
    if (!m_project->isPlaying()) {
        undoStackPushCommand(undoStack(), command);
        m_project->eventPublisher()->publishUndoChangeEvent();
    }
    else {
        undoCommandDestroy(command);
    }
}

void
Model::solveAllConstraints()
{
    nanoem_rsize_t numConstraints;
    nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
    nanoem_model_constraint_t *const *constraints = nanoemModelGetAllConstraintObjects(m_opaque, &numConstraints);
    for (nanoem_rsize_t i = 0; i < numConstraints; i++) {
        const nanoem_model_constraint_t *constraintPtr = constraints[i];
        if (const model::Constraint *constraint = model::Constraint::cast(constraintPtr)) {
            if (constraint->isEnabled()) {
                const int numIterations = nanoemModelConstraintGetNumIterations(constraintPtr);
                solveConstraint(constraintPtr, numIterations, factory);
            }
            else {
                nanoem_rsize_t numJoints;
                nanoem_model_constraint_joint_t *const *joints =
                    nanoemModelConstraintGetAllJointObjects(constraintPtr, &numJoints);
                for (nanoem_rsize_t i = 0; i < numJoints; i++) {
                    if (model::Bone *jointBone =
                            model::Bone::cast(nanoemModelConstraintJointGetBoneObject(joints[i]))) {
                        jointBone->setConstraintJointOrientation(Constants::kZeroQ);
                    }
                }
            }
        }
    }
}

void
Model::reset()
{
    initializeAllRigidBodiesTransformFeedback();
    initializeAllSoftBodiesTransformFeedback();
    resetAllBoneTransforms();
    resetAllMaterials();
    resetAllMorphs();
    resetAllVertices();
    m_boundingBox.reset();
}

void
Model::draw(DrawType type)
{
    if (isVisible()) {
        switch (type) {
        case IDrawable::kDrawTypeColor:
        case IDrawable::kDrawTypeScriptExternalColor:
        case IDrawable::kDrawTypeVertexWeight: {
            drawColor(type == IDrawable::kDrawTypeScriptExternalColor);
            if (isShowAllVertexFaces()) {
                drawAllVertexFaces();
            }
            if (isShowAllVertexPoints()) {
                drawAllVertexPoints();
            }
            const bool enabled = isPhysicsSimulationEnabled() && m_project->isPhysicsSimulationEnabled();
            if (isShowAllRigidBodies() && enabled) {
                drawAllRigidBodyShapes();
            }
            if (isShowAllJoints() && enabled) {
                drawAllJointShapes();
            }
            break;
        }
        case IDrawable::kDrawTypeEdge: {
            nanoem_f32_t edgeSizeScaleFactor = edgeSize();
            if (edgeSizeScaleFactor > 0) {
                drawEdge(edgeSizeScaleFactor);
            }
            break;
        }
        case IDrawable::kDrawTypeGroundShadow: {
            if (isGroundShadowEnabled()) {
                drawGroundShadow();
            }
            break;
        }
        case IDrawable::kDrawTypeShadowMap: {
            if (isShadowMapEnabled()) {
                drawShadowMap();
            }
            break;
        }
        default:
        case IDrawable::kDrawTypeMaxEnum:
            break;
        }
    }
}

const IEffect *
Model::findOffscreenPassiveRenderTargetEffect(const String &ownerName) const NANOEM_DECL_NOEXCEPT
{
    OffscreenPassiveRenderTargetEffectMap::const_iterator it = m_offscreenPassiveRenderTargetEffects.find(ownerName);
    return it != m_offscreenPassiveRenderTargetEffects.end() ? it->second.m_passiveEffect : nullptr;
}

IEffect *
Model::findOffscreenPassiveRenderTargetEffect(const String &ownerName) NANOEM_DECL_NOEXCEPT
{
    OffscreenPassiveRenderTargetEffectMap::const_iterator it = m_offscreenPassiveRenderTargetEffects.find(ownerName);
    return it != m_offscreenPassiveRenderTargetEffects.end() ? it->second.m_passiveEffect : nullptr;
}

void
Model::setOffscreenDefaultRenderTargetEffect(const String &ownerName)
{
    setOffscreenPassiveRenderTargetEffect(ownerName, m_project->sharedResourceRepository()->modelProgramBundle());
}

void
Model::setOffscreenPassiveRenderTargetEffect(const String &ownerName, IEffect *value)
{
    if (!ownerName.empty() && value) {
        if (ownerName == Effect::kOffscreenOwnerNameMain) {
            setActiveEffect(value);
        }
        else {
            OffscreenPassiveRenderTargetEffectMap::iterator it = m_offscreenPassiveRenderTargetEffects.find(ownerName);
            if (it != m_offscreenPassiveRenderTargetEffects.end()) {
                OffscreenPassiveRenderTargetEffect &effect = it->second;
                effect.m_passiveEffect = value;
            }
            else {
                const OffscreenPassiveRenderTargetEffect effect = { value, true };
                m_offscreenPassiveRenderTargetEffects.insert(tinystl::make_pair(ownerName, effect));
            }
        }
    }
}

void
Model::removeOffscreenPassiveRenderTargetEffect(const String &ownerName)
{
    if (ownerName == Effect::kOffscreenOwnerNameMain) {
        setActiveEffect(nullptr);
    }
    else {
        OffscreenPassiveRenderTargetEffectMap::const_iterator it =
            m_offscreenPassiveRenderTargetEffects.find(ownerName);
        if (it != m_offscreenPassiveRenderTargetEffects.end()) {
            m_offscreenPassiveRenderTargetEffects.erase(it);
        }
    }
}

bool
Model::isOffscreenPassiveRenderTargetEffectEnabled(const String &ownerName) const NANOEM_DECL_NOEXCEPT
{
    OffscreenPassiveRenderTargetEffectMap::const_iterator it = m_offscreenPassiveRenderTargetEffects.find(ownerName);
    return it != m_offscreenPassiveRenderTargetEffects.end() ? it->second.m_enabled : false;
}

void
Model::setOffscreenPassiveRenderTargetEffectEnabled(const String &ownerName, bool value)
{
    if (!ownerName.empty()) {
        OffscreenPassiveRenderTargetEffectMap::iterator it = m_offscreenPassiveRenderTargetEffects.find(ownerName);
        if (it != m_offscreenPassiveRenderTargetEffects.end()) {
            it->second.m_enabled = value;
        }
        else {
            OffscreenPassiveRenderTargetEffect effect = { nullptr, true };
            m_offscreenPassiveRenderTargetEffects.insert(tinystl::make_pair(ownerName, effect));
        }
    }
}

bool
Model::isBoneSelectable(const nanoem_model_bone_t *value) const NANOEM_DECL_NOEXCEPT
{
    return isShowAllBones() || model::Bone::isSelectable(value);
}

bool
Model::isMaterialSelected(const nanoem_model_material_t *material) const NANOEM_DECL_NOEXCEPT
{
    return !m_activeMaterialPtr || m_activeMaterialPtr == material || m_selection->containsMaterial(material);
}

bool
Model::isBoneConnectionDrawable(const nanoem_model_bone_t *bone) const NANOEM_DECL_NOEXCEPT
{
    return model::Bone::isSelectable(bone) && !isRigidBodyBound(bone);
}

void
Model::drawBoneConnections(
    const nanoem_model_bone_t *bone, const nanoem_model_bone_t *parentBone, nanoem_f32_t thickness)
{
    if (const model::Bone *toBone = model::Bone::cast(parentBone)) {
        const Vector3 destinationPosition(worldTransform(toBone->worldTransform())[3]);
        if (isBoneConnectionDrawable(bone) && isBoneConnectionDrawable(parentBone)) {
            const Vector4 color(connectionBoneColor(bone, Vector4(0, 0, 1, 1), false));
            drawBoneConnection(bone, destinationPosition, color, m_project->deviceScaleCircleRadius(), thickness);
        }
        else if (isShowAllBones()) {
            const Vector4 color(connectionBoneColor(bone, Vector4(0.25f, 0.25f, 0.25f, 1), false));
            drawBoneConnection(bone, destinationPosition, color, m_project->deviceScaleCircleRadius(), thickness);
        }
    }
}

void
Model::drawBoneConnections(const Vector2 &deviceScaleCursor)
{
    nanoem_rsize_t numBones;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_opaque, &numBones);
    nanoem_f32_t circleRadius = m_project->deviceScaleCircleRadius();
    nanoem_f32_t thickness = m_project->windowDevicePixelRatio();
    for (nanoem_rsize_t i = 0; i < numBones; i++) {
        const nanoem_model_bone_t *bone = bones[i];
        if (const nanoem_model_bone_t *targetBone = nanoemModelBoneGetTargetBoneObject(bone)) {
            drawBoneConnections(bone, targetBone, thickness);
        }
        else if (isShowAllBones() || isBoneConnectionDrawable(bone)) {
            const nanoem_f32_t *v = nanoemModelBoneGetDestinationOrigin(bone);
            const Matrix4x4 transform(worldTransform(model::Bone::cast(bone)->worldTransform()));
            const Vector3 destinationPositon((Matrix3x3(transform) * glm::make_vec3(v)) + Vector3(transform[3]));
            const Vector4 color(connectionBoneColor(bone, Vector4(0, 0, 1, 1), false));
            drawBoneConnection(bone, destinationPositon, color, circleRadius, thickness);
        }
    }
    for (nanoem_rsize_t i = 0; i < numBones; i++) {
        const nanoem_model_bone_t *bone = bones[i];
        const bool selected = m_selection->containsBone(bone);
        if (isBoneConnectionDrawable(bone)) {
            const Vector4 inactive(connectionBoneColor(bone, Vector4(0, 0, 1, 1), true));
            const Vector4 hovered(hoveredBoneColor(inactive, selected));
            drawBonePoint(deviceScaleCursor, bone, inactive, hovered);
        }
        else if (isShowAllBones()) {
            const Vector4 inactive(connectionBoneColor(bone, Vector4(0.25f, 0.25f, 0.25f, 1), true));
            const Vector4 hovered(hoveredBoneColor(inactive, selected));
            drawBonePoint(deviceScaleCursor, bone, inactive, hovered);
        }
    }
}

void
Model::clearAllBoneBoundsRigidBodies()
{
    m_boneBoundRigidBodies.clear();
}

void
Model::createAllBoneBoundsRigidBodies()
{
    nanoem_rsize_t numRigidBodies;
    nanoem_model_rigid_body_t *const *rigidBodies = nanoemModelGetAllRigidBodyObjects(m_opaque, &numRigidBodies);
    for (nanoem_rsize_t i = 0; i < numRigidBodies; i++) {
        const nanoem_model_rigid_body_t *rigidBody = rigidBodies[i];
        nanoem_model_rigid_body_transform_type_t type = nanoemModelRigidBodyGetTransformType(rigidBody);
        if (type != NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_FROM_BONE_TO_SIMULATION) {
            m_boneBoundRigidBodies.insert(tinystl::make_pair(nanoemModelRigidBodyGetBoneObject(rigidBody), rigidBody));
        }
    }
}

bool
Model::intersectsBoneInWindow(
    const Vector2 &deviceScaleCursor, const model::Bone *bone, Vector2 &coord) const NANOEM_DECL_NOEXCEPT
{
    const Vector3 position(worldTransform(bone->worldTransform())[3]);
    coord = m_project->activeCamera()->toDeviceScreenCoordinateInWindow(position);
    return glm::distance(coord, deviceScaleCursor) < m_project->deviceScaleCircleRadius();
}

bool
Model::intersectsBoneInViewport(
    const Vector2 &deviceScaleCursor, const model::Bone *bone, Vector2 &coord) const NANOEM_DECL_NOEXCEPT
{
    const Vector3 position(worldTransform(bone->worldTransform())[3]);
    coord = m_project->activeCamera()->toDeviceScreenCoordinateInViewport(position);
    return glm::distance(coord, deviceScaleCursor) < m_project->deviceScaleCircleRadius();
}

void
Model::saveBindPose(model::BindPose &value) const
{
    nanoem_rsize_t numObjects;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllOrderedBoneObjects(m_opaque, &numObjects);
    value.m_parameters.resize(numObjects);
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        const nanoem_model_bone_t *bone = bones[i];
        if (!isRigidBodyBound(bone)) {
            value.m_parameters[i].save(bone);
        }
    }
}

void
Model::restoreBindPose(const model::BindPose &value)
{
    nanoem_rsize_t numObjects;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllOrderedBoneObjects(m_opaque, &numObjects);
    if (value.m_parameters.size() == numObjects) {
        for (nanoem_rsize_t i = 0; i < numObjects; i++) {
            const nanoem_model_bone_t *bonePtr = bones[i];
            if (model::Bone *bone = model::Bone::cast(bonePtr)) {
                bone->resetLocalTransform();
                value.m_parameters[i].restore(bonePtr);
            }
        }
    }
}

void
Model::resetAllBoneTransforms()
{
    nanoem_rsize_t numBones;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllOrderedBoneObjects(m_opaque, &numBones);
    for (nanoem_rsize_t i = 0; i < numBones; i++) {
        const nanoem_model_bone_t *bonePtr = bones[i];
        if (model::Bone *bone = model::Bone::cast(bonePtr)) {
            bone->resetLocalTransform();
            bone->resetMorphTransform();
            bone->resetUserTransform();
        }
    }
}

void
Model::resetAllBoneLocalTransform()
{
    nanoem_rsize_t numBones;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllOrderedBoneObjects(m_opaque, &numBones);
    for (nanoem_rsize_t i = 0; i < numBones; i++) {
        const nanoem_model_bone_t *bonePtr = bones[i];
        if (model::Bone *bone = model::Bone::cast(bonePtr)) {
            bone->resetLocalTransform();
        }
    }
}

void
Model::resetAllBoneMorphTransform()
{
    nanoem_rsize_t numBones;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllOrderedBoneObjects(m_opaque, &numBones);
    for (nanoem_rsize_t i = 0; i < numBones; i++) {
        const nanoem_model_bone_t *bonePtr = bones[i];
        if (model::Bone *bone = model::Bone::cast(bonePtr)) {
            bone->resetLocalTransform();
            bone->resetMorphTransform();
        }
    }
}

void
Model::resetAllMaterials()
{
    nanoem_rsize_t numMaterials;
    nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(m_opaque, &numMaterials);
    for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
        const nanoem_model_material_t *materialPtr = materials[i];
        if (model::Material *material = model::Material::cast(materialPtr)) {
            material->reset(materialPtr);
        }
    }
}

void
Model::resetAllMorphs()
{
    nanoem_rsize_t numObjects;
    nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(m_opaque, &numObjects);
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        const nanoem_model_morph_t *morphPtr = morphs[i];
        if (model::Morph *morph = model::Morph::cast(morphPtr)) {
            morph->reset();
        }
    }
}

void
Model::resetAllVertices()
{
    nanoem_rsize_t numObjects;
    nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(m_opaque, &numObjects);
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        if (model::Vertex *vertex = model::Vertex::cast(vertices[i])) {
            vertex->reset();
        }
    }
}

void
Model::initializeAllRigidBodiesTransformFeedback()
{
    nanoem_rsize_t numRigidBodies;
    nanoem_model_rigid_body_t *const *bodies = nanoemModelGetAllRigidBodyObjects(m_opaque, &numRigidBodies);
    for (nanoem_rsize_t i = 0; i < numRigidBodies; i++) {
        const nanoem_model_rigid_body_t *rigidBodyPtr = bodies[i];
        if (model::RigidBody *rigidBody = model::RigidBody::cast(rigidBodyPtr)) {
            rigidBody->initializeTransformFeedback(rigidBodyPtr);
        }
    }
}

void
Model::initializeAllSoftBodiesTransformFeedback()
{
    nanoem_rsize_t numSoftBodies;
    nanoem_model_soft_body_t *const *softBodies = nanoemModelGetAllSoftBodyObjects(m_opaque, &numSoftBodies);
    for (nanoem_rsize_t i = 0; i < numSoftBodies; i++) {
        const nanoem_model_soft_body_t *softBodyPtr = softBodies[i];
        if (model::SoftBody *softBody = model::SoftBody::cast(softBodyPtr)) {
            softBody->initializeTransformFeedback();
        }
    }
}

void
Model::setRigidBodiesVisualization(const model::RigidBody::VisualizationClause &clause)
{
    nanoem_rsize_t numRigidBodies;
    nanoem_model_rigid_body_t *const *rigidBodies = nanoemModelGetAllRigidBodyObjects(m_opaque, &numRigidBodies);
    for (nanoem_rsize_t i = 0; i < numRigidBodies; i++) {
        const nanoem_model_rigid_body_t *rigidBodyPtr = rigidBodies[i];
        if (model::RigidBody *rigidBody = model::RigidBody::cast(rigidBodyPtr)) {
            bool enabled = true;
            if (clause.group <= 0xffff) {
                enabled &= EnumUtils::isEnabled(
                    1 << glm::clamp(nanoemModelRigidBodyGetCollisionGroupId(rigidBodyPtr), 0, 15), clause.group);
            }
            if (clause.shapeType != NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_UNKNOWN) {
                enabled &= nanoemModelRigidBodyGetShapeType(rigidBodyPtr) == clause.shapeType;
            }
            if (clause.transformType != NANOEM_MODEL_RIGID_BODY_TRANSFORM_TYPE_UNKNOWN) {
                enabled &= nanoemModelRigidBodyGetTransformType(rigidBodyPtr) == clause.transformType;
            }
            rigidBody->physicsEngine()->setVisualizedEnabled(rigidBody->physicsRigidBody(), enabled);
        }
    }
}

void
Model::setSoftBodiesVisualization()
{
    nanoem_rsize_t numSoftBodies;
    nanoem_model_soft_body_t *const *softBodies = nanoemModelGetAllSoftBodyObjects(m_opaque, &numSoftBodies);
    bool enabled = true;
    for (nanoem_rsize_t i = 0; i < numSoftBodies; i++) {
        const nanoem_model_soft_body_t *softBodyPtr = softBodies[i];
        if (model::SoftBody *softBody = model::SoftBody::cast(softBodyPtr)) {
            softBody->physicsEngine()->setVisualizedEnabled(softBody->physicsSoftBody(), enabled);
        }
    }
}

void
Model::rename(const String &value, nanoem_language_type_t language)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
    nanoem_mutable_model_t *model = nanoemMutableModelCreateAsReference(m_opaque, &status);
    StringUtils::UnicodeStringScope scope(factory);
    if (StringUtils::tryGetString(factory, value, scope)) {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoemMutableModelSetName(model, scope.value(), language, &status);
    }
    nanoemMutableModelDestroy(model);
    StringUtils::getUtf8String(nanoemModelGetName(m_opaque, NANOEM_LANGUAGE_TYPE_FIRST_ENUM), factory, m_canonicalName);
}

void
Model::addAttachment(const String &name, const URI &fullPath)
{
    m_attachmentURIs.insert(tinystl::make_pair(name, fullPath));
}

void
Model::removeAttachment(const String &name)
{
    FileEntityMap::const_iterator it = m_attachmentURIs.find(name);
    if (it != m_attachmentURIs.end()) {
        m_attachmentURIs.erase(it);
    }
}

FileEntityMap
Model::attachments() const
{
    return m_attachmentURIs;
}

model::Bone::OutsideParentMap
Model::allOutsideParents() const
{
    return m_outsideParents;
}

StringPair
Model::findOutsideParent(const nanoem_model_bone_t *key) const
{
    model::Bone::OutsideParentMap::const_iterator it = m_outsideParents.find(key);
    return it != m_outsideParents.end() ? it->second : StringPair();
}

bool
Model::hasOutsideParent(const nanoem_model_bone_t *key) const NANOEM_DECL_NOEXCEPT
{
    return m_outsideParents.find(key) != m_outsideParents.end();
}

void
Model::setOutsideParent(const nanoem_model_bone_t *key, const StringPair &value)
{
    internalSetOutsideParent(key, value);
    setDirty(true);
}

void
Model::removeOutsideParent(const nanoem_model_bone_t *key)
{
    model::Bone::OutsideParentMap::const_iterator it = m_outsideParents.find(key);
    if (it != m_outsideParents.end()) {
        m_outsideParents.erase(it);
        setDirty(true);
    }
}

sg_image *
Model::uploadImage(const String &filename, const sg_image_desc &desc)
{
    SG_PUSH_GROUPF("Model::uploadImage(name=%s, width=%d, height=%d)", filename.c_str(), desc.width, desc.height);
    sg_image *handlePtr = nullptr;
    ImageMap::iterator it = m_imageHandles.find(filename);
    if (it != m_imageHandles.end()) {
        Image *image = it->second;
        sg_image &handleRef = image->m_handle;
        sg::destroy_image(handleRef);
        if (Inline::isDebugLabelEnabled()) {
            char label[Inline::kMarkerStringLength];
            StringUtils::format(label, sizeof(label), "Models/%s/%s", canonicalNameConstString(), filename.c_str());
            sg_image_desc newDesc(desc);
            newDesc.label = label;
            handleRef = sg::make_image(&newDesc);
            handlePtr = &handleRef;
            SG_LABEL_IMAGE(handleRef, label);
        }
        else {
            handleRef = sg::make_image(&desc);
            handlePtr = &handleRef;
        }
        ImageLoader::copyImageDescrption(desc, image);
        nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
        StringUtils::UnicodeStringScope scope(factory);
        StringUtils::tryGetString(factory, filename, scope);
        nanoem_rsize_t numMaterials;
        nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(m_opaque, &numMaterials);
        for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
            const nanoem_model_material_t *materialPtr = materials[i];
            if (!nanoemModelMaterialIsToonShared(materialPtr)) {
                const nanoem_model_texture_t *texture = nanoemModelMaterialGetToonTextureObject(materialPtr);
                const nanoem_unicode_string_t *texturePath = nanoemModelTextureGetPath(texture);
                if (nanoemUnicodeStringFactoryCompareString(factory, scope.value(), texturePath) == 0) {
                    if (model::Material *material = model::Material::cast(materialPtr)) {
                        /* fetch left-bottom corner pixel */
                        const nanoem_rsize_t offset = nanoem_rsize_t(glm::max(desc.height - 1, 0)) * desc.width * 4;
                        const nanoem_u8_t *dataPtr = static_cast<const nanoem_u8_t *>(desc.data.subimage[0][0].ptr);
                        const Vector4 toonColor(glm::make_vec4(dataPtr + offset));
                        material->setToonColor(toonColor / Vector4(0xff));
                    }
                }
            }
        }
        BX_TRACE("The image is allocated: name=%s ID=%d", filename.c_str(), handleRef.id);
    }
    SG_POP_GROUP();
    return handlePtr;
}

int
Model::compareBoneVertexList(const void *a, const void *b)
{
    const BoneVertexPair *left = static_cast<const BoneVertexPair *>(a);
    const BoneVertexPair *right = static_cast<const BoneVertexPair *>(b);
    return Inline::saturateInt32(right->second.size()) - Inline::saturateInt32(left->second.size());
}

void
Model::handlePerformSkinningVertexTransform(void *opaque, size_t index)
{
    const ParallelSkinningTaskData *s = static_cast<const ParallelSkinningTaskData *>(opaque);
    model::Vertex *vertex = model::Vertex::cast(s->m_vertices[index]);
    VertexUnit &p = reinterpret_cast<VertexUnit *>(s->m_output)[index];
    switch (s->m_drawType) {
    case IDrawable::kDrawTypeColor:
    case IDrawable::kDrawTypeEdge:
    case IDrawable::kDrawTypeGroundShadow:
    case IDrawable::kDrawTypeShadowMap:
    case IDrawable::kDrawTypeScriptExternalColor:
        p.performSkinning(s->m_edgeSizeScaleFactor, vertex);
        vertex->reset();
        break;
    case IDrawable::kDrawTypeVertexWeight:
        p.setWeightColor(model::Bone::cast(s->m_model->activeBone()), vertex);
        vertex->reset();
        break;
    default:
        break;
    }
}

void
Model::setCommonPipelineDescription(sg_pipeline_desc &desc)
{
    sg_layout_desc &ld = desc.layout;
    ld.buffers[0].stride = sizeof(VertexUnit);
    ld.attrs[1] =
        sg_vertex_attr_desc { 0, Inline::saturateInt32(offsetof(VertexUnit, m_normal)), SG_VERTEXFORMAT_FLOAT3 };
    ld.attrs[2] =
        sg_vertex_attr_desc { 0, Inline::saturateInt32(offsetof(VertexUnit, m_texcoord)), SG_VERTEXFORMAT_FLOAT2 };
    ld.attrs[3] =
        sg_vertex_attr_desc { 0, Inline::saturateInt32(offsetof(VertexUnit, m_uva[0])), SG_VERTEXFORMAT_FLOAT4 };
    ld.attrs[4] =
        sg_vertex_attr_desc { 0, Inline::saturateInt32(offsetof(VertexUnit, m_uva[1])), SG_VERTEXFORMAT_FLOAT4 };
    ld.attrs[5] =
        sg_vertex_attr_desc { 0, Inline::saturateInt32(offsetof(VertexUnit, m_uva[2])), SG_VERTEXFORMAT_FLOAT4 };
    ld.attrs[6] =
        sg_vertex_attr_desc { 0, Inline::saturateInt32(offsetof(VertexUnit, m_uva[3])), SG_VERTEXFORMAT_FLOAT4 };
    desc.index_type = SG_INDEXTYPE_UINT32;
    Project::setStandardDepthStencilState(desc.depth, desc.stencil);
}

const IEffect *
Model::activeEffect(const model::Material *material) const NANOEM_DECL_NOEXCEPT
{
    const IEffect *effect = material->effect();
    return effect ? effect : m_activeEffectPtrPair.first;
}

IEffect *
Model::activeEffect(model::Material *material)
{
    IEffect *effect = material->effect();
    return effect ? effect : m_activeEffectPtrPair.first;
}

IEffect *
Model::internalEffect(model::Material *material)
{
    IEffect *passiveEffectPtr = passiveEffect();
    return passiveEffectPtr ? passiveEffectPtr : activeEffect(material);
}

const Image *
Model::createImage(const nanoem_unicode_string_t *path, sg_wrap wrap, nanoem_u32_t flags)
{
    nanoem_parameter_assert(path, "must not be nullptr");
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_rsize_t length;
    nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
    const Image *imagePtr = nullptr;
    if (nanoem_u8_t *utf8Path = nanoemUnicodeStringFactoryGetByteArray(factory, path, &length, &status)) {
        String filename;
        FileUtils::canonicalizePathSeparator(reinterpret_cast<const char *>(utf8Path), filename);
        nanoemUnicodeStringFactoryDestroyByteArray(factory, utf8Path);
        if (!filename.empty()) {
            ImageMap::const_iterator it = m_imageHandles.find(filename);
            if (it != m_imageHandles.end()) {
                imagePtr = it->second;
            }
            else {
                const URI &imageURI = Project::resolveArchiveURI(resolvedFileURI(), filename);
                Image *image = nanoem_new(Image);
                image->m_filename = filename;
                image->m_handle = { SG_INVALID_ID };
                Inline::clearZeroMemory(image->m_description);
                m_imageHandles.insert(tinystl::make_pair(filename, image));
                m_imageURIs.insert(tinystl::make_pair(filename, imageURI));
                m_loadingImageItems.push_back(nanoem_new(LoadingImageItem(imageURI, filename, wrap, flags)));
                imagePtr = image;
            }
        }
    }
    return imagePtr;
}

internal::LineDrawer *
Model::lineDrawer()
{
    if (!m_drawer) {
        m_drawer = nanoem_new(internal::LineDrawer(m_project));
        m_drawer->initialize();
    }
    return m_drawer;
}

void
Model::splitBonesPerMaterial(model::Material::BoneIndexHashMap &boneIndexHash) const
{
    typedef tinystl::unordered_map<int, model::Vertex::Set *, TinySTLAllocator> VertexReference;
    nanoem_rsize_t numMaterials, numIndices, numVertices, offset = 0;
    int uniqueBoneIndexPerMaterial = 0;
    VertexReference references;
    Int32HashMap indexHash;
    nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(m_opaque, &numMaterials);
    nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(m_opaque, &numVertices);
    const nanoem_u32_t *indices = nanoemModelGetAllVertexIndices(m_opaque, &numIndices);
    m_countVertexSkinningNeeded = 0;
    for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
        const nanoem_model_material_t *material = materials[i];
        nanoem_rsize_t nindices = nanoemModelMaterialGetNumVertexIndices(material);
        for (nanoem_rsize_t j = offset; j < offset + nindices; j++) {
            nanoem_u32_t vertexIndex = indices[j];
            const nanoem_model_vertex_t *vertex = vertices[vertexIndex];
            for (nanoem_rsize_t k = 0; k < 4; k++) {
                const nanoem_model_bone_t *bone = nanoemModelVertexGetBoneObject(vertex, k);
                int boneIndex = nanoemModelObjectGetIndex(nanoemModelBoneGetModelObject(bone));
                if (boneIndex >= 0) {
                    if (indexHash.insert(tinystl::make_pair(boneIndex, uniqueBoneIndexPerMaterial)).second) {
                        uniqueBoneIndexPerMaterial++;
                    }
                    VertexReference::const_iterator it = references.find(boneIndex);
                    if (it != references.end()) {
                        it->second->insert(vertex);
                    }
                    else {
                        model::Vertex::Set *set = nanoem_new(model::Vertex::Set);
                        references.insert(tinystl::make_pair(boneIndex, set));
                        set->insert(vertex);
                    }
                }
            }
        }
        if (!indexHash.empty()) {
            if (references.size() > kMaxBoneUniforms) {
                model::Vertex::List vertexList;
                tinystl::vector<BoneVertexPair, TinySTLAllocator> boneVertexList;
                for (VertexReference::const_iterator it = references.begin(), end = references.end(); it != end; ++it) {
                    vertexList.clear();
                    for (model::Vertex::Set::const_iterator it2 = it->second->begin(), end2 = it->second->end();
                         it2 != end2; ++it2) {
                        vertexList.push_back(*it2);
                    }
                    boneVertexList.push_back(tinystl::make_pair(it->first, vertexList));
                }
                qsort(boneVertexList.data(), boneVertexList.size(), sizeof(boneVertexList[0]),
                    &Model::compareBoneVertexList);
                indexHash.clear();
                for (size_t j = 0; j < kMaxBoneUniforms; j++) {
                    const BoneVertexPair &pair = boneVertexList[j];
                    indexHash.insert(tinystl::make_pair(pair.first, nanoem_i32_t(j)));
                }
                for (tinystl::vector<BoneVertexPair, TinySTLAllocator>::const_iterator it = boneVertexList.begin(),
                                                                                       end = boneVertexList.end();
                     it != end; ++it) {
                    const BoneVertexPair &pair = *it;
                    const model::Vertex::List &allVertices = pair.second;
                    for (model::Vertex::List::const_iterator it2 = allVertices.begin(), end2 = allVertices.end();
                         it2 != end2; ++it2) {
                        if (model::Vertex *vertex = model::Vertex::cast(*it2)) {
                            vertex->setSkinningEnabled(true);
                        }
                        m_countVertexSkinningNeeded++;
                    }
                }
            }
            boneIndexHash.insert(tinystl::make_pair(material, indexHash));
        }
        for (VertexReference::const_iterator it = references.begin(), end = references.end(); it != end; ++it) {
            it->second->clear();
            nanoem_delete(it->second);
        }
        offset += nindices;
        uniqueBoneIndexPerMaterial = 0;
        indexHash.clear();
        references.clear();
    }
}

void
Model::bindConstraint(nanoem_model_constraint_t *constraintPtr)
{
    model::Constraint *constriant = model::Constraint::create();
    constriant->bind(constraintPtr);
    constriant->resetLanguage(constraintPtr, m_project->unicodeStringFactory(), m_project->castLanguage());
    constriant->initialize(constraintPtr);
    nanoem_rsize_t numJoints;
    nanoem_model_constraint_joint_t *const *joints = nanoemModelConstraintGetAllJointObjects(constraintPtr, &numJoints);
    const nanoem_model_constraint_t *constraintConstPtr = constraintPtr;
    for (nanoem_rsize_t i = 0; i < numJoints; i++) {
        const nanoem_model_constraint_joint_t *joint = joints[i];
        m_constraintJointBones.insert(
            tinystl::make_pair(nanoemModelConstraintJointGetBoneObject(joint), constraintConstPtr));
    }
    m_constraintEffectorBones.insert(nanoemModelConstraintGetEffectorBoneObject(constraintPtr));
}

void
Model::applyAllBonesTransform(PhysicsEngine::SimulationTimingType timing)
{
    nanoem_rsize_t numObjects;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllOrderedBoneObjects(m_opaque, &numObjects);
    if (timing == PhysicsEngine::kSimulationTimingBefore) {
        m_boundingBox.reset();
    }
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        const nanoem_model_bone_t *bonePtr = bones[i];
        if ((nanoemModelBoneIsAffectedByPhysicsSimulation(bonePtr) != 0) == timing) {
            if (model::Bone *bone = model::Bone::cast(bonePtr)) {
                bone->applyAllLocalTransform(bonePtr, this);
                bone->applyOutsideParentTransform(bonePtr, this);
                m_boundingBox.set(bone->worldTransformOrigin());
            }
        }
    }
}

void
Model::internalClear()
{
    m_selection->clearAll();
    nanoem_rsize_t numMaterials, numBodies, numJoints;
    nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(m_opaque, &numMaterials);
    for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
        if (model::Material *material = model::Material::cast(materials[i])) {
            material->destroy();
        }
    }
    nanoem_model_joint_t *const *joints = nanoemModelGetAllJointObjects(m_opaque, &numJoints);
    for (nanoem_rsize_t i = 0; i < numJoints; i++) {
        if (model::Joint *joint = model::Joint::cast(joints[i])) {
            joint->destroy();
        }
    }
    nanoem_model_rigid_body_t *const *bodies = nanoemModelGetAllRigidBodyObjects(m_opaque, &numBodies);
    for (nanoem_rsize_t i = 0; i < numBodies; i++) {
        if (model::RigidBody *rigidBody = model::RigidBody::cast(bodies[i])) {
            rigidBody->destroy();
        }
    }
    for (ImageMap::const_iterator it = m_imageHandles.begin(), end = m_imageHandles.end(); it != end; ++it) {
        SG_INSERT_MARKERF("Model::internalClear(image=%d, name=%s)", it->second->m_handle.id, it->first.c_str());
        sg::destroy_image(it->second->m_handle);
        nanoem_delete(it->second);
    }
    SG_INSERT_MARKERF("Model::internalClear(vertex0=%d, vertex1=%d, index=%d)", m_vertexBuffers[0].id,
        m_vertexBuffers[1].id, m_indexBuffer.id);
    if (m_skinDeformer) {
        m_skinDeformer->destroy(m_vertexBuffers[0], 0);
        m_skinDeformer->destroy(m_vertexBuffers[1], 1);
        nanoem_delete_safe(m_skinDeformer);
    }
    else {
        sg::destroy_buffer(m_vertexBuffers[0]);
        sg::destroy_buffer(m_vertexBuffers[1]);
    }
    m_vertexBuffers[0] = { SG_INVALID_ID };
    m_vertexBuffers[1] = { SG_INVALID_ID };
    sg::destroy_buffer(m_indexBuffer);
    m_indexBuffer = { SG_INVALID_ID };
    m_drawAllLines.destroy();
    m_drawAllPoints.destroy();
    for (RigidBodyBuffers::iterator it = m_drawRigidBody.begin(), end = m_drawRigidBody.end(); it != end; ++it) {
        it->second.destroy();
    }
    m_drawRigidBody.clear();
    for (JointBuffers::iterator it = m_drawJoint.begin(), end = m_drawJoint.end(); it != end; ++it) {
        it->second.destroy();
    }
    m_drawJoint.clear();
    m_imageHandles.clear();
}

void
Model::internalSetOutsideParent(const nanoem_model_bone_t *key, const StringPair &value)
{
    m_outsideParents[key] = value;
    if (!activeOutsideParentSubjectBone()) {
        setActiveOutsideParentSubjectBone(key);
    }
}

void
Model::createVertexIndexBuffers()
{
    initializeVertexBuffers();
    nanoem_rsize_t numIndices;
    const nanoem_u32_t *indices = nanoemModelGetAllVertexIndices(m_opaque, &numIndices);
    sg_buffer_desc desc;
    Inline::clearZeroMemory(desc);
    desc.type = SG_BUFFERTYPE_INDEXBUFFER;
    desc.usage = SG_USAGE_IMMUTABLE;
    sg::destroy_buffer(m_indexBuffer);
    if (numIndices > 0) {
        desc.data.ptr = indices;
        desc.data.size = desc.size = numIndices * sizeof(*indices);
        char label[Inline::kMarkerStringLength];
        if (Inline::isDebugLabelEnabled()) {
            StringUtils::format(label, sizeof(label), "Models/%s/IndexBuffer", canonicalNameConstString());
            desc.label = label;
        }
        else {
            *label = 0;
        }
        m_indexBuffer = sg::make_buffer(&desc);
        nanoem_assert(sg::query_buffer_state(m_indexBuffer) == SG_RESOURCESTATE_VALID, "index buffer must be valid");
        SG_LABEL_BUFFER(m_indexBuffer, label);
    }
    else {
        static const int kDummyIndex = 0;
        desc.data.ptr = &kDummyIndex;
        desc.data.size = desc.size = sizeof(kDummyIndex);
        m_indexBuffer = sg::make_buffer(&desc);
    }
}

void
Model::initializeVertexBuffers()
{
    nanoem_rsize_t numVertices;
    nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(m_opaque, &numVertices);
    sg_buffer_desc desc;
    bool initialized = sg::is_valid(m_vertexBuffers[0]) && sg::is_valid(m_vertexBuffers[1]);
    Inline::clearZeroMemory(desc);
    desc.usage = SG_USAGE_STREAM;
    if (numVertices > 0 && !initialized) {
        char label[Inline::kMarkerStringLength];
        Project::ISkinDeformerFactory *factory = m_project->skinDeformerFactory();
        nanoem_rsize_t numSoftBodies;
        nanoemModelGetAllSoftBodyObjects(m_opaque, &numSoftBodies);
        /* skin deformer doesn't support softbody currently due to vertex transform */
        if (factory && numSoftBodies == 0) {
            if (model::ISkinDeformer *skinDeformer = factory->create(this)) {
                ByteArray vertexBufferData(numVertices * sizeof(VertexUnit));
                for (nanoem_rsize_t i = 0; i < numVertices; i++) {
                    if (const model::Vertex *v = model::Vertex::cast(vertices[i])) {
                        VertexUnit &unit = reinterpret_cast<VertexUnit *>(vertexBufferData.data())[i];
                        unit.m_position = unit.m_edge = v->m_simd.m_origin;
                        unit.m_normal = v->m_simd.m_normal;
                        unit.m_texcoord = v->m_simd.m_texcoord;
                        unit.m_weights = v->m_simd.m_weights;
                        unit.m_indices = v->m_simd.m_indices;
                        unit.m_info = v->m_simd.m_info;
                        unit.m_uva[0] = v->m_simd.m_originUVA[0];
                        unit.m_uva[1] = v->m_simd.m_originUVA[1];
                        unit.m_uva[2] = v->m_simd.m_originUVA[2];
                        unit.m_uva[3] = v->m_simd.m_originUVA[3];
                    }
                }
                desc.data.ptr = vertexBufferData.data();
                desc.data.size = desc.size = vertexBufferData.size();
                if (Inline::isDebugLabelEnabled()) {
                    StringUtils::format(
                        label, sizeof(label), "Models/%s/VertexBuffer/Even", canonicalNameConstString());
                    desc.label = label;
                }
                else {
                    *label = 0;
                }
                m_vertexBuffers[0] = skinDeformer->create(desc, 0);
                if (Inline::isDebugLabelEnabled()) {
                    StringUtils::format(label, sizeof(label), "Models/%s/VertexBuffer/Odd", canonicalNameConstString());
                    desc.label = label;
                }
                m_vertexBuffers[1] = skinDeformer->create(desc, 1);
                m_skinDeformer = skinDeformer;
            }
        }
        if (!m_skinDeformer) {
            initializeVertexBufferByteArray();
            desc.size = m_vertexBufferData.size();
            if (Inline::isDebugLabelEnabled()) {
                StringUtils::format(label, sizeof(label), "Models/%s/VertexBuffer/Even", canonicalNameConstString());
                desc.label = label;
            }
            else {
                *label = 0;
            }
            m_vertexBuffers[0] = sg::make_buffer(&desc);
            SG_LABEL_BUFFER(m_vertexBuffers[0], label);
            if (Inline::isDebugLabelEnabled()) {
                StringUtils::format(label, sizeof(label), "Models/%s/VertexBuffer/Odd", canonicalNameConstString());
                desc.label = label;
            }
            m_vertexBuffers[1] = sg::make_buffer(&desc);
            SG_LABEL_BUFFER(m_vertexBuffers[1], label);
        }
        nanoem_assert(
            sg::query_buffer_state(m_vertexBuffers[0]) == SG_RESOURCESTATE_VALID, "vertex buffer must be valid");
        nanoem_assert(
            sg::query_buffer_state(m_vertexBuffers[1]) == SG_RESOURCESTATE_VALID, "vertex buffer must be valid");
    }
    else if (!initialized) {
        desc.data.size = desc.size = sizeof(VertexUnit);
        m_vertexBuffers[0] = sg::make_buffer(&desc);
        m_vertexBuffers[1] = sg::make_buffer(&desc);
    }
}

void
Model::initializeVertexBufferByteArray()
{
    ParallelSkinningTaskData s(this, m_project->drawType(), edgeSize());
    m_vertexBufferData.resize(sizeof(Model::VertexUnit) * glm::max(s.m_numVertices, nanoem_rsize_t(1)));
    s.m_output = m_vertexBufferData.data();
    dispatchParallelTasks(&Model::handlePerformSkinningVertexTransform, &s, s.m_numVertices);
}

void
Model::internalUpdateStagingVertexBuffer(nanoem_u8_t *ptr, nanoem_rsize_t numVertices)
{
    nanoem_rsize_t numSoftBodies;
    nanoem_model_soft_body_t *const *softBodies = nanoemModelGetAllSoftBodyObjects(m_opaque, &numSoftBodies);
    if (softBodies && numSoftBodies > 0) {
        VertexUnit *vertexUnits = reinterpret_cast<VertexUnit *>(ptr);
        for (nanoem_rsize_t i = 0; i < numSoftBodies; i++) {
            const nanoem_model_soft_body_t *softBodyPtr = softBodies[i];
            if (model::SoftBody *softBody = model::SoftBody::cast(softBodyPtr)) {
                softBody->synchronizeTransformFeedbackFromSimulation(vertexUnits, numVertices);
            }
        }
        ParallelSkinningTaskData s(this, m_project->drawType(), edgeSize());
        s.m_output = ptr;
        dispatchParallelTasks(&Model::handlePerformSkinningVertexTransform, &s, numVertices);
        for (nanoem_rsize_t i = 0; i < numSoftBodies; i++) {
            const nanoem_model_soft_body_t *softBodyPtr = softBodies[i];
            if (model::SoftBody *softBody = model::SoftBody::cast(softBodyPtr)) {
                softBody->synchronizeTransformFeedbackToSimulation(softBodyPtr, vertexUnits, numVertices);
            }
        }
    }
    else {
        ParallelSkinningTaskData s(this, m_project->drawType(), edgeSize());
        s.m_output = ptr;
        dispatchParallelTasks(&Model::handlePerformSkinningVertexTransform, &s, numVertices);
    }
}

void
Model::clearAllLoadingImageItems()
{
    for (LoadingImageItemList::iterator it = m_loadingImageItems.begin(), end = m_loadingImageItems.end(); it != end;
         ++it) {
        nanoem_delete(*it);
    }
    m_loadingImageItems.clear();
}

void
Model::predeformMorph(const nanoem_model_morph_t *morphPtr)
{
    nanoem_parameter_assert(morphPtr, "must not be nullptr");
    const model::Morph *morph = model::Morph::cast(morphPtr);
    nanoem_f32_t weight = morph ? morph->weight() : 0;
    switch (nanoemModelMorphGetType(morphPtr)) {
    case NANOEM_MODEL_MORPH_TYPE_GROUP: {
        nanoem_rsize_t numChildren;
        const nanoem_model_morph_group_t *const *children =
            nanoemModelMorphGetAllGroupMorphObjects(morphPtr, &numChildren);
        for (nanoem_rsize_t i = 0; i < numChildren; i++) {
            const nanoem_model_morph_group_t *child = children[i];
            const nanoem_model_morph_t *targetMorphPtr = nanoemModelMorphGroupGetMorphObject(child);
            if (nanoemModelMorphGetType(targetMorphPtr) == NANOEM_MODEL_MORPH_TYPE_FLIP) {
                if (model::Morph *targetMorph = model::Morph::cast(targetMorphPtr)) {
                    targetMorph->setForcedWeight(weight * nanoemModelMorphGroupGetWeight(child));
                    predeformMorph(targetMorphPtr);
                }
            }
        }
        break;
    }
    case NANOEM_MODEL_MORPH_TYPE_FLIP: {
        nanoem_rsize_t numChildren;
        const nanoem_model_morph_flip_t *const *children =
            nanoemModelMorphGetAllFlipMorphObjects(morphPtr, &numChildren);
        if (weight > 0 && numChildren > 0) {
            int targetIndex = glm::clamp(
                int(Inline::saturateInt32(numChildren + 1) * weight) - 1, 0, Inline::saturateInt32(numChildren) - 1);
            const nanoem_model_morph_flip_t *child = children[targetIndex];
            const nanoem_model_morph_t *targetMorphPtr = nanoemModelMorphFlipGetMorphObject(child);
            if (model::Morph *targetMorph = model::Morph::cast(targetMorphPtr)) {
                targetMorph->setWeight(nanoemModelMorphFlipGetWeight(child));
            }
        }
        break;
    }
    case NANOEM_MODEL_MORPH_TYPE_IMPULUSE:
    case NANOEM_MODEL_MORPH_TYPE_MATERIAL:
    case NANOEM_MODEL_MORPH_TYPE_BONE:
    case NANOEM_MODEL_MORPH_TYPE_VERTEX:
    case NANOEM_MODEL_MORPH_TYPE_TEXTURE:
    case NANOEM_MODEL_MORPH_TYPE_UVA1:
    case NANOEM_MODEL_MORPH_TYPE_UVA2:
    case NANOEM_MODEL_MORPH_TYPE_UVA3:
    case NANOEM_MODEL_MORPH_TYPE_UVA4:
    default:
        /* do nothing */
        break;
    }
}

void
Model::deformMorph(const nanoem_model_morph_t *morphPtr, bool checkDirty)
{
    nanoem_parameter_assert(morphPtr, "must not be nullptr");
    const model::Morph *morph = model::Morph::cast(morphPtr);
    if (morph && (!checkDirty || (checkDirty && morph->isDirty()))) {
        const nanoem_f32_t weight = morph->weight();
        const nanoem_model_morph_type_t type = nanoemModelMorphGetType(morphPtr);
        switch (type) {
        case NANOEM_MODEL_MORPH_TYPE_GROUP: {
            nanoem_rsize_t numChildren;
            const nanoem_model_morph_group_t *const *children =
                nanoemModelMorphGetAllGroupMorphObjects(morphPtr, &numChildren);
            for (nanoem_rsize_t i = 0; i < numChildren; i++) {
                const nanoem_model_morph_group_t *child = children[i];
                const nanoem_model_morph_t *targetMorphPtr = nanoemModelMorphGroupGetMorphObject(child);
                if (morphPtr != targetMorphPtr &&
                    nanoemModelMorphGetType(targetMorphPtr) != NANOEM_MODEL_MORPH_TYPE_FLIP) {
                    if (model::Morph *targetMorph = model::Morph::cast(targetMorphPtr)) {
                        targetMorph->setForcedWeight(weight * nanoemModelMorphGroupGetWeight(child));
                        deformMorph(targetMorphPtr, false);
                    }
                }
            }
            break;
        }
        case NANOEM_MODEL_MORPH_TYPE_FLIP: {
            /* do nothing */
            break;
        }
        case NANOEM_MODEL_MORPH_TYPE_IMPULUSE: {
            nanoem_rsize_t numChildren;
            const nanoem_model_morph_impulse_t *const *children =
                nanoemModelMorphGetAllImpulseMorphObjects(morphPtr, &numChildren);
            for (nanoem_rsize_t i = 0; i < numChildren; i++) {
                const nanoem_model_morph_impulse_t *child = children[i];
                const nanoem_model_rigid_body_t *rigidBodyPtr = nanoemModelMorphImpulseGetRigidBodyObject(child);
                if (model::RigidBody *rigidBody = model::RigidBody::cast(rigidBodyPtr)) {
                    const Vector3 torque(glm::make_vec3(nanoemModelMorphImpulseGetTorque(child))),
                        velocity(glm::make_vec3(nanoemModelMorphImpulseGetVelocity(child)));
                    bool local = nanoemModelMorphImpulseIsLocal(child) != 0;
                    if (glm::all(glm::epsilonEqual(torque, Constants::kZeroV3, Constants::kEpsilon)) &&
                        glm::all(glm::epsilonEqual(velocity, Constants::kZeroV3, Constants::kEpsilon))) {
                        rigidBody->markAllForcesReset();
                    }
                    else if (local) {
                        rigidBody->addLocalTorqueForce(torque, weight);
                        rigidBody->addLocalVelocityForce(velocity, weight);
                    }
                    else {
                        rigidBody->addGlobalTorqueForce(torque, weight);
                        rigidBody->addGlobalVelocityForce(velocity, weight);
                    }
                }
            }
            break;
        }
        case NANOEM_MODEL_MORPH_TYPE_MATERIAL: {
            nanoem_rsize_t numChildren, numMaterials;
            const nanoem_model_morph_material_t *const *children =
                nanoemModelMorphGetAllMaterialMorphObjects(morphPtr, &numChildren);
            const nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(m_opaque, &numMaterials);
            for (nanoem_rsize_t i = 0; i < numChildren; i++) {
                const nanoem_model_morph_material_t *child = children[i];
                const nanoem_model_material_t *materialPtr = nanoemModelMorphMaterialGetMaterialObject(child);
                if (model::Material *material = model::Material::cast(materialPtr)) {
                    material->update(child, weight);
                }
                else {
                    for (nanoem_rsize_t j = 0; j < numMaterials; j++) {
                        const nanoem_model_material_t *materialPtr = materials[j];
                        if (model::Material *material = model::Material::cast(materialPtr)) {
                            material->update(child, weight);
                        }
                    }
                }
            }
            break;
        }
        case NANOEM_MODEL_MORPH_TYPE_BONE: {
            nanoem_rsize_t numChildren;
            const nanoem_model_morph_bone_t *const *children =
                nanoemModelMorphGetAllBoneMorphObjects(morphPtr, &numChildren);
            for (nanoem_rsize_t i = 0; i < numChildren; i++) {
                const nanoem_model_morph_bone_t *child = children[i];
                const nanoem_model_bone_t *bonePtr = nanoemModelMorphBoneGetBoneObject(child);
                if (model::Bone *bone = model::Bone::cast(bonePtr)) {
                    bone->updateLocalMorphTransform(child, weight);
                }
            }
            break;
        }
        case NANOEM_MODEL_MORPH_TYPE_VERTEX: {
            nanoem_rsize_t numChildren;
            const nanoem_model_morph_vertex_t *const *children =
                nanoemModelMorphGetAllVertexMorphObjects(morphPtr, &numChildren);
            for (nanoem_rsize_t i = 0; i < numChildren; i++) {
                const nanoem_model_morph_vertex_t *child = children[i];
                const nanoem_model_vertex_t *vertexPtr = nanoemModelMorphVertexGetVertexObject(child);
                if (model::Vertex *vertex = model::Vertex::cast(vertexPtr)) {
                    vertex->deform(child, weight);
                }
            }
            break;
        }
        case NANOEM_MODEL_MORPH_TYPE_TEXTURE:
        case NANOEM_MODEL_MORPH_TYPE_UVA1:
        case NANOEM_MODEL_MORPH_TYPE_UVA2:
        case NANOEM_MODEL_MORPH_TYPE_UVA3:
        case NANOEM_MODEL_MORPH_TYPE_UVA4: {
            nanoem_rsize_t numChildren;
            int index =
                static_cast<int>(nanoemModelMorphGetType(morphPtr)) - static_cast<int>(NANOEM_MODEL_MORPH_TYPE_TEXTURE);
            const nanoem_model_morph_uv_t *const *children =
                nanoemModelMorphGetAllUVMorphObjects(morphPtr, &numChildren);
            for (nanoem_rsize_t i = 0; i < numChildren; i++) {
                const nanoem_model_morph_uv_t *child = children[i];
                const nanoem_model_vertex_t *vertexPtr = nanoemModelMorphUVGetVertexObject(child);
                if (model::Vertex *vertex = model::Vertex::cast(vertexPtr)) {
                    vertex->deform(child, index, weight);
                }
            }
            break;
        }
        default:
            break;
        }
    }
}

void
Model::solveConstraint(
    const nanoem_model_constraint_t *constraintPtr, int numIterations, nanoem_unicode_string_factory_t *factory)
{
    nanoem_parameter_assert(constraintPtr, "must not be nullptr");
    nanoem_parameter_assert(factory, "must not be nullptr");
    nanoem_rsize_t numJoints;
    nanoem_model_constraint_joint_t *const *joints = nanoemModelConstraintGetAllJointObjects(constraintPtr, &numJoints);
    const nanoem_model_bone_t *targetBonePtr = nanoemModelConstraintGetTargetBoneObject(constraintPtr);
    const nanoem_model_bone_t *effectorBonePtr = nanoemModelConstraintGetEffectorBoneObject(constraintPtr);
    nanoem_f32_t angleLimit = nanoemModelConstraintGetAngleLimit(constraintPtr);
    const model::Bone *targetBone = model::Bone::cast(targetBonePtr);
    model::Bone *effectorBone = model::Bone::cast(effectorBonePtr);
    model::Constraint *constraint = model::Constraint::cast(constraintPtr);
    const Vector4 effectorBonePosition(effectorBone->worldTransformOrigin(), 1),
        targetBonePosition(targetBone->worldTransformOrigin(), 1);
    for (int i = 0; i < numIterations; i++) {
        const bool firstIteration = i == 0;
        for (nanoem_rsize_t j = 0; j < numJoints; j++) {
            const nanoem_model_constraint_joint_t *joint = joints[j];
            const nanoem_model_bone_t *jointBonePtr = nanoemModelConstraintJointGetBoneObject(joint);
            model::Bone *jointBone = model::Bone::cast(jointBonePtr);
            model::Constraint::Joint *jointResult = constraint->jointIterationResult(joint, i);
            if (!model::Constraint::solveAxisAngle(
                    jointBone->worldTransform(), effectorBonePosition, targetBonePosition, jointResult)) {
                nanoem_f32_t newAngleLimit = angleLimit * (j + 1);
                const bool hasUnitXConstraint = model::Constraint::hasUnitXConstraint(jointBonePtr, factory);
                if (firstIteration && hasUnitXConstraint) {
                    jointResult->setAxis(Constants::kUnitX);
                }
                const Quaternion orientation(
                    glm::angleAxis(glm::min(jointResult->m_angle, newAngleLimit), jointResult->m_axis));
                Quaternion mixedOrientation;
                if (firstIteration) {
                    mixedOrientation = orientation * jointBone->localOrientation();
                }
                else {
                    mixedOrientation = jointBone->constraintJointOrientation() * orientation;
                }
                if (hasUnitXConstraint) {
                    static const Vector3 kLowerLimit(glm::radians(0.5f), 0.0f, 0.0f);
                    static const Vector3 kUpperLimit(glm::radians(180.0f), 0.0f, 0.0f);
                    model::Bone::constrainOrientation(kUpperLimit, kLowerLimit, mixedOrientation);
                }
                jointBone->setConstraintJointOrientation(mixedOrientation);
                for (int k = Inline::saturateInt32(j); k >= 0; k--) {
                    const nanoem_model_constraint_joint_t *upperJoint = joints[k];
                    const nanoem_model_bone_t *upperJointBonePtr = nanoemModelConstraintJointGetBoneObject(upperJoint);
                    model::Bone *upperJointBone = model::Bone::cast(upperJointBonePtr);
                    upperJointBone->updateLocalTransform(upperJointBonePtr, upperJointBone->localTranslation(),
                        upperJointBone->constraintJointOrientation());
                }
                jointResult->setTransform(jointBone->worldTransform());
                effectorBone->updateLocalTransform(effectorBonePtr);
                model::Constraint::Joint *effectorResult = constraint->effectorIterationResult(joint, i);
                effectorResult->setTransform(effectorBone->worldTransform());
            }
        }
    }
}

void
Model::synchronizeBoneMotion(const Motion *motion, nanoem_frame_index_t frameIndex, nanoem_f32_t amount,
    PhysicsEngine::SimulationTimingType timing)
{
    nanoem_rsize_t numBones;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllOrderedBoneObjects(m_opaque, &numBones);
    if (timing == PhysicsEngine::kSimulationTimingBefore) {
        for (nanoem_rsize_t i = 0; i < numBones; i++) {
            const nanoem_model_bone_t *bonePtr = bones[i];
            const nanoem_model_rigid_body_t *rigidBodyPtr = nullptr;
            BoneBoundRigidBodyMap::const_iterator it = m_boneBoundRigidBodies.find(bonePtr);
            if (it != m_boneBoundRigidBodies.end()) {
                rigidBodyPtr = it->second;
            }
            if (model::Bone *bone = model::Bone::cast(bonePtr)) {
                bone->synchronizeMotion(motion, bonePtr, rigidBodyPtr, frameIndex, amount);
            }
        }
    }
    for (nanoem_rsize_t i = 0; i < numBones; i++) {
        const nanoem_model_bone_t *bonePtr = bones[i];
        if ((nanoemModelBoneIsAffectedByPhysicsSimulation(bonePtr) != 0) == timing) {
            if (model::Bone *bone = model::Bone::cast(bonePtr)) {
                bone->applyAllLocalTransform(bonePtr, this);
                bone->applyOutsideParentTransform(bonePtr, this);
                m_boundingBox.set(bone->worldTransformOrigin());
            }
        }
    }
}

void
Model::synchronizeMorphMotion(const Motion *motion, nanoem_frame_index_t frameIndex, nanoem_f32_t amount)
{
    /* prevent updating vertex morph twice or more */
    if (!EnumUtils::isEnabled(kPrivateStateDirtyMorph, m_states)) {
        resetAllMorphs();
        nanoem_rsize_t numObjects;
        nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(m_opaque, &numObjects);
        for (nanoem_rsize_t i = 0; i < numObjects; i++) {
            const nanoem_model_morph_t *morphPtr = morphs[i];
            const nanoem_unicode_string_t *name = nanoemModelMorphGetName(morphPtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
            if (model::Morph *morph = model::Morph::cast(morphPtr)) {
                morph->synchronizeMotion(motion, name, frameIndex, amount);
            }
        }
        deformAllMorphs(true);
        for (nanoem_rsize_t i = 0; i < numObjects; i++) {
            const nanoem_model_morph_t *morphPtr = morphs[i];
            if (model::Morph *morph = model::Morph::cast(morphPtr)) {
                morph->setDirty(false);
            }
        }
        EnumUtils::setEnabled(kPrivateStateDirtyMorph, m_states, true);
    }
}

void
Model::synchronizeAllConstraintStates(const nanoem_motion_model_keyframe_t *keyframe)
{
    nanoem_rsize_t numStates;
    nanoem_motion_model_keyframe_constraint_state_t *const *states =
        nanoemMotionModelKeyframeGetAllConstraintStateObjects(keyframe, &numStates);
    for (nanoem_rsize_t i = 0; i < numStates; i++) {
        const nanoem_motion_model_keyframe_constraint_state_t *state = states[i];
        const nanoem_unicode_string_t *name = nanoemMotionModelKeyframeConstraintStateGetBoneName(state);
        const nanoem_model_constraint_t *constraintPtr = findConstraint(findBone(name));
        if (model::Constraint *constraint = model::Constraint::cast(constraintPtr)) {
            const bool enabled = nanoemMotionModelKeyframeConstraintStateIsEnabled(state) != 0;
            constraint->setEnabled(enabled);
        }
    }
}

void
Model::synchronizeAllOutsideParents(const nanoem_motion_model_keyframe_t *keyframe)
{
    nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
    nanoem_rsize_t numOutsideParents;
    nanoem_motion_outside_parent_t *const *ops =
        nanoemMotionModelKeyframeGetAllOutsideParentObjects(keyframe, &numOutsideParents);
    m_outsideParents.clear();
    setActiveOutsideParentSubjectBone(nullptr);
    for (nanoem_rsize_t i = 0; i < numOutsideParents; i++) {
        const nanoem_motion_outside_parent_t *op = ops[i];
        String boundBoneString;
        StringUtils::getUtf8String(nanoemMotionOutsideParentGetSubjectBoneName(op), factory, boundBoneString);
        if (const nanoem_model_bone_t *boundBone = findBone(boundBoneString)) {
            String opModelString;
            StringUtils::getUtf8String(nanoemMotionOutsideParentGetTargetObjectName(op), factory, opModelString);
            if (const Model *opModel = m_project->findModelByName(opModelString)) {
                String opBoneString;
                StringUtils::getUtf8String(nanoemMotionOutsideParentGetTargetBoneName(op), factory, opBoneString);
                if (opModel->findBone(opBoneString)) {
                    internalSetOutsideParent(boundBone, tinystl::make_pair(opModelString, opBoneString));
                }
            }
        }
    }
}

void
Model::synchronizeAllRigidBodyKinematics(const Motion *motion, nanoem_frame_index_t frameIndex)
{
    nanoem_rsize_t numBodies;
    nanoem_model_rigid_body_t *const *bodies = nanoemModelGetAllRigidBodyObjects(m_opaque, &numBodies);
    const nanoem_motion_t *opaque = motion->data();
    for (nanoem_rsize_t i = 0; i < numBodies; i++) {
        const nanoem_model_rigid_body_t *rigidBodyPtr = bodies[i];
        if (const nanoem_model_bone_t *bonePtr = nanoemModelRigidBodyGetBoneObject(rigidBodyPtr)) {
            const nanoem_unicode_string_t *name = nanoemModelBoneGetName(bonePtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
            const nanoem_motion_bone_keyframe_t *prevKeyframe = motion->findBoneKeyframe(name, frameIndex),
                                                *nextKeyframe = motion->findBoneKeyframe(name, frameIndex + 1);
            if (prevKeyframe && nextKeyframe) {
                model::RigidBody *rigidBody = model::RigidBody::cast(rigidBodyPtr);
                if (rigidBody && nanoemMotionBoneKeyframeIsPhysicsSimulationEnabled(prevKeyframe) &&
                    !nanoemMotionBoneKeyframeIsPhysicsSimulationEnabled(nextKeyframe)) {
                    rigidBody->enableKinematic();
                }
            }
            else {
                nanoem_motion_bone_keyframe_t *mutPrevKeyframe, *mutNextKeyframe;
                model::RigidBody *rigidBody = model::RigidBody::cast(rigidBodyPtr);
                nanoemMotionSearchClosestBoneKeyframes(opaque, name, frameIndex, &mutPrevKeyframe, &mutNextKeyframe);
                if (rigidBody && nanoemMotionBoneKeyframeIsPhysicsSimulationEnabled(mutPrevKeyframe) &&
                    !nanoemMotionBoneKeyframeIsPhysicsSimulationEnabled(mutNextKeyframe)) {
                    rigidBody->enableKinematic();
                }
            }
        }
    }
}

void
Model::dispatchParallelTasks(DispatchParallelTasksIterator iterator, void *opaque, size_t iterations)
{
#if defined(NANOEM_ENABLE_TBB)
    struct ParallelExecutor {
        typedef void (*DispatchParallelTasksIterator)(void *, size_t);
        ParallelExecutor(DispatchParallelTasksIterator iterator, void *opaque)
            : m_iterator(iterator)
            , m_opaque(opaque)
        {
        }
        void
        operator()(const tbb::blocked_range<size_t> &range) const
        {
            for (size_t it = range.begin(), end = range.end(); it != end; ++it) {
                m_iterator(m_opaque, it);
            }
        }
        DispatchParallelTasksIterator m_iterator;
        void *m_opaque;
    } executor(iterator, opaque);
    tbb::parallel_for(tbb::blocked_range<size_t>(0, iterations), executor);
#elif defined(__APPLE__)
    dispatch_queue_t queue = static_cast<dispatch_queue_t>(m_dispatchParallelTaskQueue);
    dispatch_apply_f(iterations, queue, opaque, iterator);
#else /* NANOEM_ENABLE_TBB */
#if defined(NANOEM_ENABLE_OPENMP)
    const int numIterations = Inline::saturatedInt(iterations);
#pragma omp parallel for
    for (int i = 0; i < numIterations; i++) {
#else
    for (size_t i = 0; i < iterations; i++) {
#endif /* NANOEM_ENABLE_OPENMP */
        iterator(opaque, i);
    }
#endif /* NANOEM_ENABLE_TBB */
}

bool
Model::saveAllAttachments(const String &prefix, const FileEntityMap &allAttachments, Archiver &archiver, Error &error)
{
    const nanoem_u8_t *ptr;
    bool succeeded = true;
    if (Project::isArchiveURI(fileURI())) {
        const URI &baseURI = resolvedFileURI();
        FileReaderScope scope(m_project->translator());
        if (scope.open(baseURI, error)) {
            Archiver sourceArchive(scope.reader());
            if (sourceArchive.open(error)) {
                Archiver::Entry entry;
                ByteArray bytes;
                for (FileEntityMap::const_iterator it = allAttachments.begin(), end = allAttachments.end();
                     succeeded && it != end; ++it) {
                    ByteArray attachmentData;
                    const URI &fileURI = it->second;
                    if (!it->first.empty() && sourceArchive.findEntry(fileURI.fragment(), entry, error) &&
                        sourceArchive.extract(entry, attachmentData, error)) {
                        String path(prefix);
                        path.append(it->first.c_str());
                        entry.m_path = path;
                        ptr = attachmentData.data();
                        bytes.assign(ptr, ptr + attachmentData.size());
                        succeeded &= archiver.addEntry(entry, bytes, error);
                    }
                }
                succeeded &= sourceArchive.close(error);
            }
            else {
                succeeded = false;
            }
        }
    }
    else {
        Archiver::Entry entry;
        ByteArray bytes;
        for (FileEntityMap::const_iterator it = allAttachments.begin(), end = allAttachments.end();
             succeeded && it != end; ++it) {
            ByteArray attachmentData;
            const URI &fileURI = it->second;
            if (FileUtils::exists(fileURI)) {
                FileReaderScope scope(m_project->translator());
                if (scope.open(fileURI, error)) {
                    FileUtils::read(scope, attachmentData, error);
                    String path(prefix);
                    path.append(it->first.c_str());
                    entry.m_path = path;
                    ptr = attachmentData.data();
                    bytes.assign(ptr, ptr + attachmentData.size());
                    succeeded &= archiver.addEntry(entry, bytes, error);
                }
                if (error.hasReason()) {
                    break;
                }
            }
        }
    }
    return succeeded;
}

bool
Model::getVertexIndexBuffer(const model::Material *material, IPass::Buffer &buffer) const NANOEM_DECL_NOEXCEPT
{
    nanoem_parameter_assert(material, "must not be nullptr");
    bool renderable = material->isVisible();
    if (renderable) {
        buffer.m_vertexBuffer = m_vertexBuffers[1 - m_stageVertexBufferIndex];
        buffer.m_indexBuffer = m_indexBuffer;
    }
    return renderable;
}

bool
Model::getEdgeIndexBuffer(const model::Material *material, IPass::Buffer &buffer) const NANOEM_DECL_NOEXCEPT
{
    nanoem_parameter_assert(material, "must not be nullptr");
    bool renderable = material->isVisible();
    if (renderable) {
        buffer.m_vertexBuffer = m_vertexBuffers[1 - m_stageVertexBufferIndex];
        buffer.m_indexBuffer = m_indexBuffer;
    }
    return renderable;
}

Vector4
Model::connectionBoneColor(
    const nanoem_model_bone_t *bone, const Vector4 &base, bool enableFixedAxis) const NANOEM_DECL_NOEXCEPT
{
    static const nanoem_f32_t kOpacity = 1.0f;
    Vector4 color;
    if (m_selection->containsBone(bone)) {
        color = Vector4(1, 0, 0, kOpacity);
    }
    else if (model::Bone::cast(bone)->isDirty()) {
        color = Vector4(0, 1, 0, kOpacity);
    }
    else if (isConstraintJointBone(bone)) {
        color = Vector4(1, 1, 0, kOpacity);
    }
    else if (enableFixedAxis && nanoemModelBoneHasFixedAxis(bone)) {
        color = Vector4(1, 0, 1, kOpacity);
    }
    else {
        color = base;
    }
    return color;
}

Vector4
Model::hoveredBoneColor(const Vector4 &inactive, bool selected) const NANOEM_DECL_NOEXCEPT
{
    static const nanoem_f32_t kOpacity = 1.0f;
    Vector4 color;
    if (selected) {
        color = Vector4(1, 0, 0, kOpacity);
    }
    else {
        color = inactive;
    }
    return color;
}

void
Model::drawColor(bool scriptExternalColor)
{
    SG_PUSH_GROUPF("Model::drawColor(name=%s, scriptExternalColor=%s)", canonicalNameConstString(),
        scriptExternalColor ? "true" : "false");
    nanoem_rsize_t numMaterials, numIndices, indexOffset = 0;
    const ILight *globalLight = m_project->globalLight();
    const ICamera *activeCamera = m_project->activeCamera();
    const ShadowCamera *shadowCamera = m_project->shadowCamera();
    nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(m_opaque, &numMaterials);
    const String &passType = isShadowMapEnabled() && m_project->isShadowMapEnabled() ? Effect::kPassTypeObjectSelfShadow
                                                                                     : Effect::kPassTypeObject;
    if (m_screenImage) {
        m_screenImage->m_handle = m_project->viewportSecondaryImage();
    }
    for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
        const nanoem_model_material_t *materialPtr = materials[i];
        numIndices = nanoemModelMaterialGetNumVertexIndices(materialPtr);
        model::Material *material = model::Material::cast(materialPtr);
        IPass::Buffer buffer(numIndices, indexOffset);
        if (getVertexIndexBuffer(material, buffer)) {
            IEffect *effect = internalEffect(material);
            if (ITechnique *technique = effect->findTechnique(passType, materialPtr, i, numMaterials, this)) {
                SG_PUSH_GROUPF("Model::drawColor(offset=%d, name=%s)", i, material->canonicalNameConstString());
                while (IPass *pass = technique->execute(this, scriptExternalColor)) {
                    pass->setGlobalParameters(this, m_project);
                    pass->setCameraParameters(activeCamera, kInitialWorldMatrix);
                    pass->setLightParameters(globalLight, false);
                    pass->setAllModelParameters(this, m_project);
                    pass->setMaterialParameters(materialPtr);
                    pass->setShadowMapParameters(shadowCamera, kInitialWorldMatrix);
                    pass->execute(this, buffer);
                }
                if (!technique->hasNextScriptCommand() && !scriptExternalColor) {
                    technique->resetScriptCommandState();
                    technique->resetScriptExternalColor();
                }
                SG_POP_GROUP();
            }
        }
        indexOffset += numIndices;
    }
    SG_POP_GROUP();
}

void
Model::drawEdge(nanoem_f32_t edgeSizeScaleFactor)
{
    SG_PUSH_GROUPF("Model::drawEdge(name=%s)", canonicalNameConstString());
    nanoem_rsize_t numMaterials, numIndices, indexOffset = 0;
    const ILight *globalLight = m_project->globalLight();
    const ICamera *activeCamera = m_project->activeCamera();
    nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(m_opaque, &numMaterials);
    for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
        const nanoem_model_material_t *materialPtr = materials[i];
        numIndices = nanoemModelMaterialGetNumVertexIndices(materialPtr);
        if (nanoemModelMaterialIsEdgeEnabled(materialPtr) && !nanoemModelMaterialIsLineDrawEnabled(materialPtr) &&
            !nanoemModelMaterialIsPointDrawEnabled(materialPtr)) {
            model::Material *material = model::Material::cast(materialPtr);
            IPass::Buffer buffer(numIndices, indexOffset);
            if (getEdgeIndexBuffer(material, buffer)) {
                IEffect *effect = internalEffect(material);
                if (ITechnique *technique =
                        effect->findTechnique(Effect::kPassTypeEdge, materialPtr, i, numMaterials, this)) {
                    SG_PUSH_GROUPF("Model::drawEdge(offset=%d, name=%s)", i, material->canonicalNameConstString());
                    while (IPass *pass = technique->execute(this, false)) {
                        pass->setGlobalParameters(this, m_project);
                        pass->setCameraParameters(activeCamera, kInitialWorldMatrix);
                        pass->setLightParameters(globalLight, false);
                        pass->setAllModelParameters(this, m_project);
                        pass->setMaterialParameters(materialPtr);
                        pass->setEdgeParameters(materialPtr, edgeSizeScaleFactor);
                        pass->execute(this, buffer);
                    }
                    if (!technique->hasNextScriptCommand()) {
                        technique->resetScriptCommandState();
                    }
                    SG_POP_GROUP();
                }
            }
        }
        indexOffset += numIndices;
    }
    SG_POP_GROUP();
}

void
Model::drawGroundShadow()
{
    SG_PUSH_GROUPF("Model::drawGroundShadow(name=%s)", canonicalNameConstString());
    nanoem_rsize_t numMaterials, numIndices, indexOffset = 0;
    const ICamera *activeCamera = m_project->activeCamera();
    const ILight *globalLight = m_project->globalLight();
    Matrix4x4 world;
    globalLight->getShadowTransform(world);
    nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(m_opaque, &numMaterials);
    for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
        const nanoem_model_material_t *materialPtr = materials[i];
        numIndices = nanoemModelMaterialGetNumVertexIndices(materialPtr);
        if (nanoemModelMaterialIsCastingShadowEnabled(materialPtr) &&
            !nanoemModelMaterialIsPointDrawEnabled(materialPtr)) {
            model::Material *material = model::Material::cast(materialPtr);
            IPass::Buffer buffer(numIndices, indexOffset);
            if (getVertexIndexBuffer(material, buffer)) {
                IEffect *effect = internalEffect(material);
                if (ITechnique *technique =
                        effect->findTechnique(Effect::kPassTypeShadow, materialPtr, i, numMaterials, this)) {
                    SG_PUSH_GROUPF(
                        "Model::drawGroundShadow(offset=%d, name=%s)", i, material->canonicalNameConstString());
                    while (IPass *pass = technique->execute(this, false)) {
                        pass->setGlobalParameters(this, m_project);
                        pass->setCameraParameters(activeCamera, world);
                        pass->setLightParameters(globalLight, false);
                        pass->setAllModelParameters(this, m_project);
                        pass->setMaterialParameters(materialPtr);
                        pass->setGroundShadowParameters(globalLight, activeCamera, world);
                        pass->execute(this, buffer);
                    }
                    if (!technique->hasNextScriptCommand()) {
                        technique->resetScriptCommandState();
                    }
                    SG_POP_GROUP();
                }
            }
        }
        indexOffset += numIndices;
    }
    SG_POP_GROUP();
}

void
Model::drawShadowMap()
{
    SG_PUSH_GROUPF("Model::drawShadowMap(name=%s)", canonicalNameConstString());
    nanoem_rsize_t numMaterials, indexOffset = 0;
    nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(m_opaque, &numMaterials);
    const ILight *globalLight = m_project->globalLight();
    const ICamera *activeCamera = m_project->activeCamera();
    const ShadowCamera *shadowCamera = m_project->shadowCamera();
    for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
        const nanoem_model_material_t *materialPtr = materials[i];
        const nanoem_rsize_t numIndices = nanoemModelMaterialGetNumVertexIndices(materialPtr);
        if (nanoemModelMaterialIsCastingShadowMapEnabled(materialPtr) &&
            !nanoemModelMaterialIsPointDrawEnabled(materialPtr)) {
            model::Material *material = model::Material::cast(materialPtr);
            IPass::Buffer buffer(numIndices, indexOffset);
            if (getVertexIndexBuffer(material, buffer)) {
                IEffect *effect = internalEffect(material);
                if (ITechnique *technique =
                        effect->findTechnique(Effect::kPassTypeZplot, materialPtr, i, numMaterials, this)) {
                    SG_PUSH_GROUPF("Model::drawShadowMap(offset=%d, name=%s)", i, material->canonicalNameConstString());
                    while (IPass *pass = technique->execute(this, false)) {
                        pass->setGlobalParameters(this, m_project);
                        pass->setCameraParameters(activeCamera, kInitialWorldMatrix);
                        pass->setLightParameters(globalLight, false);
                        pass->setAllModelParameters(this, m_project);
                        pass->setShadowMapParameters(shadowCamera, kInitialWorldMatrix);
                        pass->execute(this, buffer);
                    }
                    if (!technique->hasNextScriptCommand()) {
                        technique->resetScriptCommandState();
                    }
                    SG_POP_GROUP();
                }
            }
        }
        indexOffset += numIndices;
    }
    SG_POP_GROUP();
}

void
Model::drawAllVertexPoints()
{
    nanoem_rsize_t numVertices;
    nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(m_opaque, &numVertices);
    model::Vertex::List newVertices;
    if (!m_activeMaterialPtr) {
        newVertices.resize(numVertices);
        memcpy(newVertices.data(), vertices, sizeof(newVertices[0]) * numVertices);
    }
    else {
        newVertices.reserve(numVertices);
        for (nanoem_rsize_t i = 0; i < numVertices; i++) {
            const nanoem_model_vertex_t *vertexPtr = vertices[i];
            const model::Vertex *vertex = model::Vertex::cast(vertexPtr);
            if (vertex && isMaterialSelected(vertex->material())) {
                newVertices.push_back(vertexPtr);
            }
        }
    }
    const size_t numNewVertices = newVertices.size();
    if (!sg::is_valid(m_drawAllPoints.m_buffer)) {
        sg_buffer_desc bd;
        Inline::clearZeroMemory(bd);
        bd.size = sizeof(m_drawAllPoints.m_vertices[0]) * glm::max(numNewVertices, size_t(1));
        bd.usage = SG_USAGE_DYNAMIC;
        char label[Inline::kMarkerStringLength];
        if (Inline::isDebugLabelEnabled()) {
            StringUtils::format(label, sizeof(label), "Models/%s/Points/VertexBuffer", canonicalNameConstString());
            bd.label = label;
        }
        m_drawAllPoints.m_buffer = sg::make_buffer(&bd);
        m_drawAllPoints.m_vertices.resize(numNewVertices);
        nanoem_assert(
            sg::query_buffer_state(m_drawAllPoints.m_buffer) == SG_RESOURCESTATE_VALID, "vertex buffer must be valid");
        SG_LABEL_BUFFER(m_drawAllPoints.m_buffer, bd.label);
    }
    bx::simd128_t normal;
    sg::LineVertexUnit *vertexUnits = m_drawAllPoints.m_vertices.data();
    for (size_t i = 0; i < numNewVertices; i++) {
        const nanoem_model_vertex_t *vertexPtr = newVertices[i];
        const int index = nanoemModelObjectGetIndex(nanoemModelVertexGetModelObject(vertexPtr));
        const model::Vertex *vertex = model::Vertex::cast(vertexPtr);
        sg::LineVertexUnit &vertexUnit = vertexUnits[index];
        bx::simd128_t *ptr = reinterpret_cast<bx::simd128_t *>(glm::value_ptr(vertexUnit.m_position));
        if (const model::SoftBody *softBody = model::SoftBody::cast(vertex->softBody())) {
            bx::simd128_t position;
            softBody->getVertexPosition(vertexPtr, &position);
            vertexUnit.m_position = glm::make_vec3(reinterpret_cast<const nanoem_f32_t *>(&position));
        }
        else {
            Model::VertexUnit::performSkinningByType(vertex, ptr, &normal);
        }
        vertexUnit.m_color =
            m_selection->containsVertex(vertexPtr) ? Vector4U8(0xff, 0, 0, 0xff) : Vector4U8(0, 0, 0xff, 0xff);
    }
    const nanoem_rsize_t size = Inline::saturateInt32(sizeof(*vertexUnits) * numNewVertices);
    if (size > 0) {
        sg::update_buffer(m_drawAllPoints.m_buffer, vertexUnits, size);
    }
    internal::LineDrawer *drawer = lineDrawer();
    internal::LineDrawer::Option option(m_drawAllPoints.m_buffer, numNewVertices);
    option.m_primitiveType = SG_PRIMITIVETYPE_POINTS;
    drawer->drawPass(option);
}

void
Model::drawAllVertexFaces()
{
    nanoem_rsize_t numMaterials, numVertices, numVertexIndices, indexOffset = 0;
    nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(m_opaque, &numVertices);
    nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(m_opaque, &numMaterials);
    if (!sg::is_valid(m_drawAllLines.m_vertexBuffer)) {
        m_drawAllLines.m_vertices.resize(glm::max(numVertices, size_t(1)));
        sg_buffer_desc desc;
        Inline::clearZeroMemory(desc);
        desc.size = sizeof(m_drawAllLines.m_vertices[0]) * m_drawAllLines.m_vertices.size();
        desc.usage = SG_USAGE_DYNAMIC;
        char label[Inline::kMarkerStringLength];
        if (Inline::isDebugLabelEnabled()) {
            StringUtils::format(label, sizeof(label), "Models/%s/Lines/VertexBuffer", canonicalNameConstString());
            desc.label = label;
        }
        m_drawAllLines.m_vertexBuffer = sg::make_buffer(&desc);
        nanoem_assert(sg::query_buffer_state(m_drawAllLines.m_vertexBuffer) == SG_RESOURCESTATE_VALID,
            "vertex buffer must be valid");
        SG_LABEL_BUFFER(m_drawAllLines.m_vertexBuffer, desc.label);
    }
    if (!sg::is_valid(m_drawAllLines.m_indexBuffer)) {
        const nanoem_u32_t *indices = nanoemModelGetAllVertexIndices(m_opaque, &numVertexIndices);
        nanoem_u32_t numAllocateVertexIndices = Inline::saturateInt32U(numVertexIndices * 2);
        m_drawAllLines.m_indices.resize(numAllocateVertexIndices);
        DrawIndexedBuffer::IndexType *vertexIndices = m_drawAllLines.m_indices.data();
        for (nanoem_rsize_t i = 0; i < numVertexIndices; i += 3) {
            const DrawIndexedBuffer::IndexType vertexIndex0 = indices[i + 0];
            const DrawIndexedBuffer::IndexType vertexIndex1 = indices[i + 1];
            const DrawIndexedBuffer::IndexType vertexIndex2 = indices[i + 2];
            size_t offset = i * 2;
            vertexIndices[offset + 0] = vertexIndex0;
            vertexIndices[offset + 1] = vertexIndex1;
            vertexIndices[offset + 2] = vertexIndex1;
            vertexIndices[offset + 3] = vertexIndex2;
            vertexIndices[offset + 4] = vertexIndex2;
            vertexIndices[offset + 5] = vertexIndex0;
        }
        sg_buffer_desc desc;
        Inline::clearZeroMemory(desc);
        if (numVertexIndices > 0) {
            desc.data.ptr = vertexIndices;
            desc.data.size = desc.size = sizeof(m_drawAllLines.m_indices[0]) * m_drawAllLines.m_indices.size();
        }
        else {
            static const int kDummyIndex = 0;
            desc.data.ptr = &kDummyIndex;
            desc.data.size = desc.size = sizeof(kDummyIndex);
        }
        desc.usage = SG_USAGE_IMMUTABLE;
        desc.type = SG_BUFFERTYPE_INDEXBUFFER;
        char label[Inline::kMarkerStringLength];
        if (Inline::isDebugLabelEnabled()) {
            StringUtils::format(label, sizeof(label), "Models/%s/Lines/IndexBuffer", canonicalNameConstString());
            desc.label = label;
        }
        m_drawAllLines.m_indexBuffer = sg::make_buffer(&desc);
        nanoem_assert(sg::query_buffer_state(m_drawAllLines.m_indexBuffer) == SG_RESOURCESTATE_VALID,
            "index buffer must be valid");
        SG_LABEL_BUFFER(m_drawAllLines.m_indexBuffer, desc.label);
    }
    sg::LineVertexUnit *vertexUnits = m_drawAllLines.m_vertices.data();
    bx::simd128_t normal;
    for (nanoem_rsize_t i = 0; i < numVertices; i++) {
        const nanoem_model_vertex_t *vertexPtr = vertices[i];
        const model::Vertex *vertex = model::Vertex::cast(vertexPtr);
        sg::LineVertexUnit &vertexUnit = vertexUnits[i];
        bx::simd128_t *ptr = reinterpret_cast<bx::simd128_t *>(glm::value_ptr(vertexUnit.m_position));
        if (const model::SoftBody *softBody = model::SoftBody::cast(vertex->softBody())) {
            bx::simd128_t position;
            softBody->getVertexPosition(vertexPtr, &position);
            vertexUnit.m_position = glm::make_vec3(reinterpret_cast<const nanoem_f32_t *>(&position));
        }
        else {
            Model::VertexUnit::performSkinningByType(vertex, ptr, &normal);
        }
        vertexUnit.m_color =
            m_selection->containsVertexIndex(i) ? Vector4U8(0xff, 0, 0, 0x7f) : Vector4U8(0, 0, 0, 0x7f);
    }
    internal::LineDrawer *drawer = lineDrawer();
    sg::update_buffer(m_drawAllLines.m_vertexBuffer, m_drawAllLines.m_vertices.data(),
        Inline::saturateInt32(m_drawAllLines.m_vertices.size() * sizeof(m_drawAllLines.m_vertices[0])));
    internal::LineDrawer::Option option(m_drawAllLines.m_vertexBuffer, 0);
    option.m_indexBuffer = m_drawAllLines.m_indexBuffer;
    option.m_primitiveType = SG_PRIMITIVETYPE_LINES;
    option.m_indexType = SG_INDEXTYPE_UINT32;
    option.m_enableDepthTest = option.m_enableBlendMode = true;
    for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
        const nanoem_model_material_t *material = materials[i];
        const nanoem_rsize_t numIndices = nanoemModelMaterialGetNumVertexIndices(material) * 2;
        if (isMaterialSelected(material)) {
            option.m_numIndices = numIndices;
            option.m_offset = indexOffset;
            drawer->drawPass(option);
        }
        indexOffset += numIndices;
    }
}

void
Model::drawAllJointShapes()
{
    nanoem_rsize_t numJoints;
    nanoem_model_joint_t *const *joints = nanoemModelGetAllJointObjects(m_opaque, &numJoints);
    const nanoem_model_joint_t *hoveredJointPtr = m_selection->hoveredJoint();
    for (nanoem_rsize_t i = 0; i < numJoints; i++) {
        const nanoem_model_joint_t *joint = joints[i];
        if (!hoveredJointPtr || hoveredJointPtr == joint) {
            drawJointShape(joint);
        }
    }
}

void
Model::drawAllRigidBodyShapes()
{
    nanoem_rsize_t numRigidBodies;
    nanoem_model_rigid_body_t *const *bodies = nanoemModelGetAllRigidBodyObjects(m_opaque, &numRigidBodies);
    const nanoem_model_rigid_body_t *hoveredRigidBodyPtr = m_selection->hoveredRigidBody();
    const nanoem_model_joint_t *hoveredJointPtr = m_selection->hoveredJoint();
    for (nanoem_rsize_t i = 0; i < numRigidBodies; i++) {
        const nanoem_model_rigid_body_t *rigidBodyPtr = bodies[i];
        if (hoveredJointPtr) {
            const nanoem_model_rigid_body_t *bodyA = nanoemModelJointGetRigidBodyAObject(hoveredJointPtr);
            const nanoem_model_rigid_body_t *bodyB = nanoemModelJointGetRigidBodyBObject(hoveredJointPtr);
            if (bodyA == rigidBodyPtr || bodyB == rigidBodyPtr) {
                drawRigidBodyShape(rigidBodyPtr);
            }
        }
        else if (!hoveredRigidBodyPtr || hoveredRigidBodyPtr == rigidBodyPtr) {
            drawRigidBodyShape(rigidBodyPtr);
        }
    }
}

void
Model::drawJointShape(const nanoem_model_joint_t *jointPtr)
{
    model::Joint *joint = model::Joint::cast(jointPtr);
    if (const par_shapes_mesh_s *shape = joint->generateShapeMesh(jointPtr)) {
        DrawIndexedBuffer &buffer = m_drawJoint[shape];
        const Vector3 color(m_selection->containsJoint(jointPtr) ? Vector3(1, 0, 0) : Vector3(1, 1, 0));
        nanoem_u32_t numVertices = buffer.fillShape(shape, Vector4(color, 0.25f));
        internal::LineDrawer *drawer = lineDrawer();
        internal::LineDrawer::Option option(buffer.m_vertexBuffer, numVertices);
        option.m_indexBuffer = buffer.m_indexBuffer;
        option.m_primitiveType = SG_PRIMITIVETYPE_TRIANGLES;
        option.m_indexType = SG_INDEXTYPE_UINT32;
        option.m_enableBlendMode = true;
        joint->getWorldTransformA(glm::value_ptr(option.m_worldTransform));
        drawer->drawPass(option);
        joint->getWorldTransformB(glm::value_ptr(option.m_worldTransform));
        drawer->drawPass(option);
    }
}

void
Model::drawRigidBodyShape(const nanoem_model_rigid_body_t *bodyPtr)
{
    model::RigidBody *rigidBody = model::RigidBody::cast(bodyPtr);
    if (const par_shapes_mesh_s *shape = rigidBody->generateShapeMesh(bodyPtr)) {
        DrawIndexedBuffer &buffer = m_drawRigidBody[shape];
        Vector3 color;
        if (m_selection->containsRigidBody(bodyPtr)) {
            color = Vector3(1, 0, 0);
        }
        else if (EnumUtils::isEnabled(m_states, kPrivateStateShowAllRigidBodiesColorByShape)) {
            color = model::RigidBody::colorByShapeType(bodyPtr);
        }
        else {
            color = model::RigidBody::colorByObjectType(bodyPtr);
        }
        nanoem_u32_t numVertices = buffer.fillShape(shape, Vector4(color, 0.25f));
        internal::LineDrawer *drawer = lineDrawer();
        internal::LineDrawer::Option option(buffer.m_vertexBuffer, numVertices);
        option.m_indexBuffer = buffer.m_indexBuffer;
        option.m_primitiveType = SG_PRIMITIVETYPE_TRIANGLES;
        option.m_indexType = SG_INDEXTYPE_UINT32;
        option.m_enableBlendMode = true;
        option.m_worldTransform = rigidBody->worldTransform();
        drawer->drawPass(option);
    }
}

void
Model::drawBoneConnection(const nanoem_model_bone_t *from, const Vector3 &destinationPosition, const Vector4 &color,
    nanoem_f32_t circleRadius, nanoem_f32_t thickness)
{
    nanoem_parameter_assert(from, "must not be nullptr");
    if (const model::Bone *fromBone = model::Bone::cast(from)) {
        const ICamera *camera = m_project->activeCamera();
        const Vector3 fromPosition(worldTransform(fromBone->worldTransform())[3]);
        const Vector2 fromCoord(camera->toDeviceScreenCoordinateInViewport(fromPosition));
        const Vector2 toCoord(camera->toDeviceScreenCoordinateInViewport(destinationPosition));
        nanoem_f32_t radians;
        if (glm::abs(glm::distance(toCoord, fromCoord)) > 0) {
            Vector2 n(glm::normalize(toCoord - fromCoord));
            radians = glm::atan(n.x, n.y);
        }
        else {
            radians = 0;
        }
        nanoem_f32_t x = circleRadius * glm::cos(radians);
        nanoem_f32_t y = circleRadius * glm::sin(radians);
        IPrimitive2D *primitive = m_project->primitive2D();
        primitive->strokeLine(fromCoord + Vector2(x, y), toCoord, color, thickness);
        primitive->strokeLine(fromCoord - Vector2(x, y), toCoord, color, thickness);
    }
}

void
Model::drawBonePoint(const Vector2 &deviceScaleCursor, const nanoem_model_bone_t *bonePtr, const Vector4 &inactive,
    const Vector4 &hovered)
{
    nanoem_parameter_assert(bonePtr, "must not be nullptr");
    Vector2 coord;
    if (const model::Bone *bone = model::Bone::cast(bonePtr)) {
        const bool intersected = intersectsBoneInViewport(deviceScaleCursor, bone, coord);
        const Vector4 color(intersected ? hovered : inactive);
        const nanoem_f32_t &circleRadius = m_project->deviceScaleCircleRadius();
        IPrimitive2D *primitive = m_project->primitive2D();
        if (nanoemModelBoneIsMovable(bonePtr)) {
            const nanoem_f32_t outerOffset = circleRadius, innerOffset = outerOffset * 0.75f,
                               innerWidth = circleRadius * 1.5f, outerWidth = circleRadius * 2.0f;
            primitive->strokeRect(
                Vector4(coord.x - outerOffset, coord.y - outerOffset, outerWidth, outerWidth), color, 0.0f, 2.0f);
            primitive->fillRect(
                Vector4(coord.x - innerOffset, coord.y - innerOffset, innerWidth, innerWidth), color, 0.0f);
        }
        else {
            const nanoem_f32_t innerRadius = circleRadius * 0.65f;
            primitive->strokeCircle(
                Vector4(coord.x - circleRadius, coord.y - circleRadius, circleRadius * 2, circleRadius * 2), color,
                2.0f);
            primitive->fillCircle(
                Vector4(coord.x - innerRadius, coord.y - innerRadius, innerRadius * 2, innerRadius * 2), color);
        }
    }
}

void
Model::drawBoneTooltip(const nanoem_model_bone_t *bonePtr)
{
    nanoem_parameter_assert(bonePtr, "must not be nullptr");
    if (const model::Bone *bone = model::Bone::cast(bonePtr)) {
        const char *name = bone->nameConstString();
        m_project->primitive2D()->drawTooltip(name, StringUtils::length(name));
    }
}

void
Model::drawConstraintConnections(const Vector2 &deviceScaleCursor, const nanoem_model_constraint_t *constraint)
{
    nanoem_parameter_assert(constraint, "must not be nullptr");
    nanoem_rsize_t numJoints;
    nanoem_model_constraint_joint_t *const *joints = nanoemModelConstraintGetAllJointObjects(constraint, &numJoints);
    nanoem_f32_t thickness = m_project->logicalScaleCircleRadius() * 0.75f;
    nanoem_f32_t circleRadius = m_project->deviceScaleCircleRadius();
    for (nanoem_rsize_t j = 0; j < numJoints - 1; j++) {
        const nanoem_model_bone_t *jointBone = nanoemModelConstraintJointGetBoneObject(joints[j]);
        const nanoem_model_bone_t *nextJointBone = nanoemModelConstraintJointGetBoneObject(joints[j + 1]);
        const Vector3 &destinationPosition =
            Vector3(worldTransform(model::Bone::cast(nextJointBone)->worldTransform())[3]);
        const Vector4 color(Color::hotToCold((numJoints - j - 1.5f) / nanoem_f32_t(numJoints)), 1);
        drawBoneConnection(jointBone, destinationPosition, color, circleRadius, thickness);
    }
    if (numJoints > 0) {
        const nanoem_model_bone_t *effectorBone = nanoemModelConstraintGetEffectorBoneObject(constraint);
        const nanoem_model_bone_t *jointBone = nanoemModelConstraintJointGetBoneObject(joints[0]);
        const Vector3 destinationPosition(worldTransform(model::Bone::cast(jointBone)->worldTransform())[3]);
        const Vector4 color(Color::hotToCold(1.0f), 1);
        drawBoneConnection(effectorBone, destinationPosition, color, circleRadius, thickness);
    }
    for (nanoem_rsize_t j = 0; j < numJoints; j++) {
        const nanoem_model_bone_t *jointBone = nanoemModelConstraintJointGetBoneObject(joints[j]);
        const Vector3 color(Color::hotToCold((numJoints - j - 1.0f) / nanoem_f32_t(numJoints)));
        drawBonePoint(deviceScaleCursor, jointBone, Vector4(color.x, color.y, color.z, 1.0f), Vector4(1, 0, 0, 1));
    }
    const nanoem_model_bone_t *effectorBone = nanoemModelConstraintGetEffectorBoneObject(constraint);
    const Vector3 color(Color::hotToCold(1.0f));
    drawBonePoint(deviceScaleCursor, effectorBone, Vector4(color.x, color.y, color.z, 1.0f), Vector4(1, 0, 0, 1));
    const nanoem_model_bone_t *targetBone = nanoemModelConstraintGetTargetBoneObject(constraint);
    const Vector3 color2(Color::hotToCold(1.0f));
    drawBonePoint(deviceScaleCursor, targetBone, Vector4(color2.x, color2.y, color2.z, 1.0f), Vector4(1, 0, 0, 1));
}

void
Model::drawConstraintConnections(const Vector2 &deviceScaleCursor)
{
    nanoem_rsize_t numBones, numConstraints;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_opaque, &numBones);
    nanoem_model_constraint_t *const *constraints = nanoemModelGetAllConstraintObjects(m_opaque, &numConstraints);
    for (nanoem_rsize_t i = 0; i < numBones; i++) {
        const nanoem_model_bone_t *bone = bones[i];
        if (const nanoem_model_constraint_t *constraint = nanoemModelBoneGetConstraintObject(bone)) {
            drawConstraintConnections(deviceScaleCursor, constraint);
        }
    }
    for (nanoem_rsize_t i = 0; i < numConstraints; i++) {
        const nanoem_model_constraint_t *constraint = constraints[i];
        drawConstraintConnections(deviceScaleCursor, constraint);
    }
}

void
Model::drawConstraintPoint(const Vector4 &position, int j, int numIterations)
{
    const ICamera *camera = m_project->activeCamera();
    nanoem_f32_t radius = m_project->deviceScaleCircleRadius();
    const Vector2 coord(camera->toDeviceScreenCoordinateInViewport(Vector3(position)));
    const Vector3 jet(Color::jet((j + 1) / nanoem_f32_t(numIterations)));
    nanoem_f32_t extent = radius * 2;
    m_project->primitive2D()->fillCircle(
        Vector4(coord.x - radius, coord.y - radius, extent, extent), Vector4(jet, 1.0f));
}

void
Model::drawConstraintsHeatMap(const nanoem_model_constraint_t *constraintPtr)
{
    nanoem_parameter_assert(constraintPtr, "must not be nullptr");
    nanoem_rsize_t numJoints;
    const ICamera *camera = m_project->activeCamera();
    nanoem_f32_t radius = m_project->deviceScaleCircleRadius();
    const nanoem_rsize_t numIterations = nanoem_rsize_t(nanoemModelConstraintGetNumIterations(constraintPtr));
    model::Constraint *constraint = model::Constraint::cast(constraintPtr);
    nanoem_model_constraint_joint_t *const *joints = nanoemModelConstraintGetAllJointObjects(constraintPtr, &numJoints);
    IPrimitive2D *primitive = m_project->primitive2D();
    nanoem_f32_t extent = radius * 2;
    for (nanoem_rsize_t j = 0; j < numJoints; j++) {
        const nanoem_model_constraint_joint_t *joint = joints[j];
        for (nanoem_rsize_t k = 0; k < numIterations; k++) {
            {
                const model::Constraint::Joint *jointResult = constraint->jointIterationResult(joint, k);
                const Vector2 jointCoord(camera->toDeviceScreenCoordinateInViewport(jointResult->m_translation));
                const Vector3 jointJet(Color::jet((k + 1) / nanoem_f32_t(numIterations)));
                primitive->fillCircle(
                    Vector4(jointCoord.x - radius, jointCoord.y - radius, extent, extent), Vector4(jointJet, 1.0f));
            }
            {
                const model::Constraint::Joint *effectorResult = constraint->effectorIterationResult(joint, k);
                const Vector2 &effectorCoord =
                    camera->toDeviceScreenCoordinateInViewport(effectorResult->m_translation);
                const Vector3 effectorJet(Color::jet((k + 1) / nanoem_f32_t(numIterations)));
                primitive->fillCircle(Vector4(effectorCoord.x - radius, effectorCoord.y - radius, extent, extent),
                    Vector4(effectorJet, 1.0f));
            }
        }
    }
}

void
Model::drawConstraintsHeatMap()
{
    nanoem_rsize_t numBones, numConstraints;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_opaque, &numBones);
    nanoem_model_constraint_t *const *constraints = nanoemModelGetAllConstraintObjects(m_opaque, &numConstraints);
    for (nanoem_rsize_t i = 0; i < numBones; i++) {
        const nanoem_model_bone_t *bone = bones[i];
        if (const nanoem_model_constraint_t *constraint = nanoemModelBoneGetConstraintObject(bone)) {
            drawConstraintsHeatMap(constraint);
        }
    }
    for (nanoem_rsize_t i = 0; i < numConstraints; i++) {
        const nanoem_model_constraint_t *constraint = constraints[i];
        drawConstraintsHeatMap(constraint);
    }
}

nanoem_u16_t
Model::handle() const NANOEM_DECL_NOEXCEPT
{
    return m_handle;
}

const Project *
Model::project() const NANOEM_DECL_NOEXCEPT
{
    return m_project;
}

Project *
Model::project() NANOEM_DECL_NOEXCEPT
{
    return m_project;
}

const ICamera *
Model::localCamera() const NANOEM_DECL_NOEXCEPT
{
    return m_camera;
}

ICamera *
Model::localCamera() NANOEM_DECL_NOEXCEPT
{
    return m_camera;
}

const IModelObjectSelection *
Model::selection() const NANOEM_DECL_NOEXCEPT
{
    return m_selection;
}

IModelObjectSelection *
Model::selection() NANOEM_DECL_NOEXCEPT
{
    return m_selection;
}

String
Model::name() const
{
    return m_name;
}

const char *
Model::nameConstString() const NANOEM_DECL_NOEXCEPT
{
    return m_name.c_str();
}

String
Model::canonicalName() const
{
    return m_canonicalName;
}

const char *
Model::canonicalNameConstString() const NANOEM_DECL_NOEXCEPT
{
    return m_canonicalName.c_str();
}

void
Model::setName(const String &value)
{
    if (!(m_name == value)) {
        m_name = value;
        setDirty(true);
    }
}

String
Model::comment() const
{
    return m_comment;
}

void
Model::setComment(const String &value)
{
    setComment(value, m_project->castLanguage());
}

void
Model::setComment(const String &value, nanoem_language_type_t language)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    if (!(m_comment == value)) {
        nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
        nanoem_mutable_model_t *model = nanoemMutableModelCreateAsReference(m_opaque, &status);
        StringUtils::UnicodeStringScope scope(factory);
        if (StringUtils::tryGetString(factory, value, scope)) {
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            nanoemMutableModelSetComment(model, scope.value(), language, &status);
        }
        nanoemMutableModelDestroy(model);
        m_comment = value;
        setDirty(true);
    }
}

nanoem_codec_type_t
Model::codecType() const NANOEM_DECL_NOEXCEPT
{
    return nanoemModelGetCodecType(m_opaque);
}

void
Model::setCodecType(nanoem_codec_type_t value)
{
    if (codecType() != value) {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoem_mutable_model_t *mutableModel = nanoemMutableModelCreateAsReference(m_opaque, &status);
        nanoemMutableModelSetCodecType(mutableModel, value);
        nanoemMutableModelDestroy(mutableModel);
    }
}

StringMap
Model::annotations() const
{
    return m_annotations;
}

void
Model::setAnnotations(const StringMap &value)
{
    m_annotations = value;
}

const nanoem_model_t *
Model::data() const NANOEM_DECL_NOEXCEPT
{
    return m_opaque;
}

nanoem_model_t *
Model::data() NANOEM_DECL_NOEXCEPT
{
    return m_opaque;
}

model::Bone *
Model::sharedFallbackBone()
{
    if (!m_sharedFallbackBone) {
        m_sharedFallbackBone = model::Bone::create();
    }
    return m_sharedFallbackBone;
}

const nanoem_model_bone_t *
Model::activeBone() const NANOEM_DECL_NOEXCEPT
{
    return m_activeBonePairPtr.first;
}

nanoem_model_bone_t *
Model::activeBone() NANOEM_DECL_NOEXCEPT
{
    return const_cast<nanoem_model_bone_t *>(m_activeBonePairPtr.first);
}

void
Model::setActiveBone(const nanoem_model_bone_t *value)
{
    if (m_activeBonePairPtr.first != value) {
        const model::Bone *bone = model::Bone::cast(value);
        m_project->eventPublisher()->publishSetActiveBoneEvent(this, bone ? bone->nameConstString() : nullptr);
        m_activeBonePairPtr.first = value;
        if (m_project->drawType() == kDrawTypeVertexWeight) {
            markStagingVertexBufferDirty();
        }
    }
}

const nanoem_model_constraint_t *
Model::activeConstraint() const NANOEM_DECL_NOEXCEPT
{
    return m_activeConstraintPtr;
}

nanoem_model_constraint_t *
Model::activeConstraint() NANOEM_DECL_NOEXCEPT
{
    return const_cast<nanoem_model_constraint_t *>(m_activeConstraintPtr);
}

void
Model::setActiveConstraint(const nanoem_model_constraint_t *value)
{
    m_activeConstraintPtr = value;
}

const nanoem_model_morph_t *
Model::activeMorph(nanoem_model_morph_category_t category) const NANOEM_DECL_NOEXCEPT
{
    const nanoem_model_morph_t *morph = nullptr;
    if (category >= NANOEM_MODEL_MORPH_CATEGORY_FIRST_ENUM && category < NANOEM_MODEL_MORPH_CATEGORY_MAX_ENUM) {
        morph = m_activeMorphPtr[category];
    }
    return morph;
}

nanoem_model_morph_t *
Model::activeMorph(nanoem_model_morph_category_t category) NANOEM_DECL_NOEXCEPT
{
    nanoem_model_morph_t *morph = nullptr;
    if (category >= NANOEM_MODEL_MORPH_CATEGORY_FIRST_ENUM && category < NANOEM_MODEL_MORPH_CATEGORY_MAX_ENUM) {
        morph = const_cast<nanoem_model_morph_t *>(m_activeMorphPtr[category]);
    }
    return morph;
}

void
Model::setActiveMorph(nanoem_model_morph_category_t category, const nanoem_model_morph_t *value)
{
    if (category >= NANOEM_MODEL_MORPH_CATEGORY_FIRST_ENUM && category < NANOEM_MODEL_MORPH_CATEGORY_MAX_ENUM &&
        m_activeMorphPtr[category] != value) {
        const model::Morph *morph = model::Morph::cast(value);
        m_project->eventPublisher()->publishSetActiveMorphEvent(this, morph ? morph->nameConstString() : nullptr);
        m_activeMorphPtr[category] = value;
    }
}

const nanoem_model_morph_t *
Model::activeMorph() const NANOEM_DECL_NOEXCEPT
{
    return activeMorph(NANOEM_MODEL_MORPH_CATEGORY_BASE);
}

nanoem_model_morph_t *
Model::activeMorph() NANOEM_DECL_NOEXCEPT
{
    return activeMorph(NANOEM_MODEL_MORPH_CATEGORY_BASE);
}

void
Model::setActiveMorph(const nanoem_model_morph_t *value)
{
    setActiveMorph(NANOEM_MODEL_MORPH_CATEGORY_BASE, value);
}

const nanoem_model_material_t *
Model::activeMaterial() const NANOEM_DECL_NOEXCEPT
{
    return m_activeMaterialPtr;
}

void
Model::setActiveMaterial(const nanoem_model_material_t *value)
{
    if (m_activeMaterialPtr != value) {
        m_activeMaterialPtr = value;
    }
}

const nanoem_model_bone_t *
Model::activeOutsideParentSubjectBone() const NANOEM_DECL_NOEXCEPT
{
    return m_activeBonePairPtr.second;
}

void
Model::setActiveOutsideParentSubjectBone(const nanoem_model_bone_t *value)
{
    if (m_activeBonePairPtr.second != value) {
        m_activeBonePairPtr.second = value;
    }
}

const undo_stack_t *
Model::undoStack() const NANOEM_DECL_NOEXCEPT
{
    return m_undoStack;
}

undo_stack_t *
Model::undoStack() NANOEM_DECL_NOEXCEPT
{
    return m_undoStack;
}

Matrix4x4
Model::worldTransform() const NANOEM_DECL_NOEXCEPT
{
    return worldTransform(kInitialWorldMatrix);
}

Matrix4x4
Model::worldTransform(const Matrix4x4 &initial) const NANOEM_DECL_NOEXCEPT
{
    return initial;
}

bool
Model::containsBone(const nanoem_unicode_string_t *name) const NANOEM_DECL_NOEXCEPT
{
    nanoem_parameter_assert(name, "must not be nullptr");
    nanoem_rsize_t numObjects;
    nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_opaque, &numObjects);
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        const nanoem_model_bone_t *bone = bones[i];
        const nanoem_unicode_string_t *boneName = nanoemModelBoneGetName(bone, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        if (nanoemUnicodeStringFactoryCompareString(factory, name, boneName) == 0) {
            return true;
        }
    }
    return false;
}

bool
Model::containsBone(const nanoem_model_bone_t *bone) const NANOEM_DECL_NOEXCEPT
{
    for (BoneHashMap::const_iterator it = m_bones.begin(), end = m_bones.end(); it != end; ++it) {
        if (it->second == bone) {
            return true;
        }
    }
    return false;
}

bool
Model::containsMorph(const nanoem_unicode_string_t *name) const NANOEM_DECL_NOEXCEPT
{
    nanoem_parameter_assert(name, "must not be nullptr");
    nanoem_rsize_t numObjects;
    nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
    nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(m_opaque, &numObjects);
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        const nanoem_model_morph_t *morph = morphs[i];
        const nanoem_unicode_string_t *morphName = nanoemModelMorphGetName(morph, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        if (nanoemUnicodeStringFactoryCompareString(factory, name, morphName) == 0) {
            return true;
        }
    }
    return false;
}

bool
Model::containsMorph(const nanoem_model_morph_t *value) const NANOEM_DECL_NOEXCEPT
{
    for (MorphHashMap::const_iterator it = m_morphs.begin(), end = m_morphs.end(); it != end; ++it) {
        if (it->second == value) {
            return true;
        }
    }
    return false;
}

bool
Model::hasAnyDirtyBone() const NANOEM_DECL_NOEXCEPT
{
    nanoem_rsize_t numBones;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_opaque, &numBones);
    bool result = false;
    for (nanoem_rsize_t i = 0; i < numBones; i++) {
        if (const model::Bone *bone = model::Bone::cast(bones[i])) {
            result |= bone->isDirty();
        }
    }
    return result;
}

bool
Model::hasAnyDirtyMorph() const NANOEM_DECL_NOEXCEPT
{
    nanoem_rsize_t numMorphs;
    nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(m_opaque, &numMorphs);
    bool result = false;
    for (nanoem_rsize_t i = 0; i < numMorphs; i++) {
        if (const model::Morph *morph = model::Morph::cast(morphs[i])) {
            result |= morph->isDirty();
        }
    }
    return result;
}

bool
Model::isRigidBodyBound(const nanoem_model_bone_t *value) const NANOEM_DECL_NOEXCEPT
{
    return !m_boneBoundRigidBodies.empty() && m_boneBoundRigidBodies.find(value) != m_boneBoundRigidBodies.end();
}

model::Bone::Set
Model::findInherentBoneSet(const nanoem_model_bone_t *bone) const
{
    model::Bone::SetTree::const_iterator it = m_inherentBones.find(bone);
    return it != m_inherentBones.end() ? it->second : model::Bone::Set();
}

const nanoem_model_bone_t *
Model::intersectsBone(const Vector2 &deviceScaleCursor, nanoem_rsize_t &candidateBoneIndex) const NANOEM_DECL_NOEXCEPT
{
    Vector2 coord;
    nanoem_rsize_t numBones;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_opaque, &numBones);
    const bool showAllBones = isShowAllBones();
    model::Bone::List candidateBones;
    for (nanoem_rsize_t i = 0; i < numBones; i++) {
        const nanoem_model_bone_t *bonePtr = bones[i];
        if (isBoneSelectable(bonePtr) && (showAllBones || !isRigidBodyBound(bonePtr))) {
            const model::Bone *bone = model::Bone::cast(bonePtr);
            if (intersectsBoneInWindow(deviceScaleCursor, bone, coord)) {
                candidateBones.push_back(bonePtr);
            }
        }
    }
    const nanoem_model_bone_t *bonePtr = nullptr;
    if (candidateBones.size() > 1) {
        candidateBoneIndex = (candidateBoneIndex + 1) % candidateBones.size();
        bonePtr = candidateBones[candidateBoneIndex];
    }
    else {
        candidateBoneIndex = 0;
        if (candidateBones.size() == 1) {
            bonePtr = candidateBones.front();
        }
    }
    return bonePtr;
}

const nanoem_model_bone_t *
Model::findBone(const nanoem_unicode_string_t *name) const
{
    String s;
    StringUtils::getUtf8String(name, m_project->unicodeStringFactory(), s);
    return findBone(s);
}

const nanoem_model_bone_t *
Model::findBone(const String &name) const NANOEM_DECL_NOEXCEPT
{
    const nanoem_model_bone_t *found = nullptr;
    if (!name.empty()) {
        BoneHashMap::const_iterator it = m_bones.find(name);
        found = it != m_bones.end() ? it->second : nullptr;
    }
    return found;
}

const nanoem_model_bone_t *
Model::findRedoBone(nanoem_rsize_t index) const NANOEM_DECL_NOEXCEPT
{
    const nanoem_model_bone_t *bonePtr = nullptr;
    if (index < m_redoBoneNames.size()) {
        bonePtr = findBone(m_redoBoneNames[index]);
    }
    return bonePtr;
}

const nanoem_model_morph_t *
Model::findMorph(const nanoem_unicode_string_t *name) const
{
    String s;
    StringUtils::getUtf8String(name, m_project->unicodeStringFactory(), s);
    return findMorph(s);
}

const nanoem_model_morph_t *
Model::findMorph(const String &name) const NANOEM_DECL_NOEXCEPT
{
    const nanoem_model_morph_t *found = nullptr;
    if (!name.empty()) {
        MorphHashMap::const_iterator it = m_morphs.find(name);
        found = it != m_morphs.end() ? it->second : nullptr;
    }
    return found;
}

const nanoem_model_morph_t *
Model::findRedoMorph(nanoem_rsize_t index) const NANOEM_DECL_NOEXCEPT
{
    const nanoem_model_morph_t *morphPtr = nullptr;
    if (index < m_redoMorphNames.size()) {
        morphPtr = findMorph(m_redoMorphNames[index]);
    }
    return morphPtr;
}

const nanoem_model_constraint_t *
Model::findConstraint(const nanoem_unicode_string_t *name) const NANOEM_DECL_NOEXCEPT
{
    nanoem_parameter_assert(name, "must not be nullptr");
    nanoem_rsize_t numObjects;
    nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
    if (nanoem_model_constraint_t *const *constraints = nanoemModelGetAllConstraintObjects(m_opaque, &numObjects)) {
        for (nanoem_rsize_t i = 0; i < numObjects; i++) {
            const nanoem_model_constraint_t *constraint = constraints[i];
            const nanoem_model_bone_t *targetBone = nanoemModelConstraintGetTargetBoneObject(constraint);
            if (nanoemUnicodeStringFactoryCompareString(
                    factory, nanoemModelBoneGetName(targetBone, NANOEM_LANGUAGE_TYPE_FIRST_ENUM), name) == 0) {
                return constraint;
            }
        }
    }
    else {
        nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_opaque, &numObjects);
        for (nanoem_rsize_t i = 0; i < numObjects; i++) {
            const nanoem_model_bone_t *bone = bones[i];
            if (const nanoem_model_constraint_t *constraint = nanoemModelBoneGetConstraintObject(bone)) {
                if (nanoemUnicodeStringFactoryCompareString(
                        factory, nanoemModelBoneGetName(bone, NANOEM_LANGUAGE_TYPE_FIRST_ENUM), name) == 0) {
                    return constraint;
                }
            }
        }
    }
    return nullptr;
}

const nanoem_model_constraint_t *
Model::findConstraint(const nanoem_model_bone_t *bone) const NANOEM_DECL_NOEXCEPT
{
    ConstraintMap::const_iterator it = m_constraints.find(bone);
    return it != m_constraints.end() ? it->second : nullptr;
}

bool
Model::isConstraintJointBone(const nanoem_model_bone_t *bone) const NANOEM_DECL_NOEXCEPT
{
    return m_constraintJointBones.find(bone) != m_constraintJointBones.end();
}

bool
Model::isConstraintJointBoneActive(const nanoem_model_bone_t *bone) const NANOEM_DECL_NOEXCEPT
{
    ResolveConstraintJointParentMap::const_iterator it = m_constraintJointBones.find(bone);
    bool result = false;
    if (it != m_constraintJointBones.end()) {
        if (const model::Constraint *constraint = model::Constraint::cast(it->second)) {
            result = constraint->isEnabled();
        }
    }
    return result;
}

bool
Model::isConstraintEffectorBone(const nanoem_model_bone_t *bone) const NANOEM_DECL_NOEXCEPT
{
    return m_constraintEffectorBones.find(bone) != m_constraintEffectorBones.end();
}

model::Bone::ListTree
Model::parentBoneTree() const
{
    return m_parentBoneTree;
}

void
Model::getAllImageViews(ImageViewMap &value) const
{
    value.clear();
    for (ImageMap::const_iterator it = m_imageHandles.begin(), end = m_imageHandles.end(); it != end; ++it) {
        value.insert(tinystl::make_pair(it->first, static_cast<IImageView *>(it->second)));
    }
}

URI
Model::resolveImageURI(const String &filename) const
{
    nanoem_parameter_assert(!filename.empty(), "must not be empty");
    FileEntityMap::const_iterator it = m_imageURIs.find(filename);
    return it != m_imageURIs.end() ? it->second : URI();
}

BoundingBox
Model::boundingBox() const NANOEM_DECL_NOEXCEPT
{
    return m_boundingBox;
}

String
Model::filename() const
{
    return Project::isArchiveURI(m_fileURI) ? URI::lastPathComponent(m_fileURI.fragment())
                                            : m_fileURI.lastPathComponent();
}

const URI *
Model::fileURIPtr() const NANOEM_DECL_NOEXCEPT
{
    return &m_fileURI;
}

URI
Model::fileURI() const
{
    return m_fileURI;
}

URI
Model::resolvedFileURI() const
{
    return m_project->resolveFileURI(m_fileURI);
}

void
Model::setFileURI(const URI &value)
{
    m_fileURI = value;
}

const IEffect *
Model::activeEffect() const NANOEM_DECL_NOEXCEPT
{
    nanoem_assert(m_activeEffectPtrPair.first, "must be called after Model::setActiveEffect");
    return m_activeEffectPtrPair.first;
}

IEffect *
Model::activeEffect() NANOEM_DECL_NOEXCEPT
{
    nanoem_assert(m_activeEffectPtrPair.first, "must be called after Model::setActiveEffect");
    return m_activeEffectPtrPair.first;
}

void
Model::setActiveEffect(IEffect *value)
{
    ModelProgramBundle *bundle = m_project->sharedResourceRepository()->modelProgramBundle();
    OffscreenPassiveRenderTargetEffect effect = { nullptr, true };
    if (value && value != bundle) {
        effect.m_passiveEffect = value;
        m_activeEffectPtrPair.first = value;
    }
    else {
        effect.m_passiveEffect = static_cast<IEffect *>(bundle);
        m_activeEffectPtrPair.first = bundle;
    }
    m_offscreenPassiveRenderTargetEffects[Effect::kOffscreenOwnerNameMain] = effect;
}

const IEffect *
Model::passiveEffect() const NANOEM_DECL_NOEXCEPT
{
    return m_activeEffectPtrPair.second;
}

IEffect *
Model::passiveEffect() NANOEM_DECL_NOEXCEPT
{
    return m_activeEffectPtrPair.second;
}

void
Model::setPassiveEffect(IEffect *value)
{
    if (m_activeEffectPtrPair.second != value) {
        if (Effect *effect = m_project->upcastEffect(value)) {
            effect->createAllDrawableRenderTargetColorImages(this);
        }
        m_activeEffectPtrPair.second = value;
    }
}

Model::UserData
Model::userData() const
{
    return m_userData;
}

void
Model::setUserData(const UserData &value)
{
    m_userData = value;
}

Model::AxisType
Model::transformAxisType() const NANOEM_DECL_NOEXCEPT
{
    return m_transformAxisType;
}

void
Model::setTransformAxisType(AxisType value)
{
    if (value != m_transformAxisType) {
        if (value >= kAxisTypeFirstEnum && value < kAxisTypeMaxEnum) {
            m_transformAxisType = value;
        }
        else {
            m_transformAxisType = kAxisTypeMaxEnum;
        }
    }
}

Model::TransformCoordinateType 
Model::gizmoTransformCoordinateType() const NANOEM_DECL_NOEXCEPT
{
    return m_gizmoTransformCoordinateType;
}

void 
Model::setGizmoTransformCoordinateType(TransformCoordinateType value)
{
    m_gizmoTransformCoordinateType = value;
}

Model::GizmoOperationType 
Model::gizmoOperationType() const NANOEM_DECL_NOEXCEPT
{
    return m_gizmoOperationType;
}

void 
Model::setGizmoOperationType(GizmoOperationType value)
{
    m_gizmoOperationType = value;
}

Model::TransformCoordinateType
Model::transformCoordinateType() const NANOEM_DECL_NOEXCEPT
{
    return m_transformCoordinateType;
}

void
Model::setTransformCoordinateType(TransformCoordinateType value)
{
    if (value != m_transformCoordinateType) {
        if (value >= kTransformCoordinateTypeFirstEnum && value < kTransformCoordinateTypeMaxEnum) {
            m_transformCoordinateType = value;
        }
        else {
            m_transformCoordinateType = kTransformCoordinateTypeFirstEnum;
        }
    }
}

void
Model::toggleTransformCoordinateType()
{
    switch (transformCoordinateType()) {
    case kTransformCoordinateTypeGlobal:
    default:
        setTransformCoordinateType(kTransformCoordinateTypeLocal);
        break;
    case kTransformCoordinateTypeLocal:
        setTransformCoordinateType(kTransformCoordinateTypeGlobal);
        break;
    }
}

Matrix4x4 
Model::pivotMatrix() const NANOEM_DECL_NOEXCEPT
{
    return m_pivotMatrix;
}

void 
Model::setPivotMatrix(const Matrix4x4 &value)
{
    m_pivotMatrix = value;
}

Vector4
Model::edgeColor() const NANOEM_DECL_NOEXCEPT
{
    return m_edgeColor;
}

void
Model::setEdgeColor(const Vector4 &value)
{
    m_edgeColor = value;
}

nanoem_f32_t
Model::edgeSize() const NANOEM_DECL_NOEXCEPT
{
    nanoem_rsize_t numBones;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_opaque, &numBones);
    nanoem_f32_t value = 0;
    if (numBones > 1) {
        const ICamera *camera = m_project->activeCamera();
        const Vector3 bonePosition(model::Bone::cast(bones[1])->worldTransformOrigin());
        value = glm::distance(bonePosition, camera->position()) * glm::clamp(camera->fov() / 30.0f, 0.0f, 1.0f) *
            0.001f * m_edgeSizeScaleFactor;
    }
    return value;
}

nanoem_f32_t
Model::edgeSizeScaleFactor() const NANOEM_DECL_NOEXCEPT
{
    return m_edgeSizeScaleFactor;
}

void
Model::setEdgeSizeScaleFactor(nanoem_f32_t value)
{
    m_edgeSizeScaleFactor = value;
}

nanoem_f32_t
Model::opacity() const NANOEM_DECL_NOEXCEPT
{
    return m_opacity;
}

void
Model::setOpacity(nanoem_f32_t value)
{
    m_opacity = glm::clamp(value, 0.0f, 1.0f);
}

bool
Model::isMorphWeightFocused() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kPrivateStateMorphWeightFocus, m_states);
}

void
Model::setMorphWeightFocused(bool value)
{
    EnumUtils::setEnabled(kPrivateStateMorphWeightFocus, m_states, value);
}

bool
Model::isShowAllBones() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kPrivateStateShowAllBones, m_states);
}

void
Model::setShowAllBones(bool value)
{
    if (isShowAllBones() != value) {
        EnumUtils::setEnabled(kPrivateStateShowAllBones, m_states, value);
        if (m_project->activeModel() == this) {
            m_project->eventPublisher()->publishToggleActiveModelShowAllBonesEvent(value);
        }
    }
}

bool
Model::isShowAllVertexPoints() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kPrivateStateShowAllVertexPoints, m_states);
}

void
Model::setShowAllVertexPoints(bool value)
{
    if (isShowAllVertexPoints() != value) {
        EnumUtils::setEnabled(kPrivateStateShowAllVertexPoints, m_states, value);
        if (m_project->activeModel() == this) {
            m_project->eventPublisher()->publishToggleActiveModelShowAllVertexPointsEvent(value);
        }
    }
}

bool
Model::isShowAllVertexFaces() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kPrivateStateShowAllVertexFaces, m_states);
}

void
Model::setShowAllVertexFaces(bool value)
{
    if (isShowAllVertexFaces() != value) {
        EnumUtils::setEnabled(kPrivateStateShowAllVertexFaces, m_states, value);
        if (m_project->activeModel() == this) {
            m_project->eventPublisher()->publishToggleActiveModelShowAllVertexFacesEvent(value);
        }
    }
}

bool
Model::isShowAllRigidBodies() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kPrivateStateShowAllRigidBodies, m_states);
}

void
Model::setShowAllRigidBodies(bool value)
{
    if (isShowAllRigidBodies() != value) {
        EnumUtils::setEnabled(kPrivateStateShowAllRigidBodies, m_states, value);
        if (m_project->activeModel() == this) {
            m_project->eventPublisher()->publishToggleActiveModelShowAllRigidBodiesEvent(value);
        }
    }
}

bool
Model::isShowAllJoints() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kPrivateStateShowAllJoints, m_states);
}

void
Model::setShowAllJoints(bool value)
{
    if (isShowAllJoints() != value) {
        EnumUtils::setEnabled(kPrivateStateShowAllJoints, m_states, value);
    }
}

bool
Model::isGroundShadowEnabled() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kPrivateStateEnableGroundShadow, m_states);
}

void
Model::setGroundShadowEnabled(bool value)
{
    if (isShadowMapEnabled() != value) {
        EnumUtils::setEnabled(kPrivateStateEnableGroundShadow, m_states, value);
    }
}

bool
Model::isShadowMapEnabled() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kPrivateStateEnableShadowMap, m_states);
}

void
Model::setShadowMapEnabled(bool value)
{
    if (isShadowMapEnabled() != value) {
        EnumUtils::setEnabled(kPrivateStateEnableShadowMap, m_states, value);
        if (m_project->activeModel() == this) {
            m_project->eventPublisher()->publishToggleActiveModelShadowMapEnabledEvent(value);
        }
    }
}

bool
Model::isPhysicsSimulationEnabled() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kPrivateStatePhysicsSimulation, m_states);
}

void
Model::setPhysicsSimulationEnabled(bool value)
{
    if (isPhysicsSimulationEnabled() != value) {
        EnumUtils::setEnabled(kPrivateStatePhysicsSimulation, m_states, value);
        nanoem_rsize_t numRigidBodies, numJoints, numSoftBodies;
        nanoem_model_rigid_body_t *const *rigidBodies = nanoemModelGetAllRigidBodyObjects(m_opaque, &numRigidBodies);
        nanoem_model_joint_t *const *joints = nanoemModelGetAllJointObjects(m_opaque, &numJoints);
        nanoem_model_soft_body_t *const *softBodies = nanoemModelGetAllSoftBodyObjects(m_opaque, &numSoftBodies);
        if (value) {
            for (nanoem_rsize_t i = 0; i < numSoftBodies; i++) {
                if (model::SoftBody *softBody = model::SoftBody::cast(softBodies[i])) {
                    softBody->enable();
                }
            }
            for (nanoem_rsize_t i = 0; i < numRigidBodies; i++) {
                if (model::RigidBody *rigidBody = model::RigidBody::cast(rigidBodies[i])) {
                    rigidBody->enable();
                }
            }
            for (nanoem_rsize_t i = 0; i < numJoints; i++) {
                if (model::Joint *joint = model::Joint::cast(joints[i])) {
                    joint->enable();
                }
            }
        }
        else {
            for (nanoem_rsize_t i = 0; i < numJoints; i++) {
                if (model::Joint *joint = model::Joint::cast(joints[i])) {
                    joint->disable();
                }
            }
            for (nanoem_rsize_t i = 0; i < numRigidBodies; i++) {
                if (model::RigidBody *rigidBody = model::RigidBody::cast(rigidBodies[i])) {
                    rigidBody->disable();
                }
            }
            for (nanoem_rsize_t i = 0; i < numSoftBodies; i++) {
                if (model::SoftBody *softBody = model::SoftBody::cast(softBodies[i])) {
                    softBody->disable();
                }
            }
        }
    }
}

bool
Model::isAddBlendEnabled() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kPrivateStateEnableAddBlend, m_states);
}

void
Model::setAddBlendEnabled(bool value)
{
    if (isAddBlendEnabled() != value) {
        EnumUtils::setEnabled(kPrivateStateEnableAddBlend, m_states, value);
        if (m_project->activeModel() == this) {
            m_project->eventPublisher()->publishToggleActiveModelAddBlendEnabledEvent(value);
        }
    }
}

bool
Model::isVisible() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kPrivateStateVisible, m_states);
}

void
Model::setVisible(bool value)
{
    if (isVisible() != value) {
        EnumUtils::setEnabled(kPrivateStateVisible, m_states, value);
        if (Effect *effect = m_project->resolveEffect(this)) {
            effect->setEnabled(value);
        }
        if (m_project->activeModel() == this) {
            m_project->eventPublisher()->publishToggleActiveModelVisibleEvent(value);
        }
    }
}

bool
Model::isDirty() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kPrivateStateDirty, m_states);
}

void
Model::setDirty(bool value)
{
    EnumUtils::setEnabled(kPrivateStateDirty, m_states, value);
}

bool
Model::isTranslucent() const NANOEM_DECL_NOEXCEPT
{
    return opacity() - Constants::kEpsilon < 0.5f;
}

bool
Model::isUploaded() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kPrivateStateUploaded, m_states);
}

} /* namespace nanoem */
