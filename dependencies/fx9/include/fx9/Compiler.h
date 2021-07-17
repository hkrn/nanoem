/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is licensed under MIT license. for more details, see LICENSE.txt.
 */

#pragma once
#ifndef FX9_COMPILER_H_
#define FX9_COMPILER_H_

#include <memory>
#include <stdint.h>

#include "Parser.h"

struct glslopt_ctx;

namespace spv {
class SpvBuildLogger;
}
namespace spvtools {
class Optimizer;
}
namespace spirv_cross {
class Compiler;
}

namespace fx9 {

class Compiler {
public:
    enum LanguageType {
        kLanguageTypeFirstEnum,
        kLanguageTypeGLSL,
        kLanguageTypeESSL,
        kLanguageTypeHLSL,
        kLanguageTypeMSL,
        kLanguageTypeSPIRV,
        kLanguageTypeMaxEnum
    };
    struct EffectProduct {
        struct LogSink {
            using StringSet = std::unordered_set<std::string>;
            std::string info;
            std::string debug;
            std::string builder;
            std::string validator;
            StringSet translator;
            StringSet optimizer;
            bool
            isEmpty() const
            {
                return info.empty() && debug.empty() && builder.empty() && translator.empty() && optimizer.empty() &&
                    validator.empty();
            }
            void
            merge(const std::string &name, const TInfoSink &infoSink)
            {
                debug += "[" + name + "]\n" + infoSink.debug.c_str();
                info += "[" + name + "]\n" + infoSink.info.c_str();
            }
        } sink;
        std::vector<uint8_t> message;
        size_t numPasses = 0;
        size_t numCompiledPasses = 0;
        size_t numValidatedPasses = 0;
        bool
        hasAllPassCompiled() const
        {
            return numPasses > 0 && numPasses == numCompiledPasses;
        }
        bool
        hasAnyCompiledPass() const
        {
            return numPasses > 0 && numCompiledPasses > 0;
        }
        bool
        isEmpty() const
        {
            return numPasses == 0;
        }
    };

    static void initialize();
    static void terminate();

    Compiler(EProfile profile, EShMessages messages);
    ~Compiler();

    bool compile(const char *path, EffectProduct &effectProduct);
    bool compile(const std::string &source, const char *filename, EffectProduct &effectProduct);
    void addIncludeSource(const std::string &filePath, const std::string &sourceData);
    void setDefineMacro(const std::string &key, const std::string &value);
    bool containsDefineMacro(const std::string &key) const;
    void removeDefineMacro(const std::string &key);

    ParserContext::BuiltInLocationMap vertexShaderInputLocations() const;
    void setVertexShaderInputLocations(const ParserContext::BuiltInLocationMap &value);
    ParserContext::BuiltInVariableMap vertexShaderInputVariables() const;
    void setVertexShaderInputVariables(const ParserContext::BuiltInVariableMap &value);
    ParserContext::BuiltInVariableMap pixelShaderInputVariables() const;
    void setPixelShaderInputVariables(const ParserContext::BuiltInVariableMap &value);
    std::string metalShaderEntryPoint() const;
    void setMetalShaderEntryPoint(const std::string &value);
    std::string metalShaderUniformBufferName() const;
    void setMetalShaderUniformBufferName(const std::string &value);
    LanguageType targetLanguage() const;
    void setTargetLanguage(LanguageType value);
    int version() const;
    void setVersion(int value);
    bool isOptimizeEnabled() const;
    void setOptimizeEnabled(bool value);
    bool isValidationEnabled() const;
    void setValidationEnabled(bool value);

    template <typename TType> TType *allocate();
    template <typename TType> TType **allocateArray(size_t numItems);

private:
    using Convertions = std::unordered_map<std::string, uint32_t>;
    using InstructionList = std::vector<uint32_t>;
    struct BaseParameterConverter {
        BaseParameterConverter(
            const ParserContext &parser, Compiler *parent, EffectProduct &effectProduct, void *opaque);
        virtual ~BaseParameterConverter();

        void convertAllConstantParameters(void *opaque, size_t &index);
        void convertAllUniformParameters(void *opaque, size_t &index);
        void convertAllSamplerParameters(void *opaque, size_t &index);
        void convertAllTextureParameters(void *opaque, size_t &index);
        void convertAll();
        void fillParameterValues(const ParserContext::NodeItem *value, void *opaque);
        void fillParameterFromConstantUnionNode(
            void *opaque, const glslang::TIntermConstantUnion *constantUnionNode, int maxComponents, size_t &offset);
        void assignIntegerParameter(void *opaque, size_t offset, int value);
        void assignFloatParameter(void *opaque, size_t offset, float value);

