/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/Project.h"

#include "emapp/Accessory.h"
#include "emapp/AccessoryProgramBundle.h"
#include "emapp/Archiver.h"
#include "emapp/CommandRegistrator.h"
#include "emapp/DirectionalLight.h"
#include "emapp/Effect.h"
#include "emapp/EnumUtils.h"
#include "emapp/FileUtils.h"
#include "emapp/Grid.h"
#include "emapp/IAudioPlayer.h"
#include "emapp/IBackgroundVideoRenderer.h"
#include "emapp/IDebugCapture.h"
#include "emapp/IEventPublisher.h"
#include "emapp/IFileManager.h"
#include "emapp/IModelObjectSelection.h"
#include "emapp/IMotionKeyframeSelection.h"
#include "emapp/IPrimitive2D.h"
#include "emapp/ITrack.h"
#include "emapp/ITranslator.h"
#include "emapp/ImageLoader.h"
#include "emapp/ListUtils.h"
#include "emapp/ModalDialogFactory.h"
#include "emapp/Model.h"
#include "emapp/ModelProgramBundle.h"
#include "emapp/PerspectiveCamera.h"
#include "emapp/PhysicsEngine.h"
#include "emapp/PixelFormat.h"
#include "emapp/PluginFactory.h"
#include "emapp/Progress.h"
#include "emapp/ShadowCamera.h"
#include "emapp/StringUtils.h"
#include "emapp/UUID.h"
#include "emapp/command/BatchUndoCommandListCommand.h"
#include "emapp/command/MotionSnapshotCommand.h"
#include "emapp/command/TransformBoneCommand.h"
#include "emapp/internal/BlitPass.h"
#include "emapp/internal/ClearPass.h"
#include "emapp/internal/DebugDrawer.h"
#include "emapp/internal/project/Archive.h"
#include "emapp/internal/project/JSON.h"
#include "emapp/internal/project/Native.h"
#include "emapp/internal/project/PMM.h"
#include "emapp/internal/project/Redo.h"
#include "emapp/internal/project/Track.h"
#include "emapp/model/Morph.h"
#include "emapp/private/CommonInclude.h"
#include "protoc/application.pb-c.h"

#include "bx/handlealloc.h"
#include "bx/hash.h"
#include "bx/timer.h"

#include "lz4/lib/lz4.h"
#include "sokol/sokol_time.h"
#include "undo/undo.h"
#include "zlib.h"

extern "C" {
#include "wildcardcmp.h"
}

namespace nanoem {
namespace {

static const Vector4 kRectCoordination = Vector4(0, 0, 1, 1);
static const nanoem_u64_t kDisableHiddenBoneBoundsRigidBody = 1ull << 1;
static const nanoem_u64_t kDisplayUserInterface = 1ull << 2;
static const nanoem_u64_t kDisplayTransformHandle = 1ull << 3;
static const nanoem_u64_t kEnableLoop = 1ull << 4;
static const nanoem_u64_t kEnableSharedCamera = 1ull << 5;
static const nanoem_u64_t kEnableGroundShadow = 1ull << 6;
static const nanoem_u64_t kEnableMultipleBoneSelection = 1ull << 7;
static const nanoem_u64_t kEnableBezierCurveAdjustment = 1ull << 8;
static const nanoem_u64_t kEnableMotionMerge = 1ull << 9;
static const nanoem_u64_t kEnableEffectPlugin = 1ull << 10;
static const nanoem_u64_t kEnableViewportLocked = 1ull << 11;
static const nanoem_u64_t kDisableDisplaySync = 1ull << 12;
static const nanoem_u64_t kPrimaryCursorTypeLeft = 1ull << 13;
static const nanoem_u64_t kLoadingRedoFile = 1ull << 14;
static const nanoem_u64_t kEnablePlayingAudioPart = 1ull << 15;
static const nanoem_u64_t kEnableViewportWithTransparent = 1ull << 16;
static const nanoem_u64_t kEnableCompiledEffectCache = 1ull << 17;
static const nanoem_u64_t kResetAllPasses = 1ull << 18;
static const nanoem_u64_t kCancelRequested = 1ull << 19;
static const nanoem_u64_t kEnableUniformedViewportImageSize = 1ull << 20;
static const nanoem_u64_t kViewportHovered = 1ull << 21;
static const nanoem_u64_t kEnableFPSCounter = 1ull << 22;
static const nanoem_u64_t kEnablePerformanceMonitor = 1ull << 23;
static const nanoem_u64_t kInputTextFocus = 1ull << 24;
static const nanoem_u64_t kViewportImageSizeChanged = 1ull << 25;
static const nanoem_u64_t kEnablePhysicsSimulationForBoneKeyframe = 1ull << 26;
static const nanoem_u64_t kEnableImageAnisotropy = 1ull << 27;
static const nanoem_u64_t kEnableImageMipmap = 1ull << 28;
static const nanoem_u64_t kEnablePowerSaving = 1ull << 29;
static const nanoem_u64_t kEnableModelEditing = 1ull << 30;
static const nanoem_u64_t kViewportWindowDetached = 1ull << 31;

static const nanoem_u64_t kPrivateStateInitialValue = kDisplayTransformHandle | kDisplayUserInterface |
    kEnableMotionMerge | kEnableUniformedViewportImageSize | kEnableFPSCounter | kEnablePerformanceMonitor |
    kEnablePhysicsSimulationForBoneKeyframe | kEnableImageAnisotropy;

struct AccessoryFinder {
    typedef bool (*FindProc)(const Accessory *item, const void *arg);
    static bool findByURI(const Accessory *item, const void *arg);
    static bool findByName(const Accessory *item, const void *arg);
    static bool findByFilename(const Accessory *item, const void *arg);
    static Accessory *execute(const Project::AccessoryList &models, FindProc finder, const void *arg);
};

bool
AccessoryFinder::findByURI(const Accessory *item, const void *arg)
{
    const URI *right = static_cast<const URI *>(arg);
    return item->fileURIPtr()->equalsToAbsolutePathConstString(right->absolutePathConstString());
}

bool
AccessoryFinder::findByName(const Accessory *item, const void *arg)
{
    const String *right = static_cast<const String *>(arg);
    return StringUtils::equals(item->nameConstString(), right->c_str());
}

bool
AccessoryFinder::findByFilename(const Accessory *item, const void *arg)
{
    const String *right = static_cast<const String *>(arg);
    return item->fileURIPtr()->equalsToFilenameConstString(right->c_str());
}

Accessory *
AccessoryFinder::execute(const Project::AccessoryList &models, AccessoryFinder::FindProc finder, const void *arg)
{
    Accessory *candidateModelPtr = nullptr;
    for (Project::AccessoryList::const_iterator it = models.begin(), end = models.end(); it != end; ++it) {
        Accessory *accessory = *it;
        if (finder(accessory, arg)) {
            candidateModelPtr = accessory;
            break;
        }
    }
    return candidateModelPtr;
}

struct ModelFinder {
    typedef bool (*FindProc)(const Model *item, const void *arg);
    static bool findByURI(const Model *item, const void *arg);
    static bool findByName(const Model *item, const void *arg);
    static bool findByFilename(const Model *item, const void *arg);
    static Model *execute(const Project::ModelList &models, FindProc finder, const void *arg);
};

bool
ModelFinder::findByURI(const Model *item, const void *arg)
{
    const URI *right = static_cast<const URI *>(arg);
    return item->fileURIPtr()->equalsToAbsolutePathConstString(right->absolutePathConstString());
}

bool
ModelFinder::findByName(const Model *item, const void *arg)
{
    const String *right = static_cast<const String *>(arg);
    return StringUtils::equals(item->nameConstString(), right->c_str()) ||
        StringUtils::equals(item->canonicalNameConstString(), right->c_str());
}

bool
ModelFinder::findByFilename(const Model *item, const void *arg)
{
    const String *right = static_cast<const String *>(arg);
    return item->fileURIPtr()->equalsToFilenameConstString(right->c_str());
}

Model *
ModelFinder::execute(const Project::ModelList &models, ModelFinder::FindProc finder, const void *arg)
{
    Model *candidateModelPtr = nullptr;
    for (Project::ModelList::const_iterator it = models.begin(), end = models.end(); it != end; ++it) {
        Model *model = *it;
        if (finder(model, arg)) {
            candidateModelPtr = model;
            break;
        }
    }
    return candidateModelPtr;
}

struct PassScope {
    PassScope(sg_pass &target, sg_pass value);
    ~PassScope() NANOEM_DECL_NOEXCEPT;
    sg_pass &m_target;
};

PassScope::PassScope(sg_pass &target, sg_pass value)
    : m_target(target)
{
    target = value;
}

PassScope::~PassScope() NANOEM_DECL_NOEXCEPT
{
    m_target = { SG_INVALID_ID };
}

struct EnumStringifyUtils : private NonCopyable {
    static const char *toString(IEffect::ScriptOrderType order) NANOEM_DECL_NOEXCEPT;
    static const char *toString(IDrawable::DrawType type) NANOEM_DECL_NOEXCEPT;
    static const char *toString(sg_action action) NANOEM_DECL_NOEXCEPT;
};

const char *
EnumStringifyUtils::toString(IEffect::ScriptOrderType order) NANOEM_DECL_NOEXCEPT
{
    switch (order) {
    case IEffect::kScriptOrderTypeDependsOnScriptExternal:
        return "DependsOnScriptExternal";
    case IEffect::kScriptOrderTypePostProcess:
        return "PostProcess";
    case IEffect::kScriptOrderTypeStandard:
        return "Standard";
    case IEffect::kScriptOrderTypePreProcess:
        return "PreProcess";
    default:
        return "(Unknown)";
    }
}

const char *
EnumStringifyUtils::toString(IDrawable::DrawType type) NANOEM_DECL_NOEXCEPT
{
    switch (type) {
    case IDrawable::kDrawTypeColor:
        return "Color";
    case IDrawable::kDrawTypeEdge:
        return "Edge";
    case IDrawable::kDrawTypeGroundShadow:
        return "GroundShadow";
    case IDrawable::kDrawTypeShadowMap:
        return "ShadowMap";
    default:
        return "(Unknown)";
    }
}

const char *
EnumStringifyUtils::toString(sg_action action) NANOEM_DECL_NOEXCEPT
{
    switch (action) {
    case SG_ACTION_CLEAR:
        return "Clear";
    case SG_ACTION_LOAD:
        return "Load";
    case SG_ACTION_DONTCARE:
        return "DontCare";
    default:
        return "(Unknown)";
    }
}

static int
compareOffscreenRenderTargetOption(const void *left, const void *right)
{
    effect::OffscreenRenderTargetOption *const *lv = static_cast<effect::OffscreenRenderTargetOption *const *>(left);
    effect::OffscreenRenderTargetOption *const *rv = static_cast<effect::OffscreenRenderTargetOption *const *>(right);
    const effect::OffscreenRenderTargetOption *lvalue = *lv, *rvalue = *rv;
    int result = 0;
    if (lvalue->m_sharedImageReferenceCount != rvalue->m_sharedImageReferenceCount) {
        result = rvalue->m_sharedImageReferenceCount - lvalue->m_sharedImageReferenceCount;
    }
    else {
        result = StringUtils::compare(lvalue->m_name.c_str(), rvalue->m_name.c_str());
    }
    return result;
}

static const Vector2UI16 kDefaultViewportImageSize = Vector2UI16(640, 360);
static const nanoem_u32_t kTimeBasedAudioSourceDefaultSampleRate = 1440;

} /* namespace anonymous */

#include "sha256.h"

const char *const Project::kRedoLogFileExtension = "redo";
const char *const Project::kArchivedNativeFormatFileExtension = "nma";
const char *const Project::kFileSystemBasedNativeFormatFileExtension = "nmm";
const char *const Project::kPolygonMovieMakerFileExtension = "pmm";
const char *const Project::kViewportPrimaryName = "@nanoem/Viewport/Primary";
const char *const Project::kViewportSecondaryName = "@nanoem/Viewport/Secondary";
const nanoem_frame_index_t Project::kMinimumBaseDuration = 300u;
const nanoem_frame_index_t Project::kMaximumBaseDuration = (1u << 31) - 1; // INT32_MAX
const nanoem_f32_t Project::kDefaultCircleRadiusSize = 7.5f;

struct Project::DrawQueue {
    enum CommandType {
        kCommandTypeFirstEnum,
        kCommandTypeSetPassAction = kCommandTypeFirstEnum,
        kCommandTypeApplyPipelineBinding,
        kCommandTypeApplyViewport,
        kCommandTypeApplyScissorRect,
        kCommandTypeApplyUniformBlockVertex,
        kCommandTypeApplyUniformBlockFragment,
        kCommandTypeDraw,
        kCommandTypeCallback,
        kCommandTypeMaxEnum,
    };
    struct Command {
        CommandType m_type;
        union {
            sg_pass_action m_action;
            struct {
                sg_pipeline m_pipeline;
                sg_bindings m_bindings;
            } m_pb;
            struct {
                int m_x;
                int m_y;
                int m_width;
                int m_height;
            } m_rect;
            struct {
                nanoem_u8_t *m_data;
                nanoem_rsize_t m_size;
            } m_ub;
            struct {
                int m_offset;
                int m_count;
            } m_draw;
            struct {
                sg::PassBlock::Callback m_func;
                void *m_opaque;
            } m_callback;
        } u;
    };
    typedef tinystl::vector<Command, TinySTLAllocator> CommandBuffer;
    struct PassCommandBuffer {
        CommandBuffer *m_items;
        sg_pass m_handle;
        bool m_batch;
        PassCommandBuffer()
            : m_items(nullptr)
            , m_batch(false)
        {
            m_handle = { SG_INVALID_ID };
        }
    };
    typedef tinystl::vector<PassCommandBuffer, TinySTLAllocator> PassCommandBufferList;

    static void applyPipelineBindings(CommandBuffer &buffer, sg_pipeline pipeline, const sg_bindings &bindings);
    static void applyViewport(CommandBuffer &buffer, int x, int y, int width, int height);
    static void applyScissorRect(CommandBuffer &buffer, int x, int y, int width, int height);
    static void applyUniformBlock(CommandBuffer &buffer, const void *data, nanoem_rsize_t size);
    static void applyUniformBlock(CommandBuffer &buffer, sg_shader_stage stage, const void *data, nanoem_rsize_t size);
    static void draw(CommandBuffer &buffer, int offset, int count);
    static void registerCallback(CommandBuffer &buffer, sg::PassBlock::Callback callback, void *userData);
    static void drawPass(const PassCommandBuffer *pass, Project *project, bx::HashMurmur2A &hasher);

    DrawQueue();
    ~DrawQueue() NANOEM_DECL_NOEXCEPT;

    size_t size() const NANOEM_DECL_NOEXCEPT;
    void flush(Project *project);

    Project *m_project;
    PassCommandBufferList m_commandBuffers;
    int m_counts;
};

void
Project::DrawQueue::applyPipelineBindings(
    DrawQueue::CommandBuffer &buffer, sg_pipeline pipeline, const sg_bindings &bindings)
{
    Command item;
    item.m_type = kCommandTypeApplyPipelineBinding;
    item.u.m_pb.m_bindings = bindings;
    item.u.m_pb.m_pipeline = pipeline;
    buffer.push_back(item);
}

void
Project::DrawQueue::applyViewport(DrawQueue::CommandBuffer &buffer, int x, int y, int width, int height)
{
    Command item;
    item.m_type = kCommandTypeApplyViewport;
    item.u.m_rect.m_x = x;
    item.u.m_rect.m_y = y;
    item.u.m_rect.m_width = width;
    item.u.m_rect.m_height = height;
    buffer.push_back(item);
}

void
Project::DrawQueue::applyScissorRect(DrawQueue::CommandBuffer &buffer, int x, int y, int width, int height)
{
    Command item;
    item.m_type = kCommandTypeApplyScissorRect;
    item.u.m_rect.m_x = x;
    item.u.m_rect.m_y = y;
    item.u.m_rect.m_width = width;
    item.u.m_rect.m_height = height;
    buffer.push_back(item);
}

void
Project::DrawQueue::applyUniformBlock(DrawQueue::CommandBuffer &buffer, const void *data, nanoem_rsize_t size)
{
    applyUniformBlock(buffer, SG_SHADERSTAGE_VS, data, size);
    applyUniformBlock(buffer, SG_SHADERSTAGE_FS, data, size);
}

void
Project::DrawQueue::applyUniformBlock(
    DrawQueue::CommandBuffer &buffer, sg_shader_stage stage, const void *data, nanoem_rsize_t size)
{
    Command item;
    switch (stage) {
    case SG_SHADERSTAGE_FS: {
        item.m_type = kCommandTypeApplyUniformBlockFragment;
        break;
    }
    case SG_SHADERSTAGE_VS: {
        item.m_type = kCommandTypeApplyUniformBlockVertex;
        break;
    }
    default:
        break;
    }
    item.u.m_ub.m_data = new nanoem_u8_t[size];
    memcpy(item.u.m_ub.m_data, data, size);
    item.u.m_ub.m_size = size;
    buffer.push_back(item);
}

void
Project::DrawQueue::draw(DrawQueue::CommandBuffer &buffer, int offset, int count)
{
    Command item;
    item.m_type = kCommandTypeDraw;
    item.u.m_draw.m_offset = offset;
    item.u.m_draw.m_count = count;
    buffer.push_back(item);
}

void
Project::DrawQueue::registerCallback(CommandBuffer &buffer, sg::PassBlock::Callback callback, void *userData)
{
    if (callback) {
        Command item;
        item.m_type = kCommandTypeCallback;
        item.u.m_callback.m_func = callback;
        item.u.m_callback.m_opaque = userData;
        buffer.push_back(item);
    }
}

void
Project::DrawQueue::drawPass(const DrawQueue::PassCommandBuffer *pass, Project *project, bx::HashMurmur2A &hasher)
{
    BX_UNUSED_1(project);
    SG_PUSH_GROUPF(
        "Project::DrawQueue::drawPass(id=%d, name=%s)", pass->m_handle.id, project->findRenderPassName(pass->m_handle));
    sg_pass_action pa(pass->m_items->data()->u.m_action);
    for (int i = 0; i < SG_MAX_COLOR_ATTACHMENTS; i++) {
        sg_color_attachment_action &ca = pa.colors[i];
        if (ca.action == _SG_ACTION_DEFAULT) {
            ca.action = SG_ACTION_LOAD;
        }
    }
    if (pa.depth.action == _SG_ACTION_DEFAULT) {
        pa.depth.action = SG_ACTION_LOAD;
    }
    if (pa.stencil.action == _SG_ACTION_DEFAULT) {
        pa.stencil.action = SG_ACTION_LOAD;
    }
    sg::begin_pass(pass->m_handle, &pa);
    SG_INSERT_MARKERF("Project::DrawQueue::beginPass(color=%s, depth=%s, stencil=%s, batch=%s)",
        EnumStringifyUtils::toString(pa.colors[0].action), EnumStringifyUtils::toString(pa.depth.action),
        EnumStringifyUtils::toString(pa.stencil.action), pass->m_batch ? "true" : "false");
    nanoem_u32_t lastDrawPassHash = 0;
    bool pipelineApplied = false;
    hasher.begin();
    for (CommandBuffer::const_iterator it2 = pass->m_items->begin() + 1, end2 = pass->m_items->end(); it2 != end2;
         ++it2) {
        const Command &item = *it2;
        switch (item.m_type) {
        case kCommandTypeApplyPipelineBinding: {
            SG_INSERT_MARKERF("Project::DrawQueue::drawPass(index=%d, type=kCommandTypeApplyPipelineBinding, "
                              "pipeline=%d, name=%s)",
                it2 - pass->m_items->begin(), item.u.m_pb.m_pipeline.id,
                project->findRenderPipelineName(item.u.m_pb.m_pipeline));
#if defined(NANOEM_ENABLE_DEBUG_LABEL)
            char buffer[Inline::kMarkerStringLength];
            StringUtils::format(buffer, sizeof(buffer),
                "Project::DrawQueue::drawPass(pass=%s, index=%d, type=kCommandTypeApplyPipelineBinding, name=%s)",
                project->findRenderPassName(pass->m_handle), it2 - pass->m_items->begin(),
                project->findRenderPipelineName(item.u.m_pb.m_pipeline));
#endif /* NANOEM_ENABLE_DEBUG_LABEL */
            const sg_pipeline pipeline = item.u.m_pb.m_pipeline;
            if (sg::query_pipeline_state(pipeline) == SG_RESOURCESTATE_VALID) {
                sg::apply_pipeline(pipeline);
                sg::apply_bindings(&item.u.m_pb.m_bindings);
                hasher.add(item.u.m_pb);
                pipelineApplied = true;
            }
            break;
        }
        case kCommandTypeApplyViewport: {
            SG_INSERT_MARKERF("Project::DrawQueue::drawPass(index=%d, type=kCommandTypeApplyViewport, x=%d, y=%d, "
                              "width=%d, height=%d)",
                it2 - pass->m_items->begin(), item.u.m_rect.m_x, item.u.m_rect.m_y, item.u.m_rect.m_width,
                item.u.m_rect.m_height);
            sg::apply_viewport(
                item.u.m_rect.m_x, item.u.m_rect.m_y, item.u.m_rect.m_width, item.u.m_rect.m_height, true);
            hasher.add(item.u.m_rect);
            break;
        }
        case kCommandTypeApplyScissorRect: {
            SG_INSERT_MARKERF("Project::DrawQueue::drawPass(index=%d, type=kCommandTypeApplyScissorRect, x=%d, y=%d, "
                              "width=%d, height=%d)",
                it2 - pass->m_items->begin(), item.u.m_rect.m_x, item.u.m_rect.m_y, item.u.m_rect.m_width,
                item.u.m_rect.m_height);
            sg::apply_scissor_rect(
                item.u.m_rect.m_x, item.u.m_rect.m_y, item.u.m_rect.m_width, item.u.m_rect.m_height, true);
            hasher.add(item.u.m_rect);
            break;
        }
        case kCommandTypeApplyUniformBlockVertex: {
            SG_INSERT_MARKERF(
                "Project::DrawQueue::drawPass(index=%d, type=kCommandTypeApplyUniformBlockVertex, size=%d)",
                it2 - pass->m_items->begin(), item.u.m_ub.m_size);
            nanoem_u8_t *data = item.u.m_ub.m_data;
            int size = Inline::saturateInt32(item.u.m_ub.m_size);
            sg::apply_uniforms(SG_SHADERSTAGE_VS, 0, data, size);
            hasher.add(data, size);
            delete[] data;
            break;
        }
        case kCommandTypeApplyUniformBlockFragment: {
            SG_INSERT_MARKERF(
                "Project::DrawQueue::drawPass(index=%d, type=kCommandTypeApplyUniformBlockFragment, size=%d)",
                it2 - pass->m_items->begin(), item.u.m_ub.m_size);
            nanoem_u8_t *data = item.u.m_ub.m_data;
            int size = Inline::saturateInt32(item.u.m_ub.m_size);
            sg::apply_uniforms(SG_SHADERSTAGE_FS, 0, data, size);
            hasher.add(data, size);
            delete[] data;
            break;
        }
        case kCommandTypeDraw: {
            SG_INSERT_MARKERF("Project::DrawQueue::drawPass(index=%d, type=kCommandTypeDraw, offset=%d, count=%d)",
                it2 - pass->m_items->begin(), item.u.m_draw.m_offset, item.u.m_draw.m_count);
            hasher.add(item.u.m_draw);
            nanoem_u32_t drawPassHash = hasher.end();
            hasher.begin();
            if (pipelineApplied && lastDrawPassHash != drawPassHash) {
                sg::draw(item.u.m_draw.m_offset, item.u.m_draw.m_count, 1);
                lastDrawPassHash = drawPassHash;
                pipelineApplied = false;
            }
            break;
        }
        case kCommandTypeCallback: {
            SG_INSERT_MARKERF(
                "Project::DrawQueue::drawPass(index=%d, type=kCommandTypeCallback)", it2 - pass->m_items->begin());
            item.u.m_callback.m_func(pass->m_handle, item.u.m_callback.m_opaque);
            break;
        }
        default:
            break;
        }
    }
    sg::end_pass();
    SG_INSERT_MARKER("Project::DrawQueue::endPass()");
    SG_POP_GROUP();
}

Project::DrawQueue::DrawQueue()
    : m_counts(0)
{
}

Project::DrawQueue::~DrawQueue() NANOEM_DECL_NOEXCEPT
{
    for (PassCommandBufferList::const_iterator it = m_commandBuffers.begin(), end = m_commandBuffers.end(); it != end;
         ++it) {
        nanoem_delete(it->m_items);
    }
    m_commandBuffers.clear();
}

size_t
Project::DrawQueue::size() const NANOEM_DECL_NOEXCEPT
{
    return m_commandBuffers.size();
}

void
Project::DrawQueue::flush(Project *project)
{
    bx::HashMurmur2A hasher;
    for (PassCommandBufferList::const_iterator it = m_commandBuffers.begin(), end = m_commandBuffers.end(); it != end;
         ++it) {
        const sg_pass pass = it->m_handle;
        if (sg::query_pass_state(pass) == SG_RESOURCESTATE_VALID) {
            drawPass(it, project, hasher);
        }
        else {
            SG_INSERT_MARKERF("[WARN] The pass \"%s\" (%d) was skipped", project->findRenderPassName(pass), pass.id);
        }
        nanoem_delete(it->m_items);
    }
    m_commandBuffers.clear();
}

struct Project::BatchDrawQueue : sg::PassBlock::IDrawQueue {
    typedef tinystl::unordered_map<nanoem_u32_t, DrawQueue::CommandBuffer *, TinySTLAllocator> CommandBufferMap;

    BatchDrawQueue(DrawQueue *drawQueue);
    ~BatchDrawQueue();

    void beginPass(sg_pass pass, const sg_pass_action &action) NANOEM_DECL_OVERRIDE;
    void endPass() NANOEM_DECL_OVERRIDE;
    void applyPipelineBindings(sg_pipeline pipeline, const sg_bindings &bindings) NANOEM_DECL_OVERRIDE;
    void applyViewport(int x, int y, int width, int height) NANOEM_DECL_OVERRIDE;
    void applyScissorRect(int x, int y, int width, int height) NANOEM_DECL_OVERRIDE;
    void applyUniformBlock(const void *data, nanoem_rsize_t size) NANOEM_DECL_OVERRIDE;
    void applyUniformBlock(sg_shader_stage stage, const void *data, nanoem_rsize_t size) NANOEM_DECL_OVERRIDE;
    void draw(int offset, int count) NANOEM_DECL_OVERRIDE;
    void registerCallback(sg::PassBlock::Callback callback, void *userData) NANOEM_DECL_OVERRIDE;
    void clear();

    DrawQueue *m_drawQueue;
    sg_pass m_pass;
    CommandBufferMap m_batch;
};

Project::BatchDrawQueue::BatchDrawQueue(DrawQueue *drawQueue)
    : m_drawQueue(drawQueue)
{
}

Project::BatchDrawQueue::~BatchDrawQueue()
{
}

void
Project::BatchDrawQueue::beginPass(sg_pass pass, const sg_pass_action &action)
{
    CommandBufferMap::const_iterator it = m_batch.find(pass.id);
    const bool found = it != m_batch.end();
    if (!found) {
        SG_INSERT_MARKERF("Project::BatchDrawQueue::beginPass(handle=%d, name=%s, color=%s, depth=%s, stencil=%s)",
            pass, m_drawQueue->m_project->findRenderPassName(pass),
            EnumStringifyUtils::toString(action.colors[0].action), EnumStringifyUtils::toString(action.depth.action),
            EnumStringifyUtils::toString(action.stencil.action));
        DrawQueue::PassCommandBuffer pb;
        DrawQueue::Command item;
        item.m_type = DrawQueue::kCommandTypeSetPassAction;
        item.u.m_action = action;
        pb.m_handle = pass;
        pb.m_items = nanoem_new(DrawQueue::CommandBuffer);
        pb.m_items->push_back(item);
        pb.m_batch = true;
        m_batch[pass.id] = pb.m_items;
        m_drawQueue->m_commandBuffers.push_back(pb);
    }
    m_pass = pass;
}

void
Project::BatchDrawQueue::endPass()
{
    m_pass = { SG_INVALID_ID };
}

void
Project::BatchDrawQueue::applyPipelineBindings(sg_pipeline pipeline, const sg_bindings &bindings)
{
    CommandBufferMap::iterator it = m_batch.find(m_pass.id);
    if (it != m_batch.end()) {
        DrawQueue::applyPipelineBindings(*it->second, pipeline, bindings);
    }
}

void
Project::BatchDrawQueue::applyViewport(int x, int y, int width, int height)
{
    CommandBufferMap::iterator it = m_batch.find(m_pass.id);
    if (it != m_batch.end()) {
        DrawQueue::applyViewport(*it->second, x, y, width, height);
    }
}

void
Project::BatchDrawQueue::applyScissorRect(int x, int y, int width, int height)
{
    CommandBufferMap::iterator it = m_batch.find(m_pass.id);
    if (it != m_batch.end()) {
        DrawQueue::applyScissorRect(*it->second, x, y, width, height);
    }
}

void
Project::BatchDrawQueue::applyUniformBlock(const void *data, nanoem_rsize_t size)
{
    CommandBufferMap::iterator it = m_batch.find(m_pass.id);
    if (it != m_batch.end()) {
        DrawQueue::applyUniformBlock(*it->second, SG_SHADERSTAGE_VS, data, size);
        DrawQueue::applyUniformBlock(*it->second, SG_SHADERSTAGE_FS, data, size);
    }
}

void
Project::BatchDrawQueue::applyUniformBlock(sg_shader_stage stage, const void *data, nanoem_rsize_t size)
{
    CommandBufferMap::iterator it = m_batch.find(m_pass.id);
    if (it != m_batch.end()) {
        DrawQueue::applyUniformBlock(*it->second, stage, data, size);
    }
}

void
Project::BatchDrawQueue::draw(int offset, int count)
{
    CommandBufferMap::iterator it = m_batch.find(m_pass.id);
    if (it != m_batch.end()) {
        SG_INSERT_MARKERF("Project::BatchDrawQueue::draw(offset=%d, count=%d)", offset, count);
        DrawQueue::draw(*it->second, offset, count);
    }
}

void
Project::BatchDrawQueue::registerCallback(sg::PassBlock::Callback callback, void *userData)
{
    CommandBufferMap::iterator it = m_batch.find(m_pass.id);
    if (it != m_batch.end()) {
        DrawQueue::registerCallback(*it->second, callback, userData);
    }
}

void
Project::BatchDrawQueue::clear()
{
    m_batch.clear();
}

struct Project::SerialDrawQueue : sg::PassBlock::IDrawQueue {
    SerialDrawQueue(DrawQueue *drawQueue);
    ~SerialDrawQueue();

    void beginPass(sg_pass pass, const sg_pass_action &action) NANOEM_DECL_OVERRIDE;
    void endPass() NANOEM_DECL_OVERRIDE;
    void applyPipelineBindings(sg_pipeline pipeline, const sg_bindings &bindings) NANOEM_DECL_OVERRIDE;
    void applyViewport(int x, int y, int width, int height) NANOEM_DECL_OVERRIDE;
    void applyScissorRect(int x, int y, int width, int height) NANOEM_DECL_OVERRIDE;
    void applyUniformBlock(const void *data, nanoem_rsize_t size) NANOEM_DECL_OVERRIDE;
    void applyUniformBlock(sg_shader_stage stage, const void *data, nanoem_rsize_t size) NANOEM_DECL_OVERRIDE;
    void draw(int offset, int count) NANOEM_DECL_OVERRIDE;
    void registerCallback(sg::PassBlock::Callback callback, void *userData) NANOEM_DECL_OVERRIDE;

