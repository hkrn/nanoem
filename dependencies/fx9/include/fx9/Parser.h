/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is licensed under MIT license. for more details, see LICENSE.txt.
 */

#pragma once
#ifndef FX9_PARSER_H_
#define FX9_PARSER_H_

/* GLSLang */
#define ENABLE_HLSL
#include "hlsl/hlslParseHelper.h"
#include "hlsl/hlslParseables.h"
#include "hlsl/hlslScanContext.h"

namespace fx9 {

using atom_t = uintptr_t;

struct LexerToken;

class ParserContext {
public:
    struct Annotation {
        Annotation(const glslang::TString name)
            : m_name(name)
        {
        }
        const glslang::TString m_name;
        const TIntermNode *m_value = nullptr;
    };
    using AnnotationList = glslang::TVector<Annotation>;
    using AnnotationListMap = glslang::TUnorderedMap<glslang::TString, AnnotationList>;
    struct Pass;
    struct PassState {
        PassState(const Pass *parent, const glslang::TString name)
            : m_parent(parent)
            , m_name(name)
        {
        }
        const Pass *m_parent;
        const glslang::TString m_name;
        const TIntermNode *m_value = nullptr;
    };
    using PassStateList = glslang::TVector<PassState>;
    struct Technique;
    struct Pass {
        using EntryPoint = std::pair<std::string, glslang::TIntermAggregate *>;
        Pass(const Technique *parent, const glslang::TString name)
            : m_parent(parent)
            , m_name(name)
        {
        }
        const Technique *m_parent;
        const glslang::TString m_name;
        EntryPoint m_vertexShaderEntryPoint;
        EntryPoint m_pixelShaderEntryPoint;
        PassStateList m_states;
        AnnotationList m_annotations;
    };
    using PassList = glslang::TVector<Pass>;
    struct Technique {
        Technique(const glslang::TString name)
            : m_name(name)
        {
        }
        const glslang::TString m_name;
        PassList m_passes;
        AnnotationList m_annotations;
    };
    using TechniqueList = glslang::TVector<Technique>;
    struct NodeItem {
        NodeItem(size_t index, const glslang::TString *variableName, glslang::TIntermBinary *semanticAnnotationNode,
            glslang::TType *type, glslang::TIntermTyped *initializerNode)
            : m_variableName(*variableName)
            , m_type(type)
            , m_semanticAnnotationNode(semanticAnnotationNode)
            , m_initializerNode(initializerNode)
            , m_index(index)
        {
        }
        ~NodeItem()
        {
        }
        glslang::TString m_variableName;
        glslang::TType *m_type = nullptr;
        glslang::TIntermBinary *m_semanticAnnotationNode = nullptr;
        glslang::TIntermTyped *m_initializerNode = nullptr;
        size_t m_index = 0;
    };
    using StateBlockMap = glslang::TUnorderedMap<glslang::TString, glslang::TIntermAggregate *>;
    using NodeMap = glslang::TUnorderedMap<glslang::TString, NodeItem>;
    struct TextureNodeItem {
        glslang::TType *m_type;
        glslang::TIntermBinary *m_semanticAnnotationNode;
        glslang::TString m_name;
    };
    struct SamplerNodeItem : NodeItem {
        SamplerNodeItem(size_t index, const glslang::TString *variableName,
            glslang::TIntermBinary *semanticAnnotationNode, glslang::TType *type,
            glslang::TIntermTyped *initializerNode, glslang::TIntermAggregate *samplerStatesNode)
            : NodeItem(index, variableName, semanticAnnotationNode, type, initializerNode)
            , m_samplerStatesNode(samplerStatesNode)
        {
        }
        ~SamplerNodeItem()
        {
        }
        glslang::TIntermAggregate *m_samplerStatesNode = nullptr;
        TextureNodeItem *m_textureNode = nullptr;
        size_t m_samplerIndex = SIZE_MAX;
    };
    using SamplerNodeMap = glslang::TUnorderedMap<glslang::TString, SamplerNodeItem>;
    using TextureNodeMap = glslang::TUnorderedMap<glslang::TString, TextureNodeItem>;
    using TStringList = std::vector<glslang::TString>;
    using TStringSet = std::unordered_set<glslang::TString>;
    using BuiltInLocationMap = std::unordered_map<uint32_t, uint32_t>;
    using BuiltInVariableMap = std::unordered_map<uint32_t, std::string>;