        void convertParameterType(const glslang::TType &type, void *opaque);
        void convertParameter(const glslang::TType &type, const glslang::TString &name,
            const ParserContext::NodeItem *value, const glslang::TIntermBinary *semanticAnnotationNode, void *opaque);

        const ParserContext &m_parser;
        EffectProduct &m_effectProduct;
        Compiler *m_parent = nullptr;
        void *m_opaque = nullptr;
    };
    struct BasePassShader {
        BasePassShader(Compiler *parent, const char *path, const glslang::TString &source, EffectProduct &effectProduct,
            void *opaque);
        virtual ~BasePassShader();

        void convertPassState(
            const glslang::TString &name, const ParserContext::PassState &state, uint32_t &key, uint32_t &value) const;
        void convertAllSamplerStates(const glslang::TIntermAggregate *samplerStates, void *opaque) const;
        bool generateSPVInstructions(ParserContext &parser, const ParserContext::Pass::EntryPoint &entryPoint,
            InstructionList &instructions, TInfoSink &infoSink);
        bool compile(
            EShLanguage language, const ParserContext::Pass::EntryPoint &entryPoint, InstructionList &instructions);
        void convertAllPassStates(const ParserContext::Pass &pass);

        virtual void configureParserContext(ParserContext &parser) = 0;
        virtual bool translate(const InstructionList &vertexShaderInstructions,
            const InstructionList &fragmentShaderInstructions, std::string &translatedVertexShaderSource,
            std::string &translatedFragmentShaderSource, EffectProduct::LogSink &sink) = 0;

        const glslang::TString m_source;
        const char *m_path = nullptr;
        ParserContext::BuiltInVariableMap m_writtenVertexShaderInputVariables;
        glslang::TUnorderedMap<std::string, int> m_samplerName2Index[EShLangCount];
        EffectProduct &m_effectProduct;
        Compiler *m_parent = nullptr;
        void *m_opaque = nullptr;
    };
    struct DX9MSPassShader : BasePassShader {
        DX9MSPassShader(Compiler *parent, const char *path, const glslang::TString &source,
            EffectProduct &effectProduct, void *opaque);
        ~DX9MSPassShader() override;

        void configureParserContext(ParserContext &parser) override;
        bool translate(const InstructionList &vertexShaderInstructions,
            const InstructionList &fragmentShaderInstructions, std::string &translatedVertexShaderSource,
            std::string &translatedFragmentShaderSource, EffectProduct::LogSink &sink) override;
    };
    struct MSLPassShader : BasePassShader {
        MSLPassShader(Compiler *parent, const char *path, const glslang::TString &source, EffectProduct &effectProduct,
            void *opaque);
        ~MSLPassShader() override;

        void configureParserContext(ParserContext &parser) override;
        bool translate(const InstructionList &vertexShaderInstructions,
            const InstructionList &fragmentShaderInstructions, std::string &translatedVertexShaderSource,
            std::string &translatedFragmentShaderSource, EffectProduct::LogSink &sink) override;
    };
    struct HLSLPassShader : BasePassShader {
        HLSLPassShader(Compiler *parent, const char *path, const glslang::TString &source, EffectProduct &effectProduct,
            void *opaque);
        ~HLSLPassShader() override;

        void configureParserContext(ParserContext &parser) override;
        bool translate(const InstructionList &vertexShaderInstructions,
            const InstructionList &fragmentShaderInstructions, std::string &translatedVertexShaderSource,
            std::string &translatedFragmentShaderSource, EffectProduct::LogSink &sink) override;
    };
    struct SPIRVPassShader : BasePassShader {
        SPIRVPassShader(Compiler *parent, const char *path, const glslang::TString &source,
            EffectProduct &effectProduct, void *opaque);
        ~SPIRVPassShader() override;

        void configureParserContext(ParserContext &parser) override;
        bool translate(const InstructionList &vertexShaderInstructions,
            const InstructionList &fragmentShaderInstructions, std::string &translatedVertexShaderSource,
            std::string &translatedFragmentShaderSource, EffectProduct::LogSink &sink) override;
    };

    static uint32_t resolveRenderStateValue(const ParserContext::PassState &state, const Convertions &conversions);
    static uint32_t resolveSamplerStateValue(const glslang::TIntermSymbol *valueNode, const Convertions &conversions);
    static void copyString(const glslang::TString &source, glslang::TPoolAllocator &allocator, char **destination);