    DrawQueue *m_drawQueue;
};

Project::SerialDrawQueue::SerialDrawQueue(DrawQueue *drawQueue)
    : m_drawQueue(drawQueue)
{
}

Project::SerialDrawQueue::~SerialDrawQueue()
{
}

void
Project::SerialDrawQueue::beginPass(sg_pass pass, const sg_pass_action &action)
{
    DrawQueue::PassCommandBufferList &buffers = m_drawQueue->m_commandBuffers;
    bool mergeable = false;
    if (!buffers.empty()) {
        DrawQueue::PassCommandBuffer &buffer = buffers.back();
        DrawQueue::Command &command = buffer.m_items->front();
        sg_pass_action &dstAction = command.u.m_action;
        mergeable = buffer.m_handle.id == pass.id && !buffer.m_batch;
        if (mergeable) {
            for (int i = 0; i < SG_MAX_COLOR_ATTACHMENTS; i++) {
                const sg_color_attachment_action &sca = action.colors[i];
                sg_color_attachment_action &dca = dstAction.colors[i];
                if (dca.action == _SG_ACTION_DEFAULT && sca.action != _SG_ACTION_DEFAULT) {
                    SG_INSERT_MARKERF("Project::SerialDrawQueue::setColorAttachmentAction(index=%d, action=%s, "
                                      "R=%.2f, G=%.2f, B=%.2f, A=%.2f)",
                        i, EnumStringifyUtils::toString(sca.action), sca.value.r, sca.value.g, sca.value.b,
                        sca.value.a);
                    dca = sca;
                    mergeable &= sca.action != SG_ACTION_LOAD;
                }
            }
            if (dstAction.depth.action == _SG_ACTION_DEFAULT && action.depth.action != _SG_ACTION_DEFAULT) {
                SG_INSERT_MARKERF("Project::SerialDrawQueue::setDepthAttachmentAction(action=%s, value=%.2f)",
                    EnumStringifyUtils::toString(action.depth.action), action.depth.value);
                dstAction.depth = action.depth;
                mergeable &= dstAction.depth.action != SG_ACTION_LOAD;
            }
            if (dstAction.stencil.action == _SG_ACTION_DEFAULT && action.stencil.action != _SG_ACTION_DEFAULT) {
                SG_INSERT_MARKERF("Project::SerialDrawQueue::setStencilAttachmentAction(action=%s, value=%d)",
                    EnumStringifyUtils::toString(action.stencil.action), action.stencil.value);
                dstAction.stencil = action.stencil;
                mergeable &= dstAction.stencil.action != SG_ACTION_LOAD;
            }
        }
    }
    if (!mergeable) {
        SG_INSERT_MARKERF(
            "Project::SerialDrawQueue::beginPass(offset=%d, handle=%d, name=%s, color=%s, depth=%s, stencil=%s)",
            Inline::saturateInt32(buffers.size() + 1), pass, m_drawQueue->m_project->findRenderPassName(pass),
            EnumStringifyUtils::toString(action.colors[0].action), EnumStringifyUtils::toString(action.depth.action),
            EnumStringifyUtils::toString(action.stencil.action));
        DrawQueue::PassCommandBuffer buffer;
        DrawQueue::Command item;
        item.m_type = DrawQueue::kCommandTypeSetPassAction;
        item.u.m_action = action;
        buffer.m_handle = pass;
        buffer.m_items = nanoem_new(DrawQueue::CommandBuffer);
        buffer.m_items->push_back(item);
        buffers.push_back(buffer);
    }
}

void
Project::SerialDrawQueue::endPass()
{
}

void
Project::SerialDrawQueue::applyPipelineBindings(sg_pipeline pipeline, const sg_bindings &bindings)
{
    DrawQueue::applyPipelineBindings(*m_drawQueue->m_commandBuffers.back().m_items, pipeline, bindings);
}

void
Project::SerialDrawQueue::applyViewport(int x, int y, int width, int height)
{
    DrawQueue::applyViewport(*m_drawQueue->m_commandBuffers.back().m_items, x, y, width, height);
}

void
Project::SerialDrawQueue::applyScissorRect(int x, int y, int width, int height)
{
    DrawQueue::applyScissorRect(*m_drawQueue->m_commandBuffers.back().m_items, x, y, width, height);
}

void
Project::SerialDrawQueue::applyUniformBlock(const void *data, nanoem_rsize_t size)
{
    DrawQueue::applyUniformBlock(*m_drawQueue->m_commandBuffers.back().m_items, data, size);
}

void
Project::SerialDrawQueue::applyUniformBlock(sg_shader_stage stage, const void *data, nanoem_rsize_t size)
{
    DrawQueue::applyUniformBlock(*m_drawQueue->m_commandBuffers.back().m_items, stage, data, size);
}

void
Project::SerialDrawQueue::draw(int offset, int count)
{
    SG_INSERT_MARKERF("Project::SerialDrawQueue::draw(offset=%d, count=%d)", offset, count);
    DrawQueue::draw(*m_drawQueue->m_commandBuffers.back().m_items, offset, count);
}

void
Project::SerialDrawQueue::registerCallback(sg::PassBlock::Callback callback, void *userData)
{
    DrawQueue::registerCallback(*m_drawQueue->m_commandBuffers.back().m_items, callback, userData);
}

struct Project::SaveState {
    SaveState(Project *project)
        : m_tag(project)
        , m_activeModel(nullptr)
        , m_activeAccessory(nullptr)
        , m_camera(project)
        , m_light(project)
        , m_physicsSimulationMode(PhysicsEngine::kSimulationModeDisable)
        , m_localFrameIndex(0)
        , m_stateFlags(0)
        , m_confirmSeekFlags(0)
        , m_lastPhysicsDebugFlags(0)
        , m_visibleGrid(false)
    {
    }
    ~SaveState() NANOEM_DECL_NOEXCEPT
    {
        m_tag = nullptr;
        m_activeModel = nullptr;
        m_activeAccessory = nullptr;
        m_localFrameIndex = 0;
        m_stateFlags = 0;
        m_confirmSeekFlags = 0;
        m_visibleGrid = false;
    }
    const Project *m_tag;
    Model *m_activeModel;
    Accessory *m_activeAccessory;
    PerspectiveCamera m_camera;
    DirectionalLight m_light;
    PhysicsEngine::SimulationModeType m_physicsSimulationMode;
    nanoem_frame_index_t m_localFrameIndex;
    nanoem_u64_t m_stateFlags;
    nanoem_u64_t m_confirmSeekFlags;
    nanoem_u32_t m_lastPhysicsDebugFlags;
    bool m_visibleGrid;
};

Project::Pass::Pass(Project *project, const char *name)
    : m_project(project)
    , m_name(name)
{
    m_handle = { SG_INVALID_ID };
    m_colorImage = m_depthImage = { SG_INVALID_ID };
}

void
Project::Pass::update(const Vector2UI16 &size)
{
    SG_PUSH_GROUPF("Project::Pass::update(name=%s, width=%d, height=%d)", m_name.c_str(), size.x, size.y);
    sg_image_desc id;
    const sg_pixel_format colorPixelFormat = m_project->viewportPixelFormat();
    const bool enableMSAA = sg::query_features().msaa_render_targets && sg::query_pixelformat(colorPixelFormat).msaa;
    char label[Inline::kMarkerStringLength];
    Inline::clearZeroMemory(id);
    if (Inline::isDebugLabelEnabled()) {
        StringUtils::format(label, sizeof(label), "%s/ColorImage", m_name.c_str());
        id.label = label;
    }
    id.render_target = true;
    id.width = size.x;
    id.height = size.y;
    id.pixel_format = colorPixelFormat;
    id.mag_filter = id.min_filter = SG_FILTER_LINEAR;
    id.wrap_u = id.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
    id.sample_count = enableMSAA ? m_project->sampleCount() : 1;
    sg::destroy_image(m_colorImage);
    m_colorImage = sg::make_image(&id);
    nanoem_assert(sg::query_image_state(m_colorImage) == SG_RESOURCESTATE_VALID, "color image must be valid");
    SG_LABEL_IMAGE(m_colorImage, label);
    id.pixel_format = SG_PIXELFORMAT_DEPTH_STENCIL;
    sg::destroy_image(m_depthImage);
    if (Inline::isDebugLabelEnabled()) {
        StringUtils::format(label, sizeof(label), "%s/DepthImage", m_name.c_str());
        id.label = label;
    }
    m_depthImage = sg::make_image(&id);
    nanoem_assert(sg::query_image_state(m_depthImage) == SG_RESOURCESTATE_VALID, "color image must be valid");
    SG_LABEL_IMAGE(m_depthImage, label);
    sg::destroy_pass(m_handle);
    sg_pass_desc pd;
    getDescription(pd);
    if (Inline::isDebugLabelEnabled()) {
        StringUtils::format(label, sizeof(label), "%s/ViewportPass", m_name.c_str());
        pd.label = label;
    }
    m_handle = sg::make_pass(&pd);
    nanoem_assert(sg::query_pass_state(m_handle) == SG_RESOURCESTATE_VALID, "pass must be valid");
    RenderPassBundle &desc = m_project->m_renderPassBundleMap[m_handle.id];
    m_project->m_hashedRenderPassBundleMap[bx::hash<bx::HashMurmur2A>(pd)] = &desc;
    m_project->setRenderPassName(m_handle, m_name.c_str());
    desc.m_handle = m_handle;
    desc.m_colorImage = m_colorImage;
    desc.m_depthImage = m_depthImage;
    desc.m_desciption = pd;
    PixelFormat &format = desc.m_format;
    format.setColorPixelFormat(colorPixelFormat, 0);
    format.setDepthPixelFormat(SG_PIXELFORMAT_DEPTH_STENCIL);
    format.setNumSamples(id.sample_count);
    format.setNumColorAttachemnts(1);
    SG_POP_GROUP();
}

void
Project::Pass::getDescription(sg_pass_desc &pd) const NANOEM_DECL_NOEXCEPT
{
    Inline::clearZeroMemory(pd);
    pd.color_attachments[0].image = m_colorImage;
    pd.depth_stencil_attachment.image = m_depthImage;
}

void
Project::Pass::destroy()
{
    SG_PUSH_GROUPF("Project::Pass::destroy(name=%s)", m_name.c_str());
    sg::destroy_pass(m_handle);
    m_handle = { SG_INVALID_ID };
    sg::destroy_image(m_colorImage);
    m_colorImage = { SG_INVALID_ID };
    sg::destroy_image(m_depthImage);
    m_depthImage = { SG_INVALID_ID };
    SG_POP_GROUP();
}

Project::FPSUnit::FPSUnit() NANOEM_DECL_NOEXCEPT : m_value(Constants::kHalfBaseFPS),
                                                   m_scaleFactor(1),
                                                   m_invertedValue(1.0f / Constants::kHalfBaseFPSFloat),
                                                   m_invertedScaleFactor(1)
{
}

Project::FPSUnit::~FPSUnit() NANOEM_DECL_NOEXCEPT
{
}

bool
Project::FPSUnit::operator!=(nanoem_u32_t value) const NANOEM_DECL_NOEXCEPT
{
    return m_value != value;
}

void
Project::FPSUnit::operator=(nanoem_u32_t value) NANOEM_DECL_NOEXCEPT
{
    m_value = glm::max(value, Constants::kHalfBaseFPS);
    m_invertedValue = 1.0f / value;
    m_scaleFactor = value / Constants::kHalfBaseFPSFloat;
    m_invertedScaleFactor = 1.0f / m_scaleFactor;
}

Project::RenderPassBundle::RenderPassBundle()
{
    Inline::clearZeroMemory(m_format);
    Inline::clearZeroMemory(m_desciption);
    m_handle = { SG_INVALID_ID };
    m_colorImage = m_depthImage = { SG_INVALID_ID };
}

Project::RenderPassBundle::~RenderPassBundle() NANOEM_DECL_NOEXCEPT
{
}

URI
Project::resolveArchiveURI(const URI &fileURI, const String &filename)
{
    String path;
    URI resolvedURI;
    if (isArchiveURI(fileURI)) {
        const String &basePath = URI::stringByDeletingLastPathComponent(fileURI.fragment());
        if (!basePath.empty()) {
            path.append(basePath.c_str());
        }
        resolvedURI = URI::createFromFilePath(fileURI.absolutePath(), FileUtils::canonicalizePath(path, filename));
    }
    else {
        path.append(fileURI.absolutePathByDeletingLastPathComponent().c_str());
        if (filename.size() >= 3) {
            const char *c = filename.c_str();
            /* remove drive letter for windows */
            if (((c[0] >= 'a' && c[0] <= 'z') || (c[0] >= 'A' && c[0] <= 'Z')) && c[1] == ':' && c[2] == '/') {
                path.append(c + 3);
            }
        }
        resolvedURI = URI::createFromFilePath(FileUtils::canonicalizePath(path, filename));
    }
    return resolvedURI;
}

String
Project::resolveNameConfliction(const IDrawable *drawable, StringSet &reservedNameSet)
{
    String name(drawable->name());
    if (name.empty()) {
        name = URI::stringByDeletingPathExtension(drawable->fileURI().lastPathComponent());
    }
    return resolveNameConfliction(name, reservedNameSet);
}

String
Project::resolveNameConfliction(const String &name, StringSet &reservedNameSet)
{
    String actualName(name);
    if (reservedNameSet.find(actualName) != reservedNameSet.end()) {
        int index = 1;
        do {
            actualName = String();
            StringUtils::format(actualName, "%s-%d", name.c_str(), index++);
        } while (reservedNameSet.find(actualName) != reservedNameSet.end());
    }
    reservedNameSet.insert(actualName);
    return actualName;
}

int
Project::countColorAttachments(const sg_pass_desc &pd) NANOEM_DECL_NOEXCEPT
{
    int numAttachments = 0;
    for (size_t i = 0; i < SG_MAX_COLOR_ATTACHMENTS; i++) {
        if (sg::is_valid(pd.color_attachments[i].image)) {
            numAttachments++;
        }
        else {
            break;
        }
    }
    return numAttachments;
}

StringList
Project::loadableExtensions()
{
    static const String kLoadableMotionExtensions[] = { kFileSystemBasedNativeFormatFileExtension,
        kPolygonMovieMakerFileExtension, kArchivedNativeFormatFileExtension, String() };
    return StringList(
        &kLoadableMotionExtensions[0], &kLoadableMotionExtensions[BX_COUNTOF(kLoadableMotionExtensions) - 1]);
}

StringSet
Project::loadableExtensionsSet()
{
    return ListUtils::toSetFromList<String>(loadableExtensions());
}

bool
Project::isArchiveURI(const URI &fileURI)
{
    const String pathExtension(fileURI.pathExtension());
    return StringUtils::equals(pathExtension.c_str(), "zip") ||
        StringUtils::equals(pathExtension.c_str(), kArchivedNativeFormatFileExtension);
}

bool
Project::isLoadableExtension(const String &extension)
{
    return FileUtils::isLoadableExtension(extension, loadableExtensionsSet());
}

bool
Project::isLoadableExtension(const URI &fileURI)
{
    return isLoadableExtension(fileURI.pathExtension());
}

void
Project::setAlphaBlendMode(sg_color_state &state) NANOEM_DECL_NOEXCEPT
{
    sg_blend_state &bs = state.blend;
    bs.enabled = true;
    bs.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA;
    bs.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
#if defined(NANOEM_ENABLE_BLENDOP_MINMAX)
    bs.op_alpha = SG_BLENDOP_MAX;
    state.write_mask = SG_COLORMASK_RGBA;
#else
    state.write_mask = SG_COLORMASK_RGB;
#endif
}

void
Project::setAddBlendMode(sg_color_state &state) NANOEM_DECL_NOEXCEPT
{
    sg_blend_state &bs = state.blend;
    bs.enabled = true;
    bs.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA;
    bs.dst_factor_rgb = SG_BLENDFACTOR_ONE;
#if defined(NANOEM_ENABLE_BLENDOP_MINMAX)
    bs.op_alpha = SG_BLENDOP_MAX;
    state.write_mask = SG_COLORMASK_RGBA;
#else
    state.color_write_mask = SG_COLORMASK_RGB;
#endif
}

void
Project::setStandardDepthStencilState(sg_depth_state &ds, sg_stencil_state &ss) NANOEM_DECL_NOEXCEPT
{
    ds.write_enabled = true;
    ds.compare = SG_COMPAREFUNC_LESS_EQUAL;
    ss.enabled = true;
    ss.ref = 0x1;
    ss.read_mask = ss.write_mask = 0xff;
    ss.front.pass_op = ss.back.pass_op = SG_STENCILOP_REPLACE;
}

void
Project::setShadowDepthStencilState(sg_depth_state &ds, sg_stencil_state &ss) NANOEM_DECL_NOEXCEPT
{
    ds.write_enabled = true;
    ds.compare = SG_COMPAREFUNC_LESS;
    ss.enabled = true;
    ss.ref = 0x2;
    ss.front.compare = ss.back.compare = SG_COMPAREFUNC_GREATER;
    ss.front.pass_op = ss.back.pass_op = SG_STENCILOP_REPLACE;
}

Project::Project(const Injector &injector)
    : m_unicodeStringFactoryRepository(injector.m_unicodeStringFactoryRepository)
    , m_applicationConfiguration(injector.m_applicationConfiguration)
    , m_backgroundVideoRenderer(injector.m_backgroundVideoRenderer)
    , m_confirmer(injector.m_confirmerPtr)
    , m_fileManager(injector.m_fileManagerPtr)
    , m_eventPublisher(injector.m_eventPublisherPtr)
    , m_primitive2D(injector.m_primitive2DPtr)
    , m_rendererCapability(injector.m_rendererCapability)
    , m_sharedCancelPublisherRepository(injector.m_sharedCancelPublisherRepositoryPtr)
    , m_sharedDebugCaptureRepository(injector.m_sharedDebugCaptureRepository)
    , m_sharedResourceRepository(injector.m_sharedResourceRepositoryPtr)
    , m_translator(injector.m_translatorPtr)
    , m_sharedImageLoader(nullptr)
    , m_activeModelPairPtr(nullptr, nullptr)
    , m_activeAccessoryPtr(nullptr)
    , m_audioPlayer(injector.m_audioPlayer)
    , m_skinDeformerFactory(injector.m_skinDeformerFactory)
    , m_physicsEngine(nullptr)
    , m_camera(nullptr)
    , m_light(nullptr)
    , m_cameraMotionPtr(nullptr)
    , m_lightMotionPtr(nullptr)
    , m_selfShadowMotionPtr(nullptr)
    , m_shadowCamera(nullptr)
    , m_undoStack(nullptr)
    , m_selectedTrack(nullptr)
    , m_lastSaveState(nullptr)
    , m_drawQueue(nullptr)
    , m_batchDrawQueue(nullptr)
    , m_serialDrawQueue(nullptr)
    , m_offscreenRenderPassScope(nullptr)
    , m_viewportPassBlitter(nullptr)
    , m_renderPassBlitter(nullptr)
    , m_sharedImageBlitter(nullptr)
    , m_renderPassCleaner(nullptr)
    , m_sharedDebugDrawer(nullptr)
    , m_viewportPixelFormat(injector.m_pixelFormat, injector.m_pixelFormat)
    , m_drawType(IDrawable::kDrawTypeColor)
    , m_editingMode(kEditingModeNone)
    , m_filePathMode(kFilePathModeRelative)
    , m_baseDuration(kMinimumBaseDuration)
    , m_language(m_translator->language())
    , m_uniformViewportLayoutRect(Vector4UI16(0), Vector4UI16(0))
    , m_uniformViewportImageSize(kDefaultViewportImageSize, kDefaultViewportImageSize)
    , m_backgroundVideoRect(0)
    , m_boneSelectionRect(0)
    , m_logicalScaleMovingCursorPosition(0)
    , m_scrollDelta(0)
    , m_windowSize(injector.m_windowSize)
    , m_viewportImageSize(kDefaultViewportImageSize)
    , m_viewportPadding(0)
    , m_viewportBackgroundColor(1.0f)
    , m_objectHandleAllocator(nullptr)
    , m_viewportPrimaryPass(this, kViewportPrimaryName)
    , m_viewportSecondaryPass(this, kViewportSecondaryName)
    , m_context2DPass(this, "@nanoem/Context2D")
    , m_editingFPS(0)
    , m_boneInterpolationType(NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM)
    , m_cameraInterpolationType(NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM)
    , m_transformPerformedAt(Motion::kMaxFrameIndex, 0)
    , m_indicesOfMaterialToAttachEffect(bx::kInvalidHandle, ModelMaterialIndexSet())
    , m_windowDevicePixelRatio(injector.m_windowDevicePixelRatio, injector.m_windowDevicePixelRatio)
    , m_viewportDevicePixelRatio(injector.m_viewportDevicePixelRatio, injector.m_viewportDevicePixelRatio)
    , m_uptime(0.0, 0.0f)
    , m_localFrameIndex(0, 0)
    , m_timeStepFactor(1.0f)
    , m_backgroundVideoScaleFactor(1.0f)
    , m_circleRadius(kDefaultCircleRadiusSize)
    , m_sampleLevel(0, 0)
    , m_stateFlags(kPrivateStateInitialValue)
    , m_confirmSeekFlags(0)
    , m_lastPhysicsDebugFlags(0)
    , m_coordinationSystem(GLM_LEFT_HANDED)
    , m_actualFPS(0)
    , m_actionSequence(0)
    , m_active(false)
{
    const bool topLeft = sg::query_features().origin_top_left;
    Inline::clearZeroMemory(m_logicalScaleCursorPositions);
    m_fallbackImage = { SG_INVALID_ID };
    m_backgroundImage.first = { SG_INVALID_ID };
    m_currentRenderPass = m_currentOffscreenRenderPass = m_lastDrawnRenderPass = m_originOffscreenRenderPass =
        m_scriptExternalRenderPass = { SG_INVALID_ID };
    m_camera = nanoem_new(PerspectiveCamera(this));
    m_light = nanoem_new(DirectionalLight(this));
    m_grid = nanoem_new(Grid(this));
    m_physicsEngine = nanoem_new(PhysicsEngine);
    m_shadowCamera = nanoem_new(ShadowCamera(this));
    m_renderPassBlitter = nanoem_new(internal::BlitPass(this, !topLeft));
    m_viewportPassBlitter = nanoem_new(internal::BlitPass(this, false));
    m_renderPassCleaner = nanoem_new(internal::ClearPass(this));
    m_drawQueue = nanoem_new(DrawQueue);
    m_drawQueue->m_project = this;
    m_batchDrawQueue = nanoem_new(BatchDrawQueue(m_drawQueue));
    m_serialDrawQueue = nanoem_new(SerialDrawQueue(m_drawQueue));
    m_undoStack = undoStackCreateWithSoftLimit(glm::clamp(injector.m_preferredUndoCount, 64, undoStackGetHardLimit()));
    nanoem_assert(m_audioPlayer, "must not be nullptr");
    nanoem_assert(m_backgroundVideoRenderer, "must not be nullptr");
    nanoem_assert(m_physicsEngine, "must not be nullptr");
    setBezierCurveAdjustmentEnabled(true);
    setPreferredMotionFPS(60, false);
    m_physicsEngine->initialize(injector.m_dllPath);
}

Project::~Project() NANOEM_DECL_NOEXCEPT
{
    setActiveAccessory(nullptr);
    setActiveModel(nullptr);
    m_cameraMotionPtr = nullptr;
    m_lightMotionPtr = nullptr;
    m_selfShadowMotionPtr = nullptr;
    for (ModelList::const_iterator it = m_allModelPtrs.begin(), end = m_allModelPtrs.end(); it != end; ++it) {
        destroyModel(*it);
    }
    m_allModelPtrs.clear();
    for (AccessoryList::const_iterator it = m_allAccessoryPtrs.begin(), end = m_allAccessoryPtrs.end(); it != end;
         ++it) {
        destroyAccessory(*it);
    }
    m_allAccessoryPtrs.clear();
    for (MotionList::const_iterator it = m_allMotions.begin(), end = m_allMotions.end(); it != end; ++it) {
        destroyMotion(*it);
    }
    m_allMotions.clear();
    for (TrackList::const_iterator it = m_allTracks.begin(), end = m_allTracks.end(); it != end; ++it) {
        nanoem_delete(*it);
    }
    m_allTracks.clear();
    m_drawable2MotionPtrs.clear();
    undoStackDestroy(m_undoStack);
    m_undoStack = nullptr;
    nanoem_delete_safe(m_audioPlayer);
    nanoem_delete_safe(m_batchDrawQueue);
    nanoem_delete_safe(m_serialDrawQueue);
    nanoem_delete_safe(m_drawQueue);
    nanoem_delete_safe(m_rendererCapability);
    nanoem_delete_safe(m_skinDeformerFactory);
    nanoem_delete_safe(m_backgroundVideoRenderer);
    nanoem_delete_safe(m_grid);
    nanoem_delete_safe(m_shadowCamera);
    nanoem_delete_safe(m_camera);
    nanoem_delete_safe(m_light);
    nanoem_delete_safe(m_physicsEngine);
    nanoem_delete_safe(m_sharedDebugDrawer);
    nanoem_delete_safe(m_sharedImageLoader);
    nanoem_delete_safe(m_renderPassBlitter);
    nanoem_delete_safe(m_sharedImageBlitter);
    nanoem_delete_safe(m_renderPassCleaner);
    nanoem_delete_safe(m_viewportPassBlitter);
}

bool
Project::initialize(Error &error)
{
    SG_PUSH_GROUP("Project::initialize");
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_objectHandleAllocator = bx::createHandleAlloc(g_emapp_allocator, UINT16_MAX);
    /* object handle 0 as reserved */
    m_objectHandleAllocator->alloc();
    m_audioPlayer->initialize(baseDuration(), kTimeBasedAudioSourceDefaultSampleRate, error);
    m_physicsEngine->create(status);
    if (status != NANOEM_STATUS_SUCCESS) {
        const char *reason = Error::convertStatusToMessage(status, m_translator);
        error = Error(reason, status, Error::kDomainTypeNanoem);
    }
    m_camera->reset();
    m_camera->update();
    m_light->reset();
    m_grid->initialize();
    m_grid->setVisible(true);
    m_shadowCamera->initialize();
    const sg_backend backend = sg::query_backend();
    setShadowMapEnabled(backend != SG_BACKEND_GLES3);
    setCameraMotion(createMotion());
    setLightMotion(createMotion());
    setSelfShadowMotion(createMotion());
    setPhysicsSimulationMode(PhysicsEngine::kSimulationModeEnableTracing);
    setGroundShadowEnabled(true);
    createFallbackImage();
    rebuildAllTracks();
    m_camera->setDirty(false);
    m_light->setDirty(false);
    m_shadowCamera->setDirty(false);
    SG_POP_GROUP();
    return !error.hasReason();
}

void
Project::destroy() NANOEM_DECL_NOEXCEPT
{
    SG_PUSH_GROUP("Project::destroy");
    MotionList deletingMotions;
    for (MotionList::const_iterator it = m_allMotions.begin(), end = m_allMotions.end(); it != end; ++it) {
        deletingMotions.push_back(*it);
    }
    ModelList deletingModels;
    for (ModelList::const_iterator it = m_allModelPtrs.begin(), end = m_allModelPtrs.end(); it != end; ++it) {
        deletingModels.push_back(*it);
    }
    AccessoryList deletingAccessories;
    for (AccessoryList::const_iterator it = m_allAccessoryPtrs.begin(), end = m_allAccessoryPtrs.end(); it != end;
         ++it) {
        deletingAccessories.push_back(*it);
    }
    setActiveModel(nullptr);
    undoStackClear(m_undoStack);
    m_grid->destroy();
    m_shadowCamera->destroy();
    m_drawableOrderList.clear();
    m_transformModelOrderList.clear();
    m_allMotions.clear();
    for (ModelList::const_iterator it = m_allModelPtrs.begin(), end = m_allModelPtrs.end(); it != end; ++it) {
        removeModel(*it);
    }
    for (AccessoryList::const_iterator it = m_allAccessoryPtrs.begin(), end = m_allAccessoryPtrs.end(); it != end;
         ++it) {
        removeAccessory(*it);
    }
    for (ModelList::const_iterator it = deletingModels.begin(), end = deletingModels.end(); it != end; ++it) {
        destroyModel(*it);
    }
    for (AccessoryList::const_iterator it = deletingAccessories.begin(), end = deletingAccessories.end(); it != end;
         ++it) {
        destroyAccessory(*it);
    }
    for (MotionList::const_iterator it = deletingMotions.begin(), end = deletingMotions.end(); it != end; ++it) {
        destroyMotion(*it);
    }
    EffectList deletingEffects;
    for (LoadedEffectSet::const_iterator it = m_loadedEffectSet.begin(), end = m_loadedEffectSet.end(); it != end;
         ++it) {
        deletingEffects.push_back(*it);
    }
    m_loadedEffectSet.clear();
    for (EffectList::const_iterator it = deletingEffects.begin(), end = deletingEffects.end(); it != end; ++it) {
        destroyDetachedEffect(*it);
    }
    m_viewportPrimaryPass.destroy();
    m_viewportSecondaryPass.destroy();
    m_context2DPass.destroy();
    m_renderPassBlitter->destroy();
    m_renderPassCleaner->destroy();
    m_viewportPassBlitter->destroy();
    if (m_sharedImageBlitter) {
        m_sharedImageBlitter->destroy();
    }
    sg::destroy_image(m_fallbackImage);
    sg::destroy_image(m_backgroundImage.first);
    m_objectHandleAllocator->free(0);
    bx::destroyHandleAlloc(g_emapp_allocator, m_objectHandleAllocator);
    m_objectHandleAllocator = nullptr;
    m_physicsEngine->destroy();
    m_camera->destroy();
    m_light->destroy();
    m_backgroundVideoRenderer->destroy();
    m_audioPlayer->destroy();
    FileUtils::deleteTransientFile(m_fileURI.second);
    SG_POP_GROUP();
}

void
Project::loadFromJSON(const JSON_Value *value)
{
    internal::project::JSON json(this);
    json.load(value);
}

bool
Project::loadFromBinary(
    const nanoem_u8_t *data, size_t size, BinaryFormatType format, Error &error, IDiagnostics *diagnostics)
{
    bool succeeded = false;
    switch (format) {
    case kBinaryFormatNative: {
        internal::project::Native loader(this);
        succeeded = loader.load(data, size, internal::project::Native::kFileTypeData, error, diagnostics);
        break;
    }
    case kBinaryFormatPMM: {
        internal::project::PMM loader(this);
        succeeded = loader.load(data, size, error, diagnostics);
        break;
    }
    default:
        break;
    }
    return succeeded;
}

bool
Project::loadFromBinary(const ByteArray &bytes, BinaryFormatType format, Error &error, IDiagnostics *diagnostics)
{
    return loadFromBinary(bytes.data(), bytes.size(), format, error, diagnostics);
}

bool
Project::loadFromArchive(ISeekableReader *reader, const URI &fileURI, Error &error)
{
    internal::project::Archive archive(this, fileURI);
    return archive.load(reader, error);
}

void
Project::saveAsJSON(JSON_Value *value)
{
    internal::project::JSON json(this);
    json.save(value);
}

bool
Project::saveAsBinary(ByteArray &bytes, BinaryFormatType format, Error &error)
{
    bool succeeded = false;
    switch (format) {
    case kBinaryFormatNative: {
        internal::project::Native saver(this);
        succeeded = saver.save(bytes, internal::project::Native::kFileTypeData, error);
        break;
    }
    case kBinaryFormatPMM: {
        internal::project::PMM saver(this);
        succeeded = saver.save(bytes, error);
        break;
    }
    default:
        break;
    }
    return succeeded;
}

bool
Project::saveAsBinary(ISeekableWriter *writer, BinaryFormatType format, nanoem_u8_t *checksum, Error &error)
{
    bool succeeded = false;
    ByteArray bytes;
    if (saveAsBinary(bytes, format, error)) {
        int dataSize = Inline::saturateInt32(bytes.size());
        int written = writer->write(bytes.data(), dataSize, error);
        succeeded = written == dataSize && !error.hasReason();
        if (succeeded) {
            SHA256_CTX ctx;
            sha256_init(&ctx);
            sha256_update(&ctx, bytes.data(), bytes.size());
            sha256_final(&ctx, checksum);
        }
    }
    return succeeded;
}

bool
Project::saveAsArchive(ISeekableWriter *writer, Error &error)
{
    internal::project::Archive archive(this, fileURI());
    return archive.save(writer, error);
}

bool
Project::loadEffectFromSource(const URI &baseURI, Effect *effect, URI &sourceURI, Progress &progress, Error &error)
{
    return loadEffectFromSource(baseURI, effect, isCompiledEffectCacheEnabled(), sourceURI, progress, error);
}

bool
Project::loadEffectFromSource(
    const URI &baseURI, Effect *effect, bool enableCache, URI &sourceURI, Progress &progress, Error &error)
{
    nanoem_parameter_assert(!baseURI.isEmpty(), "must NOT be empty");
    nanoem_parameter_assert(effect, "must not be nullptr");
    bool succeeded = false;
    ByteArray output;
    const URI &resolvedURI = Effect::resolveSourceURI(m_fileManager, baseURI);
    if (!resolvedURI.isEmpty()) {
        bool hitCache = enableCache && findSourceEffectCache(resolvedURI, output, error);
        if (hitCache ||
            Effect::compileFromSource(resolvedURI, m_fileManager, isMipmapEnabled(), output, progress, error)) {
            effect->setName(resolvedURI.lastPathComponent());
            if (effect->load(output, progress, error)) {
                sourceURI = resolvedURI;
                effect->setFileURI(resolvedURI);
                succeeded = effect->upload(effect::kAttachmentTypeNone, progress, error);
            }
            if (succeeded && enableCache && !hitCache) {
                setSourceEffectCache(resolvedURI, output, error);
            }
        }
    }
    return succeeded;
}

bool
Project::loadEffectFromBinary(const URI &fileURI, Effect *effect, Progress &progress, Error &error)
{
    nanoem_parameter_assert(!fileURI.isEmpty(), "must NOT be empty");
    nanoem_parameter_assert(effect, "must not be nullptr");
    bool succeeded = false;
    if (FileUtils::exists(fileURI)) {
        FileReaderScope scope(m_translator);
        if (scope.open(fileURI, error)) {
            ByteArray bytes;
            FileUtils::read(scope, bytes, error);
            if (!error.hasReason()) {
                effect->setName(fileURI.lastPathComponent());
                if (effect->load(bytes, progress, error)) {
                    effect->setFileURI(fileURI);
                    succeeded = effect->upload(effect::kAttachmentTypeNone, progress, error);
                }
            }
        }
    }
    return succeeded;
}

const Accessory *
Project::findAccessoryByURI(const URI &fileURI) const NANOEM_DECL_NOEXCEPT
{
    const Accessory *candidateAccessoryPtr = nullptr;
    if (!fileURI.isEmpty()) {
        candidateAccessoryPtr = AccessoryFinder::execute(m_allAccessoryPtrs, AccessoryFinder::findByURI, &fileURI);
    }
    return candidateAccessoryPtr;
}

const Accessory *
Project::findAccessoryByFilename(const String &name) const NANOEM_DECL_NOEXCEPT
{
    const Accessory *candidateAccessoryPtr = nullptr;
    if (!name.empty()) {
        candidateAccessoryPtr = AccessoryFinder::execute(m_allAccessoryPtrs, AccessoryFinder::findByFilename, &name);
    }
    return candidateAccessoryPtr;
}

const Accessory *
Project::findAccessoryByName(const String &name) const NANOEM_DECL_NOEXCEPT
{
    const Accessory *candidateAccessoryPtr = nullptr;
    if (!name.empty()) {
        candidateAccessoryPtr = AccessoryFinder::execute(m_allAccessoryPtrs, AccessoryFinder::findByName, &name);
    }
    return candidateAccessoryPtr;
}

const Accessory *
Project::findAccessoryByHandle(nanoem_u16_t handle) const NANOEM_DECL_NOEXCEPT
{
    AccessoryHandleMap::const_iterator it = m_accessoryHandleMap.find(handle);
    return it != m_accessoryHandleMap.end() ? it->second : nullptr;
}

Accessory *
Project::findAccessoryByURI(const URI &fileURI) NANOEM_DECL_NOEXCEPT
{
    Accessory *candidateAccessoryPtr = nullptr;
    if (!fileURI.isEmpty()) {
        candidateAccessoryPtr = AccessoryFinder::execute(m_allAccessoryPtrs, AccessoryFinder::findByURI, &fileURI);
    }
    return candidateAccessoryPtr;
}

Accessory *
Project::findAccessoryByFilename(const String &name) NANOEM_DECL_NOEXCEPT
{
    Accessory *candidateAccessoryPtr = nullptr;
    if (!name.empty()) {
        candidateAccessoryPtr = AccessoryFinder::execute(m_allAccessoryPtrs, AccessoryFinder::findByFilename, &name);
    }
    return candidateAccessoryPtr;
}

Accessory *
Project::findAccessoryByName(const String &name) NANOEM_DECL_NOEXCEPT
{
    Accessory *candidateAccessoryPtr = nullptr;
    if (!name.empty()) {
        candidateAccessoryPtr = AccessoryFinder::execute(m_allAccessoryPtrs, AccessoryFinder::findByName, &name);
    }
    return candidateAccessoryPtr;
}

Accessory *
Project::findAccessoryByHandle(nanoem_u16_t handle) NANOEM_DECL_NOEXCEPT
{
    AccessoryHandleMap::const_iterator it = m_accessoryHandleMap.find(handle);
    return it != m_accessoryHandleMap.end() ? it->second : nullptr;
}

const Model *
Project::findModelByURI(const URI &fileURI) const NANOEM_DECL_NOEXCEPT
{
    const Model *candidateModelPtr = nullptr;
    if (!fileURI.isEmpty()) {
        candidateModelPtr = ModelFinder::execute(m_transformModelOrderList, ModelFinder::findByURI, &fileURI);
    }
    return candidateModelPtr;
}

const Model *
Project::findModelByFilename(const String &name) const NANOEM_DECL_NOEXCEPT
{
    const Model *candidateModelPtr = nullptr;
    if (!name.empty()) {
        candidateModelPtr = ModelFinder::execute(m_transformModelOrderList, ModelFinder::findByFilename, &name);
    }
    return candidateModelPtr;
}

const Model *
Project::findModelByName(const String &name) const NANOEM_DECL_NOEXCEPT
{
    const Model *candidateModelPtr = nullptr;
    if (!name.empty()) {
        candidateModelPtr = ModelFinder::execute(m_transformModelOrderList, ModelFinder::findByName, &name);
    }
    return candidateModelPtr;
}

const Model *
Project::findModelByHandle(nanoem_u16_t handle) const NANOEM_DECL_NOEXCEPT
{
    ModelHandleMap::const_iterator it = m_modelHandleMap.find(handle);
    return it != m_modelHandleMap.end() ? it->second : nullptr;
}

Model *
Project::findModelByURI(const URI &fileURI) NANOEM_DECL_NOEXCEPT
{
    Model *candidateModelPtr = nullptr;
    if (!fileURI.isEmpty()) {
        candidateModelPtr = ModelFinder::execute(m_transformModelOrderList, ModelFinder::findByURI, &fileURI);
    }
    return candidateModelPtr;
}

Model *
Project::findModelByFilename(const String &name) NANOEM_DECL_NOEXCEPT
{
    Model *candidateModelPtr = nullptr;
    if (!name.empty()) {
        candidateModelPtr = ModelFinder::execute(m_transformModelOrderList, ModelFinder::findByFilename, &name);
    }
    return candidateModelPtr;
}

Model *
Project::findModelByName(const String &name) NANOEM_DECL_NOEXCEPT
{
    Model *candidateModelPtr = nullptr;
    if (!name.empty()) {
        candidateModelPtr = ModelFinder::execute(m_transformModelOrderList, ModelFinder::findByName, &name);
    }
    return candidateModelPtr;
}

Model *
Project::findModelByHandle(nanoem_u16_t handle) NANOEM_DECL_NOEXCEPT
{
    ModelHandleMap::const_iterator it = m_modelHandleMap.find(handle);
    return it != m_modelHandleMap.end() ? it->second : nullptr;
}

const Motion *
Project::findMotionByHandle(const nanoem_u16_t handle) const NANOEM_DECL_NOEXCEPT
{
    MotionHandleMap::const_iterator it = m_motionHandleMap.find(handle);
    return it != m_motionHandleMap.end() ? it->second : nullptr;
}

Motion *
Project::findMotionByHandle(const nanoem_u16_t handle) NANOEM_DECL_NOEXCEPT
{
    MotionHandleMap::const_iterator it = m_motionHandleMap.find(handle);
    return it != m_motionHandleMap.end() ? it->second : nullptr;
}

const Effect *
Project::resolveEffect(const IDrawable *drawable) const NANOEM_DECL_NOEXCEPT
{
    const Effect *actualEffectPtr = nullptr;
    if (drawable) {
        if (const Effect *effect = upcastEffect(drawable->activeEffect())) {
            LoadedEffectSet::const_iterator it = m_loadedEffectSet.find(const_cast<Effect *>(effect));
            actualEffectPtr = it != m_loadedEffectSet.end() ? *it : nullptr;
        }
    }
    return actualEffectPtr;
}

Effect *
Project::resolveEffect(IDrawable *drawable) NANOEM_DECL_NOEXCEPT
{
    Effect *actualEffectPtr = nullptr;
    if (drawable) {
        if (Effect *effect = upcastEffect(drawable->activeEffect())) {
            LoadedEffectSet::const_iterator it = m_loadedEffectSet.find(effect);
            actualEffectPtr = it != m_loadedEffectSet.end() ? *it : nullptr;
        }
    }
    return actualEffectPtr;
}

const nanoem_model_bone_t *
Project::resolveBone(const StringPair &value) const NANOEM_DECL_NOEXCEPT
{
    const nanoem_model_bone_t *found = nullptr;
    if (const Model *model = findModelByName(value.first)) {
        if (const nanoem_model_bone_t *bone = model->findBone(value.second)) {
            found = bone;
        }
    }
    return found;
}

bool
Project::containsMotion(const Motion *value) const NANOEM_DECL_NOEXCEPT
{
    return ListUtils::contains(const_cast<Motion *>(value), m_allMotions);
}

void
Project::newModel(Error &error)
{
    static const nanoem_u8_t kNewModelInJapanese[] = { 0xe6, 0x96, 0xb0, 0xe8, 0xa6, 0x8f, 0xe3, 0x83, 0xa2, 0xe3, 0x83,
        0x87, 0xe3, 0x83, 0xab, 0 };
    const UUID uuid(generateUUID(this));
    const String uuidString(uuid.toString());
    Model::NewModelDescription desc;
    char shortUUID[9], dateTimeBuffer[32];
    StringUtils::copyString(shortUUID, uuidString.c_str(), sizeof(shortUUID));
    StringUtils::formatDateTimeUTC(dateTimeBuffer, sizeof(dateTimeBuffer), "%Y-%m-%dT%H:%M:%SZ");
    StringUtils::format(desc.m_name[NANOEM_LANGUAGE_TYPE_JAPANESE], "%s-%s", kNewModelInJapanese, shortUUID);
    StringUtils::format(desc.m_name[NANOEM_LANGUAGE_TYPE_ENGLISH], "NewModel-%s", shortUUID);
    StringUtils::format(desc.m_comment[NANOEM_LANGUAGE_TYPE_JAPANESE],
        "This model was generated by nanoem\n\nUUID: %s\nDateTime: %s", uuidString.c_str(), dateTimeBuffer);
    desc.m_comment[NANOEM_LANGUAGE_TYPE_ENGLISH] = desc.m_comment[NANOEM_LANGUAGE_TYPE_JAPANESE];
    nanoem_unicode_string_factory_t *factory = unicodeStringFactory();
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ByteArray bytes;
    Model::generateNewModelData(desc, factory, bytes, status);
    if (status == NANOEM_STATUS_SUCCESS) {
        FileWriterScope scope;
        const URI projectFileURI(fileURI());
        if (!projectFileURI.isEmpty()) {
            String newModelPath(fileURI().absolutePathByDeletingLastPathComponent());
            newModelPath.append("/");
            const char *filename = desc.m_name[castLanguage()].c_str();
            newModelPath.append(filename);
            newModelPath.append(".pmx");
            const URI fileURI(URI::createFromFilePath(newModelPath.c_str()));
            if (scope.open(fileURI, error)) {
                FileUtils::write(scope.writer(), bytes, error);
                Model *model = createModel();
                if (model->load(bytes, error)) {
                    model->setupAllBindings();
                    model->upload();
                    model->setFileURI(fileURI);
                    addModel(model);
                    setActiveModel(model);
                    model->writeLoadCommandMessage(error);
                    scope.commit(error);
                }
                else {
                    removeModel(model);
                    destroyModel(model);
                    scope.rollback(error);
                }
            }
        }
        else {
            error = Error(
                translator()->translate("nanoem.error.project.new-model.reason"), "", Error::kDomainTypeApplication);
        }
    }
    else {
        error = Error(Error::convertStatusToMessage(status, translator()), "", Error::kDomainTypeNanoem);
    }
}

void
Project::addModel(Model *model)
{
    nanoem_parameter_assert(model, "must not be nullptr");
    model->clearAllBoneBoundsRigidBodies();
    if (!EnumUtils::isEnabled(kDisableHiddenBoneBoundsRigidBody, m_stateFlags)) {
        model->createAllBoneBoundsRigidBodies();
    }
    m_drawableOrderList.push_back(model);
    m_transformModelOrderList.push_back(model);
    m_allModelPtrs.push_back(model);
    addEffectOrderSet(model);
    eventPublisher()->publishAddModelEvent(model);
    Motion *motion = createMotion();
    undoStackClear(model->undoStack());
    addModelMotion(motion, model);
    applyAllOffscreenRenderTargetEffectsToDrawable(model);
}

void
Project::addAccessory(Accessory *accessory)
{
    nanoem_parameter_assert(accessory, "must not be nullptr");
    m_drawableOrderList.push_back(accessory);
    m_allAccessoryPtrs.push_back(accessory);
    addEffectOrderSet(accessory);
    rebuildAllTracks();
    eventPublisher()->publishAddAccessoryEvent(accessory);
    Motion *motion = createMotion();
    undoStackClear(accessory->undoStack());
    addAccessoryMotion(motion, accessory);
    applyAllOffscreenRenderTargetEffectsToDrawable(accessory);
}

void
Project::convertAccessoryToModel(Accessory *accessory, Error &error)
{
    FileReaderScope scope(translator());
    const URI accessoryFileURI(accessory->fileURI());
    if (scope.open(accessoryFileURI, error)) {
        const String name(URI::lastPathComponent(accessoryFileURI.absolutePath()));
        ByteArray accessoryData, modelData;
        Model::ImportDescription desc(accessoryFileURI);
        desc.m_name[NANOEM_LANGUAGE_TYPE_JAPANESE] = name;
        desc.m_name[NANOEM_LANGUAGE_TYPE_ENGLISH] = name;
        desc.m_transform = Accessory::kInitialWorldMatrix;
        FileUtils::read(scope.reader(), accessoryData, error);
        if (!error.hasReason()) {
            Model *model = createModel();
            if (model->load(accessoryData, desc, error)) {
                String newModelPath(accessoryFileURI.absolutePathByDeletingLastPathComponent()),
                    filenameWithoutExtension(URI::stringByDeletingPathExtension(accessoryFileURI.lastPathComponent()));
                newModelPath.append("/");
                newModelPath.append(filenameWithoutExtension.c_str());
                newModelPath.append(".pmx");
                const URI modelFileURI(URI::createFromFilePath(newModelPath));
                FileWriterScope scope;
                if (model->save(modelData, error) && scope.open(modelFileURI, error)) {
                    FileUtils::write(scope.writer(), modelData, error);
                    model->setupAllBindings();
                    Progress progress(this, model->createAllImages());
                    model->upload();
                    model->loadAllImages(progress, error);
                    addModel(model);
                    setActiveModel(model);
                    model->writeLoadCommandMessage(error);
                    scope.commit(error);
                    model->setVisible(true);
                    removeAccessory(accessory);
                    accessory->writeDeleteCommandMessage(error);
                    destroyAccessory(accessory);
                    progress.complete();
                }
                else {
                    removeModel(model);
                    destroyModel(model);
                    scope.rollback(error);
                }
            }
            else {
                removeModel(model);
                destroyModel(model);
            }
        }
    }
}

void
Project::performModelSkinDeformer(Model *model)
{
    if (m_skinDeformerFactory) {
        SG_PUSH_GROUPF("Project::performModelSkinDeformer(model=%s)", model->nameConstString());
        m_skinDeformerFactory->begin();
        for (int i = 0; i < 2; i++) {
            model->markStagingVertexBufferDirty();
            model->updateStagingVertexBuffer();
        }
        m_skinDeformerFactory->commit();
        SG_POP_GROUP();
    }
}

bool
Project::loadAttachedDrawableEffect(IDrawable *drawable, Progress &progress, Error &error)
{
    return loadAttachedDrawableEffect(drawable, isCompiledEffectCacheEnabled(), progress, error);
}

bool
Project::reloadAllDrawableEffects(Progress &progress, Error &error)
{
    bool result = true;
    for (DrawableList::const_iterator it = m_drawableOrderList.begin(), end = m_drawableOrderList.end(); it != end;
         ++it) {
        result = reloadDrawableEffect(*it, progress, error);
        if (!result) {
            break;
        }
    }
    return result;
}

bool
Project::reloadDrawableEffect(Progress &progress, Error &error)
{
    bool result = false;
    if (Model *model = activeModel()) {
        result = reloadDrawableEffect(model, progress, error);
    }
    else if (Accessory *accessory = activeAccessory()) {
        result = reloadDrawableEffect(accessory, progress, error);
    }
    return result;
}

bool
Project::reloadDrawableEffect(IDrawable *drawable, Progress &progress, Error &error)
{
    bool succeeded = false;
    if (Effect *lastEffect = resolveEffect(drawable)) {
        const bool hasScriptExternal = lastEffect->hasScriptExternal();
        if (hasScriptExternal) {
            ListUtils::removeItem(drawable, m_dependsOnScriptExternal);
        }
        if (loadAttachedDrawableEffect(drawable, false, progress, error)) {
            cancelRenderOffscreenRenderTarget(lastEffect);
            /* decrement self reference to destroy correctly */
            EffectReferenceMap::iterator it = m_effectReferences.find(lastEffect->fileURI().absolutePath());
            if (it != m_effectReferences.end()) {
                it->second.second--;
            }
            destroyEffect(lastEffect);
            if (Effect *ownerEffect = resolveEffect(drawable)) {
                applyAllDrawablesToOffscreenRenderTargetEffect(drawable, ownerEffect);
            }
            succeeded = true;
        }
        else if (hasScriptExternal) {
            m_dependsOnScriptExternal.push_back(drawable);
        }
    }
    return succeeded;
}

void
Project::setCameraMotion(Motion *motion)
{
    if (m_cameraMotionPtr) {
        removeMotion(m_cameraMotionPtr);
    }
    m_cameraMotionPtr = motion;
    setBaseDuration(duration());
    if (motion) {
        motion->initialize(m_camera);
        m_allMotions.push_back(motion);
        synchronizeCamera(currentLocalFrameIndex(), 0);
        eventPublisher()->publishAddMotionEvent(motion);
    }
}

void
Project::setLightMotion(Motion *motion)
{
    if (m_lightMotionPtr) {
        removeMotion(m_lightMotionPtr);
    }
    m_lightMotionPtr = motion;
    setBaseDuration(duration());
    if (motion) {
        motion->initialize(m_light);
        m_allMotions.push_back(motion);
        synchronizeLight(currentLocalFrameIndex(), 0);
        eventPublisher()->publishAddMotionEvent(motion);
    }
}

void
Project::setSelfShadowMotion(Motion *motion)
{
    if (m_selfShadowMotionPtr) {
        removeMotion(m_selfShadowMotionPtr);
    }
    m_selfShadowMotionPtr = motion;
    setBaseDuration(duration());
    if (motion) {
        motion->initialize(shadowCamera());
        m_allMotions.push_back(motion);
        synchronizeSelfShadow(currentLocalFrameIndex());
        eventPublisher()->publishAddMotionEvent(motion);
    }
}

Motion *
Project::addAccessoryMotion(Motion *motion, Accessory *accessory)
{
    nanoem_parameter_assert(motion, "must not be nullptr");
    nanoem_parameter_assert(accessory, "must not be nullptr");
    Motion *lastModelMotion = resolveMotion(accessory);
    if (lastModelMotion) {
        removeMotion(lastModelMotion);
        if (isMotionMergeEnabled()) {
            motion->mergeAllKeyframes(lastModelMotion);
        }
    }
    motion->initialize(accessory);
    undoStackClear(accessory->undoStack());
    m_drawable2MotionPtrs.insert(tinystl::make_pair(static_cast<IDrawable *>(accessory), motion));
    m_allMotions.push_back(motion);
    setBaseDuration(duration());
    eventPublisher()->publishAddMotionEvent(motion);
    return lastModelMotion;
}

Motion *
Project::addModelMotion(Motion *motion, Model *model)
{
    nanoem_parameter_assert(motion, "must not be nullptr");
    nanoem_parameter_assert(model, "must not be nullptr");
    Motion *lastModelMotion = resolveMotion(model);
    if (lastModelMotion) {
        removeMotion(lastModelMotion);
        if (isMotionMergeEnabled()) {
            motion->mergeAllKeyframes(lastModelMotion);
        }
    }
    motion->initialize(model);
    undoStackClear(model->undoStack());
    m_drawable2MotionPtrs.insert(tinystl::make_pair(static_cast<IDrawable *>(model), motion));
    m_allMotions.push_back(motion);
    setBaseDuration(duration());
    eventPublisher()->publishAddMotionEvent(motion);
    return lastModelMotion;
}

void
Project::removeModel(Model *model)
{
    nanoem_parameter_assert(model, "must not be nullptr");
    if (model == activeModel()) {
        setActiveModel(nullptr);
        internalSeek(0);
    }
    removeDrawable(model);
    ListUtils::removeItem(model, m_transformModelOrderList);
    IEventPublisher *publisher = eventPublisher();
    if (ListUtils::removeItem(model, m_allModelPtrs)) {
        MotionHashMap::iterator it2 = m_drawable2MotionPtrs.find(model);
        if (it2 != m_drawable2MotionPtrs.end()) {
            Motion *motion = it2->second;
            if (ListUtils::removeItem(motion, m_allMotions)) {
                publisher->publishRemoveMotionEvent(motion);
                destroyMotion(motion);
            }
            m_drawable2MotionPtrs.erase(it2);
        }
        publisher->publishRemoveModelEvent(model);
    }
}

void
Project::removeAccessory(Accessory *accessory)
{
    nanoem_parameter_assert(accessory, "must not be nullptr");
    if (accessory == activeAccessory()) {
        setActiveAccessory(nullptr);
    }
    removeDrawable(accessory);
    IEventPublisher *publisher = eventPublisher();
    if (ListUtils::removeItem(accessory, m_allAccessoryPtrs)) {
        MotionHashMap::iterator it2 = m_drawable2MotionPtrs.find(accessory);
        if (it2 != m_drawable2MotionPtrs.end()) {
            Motion *motion = it2->second;
            if (ListUtils::removeItem(motion, m_allMotions)) {
                publisher->publishRemoveMotionEvent(motion);
                destroyMotion(motion);
            }
            m_drawable2MotionPtrs.erase(it2);
        }
        rebuildAllTracks();
        publisher->publishRemoveAccessoryEvent(accessory);
    }
}

void
Project::removeMotion(Motion *motion)
{
    nanoem_parameter_assert(motion, "must not be nullptr");
    if (ListUtils::removeItem(motion, m_allMotions)) {
        for (MotionHashMap::iterator it2 = m_drawable2MotionPtrs.begin(), end2 = m_drawable2MotionPtrs.end();
             it2 != end2; ++it2) {
            if (it2->second == motion) {
                m_drawable2MotionPtrs.erase(it2);
                break;
            }
        }
        eventPublisher()->publishRemoveMotionEvent(motion);
    }
}

URI
Project::resolveFileURI(const URI &fileURI) const
{
    if (isArchiveURI(fileURI) && hasTransientPath()) {
        return URI::createFromFilePath(m_fileURI.second.m_path, fileURI.fragment());
    }
    else {
        return fileURI;
    }
}

int
Project::findDrawableOrderIndex(const IDrawable *drawable) const NANOEM_DECL_NOEXCEPT
{
    return ListUtils::indexOf(const_cast<IDrawable *>(drawable), m_drawableOrderList);
}

int
Project::findTransformOrderIndex(const Model *model) const NANOEM_DECL_NOEXCEPT
{
    return ListUtils::indexOf(const_cast<Model *>(model), m_transformModelOrderList);
}

UUID
Project::generateUUID(const void *ptr) const NANOEM_DECL_NOEXCEPT
{
    bx::HashMurmur2A hash;
    hash.begin();
    hash.add(this, sizeof(this));
    hash.add(deviceScaleUniformedViewportLayoutRect());
    hash.add(deviceScaleLastCursorPosition(Project::kCursorTypeMouseLeft));
    hash.add(currentUptimeSeconds());
    hash.add(stm_now());
    hash.add(ptr, sizeof(ptr));
    bx::RngShr3 rng(hash.end());
    return UUID::create(rng);
}

sg_image
Project::backgroundImageHandle()
{
    if (!hasBackgroundImageHandle()) {
        sg_image_desc desc;
        Inline::clearZeroMemory(desc);
        desc.min_filter = desc.mag_filter = SG_FILTER_NEAREST;
        if (Inline::isDebugLabelEnabled()) {
            desc.label = "@nanoem/BackgroundImage";
        }
        m_backgroundImage.first = sg::make_image(&desc);
        nanoem_assert(
            sg::query_image_state(m_backgroundImage.first) == SG_RESOURCESTATE_VALID, "color image must be valid");
    }
    return m_backgroundImage.first;
}

bool
Project::hasBackgroundImageHandle() const NANOEM_DECL_NOEXCEPT
{
    return sg::is_valid(m_backgroundImage.first);
}

Vector2UI16
Project::windowSize() const NANOEM_DECL_NOEXCEPT
{
    return m_windowSize;
}

void
Project::resizeWindowSize(const Vector2UI16 &value)
{
    if (m_windowSize != value) {
        m_windowSize = value;
        m_camera->update();
        activeCamera()->update();
        EnumUtils::setEnabled(kResetAllPasses, m_stateFlags, true);
    }
}

void
Project::resizeUniformedViewportLayout(const Vector4UI16 &value)
{
    const Vector2UI16 previousOffset(m_uniformViewportLayoutRect.first), currentOffset(value),
        previousSize(m_uniformViewportLayoutRect.first.z, m_uniformViewportLayoutRect.first.w),
        currentSize(value.z, value.w);
    const bool offsetChanged = previousOffset != currentOffset,
               sizeChanged = previousSize != currentSize && currentSize.x * currentSize.y > 0;
    if ((EnumUtils::isEnabled(kViewportImageSizeChanged, m_stateFlags) || offsetChanged || sizeChanged) &&
        !isViewportCaptured()) {
        m_uniformViewportLayoutRect.second = value;
        if (isUniformedViewportImageSizeEnabled()) {
            const Vector2UI16 imageSize(uniformedViewportImageSize(currentSize, m_uniformViewportImageSize.first));
            internalResizeUniformedViewportImage(imageSize);
        }
    }
    EnumUtils::setEnabled(kViewportImageSizeChanged, m_stateFlags, false);
}

void
Project::resizeUniformedViewportImage(const Vector2UI16 &value)
{
    const Vector2UI16 previousSize(m_uniformViewportImageSize.first);
    if (previousSize != value && value.x > 0 && value.y > 0) {
        Vector2UI16 newSize(value);
        if (isUniformedViewportImageSizeEnabled() && !isViewportCaptured()) {
            const Vector2UI16 layoutSize(m_uniformViewportLayoutRect.first.z, m_uniformViewportLayoutRect.first.w);
            newSize = uniformedViewportImageSize(layoutSize, value);
        }
        internalResizeUniformedViewportImage(newSize);
    }
}

void
Project::saveState(SaveState *&state)
{
    if (!state) {
        state = nanoem_new(SaveState(this));
    }
    const ICamera *camera = activeCamera();
    PerspectiveCamera &c = state->m_camera;
    c.setAngle(camera->angle());
    c.setLookAt(camera->lookAt());
    c.setDistance(camera->distance());
    c.setFov(camera->fov());
    const ILight *light = activeLight();
    DirectionalLight &l = state->m_light;
    l.setColor(light->color());
    l.setDirection(light->direction());
    state->m_activeAccessory = m_activeAccessoryPtr;
    state->m_activeModel = m_activeModelPairPtr.first;
    state->m_physicsSimulationMode = physicsEngine()->simulationMode();
    state->m_localFrameIndex = currentLocalFrameIndex();
    state->m_stateFlags = m_stateFlags;
    state->m_confirmSeekFlags = m_confirmSeekFlags;
    state->m_lastPhysicsDebugFlags = m_lastPhysicsDebugFlags;
    state->m_visibleGrid = grid()->isVisible();
}

void
Project::restoreState(const SaveState *state, bool forceSeek)
{
    if (state && state->m_tag == this) {
        setActiveAccessory(state->m_activeAccessory);
        setActiveModel(state->m_activeModel);
        const PerspectiveCamera &c = state->m_camera;
        ICamera *camera = activeCamera();
        camera->setAngle(c.angle());
        camera->setLookAt(c.lookAt());
        camera->setDistance(c.distance());
        camera->setFov(c.fov());
        camera->update();
        const DirectionalLight &l = state->m_light;
        ILight *light = activeLight();
        light->setColor(l.color());
        light->setDirection(l.direction());
        if (forceSeek) {
            internalSeek(state->m_localFrameIndex);
        }
        m_stateFlags = state->m_stateFlags;
        m_confirmSeekFlags = state->m_confirmSeekFlags;
        m_lastPhysicsDebugFlags = state->m_lastPhysicsDebugFlags;
        setPhysicsSimulationMode(state->m_physicsSimulationMode);
        grid()->setVisible(state->m_visibleGrid);
    }
}

void
Project::destroyState(SaveState *&state)
{
    nanoem_delete_safe(state);
}

void
Project::writeRedoMessage()
{
    const URI &fileURI = this->fileURI();
    Nanoem__Application__SavePointCommand base = NANOEM__APPLICATION__SAVE_POINT_COMMAND__INIT;
    MutableString absolutePath, fragment;
    StringUtils::copyString(fileURI.absolutePath(), absolutePath);
    StringUtils::copyString(fileURI.fragment(), fragment);
    Nanoem__Application__URI uri = NANOEM__APPLICATION__URI__INIT;
    uri.absolute_path = absolutePath.data();
    uri.fragment = fragment.data();
    base.file_uri = &uri;
    MutableStringList names, absolutePaths, fragments, boneNames, morphNames;
    nanoem_u32_t handleIndex = 0, stringIndex = 0, boneIndex = 0, morphIndex = 0;
    names.resize(m_allAccessoryPtrs.size() + m_allModelPtrs.size() + m_allMotions.size());
    absolutePaths.resize(names.size());
    fragments.resize(absolutePaths.size());
    base.n_accessories = m_allAccessoryPtrs.size();
    base.accessories = new Nanoem__Application__RedoLoadAccessoryCommand *[base.n_accessories];
    for (AccessoryList::const_iterator it = m_allAccessoryPtrs.begin(), end = m_allAccessoryPtrs.end(); it != end;
         ++it, handleIndex++, stringIndex++) {
        Accessory *accessory = *it;
        Nanoem__Application__RedoLoadAccessoryCommand *handle =
            nanoem_new(Nanoem__Application__RedoLoadAccessoryCommand);
        nanoem__application__redo_load_accessory_command__init(handle);
        handle->name = StringUtils::cloneString(accessory->nameConstString(), names[stringIndex]);
        handle->accessory_handle = accessory->handle();
        Nanoem__Application__URI *uri = nanoem_new(Nanoem__Application__URI);
        nanoem__application__uri__init(uri);
        const URI &fileURI = accessory->fileURI();
        uri->absolute_path = StringUtils::cloneString(fileURI.absolutePathConstString(), absolutePaths[stringIndex]);
        uri->fragment = StringUtils::cloneString(fileURI.fragmentConstString(), fragments[stringIndex]);
        handle->content_case = NANOEM__APPLICATION__REDO_LOAD_ACCESSORY_COMMAND__CONTENT_FILE_URI;
        handle->file_uri = uri;
        base.accessories[handleIndex] = handle;
    }
    base.n_models = m_allModelPtrs.size();
    base.models = new Nanoem__Application__RedoLoadModelCommand *[base.n_models];
    handleIndex = 0;
    for (ModelList::const_iterator it = m_allModelPtrs.begin(), end = m_allModelPtrs.end(); it != end;
         ++it, handleIndex++, stringIndex++) {
        Model *model = *it;
        Nanoem__Application__RedoLoadModelCommand *handle = nanoem_new(Nanoem__Application__RedoLoadModelCommand);
        nanoem__application__redo_load_model_command__init(handle);
        handle->name = StringUtils::cloneString(model->nameConstString(), names[stringIndex]);
        handle->model_handle = model->handle();
        Nanoem__Application__URI *uri = nanoem_new(Nanoem__Application__URI);
        nanoem__application__uri__init(uri);
        const URI &fileURI = model->fileURI();
        uri->absolute_path = StringUtils::cloneString(fileURI.absolutePathConstString(), absolutePaths[stringIndex]);
        uri->fragment = StringUtils::cloneString(fileURI.fragmentConstString(), fragments[stringIndex]);
        handle->content_case = NANOEM__APPLICATION__REDO_LOAD_MODEL_COMMAND__CONTENT_FILE_URI;
        handle->file_uri = uri;
        nanoem_rsize_t numBones;
        nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(model->data(), &numBones);
        handle->n_bones = numBones;
        handle->bones = new Nanoem__Application__Bone *[numBones];
        boneNames.resize(boneNames.size() + numBones);
        for (nanoem_rsize_t i = 0; i < numBones; i++) {
            const model::Bone *bone = model::Bone::cast(bones[i]);
            Nanoem__Application__Bone *item = handle->bones[i] = nanoem_new(Nanoem__Application__Bone);
            nanoem__application__bone__init(item);
            item->index = Inline::saturateInt32(i);
            item->name = StringUtils::cloneString(bone->nameConstString(), boneNames[boneIndex++]);
        }
        nanoem_rsize_t numMorphs;
        nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(model->data(), &numMorphs);
        handle->n_morphs = numMorphs;
        handle->morphs = new Nanoem__Application__Morph *[numMorphs];
        morphNames.resize(morphNames.size() + numMorphs);
        for (nanoem_rsize_t i = 0; i < numMorphs; i++) {
            const model::Morph *morph = model::Morph::cast(morphs[i]);
            Nanoem__Application__Morph *item = handle->morphs[i] = nanoem_new(Nanoem__Application__Morph);
            nanoem__application__morph__init(item);
            item->index = Inline::saturateInt32(i);
            item->name = StringUtils::cloneString(morph->nameConstString(), morphNames[morphIndex++]);
        }
        base.models[handleIndex] = handle;
    }
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SAVE_POINT;
    command.save_point = &base;
    Error ignorable;
    writeRedoMessage(&command, ignorable);
    handleIndex = 0;
    for (AccessoryList::const_iterator it = m_allAccessoryPtrs.begin(), end = m_allAccessoryPtrs.end(); it != end;
         ++it) {
        Nanoem__Application__RedoLoadAccessoryCommand *handle = base.accessories[handleIndex++];
        nanoem_delete(handle->file_uri);
        nanoem_delete(handle);
    }
    delete[] base.accessories;
    handleIndex = 0;
    for (ModelList::const_iterator it = m_allModelPtrs.begin(), end = m_allModelPtrs.end(); it != end; ++it) {
        Nanoem__Application__RedoLoadModelCommand *handle = base.models[handleIndex++];
        for (nanoem_rsize_t i = 0; i < handle->n_bones; i++) {
            nanoem_delete(handle->bones[i]);
        }
        delete[] handle->bones;
        for (nanoem_rsize_t i = 0; i < handle->n_morphs; i++) {
            nanoem_delete(handle->morphs[i]);
        }
        delete[] handle->morphs;
        nanoem_delete(handle->file_uri);
        nanoem_delete(handle);
    }
    delete[] base.models;
}

void
Project::writeRedoMessage(const Nanoem__Application__Command *command, Error &error)
{
#if defined(NANOEM_ENABLE_NANOMSG)
    if (!EnumUtils::isEnabled(kLoadingRedoFile, m_stateFlags)) {
        const URI &fileURI = redoFileURI();
        if (!fileURI.isEmpty()) {
            FileWriterScope scope;
            if (scope.open(fileURI, true, error)) {
                internal::project::Redo redo(this);
                nanoem_u32_t sequence = m_actionSequence++;
                redo.save(scope.writer(), sequence, command, error);
            }
        }
    }
#else
    BX_UNUSED_2(command, error);
#endif
}

void
Project::setWritingRedoMessageDisabled(bool value)
{
    EnumUtils::setEnabled(kLoadingRedoFile, m_stateFlags, value);
}

Vector4UI16
Project::queryDevicePixelRectangle(Project::RectangleType type, const Vector2UI16 &offset) const NANOEM_DECL_NOEXCEPT
{
    return internalQueryRectangle(type, deviceScaleUniformedViewportLayoutRect(), offset, windowDevicePixelRatio());
}

Vector4UI16
Project::queryLogicalPixelRectangle(RectangleType type, const Vector2UI16 &offset) const NANOEM_DECL_NOEXCEPT
{
    return internalQueryRectangle(type, logicalScaleUniformedViewportLayoutRect(), offset, 1.0f);
}

bool
Project::intersectsTransformHandle(
    const Vector2SI32 &logicalScalePosition, RectangleType &type) const NANOEM_DECL_NOEXCEPT
{
    bool intersected = false;
    if (isTransformHandleVisible()) {
        const Vector2UI16 offset(logicalScaleUniformedViewportLayoutRect());
        for (int i = kRectangleTypeFirstEnum; i < kRectangleTypeMaxEnum; i++) {
            const Vector4 rect(queryLogicalPixelRectangle(static_cast<RectangleType>(i), offset));
            if (Inline::intersectsRectPoint(rect, logicalScalePosition)) {
                type = static_cast<RectangleType>(i);
                intersected = true;
                break;
            }
        }
    }
    return intersected;
}

void
Project::clearAudioSource(Error &error)
{
    m_audioPlayer->destroy();
    m_audioPlayer->initialize(baseDuration(), kTimeBasedAudioSourceDefaultSampleRate, error);
    setBaseDuration(m_audioPlayer);
}

void
Project::clearBackgroundVideo()
{
    m_backgroundVideoRenderer->destroy();
}

void
Project::play()
{
    const bool playable = !isModelEditingEnabled();
    if (playable) {
        const nanoem_frame_index_t durationAt = duration(), localFrameIndexAt = currentLocalFrameIndex();
        preparePlaying();
        synchronizeAllMotions(playingSegment().frameIndexFrom(), 0, PhysicsEngine::kSimulationTimingBefore);
        resetPhysicsSimulation();
        m_audioPlayer->play();
        eventPublisher()->publishPlayEvent(durationAt, localFrameIndexAt);
    }
}

void
Project::stop()
{
    const nanoem_frame_index_t lastDuration = duration(), lastLocalFrameIndex = currentLocalFrameIndex();
    m_audioPlayer->stop();
    m_audioPlayer->update();
    prepareStopping(false);
    synchronizeAllMotions(0, 0, PhysicsEngine::kSimulationTimingBefore);
    resetPhysicsSimulation();
    synchronizeAllMotions(0, 0, PhysicsEngine::kSimulationTimingAfter);
    markAllModelsDirty();
    m_localFrameIndex = tinystl::make_pair(0u, 0u);
    m_backgroundVideoRenderer->seek(0);
    eventPublisher()->publishStopEvent(lastDuration, lastLocalFrameIndex);
}

void
Project::pause(bool force)
{
    if (force || isPlaying()) {
        const nanoem_frame_index_t lastDuration = duration(), lastLocalFrameIndex = currentLocalFrameIndex();
        m_audioPlayer->pause();
        m_audioPlayer->update();
        prepareStopping(false);
        eventPublisher()->publishPauseEvent(lastDuration, lastLocalFrameIndex);
    }
}

void
Project::resume(bool force)
{
    const bool resumeable = (force || m_audioPlayer->wasPlaying()) && !isModelEditingEnabled();
    if (resumeable) {
        const nanoem_frame_index_t lastDuration = duration(), lastLocalFrameIndex = currentLocalFrameIndex();
        preparePlaying();
        m_audioPlayer->resume();
        eventPublisher()->publishResumeEvent(lastDuration, lastLocalFrameIndex);
    }
}

void
Project::togglePlaying()
{
    if (isPaused() && isDirty()) {
        m_confirmer->resume(this);
    }
    else if (isPlaying()) {
        pause(true);
    }
    else if (isDirty()) {
        m_confirmer->play(this);
    }
    else {
        play();
    }
}

void
Project::restart(nanoem_frame_index_t frameIndex)
{
    synchronizeAllMotions(frameIndex, 0, PhysicsEngine::kSimulationTimingBefore);
    for (ModelList::const_iterator it = m_allModelPtrs.begin(), end = m_allModelPtrs.end(); it != end; ++it) {
        Model *model = *it;
        model->initializeAllRigidBodiesTransformFeedback();
        model->initializeAllSoftBodiesTransformFeedback();
    }
    internalPerformPhysicsSimulation(physicsSimulationTimeStep());
    synchronizeAllMotions(frameIndex, 0, PhysicsEngine::kSimulationTimingAfter);
    markAllModelsDirty();
}

void
Project::restart()
{
    restart(currentLocalFrameIndex());
}

void
Project::seek(nanoem_frame_index_t frameIndex, bool forceSeek)
{
    seek(frameIndex, 0, forceSeek);
}

void
Project::seek(nanoem_frame_index_t frameIndex, nanoem_f32_t amount, bool forceSeek)
{
    if (canSeek()) {
        if (forceSeek) {
            const nanoem_frame_index_t lastDuration = duration(), seekFrom = currentLocalFrameIndex();
            const nanoem_u32_t fps = preferredMotionFPS(), base = baseFPS(),
                               denominator = glm::max(m_audioPlayer->sampleRate(), fps), fpsRate = fps / base;
            const nanoem_f64_t seconds = static_cast<nanoem_f64_t>(frameIndex) / static_cast<nanoem_f32_t>(baseFPS());
            const nanoem_f32_t delta =
                frameIndex > seekFrom ? (frameIndex - seekFrom) * fpsRate * physicsSimulationTimeStep() : 0;
            const IAudioPlayer::Rational rational = { static_cast<nanoem_u64_t>(seconds * denominator), denominator };
            setBaseDuration(frameIndex);
            m_audioPlayer->seek(rational);
            m_audioPlayer->update();
            internalSeek(frameIndex, amount, delta);
            eventPublisher()->publishSeekEvent(lastDuration, frameIndex, seekFrom);
        }
        else {
            m_confirmer->seek(frameIndex, this);
        }
    }
}

void
Project::update()
{
    SG_PUSH_GROUP("Project::update");
    if (isPlaying() && continuesPlaying()) {
        m_audioPlayer->update();
        const IAudioPlayer::Rational &currentRational = m_audioPlayer->currentRational(),
                                     &lastRational = m_audioPlayer->lastRational();
        const nanoem_u32_t fps = preferredMotionFPS(), base = baseFPS(), fpsRate = fps / base;
        const nanoem_frame_index_t frameIndex = static_cast<nanoem_frame_index_t>(currentRational.subdivide() * fps),
                                   lastFrameIndex = static_cast<nanoem_frame_index_t>(lastRational.subdivide() * fps);
        const nanoem_f32_t invertFPSRate = 1.0f / fpsRate, amount = (frameIndex % fpsRate) * invertFPSRate,
                           delta = frameIndex > lastFrameIndex
            ? glm::min(frameIndex - lastFrameIndex, 0xffffu) * invertFPSRate * physicsSimulationTimeStep()
            : 0;
        internalSeek(frameIndex * invertFPSRate, amount, delta);
    }
    else if (m_physicsEngine->simulationMode() == PhysicsEngine::kSimulationModeEnableAnytime) {
        for (ModelList::const_iterator it = m_allModelPtrs.begin(), end = m_allModelPtrs.end(); it != end; ++it) {
            Model *model = *it;
            model->synchronizeAllRigidBodiesTransformFeedbackToSimulation();
        }
        m_physicsEngine->stepSimulation(physicsSimulationTimeStep());
        for (ModelList::const_iterator it = m_allModelPtrs.begin(), end = m_allModelPtrs.end(); it != end; ++it) {
            Model *model = *it;
            model->synchronizeAllRigidBodiesTransformFeedbackFromSimulation(PhysicsEngine::kRigidBodyFollowBoneSkip);
            model->resetAllVertices();
            model->deformAllMorphs(false);
            model->markStagingVertexBufferDirty();
        }
    }
    if (m_skinDeformerFactory) {
        m_skinDeformerFactory->begin();
    }
    for (ModelList::const_iterator it = m_allModelPtrs.begin(), end = m_allModelPtrs.end(); it != end; ++it) {
        Model *model = *it;
        model->updateStagingVertexBuffer();
    }
    for (LoadedEffectSet::const_iterator it = m_loadedEffectSet.begin(), end = m_loadedEffectSet.end(); it != end;
         ++it) {
        Effect *effect = *it;
        effect->markAllAnimatedImagesUpdatable();
    }
    if (m_skinDeformerFactory) {
        m_skinDeformerFactory->commit();
    }
    if (m_backgroundVideoRenderer) {
        m_backgroundVideoRenderer->flush();
    }
    SG_POP_GROUP();
}

bool
Project::resetAllPasses()
{
    bool performed = false;
    if (isResetAllPassesPending()) {
        const Vector2UI16 layoutSize(
            Vector2(m_uniformViewportLayoutRect.second.z, m_uniformViewportLayoutRect.second.w) *
            m_viewportDevicePixelRatio.second);
        const Vector2UI16 imageSize(Vector2(m_uniformViewportImageSize.second) * m_viewportDevicePixelRatio.second);
        if (layoutSize.x > 0 && layoutSize.y > 0 && imageSize.x > 0 && imageSize.y > 0) {
            if (IDebugCapture *debugCapture = m_sharedDebugCaptureRepository->debugCapture()) {
                debugCapture->start(nullptr);
            }
            SG_PUSH_GROUPF("Project::resetAllPasses(width=%d, height=%d)", imageSize.x, imageSize.y);
            m_windowDevicePixelRatio.first = m_windowDevicePixelRatio.second;
            m_viewportDevicePixelRatio.first = m_viewportDevicePixelRatio.second;
            m_viewportPixelFormat.first = m_viewportPixelFormat.second;
            m_uniformViewportLayoutRect.first = m_uniformViewportLayoutRect.second;
            m_uniformViewportImageSize.first = m_uniformViewportImageSize.second;
            m_sampleLevel.first = m_sampleLevel.second;
            internalResetAllRenderTargets(imageSize);
            m_viewportPrimaryPass.update(imageSize);
            m_viewportSecondaryPass.update(imageSize);
            m_context2DPass.update(layoutSize);
            m_camera->update();
            m_lastDrawnRenderPass = m_viewportPrimaryPass.m_handle;
            activeCamera()->update();
            resetViewportPassFormatAndDescription();
            EnumUtils::setEnabled(kResetAllPasses, m_stateFlags, false);
            SG_POP_GROUP();
            if (IDebugCapture *debugCapture = m_sharedDebugCaptureRepository->debugCapture()) {
                debugCapture->stop();
            }
            performed = true;
        }
    }
    return performed;
}

void
Project::pushUndo(undo_command_t *command)
{
    nanoem_assert(!isPlaying(), "must not be called while playing");
    if (!isPlaying()) {
        undoStackPushCommand(undoStack(), command);
        eventPublisher()->publishPushUndoCommandEvent(command);
    }
    else {
        undoCommandDestroy(command);
    }
}

bool
Project::canSeek() const NANOEM_DECL_NOEXCEPT
{
    bool seekable = !isModelEditingEnabled();
    if (const Model *model = activeModel()) {
        seekable &= !(model->hasAnyDirtyBone() && isConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_BONE));
        seekable &= !(model->hasAnyDirtyMorph() && isConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MORPH));
    }
    else {
        if (const Accessory *accessory = activeAccessory()) {
            seekable &= !(accessory->isDirty() && isConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ACCESSORY));
        }
        seekable &= !(globalCamera()->isDirty() && isConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_CAMERA));
        seekable &= !(globalLight()->isDirty() && isConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_LIGHT));
    }
    return seekable;
}

