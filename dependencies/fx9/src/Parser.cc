/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is licensed under MIT license. for more details, see LICENSE.txt.
 */

#include "fx9/Parser.h"
#include "fx9/Lexer.h"

/* win32 */
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#else
#include <fcntl.h>
#include <float.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

/* GLSLang */
#include "glslang/HLSL/hlslAttributes.h"
#include "glslang/HLSL/hlslParseHelper.h"
#include "glslang/HLSL/hlslParseables.h"
#include "glslang/HLSL/hlslScanContext.h"
#include "glslang/HLSL/hlslTokenStream.h"

using namespace glslang;

#if 0 // ndef NDEBUG
#define FX9_TRACE_EXPR(expr) expr
#else
#define FX9_TRACE_EXPR(expr)
#endif

extern "C" {
struct parser_t;
parser_t *fx9LemonParserContextAlloc(void *(*mallocProc)(size_t));
void fx9LemonParserContextFree(parser_t *, void (*freeProc)(void *));
void fx9LemonParserContext(parser_t *, int, fx9::LexerToken *, fx9::ParserContext *);
void fx9LemonParserContextTrace(FILE *fp, const char *prompt);
}

namespace {

#if 0
#define FX9_INTERM_NULL_TRACE() fprintf(stderr, "%s was called\n", __PRETTY_FUNCTION__)
#else
#define FX9_INTERM_NULL_TRACE()
#endif

static const char *kGlobalConstantVariableNamePrefix = "c_";
static const char *kGlobalUniformVariableNamePrefix = "u_";
static const char *kGlobalStaticVariableNamePrefix = "s_";

struct fx_interm_null_node_t : TIntermNode {
    void
    traverse(TIntermTraverser *) override
    {
    }
    TIntermTyped *
    getAsTyped() override
    {
        FX9_INTERM_NULL_TRACE();
        return nullptr;
    }
    TIntermOperator *
    getAsOperator() override
    {
        FX9_INTERM_NULL_TRACE();
        return nullptr;
    }
    TIntermConstantUnion *
    getAsConstantUnion() override
    {
        FX9_INTERM_NULL_TRACE();
        return nullptr;
    }
    TIntermAggregate *
    getAsAggregate() override
    {
        FX9_INTERM_NULL_TRACE();
        return nullptr;
    }
    TIntermUnary *
    getAsUnaryNode() override
    {
        FX9_INTERM_NULL_TRACE();
        return nullptr;
    }
    TIntermBinary *
    getAsBinaryNode() override
    {
        FX9_INTERM_NULL_TRACE();
        return nullptr;
    }
    TIntermSelection *
    getAsSelectionNode() override
    {
        FX9_INTERM_NULL_TRACE();
        return nullptr;
    }
    TIntermSwitch *
    getAsSwitchNode() override
    {
        FX9_INTERM_NULL_TRACE();
        return nullptr;
    }
    TIntermMethod *
    getAsMethodNode() override
    {
        FX9_INTERM_NULL_TRACE();
        return nullptr;
    }
    TIntermSymbol *
    getAsSymbolNode() override
    {
        FX9_INTERM_NULL_TRACE();
        return nullptr;
    }
    TIntermBranch *
    getAsBranchNode() override
    {
        FX9_INTERM_NULL_TRACE();
        return nullptr;
    }
    const TIntermTyped *
    getAsTyped() const override
    {
        FX9_INTERM_NULL_TRACE();
        return nullptr;
    }
    const TIntermOperator *
    getAsOperator() const override
    {
        FX9_INTERM_NULL_TRACE();
        return nullptr;
    }
    const TIntermConstantUnion *
    getAsConstantUnion() const override
    {
        FX9_INTERM_NULL_TRACE();
        return nullptr;
    }
    const TIntermAggregate *
    getAsAggregate() const override
    {
        FX9_INTERM_NULL_TRACE();
        return nullptr;
    }
    const TIntermUnary *
    getAsUnaryNode() const override
    {
        FX9_INTERM_NULL_TRACE();
        return nullptr;
    }
    const TIntermBinary *
    getAsBinaryNode() const override
    {
        FX9_INTERM_NULL_TRACE();
        return nullptr;
    }
    const TIntermSelection *
    getAsSelectionNode() const override
    {
        FX9_INTERM_NULL_TRACE();
        return nullptr;
    }
    const TIntermSwitch *
    getAsSwitchNode() const override
    {
        FX9_INTERM_NULL_TRACE();
        return nullptr;
    }
    const TIntermMethod *
    getAsMethodNode() const override
    {
        FX9_INTERM_NULL_TRACE();
        return nullptr;
    }
    const TIntermSymbol *
    getAsSymbolNode() const override
    {
        FX9_INTERM_NULL_TRACE();
        return nullptr;
    }
    const TIntermBranch *
    getAsBranchNode() const override
    {
        FX9_INTERM_NULL_TRACE();
        return nullptr;
    }
};

struct PathUtils {
    static void
    normalize(TString &path)
    {
        TString slashedPath(path);
        std::transform(
            slashedPath.begin(), slashedPath.end(), slashedPath.begin(), [](char c) { return c == '\\' ? '/' : c; });
        char p = 0;
        std::vector<char> trailedPath(slashedPath.size() + 1);
        std::copy_if(slashedPath.begin(), slashedPath.end(), trailedPath.begin(), [&p](char c) {
            const bool equals = p == c;
            p = c;
            return !(c == '/' && equals);
        });
        path = trailedPath.data();
    }
};

struct MatrixSwizzleConverter : TIntermTraverser {
    MatrixSwizzleConverter(TIntermediate *intermediate, HlslParseContext *context)
        : TIntermTraverser()
        , m_intermediate(intermediate)
        , m_context(context)
    {
    }
    bool
    visitBinary(TVisit, TIntermBinary *node)
    {
        if (node->getOp() == EOpMatrixSwizzle) {
            TIntermAggregate *swizzleNode = node->getRight()->getAsAggregate();
            const TIntermSequence &sequence = swizzleNode->getSequence();
            const TType &type = node->getType();
            if (type.isVector()) {
                TIntermTyped *baseNode = node->getLeft();
                const TSourceLoc &loc = node->getLoc();
                TIntermAggregate *argumentsNode = nullptr;
                for (size_t i = 0, numSequences = sequence.size(); i < numSequences; i += 2) {
                    TIntermConstantUnion *firstConst = sequence[i]->getAsConstantUnion(),
                                         *secondConst = sequence[i + 1]->getAsConstantUnion();
                    TIntermTyped *firstNode = m_context->handleBracketDereference(loc, baseNode, firstConst),
                                 *secondNode = m_context->handleBracketDereference(loc, firstNode, secondConst);
                    argumentsNode = m_intermediate->growAggregate(argumentsNode, secondNode);
                }
                node->setLeft(m_context->handleConstructor(loc, argumentsNode, type));
                TIntermConstantUnion *oneNode = m_intermediate->addConstantUnion(1.0, EbtFloat, loc, true);
                node->setRight(m_context->handleConstructor(loc, oneNode, type));
                node->setOp(EOpMul);
                return false;
            }
        }
        return true;
    }
    TIntermediate *m_intermediate;
    HlslParseContext *m_context;
};

} /* namespace anonymous */