    void copyString(const glslang::TString &source, char **destination);
    void initializeBuiltInSymbolTable(
        const glslang::TString &builtIn, EShLanguage language, glslang::TSymbolTable &outputSymbolTable);
    void initializeBuiltInSymbolTable(EShLanguage language, glslang::TBuiltInParseables &builtIn,
        glslang::TSymbolTable &commonSymbolTable, glslang::TSymbolTable &outputSymbolTable);
    bool addContextSpecificSymbols(
        const TBuiltInResource *resources, EShLanguage language, glslang::TSymbolTable &symbolTable);
    void compileAllTechniques(const ParserContext::TechniqueList &techniques, const char *path,
        const glslang::TString &source, EffectProduct &effectProduct, void *opaque);
    void compileAllPasses(const ParserContext::Technique &technique, const char *path, const glslang::TString &source,
        EffectProduct &effectProduct, void *opaque);
    void convertAnnotation(const glslang::TString &name, const TIntermNode *value, void *opaque);
    void convertAllAnnotations(const ParserContext::AnnotationList &annotations, void *opaque, size_t *numAnnotations);
    void convertAllParameterAnnotations(const glslang::TIntermTyped *annotationsNode, void *opaque);
    void convertMetadata(const glslang::TUnorderedMap<glslang::TString, glslang::TString> &value, void *opaque);
    void convertIncludePathSet(const ParserContext &parser, void *opaque);
    bool generateSPVInstructions(const ParserContext::Pass::EntryPoint &entryPoint, const glslang::TString &source,
        ParserContext &parser, InstructionList &instructions, spv::SpvBuildLogger *logger, TInfoSink &sink);
    void saveAttributeMap(const InstructionList &instructions, std::unordered_map<uint32_t, std::string> &attributes);
    void restoreInterfaceVariableNames(EShLanguage language,
        const std::unordered_map<uint32_t, std::string> &attributes, spirv_cross::Compiler &compiler);
    void optimizeShaderInstructions(
        const InstructionList &instructions, InstructionList &newInstructions, EffectProduct::LogSink &sink);
    bool validateShaderSource(EShLanguage language, const std::string &shaderSource, std::string &message,
        std::unique_ptr<glslang::TShader> &outputShader);
    void rebuildAllSymbolTables();

    Convertions m_renderStateEnumConversions;
    Convertions m_renderStateShadeModeValueConversions;
    Convertions m_renderStateFillModeValueConversions;
    Convertions m_renderStateBlendModeValueConversions;
    Convertions m_renderStateBlendOpValueConversions;
    Convertions m_renderStateTextureAddressValueConversions;
    Convertions m_renderStateCullValueConversions;
    Convertions m_renderStateCmpFuncValueConversions;
    Convertions m_renderStateStencilOpValueConversions;
    Convertions m_renderStateFogModeValueConversions;
    Convertions m_renderStateZBufferTypeValueConversions;
    Convertions m_renderStatePrimitiveTypeValueConversions;
    Convertions m_renderStateTransformStateTypeValueConversions;
    Convertions m_samplerStateEnumConversions;
    Convertions m_samplerStateTextureAddressValueConversions;
    Convertions m_samplerStateTextureStageStateValueConversions;
    Convertions m_samplerStateTextureOpValueConversions;
    Convertions m_samplerStateTextureFilterTypeValueConversions;
    ParserContext::BuiltInVariableMap m_vertexShaderInputVariables;
    ParserContext::BuiltInVariableMap m_pixelShaderInputVariables;
    ParserContext::BuiltInLocationMap m_vertexShaderInputLocations;
    std::unordered_map<std::string, std::string> m_includeSourceData;
    std::unordered_map<std::string, std::string> m_macros;
    std::unique_ptr<glslang::TPoolAllocator> m_allocator;
    std::unique_ptr<glslang::TSymbolTable> m_commonSymbolTable;
    std::unique_ptr<glslang::TSymbolTable> m_fragmentSymbolTable;
    std::string m_metalShaderEntryPointName = "fx9_metal_main";
    std::string m_metalShaderUniformBufferName = "fx9_metal_ub";
    const EShMessages m_messages;
    EProfile m_profile;
    LanguageType m_language = kLanguageTypeGLSL;
    int m_version = 330;
    bool m_enableValidation = false;
    bool m_enableOptimize = false;
};

} /* namespace fx9 */

#endif /* FX9_COMPILER_H_ */