void
Project::resetPhysicsSimulation()
{
    m_physicsEngine->reset();
    for (ModelList::const_iterator it = m_allModelPtrs.begin(), end = m_allModelPtrs.end(); it != end; ++it) {
        Model *model = *it;
        model->initializeAllRigidBodiesTransformFeedback();
        model->initializeAllSoftBodiesTransformFeedback();
        model->synchronizeAllRigidBodiesTransformFeedbackFromSimulation(PhysicsEngine::kRigidBodyFollowBonePerform);
        model->markStagingVertexBufferDirty();
    }
    m_physicsEngine->stepSimulation(0);
}

void
Project::resetAllModelEdges()
{
    for (Project::ModelList::const_iterator it = m_allModelPtrs.begin(), end = m_allModelPtrs.end(); it != end; ++it) {
        Model *model = *it;
        if (model->edgeSizeScaleFactor() > 0.0f && !model->isStagingVertexBufferDirty()) {
            model->resetAllMorphDeformStates();
            model->deformAllMorphs(false);
            model->performAllBonesTransform();
            model->markStagingVertexBufferDirty();
        }
    }
}

void
Project::performPhysicsSimulationOnce()
{
    internalPerformPhysicsSimulation(physicsSimulationTimeStep());
}

void
Project::synchronizeAllMotions(
    nanoem_frame_index_t frameIndex, nanoem_f32_t amount, PhysicsEngine::SimulationTimingType timing)
{
    for (ModelList::const_iterator it = m_transformModelOrderList.begin(), end = m_transformModelOrderList.end();
         it != end; ++it) {
        Model *model = *it;
        if (Motion *motion = resolveMotion(model)) {
            model->synchronizeMotion(motion, frameIndex, amount, timing);
        }
    }
    if (timing == PhysicsEngine::kSimulationTimingAfter) {
        for (AccessoryList::const_iterator it = m_allAccessoryPtrs.begin(), end = m_allAccessoryPtrs.end(); it != end;
             ++it) {
            Accessory *accessory = *it;
            if (Motion *motion = resolveMotion(accessory)) {
                accessory->synchronizeMotion(motion, frameIndex);
            }
        }
        synchronizeCamera(frameIndex, amount);
        synchronizeLight(frameIndex, amount);
        synchronizeSelfShadow(frameIndex);
    }
}