    static inline glslang::TString *newTString(const glslang::TString &value);
    static inline glslang::TString *newAnonymousVariableString(atom_t uniqueId);
    static bool isNonZeroConstant(const glslang::TIntermTyped *node);
    static void convertAllAnnotations(const glslang::TIntermAggregate *annotationsNode, AnnotationList &annotations);
    static void convertPass(const glslang::TIntermSequence &sequence, const StateBlockMap &stateBlocks, Pass &pass);
    static void convertPass(const glslang::TIntermBinary *passNode, const StateBlockMap &stateBlocks, Pass &pass);
    static void convertTechnique(
        const glslang::TIntermAggregate *passesNode, const StateBlockMap &stateBlocks, Technique &technique);

    ParserContext(glslang::HlslParseContext *context, const glslang::TString &filename,
        const BuiltInVariableMap &vertexShaderInputVariables, const BuiltInVariableMap &pixelShaderInputVariables,
        const BuiltInVariableMap &writtenVertexShaderInputVariables = BuiltInVariableMap());
    ~ParserContext();

    void execute(const glslang::TString &source, const glslang::TString &preamble);
    void setEntryPoint(const Pass::EntryPoint &value);
    bool isUserDefinedType(const glslang::TString &name) const;
    SamplerNodeMap findSamplerNodes(const Pass::EntryPoint &entryPoint);

    NodeMap constantNodes() const;
    NodeMap uniformNodes() const;
    SamplerNodeMap samplerNodes() const;
    TextureNodeMap textureNodes() const;
    AnnotationListMap annotations() const;
    TechniqueList techniques() const;
    const char *filename() const;
    const char *currentProcessingFilename() const;
    const LexerToken *lastLexerToken() const;
    size_t countAllParameterNodes() const;
    bool hasAnyParameterNodes() const;

    glslang::HlslParseContext *parseContext();
    glslang::TIntermediate &intermediate();
    void dump(std::ostream &destination);

    BuiltInVariableMap writtenVertexShaderInputVariables() const;
    TStringList includedShaderSourcePathList() const;
    void addIncludeSource(const glslang::TString &key, const glslang::TString &value);

