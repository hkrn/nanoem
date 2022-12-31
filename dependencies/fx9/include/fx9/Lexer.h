/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is licensed under MIT license. for more details, see LICENSE.txt.
 */

#pragma once
#ifndef FX9_LEXER_H_
#define FX9_LEXER_H_

/* GLSLang */
#include "glslang/MachineIndependent/Initialize.h"
#include "glslang/MachineIndependent/ParseHelper.h"
#include "glslang/MachineIndependent/preprocessor/PpContext.h"
#include "glslang/MachineIndependent/preprocessor/PpTokens.h"

namespace fx9 {

class ParserContext;

struct LexerToken {
    POOL_ALLOCATOR_NEW_DELETE(glslang::GetThreadPoolAllocator())
    glslang::TPpToken value;
    int major;
};

class LexerContext : public glslang::TPpContext {
public:
    LexerContext(glslang::TParseContextBase &parserContext, const std::string &rootFileName,
        glslang::TShader::Includer &includer);
    ~LexerContext();

    int scan(const ParserContext *context, glslang::TPpToken &token);

private:
    std::unordered_map<std::string, int> m_keywordIdentifiers;
};

} /* namespace fx9 */

#endif /* FX9_LEXER_H_ */