void
Project::setRenderPassName(sg_pass pass, const char *value)
{
    SGHandleStringMap::iterator it = m_renderPassStringMap.find(pass.id);
    if (it == m_renderPassStringMap.end() && StringUtils::length(value) > 0) {
        SG_LABEL_PASS(pass, value);
        SG_INSERT_MARKERF("Project::setRenderPassName(handle=%d, name=%s)", pass.id, value);
        m_renderPassStringMap.insert(tinystl::make_pair(pass.id, String(value)));
    }
}

void
Project::setRenderPipelineName(sg_pipeline pipeline, const char *value)
{
    SGHandleStringMap::iterator it = m_renderPipelineStringMap.find(pipeline.id);
    if (it == m_renderPipelineStringMap.end() && StringUtils::length(value) > 0) {
        SG_LABEL_PIPELINE(pipeline, value);
        SG_INSERT_MARKERF("Project::setRenderPipelineName(handle=%d, name=%s)", pipeline.id, value);
        m_renderPipelineStringMap.insert(tinystl::make_pair(pipeline.id, String(value)));
    }
}

sg_image
Project::sharedFallbackImage() const NANOEM_DECL_NOEXCEPT
{
    return m_fallbackImage;
}

sg::PassBlock::IDrawQueue *
Project::sharedBatchDrawQueue() NANOEM_DECL_NOEXCEPT
{
    return m_batchDrawQueue;
}

sg::PassBlock::IDrawQueue *
Project::sharedSerialDrawQueue() NANOEM_DECL_NOEXCEPT
{
    return m_serialDrawQueue;
}

ImageLoader *
Project::sharedImageLoader()
{
    if (!m_sharedImageLoader) {
        m_sharedImageLoader = nanoem_new(ImageLoader(this));
    }
    return m_sharedImageLoader;
}

internal::BlitPass *
Project::sharedImageBlitter()
{
    if (!m_sharedImageBlitter) {
        const bool topLeft = sg::query_features().origin_top_left;
        m_sharedImageBlitter = nanoem_new(internal::BlitPass(this, !topLeft));
    }
    return m_sharedImageBlitter;
}

internal::DebugDrawer *
Project::sharedDebugDrawer()
{
    if (!m_sharedDebugDrawer) {
        m_sharedDebugDrawer = nanoem_new(internal::DebugDrawer(this));
    }
    return m_sharedDebugDrawer;
}

void
Project::drawAllOffscreenRenderTargets()
{
    if (m_drawType == IDrawable::kDrawTypeColor && !m_allOffscreenRenderTargets.empty()) {
        SG_PUSH_GROUPF("Project::drawAllOffscreenRenderTargets(size=%d)", m_allOffscreenRenderTargets.size());
        for (OffscreenRenderTargetConditionListMap::const_iterator it = m_allOffscreenRenderTargets.begin(),
                                                                   end = m_allOffscreenRenderTargets.end();
             it != end; ++it) {
            Effect *ownerEffect = it->first;
            if (ownerEffect->isEnabled()) {
                drawOffscreenRenderTarget(ownerEffect);
            }
        }
        SG_POP_GROUP();
    }
}

void
Project::drawShadowMap()
{
    if (isShadowMapEnabled()) {
        SG_PUSH_GROUP("Project::drawShadowMap");
        Matrix4x4 lightView, lightProjection;
        m_shadowCamera->getViewProjection(lightView, lightProjection);
        m_shadowCamera->clear();
        if (m_editingMode != Project::kEditingModeSelect) {
            const nanoem_rsize_t numDrawables = m_drawableOrderList.size();
            const sg_pass pass = m_shadowCamera->pass();
            PassScope scope(m_currentOffscreenRenderPass, pass), scope2(m_originOffscreenRenderPass, pass);
            BX_UNUSED_2(scope, scope2);
            for (nanoem_rsize_t j = 0; j < numDrawables; j++) {
                IDrawable *drawable = m_drawableOrderList[j];
                if (const IEffect *effect = drawable->activeEffect()) {
                    if (Effect::isScriptClassObject(effect->scriptClass())) {
                        drawable->draw(IDrawable::kDrawTypeShadowMap);
                    }
                }
                else {
                    drawable->draw(IDrawable::kDrawTypeShadowMap);
                }
            }
        }
        SG_POP_GROUP();
    }
}

void
Project::drawViewport()
{
    if (nanoem_likely(sg::is_valid(m_viewportPrimaryPass.m_handle))) {
        SG_PUSH_GROUP("Project::drawViewport");
        const bool isDrawingColorType = m_drawType == IDrawable::kDrawTypeColor;
        Matrix4x4 viewMatrix, projectionMatrix;
        activeCamera()->getViewTransform(viewMatrix, projectionMatrix);
        drawAllEffectsDependsOnScriptExternal();
        clearViewportPrimaryPass();
        if (isDrawingColorType) {
            drawBackgroundVideo();
            drawGrid();
            drawViewport(IEffect::kScriptOrderTypePreProcess, IDrawable::kDrawTypeColor);
            if (m_editingMode != kEditingModeSelect) {
                drawViewport(IEffect::kScriptOrderTypeStandard, IDrawable::kDrawTypeEdge);
            }
        }
        drawViewport(IEffect::kScriptOrderTypeStandard, m_drawType);
        if (isDrawingColorType) {
            if (isGroundShadowEnabled()) {
                drawViewport(IEffect::kScriptOrderTypeStandard, IDrawable::kDrawTypeGroundShadow);
            }
            drawViewport(IEffect::kScriptOrderTypePostProcess, IDrawable::kDrawTypeColor);
            if (m_physicsEngine->debugGeometryFlags() != 0) {
                int numObjects;
                nanoem_physics_debug_geometry_t *const *geometries = m_physicsEngine->debugGeometryObjects(&numObjects);
                for (int i = 0; i < numObjects; i++) {
                    const nanoem_physics_debug_geometry_t *geometry = geometries[i];
                    dd::line(m_physicsEngine->geometryFromPosition(geometry),
                        m_physicsEngine->geometryToPosition(geometry), m_physicsEngine->geometryColor(geometry));
                }
            }
            dd::flush();
            blitRenderPass(sharedBatchDrawQueue(), m_viewportSecondaryPass.m_handle, m_viewportPrimaryPass.m_handle,
                m_viewportPassBlitter);
        }
        m_lastDrawnRenderPass = m_viewportPrimaryPass.m_handle;
        /* reset localFrameIndex here to apply elapsed local frame in effect */
        m_localFrameIndex.second = 0;
        SG_POP_GROUP();
    }
}

void
Project::flushAllCommandBuffers()
{
    SG_PUSH_GROUPF("Project::flushAllCommandBuffers(size=%d)", m_drawQueue->size());
    m_drawQueue->flush(this);
    m_batchDrawQueue->clear();
    SG_POP_GROUP();
}

sg_pass
Project::beginRenderPass(sg_pass pass)
{
    sg_pass handle = sg::is_valid(pass) ? pass : m_viewportPrimaryPass.m_handle;
    m_lastDrawnRenderPass = handle;
    return handle;
}

void
Project::blitRenderPass(sg::PassBlock::IDrawQueue *drawQueue, sg_pass destRenderPass, sg_pass sourceRenderPass)
{
    blitRenderPass(drawQueue, destRenderPass, sourceRenderPass, m_renderPassBlitter);
}

void
Project::clearRenderPass(
    sg::PassBlock::IDrawQueue *drawQueue, sg_pass pass, const sg_pass_action &action, const PixelFormat &format)
{
    sg_pass handle = sg::is_valid(pass) ? pass : m_viewportPrimaryPass.m_handle;
    m_renderPassCleaner->clear(drawQueue, handle, action, format);
}

PixelFormat
Project::findRenderPassPixelFormat(sg_pass value, int sampleCount) const NANOEM_DECL_NOEXCEPT
{
    PixelFormat format;
    format.setColorPixelFormat(viewportPixelFormat(), 0);
    format.setNumSamples(sampleCount);
    if (sg::is_valid(value)) {
        RenderPassBundleMap::const_iterator it = m_renderPassBundleMap.find(value.id);
        if (it != m_renderPassBundleMap.end()) {
            format = it->second.m_format;
        }
    }
    return format;
}

PixelFormat
Project::currentRenderPassPixelFormat() const NANOEM_DECL_NOEXCEPT
{
    return findRenderPassPixelFormat(currentRenderPass(), sampleCount());
}

const char *
Project::findRenderPassName(sg_pass value) const NANOEM_DECL_NOEXCEPT
{
    return findRenderPassName(value, "(unknown)");
}

const char *
Project::findRenderPassName(sg_pass value, const char *fallbackName) const NANOEM_DECL_NOEXCEPT
{
    SGHandleStringMap::const_iterator it = m_renderPassStringMap.find(value.id);
    return it != m_renderPassStringMap.end() ? it->second.c_str() : fallbackName;
}

const char *
Project::findRenderPipelineName(sg_pipeline value) const NANOEM_DECL_NOEXCEPT
{
    return findRenderPipelineName(value, "(unknown)");
}

const char *
Project::findRenderPipelineName(sg_pipeline value, const char *fallbackName) const NANOEM_DECL_NOEXCEPT
{
    SGHandleStringMap::const_iterator it = m_renderPipelineStringMap.find(value.id);
    return it != m_renderPipelineStringMap.end() ? it->second.c_str() : fallbackName;
}

bool
Project::getOriginOffscreenRenderPassColorImageDescription(
    sg_pass_desc &pd, sg_image_desc &id) const NANOEM_DECL_NOEXCEPT
{
    SG_INSERT_MARKERF("Project::getOriginOffscreenPassColorImageDescription() => %d", m_originOffscreenRenderPass.id);
    bool found = false;
    if (sg::is_valid(m_originOffscreenRenderPass)) {
        RenderPassBundleMap::const_iterator it = m_renderPassBundleMap.find(m_originOffscreenRenderPass.id);
        if (it != m_renderPassBundleMap.end()) {
            const RenderPassBundle &desc = it->second;
            sg_image colorImage = desc.m_colorImage;
            found = sg::is_valid(colorImage);
            if (found) {
                const PixelFormat &format = desc.m_format;
                id.pixel_format = format.colorPixelFormat(0);
                id.sample_count = format.numSamples();
                pd.color_attachments[0].image = colorImage;
                const Vector2UI16 imageSize(deviceScaleViewportPrimaryImageSize());
                id.width = imageSize.x;
                id.height = imageSize.y;
            }
        }
    }
    return found;
}

bool
Project::getCurrentRenderPassColorImageDescription(sg_pass_desc &pd, sg_image_desc &id) const NANOEM_DECL_NOEXCEPT
{
    SG_INSERT_MARKERF("Project::getCurrentPassColorImageDescription() => %d", m_currentRenderPass.id);
    return sg::is_valid(m_currentRenderPass) ? getRenderPassColorImageDescription(m_currentRenderPass, pd, id) : false;
}

bool
Project::getScriptExternalRenderPassColorImageDescription(
    sg_pass_desc &pd, sg_image_desc &id) const NANOEM_DECL_NOEXCEPT
{
    SG_INSERT_MARKERF("Project::getScriptExternalPassColorImageDescription() => %d", m_scriptExternalRenderPass.id);
    return sg::is_valid(m_scriptExternalRenderPass)
        ? getRenderPassColorImageDescription(m_scriptExternalRenderPass, pd, id)
        : false;
}

bool
Project::getRenderPassColorImageDescription(
    sg_pass /* pass */, sg_pass_desc &pd, sg_image_desc &id) const NANOEM_DECL_NOEXCEPT
{
    RenderPassBundleMap::const_iterator it = m_renderPassBundleMap.find(m_currentRenderPass.id);
    bool found = it != m_renderPassBundleMap.end();
    if (found) {
        const RenderPassBundle &desc = it->second;
        const PixelFormat &format = desc.m_format;
        id.pixel_format = format.colorPixelFormat(0);
        id.sample_count = format.numSamples();
        pd.color_attachments[0].image = desc.m_desciption.color_attachments[0].image;
        const Vector2UI16 imageSize(deviceScaleUniformedViewportImageSize());
        id.width = imageSize.x;
        id.height = imageSize.y;
    }
    return found;
}

void
Project::getViewportRenderPassColorImageDescription(sg_pass_desc &pd, sg_image_desc &id) const NANOEM_DECL_NOEXCEPT
{
    id.pixel_format = viewportPixelFormat();
    id.sample_count = sampleCount();
    pd.color_attachments[0].image = viewportPrimaryImage();
    const Vector2UI16 imageSize(deviceScaleViewportPrimaryImageSize());
    id.width = imageSize.x;
    id.height = imageSize.y;
}

bool
Project::getOriginOffscreenRenderPassDepthImageDescription(
    sg_pass_desc &pd, sg_image_desc &id) const NANOEM_DECL_NOEXCEPT
{
    BX_UNUSED_1(id);
    bool found = false;
    if (sg::is_valid(m_originOffscreenRenderPass)) {
        RenderPassBundleMap::const_iterator it = m_renderPassBundleMap.find(m_originOffscreenRenderPass.id);
        if (it != m_renderPassBundleMap.end()) {
            const RenderPassBundle &desc = it->second;
            sg_image depthImage = desc.m_depthImage;
            found = sg::is_valid(depthImage);
            if (found) {
                pd.depth_stencil_attachment.image = depthImage;
            }
        }
    }
    return found;
}

bool
Project::getCurrentRenderPassDepthImageDescription(
    sg_pass_desc &pd, sg_image_desc & /* id */) const NANOEM_DECL_NOEXCEPT
{
    bool found = false;
    if (sg::is_valid(m_currentRenderPass)) {
        RenderPassBundleMap::const_iterator it = m_renderPassBundleMap.find(m_currentRenderPass.id);
        found = it != m_renderPassBundleMap.end();
        if (found) {
            const RenderPassBundle &desc = it->second;
            pd.depth_stencil_attachment.image = desc.m_desciption.depth_stencil_attachment.image;
        }
    }
    return found;
}

void
Project::getViewportRenderPassDepthImageDescription(sg_pass_desc &pd, sg_image_desc &id) const NANOEM_DECL_NOEXCEPT
{
    pd.depth_stencil_attachment.image = m_viewportPrimaryPass.m_depthImage;
    BX_UNUSED_1(id);
}

bool
Project::isRenderPassOutputSame(sg_pass pass, const sg_pass_desc &pd) const NANOEM_DECL_NOEXCEPT
{
    RenderPassBundleMap::const_iterator it = m_renderPassBundleMap.find(pass.id);
    bool result = false;
    if (it != m_renderPassBundleMap.end()) {
        const RenderPassBundle &namedPassDescription = it->second;
        result = namedPassDescription.m_colorImage.id == pd.color_attachments[0].image.id;
    }
    return result;
}

void
Project::attachActiveEffect(IDrawable *drawable, Effect *effect, Progress &progress, Error &error)
{
    internalSetDrawableActiveEffect(
        drawable, effect, IncludeEffectSourceMap(), true, isCompiledEffectCacheEnabled(), progress, error);
    addEffectOrderSet(drawable, effect->scriptOrder());
}

void
Project::attachActiveEffect(IDrawable *drawable, Effect *effect, const IncludeEffectSourceMap &includeEffectSources,
    Progress &progress, Error &error)
{
    internalSetDrawableActiveEffect(drawable, effect, includeEffectSources, isEffectPluginEnabled(),
        isCompiledEffectCacheEnabled(), progress, error);
    addEffectOrderSet(drawable, effect->scriptOrder());
}

void
Project::attachEffectToSelectedDrawable(Effect *targetEffect, Error &error)
{
    nanoem_u16_t handle = m_indicesOfMaterialToAttachEffect.first;
    if (handle != bx::kInvalidHandle) {
        if (Model *model = findModelByHandle(handle)) {
            nanoem_rsize_t numMaterials;
            nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(model->data(), &numMaterials);
            const ModelMaterialIndexSet &indices = m_indicesOfMaterialToAttachEffect.second;
            for (ModelMaterialIndexSet::const_iterator it = indices.begin(), end = indices.end(); it != end; ++it) {
                const nanoem_rsize_t targetMaterialIndex = *it;
                if (targetMaterialIndex < numMaterials) {
                    model::Material *material = model::Material::cast(materials[targetMaterialIndex]);
                    attachModelMaterialEffect(material, targetEffect);
                }
                else {
                    error = Error(m_translator->translate("nanoem.error.effect.material-not-found.reason"), nullptr,
                        Error::kDomainTypeApplication);
                }
            }
        }
        else {
            error = Error(m_translator->translate("nanoem.error.effect.model-effect-not-found.reason"), nullptr,
                Error::kDomainTypeApplication);
        }
    }
    else if (!m_drawablesToAttachOffscreenRenderTargetEffect.first.empty()) {
        const DrawableSet &drawables = m_drawablesToAttachOffscreenRenderTargetEffect.second;
        if (!drawables.empty()) {
            for (DrawableSet::const_iterator it = drawables.begin(), end = drawables.end(); it != end; ++it) {
                IDrawable *drawable = *it;
                setOffscreenPassiveRenderTargetEffect(
                    m_drawablesToAttachOffscreenRenderTargetEffect.first, drawable, targetEffect);
            }
        }
        else {
            error = Error(m_translator->translate("nanoem.error.effect.offscreen-not-found.reason"), nullptr,
                Error::kDomainTypeApplication);
        }
    }
    else {
        error = Error(m_translator->translate("nanoem.error.effect.invalid-attachment.reason"),
            m_translator->translate("nanoem.error.effect.invalid-attachment.recovery-suggestion"),
            Error::kDomainTypeApplication);
    }
}

void
Project::attachModelMaterialEffect(model::Material *material, Effect *effect)
{
    if (material && effect) {
        material->setEffect(effect);
        addLoadedEffectSet(effect);
    }
}

void
Project::setOffscreenPassiveRenderTargetEffect(
    const String &offscreenOwnerName, IDrawable *drawable, Effect *targetEffect)
{
    if (drawable && targetEffect) {
        drawable->setOffscreenPassiveRenderTargetEffect(offscreenOwnerName, targetEffect);
        addLoadedEffectSet(targetEffect);
    }
}

void
Project::setRedoDrawable(nanoem_u32_t key, IDrawable *value)
{
    m_redoObjectHandles.insert(tinystl::make_pair(key, value->handle()));
}

Accessory *
Project::resolveRedoAccessory(nanoem_u32_t key)
{
    RedoObjectHandleMap::const_iterator it = m_redoObjectHandles.find(key);
    return it != m_redoObjectHandles.end() ? findAccessoryByHandle(it->second) : nullptr;
}

Model *
Project::resolveRedoModel(nanoem_u32_t key)
{
    RedoObjectHandleMap::const_iterator it = m_redoObjectHandles.find(key);
    return it != m_redoObjectHandles.end() ? findModelByHandle(it->second) : nullptr;
}

void
Project::clearAllRedoHandles()
{
    m_redoObjectHandles.clear();
}

void
Project::expandAllTracks()
{
    for (TrackList::const_iterator it = m_allTracks.begin(), end = m_allTracks.end(); it != end; ++it) {
        ITrack *track = *it;
        if (track->isExpandable()) {
            track->setExpanded(true);
        }
    }
}

void
Project::collapseAllTracks()
{
    for (TrackList::const_iterator it = m_allTracks.begin(), end = m_allTracks.end(); it != end; ++it) {
        ITrack *track = *it;
        if (track->isExpandable()) {
            track->setExpanded(false);
        }
    }
}

void
Project::rebuildAllTracks()
{
    setSelectedTrack(nullptr);
    for (TrackList::const_iterator it = m_allTracks.begin(), end = m_allTracks.end(); it != end; ++it) {
        nanoem_delete(*it);
    }
    m_allTracks.clear();
    if (Model *model = activeModel()) {
        nanoem_rsize_t numLabels, numItems;
        nanoem_model_label_t *const *labels = nanoemModelGetAllLabelObjects(model->data(), &numLabels);
        m_allTracks.push_back(nanoem_new(internal::project::ModelTrack(model)));
        for (nanoem_rsize_t i = 0; i < numLabels; i++) {
            const nanoem_model_label_t *label = labels[i];
            nanoem_model_label_item_t *const *items = nanoemModelLabelGetAllItemObjects(label, &numItems);
            nanoem_rsize_t numActualItems = 0;
            for (nanoem_rsize_t j = 0; j < numItems; j++) {
                const nanoem_model_label_item_t *item = items[j];
                if (nanoemModelLabelItemGetType(item) != NANOEM_MODEL_LABEL_ITEM_TYPE_UNKNOWN) {
                    numActualItems++;
                }
            }
            if (numActualItems > 0) {
                m_allTracks.push_back(nanoem_new(internal::project::ModelLabeledTrack(label)));
            }
        }
        m_selectedTrack = m_allTracks[0];
    }
    else {
        m_allTracks.push_back(nanoem_new(internal::project::CameraTrack(m_camera)));
        m_allTracks.push_back(nanoem_new(internal::project::LightTrack(m_light)));
        m_allTracks.push_back(nanoem_new(internal::project::SelfShadowTrack(m_shadowCamera)));
        for (Project::AccessoryList::const_iterator it = m_allAccessoryPtrs.begin(), end = m_allAccessoryPtrs.end();
             it != end; ++it) {
            Accessory *accessory = *it;
            m_allTracks.push_back(nanoem_new(internal::project::AccessoryTrack(accessory)));
        }
        m_selectedTrack = m_allTracks[0];
    }
}

void
Project::selectAllKeyframes(nanoem_u32_t flags)
{
    selectAllKeyframes(activeModel(), flags);
}

void
Project::selectAllKeyframes(Model *model, nanoem_u32_t flags)
{
    if (Motion *modelMotionPtr = resolveMotion(model)) {
        modelMotionPtr->selection()->addAllKeyframes(flags);
    }
    else {
        if (Motion *cameraMotionPtr = cameraMotion()) {
            cameraMotionPtr->selection()->addAllKeyframes(flags);
        }
        if (Motion *lightMotionPtr = lightMotion()) {
            lightMotionPtr->selection()->addAllKeyframes(flags);
        }
        for (Project::AccessoryList::const_iterator it = m_allAccessoryPtrs.begin(), end = m_allAccessoryPtrs.end();
             it != end; ++it) {
            Accessory *accessory = *it;
            if (Motion *accessoryMotionPtr = resolveMotion(accessory)) {
                accessoryMotionPtr->selection()->addAllKeyframes(flags);
            }
        }
    }
}

void
Project::copyAllSelectedKeyframes(Error &error)
{
    copyAllSelectedKeyframes(activeModel(), error);
}