    void acceptTranslationUnit(atom_t unit);
    atom_t acceptStatement(atom_t statements, atom_t newStatement);
    atom_t acceptCompoundStatement(atom_t statements);
    atom_t acceptReturnStatement(atom_t expr);
    atom_t acceptBreakStatement();
    atom_t acceptContinueStatement();
    atom_t acceptDiscardStatement();
    atom_t acceptSelectBranchStatement(atom_t condExpr, atom_t trueExpr, atom_t falseExpr);
    atom_t acceptTernaryStatement(atom_t condExpr, atom_t trueExpr, atom_t falseExpr);
    atom_t acceptWhileLoopStatement(atom_t bodyExpr, atom_t condExpr, int first);
    atom_t acceptForLoopStatement(atom_t initializerExpr, atom_t condExpr, atom_t terminatorExpr, atom_t bodyExpr);
    atom_t acceptPostIncrement(atom_t expr);
    atom_t acceptPostDecrement(atom_t expr);
    atom_t acceptFunctionParameter(atom_t parameters, atom_t newParameter);
    atom_t acceptFunctionArgument(atom_t arguments, atom_t newArgument);
    atom_t acceptFunctionCall(atom_t name, atom_t arguments);
    atom_t acceptLocalVariable(atom_t variables, atom_t newVariable);
    atom_t createLocalVariable(atom_t identifier, atom_t arraySize, atom_t initializer);
    atom_t acceptDotDeference(atom_t expr, atom_t name);
    atom_t acceptBracketDeference(atom_t atomExpr, atom_t index);
    atom_t appendAssignmentExpr(atom_t left, atom_t right);
    atom_t acceptAssignmentExpr(atom_t left, atom_t op, atom_t right);
    atom_t acceptInitializerList(atom_t initializers, atom_t newInitializer);
    atom_t acceptArraySizeExpression(atom_t expressions, atom_t expr);
    atom_t acceptArraySizeSpecifier(atom_t specifier, atom_t expr);
    atom_t acceptUnaryMathPreIncrement(atom_t expr);
    atom_t acceptUnaryMathOperation(atom_t op, atom_t expr);
    atom_t acceptVariable(atom_t identifier);
    atom_t acceptUnaryMathPostIncrement(atom_t expr);
    atom_t acceptBinaryOperation(atom_t left, atom_t op, atom_t right);
    atom_t acceptConstructorCall(atom_t type, atom_t arguments);
    atom_t acceptCastOperation(atom_t type, atom_t sizeExpr, atom_t expr);
    atom_t setGlobalVariableTypeSpec(atom_t declarations, atom_t type, atom_t variables);
    atom_t acceptGlobalVariable(atom_t variables, atom_t newVariable);
    atom_t createGlobalVariable(atom_t name, atom_t sizeExpr, atom_t initializer, atom_t semanticAnnotations);
    atom_t acceptFunction(atom_t declaration, atom_t semantic, atom_t body);
    atom_t createFunction(atom_t spec, atom_t name, atom_t parameters);
    atom_t createFunctionParameter(atom_t spec, atom_t name, atom_t sizeExpr, atom_t semantic, atom_t initializer);
    atom_t createBinaryOperator(glslang::TOperator op);
    atom_t createUnaryOperator(glslang::TOperator op);
    atom_t setTypeSpecQualifier(atom_t qualifier, atom_t specifier);
    atom_t appendTypeQualifier(atom_t left, atom_t right);
    atom_t createType(const glslang::TType &type, const LexerToken *token);
    atom_t createType(const glslang::TBasicType &v, const LexerToken *token);
    atom_t createTextureType(const glslang::TSamplerDim &dim, const LexerToken *token);
    atom_t createSamplerType(const glslang::TSamplerDim &dim, const LexerToken *token);
    atom_t createVectorType(const glslang::TBasicType &v, int numComponents, const LexerToken *token);
    atom_t createMatrixType(const glslang::TBasicType &v, int numRows, int numColumns, const LexerToken *token);
    atom_t createStructType(atom_t name, atom_t body);
    atom_t appendStructMember(atom_t body, atom_t member);
    atom_t createStructMember(atom_t spec, atom_t member);
    atom_t appendStructMemberDeclarator(atom_t member, atom_t identifier);
    atom_t createStructMemberDeclarator(atom_t identifier, atom_t sizeExpr, atom_t semantic);
    atom_t createSamplerState(atom_t type, atom_t identifier, atom_t index, atom_t states);
    atom_t appendSamplerStateValue(atom_t states, atom_t newState);
    atom_t createSamplerStateTexture(atom_t identifier);
    atom_t createSamplerStateValue(atom_t identifier, atom_t value);
    atom_t appendAnnotation(atom_t annotations, atom_t newAnnotation);
    atom_t createAnnotation(atom_t type, atom_t identifier, atom_t initializer);
    atom_t appendStringList(atom_t stringList, atom_t newString);
    atom_t createSemanticAnnotations(atom_t semantic, atom_t annotations);
    atom_t createTechnique(atom_t identifier, atom_t passes, atom_t annotations);
    atom_t appendPass(atom_t passes, atom_t newPass);
    atom_t createPass(atom_t identifier, atom_t states, atom_t annotations);
    atom_t createStateBlock(atom_t identifier, atom_t block);
    atom_t appendPassState(atom_t states, atom_t newState);
    atom_t createPassState(atom_t identifier, atom_t value);
    atom_t createPassEntryPoint(atom_t type, atom_t identifier, atom_t arguments);
    atom_t acceptAttribute(atom_t attrs, atom_t name, atom_t value);
    atom_t setAllAttributes(atom_t statement, atom_t attrs);
    atom_t flattenExpression(atom_t expression);

    void pushScope();
    void popScope();
    void nestLooping();
    void unnestLooping();
    void concludeLocalVariableDeclaration();
    void enableAcceptingVariable();
    void disableAcceptingVariable();
    void enableUniformBuffer();
    void setVertexShaderInputMap(const BuiltInLocationMap &value);
    void setPixelShaderInputMap(const BuiltInLocationMap &value);
    void setPointSizeAssignment(float value);