namespace fx9 {

ParserContext::InternalNode::InternalNode(TIntermNode *value)
    : m_value(value)
{
}

ParserContext::InternalNode::InternalNode()
    : InternalNode(nullptr)
{
}

void
ParserContext::MatrixSwizzleField::setRowIndex(int value)
{
    rows[offsetRow++] = value;
}

void
ParserContext::MatrixSwizzleField::setColumnIndex(int value)
{
    columns[offsetColumn++] = value;
    numIndices++;
}

ParserContext::IncluderContext::IncluderContext()
{
}

ParserContext::IncluderContext::~IncluderContext()
{
    for (auto &it : m_sources) {
        delete it.second;
    }
    m_sources.clear();
}

ParserContext::TStringList
ParserContext::IncluderContext::includedSourcePathList() const
{
    return m_includedSourePathList;
}

void
ParserContext::IncluderContext::setSourceBasePath(const TString &value)
{
    m_basePath = value;
}

void
ParserContext::IncluderContext::addSource(const TString &path, const TString &source)
{
    TString normalized(path);
    PathUtils::normalize(normalized);
    m_sources.insert(std::make_pair(normalized, new TString(source)));
    m_includedSourePathList.push_back(normalized);
}

TShader::Includer::IncludeResult *
ParserContext::IncluderContext::includeSystem(const char *requested_source, const char *requesting_source, size_t depth)
{
    return includeLocal(requested_source, requesting_source, depth);
}

TShader::Includer::IncludeResult *
ParserContext::IncluderContext::includeLocal(
    const char *requested_source, const char * /* requesting_source */, size_t /* depth */)
{
    TString normalized(requested_source);
    PathUtils::normalize(normalized);
    auto it = m_sources.find(normalized);
    IncludeResult *result = nullptr;
    if (it != m_sources.end()) {
        const TString *source = it->second;
        result = new IncludeResult { normalized.c_str(), source->c_str(), source->size(), nullptr };
    }
    else {
        TString path(m_basePath);
        path.append("/");
        path.append(requested_source);
        PathUtils::normalize(path);
        auto it2 = m_sources.find(path);
        if (it2 != m_sources.end()) {
            const TString *source = it2->second;
            result = new IncludeResult { normalized.c_str(), source->c_str(), source->size(), nullptr };
        }
        if (!result) {
#if defined(_WIN32)
            wchar_t widePath[MAX_PATH];
            widePath[MultiByteToWideChar(CP_UTF8, 0, path.c_str(), int(path.size()), widePath, ARRAYSIZE(widePath))] =
                0;
            HANDLE handle = ::CreateFileW(
                widePath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (handle != INVALID_HANDLE_VALUE) {
                std::vector<char> bytes;
                DWORD size = ::GetFileSize(handle, nullptr);
                bytes.resize(size);
                DWORD numReadBytes = 0;
                ::ReadFile(handle, bytes.data(), size, &numReadBytes, nullptr);
                ::CloseHandle(handle);
                TString *source = new TString(bytes.begin(), bytes.end());
                m_sources.insert(std::make_pair(path, source));
                m_includedSourePathList.push_back(normalized);
                result = new IncludeResult { normalized.c_str(), source->c_str(), source->size(), nullptr };
            }
#else
            int fd = ::open(path.c_str(), O_RDONLY);
            if (fd != -1) {
                struct stat st;
                ::fstat(fd, &st);
                if (S_ISREG(st.st_mode)) {
                    std::vector<char> bytes(st.st_size);
                    ::read(fd, bytes.data(), bytes.size());
                    TString *source = new TString(bytes.begin(), bytes.end());
                    m_sources.insert(std::make_pair(path, source));
                    m_includedSourePathList.push_back(normalized);
                    result = new IncludeResult { normalized.c_str(), source->c_str(), source->size(), nullptr };
                }
                ::close(fd);
            }
#endif
        }
    }
    m_resultPtr = result;
    return result;
}

void
ParserContext::IncluderContext::releaseInclude(TShader::Includer::IncludeResult *result)
{
    m_resultPtr = nullptr;
    delete result;
}

const ParserContext::IncluderContext::IncludeResult *
ParserContext::IncluderContext::currentIncludeResult() const
{
    return m_resultPtr;
}

ParserContext::SamplerNodeTraverser::SamplerNodeTraverser(ParserContext *context, List *nodes, Set *names)
    : TIntermTraverser()
    , m_context(context)
    , m_nodes(nodes)
    , m_names(names)
{
    for (size_t i = 0; i < 16; i++) {
        m_slots.push_back(i);
    }
}

ParserContext::SamplerNodeTraverser::~SamplerNodeTraverser()
{
}

void
ParserContext::SamplerNodeTraverser::visitSymbol(TIntermSymbol *symbol)
{
    if (symbol->getType().isTexture()) {
        const TString &name = symbol->getName();
        auto it = m_context->m_samplerNodes.find(name);
        if (it != m_context->m_samplerNodes.end() && m_names->find(name) == m_names->end()) {
            const SamplerNodeItem &node = it->second;
            m_nodes->push_back(std::make_pair(name, node.m_samplerIndex));
            m_names->insert(name);
            if (node.m_samplerIndex != SIZE_MAX) {
                for (auto it = m_slots.begin(), end = m_slots.end(); it != end; ++it) {
                    if (*it == node.m_samplerIndex) {
                        m_slots.erase(it);
                        break;
                    }
                }
            }
        }
    }
}

bool
ParserContext::SamplerNodeTraverser::visitAggregate(TVisit, TIntermAggregate *aggregate)
{
    if (aggregate->getOp() == EOpFunctionCall) {
        const TString &name = aggregate->getName();
        auto it = m_context->m_allFunctions.find(name);
        if (it != m_context->m_allFunctions.end()) {
            TIntermNode *functionBody = it->second[0].second;
            functionBody->traverse(this);
        }
    }
    return true;
}

size_t
ParserContext::SamplerNodeTraverser::acquireSamplerSlot()
{
    size_t index = 0;
    if (!m_slots.empty()) {
        index = m_slots.front();
        m_slots.erase(m_slots.begin());
    }
    return index;
}

ParserContext::OutputVertexShaderVariableTraverser::OutputVertexShaderVariableTraverser(const TString *name)
    : m_name(name)
{
}

ParserContext::OutputVertexShaderVariableTraverser::~OutputVertexShaderVariableTraverser()
{
}

void
ParserContext::OutputVertexShaderVariableTraverser::visitSymbol(TIntermSymbol *symbol)
{
    m_found |= symbol->getName() == *m_name;
}

bool
ParserContext::OutputVertexShaderVariableTraverser::hasFound() const
{
    return m_found;
}

TString *
ParserContext::newTString(const TString &value)
{
    return NewPoolTString(value.c_str());
}

TString *
ParserContext::newAnonymousVariableString(atom_t uniqueId)
{
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%s%zu", AnonymousPrefix, uniqueId);
    return NewPoolTString(buffer);
}

bool
ParserContext::isNonZeroConstant(const glslang::TIntermTyped *node)
{
    bool result = false;
    if (const TIntermConstantUnion *constantUnion = node->getAsConstantUnion()) {
        const TConstUnion &value = constantUnion->getConstArray()[0];
        switch (constantUnion->getBasicType()) {
        case EbtBool:
            result = value.getBConst();
            break;
        case EbtInt:
            result = value.getIConst() != 0;
            break;
        case EbtFloat:
            result = std::abs(value.getDConst()) > DBL_EPSILON;
            break;
        default:
            break;
        }
    }
    return result;
}

void
ParserContext::convertAllAnnotations(const TIntermAggregate *annotationsNode, AnnotationList &annotations)
{
    if (annotationsNode) {
        const TIntermSequence &sequence = annotationsNode->getSequence();
        for (const auto &it : sequence) {
            if (const TIntermBinary *annotationNode = it->getAsBinaryNode()) {
                const TIntermBinary *semanticAnnotationNode = annotationNode->getLeft()->getAsBinaryNode();
                const TIntermSymbol *nameNode = semanticAnnotationNode->getRight()->getAsSymbolNode();
                Annotation annotation(nameNode->getName());
                annotation.m_value = annotationNode->getRight();
                annotations.push_back(annotation);
            }
        }
    }
}

void
ParserContext::convertPass(const TIntermSequence &sequence, const StateBlockMap &stateBlocks, Pass &pass)
{
    for (const auto &it : sequence) {
        const TIntermBinary *stateNode = it->getAsBinaryNode();
        const TString &name = stateNode->getLeft()->getAsSymbolNode()->getName();
        TString canonicalizedName(name);
        std::transform(name.begin(), name.end(), canonicalizedName.begin(), ::toupper);
        if (canonicalizedName == "VERTEXSHADER") {
            const TIntermBinary *value = stateNode->getRight()->getAsBinaryNode();
            TIntermAggregate *argumentsNode = value->getRight() ? value->getRight()->getAsAggregate() : nullptr;
            pass.m_vertexShaderEntryPoint =
                std::make_pair(value->getLeft()->getAsSymbolNode()->getName().c_str(), argumentsNode);
        }
        else if (canonicalizedName == "PIXELSHADER") {
            const TIntermBinary *value = stateNode->getRight()->getAsBinaryNode();
            TIntermAggregate *argumentsNode = value->getRight() ? value->getRight()->getAsAggregate() : nullptr;
            pass.m_pixelShaderEntryPoint =
                std::make_pair(value->getLeft()->getAsSymbolNode()->getName().c_str(), argumentsNode);
        }
        else if (canonicalizedName == "STATEBLOCK") {
            if (const TIntermSymbol *value = stateNode->getRight()->getAsSymbolNode()) {
                const auto it2 = stateBlocks.find(value->getName());
                if (it2 != stateBlocks.end()) {
                    convertPass(it2->second->getSequence(), StateBlockMap(), pass);
                }
            }
        }
        else {
            PassState state(&pass, name);
            state.m_value = stateNode->getRight();
            pass.m_states.push_back(state);
        }
    }
}

void
ParserContext::convertPass(const TIntermBinary *passNode, const StateBlockMap &stateBlocks, Pass &pass)
{
    const TIntermBinary *valueNode = passNode->getRight()->getAsBinaryNode();
    const TIntermAggregate *statesNode = valueNode->getLeft() ? valueNode->getLeft()->getAsAggregate() : nullptr;
    if (statesNode) {
        convertPass(statesNode->getSequence(), stateBlocks, pass);
    }
    TIntermTyped *annotationNode = valueNode->getRight();
    convertAllAnnotations(annotationNode ? annotationNode->getAsAggregate() : nullptr, pass.m_annotations);
}

void
ParserContext::convertTechnique(
    const TIntermAggregate *passesNode, const StateBlockMap &stateBlocks, Technique &technique)
{
    if (passesNode) {
        const TIntermSequence &sequence = passesNode->getSequence();
        for (const auto &it : sequence) {
            const TIntermBinary *passNode = it->getAsBinaryNode();
            Pass pass(&technique, passNode->getLeft()->getAsSymbolNode()->getName());
            convertPass(passNode, stateBlocks, pass);
            technique.m_passes.push_back(pass);
        }
    }
}

ParserContext::ParserContext(HlslParseContext *context, const TString &filename,
    const BuiltInVariableMap &vertexShaderInputVariables, const BuiltInVariableMap &pixelShaderInputVariables,
    const BuiltInVariableMap &writtenVertexShaderInputVariables)
    : m_vertexShaderInputVariables(vertexShaderInputVariables)
    , m_pixelShaderInputVariables(pixelShaderInputVariables)
    , m_context(context)
    , m_intermediate(context->language, context->version, context->profile)
    , m_filename(filename)
    , m_writtenVertexShaderInputVariables(writtenVertexShaderInputVariables)
{
    m_intermediate.setSpv(context->spvVersion);
    m_symbolTable.adoptLevels(m_context->symbolTable);
    m_nodes.push_back(InternalNode());
    m_context->initializeExtensionBehavior();
    m_context->updateExtensionBehavior(E_GL_GOOGLE_cpp_style_line_directive, EBhEnable);
    m_context->updateExtensionBehavior(E_GL_GOOGLE_include_directive, EBhEnable);
    static const char *kGLSLKeywords[] = { "attribute", "varying", "layout", "centroid", "flat", "smooth",
        "noperspective", "patch", "sample", "subroutine", "noperspective", "invariant", "discard", "mat2", "mat3",
        "mat4", "mat2x2", "mat2x3", "mat2x4", "mat3x2", "mat3x3", "mat3x4", "mat4x2", "mat4x3", "mat4x4", "dmat2",
        "dmat3", "dmat4", "dmat2x2", "dmat2x3", "dmat2x4", "dmat3x2", "dmat3x3", "dmat3x4", "dmat4x2", "dmat4x3",
        "dmat4x4", "vec2", "vec3", "vec4", "ivec2", "ivec3", "ivec4", "bvec2", "bvec3", "bvec4", "dvec2", "dvec3",
        "dvec4", "uvec2", "uvec3", "uvec4", "lowp", "mediump", "highp", "precision" };
    for (size_t i = 0; i < sizeof(kGLSLKeywords) / sizeof(kGLSLKeywords[0]); i++) {
        m_reservedWordSet.insert(kGLSLKeywords[i]);
    }
    static const char *kGLSLFutureUseKeywords[] = { "common", "partition", "active", "asm", "class", "union", "enum",
        "typedef", "template", "this", "packed", "goto", "inline", "noinline", "volatile", "public", "static", "extern",
        "external", "interface", "long", "short", "half", "fixed", "unsigned", "superp", "input", "output", "hvec2",
        "hvec3", "hvec4", "fvec2", "fvec3", "fvec4", "sampler3DRect", "filter", "image1D", "image2D", "image3D",
        "imageCube", "iimage1D", "iimage2D", "iimage3D", "iimageCube", "uimage1D", "uimage2D", "uimage3D", "uimageCube",
        "image1DArray", "image2DArray", "iimage1DArray", "iimage2DArray", "uimage1DArray", "uimage2DArray",
        "image1DShadow", "image2DShadow", "image1DArrayShadow", "image2DArrayShadow", "iimageBuffer", "uimageBuffer",
        "sizeof", "cast", "namespace", "using", "row_major" };
    for (size_t i = 0; i < sizeof(kGLSLFutureUseKeywords) / sizeof(kGLSLFutureUseKeywords[0]); i++) {
        m_reservedWordSet.insert(kGLSLFutureUseKeywords[i]);
    }
    static const char *kMSLKeywords[] = { "kernel", "vertex", "fragment", "compute", "bias", "level", "gradient2d",
        "gradientcube", "gradient3d", "min_lod_clamp", "assert", "quad_broadcast", "main", "fmin3", "fmax3",
        "is_function_constant_defined" };
    for (size_t i = 0; i < sizeof(kMSLKeywords) / sizeof(kMSLKeywords[0]); i++) {
        m_reservedWordSet.insert(kMSLKeywords[i]);
    }
    /* built-in function */
    static const char *kGLSLBuiltInFunctions[] = { "noise2", "noise3", "noise4" };
    for (size_t i = 0; i < sizeof(kGLSLBuiltInFunctions) / sizeof(kGLSLBuiltInFunctions[0]); i++) {
        m_reservedWordSet.insert(kGLSLBuiltInFunctions[i]);
    }
    m_builtInSemantics.insert(std::make_pair("POSITION0", EbvVertex));
    m_builtInSemantics.insert(std::make_pair("POSITION1", EbvNormal));
    m_builtInSemantics.insert(std::make_pair("POSITION", EbvVertex));
    m_builtInSemantics.insert(std::make_pair("SV_POSITION", EbvVertex));
    m_builtInSemantics.insert(std::make_pair("NORMAL", EbvNormal));
    m_builtInSemantics.insert(std::make_pair("PSIZE", EbvPointSize));
    m_builtInSemantics.insert(std::make_pair("FOG", EbvFogFragCoord));
    m_builtInSemantics.insert(std::make_pair("DEPTH", EbvFragDepth));
    m_builtInSemantics.insert(std::make_pair("VFACE", EbvFace));
    m_builtInSemantics.insert(std::make_pair("VPOS", EbvFragCoord));
    m_builtInSemantics.insert(std::make_pair("COLOR", EbvColor));
    m_builtInSemantics.insert(std::make_pair("TEXCOORD", EbvMultiTexCoord0));
    m_builtInSemantics.insert(std::make_pair("TEXCOORD0", EbvMultiTexCoord0));
    m_builtInSemantics.insert(std::make_pair("TEXCOORD1", EbvMultiTexCoord1));
    m_builtInSemantics.insert(std::make_pair("TEXCOORD2", EbvMultiTexCoord2));
    m_builtInSemantics.insert(std::make_pair("TEXCOORD3", EbvMultiTexCoord3));
    m_builtInSemantics.insert(std::make_pair("TEXCOORD4", EbvMultiTexCoord4));
    m_builtInSemantics.insert(std::make_pair("TEXCOORD5", EbvMultiTexCoord5));
    m_builtInSemantics.insert(std::make_pair("TEXCOORD6", EbvMultiTexCoord6));
    m_builtInSemantics.insert(std::make_pair("TEXCOORD7", EbvMultiTexCoord7));
    m_builtInSemantics.insert(std::make_pair("TEXCOORD8", EbvFragDepthGreater));
    m_builtInSemantics.insert(std::make_pair("TEXCOORD9", EbvFragDepthLesser));
    m_builtInSemantics.insert(std::make_pair("COLOR0", EbvFrontColor));
    m_builtInSemantics.insert(std::make_pair("COLOR1", EbvBackColor));
    m_builtInSemantics.insert(std::make_pair("COLOR2", EbvFrontSecondaryColor));
    m_builtInSemantics.insert(std::make_pair("COLOR3", EbvBackSecondaryColor));
    m_builtInSemantics.insert(std::make_pair("_INDEX", EbvVertexIndex));
    m_vertexShadreBuiltInVariableConversions.insert(
        std::make_pair(EbvVertex, std::make_pair(EvqPosition, EbvPosition)));
    m_vertexShadreBuiltInVariableConversions.insert(
        std::make_pair(EbvPosition, std::make_pair(EvqPosition, EbvPosition)));
    m_vertexShadreBuiltInVariableConversions.insert(
        std::make_pair(EbvPointSize, std::make_pair(EvqPointSize, EbvPointSize)));
    m_vertexShadreBuiltInVariableConversions.insert(
        std::make_pair(EbvClipVertex, std::make_pair(EvqClipVertex, EbvVertex)));
    m_vertexShadreBuiltInVariableConversions.insert(
        std::make_pair(EbvColor, std::make_pair(EvqFragColor, EbvFragColor)));
    m_pixelShaderBuiltInVariableConversions.insert(std::make_pair(EbvFragCoord, EvqFragCoord));
    m_pixelShaderBuiltInVariableConversions.insert(std::make_pair(EbvColor, EvqFragColor));
    m_pixelShaderBuiltInVariableConversions.insert(std::make_pair(EbvFrontColor, EvqFragColor));
    m_pixelShaderBuiltInVariableConversions.insert(std::make_pair(EbvBackColor, EvqFragColor));
    m_pixelShaderBuiltInVariableConversions.insert(std::make_pair(EbvFrontSecondaryColor, EvqFragColor));
    m_pixelShaderBuiltInVariableConversions.insert(std::make_pair(EbvBackSecondaryColor, EvqFragColor));
    m_pixelShaderBuiltInVariableConversions.insert(std::make_pair(EbvFragDepth, EvqFragDepth));
}

ParserContext::~ParserContext()
{
    for (auto &it : m_usedFunctionParameters) {
        delete it.second;
    }
    m_usedFunctionParameters.clear();
}

void
ParserContext::execute(const TString &source, const TString &preamble)
{
    FX9_TRACE_EXPR(fx9LemonParserContextTrace(stdout, "> "));
    const char *sources[] = { preamble.c_str(), source.c_str() };
    const char *names[] = { m_filename.c_str(), m_filename.c_str() };
    size_t lengths[] = { preamble.size(), source.size() };
    TInputScanner scanner(2, sources, lengths, names);
    m_context->setScanner(&scanner);
    if (const char *p = strrchr(m_filename.c_str(), '/')) {
        m_includer.setSourceBasePath(TString(m_filename.c_str(), p));
    }
    LexerContext lexer(*m_context, m_filename.c_str(), m_includer);
    lexer.setInput(scanner, false);
    TPpToken token;
    parser_t *opaque = fx9LemonParserContextAlloc(malloc);
    m_context->symbolTable.push();
    createGlobalUniformVariable(224);
    while (int major = lexer.scan(this, token)) {
        LexerToken *lexerToken = new LexerToken { token, major };
        FX9_TRACE_EXPR(fprintf(stdout, "< name=%s major=%d token=%d at %d:%d\n", lexerToken->value.name, major,
            lexerToken->value.ival, lexerToken->value.loc.line, lexerToken->value.loc.column));
        fx9LemonParserContext(opaque, major, lexerToken, this);
        if (lexerToken && m_context->getNumErrors() == 0) {
            m_lastLexerToken = lexerToken;
        }
    }
    const TString *name = globalUniformFloat4Name();
    if (TSymbol *symbol = m_context->symbolTable.find(*name)) {
        symbol->getWritableType().changeOuterArraySize(int(m_lastUniformRegisterIndex));
    }
    fx9LemonParserContext(opaque, 0, nullptr, this);
    m_context->symbolTable.pop(nullptr);
    fx9LemonParserContextFree(opaque, free);
}

void
ParserContext::setEntryPoint(const Pass::EntryPoint &value)
{
    m_entryPoint = value;
}

bool
ParserContext::isUserDefinedType(const TString &name) const
{
    bool result = false;
    if (const TSymbol *symbol = m_context->symbolTable.find(name)) {
        result = symbol->getAsVariable() && symbol->getAsVariable()->isUserType();
    }
    return result;
}

ParserContext::SamplerNodeMap
ParserContext::findSamplerNodes(const Pass::EntryPoint &entryPoint)
{
    SamplerNodeMap nodes;
    auto it = m_allFunctions.find(entryPoint.first.c_str());
    if (it != m_allFunctions.end()) {
        SamplerNodeTraverser::List used;
        SamplerNodeTraverser::Set names;
        SamplerNodeTraverser traverser(this, &used, &names);
        TIntermAggregate *node = it->second[0].second;
        node->traverse(&traverser);
        for (const auto &it : m_staticParameters[nullptr]) {
            TIntermNode *node = it.second;
            node->traverse(&traverser);
        }
        for (const auto &it : m_expressionConstantParameters) {
            TIntermNode *node = it.second;
            node->traverse(&traverser);
        }
        if (const TIntermAggregate *aggregateNode = entryPoint.second) {
            for (const TIntermNode *node : aggregateNode->getSequence()) {
                if (const TIntermSymbol *symbol = node->getAsSymbolNode()) {
                    const TString &name = symbol->getName();
                    if (names.find(name) == names.end()) {
                        used.push_back(std::make_pair(name, SIZE_MAX));
                        names.insert(name);
                    }
                }
            }
        }
        for (const auto &item : used) {
            const TString &name = item.first;
            auto it3 = m_samplerNodes.find(name);
            if (it3 != m_samplerNodes.end()) {
                SamplerNodeItem &node = it3->second;
                node.m_samplerIndex = item.second != SIZE_MAX ? item.second : traverser.acquireSamplerSlot();
                nodes.insert(std::make_pair(name, node));
            }
        }
    }
    return nodes;
}

ParserContext::NodeMap
ParserContext::constantNodes() const
{
    return m_constantNodes;
}

ParserContext::NodeMap
ParserContext::uniformNodes() const
{
    return m_uniformNodes;
}

ParserContext::SamplerNodeMap
ParserContext::samplerNodes() const
{
    return m_samplerNodes;
}

ParserContext::TextureNodeMap
ParserContext::textureNodes() const
{
    return m_textureNodes;
}

ParserContext::AnnotationListMap
ParserContext::annotations() const
{
    return m_annotations;
}

ParserContext::TechniqueList
ParserContext::techniques() const
{
    return m_techniques;
}

const char *
ParserContext::filename() const
{
    return m_filename.c_str();
}

const char *
ParserContext::currentProcessingFilename() const
{
    const IncluderContext::IncludeResult *result = m_includer.currentIncludeResult();
    return result ? result->headerName.c_str() : filename();
}

const LexerToken *
ParserContext::lastLexerToken() const
{
    return m_lastLexerToken;
}

size_t
ParserContext::countAllParameterNodes() const
{
    return m_constantNodes.size() + m_uniformNodes.size() + m_samplerNodes.size() + m_textureNodes.size();
}

bool
ParserContext::hasAnyParameterNodes() const
{
    return !m_constantNodes.empty() || !m_uniformNodes.empty() || !m_samplerNodes.empty() || !m_textureNodes.empty();
}

HlslParseContext *
ParserContext::parseContext()
{
    return m_context;
}

TIntermediate &
ParserContext::intermediate()
{
    return m_intermediate;
}

void
ParserContext::dump(std::ostream &destination)
{
#if defined(DUMP) && DUMP
    const TInfoSink &contextInfoSink = m_context->infoSink;
    destination << "[context.DEBUG]" << std::endl;
    destination << contextInfoSink.debug.c_str() << std::endl;
    destination << "[context.INFO]" << std::endl;
    destination << contextInfoSink.info.c_str() << std::endl;
#if defined(INTERMEDIATE) && INTERMEDIATE
    TInfoSink intermediateInfoSink;
    m_intermediate.output(intermediateInfoSink, true);
    destination << "[intermediate.DEBUG]" << std::endl;
    destination << intermediateInfoSink.debug.c_str() << std::endl;
    destination << "[intermediate.INFO]" << std::endl;
    destination << intermediateInfoSink.info.c_str() << std::endl;
#endif
#else
    (void) destination;
#endif
}

ParserContext::BuiltInVariableMap
ParserContext::writtenVertexShaderInputVariables() const
{
    return m_writtenVertexShaderInputVariables;
}

ParserContext::TStringList
ParserContext::includedShaderSourcePathList() const
{
    return m_includer.includedSourcePathList();
}

void
ParserContext::addIncludeSource(const TString &key, const TString &value)
{
    m_includer.addSource(key, value);
}

void
ParserContext::acceptTranslationUnit(atom_t unit)
{
    if (TIntermAggregate *unitNode = resolveIntermAggregate(unit)) {
        auto it = m_allFunctions.find(m_entryPoint.first.c_str());
        if (it != m_allFunctions.end()) {
            TFunction *function = it->second[0].first;
            m_intermediate.setEntryPointMangledName("main(");
            m_intermediate.incrementEntryPointCount();
            switch (m_intermediate.getStage()) {
            case EShLangVertex: {
                unitNode = createVertexShaderEntryPoint(unitNode, function, m_entryPoint.second);
                break;
            }
            case EShLangFragment: {
                unitNode = createPixelShaderEntryPoint(unitNode, function, m_entryPoint.second);
                break;
            }
            default:
                break;
            }
            MatrixSwizzleConverter traverser(&m_intermediate, m_context);
            unitNode->traverse(&traverser);
            if (m_intermediate.postProcess(unitNode, m_context->getLanguage())) {
                m_intermediate.setTreeRoot(unitNode);
            }
        }
    }
}

atom_t
ParserContext::acceptStatement(atom_t statements, atom_t newStatement)
{
    TIntermNode *aggregatedStatementsNode = nullptr, *statementsNode = resolveIntermAggregate(statements);
    if (TIntermNode *statementNode = resolveIntermStatement(newStatement)) {
        aggregatedStatementsNode = growAggregateNode(statementsNode, statementNode);
    }
    else {
        aggregatedStatementsNode = statementsNode;
    }
    return allocateIntermNode(aggregatedStatementsNode);
}

atom_t
ParserContext::acceptCompoundStatement(atom_t statements)
{
    if (TIntermAggregate *aggregateNode = resolveIntermAggregate(statements)) {
        aggregateNode->setDebug(false);
        aggregateNode->setOptimize(true);
        aggregateNode->setOperator(EOpSequence);
    }
    return statements;
}

atom_t
ParserContext::acceptReturnStatement(atom_t expr)
{
    TIntermBranch *branchNode = nullptr;
    if (TIntermTyped *exprNode = resolveIntermNode(expr)->getAsTyped()) {
        const TSourceLoc &loc = exprNode->getLoc();
        const TType &expectedType = m_currentFunction->getType(), &actualType = exprNode->getType();
        if (expectedType != actualType) {
            int expectedVectorSize = expectedType.getVectorSize(), actualVectorSize = actualType.getVectorSize();
            if (expectedType.isScalar() && expectedType.getBasicType() == EbtBool && actualType.isScalar()) {
                if (actualType.isFloatingDomain()) {
                    exprNode = castFloatToBool(exprNode);
                }
                else if (actualType.isIntegerDomain()) {
                    exprNode = castIntegerToBool(exprNode);
                }
            }
            else if (expectedVectorSize < actualVectorSize) {
                exprNode = swizzleVectorNode(exprNode, expectedType);
            }
            else if (expectedVectorSize > actualVectorSize) {
                if (TFunction *function = m_context->makeConstructorCall(loc, expectedType)) {
                    TIntermTyped *newArgumentsNode = nullptr;
                    m_context->handleFunctionArgument(function, newArgumentsNode, exprNode);
                    if (actualVectorSize > 1) {
                        static const int kInitialVectorValues[] = { 0, 0, 0, 1 };
                        for (int i = 0, delta = expectedVectorSize - actualVectorSize; i < delta; i++) {
                            int offset = i + actualVectorSize;
                            TIntermConstantUnion *argNode =
                                m_intermediate.addConstantUnion(kInitialVectorValues[offset], loc, true);
                            m_context->handleFunctionArgument(function, newArgumentsNode, argNode);
                        }
                    }
                    exprNode = m_context->handleFunctionCall(loc, function, newArgumentsNode);
                }
            }
        }
        branchNode = m_intermediate.addBranch(EOpReturn, exprNode, loc);
    }
    else {
        branchNode = m_intermediate.addBranch(EOpReturn, TSourceLoc());
    }
    return allocateIntermNode(branchNode);
}

atom_t
ParserContext::acceptBreakStatement()
{
    TIntermBranch *branchNode = m_intermediate.addBranch(EOpBreak, TSourceLoc());
    return allocateIntermNode(branchNode);
}

atom_t
ParserContext::acceptContinueStatement()
{
    TIntermBranch *branchNode = m_intermediate.addBranch(EOpContinue, TSourceLoc());
    return allocateIntermNode(branchNode);
}

atom_t
ParserContext::acceptDiscardStatement()
{
    TIntermBranch *branchNode = m_intermediate.addBranch(EOpKill, TSourceLoc());
    return allocateIntermNode(branchNode);
}

atom_t
ParserContext::acceptSelectBranchStatement(atom_t condExpr, atom_t trueExpr, atom_t falseExpr)
{
    TIntermTyped *selectionNode = nullptr, *condExprNode = resolveIntermNode(condExpr)->getAsTyped();
    TIntermNode *trueExprNode = resolveIntermNode(trueExpr);
    if (condExprNode && trueExprNode) {
        /* float a = 0.0; float b = a ? 1.0 : 0.0; */
        if (condExprNode->isFloatingDomain()) {
            condExprNode = castFloatToBool(condExprNode);
        }
        /* int a = 0; int b = a ? 1 : 0; */
        else if (condExprNode->isIntegerDomain()) {
            condExprNode = castIntegerToBool(condExprNode);
        }
        TIntermNode *falseExprNode = resolveIntermNode(falseExpr);
        selectionNode = new TIntermSelection(condExprNode, trueExprNode, falseExprNode);
        selectionNode->setLoc(condExprNode->getLoc());
    }
    return allocateIntermNode(selectionNode);
}

atom_t
ParserContext::acceptTernaryStatement(atom_t condExpr, atom_t trueExpr, atom_t falseExpr)
{
    TIntermTyped *selectionNode = nullptr, *condExprNode = resolveIntermNode(condExpr)->getAsTyped();
    TIntermNode *trueExprNode = resolveIntermNode(trueExpr);
    if (condExprNode && trueExprNode) {
        /* float a = 0.0; float b = a ? 1.0 : 0.0; */
        if (condExprNode->isFloatingDomain()) {
            condExprNode = castFloatToBool(condExprNode);
        }
        /* int a = 0; float b = a ? 1 : 0; */
        else if (condExprNode->isIntegerDomain()) {
            condExprNode = castIntegerToBool(condExprNode);
        }
        TIntermNode *falseExprNode = resolveIntermNode(falseExpr);
        TType selectionType, trueExprType, falseExprType;
        TIntermTyped *trueTypedNode = trueExprNode->getAsTyped(), *falseTypedNode = falseExprNode->getAsTyped();
        if (trueTypedNode && falseTypedNode) {
            trueExprType.shallowCopy(trueTypedNode->getType());
            falseExprType.shallowCopy(falseTypedNode->getType());
            if (trueExprType.getBasicType() != falseExprType.getBasicType()) {
                /* fix-up true/false expr node type */
                if (trueExprType.isFloatingDomain() && falseExprType.getBasicType() == EbtBool) {
                    falseExprNode = castBoolToFloat(falseTypedNode);
                    falseExprType.shallowCopy(falseExprNode->getAsTyped()->getType());
                }
                else if (trueExprType.isIntegerDomain() && falseExprType.getBasicType() == EbtBool) {
                    falseExprNode = castBoolToInteger(falseTypedNode);
                    falseExprType.shallowCopy(falseExprNode->getAsTyped()->getType());
                }
                else if (trueExprType.getBasicType() == EbtBool && falseExprType.isFloatingDomain()) {
                    falseExprNode = castFloatToBool(falseTypedNode);
                    falseExprType.shallowCopy(falseExprNode->getAsTyped()->getType());
                }
                else if (trueExprType.getBasicType() == EbtBool && falseExprType.isIntegerDomain()) {
                    falseExprNode = castIntegerToBool(falseTypedNode);
                    falseExprType.shallowCopy(falseExprNode->getAsTyped()->getType());
                }
                else if (trueExprType.isIntegerDomain() && falseExprType.isFloatingDomain()) {
                    trueExprType.shallowCopy(falseExprType);
                    selectionType.shallowCopy(falseExprType);
                    trueExprNode = handleSingleConstructorCall(falseExprNode->getAsTyped(), trueExprNode->getAsTyped());
                }
            }
            if (trueExprType.getBasicType() == falseExprType.getBasicType() &&
                trueExprType.getVectorSize() < falseExprType.getVectorSize()) {
                selectionType.shallowCopy(falseExprType);
                trueExprNode = handleSingleConstructorCall(falseExprNode->getAsTyped(), trueExprNode->getAsTyped());
            }
            else {
                selectionType.shallowCopy(trueExprType);
                falseExprNode = handleSingleConstructorCall(trueExprNode->getAsTyped(), falseExprNode->getAsTyped());
            }
        }
        selectionNode = new TIntermSelection(condExprNode, trueExprNode, falseExprNode, selectionType);
        selectionNode->setLoc(condExprNode->getLoc());
    }
    return allocateIntermNode(selectionNode);
}

atom_t
ParserContext::acceptWhileLoopStatement(atom_t bodyExpr, atom_t condExpr, int first)
{
    TIntermNode *loopNode = nullptr;
    TIntermAggregate *bodyExprNode = resolveIntermAggregate(bodyExpr);
    TIntermTyped *condExprNode = resolveIntermNode(condExpr)->getAsTyped();
    if (condExprNode && bodyExprNode) {
        loopNode = m_intermediate.addLoop(bodyExprNode, condExprNode, nullptr, first == 1, condExprNode->getLoc());
    }
    return allocateIntermNode(loopNode);
}

atom_t
ParserContext::acceptForLoopStatement(atom_t initializerExpr, atom_t condExpr, atom_t terminatorExpr, atom_t bodyExpr)
{
    TIntermNode *loopNode = nullptr;
    if (TIntermTyped *bodyExprNode = resolveIntermNode(bodyExpr)->getAsTyped()) {
        TIntermTyped *condExprNode = resolveIntermNode(condExpr)->getAsTyped(),
                     *initializerExprNode = resolveIntermNode(initializerExpr)->getAsTyped(),
                     *terminatorExprNode = resolveIntermNode(terminatorExpr)->getAsTyped();
        TIntermLoop *loop;
        loopNode = m_intermediate.addForLoop(
            bodyExprNode, initializerExprNode, condExprNode, terminatorExprNode, true, bodyExprNode->getLoc(), loop);
        if (TIntermAggregate *aggregateNode = bodyExprNode->getAsAggregate()) {
            aggregateNode->setOperator(EOpSequence);
        }
    }
    return allocateIntermNode(loopNode);
}

atom_t
ParserContext::acceptPostIncrement(atom_t expr)
{
    TIntermNode *newExprNode = nullptr;
    if (TIntermTyped *exprNode = resolveIntermNode(expr)->getAsTyped()) {
        newExprNode = m_intermediate.addUnaryMath(EOpPostIncrement, exprNode, exprNode->getLoc());
    }
    return allocateIntermNode(newExprNode);
}

atom_t
ParserContext::acceptPostDecrement(atom_t expr)
{
    TIntermNode *newExprNode = nullptr;
    if (TIntermTyped *exprNode = resolveIntermNode(expr)->getAsTyped()) {
        newExprNode = m_intermediate.addUnaryMath(EOpPostDecrement, exprNode, exprNode->getLoc());
    }
    return allocateIntermNode(newExprNode);
}

atom_t
ParserContext::acceptFunctionParameter(atom_t parameters, atom_t newParameter)
{
    TIntermNode *parameterNode = resolveIntermAggregate(parameters);
    if (TIntermTyped *newParameterNode = resolveIntermNode(newParameter)->getAsTyped()) {
        parameterNode = growAggregateNode(parameterNode, newParameterNode);
    }
    return allocateIntermNode(parameterNode);
}

atom_t
ParserContext::acceptFunctionArgument(atom_t arguments, atom_t newArgument)
{
    TIntermNode *argumentsNode = resolveIntermAggregate(arguments);
    if (TIntermTyped *newArgumentNode = resolveIntermNode(newArgument)->getAsTyped()) {
        if (const TIntermSymbol *symbolNode = newArgumentNode->getAsSymbolNode()) {
            newArgumentNode = findGlobalVariableNode(symbolNode);
            if (!newArgumentNode) {
                const TString *name = newTString(symbolNode->getName());
                newArgumentNode = m_context->handleVariable(symbolNode->getLoc(), name);
                const TString *functionName = currentFunctionName();
                if (newArgumentNode && functionName) {
                    registerFunctionParameter(*functionName, symbolNode);
                }
                else {
                    auto it = m_samplerNodes.find(*name);
                    if (it != m_samplerNodes.end()) {
                        newArgumentNode = it->second.m_initializerNode;
                        if (const TString *functionName = currentFunctionName()) {
                            registerFunctionParameter(*functionName, symbolNode);
                        }
                    }
                }
            }
        }
        argumentsNode = growAggregateNode(argumentsNode, newArgumentNode);
    }
    return allocateIntermNode(argumentsNode);
}

atom_t
ParserContext::acceptFunctionCall(atom_t name, atom_t arguments)
{
    TIntermTyped *callerNode = nullptr;
    TIntermAggregate *argumentsNode = resolveIntermAggregate(arguments);
    if (TIntermSymbol *nameNode = resolveIntermNode(name)->getAsSymbolNode()) {
        TString *functionName = newTString(nameNode->getName());
        TType returnType;
        TFunction *function = new TFunction(functionName, returnType);
        TIntermTyped *newArgumentsNode = nullptr;
        handleFunctionArguments(function, argumentsNode, newArgumentsNode);
        if (!isFunctionDefined(function->getMangledName())) {
            const TIntermSequence &arguments = argumentsNode ? argumentsNode->getSequence() : TIntermSequence();
            auto it = m_allFunctions.find(function->getName());
            if (it != m_allFunctions.end() && it->second.size() == 1) {
                const TFunction *actualFunction = it->second[0].first;
                handleFunctionArgumentsConversion(actualFunction, arguments, function, newArgumentsNode);
            }
            else {
                handleFunctionArgumentsConversion(arguments, function, newArgumentsNode);
            }
        }
        callerNode = overrideBuiltInCall(function, newArgumentsNode);
        if (!callerNode) {
            callerNode = m_context->handleFunctionCall(nameNode->getLoc(), function, newArgumentsNode);
            if (const TString *currentFunctionNamePtr = currentFunctionName()) {
                copyFunctionParameterSet(*functionName, *currentFunctionNamePtr);
            }
        }
        if (callerNode) {
            if (TIntermAggregate *aggregateNode = callerNode->getAsAggregate()) {
                const TOperator op = aggregateNode->getOp();
                if (op >= EOpTexture && op <= EOpTextureGradOffsetClamp) {
                    m_textureCallerNodes.push_back(aggregateNode);
                }
            }
        }
    }
    return allocateIntermNode(callerNode);
}

atom_t
ParserContext::acceptLocalVariable(atom_t variables, atom_t newVariable)
{
    TIntermAggregate *variablesNode = resolveIntermAggregate(variables);
    if (TIntermTyped *newVariableNode = resolveIntermNode(newVariable)->getAsTyped()) {
        variablesNode = growAggregateNode(variablesNode, newVariableNode)->getAsAggregate();
        variablesNode->setOperator(EOpSequence);
    }
    return allocateIntermNode(variablesNode);
}

atom_t
ParserContext::createLocalVariable(atom_t identifier, atom_t arraySize, atom_t initializer)
{
    TIntermSymbol *nameNode = resolveIntermNode(identifier)->getAsSymbolNode();
    TType specType, initializerType;
    specType.shallowCopy(m_currentTypeSpecNode->getType());
    specType.getQualifier().specConstant = false;
    handleArraySizeSpecifier(resolveIntermNode(arraySize)->getAsTyped(), specType);
    TString *name = newTString(nameNode->getName());
    TIntermTyped *initializerNode = resolveIntermNode(initializer)->getAsTyped();
    if (initializerNode) {
        initializerType.shallowCopy(initializerNode->getType());
        /* static-declared variable in function */
        if (specType.getQualifier().storage == EvqGlobal) {
            TString *originName = newTString(*name);
            auto &sp = m_staticParameters[m_currentFunction];
            sp.insert(std::make_pair(*originName, initializerNode));
            name = NewPoolTString(kGlobalStaticVariableNamePrefix);
            name->append(m_currentFunction->getName());
            name->append("_");
            name->append(*originName);
        }
    }
    else if (!specType.isArray() && !specType.isOpaque()) {
        const TSourceLoc &loc = nameNode->getLoc();
        TIntermTyped *zeroNode = nullptr;
        initializerType.shallowCopy(specType);
        if (specType.isFloatingDomain()) {
            zeroNode = m_intermediate.addConstantUnion(0.0, EbtFloat, loc, true);
        }
        else {
            zeroNode = m_intermediate.addConstantUnion(0, loc, true);
        }
        initializerNode = handleSingleConstructorCall(loc, initializerType, zeroNode);
    }
    TArraySizes *arraySizes = nullptr;
    TIntermNode *declaration = nullptr;
    if (specType != initializerType) {
        /* e.g. float3 a = float4(0.0); float a = float4(0.0) */
        if ((specType.isScalar() && initializerType.isVector()) ||
            (specType.isVector() && initializerType.isVector() &&
                specType.getVectorSize() < initializerType.getVectorSize())) {
            declaration = swizzleInitializer(m_currentTypeSpecNode, nameNode, arraySizes, initializerNode);
        }
        /* e.g. int2 a = float2(0.0); */
        else if (specType.isVector() && initializerType.isVector()) {
            TIntermTyped *newInitializerNode =
                handleSingleConstructorCall(nameNode->getLoc(), specType, initializerNode);
            if (!newInitializerNode) {
                newInitializerNode = initializerNode;
            }
        }
    }
    if (!declaration) {
        declaration = m_context->declareVariable(nameNode->getLoc(), *name, specType, initializerNode);
    }
    return allocateIntermNode(declaration);
}

atom_t
ParserContext::acceptDotDeference(atom_t expr, atom_t name)
{
    TIntermNode *newExprNode = nullptr;
    TIntermTyped *exprNode = resolveIntermNode(expr)->getAsTyped();
    const TIntermSymbol *fieldNode = resolveIntermNode(name)->getAsSymbolNode();
    if (exprNode && fieldNode) {
        newExprNode = m_context->handleDotDereference(exprNode->getLoc(), exprNode, fieldNode->getName());
    }
    return allocateIntermNode(newExprNode);
}

atom_t
ParserContext::acceptBracketDeference(atom_t atomExpr, atom_t index)
{
    TIntermTyped *newExprNode = nullptr, *exprNode = resolveIntermNode(atomExpr)->getAsTyped(),
                 *indexNode = resolveIntermNode(index)->getAsTyped();
    if (exprNode && indexNode) {
        /* bracket[1.0] -> bracket[1] */
        if (indexNode->getBasicType() != EbtInt) {
            const TType toType(EbtInt, EvqTemporary, indexNode->getVectorSize(), indexNode->getMatrixCols(),
                indexNode->getMatrixRows());
            indexNode = handleSingleConstructorCall(toType, indexNode, indexNode);
        }
        if (indexNode) {
            newExprNode = m_context->handleBracketDereference(indexNode->getLoc(), exprNode, indexNode);
        }
    }
    return allocateIntermNode(newExprNode);
}

atom_t
ParserContext::appendAssignmentExpr(atom_t left, atom_t right)
{
    TIntermTyped *exprNode = nullptr, *leftNode = resolveIntermNode(left)->getAsTyped(),
                 *rightNode = resolveIntermNode(right)->getAsTyped();
    if (leftNode && rightNode && leftNode != rightNode) {
        exprNode = m_intermediate.addComma(leftNode, rightNode, rightNode->getLoc());
    }
    else if (rightNode) {
        exprNode = rightNode;
    }
    return allocateIntermNode(exprNode);
}

atom_t
ParserContext::acceptAssignmentExpr(atom_t left, atom_t op, atom_t right)
{
    const TIntermOperator *opNode = resolveIntermNode(op)->getAsOperator();
    TIntermTyped *exprNode = nullptr, *leftNode = resolveIntermNode(left)->getAsTyped(),
                 *rightNode = resolveIntermNode(right)->getAsTyped();
    if (leftNode && opNode && rightNode) {
        const TOperator op = opNode->getOp();
        /* bool a = true; float b = a ? 1.0 : 0.0; */
        if (TIntermSelection *selectionNode = rightNode->getAsSelectionNode()) {
            exprNode = handleSelectiveAssignment(leftNode, op, selectionNode);
        }
        else {
            fixupSwizzleVectorOperation(leftNode, rightNode);
            fixupCastFloatOrBooleanOperation(EOpNull, leftNode, rightNode);
            if (rightNode) {
                TIntermBinary *binaryNode = leftNode->getAsBinaryNode();
                if (binaryNode && op != EOpAssign && binaryNode->getOp() == EOpMatrixSwizzle) {
                    exprNode = fixupSwizzleMatrixOperation(binaryNode, rightNode, op);
                }
                else if (op == EOpDivAssign && !isNonZeroConstant(rightNode) && leftNode->isFloatingDomain() &&
                    rightNode->isFloatingDomain() && (leftNode->isScalar() || leftNode->isVector()) &&
                    (rightNode->isScalar() || rightNode->isVector())) {
                    const TSourceLoc &loc = leftNode->getLoc();
                    rightNode = handleSingleConstructorCall(loc, leftNode->getType(), rightNode);
                    TIntermTyped *intermNode = m_intermediate.addBinaryMath(EOpDiv, leftNode, rightNode, loc);
                    exprNode = castNaNToZero(intermNode, leftNode->getType(), loc);
                    exprNode = m_context->handleAssign(loc, EOpAssign, leftNode, exprNode);
                }
                else {
                    exprNode = m_context->handleAssign(leftNode->getLoc(), op, leftNode, rightNode);
                }
            }
        }
    }
    return allocateIntermNode(exprNode);
}

atom_t
ParserContext::acceptInitializerList(atom_t initializers, atom_t newInitializer)
{
    TIntermNode *initializersNode = resolveIntermAggregate(initializers);
    if (TIntermTyped *newInitializerNode = resolveIntermNode(newInitializer)->getAsTyped()) {
        initializersNode = growAggregateNode(initializersNode, newInitializerNode);
    }
    return allocateIntermNode(initializersNode);
}

atom_t
ParserContext::acceptArraySizeExpression(atom_t expressions, atom_t expr)
{
    TIntermNode *expressionsNode = resolveIntermAggregate(expressions);
    if (TIntermTyped *newExpressionNode = resolveIntermNode(expr)->getAsTyped()) {
        expressionsNode = growAggregateNode(expressionsNode, newExpressionNode);
    }
    return allocateIntermNode(expressionsNode);
}

atom_t
ParserContext::acceptArraySizeSpecifier(atom_t specifier, atom_t expr)
{
    TIntermNode *node = resolveIntermAggregate(specifier);
    if (expr) {
        TIntermAggregate *aggregateNode = resolveIntermAggregate(expr);
        TIntermTyped *folded = m_intermediate.fold(aggregateNode);
        TIntermNode *firstNode = folded->getAsAggregate()->getSequence()[0];
        const TConstUnionArray &values = firstNode->getAsConstantUnion()->getConstArray();
        const int value = values[0].getIConst();
        node = growAggregateNode(node, m_intermediate.addConstantUnion(value, aggregateNode->getLoc(), true));
    }
    else {
        node = growAggregateNode(node, m_intermediate.addConstantUnion(0, TSourceLoc(), true));
    }
    return allocateIntermNode(node);
}

atom_t
ParserContext::acceptUnaryMathPreIncrement(atom_t expr)
{
    TIntermTyped *exprNode = resolveIntermNode(expr)->getAsTyped(),
                 *newExprNode = m_intermediate.addUnaryMath(EOpPreIncrement, exprNode, exprNode->getLoc());
    return allocateIntermNode(newExprNode);
}

atom_t
ParserContext::acceptUnaryMathOperation(atom_t op, atom_t expr)
{
    const TIntermOperator *opNode = resolveIntermNode(op)->getAsOperator();
    TIntermTyped *newExprNode = nullptr, *exprNode = resolveIntermNode(expr)->getAsTyped();
    if (opNode && exprNode) {
        const TOperator operation = opNode->getOp();
        if (operation != EOpAdd) {
            newExprNode = m_intermediate.addUnaryMath(operation, exprNode, exprNode->getLoc());
        }
    }
    return allocateIntermNode(newExprNode);
}

atom_t
ParserContext::acceptVariable(atom_t identifier)
{
    TIntermTyped *variableNode = nullptr;
    if (TIntermSymbol *symbolNode = resolveIntermNode(identifier)->getAsSymbolNode()) {
        if (m_enableAcceptingVariable) {
            variableNode = findGlobalVariableNode(symbolNode);
            if (!variableNode) {
                const TString *currentFunctionNamePtr = currentFunctionName();
                TString *name = newTString(symbolNode->getName());
                variableNode = m_context->handleVariable(symbolNode->getLoc(), name);
                auto it = m_uniformNodes.find(*name);
                if (it != m_uniformNodes.end() && currentFunctionNamePtr) {
                    registerFunctionParameter(*currentFunctionNamePtr, symbolNode);
                }
            }
        }
        else {
            variableNode = symbolNode;
        }
    }
    return allocateIntermNode(variableNode);
}

atom_t
ParserContext::acceptUnaryMathPostIncrement(atom_t expr)
{
    TIntermTyped *newExprNode = nullptr, *exprNode = resolveIntermNode(expr)->getAsTyped();
    newExprNode = m_intermediate.addUnaryMath(EOpPostIncrement, exprNode, exprNode->getLoc());
    return allocateIntermNode(newExprNode);
}

atom_t
ParserContext::acceptBinaryOperation(atom_t left, atom_t op, atom_t right)
{
    const TIntermOperator *opNode = resolveIntermNode(op)->getAsOperator();
    TIntermTyped *exprNode = nullptr, *leftNode = resolveIntermNode(left)->getAsTyped(),
                 *rightNode = resolveIntermNode(right)->getAsTyped();
    if (leftNode && opNode && rightNode) {
        const TOperator op = opNode->getOp();
        fixupSwizzleVectorOperation(leftNode, rightNode);
        fixupCastFloatOrBooleanOperation(op, leftNode, rightNode);
        /* alternate implementation of mod */
        if (op == EOpMod) {
            TType returnType(EbtFloat);
            TFunction *fmodFunction = new TFunction(NewPoolTString("fmod"), returnType);
            TIntermTyped *argumentsNode = nullptr;
            m_context->handleFunctionArgument(fmodFunction, argumentsNode, leftNode);
            m_context->handleFunctionArgument(fmodFunction, argumentsNode, rightNode);
            TIntermTyped *fmodCallNode = m_context->handleFunctionCall(leftNode->getLoc(), fmodFunction, argumentsNode);
            TFunction *truncFunction = new TFunction(NewPoolTString("trunc"), returnType);
            argumentsNode = nullptr;
            m_context->handleFunctionArgument(truncFunction, argumentsNode, fmodCallNode);
            exprNode = m_context->handleFunctionCall(leftNode->getLoc(), truncFunction, argumentsNode);
        }
        else if (op >= EOpEqual && op <= EOpGreaterThanEqual && leftNode->isVector() && rightNode->isVector()) {
            const int vectorSize = std::max(leftNode->getVectorSize(), rightNode->getVectorSize());
            const TType type(EbtBool, EvqTemporary, vectorSize);
            if (TFunction *constructor = m_context->makeConstructorCall(opNode->getLoc(), type)) {
                TIntermTyped *argumentsNode = nullptr;
                for (int i = 0; i < vectorSize; i++) {
                    TIntermTyped *lhs = i < leftNode->getVectorSize()
                        ? indexVectorNode(leftNode, i)
                        : m_intermediate.addConstantUnion(true, leftNode->getLoc(), true);
                    TIntermTyped *rhs = i < rightNode->getVectorSize()
                        ? indexVectorNode(rightNode, i)
                        : m_intermediate.addConstantUnion(true, rightNode->getLoc(), true);
                    TIntermTyped *arg = m_intermediate.addBinaryMath(op, lhs, rhs, rightNode->getLoc());
                    m_context->handleFunctionArgument(constructor, argumentsNode, arg);
                }
                TIntermTyped *callNode = m_context->handleFunctionCall(leftNode->getLoc(), constructor, argumentsNode);
                TType *returnType = new TType(EbtVoid);
                TFunction *all = new TFunction(NewPoolTString("all"), *returnType);
                argumentsNode = nullptr;
                m_context->handleFunctionArgument(all, argumentsNode, callNode);
                exprNode = m_context->handleFunctionCall(leftNode->getLoc(), all, argumentsNode);
            }
        }
        else if (rightNode) {
            exprNode = m_intermediate.addBinaryMath(op, leftNode, rightNode, rightNode->getLoc());
            if (op == EOpDiv && !isNonZeroConstant(rightNode) && rightNode->isFloatingDomain() &&
                (exprNode->isScalar() || exprNode->isVector())) {
                exprNode = castNaNToZero(exprNode, exprNode->getType(), rightNode->getLoc());
            }
        }
    }
    return allocateIntermNode(exprNode);
}

atom_t
ParserContext::acceptConstructorCall(atom_t type, atom_t arguments)
{
    TIntermTyped *constructorNode = nullptr, *toTypeNode = resolveIntermNode(type)->getAsTyped();
    TIntermAggregate *argumentsNode = resolveIntermAggregate(arguments);
    if (toTypeNode && argumentsNode) {
        constructorNode = m_context->handleConstructor(toTypeNode->getLoc(), argumentsNode, toTypeNode->getType());
        constructorNode = constructorNode ? flattenAggregateNode(constructorNode)->getAsTyped() : nullptr;
    }
    return allocateIntermNode(constructorNode);
}

atom_t
ParserContext::acceptCastOperation(atom_t type, atom_t sizeExpr, atom_t expr)
{
    TIntermTyped *callerNode = nullptr, *exprNode = resolveIntermNode(expr)->getAsTyped();
    TIntermSymbol *destTypeNode = resolveIntermNode(type)->getAsSymbolNode();
    if (destTypeNode && exprNode) {
        const TSourceLoc &loc = exprNode->getLoc();
        TType destType;
        destType.shallowCopy(destTypeNode->getType());
        handleArraySizeSpecifier(resolveIntermNode(sizeExpr)->getAsAggregate(), destType);
        if (exprNode->isVector() && destType.isStruct()) {
            int offset = 0;
            TFunction *func = m_context->makeConstructorCall(loc, destType);
            TIntermTyped *argumentsNode = nullptr;
            for (const auto &it : *destType.getStruct()) {
                const TType *item = it.type;
                const int vectorSize = item->getVectorSize(), vectorSizeWithOffset = offset + vectorSize;
                if (vectorSizeWithOffset <= 4) {
                    TSwizzleSelectors<TVectorSelector> selector;
                    for (int i = offset; i < vectorSizeWithOffset; i++) {
                        selector.push_back(i);
                    }
                    TIntermTyped *indexNode = m_intermediate.addIndex(
                        EOpVectorSwizzle, exprNode, m_intermediate.addSwizzle(selector, loc), loc);
                    indexNode->setType(*item);
                    m_context->handleFunctionArgument(func, argumentsNode, indexNode);
                    offset += vectorSize;
                }
            }
            callerNode = m_context->handleFunctionCall(loc, func, argumentsNode);
        }
        else {
            callerNode = m_context->handleConstructor(loc, exprNode, destType);
        }
    }
    return allocateIntermNode(callerNode);
}

atom_t
ParserContext::setGlobalVariableTypeSpec(atom_t declarations, atom_t type, atom_t variables)
{
    TIntermAggregate *declarationListNode = resolveIntermAggregate(declarations),
                     *variablesNode = resolveIntermAggregate(variables);
    TIntermTyped *typeNode = resolveIntermNode(type)->getAsTyped();
    if (variablesNode && typeNode) {
        const TIntermSequence &sequence = variablesNode->getSequence();
        for (const auto &it : sequence) {
            TIntermBinary *variableNode = it->getAsBinaryNode(),
                          *identifierNode = variableNode->getLeft()->getAsBinaryNode();
            const TIntermSymbol *nameNode = identifierNode->getLeft()->getAsSymbolNode();
            handleArraySizeSpecifier(identifierNode->getRight(), typeNode->getWritableType());
            TIntermBinary *assignmentNode = variableNode->getRight()->getAsBinaryNode();
            TIntermTyped *initializerNode = assignmentNode->getLeft();
            TIntermBinary *semanticAnnotationNode =
                assignmentNode->getRight() ? assignmentNode->getRight()->getAsBinaryNode() : nullptr;
            bool hasAnnotations = false;
            if (semanticAnnotationNode) {
                if (TIntermNode *rightNode = semanticAnnotationNode->getRight()) {
                    const TIntermAggregate *annotationsNode = rightNode->getAsAggregate();
                    AnnotationList annotations;
                    convertAllAnnotations(annotationsNode, annotations);
                    m_annotations.insert(std::make_pair(nameNode->getName().c_str(), annotations));
                    hasAnnotations = !annotations.empty();
                }
            }
            const TType &specType = typeNode->getType();
            TQualifier &mutableQualifier = typeNode->getWritableType().getQualifier();
            const TString &name = nameNode->getName();
            const bool isConstDeclared = specType.getQualifier().isConstant(),
                       isUniform = mutableQualifier.storage != EvqGlobal && !isConstDeclared;
            mutableQualifier.specConstant = false;
            if (specType.isOpaque() && specType.getSampler().isTexture()) {
                TextureNodeItem textureValue = { new TType(), semanticAnnotationNode, name };
                textureValue.m_type->shallowCopy(specType);
                m_textureNodes.insert(std::make_pair(name, textureValue));
            }
            else if (isUniform) {
                initializerNode =
                    normalizeGlobalUniformInitializer(typeNode, name, initializerNode, semanticAnnotationNode);
            }
            else if (isInitializerNodeConst(initializerNode)) {
                if (isConstDeclared) {
                    mutableQualifier.storage = EvqConst;
                    /* normalize only scalar type */
                    if ((specType.isVector() || specType.isMatrix()) && initializerNode->isScalar()) {
                        initializerNode = m_context->handleConstructor(typeNode->getLoc(), initializerNode, specType);
                    }
                    NodeItem nodeItem(0, newTString(name), nullptr, new TType(), initializerNode);
                    nodeItem.m_type->shallowCopy(specType);
                    m_constantNodes.insert(std::make_pair(name, nodeItem));
                }
                else {
                    /* prefer to use annotation initialized value than default initialized value */
                    NodeItem nodeItem(m_lastUniformRegisterIndex, newTString(name), semanticAnnotationNode, new TType(),
                        initializerNode);
                    nodeItem.m_type->shallowCopy(specType);
                    incrementRegisterIndex(specType);
                    m_uniformNodes.insert(std::make_pair(name, nodeItem));
                }
            }
            else if (initializerNode) {
                initializerNode = normalizeGlobalStaticInitializer(typeNode, initializerNode);
            }
            if (initializerNode) {
                TString *nameWithPrefix = NewPoolTString(isConstDeclared
                        ? kGlobalConstantVariableNamePrefix
                        : (isUniform ? kGlobalUniformVariableNamePrefix : kGlobalStaticVariableNamePrefix));
                nameWithPrefix->append(name);
                TType variableType;
                variableType.shallowCopy(specType);
                variableType.getQualifier().storage = EvqGlobal;
                /* skip redeclaration if already declared */
                if (!m_context->symbolTable.find(*nameWithPrefix)) {
                    m_context->declareVariable(
                        initializerNode->getLoc(), *nameWithPrefix, variableType, initializerNode);
                }
                TIntermTyped *variableNode = m_context->handleVariable(initializerNode->getLoc(), nameWithPrefix);
                declarationListNode = m_intermediate.growAggregate(declarationListNode, variableNode);
                if (isUniform) {
                    m_uniformParameters.insert(std::make_pair(name, initializerNode));
                }
                else if (mutableQualifier.storage == EvqConst) {
                    m_immediateConstantParameters.insert(std::make_pair(name, initializerNode));
                }
                else if (isConstDeclared) {
                    m_expressionConstantParameters.insert(std::make_pair(name, initializerNode));
                }
                else {
                    auto &sp = m_staticParameters[nullptr];
                    sp.insert(std::make_pair(name, initializerNode));
                }
                m_globalInitializerNameOrder.push_back(name);
            }
            else {
                TType variableType;
                TString *name = newTString(nameNode->getName());
                variableType.shallowCopy(typeNode->getType());
                m_context->declareVariable(typeNode->getLoc(), *name, variableType);
            }
        }
    }
    return allocateIntermNode(declarationListNode);
}

atom_t
ParserContext::acceptGlobalVariable(atom_t variables, atom_t newVariable)
{
    TIntermNode *variablesNode = resolveIntermAggregate(variables);
    if (TIntermTyped *newVariableNode = resolveIntermNode(newVariable)->getAsTyped()) {
        variablesNode = growAggregateNode(variablesNode, newVariableNode);
    }
    return allocateIntermNode(variablesNode);
}

atom_t
ParserContext::createGlobalVariable(atom_t name, atom_t sizeExpr, atom_t initializer, atom_t semanticAnnotations)
{
    TIntermBinary *nameNode = new TIntermBinary(EOpNull);
    nameNode->setLoc(resolveIntermNode(name)->getLoc());
    nameNode->setLeft(resolveIntermNode(name)->getAsSymbolNode());
    nameNode->setRight(resolveIntermNode(sizeExpr)->getAsTyped());
    TIntermBinary *assignmentNode = new TIntermBinary(EOpNull);
    assignmentNode->setLeft(resolveIntermNode(initializer)->getAsTyped());
    assignmentNode->setRight(resolveIntermNode(semanticAnnotations)->getAsTyped());
    TIntermBinary *variableNode = new TIntermBinary(EOpNull);
    variableNode->setLoc(nameNode->getLoc());
    variableNode->setLeft(nameNode);
    variableNode->setRight(assignmentNode);
    return allocateIntermNode(variableNode);
}

atom_t
ParserContext::acceptFunction(atom_t declaration, atom_t semantic, atom_t body)
{
    InternalNode *declarationNode = resolveNode(declaration);
    TFunction *functionPtr = declarationNode->m_functionPtr;
    TIntermAggregate *node = declarationNode->m_value->getAsAggregate();
    atom_t result = declaration;
    if (TIntermAggregate *bodyNode = resolveIntermAggregate(body)) {
        bodyNode->setOperator(EOpSequence);
        node = m_intermediate.growAggregate(node, bodyNode);
        m_intermediate.setAggregateOperator(node, EOpFunction, functionPtr->getType(), bodyNode->getLoc());
        node->setName(functionPtr->getMangledName());
        m_allFunctions[functionPtr->getName()].push_back(std::make_pair(functionPtr, bodyNode));
        m_allFunctions[functionPtr->getMangledName()].push_back(std::make_pair(functionPtr, bodyNode));
        m_context->popScope();
        /* handle shader entry point such as "float4 vs_main() : SV_Position" */
        if (const TIntermBinary *binaryNode = resolveIntermNode(semantic)->getAsBinaryNode()) {
            const TIntermSymbol *semanticNode = binaryNode->getLeft()->getAsSymbolNode();
            const TString &semantic = semanticNode->getName();
            TString canonicalizedSemantic(semantic);
            std::transform(semantic.begin(), semantic.end(), canonicalizedSemantic.begin(), ::toupper);
            auto it2 = m_allFunctions.find(functionPtr->getName());
            if (it2 != m_allFunctions.end()) {
                TFunction *functionPtr = it2->second[0].first;
                setBuiltInSemantics(canonicalizedSemantic, functionPtr->getWritableType());
            }
        }
        m_currentFunction = nullptr;
        result = allocateIntermNode(node);
    }
    return result;
}

atom_t
ParserContext::createFunction(atom_t spec, atom_t name, atom_t parameters)
{
    atom_t index = 0;
    TIntermTyped *specNode = resolveIntermNode(spec)->getAsTyped();
    const TIntermSymbol *nameNode = resolveIntermNode(name)->getAsSymbolNode();
    if (specNode && nameNode) {
        TString *nameString = newTString(nameNode->getName());
        InternalNode &node = allocateNode(index);
        TType &type = specNode->getWritableType();
        if (type.getQualifier().storage == EvqTemporary && m_context->symbolTable.atGlobalLevel()) {
            type.getQualifier().storage = EvqGlobal;
        }
        TFunction *function = new TFunction(nameString, type);
        TVector<TBuiltInVariable> builtInArguments;
        if (const TIntermAggregate *functionArguments = resolveIntermAggregate(parameters)) {
            const TIntermSequence &parameters = functionArguments->getSequence();
            for (const auto &it : parameters) {
                if (TIntermBinary *binaryNode = it->getAsBinaryNode()) {
                    TIntermTyped *leftNode = binaryNode->getLeft();
                    if (const TIntermSymbol *symbolNode = leftNode->getAsSymbolNode()) {
                        const TType &symbolNodeType = symbolNode->getType();
                        if (symbolNodeType.getBasicType() != EbtVoid) {
                            TIntermTyped *defaultParameterNode = nullptr;
                            if (TIntermTyped *rightNode = binaryNode->getRight()) {
                                defaultParameterNode = handleSingleConstructorCall(
                                    rightNode->getLoc(), symbolNodeType, rightNode->getAsTyped());
                                if (TIntermAggregate *aggregateNode = defaultParameterNode->getAsAggregate()) {
                                    defaultParameterNode = m_intermediate.fold(aggregateNode);
                                }
                            }
                            TParameter parameter = { newTString(symbolNode->getName()), new TType(),
                                defaultParameterNode };
                            parameter.type->shallowCopy(symbolNodeType);
                            m_context->paramFix(*parameter.type);
                            function->addParameter(parameter);
                            const TBuiltInVariable builtIn = parameter.type->getQualifier().builtIn;
                            builtInArguments.push_back(builtIn);
                        }
                    }
                }
            }
        }
        m_builtInVariables.insert(std::make_pair(function->getMangledName(), builtInArguments));
        m_context->handleFunctionDeclarator(nameNode->getLoc(), *function, false);
        node.m_functionPtr = m_currentFunction = function;
        TAttributes attributes;
        TIntermNode *entryPointTree = nullptr;
        node.m_value = m_context->handleFunctionDefinition(nameNode->getLoc(), *function, attributes, entryPointTree);
    }
    return index;
}

atom_t
ParserContext::createFunctionParameter(atom_t spec, atom_t name, atom_t sizeExpr, atom_t semantic, atom_t initializer)
{
    TIntermTyped *node = nullptr;
    TIntermSymbol *specNode = resolveIntermNode(spec)->getAsSymbolNode(),
                  *nameNode = resolveIntermNode(name)->getAsSymbolNode();
    if (specNode) {
        TType type;
        type.shallowCopy(specNode->getType());
        handleArraySizeSpecifier(resolveIntermNode(sizeExpr)->getAsAggregate(), type);
        const TQualifier &q = type.getQualifier();
        if (!q.isParamInput() && !q.isParamOutput()) {
            type.getQualifier().makeTemporary();
        }
        TString *nameString = nameNode ? newTString(nameNode->getName()) : newAnonymousVariableString();
        TIntermSymbol *symbolNode = new TIntermSymbol(int(spec), *nameString, type);
        decorateSemanticType(semantic, symbolNode->getWritableType());
        symbolNode->setLoc(nameNode ? nameNode->getLoc() : specNode->getLoc());
        TIntermBinary *binaryNode = new TIntermBinary(EOpNull);
        binaryNode->setLeft(symbolNode);
        binaryNode->setRight(resolveIntermNode(initializer)->getAsTyped());
        node = binaryNode;
    }
    return allocateIntermNode(node);
}

atom_t
ParserContext::createBinaryOperator(TOperator op)
{
    return allocateIntermNode(new TIntermBinary(op));
}

atom_t
ParserContext::createUnaryOperator(TOperator op)
{
    return allocateIntermNode(new TIntermUnary(op));
}

atom_t
ParserContext::setTypeSpecQualifier(atom_t qualifier, atom_t specifier)
{
    const TIntermAggregate *qualifierNode = resolveIntermAggregate(qualifier);
    TIntermTyped *specifierNode = resolveIntermNode(specifier)->getAsTyped();
    if (qualifierNode && specifierNode) {
        bool hasConstant = false;
        const TIntermSequence &sequence = qualifierNode->getSequence();
        TQualifier &q = specifierNode->getWritableType().getQualifier();
        for (const auto &it : sequence) {
            const TIntermTyped *typedNode = it->getAsTyped();
            bool constant = typedNode->getQualifier().isConstant();
            hasConstant |= constant;
            if (!constant) {
                q = typedNode->getQualifier();
            }
        }
        q.specConstant = hasConstant;
    }
    m_currentTypeSpecNode = specifierNode;
    return allocateIntermNode(specifierNode);
}

atom_t
ParserContext::appendTypeQualifier(atom_t left, atom_t right)
{
    TIntermNode *leftNode = resolveIntermAggregate(left);
    if (TIntermTyped *rightNode = resolveIntermNode(right)->getAsTyped()) {
        leftNode = growAggregateNode(leftNode, rightNode);
    }
    return allocateIntermNode(leftNode);
}

atom_t
ParserContext::createType(const TType &type, const LexerToken *token)
{
    const TPpToken v(token->value);
    TIntermSymbol *symbolNode = nullptr;
    if (const TSymbol *symbol = m_context->symbolTable.find(v.name)) {
        const TVariable *variable = symbol->getAsVariable();
        symbolNode = m_intermediate.addSymbol(*variable, v.loc);
    }
    else {
        TString *name = NewPoolTString(v.name);
        TVariable variable(name, type);
        variable.setUniqueId(m_uniqueID++);
        symbolNode = m_intermediate.addSymbol(variable, v.loc);
    }
    return allocateIntermNode(symbolNode);
}

atom_t
ParserContext::createType(const TBasicType &v, const LexerToken *token)
{
    const TType type(v, m_context->symbolTable.atGlobalLevel() ? EvqUniform : EvqTemporary);
    return createType(type, token);
}

atom_t
ParserContext::createTextureType(const TSamplerDim &dim, const LexerToken *token)
{
    TPublicType publicType;
    publicType.init(token->value.loc, true);
    publicType.basicType = EbtSampler;
    publicType.sampler.setTexture(EbtFloat, dim);
    return createType(TType(publicType), token);
}

atom_t
ParserContext::createSamplerType(const TSamplerDim &dim, const LexerToken *token)
{
    TPublicType publicType;
    publicType.init(token->value.loc, true);
    publicType.basicType = EbtSampler;
    publicType.sampler.set(EbtFloat, dim);
    return createType(TType(publicType), token);
}

atom_t
ParserContext::createVectorType(const TBasicType &v, int numComponents, const LexerToken *token)
{
    const TType type(v, m_context->symbolTable.atGlobalLevel() ? EvqUniform : EvqTemporary, numComponents);
    return createType(type, token);
}

atom_t
ParserContext::createMatrixType(const TBasicType &v, int numRows, int numColumns, const LexerToken *token)
{
    TType type(v, m_context->symbolTable.atGlobalLevel() ? EvqUniform : EvqTemporary, 0, numRows, numColumns);
    type.getQualifier().layoutMatrix = ElmColumnMajor;
    return createType(type, token);
}

atom_t
ParserContext::createStructType(atom_t name, atom_t body)
{
    atom_t atom = 0;
    if (const TIntermSymbol *nameNode = resolveIntermNode(name)->getAsSymbolNode()) {
        TString *nameString = newTString(nameNode->getName());
        if (TIntermAggregate *bodyNode = resolveIntermAggregate(body)) {
            TTypeList *typeList = new TTypeList();
            const TIntermSequence &bodyNodeSequence = bodyNode->getSequence();
            auto &builtInStructMembers = m_builtInStructMembers[*nameString];
            for (const auto &it : bodyNodeSequence) {
                if (const TIntermAggregate *memberNode = it->getAsAggregate()) {
                    const TIntermSequence &memberNodeSequence = memberNode->getSequence();
                    for (const auto &it2 : memberNodeSequence) {
                        if (const TIntermSymbol *symbol = it2->getAsSymbolNode()) {
                            TType *type = new TType();
                            type->shallowCopy(symbol->getType());
                            type->getQualifier().builtIn = EbvNone;
                            TTypeLoc item = { type, symbol->getLoc() };
                            typeList->push_back(item);
                            builtInStructMembers[symbol->getName()] = symbol->getQualifier().builtIn;
                        }
                    }
                }
            }
            const TType type(typeList, nameNode->getName());
            const LexerToken token = { TPpToken(), 0 };
            atom = createType(type, &token);
            TVariable *userTypeDef = new TVariable(nameString, type, true);
            m_context->symbolTable.insert(*userTypeDef);
        }
    }
    return atom;
}

atom_t
ParserContext::appendStructMember(atom_t body, atom_t member)
{
    TIntermNode *bodyNode = resolveIntermAggregate(body);
    if (TIntermTyped *memberNode = resolveIntermNode(member)->getAsTyped()) {
        bodyNode = growAggregateNode(bodyNode, memberNode);
    }
    return allocateIntermNode(bodyNode);
}

atom_t
ParserContext::createStructMember(atom_t spec, atom_t member)
{
    TIntermAggregate *memberNode = resolveIntermAggregate(member);
    if (const TIntermTyped *baseType = resolveIntermNode(spec)->getAsTyped()) {
        const TIntermSequence &sequence = memberNode->getSequence();
        for (const auto &it : sequence) {
            TIntermSymbol *symbolNode = it->getAsSymbolNode();
            TArraySizes *arraySizes = nullptr;
            if (const TArraySizes *sizes = symbolNode->getType().getArraySizes()) {
                arraySizes = new TArraySizes();
                *arraySizes = *sizes;
            }
            TType &type = symbolNode->getWritableType();
            const TQualifier qualifier = type.getQualifier();
            type.shallowCopy(baseType->getType());
            type.getQualifier().builtIn = qualifier.builtIn;
            type.setFieldName(symbolNode->getName());
            if (arraySizes) {
                type.copyArraySizes(*arraySizes);
            }
        }
    }
    return allocateIntermNode(memberNode);
}

atom_t
ParserContext::appendStructMemberDeclarator(atom_t member, atom_t identifier)
{
    TIntermNode *memberNode = resolveIntermAggregate(member);
    if (TIntermSymbol *symbolNode = resolveIntermNode(identifier)->getAsSymbolNode()) {
        memberNode = growAggregateNode(memberNode, symbolNode);
    }
    return allocateIntermNode(memberNode);
}

atom_t
ParserContext::createStructMemberDeclarator(atom_t identifier, atom_t sizeExpr, atom_t semantic)
{
    TIntermSymbol *node = nullptr;
    if (const TIntermSymbol *symbol = resolveIntermNode(identifier)->getAsSymbolNode()) {
        TType type;
        handleArraySizeSpecifier(resolveIntermNode(sizeExpr)->getAsAggregate(), type);
        node = new TIntermSymbol(int(identifier), symbol->getName(), type);
        decorateSemanticType(semantic, node->getWritableType());
        node->setLoc(symbol->getLoc());
    }
    return allocateIntermNode(node);
}

atom_t
ParserContext::createSamplerState(atom_t type, atom_t identifier, atom_t index, atom_t states)
{
    TIntermNode *node = nullptr;
    const TIntermTyped *typeNode = resolveIntermNode(type)->getAsTyped();
    const TIntermSymbol *nameNode = resolveIntermNode(identifier)->getAsSymbolNode();
    TIntermBinary *semanticAnnotationNode = nullptr;
    if (typeNode && nameNode) {
        /* handle "register(s0)" */
        bool hasTextureSamplerAccessor = false;
        if (TIntermSymbol *registerIndexNode = resolveIntermNode(index)->getAsSymbolNode()) {
            const TString &registerIndexString = registerIndexNode->getName();
            const TType &symbolType = typeNode->getType();
            size_t samplerIndex = registerIndexString.size() > 1
                ? std::min(std::max(int(strtol(registerIndexString.c_str() + 1, 0, 10)), 0), 15)
                : 0;
            TString overridenSamplerName;
            node = createTextureSamplerAccessor(symbolType, nameNode, samplerIndex, nullptr, semanticAnnotationNode);
            hasTextureSamplerAccessor = true;
        }
        /* handle sampler_state { ... } */
        if (TIntermAggregate *innerBlockNode = resolveIntermAggregate(states)) {
            const TIntermSequence &sequence = innerBlockNode->getSequence();
            for (const auto &it : sequence) {
                if (const TIntermSymbol *symbolName = it->getAsSymbolNode()) {
                    TString *samplerIndexString = newTString(symbolName->getName());
                    if (TSymbol *symbol = m_context->symbolTable.find(*samplerIndexString)) {
                        if (!hasTextureSamplerAccessor) {
                            node = createTextureSamplerAccessor(
                                symbol->getType(), nameNode, SIZE_MAX, innerBlockNode, semanticAnnotationNode);
                            hasTextureSamplerAccessor = true;
                        }
                        bindTextureToSampler(*samplerIndexString, nameNode->getName());
                    }
                }
                else if (const TIntermBinary *binaryNode = it->getAsBinaryNode()) {
                    const TIntermSymbol *keyNode = binaryNode->getLeft()->getAsSymbolNode();
                    if (keyNode) {
                        TString key(keyNode->getName());
                        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
                        if (key == "texture") {
                            const TIntermSymbol *valueNode = binaryNode->getRight()->getAsSymbolNode();
                            const TString &textureNodeName = valueNode->getName();
                            if (TSymbol *symbol = m_context->symbolTable.find(textureNodeName)) {
                                if (!hasTextureSamplerAccessor) {
                                    node = createTextureSamplerAccessor(
                                        symbol->getType(), nameNode, SIZE_MAX, innerBlockNode, semanticAnnotationNode);
                                    hasTextureSamplerAccessor = true;
                                }
                                bindTextureToSampler(textureNodeName, nameNode->getName());
                            }
                        }
                    }
                }
            }
        }
    }
    return allocateIntermNode(node);
}

atom_t
ParserContext::appendSamplerStateValue(atom_t states, atom_t newState)
{
    TIntermNode *statesNode = resolveIntermAggregate(states);
    if (TIntermTyped *newStateNode = resolveIntermNode(newState)->getAsTyped()) {
        statesNode = growAggregateNode(statesNode, newStateNode);
    }
    return allocateIntermNode(statesNode);
}

atom_t
ParserContext::createSamplerStateTexture(atom_t identifier)
{
    TIntermNode *node = nullptr;
    if (TIntermSymbol *nameNode = resolveIntermNode(identifier)->getAsSymbolNode()) {
        TIntermBinary *binaryNode = new TIntermBinary(EOpNull);
        TIntermSymbol *keyNode = new TIntermSymbol(0, "texture", TType());
        binaryNode->setLoc(keyNode->getLoc());
        binaryNode->setLeft(keyNode);
        binaryNode->setRight(nameNode);
        node = binaryNode;
    }
    return allocateIntermNode(node);
}

atom_t
ParserContext::createSamplerStateValue(atom_t identifier, atom_t value)
{
    TIntermNode *node = nullptr;
    TIntermTyped *keyNode = resolveIntermNode(identifier)->getAsTyped();
    TIntermTyped *valueNode = resolveIntermNode(value)->getAsTyped();
    if (keyNode && valueNode) {
        TIntermBinary *binaryNode = new TIntermBinary(EOpNull);
        binaryNode->setLoc(keyNode->getLoc());
        binaryNode->setLeft(keyNode);
        binaryNode->setRight(valueNode);
        node = binaryNode;
    }
    return allocateIntermNode(node);
}

atom_t
ParserContext::appendAnnotation(atom_t annotations, atom_t newAnnotation)
{
    TIntermNode *annotationsNode = resolveIntermAggregate(annotations);
    if (TIntermBinary *newAnnotationNode = resolveIntermNode(newAnnotation)->getAsBinaryNode()) {
        annotationsNode = growAggregateNode(annotationsNode, newAnnotationNode);
    }
    return allocateIntermNode(annotationsNode);
}

atom_t
ParserContext::createAnnotation(atom_t type, atom_t identifier, atom_t initializer)
{
    TIntermBinary *annotationNode = nullptr;
    TIntermTyped *typeNode = resolveIntermNode(type)->getAsTyped();
    TIntermSymbol *nameNode = resolveIntermNode(identifier)->getAsSymbolNode();
    if (nameNode) {
        if (!typeNode) {
            typeNode = new TIntermSymbol(int(identifier), "", TType(EbtString));
        }
        TIntermBinary *identifierNode = new TIntermBinary(EOpNull);
        identifierNode->setLoc(typeNode->getLoc());
        identifierNode->setLeft(typeNode);
        identifierNode->setRight(nameNode);
        annotationNode = new TIntermBinary(EOpNull);
        annotationNode->setLoc(identifierNode->getLoc());
        annotationNode->setLeft(identifierNode);
        annotationNode->setRight(resolveIntermNode(initializer)->getAsTyped());
    }
    return allocateIntermNode(annotationNode);
}

atom_t
ParserContext::appendStringList(atom_t stringList, atom_t newString)
{
    TIntermNode *stringsNode = resolveIntermAggregate(stringList);
    if (TIntermSymbol *newStringNode = resolveIntermNode(newString)->getAsSymbolNode()) {
        stringsNode = growAggregateNode(stringsNode, newStringNode);
    }
    return allocateIntermNode(stringsNode);
}

atom_t
ParserContext::createSemanticAnnotations(atom_t semantic, atom_t annotations)
{
    TIntermBinary *node = nullptr;
    TIntermSymbol *semanticNode = resolveIntermNode(semantic)->getAsSymbolNode();
    TIntermAggregate *annotationNode = resolveIntermAggregate(annotations);
    if (semanticNode || annotationNode) {
        node = new TIntermBinary(EOpNull);
        if (semanticNode) {
            node->setLoc(semanticNode->getLoc());
        }
        else if (annotationNode) {
            node->setLoc(annotationNode->getLoc());
        }
        node->setLeft(semanticNode);
        node->setRight(annotationNode);
    }
    return allocateIntermNode(node);
}

atom_t
ParserContext::createTechnique(atom_t identifier, atom_t passes, atom_t annotations)
{
    TIntermBinary *node = nullptr;
    if (TIntermSymbol *nameNode = resolveIntermNode(identifier)->getAsSymbolNode()) {
        TIntermAggregate *passesNode = resolveIntermAggregate(passes);
        node = new TIntermBinary(EOpNull);
        node->setLoc(nameNode->getLoc());
        node->setLeft(nameNode);
        node->setRight(passesNode);
        Technique technique(nameNode->getName());
        convertTechnique(passesNode, m_stateBlocks, technique);
        convertAllAnnotations(resolveIntermAggregate(annotations), technique.m_annotations);
        m_techniques.push_back(technique);
    }
    return allocateIntermNode(node);
}

atom_t
ParserContext::appendPass(atom_t passes, atom_t newPass)
{
    TIntermNode *passesNode = resolveIntermAggregate(passes);
    if (TIntermBinary *newPassNode = resolveIntermNode(newPass)->getAsBinaryNode()) {
        passesNode = growAggregateNode(passesNode, newPassNode);
    }
    return allocateIntermNode(passesNode);
}

atom_t
ParserContext::createPass(atom_t identifier, atom_t states, atom_t annotations)
{
    TIntermBinary *passNode = nullptr;
    if (TIntermSymbol *nameNode = resolveIntermNode(identifier)->getAsSymbolNode()) {
        passNode = new TIntermBinary(EOpNull);
        passNode->setLoc(nameNode->getLoc());
        passNode->setLeft(nameNode);
        TIntermBinary *valueNode = new TIntermBinary(EOpNull);
        valueNode->setLoc(passNode->getLoc());
        TIntermAggregate *compoundNode = resolveIntermAggregate(states);
        valueNode->setLeft(compoundNode);
        TIntermAggregate *annotationsNode = resolveIntermAggregate(annotations);
        valueNode->setRight(annotationsNode);
        passNode->setRight(valueNode);
    }
    return allocateIntermNode(passNode);
}

atom_t
ParserContext::createStateBlock(atom_t identifier, atom_t block)
{
    TIntermSymbol *symbolNode = resolveIntermNode(identifier)->getAsSymbolNode();
    TIntermAggregate *blockNode = resolveIntermAggregate(block);
    TIntermNode *stateBlockNode = nullptr;
    if (symbolNode && blockNode) {
        m_stateBlocks.insert(std::make_pair(symbolNode->getName(), blockNode));
    }
    return allocateIntermNode(stateBlockNode);
}

atom_t
ParserContext::appendPassState(atom_t states, atom_t newState)
{
    TIntermNode *passStatesNode = resolveIntermAggregate(states);
    if (TIntermBinary *newPassStateNode = resolveIntermNode(newState)->getAsBinaryNode()) {
        passStatesNode = growAggregateNode(passStatesNode, newPassStateNode);
    }
    return allocateIntermNode(passStatesNode);
}

atom_t
ParserContext::createPassState(atom_t identifier, atom_t value)
{
    TIntermBinary *passStateNode = nullptr;
    if (TIntermSymbol *nameNode = resolveIntermNode(identifier)->getAsSymbolNode()) {
        passStateNode = new TIntermBinary(EOpNull);
        passStateNode->setLoc(nameNode->getLoc());
        passStateNode->setLeft(nameNode);
        passStateNode->setRight(resolveIntermNode(value)->getAsTyped());
    }
    return allocateIntermNode(passStateNode);
}

atom_t
ParserContext::createPassEntryPoint(atom_t type, atom_t identifier, atom_t arguments)
{
    TIntermBinary *node = nullptr;
    if (TIntermSymbol *nameNode = resolveIntermNode(type)->getAsSymbolNode()) {
        node = new TIntermBinary(EOpNull);
        node->setLoc(nameNode->getLoc());
        node->setLeft(nameNode);
        TIntermBinary *valueNode = new TIntermBinary(EOpNull);
        valueNode->setLoc(node->getLoc());
        TIntermSymbol *passNameNode = resolveIntermNode(identifier)->getAsSymbolNode();
        valueNode->setLeft(passNameNode);
        TIntermAggregate *argumentsNode = resolveIntermAggregate(arguments);
        valueNode->setRight(argumentsNode);
        node->setRight(valueNode);
    }
    return allocateIntermNode(node);
}

atom_t
ParserContext::acceptAttribute(atom_t attrs, atom_t name, atom_t value)
{
    TIntermNode *attributesNode = resolveIntermAggregate(attrs);
    if (TIntermSymbol *nameNode = resolveIntermNode(name)->getAsSymbolNode()) {
        TIntermBinary *node = new TIntermBinary(EOpNull);
        node->setLoc(nameNode->getLoc());
        node->setLeft(nameNode);
        node->setRight(resolveIntermNode(value)->getAsTyped());
        attributesNode = growAggregateNode(attributesNode, node);
    }
    return allocateIntermNode(attributesNode);
}

atom_t
ParserContext::setAllAttributes(atom_t statement, atom_t attrs)
{
    if (TIntermAggregate *attributesNode = resolveIntermAggregate(attrs)) {
        TAttributes attributes;
        for (const TIntermNode *itemNode : attributesNode->getSequence()) {
            if (const TIntermBinary *binaryNode = itemNode->getAsBinaryNode()) {
                const TIntermSymbol *leftNode = binaryNode->getLeft()->getAsSymbolNode();
                TIntermTyped *rightNode = binaryNode->getLeft()->getAsTyped();
                TAttributeArgs arg;
                arg.name = m_context->attributeFromName(TString(), leftNode->getName());
                arg.args = m_intermediate.growAggregate(nullptr, rightNode);
                attributes.push_back(arg);
            }
        }
        if (TIntermLoop *loopNode = resolveIntermNode(statement)->getAsLoopNode()) {
            m_context->handleLoopAttributes(loopNode->getLoc(), loopNode, attributes);
        }
        else if (TIntermSelection *branchNode = resolveIntermNode(statement)->getAsSelectionNode()) {
            m_context->handleSelectionAttributes(branchNode->getLoc(), branchNode, attributes);
        }
        else if (TIntermSwitch *switchNode = resolveIntermNode(statement)->getAsSwitchNode()) {
            m_context->handleSwitchAttributes(switchNode->getLoc(), switchNode, attributes);
        }
    }
    return statement;
}

atom_t
ParserContext::flattenExpression(atom_t expression)
{
    return allocateIntermNode(flattenAggregateNode(resolveIntermNode(expression)));
}

void
ParserContext::pushScope()
{
    m_context->pushScope();
}

void
ParserContext::popScope()
{
    m_context->popScope();
}

void
ParserContext::nestLooping()
{
    m_context->nestLooping();
}

void
ParserContext::unnestLooping()
{
    m_context->nestLooping();
}

void
ParserContext::concludeLocalVariableDeclaration()
{
    m_currentTypeSpecNode = nullptr;
}

void
ParserContext::enableAcceptingVariable()
{
    m_enableAcceptingVariable = true;
}

void
ParserContext::disableAcceptingVariable()
{
    m_enableAcceptingVariable = false;
}

void
ParserContext::enableUniformBuffer()
{
    m_enableUniformBuffer = true;
}

void
ParserContext::setVertexShaderInputMap(const BuiltInLocationMap &value)
{
    m_vertexShaderInputMap = value;
}

void
ParserContext::setPixelShaderInputMap(const BuiltInLocationMap &value)
{
    m_pixelShaderInputMap = value;
}

void
ParserContext::setPointSizeAssignment(float value)
{
    m_pointSizeAssignment = value;
}

atom_t
ParserContext::allocateIntermNode(TIntermNode *node)
{
    const size_t numNodes = m_nodes.size();
    atom_t index = 0;
    for (size_t i = 0; i < numNodes; i++) {
        if (m_nodes[i].m_value == node) {
            index = static_cast<atom_t>(i);
            break;
        }
    }
    if (!index && node) {
        allocateNode(index).m_value = node;
    }
    return index;
}

atom_t
ParserContext::acceptTrueLiteral(const LexerToken *token)
{
    TIntermConstantUnion *node = m_intermediate.addConstantUnion(true, token->value.loc, true);
    return allocateIntermNode(node);
}

atom_t
ParserContext::acceptFalseLiteral(const LexerToken *token)
{
    TIntermConstantUnion *node = m_intermediate.addConstantUnion(false, token->value.loc, true);
    return allocateIntermNode(node);
}

atom_t
ParserContext::acceptIntLiteral(const LexerToken *token)
{
    TIntermConstantUnion *node = m_intermediate.addConstantUnion(token->value.ival, token->value.loc, true);
    return allocateIntermNode(node);
}

atom_t
ParserContext::acceptFloatLiteral(const LexerToken *token)
{
    TIntermConstantUnion *node = m_intermediate.addConstantUnion(token->value.dval, EbtFloat, token->value.loc, true);
    return allocateIntermNode(node);
}

atom_t
ParserContext::acceptStringLiteral(const LexerToken *token)
{
    return acceptIdentifier(token);
}

atom_t
ParserContext::acceptIdentifier(const LexerToken *token)
{
    TIntermSymbol *symbol = nullptr;
    const TString &value = token->value.name;
    if (m_reservedWordSet.find(value) != m_reservedWordSet.end()) {
        TString valueWithPrefix("__fx9_");
        valueWithPrefix.append(value);
        symbol = new TIntermSymbol(m_uniqueID++, valueWithPrefix, TType(EbtString));
    }
    else {
        symbol = new TIntermSymbol(m_uniqueID++, value, TType(EbtString));
    }
    symbol->setLoc(token->value.loc);
    return allocateIntermNode(symbol);
}

glslang::TString *
ParserContext::newAnonymousVariableString()
{
    return newAnonymousVariableString(m_uniqueID++);
}

ParserContext::InternalNode &
ParserContext::allocateNode(atom_t &index)
{
    index = static_cast<atom_t>(m_nodes.size());
    m_nodes.push_back(InternalNode());
    return m_nodes[index];
}

void
ParserContext::createGlobalUniformVariable(int size)
{
    if (const TString *variableName = globalUniformFloat4Name()) {
        TArraySizes sizes;
        sizes.addInnerSize(size);
        TType type(EbtFloat, EvqUniform, 4);
        type.copyArraySizes(sizes);
        if (m_enableUniformBuffer) {
            TTypeLoc member = { new TType(), TSourceLoc() };
            member.type->shallowCopy(type);
            member.type->setFieldName(*variableName);
            TTypeList *typeList = new TTypeList();
            typeList->push_back(member);
            TString *blockName = newTString(*variableName);
            blockName->append("_buffer");
            TType blockType(typeList, *blockName, TType(EbtBlock, EvqUniform).getQualifier());
            m_context->declareBlock(TSourceLoc(), blockType);
        }
        else {
            m_context->declareVariable(TSourceLoc(), *variableName, type);
        }
    }
}

const TString *
ParserContext::currentFunctionName() const
{
    TString *functionName = nullptr;
    if (m_currentFunction) {
        functionName = newTString(m_currentFunction->getName());
        auto it = functionName->find("(");
        if (it != std::string::npos) {
            functionName->erase(it);
        }
    }
    return functionName;
}

TIntermTyped *
ParserContext::convertFunctionAssignment(TIntermAggregate *callerNode)
{
    TIntermTyped *node = callerNode;
    if (callerNode && callerNode->getOp() == EOpFunctionCall) {
        auto it = m_allFunctions.find(callerNode->getName());
        if (it != m_allFunctions.end()) {
            TString *variableName = NewPoolTString("_var_");
            variableName->append(callerNode->getName());
            variableName->pop_back();
            TSymbol *symbol = m_context->symbolTable.find(*variableName);
            if (symbol) {
                node = m_context->handleVariable(callerNode->getLoc(), variableName);
            }
            else {
                TType variableType;
                variableType.shallowCopy(callerNode->getType());
                TIntermNode *initializerNode =
                    m_context->declareVariable(callerNode->getLoc(), *variableName, variableType, callerNode);
                node = m_intermediate.growAggregate(node, initializerNode);
                TIntermTyped *variableNode = m_context->handleVariable(callerNode->getLoc(), variableName);
                node = m_intermediate.growAggregate(node, variableNode);
                node->setLoc(callerNode->getLoc());
                node->getAsAggregate()->setOperator(EOpSequence);
            }
        }
    }
    return node;
}

TIntermTyped *
ParserContext::findGlobalVariableNode(const TIntermSymbol *symbolNode)
{
    /* skip local defined variable */
    const TString &name = symbolNode->getName();
    const TSymbol *symbol = m_context->symbolTable.find(name);
    return symbol ? nullptr : findGlobalVariableNode(name, symbolNode);
}

TIntermTyped *
ParserContext::findGlobalVariableNode(const TString &name, const TIntermNode *node)
{
    TString *nameWithPrefix = nullptr;
    auto findGlobalVariable = [&nameWithPrefix](
                                  const char *prefix, const TIntermTypedNodeMap &nodes, const TString &name) {
        const auto it = nodes.find(name);
        if (it != nodes.end()) {
            nameWithPrefix = NewPoolTString(prefix);
            nameWithPrefix->append(name);
        }
        return nameWithPrefix != nullptr;
    };
    auto findInnerStaticVariable = [this, &nameWithPrefix](const TString &name) {
        if (m_currentFunction) {
            const auto &map2 = m_staticParameters[m_currentFunction];
            auto it3 = map2.find(name);
            if (it3 != map2.end()) {
                nameWithPrefix = NewPoolTString(kGlobalStaticVariableNamePrefix);
                nameWithPrefix->append(m_currentFunction->getName());
                nameWithPrefix->append("_");
                nameWithPrefix->append(name);
            }
        }
        return nameWithPrefix != nullptr;
    };
    return (findGlobalVariable(kGlobalConstantVariableNamePrefix, m_immediateConstantParameters, name) ||
               findGlobalVariable(kGlobalConstantVariableNamePrefix, m_expressionConstantParameters, name) ||
               findGlobalVariable(kGlobalUniformVariableNamePrefix, m_uniformParameters, name) ||
               findGlobalVariable(kGlobalStaticVariableNamePrefix, m_staticParameters[nullptr], name) ||
               findInnerStaticVariable(name))
        ? m_context->handleVariable(node->getLoc(), nameWithPrefix)
        : nullptr;
}

TIntermTyped *
ParserContext::normalizeGlobalUniformInitializer(const TIntermTyped *typeNode, const TString &name,
    TIntermTyped *initializerNode, TIntermBinary *semanticAnnotationNode)
{
    const TType &specType = typeNode->getType();
    const TString *uniformName = globalUniformFloat4Name();
    TIntermTyped *uniformFloat4Node = m_context->handleVariable(typeNode->getLoc(), uniformName),
                 *fetchFromGlobalUniformInitializer = nullptr;
    const TSourceLoc &loc = initializerNode ? initializerNode->getLoc() : typeNode->getLoc();
    size_t uniformRegisterIndex = m_lastUniformRegisterIndex, registerSize = 0;
    int uniformRegisterIndexInt = int(uniformRegisterIndex);
    if (specType.isMatrix()) {
        /*
         * initializer = constructor(u_uniform[offset, ... offset + column * row])
         */
        if (TFunction *constructor = m_context->makeConstructorCall(typeNode->getLoc(), specType)) {
            TIntermTyped *argumentsNode = nullptr;
            for (int i = 0, numRows = specType.getMatrixRows(), numColumns = specType.getMatrixCols(); i < numRows;
                 i++) {
                TIntermConstantUnion *indexNode =
                    m_intermediate.addConstantUnion(uniformRegisterIndexInt + i, typeNode->getLoc(), true);
                TIntermTyped *uniformVectorNode =
                    m_context->handleBracketDereference(uniformFloat4Node->getLoc(), uniformFloat4Node, indexNode);
                TIntermTyped *swizzledNode = swizzleVectorNode(uniformVectorNode, numColumns);
                m_context->handleFunctionArgument(constructor, argumentsNode, swizzledNode);
            }
            fetchFromGlobalUniformInitializer = m_context->handleFunctionCall(loc, constructor, argumentsNode);
            registerSize = specType.getMatrixRows();
        }
    }
    else if (specType.isVector() || specType.isArray() || specType.isScalar()) {
        auto swizzleGlobalUniformNodeCall = [this, uniformFloat4Node, typeNode, uniformRegisterIndexInt, &specType](
                                                int index) {
            TIntermConstantUnion *indexNode =
                m_intermediate.addConstantUnion(uniformRegisterIndexInt + index, typeNode->getLoc(), true);
            TIntermTyped *uniformVectorNode =
                m_context->handleBracketDereference(uniformFloat4Node->getLoc(), uniformFloat4Node, indexNode);
            TIntermTyped *swizzleNode = swizzleVectorNode(uniformVectorNode, specType);
            /* float a = 0.0; bool b = (a != 0.0f); */
            return specType.getBasicType() == EbtBool ? castFloatToBool(swizzleNode) : swizzleNode;
        };
        int outerSize = 1;
        if (const TArraySizes *sizes = specType.getArraySizes()) {
            /*
             * initializer = constructor(u_uniform[offset + 0, ...  offset + n])
             */
            TFunction *function = m_context->makeConstructorCall(typeNode->getLoc(), specType);
            TIntermTyped *argumentsNode = nullptr;
            outerSize = sizes->getOuterSize();
            if (outerSize == 0 && initializerNode) {
                if (const TIntermAggregate *aggregateNode = initializerNode->getAsAggregate()) {
                    outerSize = aggregateNode->getSequence().size();
                }
            }
            for (int i = 0; i < outerSize; i++) {
                m_context->handleFunctionArgument(function, argumentsNode, swizzleGlobalUniformNodeCall(i));
            }
            fetchFromGlobalUniformInitializer =
                m_context->handleFunctionCall(typeNode->getLoc(), function, argumentsNode);
        }
        else {
            /*
             * initializer = constructor(u_uniform[offset])
             */
            fetchFromGlobalUniformInitializer = swizzleGlobalUniformNodeCall(0);
            if (initializerNode) {
                /* force integer domain initializer type as float e.g. "float a = 1;" */
                if (initializerNode->isIntegerDomain() && specType.isFloatingDomain()) {
                    initializerNode = handleSingleConstructorCall(loc, specType, initializerNode);
                }
                /* normalize sequential argument e.g. float4 value = { 0, 0, 0, 0 } */
                else if (TIntermAggregate *aggregateNode = initializerNode->getAsAggregate()) {
                    TIntermAggregate *newAggregateNode = new TIntermAggregate(EOpSequence);
                    TIntermSequence &mutableSequence = newAggregateNode->getSequence();
                    for (TIntermNode *node : aggregateNode->getSequence()) {
                        TIntermTyped *typedNode = node->getAsTyped();
                        TIntermNode *newNode = node;
                        if (typedNode && typedNode->isIntegerDomain()) {
                            newNode = handleSingleConstructorCall(loc, specType, typedNode);
                        }
                        mutableSequence.push_back(newNode);
                    }
                    initializerNode = newAggregateNode;
                }
            }
        }
        registerSize = outerSize;
    }
    m_lastUniformRegisterIndex += registerSize;
    TString *uniformNamePtr = newTString(name);
    NodeItem nodeItem(uniformRegisterIndex, uniformNamePtr, semanticAnnotationNode, new TType(), initializerNode);
    nodeItem.m_type->shallowCopy(specType);
    m_uniformNodes.insert(std::make_pair(*uniformNamePtr, nodeItem));
    return fetchFromGlobalUniformInitializer;
}

TIntermTyped *
ParserContext::normalizeGlobalStaticInitializer(const TIntermTyped *typeNode, TIntermTyped *initializerNode)
{
    TType specType;
    specType.shallowCopy(typeNode->getType());
    if (TIntermSelection *selectionNode = initializerNode->getAsSelectionNode()) {
        TIntermNode *trueNode = selectionNode->getTrueBlock(), *falseNode = selectionNode->getFalseBlock();
        if (trueNode && falseNode) {
            TString *variableName = newAnonymousVariableString();
            m_context->declareVariable(initializerNode->getLoc(), *variableName, specType);
            TIntermTyped *leftNode = m_context->handleVariable(initializerNode->getLoc(), variableName);
            trueNode = m_intermediate.addAssign(EOpAssign, leftNode, trueNode->getAsTyped(), trueNode->getLoc());
            falseNode = m_intermediate.addAssign(EOpAssign, leftNode, falseNode->getAsTyped(), falseNode->getLoc());
            initializerNode = new TIntermSelection(selectionNode->getCondition(), trueNode, falseNode, specType);
        }
    }
    else if (specType.isArray()) {
        /* do nothing */
    }
    else if (specType.isMatrix()) {
        TFunction *constructor = m_context->makeConstructorCall(typeNode->getLoc(), specType);
        TIntermAggregate *aggregateNode = initializerNode->getAsAggregate();
        if (constructor && aggregateNode && aggregateNode->getOp() != EOpFunctionCall) {
            TIntermTyped *argumentsNode = nullptr, *resultNode = nullptr;
            foldAggregateNode(aggregateNode, resultNode);
            if (TIntermAggregate *newAggregateNode = resultNode->getAsAggregate()) {
                const TIntermSequence &sequence = newAggregateNode->getSequence();
                for (const auto &it : sequence) {
                    if (TIntermTyped *typedNode = it->getAsTyped()) {
                        m_context->handleFunctionArgument(constructor, argumentsNode, typedNode);
                    }
                }
            }
            else {
                m_context->handleFunctionArgument(constructor, argumentsNode, resultNode);
            }
            initializerNode = m_context->handleFunctionCall(initializerNode->getLoc(), constructor, argumentsNode);
        }
    }
    else if ((specType.isVector() || specType.isMatrix()) && initializerNode->isScalar()) {
        initializerNode = m_context->handleConstructor(typeNode->getLoc(), initializerNode, specType);
    }
    return initializerNode;
}

void
ParserContext::fixupSwizzleVectorOperation(TIntermTyped *&leftNode, TIntermTyped *&rightNode)
{
    /* FIXME: array specification */
    const TType &leftType = leftNode->getType();
    const TType &rightType = rightNode->getType();
    if (leftType.isVector() && rightType.isVector() && leftType.getVectorSize() != rightType.getVectorSize()) {
        /* e.g. float2 a = float3(0.0).xy; */
        if (leftType.getVectorSize() < rightType.getVectorSize()) {
            rightNode = swizzleVectorNode(rightNode, leftType);
        }
        /* e.g. float3 a; a.xy = float2(0.0); */
        else if (leftType.getVectorSize() > rightType.getVectorSize()) {
            leftNode = swizzleVectorNode(leftNode, rightType);
        }
    }
    /* e.g. float4x4 a = 0.0; */
    else if (leftType.isMatrix() && rightType.isScalar()) {
        const TSourceLoc &loc = leftNode->getLoc();
        TIntermAggregate *argumentsNode = nullptr;
        for (int i = 0, offset = 0, numRows = leftType.getMatrixRows(), numCols = leftType.getMatrixCols(); i < numRows;
             i++) {
            int constructAt = i * numRows + i;
            for (int j = 0; j < numCols; j++) {
                TIntermTyped *arg = offset == constructAt ? rightNode : m_intermediate.addConstantUnion(0, loc, true);
                argumentsNode = m_intermediate.growAggregate(argumentsNode, arg);
                offset++;
            }
        }
        argumentsNode->setOperator(EOpSequence);
        rightNode = m_context->handleConstructor(loc, argumentsNode, rightType);
    }
    /* e.g. float4 a = 0.0; */
    else if (leftType.isVector() && rightType.isScalar()) {
        rightNode = handleSingleConstructorCall(leftNode, rightNode);
    }
}

TIntermTyped *
ParserContext::fixupSwizzleMatrixOperation(TIntermBinary *binaryNode, TIntermTyped *rightNode, const TOperator op)
{
    /* e.g. matrix._11_12_13_14 *= 1; */
    TIntermTyped *baseMatrixNode = binaryNode->getLeft(), *exprNode = nullptr;
    const TIntermSequence &swizzleNodes = binaryNode->getRight()->getAsAggregate()->getSequence();
    const TSourceLoc &loc = baseMatrixNode->getLoc();
    size_t numSwizzleNodes = swizzleNodes.size();
    TIntermAggregate *aggregateNode = nullptr;
    if (swizzleNodes.size() % 2 == 0) {
        for (size_t i = 0; i < numSwizzleNodes; i += 2) {
            TIntermConstantUnion *rowNode = swizzleNodes[i]->getAsConstantUnion(),
                                 *columnNode = swizzleNodes[i + 1]->getAsConstantUnion();
            TIntermTyped *derefNode = m_context->handleBracketDereference(
                loc, m_context->handleBracketDereference(loc, baseMatrixNode, rowNode), columnNode);
            TIntermTyped *accessorNode = rightNode->isVector() ? indexVectorNode(rightNode, i / 2) : rightNode;
            aggregateNode =
                m_intermediate.growAggregate(aggregateNode, m_context->handleAssign(loc, op, derefNode, accessorNode));
        }
        aggregateNode->setOperator(EOpSequence);
        exprNode = aggregateNode;
    }
    else {
        m_context->error(loc, "inconsistent matrix swizzle operation", "", "");
    }
    return exprNode;
}

void
ParserContext::fixupCastFloatOrBooleanOperation(TOperator op, TIntermTyped *&leftNode, TIntermTyped *&rightNode)
{
    /* FIXME: array specification */
    if (leftNode && rightNode) {
        const TType &leftType = leftNode->getType();
        const TType &rightType = rightNode->getType();
        /* e.g. bool a = true; float b = a ? 1.0 : 0.0; */
        if (leftType.isFloatingDomain() && rightType.isScalar() && rightType.getBasicType() == EbtBool) {
            rightNode = castBoolToFloat(rightNode);
        }
        /* e.g. bool a = true; int b = a ? 1 : 0; */
        else if (leftType.isIntegerDomain() && rightType.isScalar() && rightType.getBasicType() == EbtBool) {
            rightNode = castBoolToInteger(rightNode);
        }
        /* float a = 0.0; float b = a ? 1.0 : 0.0; */
        else if (leftType.getBasicType() == EbtBool && rightType.isScalar() && rightType.isFloatingDomain()) {
            switch (op) {
            case EOpAdd:
            case EOpSub:
            case EOpMul:
            case EOpDiv:
            case EOpMod:
                leftNode = castBoolToFloat(leftNode);
                break;
            default:
                rightNode = castFloatToBool(rightNode);
                break;
            }
        }
        else if (leftType.getBasicType() == EbtBool && rightType.isScalar() && rightType.isIntegerDomain()) {
            switch (op) {
            case EOpAdd:
            case EOpSub:
            case EOpMul:
            case EOpDiv:
            case EOpMod:
                leftNode = castBoolToInteger(leftNode);
                break;
            default:
                rightNode = castIntegerToBool(rightNode);
                break;
            }
        }
    }
}

void
ParserContext::handleFunctionArgumentsConversion(
    const TIntermSequence &arguments, TFunction *&function, TIntermTyped *&newArgumentsNode)
{
    TVector<const TFunction *> candidateList;
    bool builtIn = false;
    const TString &functionName = function->getName();
    m_symbolTable.findFunctionNameList(functionName + "(", candidateList, builtIn);
    if (!candidateList.empty()) {
        bool ok = false;
        std::vector<std::pair<const TFunction *, int>> possibilityList;
        for (auto it = candidateList.begin(), end = candidateList.end(); it != end; ++it) {
            const TFunction *candidate = *it;
            if (candidate->getParamCount() == function->getParamCount()) {
                int matchScore = 0;
                for (int i = 0, numParameters = function->getParamCount(); i < numParameters; i++) {
                    const TType &candidateType = *(*candidate)[i].type;
                    const TType &functionType = *(*function)[i].type;
                    if (candidateType.getBasicType() != functionType.getBasicType()) {
                        matchScore -= 200;
                    }
                    if (candidateType.getVectorSize() != functionType.getVectorSize()) {
                        int weight = candidateType.getVectorSize() > functionType.getVectorSize() ? 5 : 50;
                        matchScore -= std::abs(candidateType.getVectorSize() - functionType.getVectorSize()) * weight;
                    }
                    if (candidateType.getMatrixRows() != functionType.getMatrixRows()) {
                        int weight = candidateType.getMatrixRows() > functionType.getMatrixRows() ? 5 : 50;
                        matchScore -= std::abs(candidateType.getMatrixRows() - functionType.getMatrixRows()) * weight;
                    }
                    if (candidateType.getMatrixCols() != functionType.getMatrixCols()) {
                        int weight = candidateType.getMatrixCols() > functionType.getMatrixCols() ? 5 : 50;
                        matchScore -= std::abs(candidateType.getMatrixCols() - functionType.getMatrixCols()) * weight;
                    }
                }
                possibilityList.push_back(std::make_pair(candidate, matchScore));
                if (matchScore == 0) {
                    ok = true;
                    it = candidateList.end();
                    break;
                }
            }
        }
        if (!ok && !possibilityList.empty()) {
            std::sort(possibilityList.begin(), possibilityList.end(),
                [](std::pair<const TFunction *, int> left, std::pair<const TFunction *, int> right) {
                    return left.second > right.second;
                });
            handleFunctionArgumentsConversion(possibilityList.front().first, arguments, function, newArgumentsNode);
        }
    }
}

void
ParserContext::handleFunctionArgumentsConversion(
    const TFunction *defined, const TIntermSequence &arguments, TFunction *&function, TIntermTyped *&newArgumentsNode)
{
    TString *name = newTString(function->getName());
    TFunction *adjustedFunction = new TFunction(name, defined->getType());
    newArgumentsNode = nullptr;
    for (int i = 0, numParameters = function->getParamCount(); i < numParameters; i++) {
        const TParameter &fromParameter = (*function)[i];
        const TParameter &toParameter = (*defined)[i];
        const TType &fromType = *fromParameter.type;
        const TType &toType = *toParameter.type;
        TIntermTyped *argumentNode = arguments[i]->getAsTyped();
        if (fromType != toType) {
            /* float4x4 a; float3x3 b = a; */
            if (fromType.isMatrix() && toType.isMatrix()) {
                argumentNode = handleSingleConstructorCall(toType, argumentNode, argumentNode);
            }
            else if ((fromType.isVector() || fromType.isScalar()) && (toType.isVector() || toType.isScalar())) {
                int fromVectorSize = fromType.getVectorSize(), toVectorSize = toType.getVectorSize();
                /* float2 a = float2(1, 1); float4 b = a; */
                if (fromVectorSize > toVectorSize) {
                    argumentNode = swizzleVectorNode(argumentNode, toVectorSize);
                }
                /* float4 a = float4(1, 1, 1, 1); float2 b = a; */
                else if (fromVectorSize < toVectorSize) {
                    TIntermTyped *callerNode = nullptr, *constructArgumentsNode = nullptr;
                    if (TFunction *constructor = m_context->makeConstructorCall(TSourceLoc(), toType)) {
                        m_context->handleFunctionArgument(constructor, constructArgumentsNode, argumentNode);
                        if (fromVectorSize > 1 || fromType.getBasicType() == EbtBool) {
                            for (int i = fromVectorSize; i < toVectorSize; i++) {
                                TIntermTyped *newArgumentNode =
                                    m_intermediate.addConstantUnion(0.0, EbtFloat, TSourceLoc(), true);
                                m_context->handleFunctionArgument(constructor, constructArgumentsNode, newArgumentNode);
                            }
                        }
                        callerNode =
                            m_context->handleFunctionCall(argumentNode->getLoc(), constructor, constructArgumentsNode);
                    }
                    argumentNode = callerNode;
                }
                /* bool a = true; float b = a ? 1.0 : 0.0; */
                else if (fromType.getBasicType() == EbtBool && toType.isFloatingDomain()) {
                    argumentNode = castBoolToFloat(argumentNode);
                }
                /* bool a = true; int b = a ? 1 : 0; */
                else if (fromType.getBasicType() == EbtBool && toType.isIntegerDomain()) {
                    argumentNode = castBoolToInteger(argumentNode);
                }
                else {
                    argumentNode = handleSingleConstructorCall(toType, argumentNode, argumentNode);
                }
            }
        }
        if (argumentNode) {
            m_context->handleFunctionArgument(adjustedFunction, newArgumentsNode, argumentNode);
        }
    }
    function = adjustedFunction;
}

ParserContext::InternalNode *
ParserContext::resolveNode(atom_t atom)
{
    static fx_interm_null_node_t kIntermNullNode;
    static InternalNode kNullNode(&kIntermNullNode);
    InternalNode *node = &kNullNode;
    if (atom > 0 && atom < static_cast<atom_t>(m_nodes.size())) {
        node = &m_nodes[atom];
    }
    return node;
}

TIntermNode *
ParserContext::flattenAggregateNode(TIntermNode *node)
{
    TIntermNode *result = node;
    if (TIntermAggregate *aggregateNode = node->getAsAggregate()) {
        const TIntermSequence &sequence = aggregateNode->getSequence();
        if (aggregateNode->getOp() == EOpNull && sequence.size() == 1) {
            result = sequence[0];
        }
    }
    return result;
}

TIntermNode *
ParserContext::resolveIntermNode(atom_t atom)
{
    return resolveNode(atom)->m_value;
}

TIntermNode *
ParserContext::resolveIntermStatement(atom_t atom)
{
    TIntermNode *node = nullptr;
    if (atom > 0 && atom < static_cast<atom_t>(m_nodes.size())) {
        node = m_nodes[atom].m_value;
    }
    return node;
}

TIntermAggregate *
ParserContext::resolveIntermAggregate(atom_t atom)
{
    TIntermNode *node = resolveIntermStatement(atom);
    return node ? node->getAsAggregate() : nullptr;
}

TIntermNode *
ParserContext::swizzleInitializer(const TIntermTyped *typeNode, const TIntermSymbol *nameNode,
    TArraySizes * /* arraySizes */, TIntermTyped *initializerNode)
{
    TType specType;
    specType.shallowCopy(typeNode->getType());
    const TType &initializerType = initializerNode ? initializerNode->getType() : specType;
    const int vectorSize = specType.getVectorSize();
    TIntermNode *declarationNode = nullptr;
    if (vectorSize < initializerType.getVectorSize() && vectorSize > 0 && vectorSize <= 4) {
        TIntermTyped *swizzleNode = swizzleVectorNode(initializerNode, specType);
        TString *name = newTString(nameNode->getName());
        declarationNode = m_context->declareVariable(nameNode->getLoc(), *name, specType, swizzleNode);
    }
    return declarationNode;
}

TIntermTyped *
ParserContext::swizzleVectorNode(TIntermTyped *baseNode, const TType &type)
{
    return swizzleVectorNode(baseNode, type.getVectorSize());
}

TIntermTyped *
ParserContext::swizzleVectorNode(TIntermTyped *baseNode, int index)
{
    static const char *kFields[] = { "", "x", "xy", "xyz", "xyzw" };
    return m_context->handleDotDereference(baseNode->getLoc(), baseNode, kFields[index]);
}

TIntermTyped *
ParserContext::indexVectorNode(TIntermTyped *baseNode, int index)
{
    static const char *kFields[] = { "x", "y", "z", "w" };
    return m_context->handleDotDereference(baseNode->getLoc(), baseNode, kFields[index]);
}

TIntermNode *
ParserContext::createTextureSamplerAccessor(const TType &samplerType, const TIntermSymbol *samplerNameNode,
    size_t samplerIndex, TIntermAggregate *argumentsNode, TIntermBinary *semanticAnnotationNode)
{
    const TSampler &sampler = samplerType.getSampler();
    TPublicType publicType;
    publicType.init(samplerNameNode->getLoc(), true);
    publicType.basicType = samplerType.getBasicType();
    publicType.sampler.set(sampler.type, sampler.dim);
    TType *type = new TType(publicType);
    TQualifier &qualifier = type->getQualifier();
    qualifier.storage = EvqUniform;
    /* sampler*/
    TString *samplerName = newTString(samplerNameNode->getName());
    m_context->declareVariable(samplerNameNode->getLoc(), *samplerName, *type);
    TIntermTyped *samplerNode = m_context->handleVariable(samplerNameNode->getLoc(), samplerName);
    auto it = m_samplerNodes.find(*samplerName);
    if (it == m_samplerNodes.end()) {
        SamplerNodeItem samplerNodeItem(
            samplerIndex, samplerName, semanticAnnotationNode, type, samplerNode, argumentsNode);
        samplerNodeItem.m_samplerIndex = samplerIndex;
        m_samplerNodes.insert(std::make_pair(samplerNameNode->getName(), samplerNodeItem));
    }
    return samplerNode;
}

void
ParserContext::bindTextureToSampler(const TString &textureName, const TString &samplerName)
{
    auto it = m_textureNodes.find(textureName);
    auto it2 = m_samplerNodes.find(samplerName);
    if (it != m_textureNodes.end() && it2 != m_samplerNodes.end()) {
        it2->second.m_textureNode = &it->second;
    }
}

TIntermNode *
ParserContext::growAggregateNode(TIntermNode *left, TIntermNode *right)
{
    TIntermAggregate *node = m_intermediate.growAggregate(left != right ? left : nullptr, right);
    if (node && right) {
        node->setLoc(right->getLoc());
    }
    return node;
}

void
ParserContext::handleFunctionArguments(
    TFunction *function, TIntermNode *originArgumentsNode, TIntermTyped *&newArgumentsNode)
{
    newArgumentsNode = nullptr;
    if (originArgumentsNode) {
        if (TIntermAggregate *aggregatedArguments = originArgumentsNode->getAsAggregate()) {
            const TIntermSequence &sequence = aggregatedArguments->getSequence();
            for (const auto &it : sequence) {
                m_context->handleFunctionArgument(function, newArgumentsNode, it->getAsTyped());
            }
        }
    }
}

const TString *
ParserContext::globalUniformFloat4Name() const
{
    TString *name = nullptr;
    switch (m_context->getLanguage()) {
    case EShLangFragment:
        name = NewPoolTString("ps_uniforms_vec4");
        break;
    case EShLangVertex:
        name = NewPoolTString("vs_uniforms_vec4");
        break;
    default:
        break;
    }
    return name;
}

const TString *
ParserContext::globalSamplerRegisterName(size_t registerIndex) const
{
    TString *name = nullptr;
    char buffer[32];
    switch (m_context->getLanguage()) {
    case EShLangFragment:
        snprintf(buffer, sizeof(buffer), "ps_s%lu", registerIndex);
        name = NewPoolTString(buffer);
        break;
    case EShLangVertex:
        snprintf(buffer, sizeof(buffer), "vs_s%lu", registerIndex);
        name = NewPoolTString(buffer);
        break;
    default:
        break;
    }
    return name;
}

bool
ParserContext::isFunctionDefined(const TString &name) const
{
    return m_allFunctions.find(name) != m_allFunctions.end();
}

TIntermAggregate *
ParserContext::createVertexShaderEntryPoint(
    TIntermAggregate *unitNode, TFunction *function, TIntermAggregate *entryPointArguments)
{
    TIntermTyped *argumentsNode = nullptr;
    TFunction *declaration = createVertexShaderEntryPointFunction(function, argumentsNode);
    addExtraEntryPointFunctionArguments(declaration, entryPointArguments, argumentsNode);
    TIntermTyped *callEntryPointNode = m_context->handleFunctionCall(TSourceLoc(), declaration, argumentsNode);
    TIntermAggregate *bodyNode = createGlobalVariableInitializers();
    if (callEntryPointNode->getType().getBasicType() == EbtVoid) {
        bodyNode = m_intermediate.growAggregate(bodyNode, callEntryPointNode);
        for (auto node : m_outputVertexShaderNodes) {
            bodyNode = m_intermediate.growAggregate(bodyNode, node);
        }
    }
    else if (TIntermNode *assignmentNode =
                 createBuiltInVertexShaderOutputAssignmentNode(function, callEntryPointNode)) {
        bodyNode = m_intermediate.growAggregate(bodyNode, assignmentNode);
    }
    bodyNode->setOperator(EOpSequence);
    return createMainFunction(unitNode, bodyNode);
}

TIntermAggregate *
ParserContext::createPixelShaderEntryPoint(
    TIntermAggregate *unitNode, TFunction *function, TIntermAggregate *entryPointArguments)
{
    TIntermTyped *argumentsNode = nullptr;
    TFunction *declaration = createPixelShaderEntryPointFunction(function, argumentsNode);
    addExtraEntryPointFunctionArguments(declaration, entryPointArguments, argumentsNode);
    TIntermTyped *callEntryPointNode = m_context->handleFunctionCall(TSourceLoc(), declaration, argumentsNode);
    TIntermAggregate *bodyNode = createGlobalVariableInitializers();
    const TType &outputCallNodeType = callEntryPointNode->getType();
    {
        for (const auto &it : m_writtenVertexShaderInputVariables) {
            const TString *builtInName = NewPoolTString(it.second.c_str()), *anon = newAnonymousVariableString();
            if (!m_context->symbolTable.find(*builtInName)) {
                TType inputType(EbtFloat, EvqVaryingIn, 4), varType(EbtFloat, EvqTemporary, 4);
                m_context->declareVariable(TSourceLoc(), *anon, varType);
                m_context->declareVariable(TSourceLoc(), *builtInName, inputType);
                TIntermTyped *assigneeNode = m_context->handleVariable(TSourceLoc(), anon),
                             *assignerNode = m_context->handleVariable(TSourceLoc(), builtInName),
                             *assignmentNode =
                                 m_intermediate.addAssign(EOpAssign, assigneeNode, assignerNode, TSourceLoc());
                bodyNode = m_intermediate.growAggregate(bodyNode, assignmentNode);
            }
        }
    }
    if (outputCallNodeType.getBasicType() == EbtVoid) {
        bodyNode = m_intermediate.growAggregate(bodyNode, callEntryPointNode);
        const TIntermSequence &arguments = argumentsNode->getAsAggregate()->getSequence();
        for (const auto argument : arguments) {
            if (TIntermSymbol *symbol = argument->getAsSymbolNode()) {
                auto it = m_outputPixelShaderNodes.find(symbol->getName());
                if (it != m_outputPixelShaderNodes.end()) {
                    TType outputType(EbtFloat, EvqVaryingOut, 4);
                    outputType.getQualifier().builtIn = it->second.second;
                    bodyNode = m_intermediate.growAggregate(
                        bodyNode, createBuiltInPixelShaderOutputAssignmentNode(outputType, it->second.first));
                }
            }
        }
    }
    else if (TIntermNode *assignmentNode =
                 createBuiltInPixelShaderOutputAssignmentNode(function->getType(), callEntryPointNode)) {
        bodyNode = m_intermediate.growAggregate(bodyNode, assignmentNode);
    }
    bodyNode->setOperator(EOpSequence);
    return createMainFunction(unitNode, bodyNode);
}

void
ParserContext::addExtraEntryPointFunctionArguments(
    TFunction *declaration, TIntermAggregate *entryPointArguments, TIntermTyped *&argumentsNode)
{
    if (entryPointArguments) {
        const TIntermSequence &sequence = entryPointArguments->getSequence();
        for (const auto &it : sequence) {
            TIntermNode *argumentNode = it;
            if (TIntermTyped *typedArgument = argumentNode->getAsTyped()) {
                TIntermSymbol *symbol = typedArgument->getAsSymbolNode();
                TParameter parameter = { symbol ? newTString(symbol->getName()) : nullptr, new TType(), nullptr };
                parameter.type->shallowCopy(typedArgument->getType());
                if (const TIntermConstantUnion *constantUnionNode = typedArgument->getAsConstantUnion()) {
                    if (constantUnionNode->isLiteral()) {
                        parameter.type->getQualifier().makeSpecConstant();
                    }
                }
                declaration->addParameter(parameter);
                argumentsNode =
                    argumentsNode ? m_intermediate.growAggregate(argumentsNode, typedArgument) : typedArgument;
            }
        }
    }
}

TIntermAggregate *
ParserContext::createMainFunction(TIntermAggregate *unitNode, TIntermAggregate *functionBodyNode)
{
    TString *name = NewPoolTString("main");
    TType *returnType = new TType(EbtVoid);
    TFunction *entryPoint = new TFunction(name, *returnType);
    m_context->handleFunctionDeclarator(TSourceLoc(), *entryPoint, false);
    TAttributes attributes;
    TIntermNode *entryPointTree = nullptr,
                *definitionNode =
                    m_context->handleFunctionDefinition(TSourceLoc(), *entryPoint, attributes, entryPointTree);
    m_context->handleFunctionBody(TSourceLoc(), *entryPoint, functionBodyNode, definitionNode);
    return m_intermediate.growAggregate(unitNode, definitionNode);
}

TFunction *
ParserContext::createVertexShaderEntryPointFunction(const TFunction *base, TIntermTyped *&argumentsNode)
{
    TString *callerName = newTString(base->getName());
    TType *returnType = new TType(EbtVoid);
    TFunction *declaration = new TFunction(callerName, *returnType);
    auto it = m_builtInVariables.find(base->getMangledName().c_str());
    auto builtInArguments = it != m_builtInVariables.end() ? it->second : TVector<TBuiltInVariable>();
    for (int i = 0, numParameters = base->getParamCount(); i < numParameters; i++) {
        const TParameter &parameter = (*base)[i];
        const TType &inputType = *parameter.type;
        TIntermTyped *arguments = nullptr;
        if (inputType.isStruct()) {
            const TTypeList *fields = inputType.getStruct();
            if (TFunction *constructor = m_context->makeConstructorCall(TSourceLoc(), inputType)) {
                for (auto it = fields->begin(), end = fields->end(); it != end; ++it) {
                    const TType *fieldType = it->type;
                    const TBuiltInVariable builtIn = findBuiltInFromStructMember(inputType, *fieldType);
                    addVertexShaderEntryPointFunctionArgument(*fieldType, builtIn, constructor, arguments);
                }
                TIntermTyped *constructorNode = m_context->handleFunctionCall(TSourceLoc(), constructor, arguments);
                argumentsNode =
                    argumentsNode ? m_intermediate.growAggregate(arguments, constructorNode) : constructorNode;
                TParameter argumentParameter = { nullptr, new TType(), nullptr };
                argumentParameter.type->shallowCopy(inputType);
                declaration->addParameter(argumentParameter);
            }
        }
        else if (inputType.isMatrix() || inputType.isVector() || inputType.isScalar()) {
            bool found = true;
            if (inputType.getQualifier().isParamOutput()) {
                OutputVertexShaderVariableTraverser traverser(parameter.name);
                auto it = m_allFunctions.find(base->getMangledName());
                if (it != m_allFunctions.end()) {
                    TIntermNode *node = it->second[0].second;
                    node->traverse(&traverser);
                }
                found = traverser.hasFound();
            }
            if (found) {
                const TBuiltInVariable &variable = builtInArguments[i];
                TType newInputType;
                newInputType.shallowCopy(inputType);
                addVertexShaderEntryPointFunctionArgument(newInputType, variable, declaration, argumentsNode);
            }
            else {
                /* add dummy stub for unused output variable */
                TType newInputType(inputType.getBasicType(), EvqTemporary, 4);
                TString *variableName = newAnonymousVariableString();
                m_context->declareVariable(TSourceLoc(), *variableName, newInputType);
                TIntermTyped *stubVariableNode = m_context->handleVariable(TSourceLoc(), variableName);
                m_context->handleFunctionArgument(
                    declaration, argumentsNode, swizzleVectorNode(stubVariableNode, inputType));
            }
        }
    }
    return declaration;
}

void
ParserContext::addVertexShaderEntryPointFunctionArgument(
    const TType &inputType, TBuiltInVariable builtIn, TFunction *declaration, TIntermTyped *&argumentsNode)
{
    const TQualifier &inputTypeQualifier = inputType.getQualifier();
    auto it = m_vertexShaderInputVariables.find(builtIn);
    if (it != m_vertexShaderInputVariables.end()) {
        if (inputTypeQualifier.isParamOutput()) {
            auto it2 = m_pixelShaderInputVariables.find(builtIn);
            if (it2 != m_pixelShaderInputVariables.end()) {
                /* build input vertex variable as float4 and swizzle by usage */
                TType newInputType(inputType.getBasicType(), inputTypeQualifier.storage, 4);
                TQualifier &newInputTypeQualifier = newInputType.getQualifier();
                newInputTypeQualifier.builtIn = builtIn;
                newInputTypeQualifier.storage = EvqVaryingOut;
                auto it3 = m_pixelShaderInputMap.find(builtIn);
                if (it3 != m_pixelShaderInputMap.end()) {
                    newInputTypeQualifier.layoutLocation = it3->second;
                }
                const TString variableName(it2->second.c_str(), it2->second.size());
                m_context->declareVariable(TSourceLoc(), variableName, newInputType);
                TIntermTyped *outputVariableNode = m_context->handleVariable(TSourceLoc(), &variableName);
                m_context->handleFunctionArgument(
                    declaration, argumentsNode, swizzleVectorNode(outputVariableNode, inputType));
                m_writtenVertexShaderInputVariables.insert(*it2);
                const TQualifier &inputTypeQualifier = inputType.getQualifier();
                if (inputTypeQualifier.builtIn == EbvVertex || inputTypeQualifier.declaredBuiltIn == EbvVertex) {
                    TString *output = newAnonymousVariableString();
                    TType copyInputType;
                    copyInputType.shallowCopy(inputType);
                    TQualifier &q = copyInputType.getQualifier();
                    q.builtIn = EbvPosition;
                    q.storage = EvqPosition;
                    m_context->declareVariable(TSourceLoc(), *output, copyInputType);
                    m_outputVertexShaderNodes.push_back(m_intermediate.addAssign(
                        EOpAssign, m_context->handleVariable(TSourceLoc(), output), outputVariableNode, TSourceLoc()));
                }
            }
        }
        else {
            TType newInputType;
            /* uint gl_VertexIndex */
            if (builtIn == EbvVertexIndex) {
                newInputType.shallowCopy(TType(EbtUint));
            }
            else {
                newInputType.shallowCopy(inputType);
            }
            TQualifier &newInputTypeQualifier = newInputType.getQualifier();
            newInputTypeQualifier.clear();
            newInputTypeQualifier.builtIn = builtIn;
            newInputTypeQualifier.storage = EvqVaryingIn;
            auto it2 = m_vertexShaderInputMap.find(builtIn);
            if (it2 != m_vertexShaderInputMap.end()) {
                newInputTypeQualifier.layoutLocation = it2->second;
            }
            /* prevent confliction of a_texcoord0 */
            TString *builtInName = NewPoolTString(it->second.c_str());
            if (!m_context->symbolTable.find(*builtInName)) {
                m_context->declareVariable(TSourceLoc(), *builtInName, newInputType);
            }
            TIntermTyped *argumentNode = m_context->handleVariable(TSourceLoc(), builtInName);
            m_context->handleFunctionArgument(declaration, argumentsNode, argumentNode);
        }
    }
}

TIntermNode *
ParserContext::addBuiltInVertexShaderOutputAssignment(
    const TType &outputType, TBuiltInVariable builtIn, TIntermTyped *valueNode)
{
    TIntermTyped *assignmentNode = nullptr;
    auto it = m_vertexShadreBuiltInVariableConversions.find(builtIn);
    if (it != m_vertexShadreBuiltInVariableConversions.end()) {
        TString *name = NewPoolTString("");
        TIntermTyped *asgineeNode = new TIntermSymbol(0, *name, outputType);
        TQualifier &q = asgineeNode->getWritableType().getQualifier();
        q.storage = it->second.first;
        q.builtIn = it->second.second;
        assignmentNode = m_intermediate.addAssign(EOpAssign, asgineeNode, valueNode, TSourceLoc());
    }
    return assignmentNode;
}

TIntermNode *
ParserContext::addVertexShaderOutputAssignment(
    const TType &outputType, TBuiltInVariable builtIn, TIntermTyped *callerNode)
{
    TIntermNode *assignmentNode = nullptr;
    auto it = m_pixelShaderInputVariables.find(builtIn);
    if (it != m_pixelShaderInputVariables.end()) {
        TString *name = NewPoolTString(it->second.c_str());
        TType type(outputType.getBasicType(), EvqVaryingOut, 4);
        auto it2 = m_pixelShaderInputMap.find(builtIn);
        if (it2 != m_pixelShaderInputMap.end()) {
            type.getQualifier().layoutLocation = it2->second;
        }
        if (!type.sameElementShape(outputType)) {
            TIntermTyped *argumentsNode = nullptr;
            if (TFunction *function = m_context->makeConstructorCall(TSourceLoc(), type)) {
                /* float/float2/float3 -> float4 with zero fill */
                m_context->handleFunctionArgument(function, argumentsNode, callerNode);
                for (int i = 0, vectorSize = type.getVectorSize() - outputType.getVectorSize(); i < vectorSize; i++) {
                    TIntermTyped *zeroNode =
                        m_intermediate.addConstantUnion(0u, type.getBasicType(), TSourceLoc(), true);
                    m_context->handleFunctionArgument(function, argumentsNode, zeroNode);
                }
                callerNode = m_context->handleFunctionCall(TSourceLoc(), function, argumentsNode);
            }
        }
        assignmentNode = m_context->declareVariable(TSourceLoc(), *name, type, callerNode);
        m_writtenVertexShaderInputVariables.insert(*it);
    }
    return assignmentNode;
}

TFunction *
ParserContext::createPixelShaderEntryPointFunction(const TFunction *base, TIntermTyped *&argumentsNode)
{
    TString *callerName = newTString(base->getName());
    TFunction *declaration = new TFunction(callerName, base->getType());
    argumentsNode = nullptr;
    auto it = m_builtInVariables.find(base->getMangledName().c_str());
    auto builtInArguments = it != m_builtInVariables.end() ? it->second : TVector<TBuiltInVariable>();
    for (int i = 0, numParameters = base->getParamCount(); i < numParameters; i++) {
        const TParameter &parameter = (*base)[i];
        const TType &inputType = *parameter.type;
        TIntermTyped *arguments = nullptr;
        if (inputType.isStruct()) {
            const TTypeList *fields = inputType.getStruct();
            if (TFunction *constructor = m_context->makeConstructorCall(TSourceLoc(), inputType)) {
                traverseAllPixelShaderInputStructFields(inputType, fields, constructor, arguments);
                TIntermTyped *constructorNode = m_context->handleFunctionCall(TSourceLoc(), constructor, arguments);
                argumentsNode =
                    argumentsNode ? m_intermediate.growAggregate(arguments, constructorNode) : constructorNode;
                TParameter argumentParameter = { nullptr, new TType(), nullptr };
                argumentParameter.type->shallowCopy(inputType);
                declaration->addParameter(argumentParameter);
            }
        }
        else if (inputType.isMatrix() || inputType.isVector() || inputType.isScalar()) {
            TType newInputType;
            newInputType.shallowCopy(inputType);
            addPixelShaderEntryPointFunctionArgument(newInputType, builtInArguments[i], declaration, argumentsNode);
        }
    }
    return declaration;
}

void
ParserContext::traverseAllPixelShaderInputStructFields(
    const TType &inputType, const TTypeList *fields, TFunction *constructor, TIntermTyped *&arguments)
{
    for (auto it = fields->begin(), end = fields->end(); it != end; ++it) {
        const TType *fieldType = it->type;
        if (fieldType->isStruct()) {
            if (TFunction *innerConstructor = m_context->makeConstructorCall(TSourceLoc(), *fieldType)) {
                TIntermTyped *innerArguments = nullptr;
                traverseAllPixelShaderInputStructFields(
                    *fieldType, fieldType->getStruct(), innerConstructor, innerArguments);
                TIntermTyped *constructorNode =
                    m_context->handleFunctionCall(TSourceLoc(), innerConstructor, innerArguments);
                m_context->handleFunctionArgument(constructor, arguments, constructorNode);
            }
        }
        else {
            const TBuiltInVariable builtIn = findBuiltInFromStructMember(inputType, *fieldType);
            addPixelShaderEntryPointFunctionArgument(*fieldType, builtIn, constructor, arguments);
        }
    }
}

void
ParserContext::addPixelShaderEntryPointFunctionArgument(
    const TType &inputType, TBuiltInVariable builtIn, TFunction *declaration, TIntermTyped *&argumentsNode)
{
    TStorageQualifier storageQualifier = EvqTemporary;
    switch (builtIn) {
    case EbvVertex: {
        builtIn = EbvFragCoord;
        storageQualifier = EvqFragCoord;
        break;
    }
    case EbvFragCoord: {
        storageQualifier = EvqFragCoord;
        break;
    }
    case EbvFragDepth: {
        storageQualifier = EvqFragDepth;
        break;
    }
    case EbvPointSize: {
        storageQualifier = EvqPointSize;
        break;
    }
    default:
        break;
    }
    if (storageQualifier != EvqTemporary) {
        TType type(EbtFloat, storageQualifier, 4);
        type.getQualifier().builtIn = builtIn;
        TString *name = newAnonymousVariableString();
        m_context->declareVariable(TSourceLoc(), *name, type);
        TIntermTyped *baseNode = swizzleVectorNode(m_context->handleVariable(TSourceLoc(), name), inputType);
        if (builtIn == EbvFragCoord) {
            baseNode = m_intermediate.addAssign(EOpAdd, baseNode,
                handleSingleConstructorCall(
                    TSourceLoc(), inputType, m_intermediate.addConstantUnion(-0.5, EbtFloat, TSourceLoc(), true)),
                TSourceLoc());
        }
        m_context->handleFunctionArgument(declaration, argumentsNode, baseNode);
    }
    else if (builtIn == EbvVertexIndex) {
        TIntermTyped *zeroNode = m_intermediate.addConstantUnion(0, TSourceLoc(), true);
        m_context->handleFunctionArgument(declaration, argumentsNode, zeroNode);
    }
    else {
        auto it = m_pixelShaderInputVariables.find(builtIn);
        if (it != m_pixelShaderInputVariables.end()) {
            auto it3 = m_writtenVertexShaderInputVariables.find(builtIn);
            if (it3 != m_writtenVertexShaderInputVariables.end() && it3->second == it->second) {
                /* build input pixel shader variable as float4 and swizzle by usage */
                TType newInputType(inputType.getBasicType(), EvqVaryingIn, 4);
                auto it2 = m_pixelShaderInputMap.find(builtIn);
                if (it2 != m_pixelShaderInputMap.end()) {
                    newInputType.getQualifier().layoutLocation = it2->second;
                }
                TIntermTyped *argumentNode;
                /* handle array for multiple TexCoords (e.g. MRT) */
                if (inputType.isArray() && (builtIn == EbvTexCoord || builtIn == EbvMultiTexCoord0)) {
                    TFunction *function = m_context->makeConstructorCall(TSourceLoc(), inputType);
                    TIntermTyped *arguments = nullptr;
                    for (int i = 0, arraySize = inputType.getArraySizes()->getOuterSize(); i < arraySize; i++) {
                        auto texCoordKey = static_cast<TBuiltInVariable>(EbvMultiTexCoord0 + i);
                        auto it = m_pixelShaderInputVariables.find(texCoordKey);
                        if (it != m_pixelShaderInputVariables.end()) {
                            auto it2 = m_pixelShaderInputMap.find(texCoordKey);
                            if (it2 != m_pixelShaderInputMap.end()) {
                                newInputType.getQualifier().layoutLocation = it2->second;
                            }
                            TString name(it->second.c_str(), it->second.size());
                            m_context->declareVariable(TSourceLoc(), name, newInputType);
                            argumentNode = m_context->handleVariable(TSourceLoc(), &name);
                            m_context->handleFunctionArgument(
                                function, arguments, swizzleVectorNode(argumentNode, inputType));
                        }
                    }
                    argumentNode = m_context->handleFunctionCall(TSourceLoc(), function, arguments);
                }
                else {
                    /* prevent confliction of a_texcoord0 */
                    TString *builtInName = NewPoolTString(it->second.c_str());
                    if (!m_context->symbolTable.find(*builtInName)) {
                        m_context->declareVariable(TSourceLoc(), *builtInName, newInputType);
                    }
                    argumentNode = swizzleVectorNode(m_context->handleVariable(TSourceLoc(), builtInName), inputType);
                }
                m_context->handleFunctionArgument(declaration, argumentsNode, argumentNode);
                m_writtenVertexShaderInputVariables.erase(builtIn);
            }
            else if (inputType.getQualifier().isParamOutput()) {
                auto it = m_pixelShaderInputVariables.find(builtIn);
                if (it != m_pixelShaderInputVariables.end()) {
                    const TString name(it->second.c_str(), it->second.size());
                    TType outputType(inputType.getBasicType(), EvqTemporary, 4);
                    m_context->declareVariable(TSourceLoc(), name, outputType);
                    TIntermTyped *outputVariableNode = m_context->handleVariable(TSourceLoc(), &name);
                    m_context->handleFunctionArgument(declaration, argumentsNode, outputVariableNode);
                    m_outputPixelShaderNodes.insert(std::make_pair(name, std::make_pair(outputVariableNode, builtIn)));
                }
            }
            else {
                TIntermTyped *argumentNode = handleSingleConstructorCall(
                    TSourceLoc(), inputType, m_intermediate.addConstantUnion(0, TSourceLoc(), true));
                m_context->handleFunctionArgument(declaration, argumentsNode, argumentNode);
            }
        }
    }
}

TIntermAggregate *
ParserContext::growVertexShaderBuiltInVariableAssignment(
    const TType &fieldType, TBuiltInVariable builtIn, TIntermAggregate *bodyNode, TIntermTyped *dereferenceNode)
{
    if (TIntermNode *statementNode = addBuiltInVertexShaderOutputAssignment(fieldType, builtIn, dereferenceNode)) {
        bodyNode = m_intermediate.growAggregate(bodyNode, statementNode);
    }
    if (TIntermNode *statementNode = addVertexShaderOutputAssignment(fieldType, builtIn, dereferenceNode)) {
        bodyNode = m_intermediate.growAggregate(bodyNode, statementNode);
    }
    return bodyNode;
}

TIntermAggregate *
ParserContext::growVertexShaderOutputStructAssignment(
    const TType &newOutputType, TIntermTyped *outputVariableNode, TIntermAggregate *bodyNode)
{
    const TTypeList *fields = newOutputType.getStruct();
    for (auto it = fields->begin(), end = fields->end(); it != end; ++it) {
        const TType *fieldType = it->type;
        TIntermTyped *dereferenceNode =
            m_context->handleDotDereference(TSourceLoc(), outputVariableNode, fieldType->getFieldName());
        /* skip built-in readonly variable (gl_VertexIndex) */
        if (fieldType->getQualifier().builtIn != EbvVertexIndex) {
            const TBuiltInVariable builtIn = findBuiltInFromStructMember(newOutputType, *fieldType);
            if (fieldType->isArray()) {
                /* possible for multiple TexCoords */
                for (int i = 0, outerSize = fieldType->getArraySizes()->getOuterSize(); i < outerSize; i++) {
                    TIntermTyped *itemNode = m_context->handleBracketDereference(
                        TSourceLoc(), dereferenceNode, m_intermediate.addConstantUnion(i, TSourceLoc(), true));
                    if (builtIn == EbvTexCoord || builtIn == EbvMultiTexCoord0) {
                        const TBuiltInVariable newBuiltIn =
                            static_cast<TBuiltInVariable>(static_cast<int>(EbvMultiTexCoord0) + i);
                        bodyNode = growVertexShaderBuiltInVariableAssignment(
                            itemNode->getType(), newBuiltIn, bodyNode, itemNode);
                    }
                    else {
                        bodyNode =
                            growVertexShaderBuiltInVariableAssignment(itemNode->getType(), builtIn, bodyNode, itemNode);
                    }
                }
            }
            else if (fieldType->isStruct()) {
                bodyNode = growVertexShaderOutputStructAssignment(*fieldType, dereferenceNode, bodyNode);
            }
            else {
                bodyNode = growVertexShaderBuiltInVariableAssignment(*fieldType, builtIn, bodyNode, dereferenceNode);
            }
        }
    }
    return bodyNode;
}

TIntermNode *
ParserContext::createBuiltInVertexShaderOutputAssignmentNode(const TFunction *function, TIntermTyped *outputCallNode)
{
    TIntermAggregate *bodyNode = nullptr;
    TType newOutputType;
    newOutputType.shallowCopy(outputCallNode->getType());
    newOutputType.getQualifier().makeTemporary();
    TString *outputName = NewPoolTString("vs_output_t");
    TIntermNode *initializerNode = m_context->declareVariable(TSourceLoc(), *outputName, newOutputType, outputCallNode);
    bodyNode = m_intermediate.growAggregate(bodyNode, initializerNode);
    TIntermNode *assignmentNode = m_context->handleVariable(TSourceLoc(), outputName);
    bodyNode = m_intermediate.growAggregate(bodyNode, assignmentNode);
    if (TIntermTyped *outputVariableNode = assignmentNode->getAsTyped()) {
        if (newOutputType.isStruct()) {
            bodyNode = growVertexShaderOutputStructAssignment(newOutputType, outputVariableNode, bodyNode);
            if (m_pointSizeAssignment > 0.0f) {
                TType specType(EbtFloat);
                specType.getQualifier().storage = EvqPointSize;
                specType.getQualifier().builtIn = EbvPointSize;
                TIntermTyped *valueNode =
                    m_intermediate.addConstantUnion(m_pointSizeAssignment, EbtFloat, TSourceLoc(), true);
                TString *variableName = newAnonymousVariableString();
                m_context->declareVariable(initializerNode->getLoc(), *variableName, specType);
                TIntermTyped *leftNode = m_context->handleVariable(initializerNode->getLoc(), variableName);
                TIntermTyped *innerAssignmentNode =
                    m_intermediate.addAssign(EOpAssign, leftNode, valueNode, TSourceLoc());
                bodyNode = m_intermediate.growAggregate(bodyNode, innerAssignmentNode);
            }
        }
        else if (newOutputType.isVector()) {
            const TType &functionType = function->getType();
            const TBuiltInVariable builtIn = functionType.getQualifier().builtIn;
            bodyNode = growVertexShaderBuiltInVariableAssignment(functionType, builtIn, bodyNode, outputCallNode);
        }
    }
    bodyNode->setOperator(EOpSequence);
    return bodyNode;
}

TIntermNode *
ParserContext::createBuiltInPixelShaderOutputAssignmentNode(const TType &outputType, TIntermTyped *node)
{
    TIntermTyped *assignmentNode = nullptr;
    auto createOutputNode = [this](const TType *type, TBuiltInVariable builtIn,
                                TStorageQualifier storage) -> TIntermTyped * {
        int index = -1;
        switch (builtIn) {
        case EbvColor:
        case EbvFrontColor:
            index = 0;
            break;
        case EbvBackColor:
            index = 1;
            break;
        case EbvFrontSecondaryColor:
            index = 2;
            break;
        case EbvBackSecondaryColor:
            index = 3;
            break;
        default:
            break;
        }
        TIntermTyped *node = nullptr;
        if (index >= 0) {
            TString *name = NewPoolTString("gl_FragData");
            TIntermSymbol *fragDataNode = new TIntermSymbol(0, *name, *type);
            TArraySizes arraySizes;
            arraySizes.addInnerSize(4);
            TType &fragDataType = fragDataNode->getWritableType();
            fragDataType.copyArraySizes(arraySizes);
            TQualifier &q = fragDataType.getQualifier();
            q.storage = storage;
            q.builtIn = storage == EvqFragDepth ? EbvFragDepth : EbvFragData;
            node = m_context->handleBracketDereference(
                TSourceLoc(), fragDataNode, m_intermediate.addConstantUnion(index, TSourceLoc(), true));
        }
        return node;
    };
    if (outputType.isStruct()) {
        const TTypeList *members = outputType.getStruct();
        TIntermAggregate *aggregateNode = nullptr;
        TType variableType;
        variableType.shallowCopy(outputType);
        variableType.makeTemporary();
        TString *variableName = newAnonymousVariableString();
        m_context->declareVariable(TSourceLoc(), *variableName, variableType);
        TIntermTyped *variableNode = m_context->handleVariable(TSourceLoc(), variableName);
        aggregateNode =
            m_intermediate.growAggregate(nullptr, m_context->handleAssign(TSourceLoc(), EOpAssign, variableNode, node));
        for (auto it = members->begin(), end = members->end(); it != end; ++it) {
            const TType *fieldType = it->type;
            const TBuiltInVariable builtIn = findBuiltInFromStructMember(outputType, *fieldType);
            auto it2 = m_pixelShaderBuiltInVariableConversions.find(builtIn);
            if (it2 != m_pixelShaderBuiltInVariableConversions.end()) {
                if (TIntermTyped *derefOut = createOutputNode(fieldType, builtIn, it2->second)) {
                    TIntermTyped *derefIn =
                        m_context->handleDotDereference(TSourceLoc(), variableNode, fieldType->getFieldName());
                    aggregateNode = m_intermediate.growAggregate(
                        aggregateNode, m_intermediate.addAssign(EOpAssign, derefOut, derefIn, TSourceLoc()));
                }
            }
        }
        aggregateNode->setOperator(EOpSequence);
        assignmentNode = aggregateNode;
    }
    else if (outputType.isVector()) {
        const TBuiltInVariable builtIn = outputType.getQualifier().builtIn;
        auto it = m_pixelShaderBuiltInVariableConversions.find(builtIn);
        if (it != m_pixelShaderBuiltInVariableConversions.end()) {
            if (TIntermTyped *derefOut = createOutputNode(&outputType, builtIn, it->second)) {
                assignmentNode = m_intermediate.addAssign(EOpAssign, derefOut, node, TSourceLoc());
            }
        }
    }
    return assignmentNode;
}

TIntermAggregate *
ParserContext::createGlobalVariableInitializers()
{
    TIntermAggregate *bodyNode = nullptr;
    auto growGlobalInitializer = [this, &bodyNode](
                                     const TString &name, const char *prefix, const TIntermTypedNodeMap &nodes) {
        auto it2 = nodes.find(name);
        if (it2 != nodes.end()) {
            TString *nameWithPrefix = NewPoolTString(prefix);
            nameWithPrefix->append(name);
            TIntermTyped *left = m_context->handleVariable(TSourceLoc(), nameWithPrefix), *right = it2->second;
            if (left && right) {
                if (!left->isArray()) {
                    right = handleSingleConstructorCall(left, right);
                }
                TIntermTyped *assignmentNode = m_intermediate.addAssign(EOpAssign, left, right, TSourceLoc());
                assert(assignmentNode);
                bodyNode = m_intermediate.growAggregate(bodyNode, assignmentNode);
            }
        }
    };
    const auto &globalStaticParameters = m_staticParameters[nullptr];
    for (const auto &name : m_globalInitializerNameOrder) {
        growGlobalInitializer(name, kGlobalConstantVariableNamePrefix, m_immediateConstantParameters);
        growGlobalInitializer(name, kGlobalConstantVariableNamePrefix, m_expressionConstantParameters);
        growGlobalInitializer(name, kGlobalUniformVariableNamePrefix, m_uniformParameters);
        growGlobalInitializer(name, kGlobalStaticVariableNamePrefix, globalStaticParameters);
    }
    return bodyNode;
}

TIntermTyped *
ParserContext::handleSingleConstructorCall(const TIntermTyped *typeNode, TIntermTyped *argNode)
{
    return handleSingleConstructorCall(typeNode->getType(), typeNode, argNode);
}

TIntermTyped *
ParserContext::handleSingleConstructorCall(const TType &type, const TIntermNode *callerNode, TIntermTyped *argNode)
{
    return handleSingleConstructorCall(callerNode->getLoc(), type, argNode);
}

TIntermTyped *
ParserContext::handleSingleConstructorCall(const TSourceLoc &loc, const TType &type, TIntermTyped *argNode)
{
    TIntermTyped *node = nullptr;
    if (TFunction *function = m_context->makeConstructorCall(loc, type)) {
        TIntermTyped *newArgumentsNode = nullptr;
        m_context->handleFunctionArgument(function, newArgumentsNode, argNode);
        node = m_context->handleFunctionCall(loc, function, newArgumentsNode);
    }
    return node;
}

TIntermSelection *
ParserContext::fixupSelectionNode(TIntermTyped *condExprNode, TIntermNode *trueExprNode, TIntermNode *falseExprNode)
{
    TIntermSelection *newSelectionNode = nullptr;
    TIntermTyped *trueTypedNode = trueExprNode->getAsTyped();
    /* skip aggregate node (accepts one-liner only) */
    if (trueTypedNode && !trueTypedNode->getAsAggregate()) {
        TType trueExprType, falseExprType;
        trueExprType.shallowCopy(trueTypedNode->getType());
        if (falseExprNode) {
        }
    }
    if (!newSelectionNode) {
        TIntermTyped *falseTypedNode = falseExprNode->getAsTyped();
        if (trueTypedNode && falseTypedNode) {
            TType trueExprType, falseExprType, selectionType;
            trueExprType.shallowCopy(trueTypedNode->getType());
            falseExprType.shallowCopy(falseTypedNode->getType());
            if (trueExprType.getBasicType() == falseExprType.getBasicType() &&
                trueExprType.getVectorSize() < falseExprType.getVectorSize()) {
                selectionType.shallowCopy(falseExprType);
                trueExprNode = handleSingleConstructorCall(falseExprNode->getAsTyped(), trueExprNode->getAsTyped());
            }
            else {
                selectionType.shallowCopy(trueExprType);
                falseExprNode = handleSingleConstructorCall(trueExprNode->getAsTyped(), falseExprNode->getAsTyped());
            }
            newSelectionNode = new TIntermSelection(condExprNode, trueExprNode, falseExprNode);
            newSelectionNode->setType(selectionType);
        }
        else {
            newSelectionNode = new TIntermSelection(condExprNode, trueExprNode, falseExprNode);
        }
    }
    return newSelectionNode;
}

TIntermTyped *
ParserContext::handleSelectiveAssignment(TIntermTyped *leftNode, TOperator op, TIntermSelection *selectionNode)
{
    TIntermTyped *trueNode = selectionNode->getTrueBlock()->getAsTyped();
    TIntermTyped *falseNode = selectionNode->getFalseBlock()->getAsTyped();
    TIntermTyped *exprNode = selectionNode;
    if (trueNode && falseNode) {
        trueNode = m_intermediate.addAssign(op, leftNode, trueNode, trueNode->getLoc());
        falseNode = m_intermediate.addAssign(op, leftNode, falseNode, falseNode->getLoc());
        exprNode = new TIntermSelection(selectionNode->getCondition(), trueNode, falseNode);
    }
    return exprNode;
}

TIntermTyped *
ParserContext::castBoolToFloat(TIntermTyped *node)
{
    const TSourceLoc &loc = node->getLoc();
    TIntermTyped *oneNode = m_intermediate.addConstantUnion(1.0, EbtFloat, loc, true);
    TIntermTyped *zeroNode = m_intermediate.addConstantUnion(0.0, EbtFloat, loc, true);
    return m_intermediate.addSelection(node, oneNode, zeroNode, loc);
}

TIntermTyped *
ParserContext::castBoolToInteger(TIntermTyped *node)
{
    const TSourceLoc &loc = node->getLoc();
    TIntermTyped *oneNode = m_intermediate.addConstantUnion(1, loc, true);
    TIntermTyped *zeroNode = m_intermediate.addConstantUnion(0, loc, true);
    return m_intermediate.addSelection(node, oneNode, zeroNode, loc);
}

TIntermTyped *
ParserContext::castFloatToBool(TIntermTyped *node)
{
    const TSourceLoc &loc = node->getLoc();
    TIntermTyped *zeroNode = m_intermediate.addConstantUnion(0.0, EbtFloat, loc, true);
    return m_intermediate.addBinaryMath(EOpNotEqual, node, zeroNode, loc);
}

TIntermTyped *
ParserContext::castIntegerToBool(TIntermTyped *node)
{
    const TSourceLoc &loc = node->getLoc();
    TIntermTyped *zeroNode = m_intermediate.addConstantUnion(0, loc, true);
    return m_intermediate.addBinaryMath(EOpNotEqual, node, zeroNode, loc);
}

bool
ParserContext::isInitializerNodeConst(TIntermTyped *initializerNode)
{
    bool isConst = false;
    if (initializerNode) {
        if (TIntermAggregate *aggregateNode = initializerNode->getAsAggregate()) {
            isConst = m_intermediate.areAllChildConst(aggregateNode);
        }
        else {
            isConst = initializerNode->getAsConstantUnion() != nullptr;
        }
    }
    return isConst;
}

void
ParserContext::foldAggregateNode(TIntermAggregate *node, TIntermTyped *&result)
{
    const TIntermSequence &sequence = node->getSequence();
    for (const auto &it : sequence) {
        TIntermNode *argNode = it;
        TIntermAggregate *childrenNode = argNode->getAsAggregate();
        if (childrenNode && childrenNode->getOp() == EOpNull) {
            foldAggregateNode(childrenNode, result);
        }
        else if (TIntermTyped *typedNode = argNode->getAsTyped()) {
            result = m_intermediate.growAggregate(result, typedNode);
            result->setLoc(typedNode->getLoc());
        }
    }
}

TIntermTyped *
ParserContext::overrideBuiltInCall(const TFunction *function, TIntermTyped *argumentsNode)
{
    bool builtIn;
    const TSourceLoc &loc = argumentsNode ? argumentsNode->getLoc() : TSourceLoc();
    TFunction *functionClone = function->clone();
    int thisDepth = 0;
    size_t argumentSize = 0;
    TIntermAggregate *agg = nullptr;
    if (argumentsNode) {
        agg = argumentsNode->getAsAggregate();
        if (agg) {
            argumentSize = agg->getSequence().size();
        }
    }
    const TFunction *builtInFunction = m_context->findFunction(loc, *functionClone, builtIn, thisDepth, argumentsNode);
    /* remove extra default initialized arguments */
    if (agg && agg->getSequence().size() > argumentSize) {
        TIntermSequence &seq = agg->getSequence();
        for (int i = int(seq.size() - 1); i >= int(argumentSize); i--) {
            seq.erase(seq.begin() + i);
        }
    }
    TIntermTyped *callerNode = nullptr;
    if (builtIn && builtInFunction) {
        switch (builtInFunction->getBuiltInOp()) {
        case EOpAtan: {
            if (!m_intermediate.areAllChildConst(agg)) {
                callerNode = overrideBuiltInAtanCall(builtInFunction, argumentsNode, loc);
            }
            break;
        }
        case EOpFract: {
            if (!argumentsNode->getAsConstantUnion()) {
                callerNode = overrideBuiltInFractCall(argumentsNode, loc);
            }
            break;
        }
        case EOpModf: {
            // callerNode = overrideBuiltInModfCall(argumentsNode, loc);
            break;
        }
        case EOpNoise: {
            callerNode = overrideBuiltInNoiseCall(argumentsNode);
            break;
        }
        case EOpNormalize: {
            if (!argumentsNode->getAsConstantUnion()) {
                callerNode = overrideBuiltInNormalizeCall(argumentsNode, loc);
            }
            break;
        }
        case EOpPow: {
            if (!m_intermediate.areAllChildConst(agg)) {
                callerNode = overrideBuiltInPowCall(builtInFunction, argumentsNode, loc);
            }
            break;
        }
        case EOpTextureLod: {
            callerNode = overrideBuiltInTextureLodCall(builtInFunction, argumentsNode, loc);
            break;
        }
        default:
            break;
        }
    }
    return callerNode;
}

glslang::TIntermTyped *
ParserContext::overrideBuiltInFractCall(glslang::TIntermTyped *argumentsNode, const glslang::TSourceLoc &loc)
{
    /* float y = normalize(x); z = isnan(y) ? 0.0 : y */
    TIntermAggregate *normalizeNode = new TIntermAggregate(EOpFract);
    normalizeNode->setType(argumentsNode->getType());
    normalizeNode->getSequence().push_back(argumentsNode);
    return castNaNToZero(normalizeNode, argumentsNode->getType(), loc);
}

TIntermTyped *
ParserContext::overrideBuiltInModfCall(TIntermTyped *argumentsNode, const glslang::TSourceLoc &loc)
{
    TIntermTyped *callerNode = nullptr;
    if (TIntermAggregate *args = argumentsNode->getAsAggregate()) {
        const TIntermSequence &sequence = args->getSequence();
        if (sequence.size() == 2) {
            TIntermTyped *arg0 = sequence[0]->getAsTyped();
            TIntermTyped *arg1 = sequence[1]->getAsTyped();
            TString *variableName = newAnonymousVariableString();
            TType type;
            type.shallowCopy(arg1->getType());
            m_context->declareVariable(loc, *variableName, type, arg1);
            /* float temp = arg1; modf(a, temp); arg1 = temp; */
            TIntermTyped *variableNode = m_context->handleVariable(loc, variableName);
            TIntermAggregate *sequence = new TIntermAggregate(EOpSequence);
            TIntermAggregate *modf = new TIntermAggregate(EOpModf);
            modf->getSequence().push_back(arg0);
            modf->getSequence().push_back(variableNode);
            sequence->getSequence().push_back(modf);
            TIntermTyped *assignmentNode = m_context->handleAssign(loc, EOpAssign, arg1, variableNode);
            sequence->getSequence().push_back(assignmentNode);
            callerNode = sequence;
        }
    }
    return callerNode;
}

TIntermTyped *
ParserContext::overrideBuiltInNoiseCall(TIntermTyped *argumentsNode)
{
    /* workaround (fetch first argument only) */
    TIntermTyped *callerNode = nullptr;
    if (TIntermAggregate *args = argumentsNode->getAsAggregate()) {
        TIntermTyped *arg0 = args->getSequence()[0]->getAsTyped();
        callerNode = arg0;
    }
    else if (TIntermTyped *arg0 = argumentsNode->getAsTyped()) {
        callerNode = arg0;
    }
    return callerNode;
}

glslang::TIntermTyped *
ParserContext::overrideBuiltInNormalizeCall(glslang::TIntermTyped *argumentsNode, const glslang::TSourceLoc &loc)
{
    /* float y = normalize(x); z = isnan(y) ? 0.0 : y */
    TIntermAggregate *normalizeNode = new TIntermAggregate(EOpNormalize);
    normalizeNode->setType(argumentsNode->getType());
    normalizeNode->getSequence().push_back(argumentsNode);
    return castNaNToZero(normalizeNode, argumentsNode->getType(), loc);
}

glslang::TIntermTyped *
ParserContext::overrideBuiltInAtanCall(
    const glslang::TFunction *builtInFunction, glslang::TIntermTyped *argumentsNode, const glslang::TSourceLoc &loc)
{
    /* float z = atan2(x, y); z = isnan(z) ? 0.0 : z */
    TIntermTyped *callerNode = nullptr;
    TIntermAggregate *args = argumentsNode->getAsAggregate();
    if (builtInFunction->getType().isScalar() && args) {
        const TIntermSequence &arguments = args->getSequence();
        if (arguments.size() == 2) {
            const TType &type = args[0].getType();
            TIntermAggregate *builtInFuncNode = new TIntermAggregate(EOpAtan);
            builtInFuncNode->setType(type);
            for (auto item : args->getSequence()) {
                builtInFuncNode->getSequence().push_back(item);
            }
            callerNode = castNaNToZero(builtInFuncNode, type, loc);
        }
    }
    return callerNode;
}

TIntermTyped *
ParserContext::overrideBuiltInPowCall(
    const glslang::TFunction *builtInFunction, TIntermTyped *argumentsNode, const glslang::TSourceLoc &loc)
{
    /* float z = pow(x, y); z = isnan(z) ? 0.0 : z */
    TIntermTyped *callerNode = nullptr;
    TIntermAggregate *args = argumentsNode->getAsAggregate();
    if (builtInFunction->getType().isScalar() && args) {
        const TType &type = args[0].getType();
        TIntermAggregate *builtInFuncNode = new TIntermAggregate(EOpPow);
        builtInFuncNode->setType(type);
        for (auto item : args->getSequence()) {
            builtInFuncNode->getSequence().push_back(item);
        }
        callerNode = castNaNToZero(builtInFuncNode, type, loc);
    }
    return callerNode;
}

TIntermTyped *
ParserContext::overrideBuiltInTextureLodCall(
    const TFunction *builtInFunction, TIntermTyped *argumentsNode, const glslang::TSourceLoc &loc)
{
    /* same as HlslParseContext::decomposeSampleMethods */
    TIntermTyped *callerNode = nullptr;
    if (TIntermAggregate *args = argumentsNode->getAsAggregate()) {
        const TIntermSequence &sequence = args->getSequence();
        if (sequence.size() == 2) {
            TIntermTyped *arg0 = sequence[0]->getAsTyped();
            TIntermTyped *arg1 = sequence[1]->getAsTyped();
            TIntermTyped *w = m_intermediate.addConstantUnion(3, loc, true);
            TIntermTyped *lod = m_intermediate.addIndex(EOpIndexDirect, arg1, w, loc);
            TOperator constructOp = EOpNull;
            const TSampler &sampler = arg0->getType().getSampler();
            switch (sampler.dim) {
            case Esd1D: {
                constructOp = EOpConstructFloat;
                break;
            }
            case Esd2D: {
                constructOp = EOpConstructVec2;
                break;
            }
            case Esd3D:
            case EsdCube: {
                constructOp = EOpConstructVec3;
                break;
            }
            default:
                break;
            }
            TIntermAggregate *constructCoord = new TIntermAggregate(constructOp);
            constructCoord->getSequence().push_back(arg1);
            constructCoord->setLoc(loc);
            constructCoord->setType(TType(arg1->getBasicType(), EvqTemporary, std::max(arg1->getVectorSize() - 1, 0)));
            TIntermAggregate *textureLod = new TIntermAggregate(EOpTextureLod);
            textureLod->setType(TType(builtInFunction->getType().getBasicType(), EvqTemporary, sampler.vectorSize));
            textureLod->getSequence().push_back(arg0);
            textureLod->getSequence().push_back(constructCoord);
            textureLod->getSequence().push_back(lod);
            callerNode = textureLod;
        }
    }
    return callerNode;
}

glslang::TIntermTyped *
ParserContext::castNaNToZero(glslang::TIntermTyped *baseNode, const TType &type, const glslang::TSourceLoc &loc)
{
    /*
     * float x = func(); y = !isnan(x) ? x : 0
     * vec4  x = func(); y = !all(isnan(x)) ? x : vec4(0)
     */
    TType resultVariableType;
    resultVariableType.shallowCopy(type);
    resultVariableType.makeTemporary();
    TString *resultVariableName = newAnonymousVariableString();
    m_context->declareVariable(loc, *resultVariableName, resultVariableType);
    TIntermTyped *resultVariableNode = m_context->handleVariable(loc, resultVariableName);
    TType testIsNaNType(EbtBool, EvqTemporary, type.getVectorSize());
    TIntermTyped *testIsNaNNode =
        m_intermediate.addBuiltInFunctionCall(loc, EOpIsNan, true, resultVariableNode, testIsNaNType);
    TIntermNodePair pair;
    pair.node1 = resultVariableNode;
    TIntermAggregate *sequence = new TIntermAggregate(EOpSequence);
    sequence->setType(resultVariableType);
    sequence->getSequence().push_back(m_context->handleAssign(loc, EOpAssign, resultVariableNode, baseNode));
    if (type.isVector()) {
        testIsNaNNode = m_intermediate.addBuiltInFunctionCall(loc, EOpAll, true, testIsNaNNode, TType(EbtBool));
        testIsNaNNode = m_intermediate.addBuiltInFunctionCall(loc, EOpLogicalNot, true, testIsNaNNode, TType(EbtBool));
        pair.node2 = handleSingleConstructorCall(
            loc, resultVariableType, m_intermediate.addConstantUnion(0.0f, EbtFloat, loc, true));
    }
    else {
        testIsNaNNode = m_intermediate.addBuiltInFunctionCall(loc, EOpLogicalNot, true, testIsNaNNode, TType(EbtBool));
        pair.node2 = m_intermediate.addConstantUnion(0.0f, EbtFloat, loc, true);
    }
    TIntermTyped *selectionNode = m_intermediate.addSelection(testIsNaNNode, pair, loc);
    selectionNode->setType(resultVariableType);
    sequence->getSequence().push_back(selectionNode);
    return sequence;
}

TBuiltInVariable
ParserContext::findBuiltInFromStructMember(const TType &type, const TType &field) const
{
    return findBuiltInFromStructMember(type.getTypeName(), field.getFieldName());
}

TBuiltInVariable
ParserContext::findBuiltInFromStructMember(const TString &typeName, const TString &fieldName) const
{
    TBuiltInVariable value = EbvNone;
    auto it = m_builtInStructMembers.find(typeName);
    if (it != m_builtInStructMembers.end()) {
        auto it2 = it->second.find(fieldName);
        if (it2 != it->second.end()) {
            value = it2->second;
        }
    }
    return value;
}

void
ParserContext::registerFunctionParameter(const TString &functionName, const TIntermSymbol *symbol)
{
    auto it = m_usedFunctionParameters.find(functionName);
    ParameterNameSet *parameterSet = nullptr;
    if (it != m_usedFunctionParameters.end()) {
        parameterSet = it->second;
    }
    else {
        parameterSet = new ParameterNameSet();
        m_usedFunctionParameters.insert(std::make_pair(functionName, parameterSet));
    }
    if (symbol) {
        parameterSet->insert(symbol->getName());
    }
}

void
ParserContext::copyFunctionParameterSet(const TString &sourcePassName, const TString &destPassName)
{
    registerFunctionParameter(sourcePassName, nullptr);
    registerFunctionParameter(destPassName, nullptr);
    auto src = m_usedFunctionParameters.find(sourcePassName), dst = m_usedFunctionParameters.find(destPassName);
    if (src != m_usedFunctionParameters.end() && dst != m_usedFunctionParameters.end()) {
        dst->second->insert(src->second->begin(), src->second->end());
    }
}

void
ParserContext::decorateSemanticType(atom_t semantic, TType &type)
{
    const TIntermSymbol *semanticNode = nullptr;
    if (const TIntermBinary *semanticAnnotationNode = resolveIntermNode(semantic)->getAsBinaryNode()) {
        const TIntermTyped *leftNode = semanticAnnotationNode->getLeft();
        semanticNode = leftNode ? leftNode->getAsSymbolNode() : nullptr;
    }
    else if (const TIntermSymbol *semanticNodePtr = resolveIntermNode(semantic)->getAsSymbolNode()) {
        semanticNode = semanticNodePtr;
    }
    if (semanticNode) {
        const TString &semanticName = semanticNode->getName();
        TString canonicalizedSemantic(semanticName);
        std::transform(semanticName.begin(), semanticName.end(), canonicalizedSemantic.begin(), ::toupper);
        setBuiltInSemantics(canonicalizedSemantic, type);
    }
}

void
ParserContext::incrementRegisterIndex(const TType &type)
{
    if (type.isMatrix()) {
        m_lastUniformRegisterIndex += type.getMatrixRows();
    }
    else if (type.isArray()) {
        m_lastUniformRegisterIndex += type.getArraySizes()->getOuterSize();
    }
    else {
        m_lastUniformRegisterIndex += 1;
    }
}

void
ParserContext::handleArraySizeSpecifier(TIntermNode *node, TType &type)
{
    if (node) {
        handleArraySizeSpecifier(node->getAsAggregate(), type);
    }
}

void
ParserContext::handleArraySizeSpecifier(TIntermAggregate *node, TType &type)
{
    if (node) {
        TArraySizes arraySizes;
        for (TIntermNode *item : node->getSequence()) {
            const TSourceLoc &loc = item->getLoc();
            TIntermTyped *typedItem = item->getAsTyped();
            if (typedItem && loc.line > 0) {
                TArraySize pair;
                m_context->arraySizeCheck(loc, typedItem, pair);
                if (pair.size > 0) {
                    arraySizes.addInnerSize(pair);
                }
            }
            else {
                arraySizes.addInnerSize(0);
            }
        }
        type.copyArraySizes(arraySizes);
    }
}

void
ParserContext::setBuiltInSemantics(const TString &semantic, TType &type)
{
    auto it = m_builtInSemantics.find(semantic);
    if (it != m_builtInSemantics.end()) {
        type.getQualifier().builtIn = it->second;
    }
}

} /* namespace fx9 */