void
Project::copyAllSelectedKeyframes(Model *model, Error &error)
{
    nanoem_assert(!isPlaying(), "must not be called while playing");
    Motion *dest = createMotion();
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_motion_t *motion = nanoemMutableMotionCreateAsReference(dest->data(), &status);
    if (const Motion *modelMotionPtr = resolveMotion(model)) {
        Motion::BoneKeyframeList boneKeyframes;
        Motion::ModelKeyframeList modelKeyframes;
        Motion::MorphKeyframeList morphKeyframes;
        int boneKeyframeStartOffset, modelKeyframeStartOffset, morphKeyframeStartOffset;
        const IMotionKeyframeSelection *selection = modelMotionPtr->selection();
        selection->getAll(boneKeyframes, &boneKeyframeStartOffset);
        selection->getAll(modelKeyframes, &modelKeyframeStartOffset);
        selection->getAll(morphKeyframes, &morphKeyframeStartOffset);
        Motion::copyAllBoneKeyframes(const_cast<nanoem_motion_bone_keyframe_t *const *>(boneKeyframes.data()),
            boneKeyframes.size(), selection, model, motion, boneKeyframeStartOffset * -1, status);
        Motion::copyAllModelKeyframes(const_cast<nanoem_motion_model_keyframe_t *const *>(modelKeyframes.data()),
            modelKeyframes.size(), selection, motion, modelKeyframeStartOffset * -1, status);
        Motion::copyAllMorphKeyframes(const_cast<nanoem_motion_morph_keyframe_t *const *>(morphKeyframes.data()),
            morphKeyframes.size(), selection, model, motion, morphKeyframeStartOffset * -1, status);
        const nanoem_unicode_string_t *name = nanoemModelGetName(model->data(), NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        nanoemMutableMotionSetTargetModelName(motion, name, &status);
    }
    else {
        Motion::AccessoryKeyframeList allAccessoryKeyframes;
        Motion::CameraKeyframeList cameraKeyframes;
        Motion::LightKeyframeList lightKeyframes;
        Motion::SelfShadowKeyframeList selfShadowKeyframes;
        int accessoryKeyframeStartOffset, cameraKeyframeStartOffset = 0, lightKeyframeStartOffset = 0,
                                          selfShadowKeyframeStartOffset = 0;
        if (const Motion *cameraMotionPtr = cameraMotion()) {
            const IMotionKeyframeSelection *selection = cameraMotionPtr->selection();
            selection->getAll(cameraKeyframes, &cameraKeyframeStartOffset);
        }
        if (const Motion *lightMotionPtr = lightMotion()) {
            const IMotionKeyframeSelection *selection = lightMotionPtr->selection();
            selection->getAll(lightKeyframes, &lightKeyframeStartOffset);
        }
        if (const Motion *selfShadowMotionPtr = selfShadowMotion()) {
            const IMotionKeyframeSelection *selection = selfShadowMotionPtr->selection();
            selection->getAll(selfShadowKeyframes, &selfShadowKeyframeStartOffset);
        }
        Motion::copyAllCameraKeyframes(const_cast<nanoem_motion_camera_keyframe_t *const *>(cameraKeyframes.data()),
            cameraKeyframes.size(), motion, cameraKeyframeStartOffset * -1, status);
        Motion::copyAllLightKeyframes(const_cast<nanoem_motion_light_keyframe_t *const *>(lightKeyframes.data()),
            lightKeyframes.size(), motion, lightKeyframeStartOffset * -1, status);
        Motion::copyAllSelfShadowKeyframes(
            const_cast<nanoem_motion_self_shadow_keyframe_t *const *>(selfShadowKeyframes.data()),
            selfShadowKeyframes.size(), motion, selfShadowKeyframeStartOffset * -1, status);
        for (Project::AccessoryList::const_iterator it = m_allAccessoryPtrs.begin(), end = m_allAccessoryPtrs.end();
             it != end; ++it) {
            const Accessory *accessory = *it;
            if (const Motion *accessoryMotionPtr = resolveMotion(accessory)) {
                Motion::AccessoryKeyframeList accessoryKeyframes;
                const IMotionKeyframeSelection *selection = accessoryMotionPtr->selection();
                selection->getAll(accessoryKeyframes, &accessoryKeyframeStartOffset);
                Motion::copyAllAccessoryKeyframes(
                    const_cast<nanoem_motion_accessory_keyframe_t *const *>(accessoryKeyframes.data()),
                    accessoryKeyframes.size(), motion, accessoryKeyframeStartOffset * -1, status);
            }
        }
    }
    nanoemMutableMotionDestroy(motion);
    clearMotionClipboard();
    dest->save(m_motionClipboard, model, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL, error);
    destroyMotion(dest);
}

void
Project::pasteAllSelectedKeyframes(nanoem_frame_index_t frameIndex, Error &error)
{
    pasteAllSelectedKeyframes(activeModel(), frameIndex, error);
}

void
Project::pasteAllSelectedKeyframes(Model *model, nanoem_frame_index_t frameIndex, Error &error)
{
    internalPasteAllSelectedKeyframes(model, frameIndex, false, error);
}

void
Project::symmetricPasteAllSelectedKeyframes(nanoem_frame_index_t frameIndex, Error &error)
{
    symmetricPasteAllSelectedKeyframes(activeModel(), frameIndex, error);
}

void
Project::symmetricPasteAllSelectedKeyframes(Model *model, nanoem_frame_index_t frameIndex, Error &error)
{
    internalPasteAllSelectedKeyframes(model, frameIndex, true, error);
    restart();
}

void
Project::selectAllKeyframesInColumn()
{
    selectAllKeyframesInColumn(activeModel());
}

void
Project::selectAllKeyframesInColumn(Model *model)
{
    const nanoem_frame_index_t frameIndex = currentLocalFrameIndex();
    if (Motion *motion = resolveMotion(model)) {
        IMotionKeyframeSelection *selection = motion->selection();
        nanoem_rsize_t numBones, numMorphs;
        nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(model->data(), &numBones);
        for (nanoem_rsize_t i = 0; i < numBones; i++) {
            const nanoem_model_bone_t *bonePtr = bones[i];
            const nanoem_unicode_string_t *name = nanoemModelBoneGetName(bonePtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
            selection->add(motion->findBoneKeyframe(name, frameIndex));
        }
        nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(model->data(), &numMorphs);
        for (nanoem_rsize_t i = 0; i < numMorphs; i++) {
            const nanoem_model_morph_t *morphPtr = morphs[i];
            const nanoem_unicode_string_t *name = nanoemModelMorphGetName(morphPtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
            selection->add(motion->findMorphKeyframe(name, frameIndex));
        }
        selection->add(motion->findModelKeyframe(frameIndex));
    }
    else {
        {
            Motion *motion = cameraMotion();
            IMotionKeyframeSelection *selection = motion->selection();
            selection->clearAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL);
            selection->add(motion->findCameraKeyframe(frameIndex));
        }
        {
            Motion *motion = lightMotion();
            IMotionKeyframeSelection *selection = motion->selection();
            selection->clearAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL);
            selection->add(motion->findLightKeyframe(frameIndex));
        }
        {
            Motion *motion = selfShadowMotion();
            IMotionKeyframeSelection *selection = motion->selection();
            selection->clearAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL);
            selection->add(motion->findSelfShadowKeyframe(frameIndex));
        }
        for (Project::AccessoryList::const_iterator it = m_allAccessoryPtrs.begin(), end = m_allAccessoryPtrs.end();
             it != end; ++it) {
            Accessory *accessory = *it;
            if (Motion *motion = resolveMotion(accessory)) {
                IMotionKeyframeSelection *selection = motion->selection();
                selection->clearAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL);
                selection->add(motion->findAccessoryKeyframe(frameIndex));
            }
        }
    }
}

void
Project::selectAllMotionKeyframesIn(const TimelineSegment &range)
{
    selectAllMotionKeyframesIn(range, activeModel());
}

void
Project::selectAllMotionKeyframesIn(const TimelineSegment &range, Model *model)
{
    if (const ITrack *track = selectedTrack()) {
        if (Motion *motion = resolveMotion(model)) {
            IMotionKeyframeSelection *selection = motion->selection();
            switch (track->type()) {
            case ITrack::kTypeModelBone: {
                const nanoem_model_bone_t *bone = static_cast<const nanoem_model_bone_t *>(track->opaque());
                selection->clearAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_BONE);
                selection->addBoneKeyframes(bone, range.m_from, range.m_to);
                break;
            }
            case ITrack::kTypeModelRoot: {
                selection->clearAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MODEL);
                selection->addModelKeyframes(range.m_from, range.m_to);
                break;
            }
            case ITrack::kTypeModelMorph: {
                const nanoem_model_morph_t *morph = static_cast<const nanoem_model_morph_t *>(track->opaque());
                selection->clearAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MORPH);
                selection->addMorphKeyframes(morph, range.m_from, range.m_to);
                break;
            }
            default:
                break;
            }
        }
        else {
            ITrack *mutableSelectedTrack = selectedTrack();
            switch (mutableSelectedTrack->type()) {
            case ITrack::kTypeCamera: {
                Motion *motion = cameraMotion();
                IMotionKeyframeSelection *selection = motion->selection();
                selection->clearAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_CAMERA);
                selection->addCameraKeyframes(range.m_from, range.m_to);
                break;
            }
            case ITrack::kTypeLight: {
                Motion *motion = lightMotion();
                IMotionKeyframeSelection *selection = motion->selection();
                selection->clearAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_LIGHT);
                selection->addLightKeyframes(range.m_from, range.m_to);
                break;
            }
            case ITrack::kTypeSelfShadow: {
                Motion *motion = selfShadowMotion();
                IMotionKeyframeSelection *selection = motion->selection();
                selection->clearAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_SELFSHADOW);
                selection->addSelfShadowKeyframes(range.m_from, range.m_to);
                break;
            }
            case ITrack::kTypeAccessory: {
                Accessory *accessory = static_cast<Accessory *>(mutableSelectedTrack->opaque());
                if (Motion *motion = resolveMotion(accessory)) {
                    IMotionKeyframeSelection *selection = motion->selection();
                    selection->clearAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ACCESSORY);
                    selection->addAccessoryKeyframes(range.m_from, range.m_to);
                }
                break;
            }
            default:
                break;
            }
        }
    }
}

void
Project::clearMotionClipboard()
{
    m_motionClipboard.clear();
}

void
Project::copyAllSelectedKeyframeInterpolations()
{
}

void
Project::pasteAllSelectedKeyframeInterpolations(nanoem_frame_index_t frameIndex)
{
    BX_UNUSED_1(frameIndex);
}

bool
Project::hasSelectedKeyframeInterpolations() const NANOEM_DECL_NOEXCEPT
{
    return false;
}

void
Project::makeAllSelectedKeyframeInterpolationsLinear(Error &error)
{
    makeAllSelectedKeyframeInterpolationsLinear(activeModel(), error);
}

void
Project::makeAllSelectedKeyframeInterpolationsLinear(Model *model, Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    Motion *cameraMotionPtr;
    ByteArray snapshot;
    if (Motion *modelMotionPtr = resolveMotion(model)) {
        Motion::BoneKeyframeList keyframes;
        cameraMotionPtr = modelMotionPtr;
        modelMotionPtr->save(snapshot, model, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL, error);
        modelMotionPtr->selection()->getAll(keyframes, nullptr);
        nanoem_motion_t *motion = modelMotionPtr->data();
        for (Motion::BoneKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end; ++it) {
            const nanoem_motion_bone_keyframe_t *keyframe = *it;
            const nanoem_unicode_string_t *name = nanoemMotionBoneKeyframeGetName(keyframe);
            const nanoem_frame_index_t frameIndex =
                nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionBoneKeyframeGetKeyframeObject(keyframe));
            nanoem_mutable_motion_bone_keyframe_t *transient =
                nanoemMutableMotionBoneKeyframeCreateByFound(motion, name, frameIndex, &status);
            nanoemMutableMotionBoneKeyframeSetInterpolation(
                transient, m_boneInterpolationType, glm::value_ptr(model::Bone::kDefaultBezierControlPoint));
            nanoemMutableMotionBoneKeyframeDestroy(transient);
        }
        if (model::Bone *bone = model::Bone::cast(model->activeBone())) {
            bone->setBezierControlPoints(m_boneInterpolationType, model::Bone::kDefaultBezierControlPoint);
        }
    }
    else {
        Motion::CameraKeyframeList keyframes;
        cameraMotionPtr = cameraMotion();
        cameraMotionPtr->save(snapshot, nullptr, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL, error);
        cameraMotionPtr->selection()->getAll(keyframes, nullptr);
        nanoem_motion_t *motion = cameraMotionPtr->data();
        for (Motion::CameraKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end;
             ++it) {
            const nanoem_motion_camera_keyframe_t *keyframe = *it;
            const nanoem_frame_index_t frameIndex =
                nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionCameraKeyframeGetKeyframeObject(keyframe));
            nanoem_mutable_motion_camera_keyframe_t *transient =
                nanoemMutableMotionCameraKeyframeCreateByFound(motion, frameIndex, &status);
            nanoemMutableMotionCameraKeyframeSetInterpolation(
                transient, m_cameraInterpolationType, glm::value_ptr(PerspectiveCamera::kDefaultBezierControlPoint));
            nanoemMutableMotionCameraKeyframeDestroy(transient);
        }
        globalCamera()->setBezierControlPoints(
            m_cameraInterpolationType, PerspectiveCamera::kDefaultBezierControlPoint);
    }
    pushUndo(command::MotionSnapshotCommand::create(cameraMotionPtr, model, snapshot,
        model ? NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_BONE : NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_CAMERA));
}

void
Project::copyAllSelectedBones(Error &error)
{
    copyAllSelectedBones(activeModel(), error);
}

void
Project::copyAllSelectedBones(Model *model, Error &error)
{
    if (model) {
        Model *dest = createModel();
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoem_unicode_string_factory_t *factory = unicodeStringFactory();
        nanoem_mutable_model_t *mutableModel = nanoemMutableModelCreateAsReference(dest->data(), &status);
        nanoem_model_t *originModel = nanoemMutableModelGetOriginObject(mutableModel);
        nanoemMutableModelSetFormatType(mutableModel, NANOEM_MODEL_FORMAT_TYPE_PMX_2_1);
        StringUtils::UnicodeStringScope scope(factory);
        if (StringUtils::tryGetString(factory, model->name(), scope)) {
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            nanoemMutableModelSetName(mutableModel, scope.value(), NANOEM_LANGUAGE_TYPE_FIRST_ENUM, &status);
        }
        nanoem_mutable_model_morph_t *morph = nanoemMutableModelMorphCreate(originModel, &status);
        nanoemMutableModelMorphSetCategory(morph, NANOEM_MODEL_MORPH_CATEGORY_OTHER);
        nanoemMutableModelMorphSetType(morph, NANOEM_MODEL_MORPH_TYPE_BONE);
        const model::Bone::Set *boneSet = model->selection()->allBoneSet();
        for (model::Bone::Set::const_iterator it = boneSet->begin(), end = boneSet->end(); it != end; ++it) {
            const model::Bone *bone = model::Bone::cast(*it);
            if (bone && StringUtils::tryGetString(factory, bone->name(), scope)) {
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                nanoem_mutable_model_bone_t *mutableBone = nanoemMutableModelBoneCreate(originModel, &status);
                nanoemMutableModelBoneSetName(mutableBone, scope.value(), NANOEM_LANGUAGE_TYPE_FIRST_ENUM, &status);
                nanoemMutableModelInsertBoneObject(mutableModel, mutableBone, -1, &status);
                nanoem_mutable_model_morph_bone_t *child = nanoemMutableModelMorphBoneCreate(morph, &status);
                nanoemMutableModelMorphBoneSetBoneObject(child, nanoemMutableModelBoneGetOriginObject(mutableBone));
                nanoemMutableModelMorphBoneSetTranslation(
                    child, glm::value_ptr(Vector4(bone->localUserTranslation(), 1)));
                nanoemMutableModelMorphBoneSetOrientation(child, glm::value_ptr(bone->localUserOrientation()));
                nanoemMutableModelMorphInsertBoneMorphObject(morph, child, -1, &status);
                nanoemMutableModelBoneDestroy(mutableBone);
                nanoemMutableModelMorphBoneDestroy(child);
            }
        }
        nanoemMutableModelInsertMorphObject(mutableModel, morph, -1, &status);
        nanoemMutableModelDestroy(mutableModel);
        nanoemMutableModelMorphDestroy(morph);
        dest->save(m_modelClipboard, error);
        destroyModel(dest);
    }
}

void
Project::pasteAllSelectedBones(Error &error)
{
    pasteAllSelectedBones(activeModel(), error);
}

void
Project::pasteAllSelectedBones(Model *model, Error &error)
{
    internalPasteAllSelectedBones(model, false, error);
}

void
Project::symmetricPasteAllSelectedBones(Error &error)
{
    symmetricPasteAllSelectedBones(activeModel(), error);
}

void
Project::symmetricPasteAllSelectedBones(Model *model, Error &error)
{
    internalPasteAllSelectedBones(model, true, error);
}

void
Project::selectAllBoneKeyframesFromSelectedBoneSet(Model *model)
{
    if (Motion *motion = resolveMotion(model)) {
        const nanoem_frame_index_t frameIndex = currentLocalFrameIndex();
        const model::Bone::Set *boneSet = model->selection()->allBoneSet();
        IMotionKeyframeSelection *selection = motion->selection();
        for (model::Bone::Set::const_iterator it = boneSet->begin(), end = boneSet->end(); it != end; ++it) {
            const nanoem_model_bone_t *bone = *it;
            const nanoem_unicode_string_t *name = nanoemModelBoneGetName(bone, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
            selection->add(motion->findBoneKeyframe(name, frameIndex));
        }
    }
}

bool
Project::isModelClipboardEmpty() const NANOEM_DECL_NOEXCEPT
{
    return m_modelClipboard.empty();
}

bool
Project::isMotionClipboardEmpty() const NANOEM_DECL_NOEXCEPT
{
    return m_motionClipboard.empty();
}

bool
Project::hasKeyframeSelection() const NANOEM_DECL_NOEXCEPT
{
    bool hasSelection = false;
    if (const Motion *motion = resolveMotion(activeModel())) {
        hasSelection |= motion->selection()->hasAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL);
    }
    else {
        if (const Motion *cameraMotionPtr = cameraMotion()) {
            hasSelection |= cameraMotionPtr->selection()->hasAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL);
        }
        if (const Motion *lightMotionPtr = lightMotion()) {
            hasSelection |= lightMotionPtr->selection()->hasAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL);
        }
        if (const Motion *selfShadowMotionPtr = selfShadowMotion()) {
            hasSelection |= selfShadowMotionPtr->selection()->hasAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL);
        }
        for (Project::AccessoryList::const_iterator it = m_allAccessoryPtrs.begin(), end = m_allAccessoryPtrs.end();
             it != end; ++it) {
            Accessory *accessory = *it;
            hasSelection |=
                resolveMotion(accessory)->selection()->hasAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL);
        }
    }
    return hasSelection;
}

void
Project::toggleTransformCoordinateType()
{
    if (Model *model = activeModel()) {
        model->toggleTransformCoordinateType();
    }
    else {
        globalCamera()->toggleTransformCoordinateType();
    }
}

void
Project::handleUndoAction()
{
    if (canUndo()) {
        undoStackUndo(activeUndoStack());
        eventPublisher()->publishUndoEvent(canUndo(), canRedo());
    }
}

void
Project::handleRedoAction()
{
    if (canRedo()) {
        undoStackRedo(activeUndoStack());
        eventPublisher()->publishRedoEvent(canRedo(), canUndo());
    }
}

void
Project::handleCopyAction(Error &error)
{
    Model *model = activeModel();
    if (model && editingMode() == Project::kEditingModeSelect) {
        copyAllSelectedBones(error);
    }
    else {
        copyAllSelectedKeyframes(model, error);
    }
    eventPublisher()->publishCanPasteEvent(true);
}

void
Project::handleCutAction(Error &error)
{
    Model *model = activeModel();
    if (!(model && editingMode() == Project::kEditingModeSelect)) {
        copyAllSelectedKeyframes(model, error);
        CommandRegistrator registrator(this);
        registrator.registerRemoveAllSelectedKeyframesCommand(model);
    }
}

void
Project::handlePasteAction(Error &error)
{
    Model *model = activeModel();
    if (model && editingMode() == Project::kEditingModeSelect) {
        pasteAllSelectedBones(model, error);
    }
    else {
        pasteAllSelectedKeyframes(model, currentLocalFrameIndex(), error);
    }
}

bool
Project::canCut() const NANOEM_DECL_NOEXCEPT
{
    return currentLocalFrameIndex() > 0 && !isPlaying();
}

bool
Project::canUndo() const NANOEM_DECL_NOEXCEPT
{
    return undoStackCanUndo(activeUndoStack()) && !isPlaying();
}

bool
Project::canRedo() const NANOEM_DECL_NOEXCEPT
{
    return undoStackCanRedo(activeUndoStack()) && !isPlaying();
}

bool
Project::isDirty() const NANOEM_DECL_NOEXCEPT
{
    bool dirty = false;
    dirty |= globalCamera()->isDirty();
    dirty |= globalLight()->isDirty();
    dirty |= shadowCamera()->isDirty();
    for (AccessoryList::const_iterator it = m_allAccessoryPtrs.begin(), end = m_allAccessoryPtrs.end(); it != end;
         ++it) {
        const Accessory *accessory = *it;
        dirty |= accessory->isDirty();
    }
    for (ModelList::const_iterator it = m_allModelPtrs.begin(), end = m_allModelPtrs.end(); it != end; ++it) {
        const Model *model = *it;
        dirty |= (model->isDirty() || model->hasAnyDirtyBone() || model->hasAnyDirtyMorph());
    }
    return dirty;
}

bool
Project::isPlaying() const NANOEM_DECL_NOEXCEPT
{
    return audioPlayer()->isPlaying();
}

bool
Project::isPaused() const NANOEM_DECL_NOEXCEPT
{
    return audioPlayer()->isPaused();
}

nanoem_frame_index_t
Project::duration(nanoem_frame_index_t baseDuration) const NANOEM_DECL_NOEXCEPT
{
    nanoem_frame_index_t duration = glm::clamp(baseDuration, kMinimumBaseDuration, kMaximumBaseDuration);
    if (m_cameraMotionPtr) {
        duration = glm::max(duration, m_cameraMotionPtr->duration());
    }
    if (m_lightMotionPtr) {
        duration = glm::max(duration, m_lightMotionPtr->duration());
    }
    for (MotionHashMap::const_iterator it = m_drawable2MotionPtrs.begin(), end = m_drawable2MotionPtrs.end(); it != end;
         ++it) {
        const Motion *motion = it->second;
        duration = glm::max(duration, motion->duration());
    }
    return duration;
}

nanoem_frame_index_t
Project::duration() const NANOEM_DECL_NOEXCEPT
{
    return duration(m_baseDuration);
}

nanoem_frame_index_t
Project::baseDuration() const NANOEM_DECL_NOEXCEPT
{
    return m_baseDuration;
}

nanoem_frame_index_t
Project::currentLocalFrameIndex() const NANOEM_DECL_NOEXCEPT
{
    return m_localFrameIndex.first;
}

nanoem_frame_index_t
Project::elapsedLocalFrameIndex() const NANOEM_DECL_NOEXCEPT
{
    return m_localFrameIndex.second;
}

void
Project::setBaseDuration(nanoem_frame_index_t value)
{
    m_baseDuration = glm::max(glm::clamp(value, kMinimumBaseDuration, kMaximumBaseDuration), m_baseDuration);
    nanoem_frame_index_t newDuration = duration(value);
    m_playingSegment.m_to = glm::max(m_playingSegment.m_to, newDuration);
    m_selectionSegment.m_to = glm::max(m_selectionSegment.m_to, newDuration);
    m_audioPlayer->expandDuration(value);
}

void
Project::setBaseDuration(const IAudioPlayer *audio)
{
    m_playingSegment.m_to = m_selectionSegment.m_to = m_baseDuration = kMinimumBaseDuration;
    setBaseDuration(nanoem_frame_index_t(audio->durationRational().subdivide() * baseFPS()));
}

TimelineSegment
Project::playingSegment() const NANOEM_DECL_NOEXCEPT
{
    return m_playingSegment;
}

void
Project::setPlayingSegment(const TimelineSegment &value)
{
    m_playingSegment = value.normalized(duration());
}

TimelineSegment
Project::selectionSegment() const NANOEM_DECL_NOEXCEPT
{
    return m_selectionSegment;
}

void
Project::setSelectionSegment(const TimelineSegment &value)
{
    m_selectionSegment = value.normalized(duration());
}

nanoem_f32_t
Project::motionFPSScaleFactor() const NANOEM_DECL_NOEXCEPT
{
    return preferredMotionFPS() / nanoem_f32_t(baseFPS());
}

const Project::DrawableList *
Project::drawableOrderList() const NANOEM_DECL_NOEXCEPT
{
    return &m_drawableOrderList;
}

void
Project::setDrawableOrderList(const DrawableList &value)
{
    typedef tinystl::unordered_set<IDrawable *, TinySTLAllocator> DrawableSet;
    DrawableList v;
    DrawableSet set;
    for (AccessoryList::const_iterator it = m_allAccessoryPtrs.begin(), end = m_allAccessoryPtrs.end(); it != end;
         ++it) {
        set.insert(*it);
    }
    for (ModelList::const_iterator it = m_allModelPtrs.begin(), end = m_allModelPtrs.end(); it != end; ++it) {
        set.insert(*it);
    }
    for (DrawableList::const_iterator it = value.begin(), end = value.end(); it != end; ++it) {
        if (*it && set.erase(*it)) {
            v.push_back(*it);
        }
    }
    for (DrawableSet::const_iterator it = set.begin(), end = set.end(); it != end; ++it) {
        v.push_back(*it);
    }
    m_drawableOrderList = v;
}

const Project::ModelList *
Project::transformOrderList() const NANOEM_DECL_NOEXCEPT
{
    return &m_transformModelOrderList;
}

void
Project::setTransformOrderList(const ModelList &value)
{
    ModelList v;
    typedef tinystl::unordered_set<Model *, TinySTLAllocator> ModelSet;
    ModelSet set(ListUtils::toSetFromList(m_allModelPtrs));
    for (ModelList::const_iterator it = value.begin(), end = value.end(); it != end; ++it) {
        if (*it && set.erase(*it)) {
            v.push_back(*it);
        }
    }
    for (ModelSet::const_iterator it = set.begin(), end = set.end(); it != end; ++it) {
        v.push_back(*it);
    }
    m_transformModelOrderList = v;
}

const Project::ModelList *
Project::allModels() const NANOEM_DECL_NOEXCEPT
{
    return &m_allModelPtrs;
}

const Project::AccessoryList *
Project::allAccessories() const NANOEM_DECL_NOEXCEPT
{
    return &m_allAccessoryPtrs;
}

const Project::MotionList *
Project::allMotions() const NANOEM_DECL_NOEXCEPT
{
    return &m_allMotions;
}

const Project::TrackList *
Project::allTracks() const NANOEM_DECL_NOEXCEPT
{
    return &m_allTracks;
}

Project::TrackList
Project::allFixedTracks() const
{
    TrackList fixedTracks;
    for (TrackList::const_iterator it = m_allTracks.begin(), end = m_allTracks.end(); it != end; ++it) {
        const ITrack *track = *it;
        if (track->isFixed()) {
            fixedTracks.push_back(*it);
        }
    }
    return fixedTracks;
}

Project::SGHandleStringMap
Project::allViewNames() const
{
    return m_renderPassStringMap;
}

Model *
Project::createModel()
{
    const nanoem_u16_t handle = m_objectHandleAllocator->alloc();
    Model *model = nanoem_new(Model(this, handle));
    m_modelHandleMap.insert(tinystl::make_pair(handle, model));
    return model;
}

void
Project::destroyModel(Model *model)
{
    if (model) {
        destroyDrawableEffect(model);
        model->destroy();
        nanoem_u16_t handle = model->handle();
        ModelHandleMap::const_iterator it = m_modelHandleMap.find(handle);
        if (it != m_modelHandleMap.end()) {
            m_modelHandleMap.erase(it);
        }
        m_objectHandleAllocator->free(handle);
        nanoem_delete(model);
    }
}

Motion *
Project::createMotion()
{
    const nanoem_u16_t handle = m_objectHandleAllocator->alloc();
    Motion *motion = nanoem_new(Motion(this, handle));
    m_motionHandleMap.insert(tinystl::make_pair(handle, motion));
    return motion;
}

void
Project::destroyMotion(Motion *motion)
{
    if (motion) {
        nanoem_u16_t handle = motion->handle();
        MotionHandleMap::const_iterator it = m_motionHandleMap.find(handle);
        if (it != m_motionHandleMap.end()) {
            m_motionHandleMap.erase(it);
        }
        m_objectHandleAllocator->free(handle);
        nanoem_delete(motion);
    }
}

ICamera *
Project::createCamera()
{
    return nanoem_new(PerspectiveCamera(this));
}

Accessory *
Project::createAccessory()
{
    const nanoem_u16_t handle = m_objectHandleAllocator->alloc();
    Accessory *accessory = nanoem_new(Accessory(this, handle));
    m_accessoryHandleMap.insert(tinystl::make_pair(handle, accessory));
    return accessory;
}

void
Project::destroyAccessory(Accessory *accessory)
{
    if (accessory) {
        destroyDrawableEffect(accessory);
        accessory->destroy();
        nanoem_u16_t handle = accessory->handle();
        AccessoryHandleMap::const_iterator it = m_accessoryHandleMap.find(handle);
        if (it != m_accessoryHandleMap.end()) {
            m_accessoryHandleMap.erase(it);
        }
        m_objectHandleAllocator->free(handle);
        nanoem_delete(accessory);
    }
}

Effect *
Project::createEffect()
{
    return nanoem_new(Effect(this, m_sharedResourceRepository->effectGlobalUniform(),
        m_sharedResourceRepository->accessoryProgramBundle(), m_sharedResourceRepository->modelProgramBundle()));
}

Effect *
Project::findEffect(const URI &fileURI)
{
    EffectReferenceMap::iterator it = m_effectReferences.find(fileURI.absolutePath());
    return it != m_effectReferences.end() ? it->second.first : nullptr;
}

const Effect *
Project::upcastEffect(const IEffect *value) const NANOEM_DECL_NOEXCEPT
{
    const Effect *effect = nullptr;
    if (value != reinterpret_cast<const IEffect *>(m_sharedResourceRepository->accessoryProgramBundle()) &&
        value != reinterpret_cast<const IEffect *>(m_sharedResourceRepository->modelProgramBundle())) {
        effect = static_cast<const Effect *>(value);
    }
    return effect;
}

Effect *
Project::upcastEffect(IEffect *value) NANOEM_DECL_NOEXCEPT
{
    Effect *effect = nullptr;
    if (value != reinterpret_cast<IEffect *>(m_sharedResourceRepository->accessoryProgramBundle()) &&
        value != reinterpret_cast<IEffect *>(m_sharedResourceRepository->modelProgramBundle())) {
        effect = static_cast<Effect *>(value);
    }
    return effect;
}

void
Project::destroyEffect(Effect *effect)
{
    if (effect) {
        EffectReferenceMap::iterator it2 = m_effectReferences.find(effect->fileURI().absolutePath());
        bool deletable = it2 != m_effectReferences.end() ? --it2->second.second <= 0 : true;
        if (deletable) {
            LoadedEffectSet::iterator it = m_loadedEffectSet.find(effect);
            if (it != m_loadedEffectSet.end()) {
                m_loadedEffectSet.erase(effect);
            }
            nanoem_rsize_t numMaterials;
            for (ModelList::const_iterator it = m_allModelPtrs.begin(), end = m_allModelPtrs.end(); it != end; ++it) {
                Model *model = *it;
                nanoem_model_material_t *const *materials =
                    nanoemModelGetAllMaterialObjects(model->data(), &numMaterials);
                for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
                    model::Material *material = model::Material::cast(materials[i]);
                    if (material && material->effect() == effect) {
                        material->setEffect(nullptr);
                    }
                }
            }
            cancelRenderOffscreenRenderTarget(effect);
            destroyDetachedEffect(effect);
        }
    }
}

void
Project::destroyDrawableEffect(IDrawable *drawable)
{
    if (Effect *effect = upcastEffect(drawable->activeEffect())) {
        drawable->setActiveEffect(nullptr);
        effect->destroyAllDrawableRenderTargetColorImages(drawable);
        destroyEffect(effect);
    }
}

const JSON_Value *
Project::applicationConfiguration() const NANOEM_DECL_NOEXCEPT
{
    return m_applicationConfiguration;
}

const ITranslator *
Project::translator() const NANOEM_DECL_NOEXCEPT
{
    return m_translator;
}

nanoem_unicode_string_factory_t *
Project::unicodeStringFactory() const NANOEM_DECL_NOEXCEPT
{
    return m_unicodeStringFactoryRepository->unicodeStringFactory();
}

IBackgroundVideoRenderer *
Project::backgroundVideoRenderer() NANOEM_DECL_NOEXCEPT
{
    return m_backgroundVideoRenderer;
}

Project::IConfirmer *
Project::confirmer() NANOEM_DECL_NOEXCEPT
{
    return m_confirmer;
}

IFileManager *
Project::fileManager() NANOEM_DECL_NOEXCEPT
{
    return m_fileManager;
}

IEventPublisher *
Project::eventPublisher() NANOEM_DECL_NOEXCEPT
{
    return m_eventPublisher;
}

IPrimitive2D *
Project::primitive2D() NANOEM_DECL_NOEXCEPT
{
    return m_primitive2D;
}

Project::ISharedCancelPublisherRepository *
Project::sharedCancelPublisherRepository() NANOEM_DECL_NOEXCEPT
{
    return m_sharedCancelPublisherRepository;
}

Project::ISharedDebugCaptureRepository *
Project::sharedDebugCaptureRepository() NANOEM_DECL_NOEXCEPT
{
    return m_sharedDebugCaptureRepository;
}

Project::ISharedResourceRepository *
Project::sharedResourceRepository() NANOEM_DECL_NOEXCEPT
{
    return m_sharedResourceRepository;
}

const Model *
Project::activeModel() const NANOEM_DECL_NOEXCEPT
{
    return m_activeModelPairPtr.first;
}

Model *
Project::activeModel() NANOEM_DECL_NOEXCEPT
{
    return m_activeModelPairPtr.first;
}

const Model *
Project::lastActiveModel() const NANOEM_DECL_NOEXCEPT
{
    return m_activeModelPairPtr.second;
}

Model *
Project::lastActiveModel() NANOEM_DECL_NOEXCEPT
{
    return m_activeModelPairPtr.second;
}

void
Project::setActiveModel(Model *value)
{
    Model *lastActiveModel = activeModel();
    if (value != lastActiveModel && !isModelEditingEnabled()) {
        m_activeModelPairPtr = tinystl::make_pair(value, lastActiveModel);
        if (m_editingMode == kEditingModeNone) {
            m_editingMode = kEditingModeSelect;
        }
        else if (!value) {
            m_editingMode = kEditingModeNone;
        }
        activeCamera()->update();
        IEventPublisher *e = eventPublisher();
        e->publishSetActiveModelEvent(value);
        e->publishToggleActiveModelAddBlendEnabledEvent(value ? value->isAddBlendEnabled() : false);
        e->publishToggleActiveModelShadowMapEnabledEvent(value ? value->isShadowMapEnabled() : false);
        e->publishToggleActiveModelVisibleEvent(value ? value->isVisible() : false);
        e->publishToggleActiveModelShowAllBonesEvent(value ? value->isShowAllBones() : false);
        e->publishToggleActiveModelShowAllRigidBodiesEvent(value ? value->isShowAllRigidBodyShapes() : false);
        e->publishToggleActiveModelShowAllVertexFacesEvent(value ? value->isShowAllVertexFaces() : false);
        e->publishToggleActiveModelShowAllVertexPointsEvent(value ? value->isShowAllVertexPoints() : false);
        rebuildAllTracks();
        internalSeek(currentLocalFrameIndex());
    }
}

const Accessory *
Project::activeAccessory() const NANOEM_DECL_NOEXCEPT
{
    return m_activeAccessoryPtr;
}

Accessory *
Project::activeAccessory() NANOEM_DECL_NOEXCEPT
{
    return m_activeAccessoryPtr;
}

void
Project::setActiveAccessory(Accessory *value)
{
    if (value != m_activeAccessoryPtr) {
        m_activeAccessoryPtr = value;
        IEventPublisher *e = eventPublisher();
        e->publishSetActiveAccessoryEvent(value);
        e->publishToggleActiveAccessoryAddBlendEnabledEvent(value ? value->isAddBlendEnabled() : false);
        e->publishToggleActiveAccessoryGroundShadowEnabledEvent(value ? value->isShadowMapEnabled() : false);
        e->publishToggleActiveAccessoryVisibleEvent(value ? value->isVisible() : false);
    }
}

const ICamera *
Project::activeCamera() const NANOEM_DECL_NOEXCEPT
{
    const Model *model = activeModel();
    return (model && !isCameraShared()) ? model->localCamera() : m_camera;
}

ICamera *
Project::activeCamera() NANOEM_DECL_NOEXCEPT
{
    Model *model = activeModel();
    return (model && !isCameraShared()) ? model->localCamera() : m_camera;
}

const ICamera *
Project::globalCamera() const NANOEM_DECL_NOEXCEPT
{
    return m_camera;
}

ICamera *
Project::globalCamera() NANOEM_DECL_NOEXCEPT
{
    return m_camera;
}

const ILight *
Project::activeLight() const NANOEM_DECL_NOEXCEPT
{
    return m_light;
}

ILight *
Project::activeLight() NANOEM_DECL_NOEXCEPT
{
    return m_light;
}

const ILight *
Project::globalLight() const NANOEM_DECL_NOEXCEPT
{
    return m_light;
}

ILight *
Project::globalLight() NANOEM_DECL_NOEXCEPT
{
    return m_light;
}

const Grid *
Project::grid() const NANOEM_DECL_NOEXCEPT
{
    return m_grid;
}

Grid *
Project::grid() NANOEM_DECL_NOEXCEPT
{
    return m_grid;
}

const ShadowCamera *
Project::shadowCamera() const NANOEM_DECL_NOEXCEPT
{
    return m_shadowCamera;
}

ShadowCamera *
Project::shadowCamera() NANOEM_DECL_NOEXCEPT
{
    return m_shadowCamera;
}

const IAudioPlayer *
Project::audioPlayer() const NANOEM_DECL_NOEXCEPT
{
    return m_audioPlayer;
}

IAudioPlayer *
Project::audioPlayer() NANOEM_DECL_NOEXCEPT
{
    return m_audioPlayer;
}

Project::ISkinDeformerFactory *
Project::skinDeformerFactory() NANOEM_DECL_NOEXCEPT
{
    return m_skinDeformerFactory;
}