    atom_t allocateIntermNode(TIntermNode *node);
    atom_t acceptTrueLiteral(const LexerToken *token);
    atom_t acceptFalseLiteral(const LexerToken *token);
    atom_t acceptIntLiteral(const LexerToken *token);
    atom_t acceptFloatLiteral(const LexerToken *token);
    atom_t acceptStringLiteral(const LexerToken *token);
    atom_t acceptIdentifier(const LexerToken *token);

private:
    using TIntermTypedNodeMap = glslang::TUnorderedMap<glslang::TString, glslang::TIntermTyped *>;
    using ParameterNameSet = std::unordered_set<glslang::TString>;
    struct InternalNode {
        InternalNode(TIntermNode *value);
        InternalNode();
        TIntermNode *m_value = nullptr;
        glslang::TFunction *m_functionPtr = nullptr;
    };
    using InternalNodeList = glslang::TVector<InternalNode>;
    struct MatrixSwizzleField {
        int rows[4];
        int columns[4];
        int offsetRow = 0;
        int offsetColumn = 0;
        int numIndices = 0;
        void setRowIndex(int value);
        void setColumnIndex(int value);
    };
    class IncluderContext : public glslang::TShader::Includer {
    public:
        IncluderContext();
        virtual ~IncluderContext();

        TStringList includedSourcePathList() const;
        void setSourceBasePath(const glslang::TString &value);
        void addSource(const glslang::TString &path, const glslang::TString &source);

        IncludeResult *includeSystem(
            const char *requested_source, const char *requesting_source, size_t /* inclusion_depth */) override;
        IncludeResult *includeLocal(
            const char *requested_source, const char *requesting_source, size_t /* inclusion_depth */) override;
        void releaseInclude(IncludeResult *result) override;

        const IncludeResult *currentIncludeResult() const;

    private:
        std::unordered_map<glslang::TString, glslang::TString *> m_sources;
        TStringList m_includedSourePathList;
        IncludeResult *m_resultPtr = nullptr;
        glslang::TString m_basePath = ".";
    };
    class SamplerNodeTraverser : public glslang::TIntermTraverser {
    public:
        using List = glslang::TVector<std::pair<glslang::TString, size_t>>;
        using Set = std::unordered_set<glslang::TString>;

        SamplerNodeTraverser(ParserContext *context, List *nodes, Set *names);
        ~SamplerNodeTraverser();

        void visitSymbol(glslang::TIntermSymbol *symbol);
        bool visitAggregate(glslang::TVisit, glslang::TIntermAggregate *aggregate);

        size_t acquireSamplerSlot();

    private:
        ParserContext *m_context;
        List *m_nodes;
        Set *m_names;
        std::vector<size_t> m_slots;
    };
    class OutputVertexShaderVariableTraverser : public glslang::TIntermTraverser {
    public:
        OutputVertexShaderVariableTraverser(const glslang::TString *name);
        ~OutputVertexShaderVariableTraverser();

        void visitSymbol(glslang::TIntermSymbol *symbol);
        bool hasFound() const;

    private:
        const glslang::TString *m_name;
        bool m_found = false;
    };