const Motion *
Project::resolveMotion(const IDrawable *drawable) const NANOEM_DECL_NOEXCEPT
{
    MotionHashMap::const_iterator it = m_drawable2MotionPtrs.find(const_cast<IDrawable *>(drawable));
    return it != m_drawable2MotionPtrs.end() ? it->second : nullptr;
}

Motion *
Project::resolveMotion(IDrawable *drawable) NANOEM_DECL_NOEXCEPT
{
    MotionHashMap::const_iterator it = m_drawable2MotionPtrs.find(drawable);
    return it != m_drawable2MotionPtrs.end() ? it->second : nullptr;
}

const IDrawable *
Project::resolveDrawable(const Motion *motion) const NANOEM_DECL_NOEXCEPT
{
    const IDrawable *drawable = nullptr;
    for (MotionHashMap::const_iterator it = m_drawable2MotionPtrs.begin(), end = m_drawable2MotionPtrs.end(); it != end;
         ++it) {
        if (motion == it->second) {
            drawable = it->first;
            break;
        }
    }
    return drawable;
}

IDrawable *
Project::resolveDrawable(Motion *motion) NANOEM_DECL_NOEXCEPT
{
    IDrawable *drawable = nullptr;
    for (MotionHashMap::const_iterator it = m_drawable2MotionPtrs.begin(), end = m_drawable2MotionPtrs.end(); it != end;
         ++it) {
        if (motion == it->second) {
            drawable = it->first;
            break;
        }
    }
    return drawable;
}

const Motion *
Project::cameraMotion() const NANOEM_DECL_NOEXCEPT
{
    return m_cameraMotionPtr;
}

Motion *
Project::cameraMotion() NANOEM_DECL_NOEXCEPT
{
    return m_cameraMotionPtr;
}

const Motion *
Project::lightMotion() const NANOEM_DECL_NOEXCEPT
{
    return m_lightMotionPtr;
}

Motion *
Project::lightMotion() NANOEM_DECL_NOEXCEPT
{
    return m_lightMotionPtr;
}

const Motion *
Project::selfShadowMotion() const NANOEM_DECL_NOEXCEPT
{
    return m_selfShadowMotionPtr;
}

Motion *
Project::selfShadowMotion() NANOEM_DECL_NOEXCEPT
{
    return m_selfShadowMotionPtr;
}

const undo_stack_t *
Project::activeUndoStack() const NANOEM_DECL_NOEXCEPT
{
    const undo_stack_t *stack;
    if (const Model *model = activeModel()) {
        stack = model->activeUndoStack();
    }
    else {
        stack = undoStack();
    }
    return stack;
}

undo_stack_t *
Project::activeUndoStack() NANOEM_DECL_NOEXCEPT
{
    undo_stack_t *stack;
    if (Model *model = activeModel()) {
        stack = model->activeUndoStack();
    }
    else {
        stack = undoStack();
    }
    return stack;
}

const undo_stack_t *
Project::undoStack() const NANOEM_DECL_NOEXCEPT
{
    return m_undoStack;
}

undo_stack_t *
Project::undoStack() NANOEM_DECL_NOEXCEPT
{
    return m_undoStack;
}

URI
Project::fileURI() const
{
    return m_fileURI.first;
}

void
Project::setFileURI(const URI &value)
{
    m_fileURI.first = value;
}

FileUtils::TransientPath
Project::transientPath() const
{
    return hasTransientPath() ? m_fileURI.second : FileUtils::TransientPath(m_fileURI.first.absolutePath());
}

bool
Project::hasTransientPath() const NANOEM_DECL_NOEXCEPT
{
    return m_fileURI.second.m_valid;
}

void
Project::setTransientPath(const FileUtils::TransientPath &value)
{
    m_fileURI.second = value;
}

bool
Project::containsIndexOfMaterialToAttachEffect(
    nanoem_u16_t handle, nanoem_rsize_t materialIndex) const NANOEM_DECL_NOEXCEPT
{
    ModelMaterialIndexSet::const_iterator it = m_indicesOfMaterialToAttachEffect.second.find(materialIndex);
    return m_indicesOfMaterialToAttachEffect.first == handle && it != m_indicesOfMaterialToAttachEffect.second.end();
}

void
Project::addIndexOfMaterialToAttachEffect(nanoem_u16_t handle, nanoem_rsize_t materialIndex)
{
    if (m_indicesOfMaterialToAttachEffect.first != handle) {
        clearAllIndicesOfMaterialToAttachEffect(handle);
    }
    m_indicesOfMaterialToAttachEffect.second.insert(materialIndex);
}

void
Project::removeIndexOfMaterialToAttachEffect(nanoem_u16_t handle, nanoem_rsize_t materialIndex)
{
    if (m_indicesOfMaterialToAttachEffect.first == handle) {
        m_indicesOfMaterialToAttachEffect.second.erase(materialIndex);
    }
    else {
        clearAllIndicesOfMaterialToAttachEffect(handle);
    }
}

void
Project::clearAllIndicesOfMaterialToAttachEffect(nanoem_u16_t handle)
{
    m_indicesOfMaterialToAttachEffect.first = handle;
    m_indicesOfMaterialToAttachEffect.second.clear();
    m_drawablesToAttachOffscreenRenderTargetEffect.first = String();
    m_drawablesToAttachOffscreenRenderTargetEffect.second.clear();
}

URI
Project::redoFileURI() const
{
    return m_redoFileURI;
}

void
Project::setRedoFileURI(const URI &value)
{
    m_redoFileURI = value;
}

Vector4SI32
Project::backgroundVideoRect() const NANOEM_DECL_NOEXCEPT
{
    return m_backgroundVideoRect;
}

void
Project::setBackgroundVideoRect(const Vector4SI32 &value)
{
    m_backgroundVideoRect = value;
}

Vector2SI32
Project::deviceScaleMovingCursorPosition() const NANOEM_DECL_NOEXCEPT
{
    return Vector2(m_logicalScaleMovingCursorPosition) * windowDevicePixelRatio();
}

Vector2SI32
Project::logicalScaleMovingCursorPosition() const NANOEM_DECL_NOEXCEPT
{
    return m_logicalScaleMovingCursorPosition;
}

void
Project::setLogicalPixelMovingCursorPosition(const Vector2SI32 &value)
{
    m_logicalScaleMovingCursorPosition = value;
}

nanoem_u32_t
Project::cursorModifiers() const NANOEM_DECL_NOEXCEPT
{
    return m_cursorModifiers;
}

void
Project::setCursorModifiers(nanoem_u32_t value)
{
    m_cursorModifiers = value;
}

Vector4SI32
Project::deviceScaleLastCursorPosition(CursorType type) const NANOEM_DECL_NOEXCEPT
{
    const int index = glm::clamp(int(type), int(kCursorTypeFirstEnum), int(kCursorTypeMaxEnum) - 1);
    const Vector4SI32 &p = m_logicalScaleCursorPositions[index];
    return Vector4SI32(Vector2(p) * windowDevicePixelRatio(), p.z, p.w);
}

Vector4SI32
Project::logicalScaleLastCursorPosition(CursorType type) const NANOEM_DECL_NOEXCEPT
{
    const int index = glm::clamp(int(type), int(kCursorTypeFirstEnum), int(kCursorTypeMaxEnum) - 1);
    return m_logicalScaleCursorPositions[index];
}

bool
Project::isCursorPressed(CursorType type) const NANOEM_DECL_NOEXCEPT
{
    const int index = glm::clamp(int(type), int(kCursorTypeFirstEnum), int(kCursorTypeMaxEnum) - 1);
    return m_logicalScaleCursorPositions[index].z != 0;
}

void
Project::setLogicalPixelLastCursorPosition(CursorType type, const Vector2SI32 &logicalScalePosition, bool pressed)
{
    const nanoem_f32_t seconds = currentLocalFrameIndex() / nanoem_f32_t(baseFPS());
    const int index = glm::clamp(int(type), int(kCursorTypeFirstEnum), int(kCursorTypeMaxEnum) - 1);
    m_logicalScaleCursorPositions[index] = Vector4SI32(logicalScalePosition, pressed, seconds);
}

Vector2SI32
Project::lastScrollDelta() const NANOEM_DECL_NOEXCEPT
{
    return m_scrollDelta;
}

void
Project::setLastScrollDelta(const Vector2SI32 &value)
{
    m_scrollDelta = value;
}

Vector4
Project::logicalScaleUniformedViewportImageRect() const NANOEM_DECL_NOEXCEPT
{
    const Vector4 viewportLayoutRect(logicalScaleUniformedViewportLayoutRect());
    Vector4 viewportImageRect(viewportLayoutRect);
    adjustViewportImageRect(viewportLayoutRect, logicalScaleUniformedViewportImageSize(), viewportImageRect);
    viewportImageRect.x -= m_viewportPadding.x;
    viewportImageRect.y -= m_viewportPadding.y;
    return viewportImageRect;
}

Vector4
Project::deviceScaleUniformedViewportImageRect() const NANOEM_DECL_NOEXCEPT
{
    const Vector4 viewportLayoutRect(deviceScaleUniformedViewportLayoutRect());
    Vector4 viewportImageRect(viewportLayoutRect);
    adjustViewportImageRect(viewportLayoutRect, deviceScaleUniformedViewportImageSize(), viewportImageRect);
    return viewportImageRect;
}

Vector4UI16
Project::logicalScaleUniformedViewportLayoutRect() const NANOEM_DECL_NOEXCEPT
{
    return m_uniformViewportLayoutRect.first;
}

Vector4UI16
Project::deviceScaleUniformedViewportLayoutRect() const NANOEM_DECL_NOEXCEPT
{
    const nanoem_f32_t dpr = viewportDevicePixelRatio(), s = windowDevicePixelRatio() / dpr;
    return Vector4(logicalScaleUniformedViewportLayoutRect()) * dpr * s;
}

Vector2UI16
Project::logicalScaleUniformedViewportImageSize() const NANOEM_DECL_NOEXCEPT
{
    return m_uniformViewportImageSize.first;
}

Vector2UI16
Project::deviceScaleUniformedViewportImageSize() const NANOEM_DECL_NOEXCEPT
{
    const nanoem_f32_t dpr = viewportDevicePixelRatio(), s = windowDevicePixelRatio() / dpr;
    return Vector2(m_uniformViewportImageSize.first) * dpr * s;
}

Vector2UI16
Project::logicalViewportPadding() const NANOEM_DECL_NOEXCEPT
{
    return m_viewportPadding;
}

void
Project::setLogicalViewportPadding(const Vector2UI16 &value)
{
    m_viewportPadding = value;
}

Vector2SI32
Project::resolveLogicalCursorPositionInViewport(const Vector2SI32 &value) const NANOEM_DECL_NOEXCEPT
{
    const Vector2UI16 size(m_uniformViewportImageSize.first),
        offset(Vector2UI16(m_uniformViewportLayoutRect.first) + m_viewportPadding),
        coord(value.x - offset.x, size.y - (value.y - offset.y));
    return coord;
}

Vector4
Project::viewportBackgroundColor() const NANOEM_DECL_NOEXCEPT
{
    return m_viewportBackgroundColor;
}

void
Project::setViewportBackgroundColor(const Vector4 &value)
{
    m_viewportBackgroundColor = value;
}

nanoem_f64_t
Project::currentUptimeSeconds() const NANOEM_DECL_NOEXCEPT
{
    return m_uptime.first;
}

nanoem_f64_t
Project::elapsedUptimeSeconds() const NANOEM_DECL_NOEXCEPT
{
    return m_uptime.second;
}

void
Project::setUptimeSeconds(nanoem_f64_t value)
{
    m_uptime.second = value - m_uptime.first;
    m_uptime.first = value;
}

nanoem_f32_t
Project::deviceScaleViewportScaleFactor() const NANOEM_DECL_NOEXCEPT
{
    return windowDevicePixelRatio() / viewportDevicePixelRatio();
}

nanoem_f32_t
Project::windowDevicePixelRatio() const NANOEM_DECL_NOEXCEPT
{
    return m_windowDevicePixelRatio.first;
}

nanoem_f32_t
Project::pendingWindowDevicePixelRatio() const NANOEM_DECL_NOEXCEPT
{
    return m_windowDevicePixelRatio.second;
}

void
Project::setWindowDevicePixelRatio(nanoem_f32_t value)
{
    if (m_windowDevicePixelRatio.second != value) {
        m_windowDevicePixelRatio.second = value;
        m_eventPublisher->publishSetWindowDevicePixelRatioEvent(value);
        EnumUtils::setEnabled(kResetAllPasses, m_stateFlags, true);
    }
}

nanoem_f32_t
Project::viewportDevicePixelRatio() const NANOEM_DECL_NOEXCEPT
{
    return m_viewportDevicePixelRatio.first;
}

void
Project::setViewportDevicePixelRatio(nanoem_f32_t value)
{
    if (m_viewportDevicePixelRatio.second != value) {
        m_viewportDevicePixelRatio.second = value;
        m_eventPublisher->publishSetViewportDevicePixelRatioEvent(value);
        EnumUtils::setEnabled(kResetAllPasses, m_stateFlags, true);
    }
}

nanoem_f32_t
Project::logicalScaleCircleRadius() const NANOEM_DECL_NOEXCEPT
{
    return m_circleRadius;
}

nanoem_f32_t
Project::deviceScaleCircleRadius() const NANOEM_DECL_NOEXCEPT
{
    return logicalScaleCircleRadius() * windowDevicePixelRatio();
}

void
Project::setCircleRadius(nanoem_f32_t value)
{
    m_circleRadius = value;
}

const PhysicsEngine *
Project::physicsEngine() const NANOEM_DECL_NOEXCEPT
{
    return m_physicsEngine;
}

PhysicsEngine *
Project::physicsEngine()
{
    return m_physicsEngine;
}

nanoem_motion_bone_keyframe_interpolation_type_t
Project::boneKeyframeInterpolationType() const NANOEM_DECL_NOEXCEPT
{
    return m_boneInterpolationType;
}

void
Project::setBoneKeyframeInterpolationType(nanoem_motion_bone_keyframe_interpolation_type_t value)
{
    m_boneInterpolationType = value;
}

nanoem_motion_camera_keyframe_interpolation_type_t
Project::cameraKeyframeInterpolationType() const NANOEM_DECL_NOEXCEPT
{
    return m_cameraInterpolationType;
}

void
Project::setCameraKeyframeInterpolationType(nanoem_motion_camera_keyframe_interpolation_type_t value)
{
    m_cameraInterpolationType = value;
}

IDrawable::DrawType
Project::drawType() const NANOEM_DECL_NOEXCEPT
{
    return m_drawType;
}

void
Project::setDrawType(IDrawable::DrawType value)
{
    m_drawType = value >= IDrawable::kDrawTypeFirstEnum && value < IDrawable::kDrawTypeMaxEnum
        ? value
        : IDrawable::kDrawTypeColor;
}

sg_pass
Project::registerRenderPass(const sg_pass_desc &desc, const PixelFormat &format)
{
    RenderPassBundle *descPtrRef = nullptr;
    return registerRenderPass(desc, format, descPtrRef);
}

sg_pass
Project::registerOffscreenRenderPass(const Effect *ownerEffect, const effect::OffscreenRenderTargetOption &option)
{
    sg_pass_desc pd;
    char label[Inline::kMarkerStringLength];
    RenderPassBundle *descPtrRef = nullptr;
    option.getPassDescription(pd);
    if (Inline::isDebugLabelEnabled()) {
        StringUtils::format(label, sizeof(label), "Effects/%s/OffscreenRenderTarget/%s/Pass",
            ownerEffect->nameConstString(), option.m_name.c_str());
        pd.label = label;
    }
    PixelFormat format;
    format.setColorPixelFormat(option.m_colorImageDescription.pixel_format, 0);
    format.setDepthPixelFormat(option.m_depthStencilImageDescription.pixel_format);
    format.setNumColorAttachemnts(countColorAttachments(pd));
    format.setNumSamples(option.m_colorImageDescription.sample_count);
    const sg_pass pass = registerRenderPass(pd, format, descPtrRef);
    descPtrRef->m_colorImage = option.m_colorImage;
    descPtrRef->m_depthImage = option.m_depthStencilImage;
    return pass;
}

void
Project::overrideOffscreenRenderPass(const sg_pass_desc &desc, const PixelFormat &format)
{
    sg_pass pass = registerRenderPass(desc, format);
    m_currentOffscreenRenderPass = pass;
}

sg_pass
Project::currentRenderPass() const NANOEM_DECL_NOEXCEPT
{
    return m_currentRenderPass;
}

sg_pass
Project::currentOffscreenRenderPass() const NANOEM_DECL_NOEXCEPT
{
    return m_currentOffscreenRenderPass;
}

bool
Project::isCurrentRenderPassActive() const NANOEM_DECL_NOEXCEPT
{
    return sg::is_valid(m_currentRenderPass);
}

bool
Project::isOffscreenRenderPassActive() const NANOEM_DECL_NOEXCEPT
{
    return sg::is_valid(m_currentOffscreenRenderPass);
}

bool
Project::isRenderPassViewport() const NANOEM_DECL_NOEXCEPT
{
    return !isCurrentRenderPassActive() && !isOffscreenRenderPassActive();
}

sg_pass
Project::lastDrawnRenderPass() const NANOEM_DECL_NOEXCEPT
{
    return m_lastDrawnRenderPass;
}

sg_pass
Project::scriptExternalRenderPass() const NANOEM_DECL_NOEXCEPT
{
    return m_scriptExternalRenderPass;
}

void
Project::setCurrentRenderPass(sg_pass value)
{
    if (value.id != m_currentRenderPass.id) {
        SG_INSERT_MARKERF("Project::resetCurrentRenderPass(id=%d, name=%s)", value, findRenderPassName(value));
        m_currentRenderPass = value;
    }
}

void
Project::resetCurrentRenderPass()
{
    if (sg::is_valid(m_currentRenderPass)) {
        SG_INSERT_MARKER("Project::resetCurrentRenderPass");
        m_currentRenderPass = { SG_INVALID_ID };
    }
}

void
Project::setScriptExternalRenderPass(sg_pass value, const Vector4 &clearColor, nanoem_f32_t clearDepth)
{
    SG_INSERT_MARKERF("Project::setScriptExternalRenderPass(id=%d, name=%s)", value, findRenderPassName(value));
    if (!sg::is_valid(m_scriptExternalRenderPass)) {
        RenderPassBundleMap::const_iterator it = m_renderPassBundleMap.find(value.id);
        if (it != m_renderPassBundleMap.end()) {
            const RenderPassBundle &pd = it->second;
            sg_pass_action pa;
            Inline::clearZeroMemory(pa);
            pa.colors[0].action = pa.depth.action = SG_ACTION_CLEAR;
            memcpy(&pa.colors[0].value, glm::value_ptr(clearColor), sizeof(pa.colors[0].value));
            pa.depth.value = clearDepth;
            const PixelFormat format(findRenderPassPixelFormat(value, pd.m_format.numSamples()));
            m_renderPassCleaner->clear(sharedBatchDrawQueue(), value, pa, format);
        }
    }
    setCurrentRenderPass(value);
    m_scriptExternalRenderPass = value;
}

void
Project::resetScriptExternalRenderPass()
{
    if (sg::is_valid(m_scriptExternalRenderPass)) {
        SG_INSERT_MARKER("Project::resetScriptExternalRenderPass");
        m_scriptExternalRenderPass = { SG_INVALID_ID };
    }
    resetCurrentRenderPass();
}

void
Project::createAllOffscreenRenderTargets(Effect *ownerEffect, const IncludeEffectSourceMap &includeEffectSources,
    bool enableEffectPlugin, bool enableSourceCache, Progress &progress, Error &error)
{
    static const String kHideLiteral = "hide";
    static const String kNoneLiteral = "none";
    nanoem_parameter_assert(ownerEffect, "must NOT be nullptr");
    SG_PUSH_GROUPF("Project::createAllOffscreenRenderTargets(owner=%s)", ownerEffect->nameConstString());
    const sg_pass lastViewIndex(currentRenderPass());
    NamedOffscreenRenderTargetConditionListMap &namedOffscreenRenderTargets = m_allOffscreenRenderTargets[ownerEffect];
    effect::OffscreenRenderTargetOptionList options;
    SortedOffscreenRenderTargetOptionList sorted;
    getAllOffscreenRenderTargetOptions(ownerEffect, options, sorted);
    for (SortedOffscreenRenderTargetOptionList::const_iterator it = sorted.begin(), end = sorted.end(); it != end;
         ++it) {
        const effect::OffscreenRenderTargetOption *option = *it;
        resetOffscreenRenderTarget(ownerEffect, *option);
        const StringPairList &conditions = option->m_conditions;
        OffscreenRenderTargetConditionList newConditions;
        for (StringPairList::const_iterator it2 = conditions.begin(), end2 = conditions.end(); it2 != end2; ++it2) {
            const StringPair &condition = *it2;
            const String &filename = condition.second;
            const bool isHidden = filename == kHideLiteral, isNone = filename == kNoneLiteral;
            if (isHidden || isNone) {
                const String &pattern = condition.first;
                OffscreenRenderTargetCondition cond;
                cond.m_pattern = pattern;
                cond.m_passiveEffect = nullptr;
                cond.m_hidden = isHidden;
                cond.m_none = isNone;
                newConditions.push_back(cond);
            }
            else {
                loadOffscreenRenderTargetEffect(ownerEffect, includeEffectSources, condition, enableEffectPlugin,
                    enableSourceCache, newConditions, progress, error);
            }
        }
        namedOffscreenRenderTargets.insert(tinystl::make_pair(option->m_name, newConditions));
    }
    if (!sorted.empty()) {
        setCurrentRenderPass(lastViewIndex);
    }
    SG_POP_GROUP();
}

void
Project::releaseAllOffscreenRenderTarget(Effect *ownerEffect)
{
    nanoem_parameter_assert(ownerEffect, "must NOT be nullptr");
    SG_PUSH_GROUPF("Project::releaseAllOffscreenRenderTarget(owner=%s, size=%d)", ownerEffect->nameConstString(),
        m_allOffscreenRenderTargets.size());
    OffscreenRenderTargetConditionListMap::iterator it = m_allOffscreenRenderTargets.find(ownerEffect);
    if (it != m_allOffscreenRenderTargets.end()) {
        NamedOffscreenRenderTargetConditionListMap &renderTargets = it->second;
        for (NamedOffscreenRenderTargetConditionListMap::iterator it2 = renderTargets.begin(),
                                                                  end2 = renderTargets.end();
             it2 != end2; ++it2) {
            const String &name = it2->first;
            OffscreenRenderTargetConditionList &conditionList = it2->second;
            for (DrawableList::const_iterator it = m_drawableOrderList.begin(), end = m_drawableOrderList.end();
                 it != end; ++it) {
                IDrawable *drawable = *it;
                drawable->removeOffscreenPassiveRenderTargetEffect(name);
            }
            for (OffscreenRenderTargetConditionList::iterator it3 = conditionList.begin(), end3 = conditionList.end();
                 it3 != end3; ++it3) {
                OffscreenRenderTargetCondition &condition = *it3;
                LoadedEffectSet::const_iterator it4 = m_loadedEffectSet.find(condition.m_passiveEffect);
                if (it4 != m_loadedEffectSet.end()) {
                    destroyEffect(*it4);
                }
            }
        }
        m_allOffscreenRenderTargets.erase(it);
    }
    OffscreenRenderTargetEffectSetMap::iterator it2 = m_allOffscreenRenderTargetEffectSets.find(ownerEffect);
    if (it2 != m_allOffscreenRenderTargetEffectSets.end()) {
        m_allOffscreenRenderTargetEffectSets.erase(it2);
    }
    SG_POP_GROUP();
}

void
Project::getAllOffscreenRenderTargetEffects(const IEffect *ownerEffect, LoadedEffectSet &allRenderTargetEffects) const
{
    OffscreenRenderTargetEffectSetMap::const_iterator it = m_allOffscreenRenderTargetEffectSets.find(ownerEffect);
    allRenderTargetEffects.clear();
    if (it != m_allOffscreenRenderTargetEffectSets.end()) {
        allRenderTargetEffects = it->second;
    }
}

Vector2UI16
Project::deviceScaleViewportPrimaryImageSize() const NANOEM_DECL_NOEXCEPT
{
    return Vector2(logicalScaleUniformedViewportImageSize()) * viewportDevicePixelRatio();
}

sg_pass
Project::viewportPrimaryPass() const NANOEM_DECL_NOEXCEPT
{
    return m_viewportPrimaryPass.m_handle;
}

sg_image
Project::viewportPrimaryImage() const NANOEM_DECL_NOEXCEPT
{
    sg_image image = m_viewportPrimaryPass.m_colorImage;
    return sg::is_valid(image) ? image : m_fallbackImage;
}

sg_image
Project::viewportSecondaryImage() const NANOEM_DECL_NOEXCEPT
{
    sg_image image = m_viewportSecondaryPass.m_colorImage;
    return sg::is_valid(image) ? image : m_fallbackImage;
}

sg_image
Project::context2DImage() const NANOEM_DECL_NOEXCEPT
{
    sg_image image = m_context2DPass.m_colorImage;
    return sg::is_valid(image) ? image : m_fallbackImage;
}

sg_pass
Project::context2DPass() const NANOEM_DECL_NOEXCEPT
{
    return m_context2DPass.m_handle;
}

bool
Project::containsDrawableToAttachOffscreenRenderTargetEffect(
    const String &key, IDrawable *value) const NANOEM_DECL_NOEXCEPT
{
    DrawableSet::const_iterator it = m_drawablesToAttachOffscreenRenderTargetEffect.second.find(value);
    return m_drawablesToAttachOffscreenRenderTargetEffect.first == key &&
        it != m_drawablesToAttachOffscreenRenderTargetEffect.second.end();
}

void
Project::addDrawableToAttachOffscreenRenderTargetEffect(const String &key, IDrawable *value)
{
    if (!(m_drawablesToAttachOffscreenRenderTargetEffect.first == key)) {
        clearAllDrawablesToAttachOffscreenRenderTargetEffect(key);
    }
    m_drawablesToAttachOffscreenRenderTargetEffect.second.insert(value);
}

void
Project::removeDrawableToAttachOffscreenRenderTargetEffect(const String &key, IDrawable *value)
{
    if (m_drawablesToAttachOffscreenRenderTargetEffect.first == key) {
        m_drawablesToAttachOffscreenRenderTargetEffect.second.erase(value);
    }
    else {
        clearAllDrawablesToAttachOffscreenRenderTargetEffect(key);
    }
}

void
Project::clearAllDrawablesToAttachOffscreenRenderTargetEffect(const String &key)
{
    m_drawablesToAttachOffscreenRenderTargetEffect.first = key;
    m_drawablesToAttachOffscreenRenderTargetEffect.second.clear();
    m_indicesOfMaterialToAttachEffect.first = bx::kInvalidHandle;
    m_indicesOfMaterialToAttachEffect.second.clear();
}

effect::RenderTargetColorImageContainer *
Project::findSharedRenderTargetImageContainer(const String &name, const IEffect *effect) const
{
    effect::RenderTargetColorImageContainer *containerPtr = nullptr;
    if (!name.empty()) {
        SharedRenderTargetImageContainerMap::const_iterator it = m_sharedRenderTargetImageContainers.find(name);
        if (it != m_sharedRenderTargetImageContainers.end()) {
            const SharedRenderTargetImageContainer &item = it->second;
            if (!effect || item.m_parent == effect) {
                containerPtr = item.m_container;
            }
        }
    }
    return containerPtr;
}

effect::RenderTargetColorImageContainer *
Project::findSharedRenderTargetImageContainer(sg_image handle, const IEffect *effect) const NANOEM_DECL_NOEXCEPT
{
    effect::RenderTargetColorImageContainer *containerPtr = nullptr;
    for (SharedRenderTargetImageContainerMap::const_iterator it = m_sharedRenderTargetImageContainers.begin(),
                                                             end = m_sharedRenderTargetImageContainers.end();
         it != end; ++it) {
        const SharedRenderTargetImageContainer &item = it->second;
        if (!effect || item.m_parent == effect) {
            effect::RenderTargetColorImageContainer *container = item.m_container;
            if (container->colorImageHandle().id == handle.id) {
                containerPtr = item.m_container;
                break;
            }
        }
    }
    return containerPtr;
}

effect::RenderPassScope *
Project::offscreenRenderPassScope() NANOEM_DECL_NOEXCEPT
{
    return m_offscreenRenderPassScope;
}

int
Project::countSharedRenderTargetImageContainer(const String &name, const IEffect *effect) const
{
    int count = 0;
    if (!name.empty()) {
        SharedRenderTargetImageContainerMap::const_iterator it = m_sharedRenderTargetImageContainers.find(name);
        if (it != m_sharedRenderTargetImageContainers.end()) {
            const SharedRenderTargetImageContainer &item = it->second;
            if (!effect || item.m_parent == effect) {
                count = item.m_count;
            }
        }
    }
    return count;
}

void
Project::setSharedRenderTargetImageContainer(
    const String &name, const IEffect *parent, effect::RenderTargetColorImageContainer *value)
{
    SharedRenderTargetImageContainerMap::iterator it = m_sharedRenderTargetImageContainers.find(name);
    if (it != m_sharedRenderTargetImageContainers.end()) {
        it->second.m_count++;
    }
    else {
        SharedRenderTargetImageContainer container;
        container.m_parent = parent;
        container.m_container = value;
        container.m_count = 1;
        m_sharedRenderTargetImageContainers.insert(tinystl::make_pair(name, container));
    }
}

void
Project::removeAllSharedRenderTargetImageContainers(const IEffect *parent)
{
    StringList targets;
    for (SharedRenderTargetImageContainerMap::iterator it = m_sharedRenderTargetImageContainers.begin(),
                                                       end = m_sharedRenderTargetImageContainers.end();
         it != end; ++it) {
        if (it->second.m_parent == parent) {
            targets.push_back(it->first);
        }
    }
    SG_PUSH_GROUPF("Project::removeAllSharedRenderTargetImageContainers(size=%d)", targets.size());
    for (StringList::const_iterator it = targets.begin(), end = targets.end(); it != end; ++it) {
        SharedRenderTargetImageContainerMap::const_iterator it2 = m_sharedRenderTargetImageContainers.find(*it);
        if (it2 != m_sharedRenderTargetImageContainers.end()) {
            m_sharedRenderTargetImageContainers.erase(it2);
        }
    }
    SG_POP_GROUP();
}

const ITrack *
Project::selectedTrack() const NANOEM_DECL_NOEXCEPT
{
    return m_selectedTrack;
}

ITrack *
Project::selectedTrack() NANOEM_DECL_NOEXCEPT
{
    return m_selectedTrack;
}

void
Project::setSelectedTrack(ITrack *value)
{
    m_selectedTrack = value;
}

Project::EditingMode
Project::editingMode() const NANOEM_DECL_NOEXCEPT
{
    return m_editingMode;
}

void
Project::setEditingMode(EditingMode value)
{
    if (value != m_editingMode) {
        if (value >= kEditingModeFirstEnum && value < kEditingModeMaxEnum) {
            Model *model = activeModel();
            if (model && value != kEditingModeSelect) {
                model->markStagingVertexBufferDirty();
            }
            m_editingMode = value;
        }
        else {
            m_editingMode = kEditingModeNone;
        }
        eventPublisher()->publishSetEditingModeEvent(static_cast<nanoem_u32_t>(m_editingMode));
    }
}

Project::FilePathMode
Project::filePathMode() const NANOEM_DECL_NOEXCEPT
{
    return m_filePathMode;
}

void
Project::setFilePathMode(FilePathMode value)
{
    m_filePathMode = value;
}

bool
Project::isPhysicsSimulationEnabled() const NANOEM_DECL_NOEXCEPT
{
    switch (m_physicsEngine->simulationMode()) {
    case PhysicsEngine::kSimulationModeDisable:
    case PhysicsEngine::kSimulationModeMaxEnum:
    default:
        return false;
    case PhysicsEngine::kSimulationModeEnableAnytime:
    case PhysicsEngine::kSimulationModeEnableTracing:
        return true;
    case PhysicsEngine::kSimulationModeEnablePlaying:
        return isPlaying();
    }
}

void
Project::setPhysicsSimulationMode(PhysicsEngine::SimulationModeType value)
{
    if (m_physicsEngine->simulationMode() != value) {
        m_physicsEngine->setDebugGeometryFlags(value ? m_lastPhysicsDebugFlags : 0);
        m_physicsEngine->setSimulationMode(value);
        resetPhysicsSimulation();
        restart(currentLocalFrameIndex());
        eventPublisher()->publishSetPhysicsSimulationModeEvent(static_cast<nanoem_u32_t>(value));
    }
}

ITranslator::LanguageType
Project::language() const NANOEM_DECL_NOEXCEPT
{
    return m_language;
}

nanoem_language_type_t
Project::castLanguage() const NANOEM_DECL_NOEXCEPT
{
    switch (m_language) {
    case ITranslator::kLanguageTypeJapanese:
    case ITranslator::kLanguageTypeKorean:
    case ITranslator::kLanguageTypeChineseSimplified:
    case ITranslator::kLanguageTypeChineseTraditional:
        return NANOEM_LANGUAGE_TYPE_JAPANESE;
    default:
        return NANOEM_LANGUAGE_TYPE_ENGLISH;
    }
}

void
Project::setLanguage(ITranslator::LanguageType value)
{
    if (value != m_language) {
        Accessory *lastActiveAccessory = activeAccessory();
        Model *lastActiveModel = activeModel();
        if (value >= ITranslator::kLanguageTypeFirstEnum && value < ITranslator::kLanguageTypeMaxEnum) {
            m_language = value;
        }
        else {
            m_language = ITranslator::kLanguageTypeFirstEnum;
        }
        m_translator->setLanguage(value);
        for (ModelList::const_iterator it = m_allModelPtrs.begin(), end = m_allModelPtrs.end(); it != end; ++it) {
            Model *model = *it;
            model->resetLanguage();
        }
        rebuildAllTracks();
        eventPublisher()->publishSetLanguageEvent(static_cast<nanoem_u32_t>(m_language));
        m_activeAccessoryPtr = nullptr;
        m_activeModelPairPtr.first = nullptr;
        setActiveAccessory(lastActiveAccessory);
        setActiveModel(lastActiveModel);
    }
}

Vector2UI16
Project::shadowMapSize() const NANOEM_DECL_NOEXCEPT
{
    return m_shadowCamera->imageSize();
}

void
Project::setShadowMapSize(const Vector2UI16 &value)
{
    if (m_shadowCamera->imageSize() != value) {
        m_shadowCamera->resize(value);
        EnumUtils::setEnabled(kResetAllPasses, m_stateFlags, true);
    }
}

Vector2UI16
Project::viewportImageSize() const NANOEM_DECL_NOEXCEPT
{
    return m_viewportImageSize;
}

void
Project::setViewportImageSize(const Vector2UI16 &value)
{
    if (m_viewportImageSize != value) {
        internalResizeUniformedViewportImage(value);
        m_viewportImageSize = value;
    }
}

sg_pixel_format
Project::viewportPixelFormat() const NANOEM_DECL_NOEXCEPT
{
    return m_viewportPixelFormat.first;
}

void
Project::setViewportPixelFormat(sg_pixel_format value)
{
    if (viewportPixelFormat() != value) {
        m_viewportPixelFormat.second = value;
        EnumUtils::setEnabled(kResetAllPasses, m_stateFlags, true);
    }
}

Project::TransformPerformIndex
Project::transformPerformedAt() const NANOEM_DECL_NOEXCEPT
{
    return m_transformPerformedAt;
}

void
Project::setTransformPerformedAt(const TransformPerformIndex &value)
{
    m_transformPerformedAt = value;
}

void
Project::updateTransformPerformedAt(const TransformPerformIndex &value)
{
    if (transformPerformedAt().first == Motion::kMaxFrameIndex) {
        setTransformPerformedAt(value);
    }
}

void
Project::resetTransformPerformedAt()
{
    setTransformPerformedAt(tinystl::make_pair(Motion::kMaxFrameIndex, 0));
}

nanoem_f32_t
Project::timeStepFactor() const NANOEM_DECL_NOEXCEPT
{
    return m_timeStepFactor;
}

void
Project::setTimeStepFactor(nanoem_f32_t value)
{
    m_timeStepFactor = value;
}

nanoem_f32_t
Project::backgroundVideoScaleFactor() const NANOEM_DECL_NOEXCEPT
{
    return m_backgroundVideoScaleFactor;
}

void
Project::setBackgroundVideoScaleFactor(nanoem_f32_t value)
{
    m_backgroundVideoScaleFactor = value;
}

nanoem_f32_t
Project::physicsSimulationTimeStep() const NANOEM_DECL_NOEXCEPT
{
    return invertedPreferredMotionFPS() * m_preferredMotionFPS.m_scaleFactor * m_timeStepFactor;
}

nanoem_u32_t
Project::baseFPS() const NANOEM_DECL_NOEXCEPT
{
    return Constants::kHalfBaseFPS;
}

nanoem_u32_t
Project::preferredMotionFPS() const NANOEM_DECL_NOEXCEPT
{
    return m_preferredMotionFPS.m_value;
}

nanoem_f32_t
Project::invertedPreferredMotionFPS() const NANOEM_DECL_NOEXCEPT
{
    return m_preferredMotionFPS.m_invertedValue;
}

void
Project::setPreferredMotionFPS(nanoem_u32_t value, bool unlimited)
{
    if (m_preferredMotionFPS != value || unlimited != isDisplaySyncDisabled()) {
        m_preferredMotionFPS = glm::min(value, kTimeBasedAudioSourceDefaultSampleRate);
        EnumUtils::setEnabled(kDisableDisplaySync, m_stateFlags, unlimited);
        eventPublisher()->publishSetPreferredMotionFPSEvent(value, unlimited);
    }
}

nanoem_u32_t
Project::preferredEditingFPS() const NANOEM_DECL_NOEXCEPT
{
    return m_editingFPS;
}

void
Project::setPreferredEditingFPS(nanoem_u32_t value)
{
    if (m_editingFPS != value) {
        switch (value) {
        case 0:
        case 30:
        case 60:
        case UINT32_MAX:
            m_editingFPS = value;
            break;
        default:
            m_editingFPS = value = 0;
        }
        eventPublisher()->publishSetPreferredEditingFPSEvent(value);
    }
}

nanoem_u32_t
Project::actualFPS() const NANOEM_DECL_NOEXCEPT
{
    return m_actualFPS;
}

void
Project::setActualFPS(nanoem_u32_t value)
{
    m_actualFPS = value;
}

void
Project::setPhysicsSimulationEngineDebugFlags(nanoem_u32_t value)
{
    if (m_physicsEngine->debugGeometryFlags() != value) {
        for (ModelList::const_iterator it = m_allModelPtrs.begin(), end = m_allModelPtrs.end(); it != end; ++it) {
            Model *model = *it;
            model->setRigidBodiesVisualization(m_rigidBodyVisualizationClause);
        }
        m_physicsEngine->setDebugGeometryFlags(glm::max(value, 0u));
        m_physicsEngine->stepSimulation(0);
        m_lastPhysicsDebugFlags = value;
        eventPublisher()->publishSetPhysicsSimulationEngineDebugFlagEvent(value);
    }
}

nanoem_u32_t
Project::coordinationSystem() const NANOEM_DECL_NOEXCEPT
{
    return m_coordinationSystem;
}

void
Project::setCoordinationSystem(nanoem_u32_t value)
{
    m_coordinationSystem = value;
}

int
Project::maxAnisotropyValue() const NANOEM_DECL_NOEXCEPT
{
    return isAnisotropyEnabled() ? 16 : 1;
}

int
Project::sampleCount() const NANOEM_DECL_NOEXCEPT
{
    return 1 << m_sampleLevel.first;
}

nanoem_u32_t
Project::sampleLevel() const NANOEM_DECL_NOEXCEPT
{
    return m_sampleLevel.first;
}

void
Project::setSampleLevel(nanoem_u32_t value)
{
    if (!m_rendererCapability->supportsSampleLevel(value)) {
        value = m_rendererCapability->suggestedSampleLevel(value);
    }
    if (value != sampleLevel()) {
        m_sampleLevel.second = value;
        EnumUtils::setEnabled(kResetAllPasses, m_stateFlags, true);
        eventPublisher()->publishSetProjectSampleLevelEvent(value);
    }
}

bool
Project::isResetAllPassesPending() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kResetAllPasses, m_stateFlags);
}

bool
Project::isDisplaySyncDisabled() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kDisableDisplaySync, m_stateFlags);
}

bool
Project::isHiddenBoneBoundsRigidBodyDisabled() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kDisableHiddenBoneBoundsRigidBody, m_stateFlags);
}

void
Project::setHiddenBoneBoundsRigidBodyDisabled(bool value)
{
    if (value != isHiddenBoneBoundsRigidBodyDisabled()) {
        for (ModelList::const_iterator it = m_allModelPtrs.begin(), end = m_allModelPtrs.end(); it != end; ++it) {
            Model *model = *it;
            model->clearAllBoneBoundsRigidBodies();
            if (!value) {
                model->createAllBoneBoundsRigidBodies();
            }
        }
        EnumUtils::setEnabled(kDisableHiddenBoneBoundsRigidBody, m_stateFlags, value);
    }
}

bool
Project::isLoopEnabled() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kEnableLoop, m_stateFlags);
}

void
Project::setLoopEnabled(bool value)
{
    if (isLoopEnabled() != value) {
        EnumUtils::setEnabled(kEnableLoop, m_stateFlags, value);
        eventPublisher()->publishToggleProjectPlayingWithLoopEnabledEvent(value);
    }
}

bool
Project::isCameraShared() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kEnableSharedCamera, m_stateFlags);
}

void
Project::setCameraShared(bool value)
{
    if (isCameraShared() != value) {
        EnumUtils::setEnabled(kEnableSharedCamera, m_stateFlags, value);
    }
}

bool
Project::isGroundShadowEnabled() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kEnableGroundShadow, m_stateFlags);
}

void
Project::setGroundShadowEnabled(bool value)
{
    if (isGroundShadowEnabled() != value) {
        EnumUtils::setEnabled(kEnableGroundShadow, m_stateFlags, value);
        eventPublisher()->publishToggleProjectGroundShadowEnabledEvent(value);
    }
}

bool
Project::isShadowMapEnabled() const NANOEM_DECL_NOEXCEPT
{
    return m_shadowCamera->isEnabled();
}

void
Project::setShadowMapEnabled(bool value)
{
    bool wasEnabled = isShadowMapEnabled();
    if (value) {
        m_shadowCamera->setEnabled(true);
        m_shadowCamera->resize(m_shadowCamera->imageSize());
        const sg_pass shadowPass = m_shadowCamera->pass();
        RenderPassBundle &d = m_renderPassBundleMap[shadowPass.id];
        d.m_colorImage = m_shadowCamera->colorImage();
        d.m_depthImage = m_shadowCamera->depthImage();
        EnumUtils::setEnabled(kResetAllPasses, m_stateFlags, true);
    }
    else {
        m_shadowCamera->setEnabled(false);
    }
    if (wasEnabled != value) {
        eventPublisher()->publishToggleShadowMapEnabledEvent(value);
    }
}

bool
Project::isMultipleBoneSelectionEnabled() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kEnableMultipleBoneSelection, m_stateFlags);
}

void
Project::setMultipleBoneSelectionEnabled(bool value)
{
    if (isMultipleBoneSelectionEnabled() != value) {
        EnumUtils::setEnabled(kEnableMultipleBoneSelection, m_stateFlags, value);
    }
}

bool
Project::isTransformHandleVisible() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kDisplayTransformHandle, m_stateFlags);
}

void
Project::setTransformHandleVisible(bool value)
{
    if (isTransformHandleVisible() != value) {
        EnumUtils::setEnabled(kDisplayTransformHandle, m_stateFlags, value);
    }
}

bool
Project::isBezierCurveAjustmentEnabled() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kEnableBezierCurveAdjustment, m_stateFlags);
}

void
Project::setBezierCurveAdjustmentEnabled(bool value)
{
    if (isBezierCurveAjustmentEnabled() != value) {
        EnumUtils::setEnabled(kEnableBezierCurveAdjustment, m_stateFlags, value);
    }
}

bool
Project::isMotionMergeEnabled() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kEnableMotionMerge, m_stateFlags);
}

void
Project::setMotionMergeEnabled(bool value)
{
    if (isMotionMergeEnabled() != value) {
        EnumUtils::setEnabled(kEnableMotionMerge, m_stateFlags, value);
    }
}

bool
Project::isEffectPluginEnabled() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kEnableEffectPlugin, m_stateFlags);
}

void
Project::setEffectPluginEnabled(bool value)
{
    if (isEffectPluginEnabled() != value) {
        EnumUtils::setEnabled(kEnableEffectPlugin, m_stateFlags, value);
        for (EffectReferenceMap::const_iterator it = m_effectReferences.begin(), end = m_effectReferences.end();
             it != end; ++it) {
            Effect *effect = it->second.first;
            effect->setEnabled(value);
        }
        Error error;
        Nanoem__Application__SetEffectPluginEnabledCommand base =
            NANOEM__APPLICATION__SET_EFFECT_PLUGIN_ENABLED_COMMAND__INIT;
        base.value = value;
        Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
        command.timestamp = stm_now();
        command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SET_EFFECT_PLUGIN_ENABLED;
        command.set_effect_plugin_enabled = &base;
        writeRedoMessage(&command, error);
        error.notify(eventPublisher());
        eventPublisher()->publishToggleProjectEffectEnabledEvent(value);
    }
}

bool
Project::isCompiledEffectCacheEnabled() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kEnableCompiledEffectCache, m_stateFlags);
}

void
Project::setCompiledEffectCacheEnabled(bool value)
{
    if (isCompiledEffectCacheEnabled() != value) {
        EnumUtils::setEnabled(kEnableCompiledEffectCache, m_stateFlags, value);
    }
}

bool
Project::isViewportCaptured() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kEnableViewportLocked, m_stateFlags);
}

void
Project::setViewportCaptured(bool value)
{
    if (isViewportCaptured() != value) {
        EnumUtils::setEnabled(kEnableViewportLocked, m_stateFlags, value);
    }
}

bool
Project::isViewportHovered() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kViewportHovered, m_stateFlags);
}

void
Project::setViewportHovered(bool value)
{
    if (isViewportHovered() != value) {
        EnumUtils::setEnabled(kViewportHovered, m_stateFlags, value);
    }
}

bool
Project::isViewportWindowDetached() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kViewportWindowDetached, m_stateFlags);
}

void
Project::setViewportWindowDetached(bool value)
{
    if (isViewportWindowDetached() != value) {
        EnumUtils::setEnabled(kViewportWindowDetached, m_stateFlags, value);
    }
}

bool
Project::isPrimaryCursorTypeLeft() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kPrimaryCursorTypeLeft, m_stateFlags);
}

void
Project::setPrimaryCursorTypeLeft(bool value)
{
    EnumUtils::setEnabled(kPrimaryCursorTypeLeft, m_stateFlags, value);
}

bool
Project::isPlayingAudioPartEnabled() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kEnablePlayingAudioPart, m_stateFlags);
}

void
Project::setPlayingAudioPartEnabled(bool value)
{
    EnumUtils::setEnabled(kEnablePlayingAudioPart, m_stateFlags, value);
}

bool
Project::isViewportWithTransparentEnabled() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kEnableViewportWithTransparent, m_stateFlags);
}

void
Project::setViewportWithTransparentEnabled(bool value)
{
    EnumUtils::setEnabled(kEnableViewportWithTransparent, m_stateFlags, value);
}

bool
Project::isConfirmSeekEnabled(nanoem_mutable_motion_keyframe_type_t type) const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(nanoem_u64_t(type), m_confirmSeekFlags);
}

void
Project::setConfirmSeekEnabled(nanoem_mutable_motion_keyframe_type_t type, bool value)
{
    if (isConfirmSeekEnabled(type) != value) {
        nanoem_u64_t v = m_confirmSeekFlags;
        EnumUtils::setEnabled(nanoem_u64_t(type), v, value);
        m_confirmSeekFlags = v;
    }
}

bool
Project::isCancelRequested() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kCancelRequested, m_stateFlags);
}

void
Project::setCancelRequested(bool value)
{
    EnumUtils::setEnabled(kCancelRequested, m_stateFlags, value);
}

bool
Project::isUniformedViewportImageSizeEnabled() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kEnableUniformedViewportImageSize, m_stateFlags);
}

void
Project::setUniformedViewportImageSizeEnabled(bool value)
{
    EnumUtils::setEnabled(kEnableUniformedViewportImageSize, m_stateFlags, value);
}

bool
Project::isFPSCounterEnabled() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kEnableFPSCounter, m_stateFlags);
}

void
Project::setFPSCounterEnabled(bool value)
{
    EnumUtils::setEnabled(kEnableFPSCounter, m_stateFlags, value);
}

bool
Project::isPerformanceMonitorEnabled() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kEnablePerformanceMonitor, m_stateFlags);
}

void
Project::setPerformanceMonitorEnabled(bool value)
{
    EnumUtils::setEnabled(kEnablePerformanceMonitor, m_stateFlags, value);
}

bool
Project::hasInputTextFocus() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kInputTextFocus, m_stateFlags);
}

void
Project::setInputTextFocus(bool value)
{
    EnumUtils::setEnabled(kInputTextFocus, m_stateFlags, value);
}

bool
Project::isPhysicsSimulationForBoneKeyframeEnabled() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kEnablePhysicsSimulationForBoneKeyframe, m_stateFlags);
}

void
Project::setPhysicsSimulationForBoneKeyframeEnabled(bool value)
{
    EnumUtils::setEnabled(kEnablePhysicsSimulationForBoneKeyframe, m_stateFlags, value);
}

bool
Project::isAnisotropyEnabled() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kEnableImageAnisotropy, m_stateFlags);
}

void
Project::setAnisotropyEnabled(bool value)
{
    EnumUtils::setEnabled(kEnableImageAnisotropy, m_stateFlags, value);
}

bool
Project::isMipmapEnabled() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kEnableImageMipmap, m_stateFlags);
}

void
Project::setMipmapEnabled(bool value)
{
    EnumUtils::setEnabled(kEnableImageMipmap, m_stateFlags, value);
}

bool
Project::isPowerSavingEnabled() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kEnablePowerSaving, m_stateFlags);
}

void
Project::setPowerSavingEnabled(bool value)
{
    EnumUtils::setEnabled(kEnablePowerSaving, m_stateFlags, value);
}

bool
Project::isModelEditingEnabled() const NANOEM_DECL_NOEXCEPT
{
    return EnumUtils::isEnabled(kEnableModelEditing, m_stateFlags);
}

void
Project::setModelEditingEnabled(bool value)
{
    Model *model = activeModel();
    if (model && isModelEditingEnabled() != value) {
        model->rebuildAllVertexBuffers(value ? false : true);
        undoStackClear(model->editingUndoStack());
        EnumUtils::setEnabled(kEnableModelEditing, m_stateFlags, value);
        IEventPublisher *ev = eventPublisher();
        ev->publishToggleModelEditingEnabledEvent(value);
        if (value) {
            ev->publishUndoEvent(false, false);
        }
        else {
            const undo_stack_t *stack = model->undoStack();
            ev->publishUndoEvent(undoStackCanUndo(stack), undoStackCanRedo(stack));
        }
    }
}

bool
Project::isActive() const NANOEM_DECL_NOEXCEPT
{
    return m_active;
}

void
Project::setActive(bool value)
{
    if (m_active != value) {
        if (value) {
            EnumUtils::setEnabled(kResetAllPasses, m_stateFlags, true);
            resume(m_audioPlayer->wasPlaying());
        }
        else {
            const bool playing = m_audioPlayer->isPlaying();
            pause(false);
            if (!playing) {
                m_audioPlayer->suspend();
            }
        }
        m_active = value;
    }
}

Vector4UI16
Project::internalQueryRectangle(RectangleType type, const Vector4UI16 &viewportRect, const Vector2UI16 &offset,
    nanoem_f32_t deviceScaleRatio) NANOEM_DECL_NOEXCEPT
{
    static const int kMarginSize = 5, kSpacingSize = 5;
    const Vector2UI16 viewportOffset(viewportRect.z + offset.x, viewportRect.w + offset.y);
    const float width = 30 * deviceScaleRatio, margin = kMarginSize * deviceScaleRatio, shift = width + margin,
                offsetX = viewportOffset.x - ((shift * 3 + margin)),
                offsetTranslateY = viewportOffset.y - ((width + margin * 2)),
                offsetOrientateY = viewportOffset.y - ((width * 2 + margin * 2 + kSpacingSize)),
                offsetCameraY = int(offset.y) + margin * 2;
    Vector4UI16 rect;
    switch (type) {
    case kRectangleActualFPS: {
        rect = Vector4UI16(offset.x + margin * 2, offsetTranslateY + kSpacingSize, width * 2, width - kSpacingSize);
        break;
    }
    case kRectangleCameraLookAt: {
        rect = Vector4UI16(offsetX + shift * 2, offsetCameraY, width, width);
        break;
    }
    case kRectangleCameraZoom: {
        rect = Vector4UI16(offsetX + shift * 1, offsetCameraY, width, width);
        break;
    }
    case kRectangleOrientateX: {
        rect = Vector4UI16(offsetX + shift * 0, offsetOrientateY, width, width);
        break;
    }
    case kRectangleOrientateY: {
        rect = Vector4UI16(offsetX + shift * 1, offsetOrientateY, width, width);
        break;
    }
    case kRectangleOrientateZ: {
        rect = Vector4UI16(offsetX + shift * 2, offsetOrientateY, width, width);
        break;
    }
    case kRectangleTransformCoordinateType: {
        const float offsetTransformCoordinateTypeY = viewportOffset.y - (width * 3 + margin * 2 + kSpacingSize);
        rect = Vector4UI16(offsetX, offsetTransformCoordinateTypeY, shift * 3 - margin, width - kSpacingSize);
        break;
    }
    case kRectangleTranslateX: {
        rect = Vector4UI16(offsetX + shift * 0, offsetTranslateY, width, width);
        break;
    }
    case kRectangleTranslateY: {
        rect = Vector4UI16(offsetX + shift * 1, offsetTranslateY, width, width);
        break;
    }
    case kRectangleTranslateZ: {
        rect = Vector4UI16(offsetX + shift * 2, offsetTranslateY, width, width);
        break;
    }
    default:
        rect = Vector4UI16(0);
        break;
    }
    return rect;
}

Vector2UI16
Project::uniformedViewportImageSize(
    const Vector2UI16 &viewportLayoutSize, const Vector2UI16 &viewportImageSize) NANOEM_DECL_NOEXCEPT
{
    const Vector2 viewportImageSizeF(viewportImageSize);
    const nanoem_f32_t viewportLayoutBaseRatio = viewportImageSize.x >= viewportImageSize.y
        ? viewportLayoutSize.x / viewportImageSizeF.x
        : viewportLayoutSize.y / viewportImageSizeF.y;
    return viewportImageSizeF * viewportLayoutBaseRatio;
}

void
Project::adjustViewportImageRect(
    const Vector4 viewportLayoutRect, const Vector2 &viewportImageSize, Vector4 &viewportImageRect) NANOEM_DECL_NOEXCEPT
{
    if (viewportLayoutRect.z > viewportImageSize.x) {
        viewportImageRect.x += (viewportLayoutRect.z - viewportImageSize.x) * 0.5f;
        viewportImageRect.z = viewportImageSize.x;
    }
    if (viewportLayoutRect.w > viewportImageSize.y) {
        viewportImageRect.y += ((viewportLayoutRect.w - viewportImageSize.y) * 0.5f);
        viewportImageRect.w = viewportImageSize.y;
    }
}

bool
Project::matchDrawableEffect(const IDrawable *drawable, const Effect *ownerEffect, const String &expr)
{
    bool result = false;
    if (expr == String("self")) {
        result = drawable->activeEffect() == ownerEffect;
    }
    else if (!expr.empty()) {
        const URI &fileURI = drawable->fileURI();
        const String &basePath = fileURI.absolutePathByDeletingLastPathComponent();
        String filename;
        if (fileURI.absolutePath().size() > basePath.size()) {
            filename = fileURI.absolutePathConstString() + basePath.size() + 1;
        }
        else {
            filename = fileURI.absolutePath();
        }
        if (!filename.empty()) {
            result = StringUtils::equalsIgnoreCase(expr.c_str(), filename.c_str());
            if (!result) {
                const String exprLC(StringUtils::toLowerCase(expr)), filenameLC(StringUtils::toLowerCase(filename));
                result = ::wildcardcmp(exprLC.c_str(), filenameLC.c_str()) != 0;
            }
        }
    }
    return result;
}

void
Project::symmetricLocalTransformBone(
    const String &name, const Vector3 &translation, const Quaternion &orientation, Model *model)
{
    if (model::Bone *newBone = model::Bone::cast(model->findBone(name))) {
        newBone->setLocalUserTranslation(Vector3(-translation.x, translation.y, translation.z));
        newBone->setLocalUserOrientation(Quaternion(orientation.w, orientation.x, -orientation.y, -orientation.z));
        newBone->setDirty(true);
    }
}

void
Project::addEffectOrderSet(IDrawable *drawable)
{
    const Effect *effect = resolveEffect(drawable);
    addEffectOrderSet(drawable, effect ? effect->scriptOrder() : IEffect::kScriptOrderTypeStandard);
}

void
Project::addEffectOrderSet(IDrawable *drawable, IEffect::ScriptOrderType order)
{
    removeEffectOrderSet(drawable);
    EffectOrderSet::iterator it = m_effectOrderSet.find(order);
    if (it != m_effectOrderSet.end()) {
        it->second.insert(drawable);
    }
    else {
        DrawableSet drawables;
        drawables.insert(drawable);
        m_effectOrderSet.insert(tinystl::make_pair(order, drawables));
    }
}

void
Project::removeEffectOrderSet(IDrawable *drawable)
{
    for (int i = IEffect::kScriptOrderTypeFirstEnum; i < IEffect::kScriptOrderTypeMaxEnum; i++) {
        IEffect::ScriptOrderType order = static_cast<IEffect::ScriptOrderType>(i);
        EffectOrderSet::iterator it = m_effectOrderSet.find(order);
        if (it != m_effectOrderSet.end()) {
            DrawableSet &drawables = it->second;
            drawables.erase(drawable);
        }
    }
}

void
Project::internalSetDrawableActiveEffect(IDrawable *drawable, Effect *effect,
    const IncludeEffectSourceMap &includeEffectSources, bool enableEffectPlugin, bool enableSourceCache,
    Progress &progress, Error &error)
{
    addLoadedEffectSet(effect);
    drawable->setActiveEffect(effect);
    if (effect->hasScriptExternal()) {
        m_dependsOnScriptExternal.push_back(drawable);
    }
    createAllOffscreenRenderTargets(
        effect, includeEffectSources, enableEffectPlugin, enableSourceCache, progress, error);
    applyDrawableToOffscreenRenderTargetEffect(drawable, effect);
    /* register all drawables with the offscreen render target effect before loading the offscreen effect */
    applyAllDrawablesToOffscreenRenderTargetEffect(drawable, effect);
}

void
Project::loadOffscreenRenderTargetEffect(Effect *ownerEffect, const IncludeEffectSourceMap &includeEffectSources,
    const StringPair &condition, bool enableEffectPlugin, bool enableSourceCache,
    OffscreenRenderTargetConditionList &newConditions, Progress &progress, Error &error)
{
    if (!loadOffscreenRenderTargetEffectFromEffectSourceMap(
            ownerEffect, includeEffectSources, condition, enableEffectPlugin, newConditions, progress, error) &&
        !error.hasReason()) {
        URI sourceURI;
        ByteArray output;
        const String &filename = condition.second;
        String basePath(FileUtils::canonicalizePath(ownerEffect->fileURI().absolutePathByDeletingLastPathComponent(),
            URI::stringByDeletingPathExtension(filename)));
        if (enableEffectPlugin) {
            String effectSourcePath(basePath);
            effectSourcePath.append(".");
            effectSourcePath.append(Effect::kSourceFileExtension);
            const URI &resolvedURI = Effect::resolveSourceURI(m_fileManager, URI::createFromFilePath(effectSourcePath));
            if (!resolvedURI.isEmpty()) {
                Effect *targetEffect = findEffect(resolvedURI);
                if (targetEffect) {
                    OffscreenRenderTargetCondition cond;
                    cond.m_pattern = condition.first;
                    cond.m_passiveEffect = targetEffect;
                    cond.m_hidden = cond.m_none = false;
                    newConditions.push_back(cond);
                    m_allOffscreenRenderTargetEffectSets[ownerEffect].insert(targetEffect);
                    m_loadedEffectSet.insert(targetEffect);
                }
                else {
                    bool hitCache = enableSourceCache && findSourceEffectCache(resolvedURI, output, error);
                    if (hitCache ||
                        Effect::compileFromSource(
                            resolvedURI, m_fileManager, isMipmapEnabled(), output, progress, error)) {
                        targetEffect = createEffect();
                        if (loadOffscreenRenderTargetEffectFromByteArray(
                                targetEffect, resolvedURI, condition, output, newConditions, progress, error)) {
                            bool hasRelativePrefix = StringUtils::equals(filename.c_str(), "./", 2) ||
                                StringUtils::equals(filename.c_str(), "../", 3);
                            targetEffect->setFilename(hasRelativePrefix
                                    ? FileUtils::canonicalizePath(URI::stringByDeletingLastPathComponent(filename),
                                          URI::lastPathComponent(filename))
                                    : filename);
                            m_allOffscreenRenderTargetEffectSets[ownerEffect].insert(targetEffect);
                            m_loadedEffectSet.insert(targetEffect);
                            if (enableSourceCache && !hitCache) {
                                setSourceEffectCache(resolvedURI, output, error);
                            }
                        }
                        else {
                            destroyDetachedEffect(targetEffect);
                        }
                    }
                }
            }
        }
        else {
            String effectBinaryPath(basePath);
            effectBinaryPath.append(".");
            effectBinaryPath.append(Effect::kBinaryFileExtension);
            if (FileUtils::exists(effectBinaryPath.c_str())) {
                FileReaderScope scope(m_translator);
                const URI &fileURI = URI::createFromFilePath(effectBinaryPath);
                if (scope.open(fileURI, error)) {
                    ByteArray bytes;
                    FileUtils::read(scope, bytes, error);
                    Effect *targetEffect = createEffect();
                    if (loadOffscreenRenderTargetEffectFromByteArray(
                            targetEffect, fileURI, condition, bytes, newConditions, progress, error)) {
                        m_allOffscreenRenderTargetEffectSets[ownerEffect].insert(targetEffect);
                        m_loadedEffectSet.insert(targetEffect);
                    }
                    else {
                        destroyDetachedEffect(targetEffect);
                    }
                }
            }
        }
    }
}

bool
Project::loadOffscreenRenderTargetEffectFromEffectSourceMap(const Effect *ownerEffect,
    const IncludeEffectSourceMap &includeEffectSources, const StringPair &condition, bool enableEffectPlugin,
    OffscreenRenderTargetConditionList &newConditions, Progress &progress, Error &error)
{
    const URI &fileURI = ownerEffect->fileURI();
    const String &filename = condition.second;
    IncludeEffectSourceMap::const_iterator it = includeEffectSources.find(filename);
    Effect *targetEffect = nullptr;
    bool loaded = false;
    if (it != includeEffectSources.end() && enableEffectPlugin) {
        if (plugin::EffectPlugin *plugin = m_fileManager->sharedEffectPlugin()) {
            PluginFactory::EffectPluginProxy proxy(plugin);
            const String &extension = URI::pathExtension(filename);
            const StringList &extensionsList = proxy.availableExtensions();
            const ByteArray &input = it->second;
            ByteArray output;
            if (ListUtils::contains(extension, extensionsList)) {
                for (IncludeEffectSourceMap::const_iterator it3 = includeEffectSources.begin(),
                                                            end3 = includeEffectSources.end();
                     it3 != end3; ++it3) {
                    proxy.addIncludeSource(it3->first, it3->second);
                }
                const String originSource(reinterpret_cast<const char *>(input.data()), input.size());
                if (proxy.compile(originSource, output)) {
                    targetEffect = createEffect();
                    const URI &targetEffectFileURI = resolveArchiveURI(fileURI, filename);
                    loaded = loadOffscreenRenderTargetEffectFromByteArray(
                        targetEffect, targetEffectFileURI, condition, output, newConditions, progress, error);
                }
            }
        }
    }
    if (!loaded) {
        String effectBinaryFilename(URI::stringByDeletingPathExtension(filename).c_str());
        effectBinaryFilename.append(".");
        effectBinaryFilename.append(Effect::kBinaryFileExtension);
        IncludeEffectSourceMap::const_iterator it2 = includeEffectSources.find(effectBinaryFilename);
        if (it2 != includeEffectSources.end()) {
            const URI &targetEffectFileURI = resolveArchiveURI(fileURI, effectBinaryFilename);
            targetEffect = createEffect();
            loaded = loadOffscreenRenderTargetEffectFromByteArray(
                targetEffect, targetEffectFileURI, condition, it2->second, newConditions, progress, error);
        }
    }
    if (loaded) {
        m_allOffscreenRenderTargetEffectSets[ownerEffect].insert(targetEffect);
        m_loadedEffectSet.insert(targetEffect);
    }
    else {
        destroyDetachedEffect(targetEffect);
    }
    return loaded;
}

bool
Project::loadOffscreenRenderTargetEffectFromByteArray(Effect *targetEffect, const URI &fileURI,
    const StringPair &condition, const ByteArray &bytes, OffscreenRenderTargetConditionList &newConditions,
    Progress &progress, Error &error)
{
    bool result = false;
    targetEffect->setName(URI::lastPathComponent(condition.second));
    if (targetEffect->load(bytes, progress, error)) {
        targetEffect->setFileURI(fileURI);
        if (targetEffect->upload(effect::kAttachmentTypeOffscreenPassive, progress, error)) {
            OffscreenRenderTargetCondition cond;
            cond.m_pattern = condition.first;
            cond.m_passiveEffect = targetEffect;
            cond.m_hidden = cond.m_none = false;
            newConditions.push_back(cond);
            result = !error.isCancelled();
        }
    }
    return result;
}

void
Project::cancelRenderOffscreenRenderTarget(Effect *ownerEffect)
{
    effect::OffscreenRenderTargetOptionList options;
    ownerEffect->getAllOffscreenRenderTargetOptions(options);
    for (effect::OffscreenRenderTargetOptionList::const_iterator it = options.begin(), end = options.end(); it != end;
         ++it) {
        sg_pass_desc pd;
        it->getPassDescription(pd);
        const nanoem_u32_t key = bx::hash<bx::HashMurmur2A>(pd);
        HashedRenderPassBundleMap::const_iterator it2 = m_hashedRenderPassBundleMap.find(key);
        if (it2 != m_hashedRenderPassBundleMap.end()) {
            const sg_pass pass = it2->second->m_handle;
            DrawQueue::PassCommandBufferList &buffers = m_drawQueue->m_commandBuffers;
            for (DrawQueue::PassCommandBufferList::iterator it3 = buffers.begin(), end3 = buffers.end(); it3 != end3;
                 ++it3) {
                if (it3->m_handle.id == pass.id) {
                    DrawQueue::CommandBuffer *items = it3->m_items;
                    it3 = buffers.erase(it3);
                    nanoem_delete(items);
                    EnumUtils::setEnabled(kResetAllPasses, m_stateFlags, true);
                }
            }
        }
    }
}

void
Project::internalPasteAllSelectedKeyframes(Model *model, nanoem_frame_index_t frameIndex, bool symmetric, Error &error)
{
    nanoem_assert(!isPlaying(), "must not be called while playing");
    if (!isMotionClipboardEmpty()) {
        Motion *source = createMotion();
        if (source->load(m_motionClipboard, frameIndex, error)) {
            ByteArray snapshot;
            if (Motion *modelMotionPtr = resolveMotion(model)) {
                modelMotionPtr->save(snapshot, model, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL, error);
                modelMotionPtr->overrideAllKeyframes(source, symmetric);
                model->pushUndo(command::MotionSnapshotCommand::create(modelMotionPtr, model, snapshot,
                    NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_BONE | NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MODEL |
                        NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MORPH));
            }
            else {
                command::BatchUndoCommandListCommand::UndoCommandList commands;
                Motion *cameraMotionPtr = cameraMotion();
                cameraMotionPtr->save(snapshot, nullptr, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL, error);
                cameraMotionPtr->overrideAllKeyframes(source, false);
                commands.push_back(command::MotionSnapshotCommand::create(
                    cameraMotionPtr, nullptr, snapshot, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_CAMERA));
                Motion *lightMotionPtr = lightMotion();
                lightMotionPtr->save(snapshot, nullptr, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL, error);
                lightMotionPtr->overrideAllKeyframes(source, false);
                commands.push_back(command::MotionSnapshotCommand::create(
                    lightMotionPtr, nullptr, snapshot, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_LIGHT));
                Motion *selfShadowMotionPtr = selfShadowMotion();
                selfShadowMotionPtr->save(snapshot, nullptr, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL, error);
                selfShadowMotionPtr->overrideAllKeyframes(source, false);
                commands.push_back(command::MotionSnapshotCommand::create(
                    selfShadowMotionPtr, nullptr, snapshot, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_SELFSHADOW));
                for (Project::AccessoryList::const_iterator it = m_allAccessoryPtrs.begin(),
                                                            end = m_allAccessoryPtrs.end();
                     it != end; ++it) {
                    Accessory *accessory = *it;
                    if (Motion *accessoryMotionPtr = resolveMotion(accessory)) {
                        accessoryMotionPtr->save(snapshot, nullptr, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL, error);
                        accessoryMotionPtr->overrideAllKeyframes(source, false);
                        commands.push_back(command::MotionSnapshotCommand::create(
                            accessoryMotionPtr, nullptr, snapshot, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ACCESSORY));
                    }
                }
                pushUndo(command::BatchUndoCommandListCommand::create(commands, nullptr, this));
            }
        }
        destroyMotion(source);
    }
}