    glslang::TString *newAnonymousVariableString();
    InternalNode &allocateNode(atom_t &index);
    void createGlobalUniformVariable(int size);
    const glslang::TString *currentFunctionName() const;
    glslang::TIntermTyped *convertFunctionAssignment(glslang::TIntermAggregate *callerNode);
    glslang::TIntermTyped *findGlobalVariableNode(const glslang::TIntermSymbol *symbolNode);
    glslang::TIntermTyped *findGlobalVariableNode(const glslang::TString &name, const TIntermNode *node);
    glslang::TIntermTyped *normalizeGlobalUniformInitializer(const glslang::TIntermTyped *typeNode,
        const glslang::TString &name, glslang::TIntermTyped *initializerNode,
        glslang::TIntermBinary *semanticAnnotationNode);
    glslang::TIntermTyped *normalizeGlobalStaticInitializer(
        const glslang::TIntermTyped *typeNode, glslang::TIntermTyped *initializerNode);
    void fixupSwizzleVectorOperation(glslang::TIntermTyped *&leftNode, glslang::TIntermTyped *&rightNode);
    glslang::TIntermTyped *fixupSwizzleMatrixOperation(
        glslang::TIntermBinary *binaryNode, glslang::TIntermTyped *rightNode, const glslang::TOperator op);
    void fixupCastFloatOrBooleanOperation(
        glslang::TOperator op, glslang::TIntermTyped *&leftNode, glslang::TIntermTyped *&rightNode);
    void handleFunctionArgumentsConversion(const glslang::TIntermSequence &arguments, glslang::TFunction *&function,
        glslang::TIntermTyped *&newArgumentsNode);
    void handleFunctionArgumentsConversion(const glslang::TFunction *defined, const glslang::TIntermSequence &arguments,
        glslang::TFunction *&function, glslang::TIntermTyped *&newArgumentsNode);
    InternalNode *resolveNode(atom_t atom);
    TIntermNode *flattenAggregateNode(TIntermNode *node);
    TIntermNode *resolveIntermNode(atom_t atom);
    TIntermNode *resolveIntermStatement(atom_t atom);
    glslang::TIntermAggregate *resolveIntermAggregate(atom_t atom);
    TIntermNode *swizzleInitializer(const glslang::TIntermTyped *typeNode, const glslang::TIntermSymbol *nameNode,
        glslang::TArraySizes *arraySizes, glslang::TIntermTyped *initializerNode);
    glslang::TIntermTyped *swizzleVectorNode(glslang::TIntermTyped *baseNode, const glslang::TType &type);
    glslang::TIntermTyped *swizzleVectorNode(glslang::TIntermTyped *baseNode, int index);
    glslang::TIntermTyped *indexVectorNode(glslang::TIntermTyped *baseNode, int index);
    TIntermNode *createTextureSamplerAccessor(const glslang::TType &samplerType,
        const glslang::TIntermSymbol *textureNameNode, size_t samplerIndex, glslang::TIntermAggregate *argumentsNode,
        glslang::TIntermBinary *semanticAnnotationNode);
    void bindTextureToSampler(const glslang::TString &textureName, const glslang::TString &samplerName);
    TIntermNode *growAggregateNode(TIntermNode *left, TIntermNode *right);
    void handleFunctionArguments(
        glslang::TFunction *function, TIntermNode *node, glslang::TIntermTyped *&argumentsNode);
    const glslang::TString *globalUniformFloat4Name() const;
    const glslang::TString *globalSamplerRegisterName(size_t registerIndex) const;
    bool isFunctionDefined(const glslang::TString &name) const;
    glslang::TIntermAggregate *createVertexShaderEntryPoint(glslang::TIntermAggregate *unitNode,
        glslang::TFunction *function, glslang::TIntermAggregate *entryPointArguments);
    glslang::TIntermAggregate *createPixelShaderEntryPoint(glslang::TIntermAggregate *unitNode,
        glslang::TFunction *function, glslang::TIntermAggregate *entryPointArguments);
    void addExtraEntryPointFunctionArguments(glslang::TFunction *declaration,
        glslang::TIntermAggregate *entryPointArguments, glslang::TIntermTyped *&argumentsNode);
    glslang::TIntermAggregate *createMainFunction(
        glslang::TIntermAggregate *statement, glslang::TIntermAggregate *unitNode);
    glslang::TFunction *createVertexShaderEntryPointFunction(
        const glslang::TFunction *base, glslang::TIntermTyped *&arguments);
    void addVertexShaderEntryPointFunctionArgument(const glslang::TType &inputType, glslang::TBuiltInVariable builtIn,
        glslang::TFunction *declaration, glslang::TIntermTyped *&argumentsNode);
    TIntermNode *addBuiltInVertexShaderOutputAssignment(
        const glslang::TType &outputType, glslang::TBuiltInVariable builtIn, glslang::TIntermTyped *valueNode);
    TIntermNode *addVertexShaderOutputAssignment(
        const glslang::TType &outputType, glslang::TBuiltInVariable builtIn, glslang::TIntermTyped *callerNode);
    glslang::TFunction *createPixelShaderEntryPointFunction(
        const glslang::TFunction *base, glslang::TIntermTyped *&arguments);
    void traverseAllPixelShaderInputStructFields(const glslang::TType &inputType, const glslang::TTypeList *fields,
        glslang::TFunction *constructor, glslang::TIntermTyped *&arguments);
    void addPixelShaderEntryPointFunctionArgument(const glslang::TType &inputType, glslang::TBuiltInVariable builtIn,
        glslang::TFunction *declaration, glslang::TIntermTyped *&argumentsNode);
    glslang::TIntermAggregate *growVertexShaderBuiltInVariableAssignment(const glslang::TType &fieldType,
        glslang::TBuiltInVariable builtIn, glslang::TIntermAggregate *bodyNode, glslang::TIntermTyped *dereferenceNode);
    glslang::TIntermAggregate *growVertexShaderOutputStructAssignment(const glslang::TType &newOutputType,
        glslang::TIntermTyped *outputVariableNode, glslang::TIntermAggregate *bodyNode);
    TIntermNode *createBuiltInVertexShaderOutputAssignmentNode(
        const glslang::TFunction *function, glslang::TIntermTyped *outputCallNode);
    TIntermNode *createBuiltInPixelShaderOutputAssignmentNode(
        const glslang::TType &outputType, glslang::TIntermTyped *node);
    glslang::TIntermAggregate *createGlobalVariableInitializers();
    glslang::TIntermTyped *handleSingleConstructorCall(
        const glslang::TIntermTyped *typeNode, glslang::TIntermTyped *argNode);
    glslang::TIntermTyped *handleSingleConstructorCall(
        const glslang::TType &type, const TIntermNode *callerNode, glslang::TIntermTyped *argNode);
    glslang::TIntermTyped *handleSingleConstructorCall(
        const glslang::TSourceLoc &loc, const glslang::TType &type, glslang::TIntermTyped *argNode);
    glslang::TIntermSelection *fixupSelectionNode(
        glslang::TIntermTyped *condExprNode, TIntermNode *trueExprNode, TIntermNode *falseExprNode);
    glslang::TIntermTyped *handleSelectiveAssignment(
        glslang::TIntermTyped *leftNode, glslang::TOperator op, glslang::TIntermSelection *selectionNode);
    glslang::TIntermTyped *castBoolToFloat(glslang::TIntermTyped *node);
    glslang::TIntermTyped *castBoolToInteger(glslang::TIntermTyped *node);
    glslang::TIntermTyped *castFloatToBool(glslang::TIntermTyped *node);
    glslang::TIntermTyped *castIntegerToBool(glslang::TIntermTyped *node);
    bool isInitializerNodeConst(glslang::TIntermTyped *initializerNode);
    void foldAggregateNode(glslang::TIntermAggregate *node, glslang::TIntermTyped *&result);
    glslang::TIntermTyped *overrideBuiltInCall(
        const glslang::TFunction *function, glslang::TIntermTyped *argumentsNode);
    glslang::TIntermTyped *overrideBuiltInFractCall(
        glslang::TIntermTyped *argumentsNode, const glslang::TSourceLoc &loc);
    glslang::TIntermTyped *overrideBuiltInModfCall(
        glslang::TIntermTyped *argumentsNode, const glslang::TSourceLoc &loc);
    glslang::TIntermTyped *overrideBuiltInNoiseCall(glslang::TIntermTyped *argumentsNode);
    glslang::TIntermTyped *overrideBuiltInNormalizeCall(
        glslang::TIntermTyped *argumentsNode, const glslang::TSourceLoc &loc);
    glslang::TIntermTyped *overrideBuiltInAtanCall(const glslang::TFunction *builtInFunction,
        glslang::TIntermTyped *argumentsNode, const glslang::TSourceLoc &loc);
    glslang::TIntermTyped *overrideBuiltInPowCall(const glslang::TFunction *builtInFunction,
        glslang::TIntermTyped *argumentsNode, const glslang::TSourceLoc &loc);
    glslang::TIntermTyped *overrideBuiltInTextureLodCall(const glslang::TFunction *builtInFunction,
        glslang::TIntermTyped *argumentsNode, const glslang::TSourceLoc &loc);
    glslang::TIntermTyped *castNaNToZero(
        glslang::TIntermTyped *baseNode, const glslang::TType &type, const glslang::TSourceLoc &loc);
    glslang::TBuiltInVariable findBuiltInFromStructMember(
        const glslang::TType &type, const glslang::TType &field) const;
    glslang::TBuiltInVariable findBuiltInFromStructMember(
        const glslang::TString &typeName, const glslang::TString &fieldName) const;
    void registerFunctionParameter(const glslang::TString &functionName, const glslang::TIntermSymbol *symbol);
    void copyFunctionParameterSet(const glslang::TString &sourcePassName, const glslang::TString &destPassName);
    void decorateSemanticType(atom_t semantic, glslang::TType &type);
    void incrementRegisterIndex(const glslang::TType &type);
    void handleArraySizeSpecifier(TIntermNode *node, glslang::TType &type);
    void handleArraySizeSpecifier(glslang::TIntermAggregate *node, glslang::TType &type);
    void setBuiltInSemantics(const glslang::TString &semantic, glslang::TType &type);