void
Project::internalPasteAllSelectedBones(Model *model, bool symmetric, Error &error)
{
    nanoem_assert(!isPlaying(), "must not be called while playing");
    if (model && !isModelClipboardEmpty()) {
        Model *source = createModel();
        if (source->load(m_modelClipboard, error)) {
            nanoem_rsize_t numMorphs;
            nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(source->data(), &numMorphs);
            if (numMorphs > 0) {
                model::BindPose lastBindPose, currentBindPose;
                model->saveBindPose(lastBindPose);
                nanoem_rsize_t numBoneMorphs;
                const nanoem_model_morph_t *rootMorph = morphs[0];
                nanoem_unicode_string_factory_t *factory = unicodeStringFactory();
                nanoem_model_morph_bone_t *const *boneMorphs =
                    nanoemModelMorphGetAllBoneMorphObjects(rootMorph, &numBoneMorphs);
                model::Bone::Set originBones;
                StringSet symmetricBoneNameSet;
                for (nanoem_rsize_t i = 0; i < numBoneMorphs; i++) {
                    const nanoem_model_morph_bone_t *boneMorph = boneMorphs[i];
                    const nanoem_model_bone_t *sourceBone = nanoemModelMorphBoneGetBoneObject(boneMorph);
                    String name;
                    StringUtils::getUtf8String(
                        nanoemModelBoneGetName(sourceBone, NANOEM_LANGUAGE_TYPE_FIRST_ENUM), factory, name);
                    const nanoem_model_bone_t *originBone = model->findBone(name);
                    if (model::Bone *bone = model::Bone::cast(originBone)) {
                        const Vector3 translation(glm::make_vec3(nanoemModelMorphBoneGetTranslation(boneMorph)));
                        const Quaternion orientation(glm::make_quat(nanoemModelMorphBoneGetOrientation(boneMorph)));
                        const char *namePtr = bone->canonicalNameConstString();
                        if (symmetric) {
                            if (StringUtils::hasPrefix(
                                    namePtr, reinterpret_cast<const char *>(model::Bone::kNameLeftInJapanese))) {
                                const String newName(StringUtils::substitutedPrefixString(
                                    reinterpret_cast<const char *>(model::Bone::kNameRightInJapanese), namePtr));
                                if (symmetricBoneNameSet.find(newName) == symmetricBoneNameSet.end()) {
                                    symmetricLocalTransformBone(newName, translation, orientation, model);
                                    symmetricBoneNameSet.insert(newName);
                                }
                            }
                            if (StringUtils::hasPrefix(
                                    namePtr, reinterpret_cast<const char *>(model::Bone::kNameRightInJapanese))) {
                                const String newName(StringUtils::substitutedPrefixString(
                                    reinterpret_cast<const char *>(model::Bone::kNameLeftInJapanese), namePtr));
                                if (symmetricBoneNameSet.find(newName) == symmetricBoneNameSet.end()) {
                                    symmetricLocalTransformBone(newName, translation, orientation, model);
                                    symmetricBoneNameSet.insert(newName);
                                }
                            }
                        }
                        else {
                            bone->setLocalUserTranslation(translation);
                            bone->setLocalUserOrientation(orientation);
                        }
                        bone->setDirty(true);
                        originBones.insert(originBone);
                    }
                }
                model->saveBindPose(currentBindPose);
                model->pushUndo(command::TransformBoneCommand::create(
                    lastBindPose, currentBindPose, ListUtils::toListFromSet(originBones), model, this));
            }
        }
        destroyModel(source);
    }
}

void
Project::internalSeek(nanoem_frame_index_t frameIndex)
{
    internalSeek(frameIndex, 0, 0);
}

void
Project::internalSeek(nanoem_frame_index_t frameIndex, nanoem_f32_t amount, nanoem_f32_t delta)
{
    if (m_transformPerformedAt.first != Motion::kMaxFrameIndex && frameIndex != m_transformPerformedAt.first) {
        if (Model *model = activeModel()) {
            undoStackSetOffset(model->undoStack(), m_transformPerformedAt.second);
        }
        resetTransformPerformedAt();
    }
    if (frameIndex < currentLocalFrameIndex()) {
        restart(frameIndex);
    }
    synchronizeAllMotions(frameIndex, amount, PhysicsEngine::kSimulationTimingBefore);
    internalPerformPhysicsSimulation(delta);
    synchronizeAllMotions(frameIndex, amount, PhysicsEngine::kSimulationTimingAfter);
    markAllModelsDirty();
    ILight *light = globalLight();
    ICamera *camera = globalCamera();
    camera->update();
    light->setDirty(false);
    camera->setDirty(false);
    m_backgroundVideoRenderer->seek((nanoem_f64_t(frameIndex) + amount) / Constants::kHalfBaseFPS);
    m_localFrameIndex.second = frameIndex - m_localFrameIndex.first;
    m_localFrameIndex.first = frameIndex;
}

void
Project::destroyDetachedEffect(Effect *effect)
{
    if (effect) {
        effect->destroy();
        nanoem_delete(effect);
    }
}

void
Project::synchronizeCamera(nanoem_frame_index_t frameIndex, nanoem_f32_t amount)
{
    static const Vector3 kCamraDirection(-1, 1, 1);
    PerspectiveCamera camera0(this), camera1(this);
    camera0.synchronizeParameters(m_cameraMotionPtr, frameIndex);
    if (amount > 0 && !m_cameraMotionPtr->findCameraKeyframe(frameIndex + 1)) {
        camera1.synchronizeParameters(m_cameraMotionPtr, frameIndex + 1);
        m_camera->setAngle(glm::mix(camera0.angle(), camera1.angle(), amount) * kCamraDirection);
        m_camera->setDistance(glm::mix(camera0.distance(), camera1.distance(), amount));
        m_camera->setFovRadians(glm::mix(camera0.fovRadians(), camera1.fovRadians(), amount));
        m_camera->setLookAt(glm::mix(camera0.lookAt(), camera1.lookAt(), amount));
    }
    else {
        m_camera->setAngle(camera0.angle() * kCamraDirection);
        m_camera->setDistance(camera0.distance());
        m_camera->setFovRadians(camera0.fovRadians());
        m_camera->setLookAt(camera0.lookAt());
    }
    m_camera->setPerspective(camera0.isPerspective());
    for (int i = NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM;
         i < NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; i++) {
        nanoem_motion_camera_keyframe_interpolation_type_t type =
            static_cast<nanoem_motion_camera_keyframe_interpolation_type_t>(i);
        m_camera->setBezierControlPoints(type, camera0.bezierControlPoints(type));
    }
    m_camera->update();
    m_camera->setDirty(false);
}

void
Project::synchronizeLight(nanoem_frame_index_t frameIndex, nanoem_f32_t amount)
{
    DirectionalLight light0(this), light1(this);
    light0.synchronizeParameters(m_lightMotionPtr, frameIndex);
    if (amount > 0) {
        light1.synchronizeParameters(m_lightMotionPtr, frameIndex + 1);
        m_light->setColor(glm::mix(light0.color(), light1.color(), amount));
        m_light->setDirection(glm::mix(light0.direction(), light1.direction(), amount));
    }
    else {
        m_light->setColor(light0.color());
        m_light->setDirection(light0.direction());
    }
    m_light->setDirty(false);
}

void
Project::synchronizeSelfShadow(nanoem_frame_index_t frameIndex)
{
    if (const nanoem_motion_self_shadow_keyframe_t *keyframe =
            m_selfShadowMotionPtr->findSelfShadowKeyframe(frameIndex)) {
        m_shadowCamera->setDistance(nanoemMotionSelfShadowKeyframeGetDistance(keyframe));
        m_shadowCamera->setCoverageMode(
            static_cast<ShadowCamera::CoverageModeType>(nanoemMotionSelfShadowKeyframeGetMode(keyframe)));
        m_shadowCamera->setDirty(false);
    }
}

void
Project::markAllModelsDirty()
{
    for (ModelList::const_iterator it = m_allModelPtrs.begin(), end = m_allModelPtrs.end(); it != end; ++it) {
        Model *model = *it;
        model->markStagingVertexBufferDirty();
    }
    m_renderPassBlitter->markAsDirty();
}

void
Project::internalPerformPhysicsSimulation(nanoem_f32_t delta)
{
    if (isPhysicsSimulationEnabled()) {
        m_physicsEngine->stepSimulation(delta);
        for (ModelList::const_iterator it = m_allModelPtrs.begin(), end = m_allModelPtrs.end(); it != end; ++it) {
            Model *model = *it;
            if (model->isPhysicsSimulationEnabled()) {
                model->synchronizeAllRigidBodiesTransformFeedbackFromSimulation(
                    PhysicsEngine::kRigidBodyFollowBonePerform);
            }
        }
    }
}

void
Project::removeDrawable(IDrawable *drawable)
{
    nanoem_parameter_assert(drawable, "must not be nullptr");
    ListUtils::removeItem(drawable, m_dependsOnScriptExternal);
    removeEffectOrderSet(drawable);
    if (ListUtils::removeItem(drawable, m_drawableOrderList)) {
        drawable->setActiveEffect(nullptr);
        RedoObjectHandleMap::const_iterator it = m_redoObjectHandles.find(drawable->handle());
        if (it != m_redoObjectHandles.end()) {
            m_redoObjectHandles.erase(it);
        }
    }
}

void
Project::internalResizeUniformedViewportImage(const Vector2UI16 &value)
{
    m_uniformViewportImageSize.second = value;
    EnumUtils::setEnabled(kResetAllPasses | kViewportImageSizeChanged, m_stateFlags, true);
}

void
Project::internalResetAllRenderTargets(const Vector2UI16 &size)
{
    SG_PUSH_GROUPF("Project::internalResetAllRenderTargets(width=%d, height=%d)", size.x, size.y);
    for (RenderPassBundleMap::const_iterator it = m_renderPassBundleMap.begin(), end = m_renderPassBundleMap.end();
         it != end; ++it) {
        sg::destroy_pass(it->second.m_handle);
    }
    m_renderPassBundleMap.clear();
    m_renderPassStringMap.clear();
    m_hashedRenderPassBundleMap.clear();
    StringSet sharedRenderColorImageNames, sharedOffscreenImageNames;
    for (LoadedEffectSet::const_iterator it = m_loadedEffectSet.begin(), end = m_loadedEffectSet.end(); it != end;
         ++it) {
        Effect *effect = *it;
        effect->resizeAllRenderTargetImages(size, sharedRenderColorImageNames, sharedOffscreenImageNames);
    }
    for (LoadedEffectSet::const_iterator it = m_loadedEffectSet.begin(), end = m_loadedEffectSet.end(); it != end;
         ++it) {
        Effect *effect = *it;
        effect->resetAllSharedRenderTargetColorImages(sharedRenderColorImageNames);
    }
    for (LoadedEffectSet::const_iterator it = m_loadedEffectSet.begin(), end = m_loadedEffectSet.end(); it != end;
         ++it) {
        Effect *effect = *it;
        effect->resetAllSharedOffscreenRenderTargets(sharedOffscreenImageNames);
    }
    for (OffscreenRenderTargetConditionListMap::const_iterator it = m_allOffscreenRenderTargets.begin(),
                                                               end = m_allOffscreenRenderTargets.end();
         it != end; ++it) {
        const Effect *effect = it->first;
        effect::OffscreenRenderTargetOptionList allRenderTargetOptions;
        effect->getAllOffscreenRenderTargetOptions(allRenderTargetOptions);
        for (effect::OffscreenRenderTargetOptionList::const_iterator it2 = allRenderTargetOptions.begin(),
                                                                     end2 = allRenderTargetOptions.end();
             it2 != end2; ++it2) {
            resetOffscreenRenderTarget(effect, *it2);
        }
    }
    if (isShadowMapEnabled()) {
        m_shadowCamera->update();
        const sg_pass shadowPass = m_shadowCamera->pass();
        RenderPassBundle &d = m_renderPassBundleMap[shadowPass.id];
        d.m_colorImage = m_shadowCamera->colorImage();
        d.m_depthImage = m_shadowCamera->depthImage();
    }
    SG_POP_GROUP();
}

void
Project::resetViewportPassFormatAndDescription()
{
    const sg_pass viewportPass = viewportPrimaryPass();
    PixelFormat format;
    format.setColorPixelFormat(viewportPixelFormat(), 0);
    format.setNumSamples(sampleCount());
    RenderPassBundle &descRef = m_renderPassBundleMap[viewportPass.id];
    descRef.m_format = format;
    m_viewportPrimaryPass.getDescription(descRef.m_desciption);
}

sg_pass
Project::registerRenderPass(const sg_pass_desc &desc, const PixelFormat &format, RenderPassBundle *&descPtrRef)
{
    sg_pass_desc pd(desc);
    pd.label = nullptr;
    const nanoem_u32_t key = bx::hash<bx::HashMurmur2A>(pd);
    HashedRenderPassBundleMap::const_iterator it = m_hashedRenderPassBundleMap.find(key);
    sg_pass pass = { SG_INVALID_ID };
    if (it != m_hashedRenderPassBundleMap.end()) {
        descPtrRef = it->second;
        pass = descPtrRef->m_handle;
    }
    else if (sg::is_valid(desc.color_attachments[0].image) && sg::is_valid(desc.depth_stencil_attachment.image)) {
        pass = sg::make_pass(&desc);
        nanoem_assert(sg::query_pass_state(pass) == SG_RESOURCESTATE_VALID, "pass must be valid");
        descPtrRef = &m_renderPassBundleMap[pass.id];
        descPtrRef->m_desciption = desc;
        descPtrRef->m_format = format;
        descPtrRef->m_handle = pass;
        m_hashedRenderPassBundleMap.insert(tinystl::make_pair(key, descPtrRef));
        sg_pass_action pa;
        Inline::clearZeroMemory(pa);
        for (int i = 0; i < SG_MAX_COLOR_ATTACHMENTS; i++) {
            pa.colors[i].action = SG_ACTION_CLEAR;
        }
        pa.depth.value = 1;
        pa.depth.action = SG_ACTION_CLEAR;
        pa.stencil.action = SG_ACTION_CLEAR;
        clearRenderPass(m_serialDrawQueue, pass, pa, format);
        SG_INSERT_MARKERF("Project::registerRenderPass(handle=%d)", pass.id);
    }
    nanoem_assert(sg::is_valid(pass), "must be valid");
    if (sg::is_valid(pass) && sg::is_valid(m_currentOffscreenRenderPass)) {
        m_currentOffscreenRenderPass = pass;
    }
    return pass;
}

void
Project::resetOffscreenRenderTarget(const Effect *ownerEffect, const effect::OffscreenRenderTargetOption &option)
{
    SG_PUSH_GROUPF("Project::resetOffscreenRenderTarget(owner=%s, name=%s)", ownerEffect->nameConstString(),
        option.m_name.c_str());
    sg_pass renderPass = registerOffscreenRenderPass(ownerEffect, option);
    char viewName[Inline::kMarkerStringLength];
    StringUtils::format(viewName, sizeof(viewName), "%s/%s", ownerEffect->nameConstString(), option.m_name.c_str());
    setRenderPassName(renderPass, viewName);
    SG_POP_GROUP();
}

void
Project::drawOffscreenRenderTarget(Effect *ownerEffect)
{
    sg_pass_action pa;
    effect::OffscreenRenderTargetOptionList options;
    SortedOffscreenRenderTargetOptionList sorted;
    getAllOffscreenRenderTargetOptions(ownerEffect, options, sorted);
    for (SortedOffscreenRenderTargetOptionList::const_iterator it = sorted.begin(), end = sorted.end(); it != end;
         ++it) {
        const effect::OffscreenRenderTargetOption *option = *it;
        const String &name = option->m_name;
        const sg_pass pass = registerOffscreenRenderPass(ownerEffect, *option);
        effect::RenderPassScope renderPassScope;
        PassScope scope(m_currentOffscreenRenderPass, pass), scope2(m_originOffscreenRenderPass, pass);
        BX_UNUSED_2(scope, scope2);
        SG_PUSH_GROUPF("Project::drawOffscreenRenderTarget(name=%s, pass=%s, owner=%s)", name.c_str(),
            findRenderPassName(pass), ownerEffect->nameConstString());
        option->getPassAction(pa);
        setOffscreenRenderPassScope(&renderPassScope);
        const int numSamples = option->m_colorImageDescription.sample_count;
        const PixelFormat format(findRenderPassPixelFormat(pass, numSamples));
        clearRenderPass(sharedBatchDrawQueue(), pass, pa, format);
        for (DrawableList::const_iterator it2 = m_drawableOrderList.begin(), end2 = m_drawableOrderList.end();
             it2 != end2; ++it2) {
            drawObjectToOffscreenRenderTarget(*it2, ownerEffect, name);
        }
        ownerEffect->generateOffscreenMipmapImagesChain(*option);
        setOffscreenRenderPassScope(nullptr);
        renderPassScope.reset(nullptr);
        SG_POP_GROUP();
    }
}

void
Project::drawObjectToOffscreenRenderTarget(IDrawable *drawable, Effect *ownerEffect, const String &name)
{
    if (drawable->isOffscreenPassiveRenderTargetEffectEnabled(name)) {
        if (IEffect *passiveEffect = drawable->findOffscreenPassiveRenderTargetEffect(name)) {
            IEffect *lastActiveEffect = drawable->activeEffect();
            if (lastActiveEffect && lastActiveEffect->scriptClass() != IEffect::kScriptClassTypeScene &&
                lastActiveEffect->scriptOrder() == IEffect::kScriptOrderTypeStandard) {
                drawable->setActiveEffect(ownerEffect);
                drawable->setPassiveEffect(passiveEffect);
                if (isGroundShadowEnabled()) {
                    drawable->draw(IDrawable::kDrawTypeGroundShadow);
                }
                if (m_editingMode != kEditingModeSelect) {
                    drawable->draw(IDrawable::kDrawTypeEdge);
                }
                drawable->draw(IDrawable::kDrawTypeColor);
                drawable->setActiveEffect(lastActiveEffect);
                drawable->setPassiveEffect(nullptr);
            }
        }
    }
}

void
Project::drawAllEffectsDependsOnScriptExternal()
{
    if (hasAnyDependsOnScriptExternalEffect()) {
        SG_PUSH_GROUPF("Project::drawDependsOnScriptExternal(size=%d)", m_dependsOnScriptExternal.size());
        for (DrawableList::const_iterator it = m_dependsOnScriptExternal.begin(), end = m_dependsOnScriptExternal.end();
             it != end; ++it) {
            IDrawable *drawable = *it;
            drawable->draw(IDrawable::kDrawTypeScriptExternalColor);
        }
        SG_POP_GROUP();
    }
    else {
        resetCurrentRenderPass();
    }
}

void
Project::getAllOffscreenRenderTargetOptions(const Effect *ownerEffect, effect::OffscreenRenderTargetOptionList &value,
    SortedOffscreenRenderTargetOptionList &sorted) const
{
    ownerEffect->getAllOffscreenRenderTargetOptions(value);
    if (!value.empty()) {
        sorted.clear();
        sorted.reserve(value.size());
        for (effect::OffscreenRenderTargetOptionList::const_iterator it = value.begin(), end = value.end(); it != end;
             ++it) {
            sorted.push_back(it);
        }
        qsort(sorted.data(), sorted.size(), sizeof(sorted[0]), compareOffscreenRenderTargetOption);
    }
}

bool
Project::hasAnyDependsOnScriptExternalEffect() const NANOEM_DECL_NOEXCEPT
{
    bool result = false;
    if (!m_dependsOnScriptExternal.empty()) {
        for (DrawableList::const_iterator it = m_dependsOnScriptExternal.begin(), end = m_dependsOnScriptExternal.end();
             it != end; ++it) {
            IDrawable *drawable = *it;
            if (drawable->isVisible()) {
                result = true;
                break;
            }
        }
    }
    return result;
}

void
Project::clearViewportPrimaryPass()
{
    SG_PUSH_GROUP("Project::clearViewportPass");
    sg_pass_action action;
    Inline::clearZeroMemory(action);
    action.colors[0].action = action.depth.action = action.stencil.action = SG_ACTION_CLEAR;
    memcpy(&action.colors[0].value, glm::value_ptr(m_viewportBackgroundColor), sizeof(action.colors[0].value));
    action.depth.value = 1;
    const PixelFormat format(findRenderPassPixelFormat(currentRenderPass(), sampleCount()));
    clearRenderPass(sharedBatchDrawQueue(), currentRenderPass(), action, format);
    SG_POP_GROUP();
}

void
Project::drawBackgroundVideo()
{
    const sg_pass pass = sg::is_valid(m_currentRenderPass) ? currentRenderPass() : viewportPrimaryPass();
    if (m_backgroundVideoRect == Vector4SI32()) {
        m_backgroundVideoRenderer->draw(pass, kRectCoordination, m_backgroundVideoScaleFactor, this);
    }
    else {
        const Vector4 base(deviceScaleUniformedViewportLayoutRect()), den(base.z, base.w, base.z, base.w);
        const Vector4 rect(Vector4(m_backgroundVideoRect) / den);
        m_backgroundVideoRenderer->draw(pass, rect, m_backgroundVideoScaleFactor, this);
    }
}

void
Project::drawGrid()
{
    const sg_pass pass = currentRenderPass();
    SG_PUSH_GROUPF("Project::drawGrid(id=%d, name=%s)", pass.id, findRenderPassName(pass));
    {
        sg_pass_action action;
        Inline::clearZeroMemory(action);
        action.colors[0].action = action.depth.action = action.stencil.action = SG_ACTION_LOAD;
        sg::PassBlock pb(sharedBatchDrawQueue(), beginRenderPass(pass), action);
        m_grid->draw(pb);
    }
    SG_POP_GROUP();
}

void
Project::drawViewport(IEffect::ScriptOrderType order, IDrawable::DrawType type)
{
    EffectOrderSet::const_iterator it = m_effectOrderSet.find(order);
    if (it != m_effectOrderSet.end()) {
        SG_PUSH_GROUPF("Project::drawViewport(order=%s, type=%s)", EnumStringifyUtils::toString(order),
            EnumStringifyUtils::toString(type));
        sg_pass_action action;
        Inline::clearZeroMemory(action);
        action.colors[0].action = action.depth.action = SG_ACTION_LOAD;
        const DrawableSet &drawableSet = it->second;
        for (DrawableList::const_iterator it2 = m_drawableOrderList.begin(), end2 = m_drawableOrderList.end();
             it2 != end2; ++it2) {
            IDrawable *drawable = *it2;
            if (drawableSet.find(drawable) != drawableSet.end()) {
                drawable->draw(type);
            }
        }
        SG_POP_GROUP();
    }
}

void
Project::blitRenderPass(
    sg::PassBlock::IDrawQueue *drawQueue, sg_pass destRenderPass, sg_pass sourceRenderPass, internal::BlitPass *blitter)
{
    if (sourceRenderPass.id != destRenderPass.id) {
        RenderPassBundleMap::const_iterator source = m_renderPassBundleMap.find(sourceRenderPass.id);
        if (source != m_renderPassBundleMap.end()) {
            sg_image sourceColorImage = source->second.m_desciption.color_attachments[0].image;
            /* prevent blitting dest(viewportPrimaryPass) == source(viewportPrimaryPass) */
            if (sourceColorImage.id == m_viewportPrimaryPass.m_colorImage.id &&
                destRenderPass.id != m_viewportSecondaryPass.m_handle.id) {
                blitter->blit(drawQueue, tinystl::make_pair(m_viewportSecondaryPass.m_handle, kViewportSecondaryName),
                    tinystl::make_pair(sourceColorImage, kViewportPrimaryName), kRectCoordination,
                    findRenderPassPixelFormat(m_viewportSecondaryPass.m_handle, sampleCount()));
                sourceColorImage = m_viewportSecondaryPass.m_colorImage;
            }
            RenderPassBundleMap::const_iterator it = m_renderPassBundleMap.find(destRenderPass.id);
            int numSamples = sampleCount();
            if (it != m_renderPassBundleMap.end()) {
                numSamples = it->second.m_format.numSamples();
            }
            blitter->blit(drawQueue,
                tinystl::make_pair(destRenderPass, findRenderPassName(destRenderPass, "(unknown)")),
                tinystl::make_pair(sourceColorImage, findRenderPassName(sourceRenderPass, "(unknown)")),
                kRectCoordination, findRenderPassPixelFormat(destRenderPass, numSamples));
        }
    }
}

void
Project::createFallbackImage()
{
    if (!sg::is_valid(m_fallbackImage)) {
        sg_image_desc desc;
        ImageLoader::fill1x1WhitePixelImage(desc);
        if (Inline::isDebugLabelEnabled()) {
            desc.label = "@nanoem/FallbackImage";
        }
        m_fallbackImage = sg::make_image(&desc);
        nanoem_assert(sg::query_image_state(m_fallbackImage) == SG_RESOURCESTATE_VALID, "image must be valid");
        SG_LABEL_IMAGE(m_fallbackImage, desc.label);
    }
}

void
Project::preparePlaying()
{
    setInputTextFocus(false);
    saveState(m_lastSaveState);
    setActiveModel(nullptr);
    setActiveAccessory(nullptr);
    const nanoem_frame_index_t from = playingSegment().frameIndexFrom();
    if (currentLocalFrameIndex() < from) {
        const nanoem_u32_t denominator = m_audioPlayer->sampleRate();
        const nanoem_f64_t seconds = nanoem_f64_t(from / nanoem_f32_t(baseFPS()));
        const IAudioPlayer::Rational rational = { nanoem_u64_t(seconds * denominator), glm::max(denominator, 1u) };
        m_audioPlayer->seek(rational);
        /* call update to make lastRational same as currentRational */
        m_audioPlayer->update();
    }
}

void
Project::prepareStopping(bool forceSeek)
{
    restoreState(m_lastSaveState, forceSeek);
    destroyState(m_lastSaveState);
}

bool
Project::loadAttachedDrawableEffect(IDrawable *drawable, bool enableSourceCache, Progress &progress, Error &error)
{
    const URI &fileURI = drawable->fileURI();
    Effect *effect = findEffect(fileURI);
    bool enableEffectPlugin = isEffectPluginEnabled(), succeeded = false;
    if (effect) {
        internalSetDrawableActiveEffect(drawable, effect, Project::IncludeEffectSourceMap(), enableEffectPlugin,
            enableSourceCache, progress, error);
        succeeded = !error.isCancelled();
    }
    else {
        effect = createEffect();
        URI sourceURI;
        if (enableEffectPlugin) {
            succeeded = loadEffectFromSource(fileURI, effect, enableSourceCache, sourceURI, progress, error) &&
                !error.isCancelled();
        }
        if (succeeded) {
            const URI &binaryURI = Effect::resolveURI(fileURI, Effect::kBinaryFileExtension);
            if (loadEffectFromBinary(binaryURI, effect, progress, error)) {
                sourceURI = binaryURI;
                succeeded = !error.isCancelled();
            }
        }
        if (succeeded) {
            effect->setFileURI(sourceURI);
            internalSetDrawableActiveEffect(drawable, effect, Project::IncludeEffectSourceMap(), enableEffectPlugin,
                enableSourceCache, progress, error);
        }
        else {
            destroyDetachedEffect(effect);
        }
    }
    return succeeded;
}

void
Project::applyAllOffscreenRenderTargetEffectsToDrawable(IDrawable *drawable)
{
    for (OffscreenRenderTargetConditionListMap::const_iterator it = m_allOffscreenRenderTargets.begin(),
                                                               end = m_allOffscreenRenderTargets.end();
         it != end; ++it) {
        applyDrawableToOffscreenRenderTargetEffect(drawable, it->first);
    }
}

void
Project::applyAllDrawablesToOffscreenRenderTargetEffect(IDrawable *ownerDrawable, Effect *ownerEffect)
{
    for (DrawableList::const_iterator it = m_drawableOrderList.begin(), end = m_drawableOrderList.end(); it != end;
         ++it) {
        IDrawable *drawablePtr = *it;
        if (drawablePtr != ownerDrawable) {
            applyDrawableToOffscreenRenderTargetEffect(drawablePtr, ownerEffect);
        }
    }
}

void
Project::applyDrawableToOffscreenRenderTargetEffect(IDrawable *drawable, Effect *ownerEffect)
{
    OffscreenRenderTargetConditionListMap::const_iterator it = m_allOffscreenRenderTargets.find(ownerEffect);
    if (it != m_allOffscreenRenderTargets.end()) {
        const NamedOffscreenRenderTargetConditionListMap &conditionMap = it->second;
        for (NamedOffscreenRenderTargetConditionListMap::const_iterator it2 = conditionMap.begin(),
                                                                        end2 = conditionMap.end();
             it2 != end2; ++it2) {
            const String &ownerName = it2->first;
            const OffscreenRenderTargetConditionList &conditions = it2->second;
            bool matched = false;
            for (OffscreenRenderTargetConditionList::const_iterator it3 = conditions.begin(), end3 = conditions.end();
                 it3 != end3; ++it3) {
                const OffscreenRenderTargetCondition &condition = *it3;
                matched = matchDrawableEffect(drawable, ownerEffect, condition.m_pattern);
                if (matched) {
                    if (!condition.m_hidden) {
                        if (!condition.m_none) {
                            setOffscreenPassiveRenderTargetEffect(ownerName, drawable, condition.m_passiveEffect);
                        }
                        else {
                            drawable->setOffscreenDefaultRenderTargetEffect(ownerName);
                        }
                    }
                    break;
                }
            }
            if (!matched) {
                drawable->setOffscreenDefaultRenderTargetEffect(ownerName);
            }
        }
    }
}

URI
Project::resolveSourceEffectCachePath(const URI &fileURI)
{
    nanoem_u8_t digest[SHA256_BLOCK_SIZE];
    char buffer[40];
    const String &filePath = fileURI.absolutePath();
    SHA256_CTX ctx;
    sha256_init(&ctx);
    nanoem_u32_t renderer = static_cast<nanoem_u32_t>(sg::query_backend());
    sha256_update(&ctx, reinterpret_cast<const nanoem_u8_t *>(&renderer), sizeof(renderer));
    sha256_update(
        &ctx, reinterpret_cast<const nanoem_u8_t *>(filePath.c_str()), Inline::saturateInt32(filePath.size()));
    sha256_final(&ctx, digest);
    for (nanoem_rsize_t i = 0; i < BX_COUNTOF(digest); i++) {
        nanoem_rsize_t offset = 2 * i;
        StringUtils::format(buffer + offset, Inline::saturateInt32(sizeof(buffer) - offset), "%02x", digest[i]);
    }
    const URI &directoryURI = m_fileManager->sharedSourceEffectCacheDirectory();
    String cachePath(directoryURI.absolutePath());
    cachePath.append("/");
    cachePath.append(buffer);
    return URI::createFromFilePath(cachePath);
}

bool
Project::findSourceEffectCache(const URI &fileURI, ByteArray &cache, Error &error)
{
    const String &extension = fileURI.pathExtension();
    if (!Accessory::isLoadableExtension(extension) && !Model::isLoadableExtension(extension)) {
        const URI &cacheURI = resolveSourceEffectCachePath(fileURI);
        FileReaderScope scope(m_translator);
        if (scope.open(cacheURI, error)) {
            ByteArray deflated;
            nanoem_u32_t signature, inflatedSize, deflatedSize;
            IFileReader *reader = scope.reader();
            FileUtils::readTyped(reader, signature, error);
            if (signature == nanoem_fourcc('n', 'm', 'C', 'E')) {
                FileUtils::readTyped(reader, inflatedSize, error);
                FileUtils::readTyped(reader, deflatedSize, error);
                deflated.resize(deflatedSize);
                cache.resize(inflatedSize);
                nanoem_u8_t expectedDigest[SHA256_BLOCK_SIZE];
                FileUtils::readTyped(reader, expectedDigest, error);
                FileUtils::read(reader, deflated.data(), deflatedSize, error);
                SHA256_CTX ctx;
                sha256_init(&ctx);
                nanoem_u64_t timestamp = FileUtils::timestamp(fileURI);
                sha256_update(&ctx, reinterpret_cast<const nanoem_u8_t *>(&timestamp), sizeof(timestamp));
                sha256_update(&ctx, deflated.data(), deflatedSize);
                nanoem_u8_t actualDigest[SHA256_BLOCK_SIZE];
                sha256_final(&ctx, actualDigest);
                if (memcmp(actualDigest, expectedDigest, BX_COUNTOF(actualDigest)) == 0) {
                    int actualSize = LZ4_decompress_safe(reinterpret_cast<const char *>(deflated.data()),
                        reinterpret_cast<char *>(cache.data()), deflatedSize, inflatedSize);
                    if (actualSize < 0 || Inline::saturateInt32U(actualSize) != inflatedSize) {
                        cache.resize(0);
                    }
                }
            }
        }
    }
    return !error.hasReason() && !cache.empty();
}

void
Project::setSourceEffectCache(const URI &fileURI, const ByteArray &cache, Error &error)
{
    const String &extension = fileURI.pathExtension();
    if (!Accessory::isLoadableExtension(extension) && !Model::isLoadableExtension(extension)) {
        FileWriterScope scope;
        if (scope.open(resolveSourceEffectCachePath(fileURI), error)) {
            ByteArray deflated;
            int inflatedSize = Inline::saturateInt32(cache.size());
            deflated.reserve(LZ4_compressBound(inflatedSize));
            int deflatedSize = LZ4_compress_fast(reinterpret_cast<const char *>(cache.data()),
                reinterpret_cast<char *>(deflated.data()), inflatedSize, Inline::saturateInt32(deflated.capacity()), 1);
            if (deflatedSize > 0) {
                SHA256_CTX ctx;
                sha256_init(&ctx);
                nanoem_u64_t timestamp = FileUtils::timestamp(fileURI);
                sha256_update(&ctx, reinterpret_cast<const nanoem_u8_t *>(&timestamp), sizeof(timestamp));
                sha256_update(&ctx, deflated.data(), deflatedSize);
                nanoem_u8_t digest[SHA256_BLOCK_SIZE];
                sha256_final(&ctx, digest);
                ISeekableWriter *writer = scope.writer();
                FileUtils::write(writer, nanoem_fourcc('n', 'm', 'C', 'E'), error);
                FileUtils::write(writer, Inline::saturateInt32U(inflatedSize), error);
                FileUtils::write(writer, Inline::saturateInt32U(deflatedSize), error);
                FileUtils::write(writer, digest, BX_COUNTOF(digest), error);
                FileUtils::write(writer, deflated.data(), deflatedSize, error);
                scope.commit(error);
            }
        }
    }
}

void
Project::addLoadedEffectSet(Effect *value)
{
    nanoem_parameter_assert(value, "value must not be NULL");
    const String path(value->fileURI().absolutePath());
    EffectReferenceMap::iterator it2 = m_effectReferences.find(path);
    if (it2 != m_effectReferences.end()) {
        it2->second.second++;
    }
    else {
        m_effectReferences.insert(tinystl::make_pair(path, tinystl::make_pair(value, 1)));
    }
    m_loadedEffectSet.insert(value);
}

void
Project::setOffscreenRenderPassScope(effect::RenderPassScope *value)
{
    m_offscreenRenderPassScope = value;
}

bool
Project::continuesPlaying()
{
    bool playing = true;
    nanoem_frame_index_t frameIndex = currentLocalFrameIndex();
    if (m_audioPlayer->isFinished() || frameIndex >= m_playingSegment.frameIndexTo(duration())) {
        stop();
        if (isLoopEnabled()) {
            internalSeek(0);
            play();
        }
        else {
            playing = false;
        }
    }
    return playing;
}

} /* namespace nanoem */