    const BuiltInVariableMap &m_vertexShaderInputVariables;
    const BuiltInVariableMap &m_pixelShaderInputVariables;
    LexerToken *m_lastLexerToken = nullptr;
    glslang::HlslParseContext *m_context = nullptr;
    glslang::TFunction *m_currentFunction = nullptr;
    glslang::TIntermTyped *m_currentTypeSpecNode = nullptr;
    glslang::TVector<glslang::TIntermTyped *> m_outputVertexShaderNodes;
    glslang::TUnorderedMap<glslang::TString, std::pair<glslang::TIntermTyped *, glslang::TBuiltInVariable>>
        m_outputPixelShaderNodes;
    glslang::TIntermediate m_intermediate;
    glslang::TSymbolTable m_symbolTable;
    glslang::TString m_filename;
    IncluderContext m_includer;
    InternalNodeList m_nodes;
    BuiltInVariableMap m_writtenVertexShaderInputVariables;
    glslang::TUnorderedMap<glslang::TString,
        glslang::TVector<std::pair<glslang::TFunction *, glslang::TIntermAggregate *>>>
        m_allFunctions;
    glslang::TUnorderedMap<glslang::TString, glslang::TUnorderedMap<glslang::TString, glslang::TBuiltInVariable>>
        m_builtInStructMembers;
    glslang::TUnorderedMap<glslang::TString, glslang::TBuiltInVariable> m_builtInSemantics;
    glslang::TUnorderedMap<int, std::pair<glslang::TStorageQualifier, glslang::TBuiltInVariable>>
        m_vertexShadreBuiltInVariableConversions;
    glslang::TUnorderedMap<int, glslang::TStorageQualifier> m_pixelShaderBuiltInVariableConversions;
    NodeMap m_uniformNodes;
    NodeMap m_constantNodes;
    SamplerNodeMap m_samplerNodes;
    TextureNodeMap m_textureNodes;
    AnnotationListMap m_annotations;
    StateBlockMap m_stateBlocks;
    TIntermTypedNodeMap m_immediateConstantParameters;
    TIntermTypedNodeMap m_expressionConstantParameters;
    TIntermTypedNodeMap m_uniformParameters;
    TStringList m_globalInitializerNameOrder;
    glslang::TUnorderedMap<glslang::TString, glslang::TVector<glslang::TBuiltInVariable>> m_builtInVariables;
    glslang::TUnorderedMap<glslang::TString, glslang::TIntermAggregate *> m_passEntryArguments;
    glslang::TUnorderedMap<glslang::TFunction *, TIntermTypedNodeMap> m_staticParameters;
    glslang::TUnorderedMap<glslang::TString, ParameterNameSet *> m_usedFunctionParameters;
    glslang::TVector<glslang::TIntermAggregate *> m_textureCallerNodes;
    std::unordered_set<uint32_t> m_samplerIndexSet;
    TStringSet m_reservedWordSet;
    TechniqueList m_techniques;
    Pass::EntryPoint m_entryPoint;
    BuiltInLocationMap m_vertexShaderInputMap;
    BuiltInLocationMap m_pixelShaderInputMap;
    size_t m_lastUniformRegisterIndex = 0;
    float m_pointSizeAssignment = 0.0f;
    int m_uniqueID = 1;
    bool m_enableAcceptingVariable = true;
    bool m_enableUniformBuffer = false;
};

} /* namespace fx9 */

#endif /* FX9_PARSER_H_ */
