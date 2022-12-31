/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is licensed under MIT license. for more details, see LICENSE.txt.
 */

#define ENABLE_HLSL
#include "fx9/Lemon.h"
#include "fx9/Lexer.h"
#include "fx9/Parser.h"

struct fx9_parser_context_t;
struct fx9_lexer_token_t;

using namespace fx9;
using namespace glslang;

extern "C" {

void
fx9ParserContextReportSyntaxError(fx9_parser_context_t *context, fx9_atom_t token)
{
    ParserContext *parserContext = reinterpret_cast<ParserContext *>(context);
    HlslParseContext *hlslParseContext = parserContext->parseContext();
    TInfoSinkBase &output = hlslParseContext->infoSink.info;
    output.prefix(EPrefixError);
    output << "Syntax Error in ";
    if (const LexerToken *lexerToken = reinterpret_cast<const LexerToken *>(token)) {
        const TSourceLoc &loc = lexerToken->value.loc;
        const char *filename = loc.name ? loc.name->c_str() : parserContext->currentProcessingFilename();
        output << filename << " at " << loc.line << ":" << loc.column;
    }
    else if (const LexerToken *lastLexerToken = parserContext->lastLexerToken()) {
        const TSourceLoc &loc = lastLexerToken->value.loc;
        const char *filename = loc.name ? loc.name->c_str() : parserContext->currentProcessingFilename();
        output << filename << " near at " << loc.line << ":" << loc.column;
    }
    output << "\n";
    hlslParseContext->addError();
}

void
fx9ParserContextReportParseFailure(fx9_parser_context_t * /* context */)
{
}

void
fx9ParserContextAcceptTranslationUnit(fx9_parser_context_t *context, atom_t unit)
{
    reinterpret_cast<ParserContext *>(context)->acceptTranslationUnit(unit);
}

fx9_atom_t
fx9ParserContextAcceptExternalDeclaration(
    fx9_parser_context_t *context, fx9_atom_t declarations, fx9_atom_t newDeclaration)
{
    return reinterpret_cast<ParserContext *>(context)->acceptStatement(declarations, newDeclaration);
}

fx9_atom_t
fx9ParserContextAcceptStatement(fx9_parser_context_t *context, fx9_atom_t statements, fx9_atom_t newStatement)
{
    return reinterpret_cast<ParserContext *>(context)->acceptStatement(statements, newStatement);
}

fx9_atom_t
fx9ParserContextAcceptCompoundStatement(fx9_parser_context_t *context, fx9_atom_t statements)
{
    return reinterpret_cast<ParserContext *>(context)->acceptCompoundStatement(statements);
}

fx9_atom_t
fx9ParserContextAcceptJumpStatement(fx9_parser_context_t *context, fx9_atom_t statement, fx9_atom_t expr)
{
    return reinterpret_cast<ParserContext *>(context)->acceptStatement(statement, expr);
}

fx9_atom_t
fx9ParserContextAcceptSelectionStatement(fx9_parser_context_t *context, fx9_atom_t statement, fx9_atom_t expr)
{
    return reinterpret_cast<ParserContext *>(context)->acceptStatement(statement, expr);
}

fx9_atom_t
fx9ParserContextAcceptIterationStatement(fx9_parser_context_t *context, fx9_atom_t statement, fx9_atom_t expr)
{
    return reinterpret_cast<ParserContext *>(context)->acceptStatement(statement, expr);
}

void
fx9ParserContextPushScope(fx9_parser_context_t *context)
{
    reinterpret_cast<ParserContext *>(context)->pushScope();
}

void
fx9ParserContextPopScope(fx9_parser_context_t *context)
{
    reinterpret_cast<ParserContext *>(context)->popScope();
}

void
fx9ParserContextNestLooping(fx9_parser_context_t *context)
{
    reinterpret_cast<ParserContext *>(context)->nestLooping();
}

void
fx9ParserContextUnnestLooping(fx9_parser_context_t *context)
{
    reinterpret_cast<ParserContext *>(context)->unnestLooping();
}

fx9_atom_t
fx9ParserContextSetGlobalVariableTypeSpec(
    fx9_parser_context_t *context, fx9_atom_t declarations, fx9_atom_t type, fx9_atom_t variables)
{
    return reinterpret_cast<ParserContext *>(context)->setGlobalVariableTypeSpec(declarations, type, variables);
}

fx9_atom_t
fx9ParserContextAcceptGlobalVariable(fx9_parser_context_t *context, fx9_atom_t variables, fx9_atom_t newVariable)
{
    return reinterpret_cast<ParserContext *>(context)->acceptGlobalVariable(variables, newVariable);
}

fx9_atom_t
fx9ParserContextCreateGlobalVariable(fx9_parser_context_t *context, fx9_atom_t name, fx9_atom_t arraySize,
    fx9_atom_t initializer, fx9_atom_t semanticAnnotations)
{
    return reinterpret_cast<ParserContext *>(context)->createGlobalVariable(
        name, arraySize, initializer, semanticAnnotations);
}

fx9_atom_t
fx9ParserContextAcceptFunction(
    fx9_parser_context_t *context, fx9_atom_t declaration, fx9_atom_t semantic, fx9_atom_t implementation)
{
    return reinterpret_cast<ParserContext *>(context)->acceptFunction(declaration, semantic, implementation);
}

fx9_atom_t
fx9ParserContextCreateFunction(
    fx9_parser_context_t *context, fx9_atom_t spec, fx9_atom_t identifier, fx9_atom_t parameters)
{
    return reinterpret_cast<ParserContext *>(context)->createFunction(spec, identifier, parameters);
}

fx9_atom_t
fx9ParserContextAcceptFunctionParameter(fx9_parser_context_t *context, fx9_atom_t arguments, fx9_atom_t newArgument)
{
    return reinterpret_cast<ParserContext *>(context)->acceptFunctionParameter(arguments, newArgument);
}

fx9_atom_t
fx9ParserContextCreateFunctionParameter(fx9_parser_context_t *context, fx9_atom_t spec, fx9_atom_t name,
    fx9_atom_t arraySize, fx9_atom_t semantic, fx9_atom_t initializer)
{
    return reinterpret_cast<ParserContext *>(context)->createFunctionParameter(
        spec, name, arraySize, semantic, initializer);
}

fx9_atom_t
fx9ParserContextAcceptReturnStatement(fx9_parser_context_t *context, fx9_atom_t expr)
{
    return reinterpret_cast<ParserContext *>(context)->acceptReturnStatement(expr);
}

fx9_atom_t
fx9ParserContextAcceptBreakStatement(fx9_parser_context_t *context)
{
    return reinterpret_cast<ParserContext *>(context)->acceptBreakStatement();
}

fx9_atom_t
fx9ParserContextAcceptContinueStatement(fx9_parser_context_t *context)
{
    return reinterpret_cast<ParserContext *>(context)->acceptContinueStatement();
}

fx9_atom_t
fx9ParserContextAcceptDiscardStatement(fx9_parser_context_t *context)
{
    return reinterpret_cast<ParserContext *>(context)->acceptDiscardStatement();
}

fx9_atom_t
fx9ParserContextAcceptSelectBranchStatement(
    fx9_parser_context_t *context, fx9_atom_t condExpr, fx9_atom_t trueExpr, fx9_atom_t falseExpr)
{
    return reinterpret_cast<ParserContext *>(context)->acceptSelectBranchStatement(condExpr, trueExpr, falseExpr);
}

fx9_atom_t
fx9ParserContextAcceptTernaryStatement(
    fx9_parser_context_t *context, fx9_atom_t condExpr, fx9_atom_t trueExpr, fx9_atom_t falseExpr)
{
    return reinterpret_cast<ParserContext *>(context)->acceptTernaryStatement(condExpr, trueExpr, falseExpr);
}

fx9_atom_t
fx9ParserContextAcceptWhileLoopStatement(
    fx9_parser_context_t *context, fx9_atom_t bodyExpr, fx9_atom_t condExpr, int first)
{
    return reinterpret_cast<ParserContext *>(context)->acceptWhileLoopStatement(bodyExpr, condExpr, first);
}

fx9_atom_t
fx9ParserContextAcceptForLoopStatement(fx9_parser_context_t *context, fx9_atom_t initializerExpr, fx9_atom_t condExpr,
    fx9_atom_t terminatorExpr, fx9_atom_t bodyExpr)
{
    return reinterpret_cast<ParserContext *>(context)->acceptForLoopStatement(
        initializerExpr, condExpr, terminatorExpr, bodyExpr);
}

fx9_atom_t
fx9ParserContextAcceptPostIncrement(fx9_parser_context_t *context, fx9_atom_t expr)
{
    return reinterpret_cast<ParserContext *>(context)->acceptPostIncrement(expr);
}

fx9_atom_t
fx9ParserContextAcceptPostDecrement(fx9_parser_context_t *context, fx9_atom_t expr)
{
    return reinterpret_cast<ParserContext *>(context)->acceptPostDecrement(expr);
}

fx9_atom_t
fx9ParserContextAcceptFunctionArgument(fx9_parser_context_t *context, fx9_atom_t arguments, fx9_atom_t newArgument)
{
    return reinterpret_cast<ParserContext *>(context)->acceptFunctionArgument(arguments, newArgument);
}

fx9_atom_t
fx9ParserContextAcceptFunctionCall(fx9_parser_context_t *context, fx9_atom_t expr, fx9_atom_t arguments)
{
    return reinterpret_cast<ParserContext *>(context)->acceptFunctionCall(expr, arguments);
}

fx9_atom_t
fx9ParserContextAcceptLocalVariable(fx9_parser_context_t *context, fx9_atom_t variables, fx9_atom_t newVariable)
{
    return reinterpret_cast<ParserContext *>(context)->acceptLocalVariable(variables, newVariable);
}

fx9_atom_t
fx9ParserContextCreateLocalVariable(
    fx9_parser_context_t *context, fx9_atom_t identifier, fx9_atom_t arraySize, fx9_atom_t initializer)
{
    return reinterpret_cast<ParserContext *>(context)->createLocalVariable(identifier, arraySize, initializer);
}

fx9_atom_t
fx9ParserContextAcceptDotDeference(fx9_parser_context_t *context, fx9_atom_t expr, fx9_atom_t name)
{
    return reinterpret_cast<ParserContext *>(context)->acceptDotDeference(expr, name);
}

fx9_atom_t
fx9ParserContextAcceptBracketDeference(fx9_parser_context_t *context, fx9_atom_t expr, fx9_atom_t index)
{
    return reinterpret_cast<ParserContext *>(context)->acceptBracketDeference(expr, index);
}

fx9_atom_t
fx9ParserContextAcceptTrueLiteral(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->acceptTrueLiteral(reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextAcceptFalseLiteral(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->acceptFalseLiteral(reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextAcceptIntLiteral(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->acceptIntLiteral(reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextAcceptFloatLiteral(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->acceptFloatLiteral(reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextAcceptStringLiteral(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->acceptStringLiteral(reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextAcceptAttribute(fx9_parser_context_t *context, fx9_atom_t attrs, fx9_atom_t name, fx9_atom_t value)
{
    return reinterpret_cast<ParserContext *>(context)->acceptAttribute(attrs, name, value);
}

fx9_atom_t
fx9ParserContextSetAllAttributes(fx9_parser_context_t *context, fx9_atom_t statement, fx9_atom_t attrs)
{
    return reinterpret_cast<ParserContext *>(context)->setAllAttributes(statement, attrs);
}

fx9_atom_t
fx9ParserContextFlattenExpression(fx9_parser_context_t *context, fx9_atom_t expression)
{
    return reinterpret_cast<ParserContext *>(context)->flattenExpression(expression);
}

fx9_atom_t
fx9ParserContextAcceptArraySizeSpecifier(fx9_parser_context_t *context, fx9_atom_t specifier, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->acceptArraySizeSpecifier(specifier, token);
}

fx9_atom_t
fx9ParserContextAcceptIndetermineArraySizeSpecifier(fx9_parser_context_t *context, fx9_atom_t specifier)
{
    return reinterpret_cast<ParserContext *>(context)->acceptArraySizeSpecifier(specifier, 0);
}

fx9_atom_t
fx9ParserContextAcceptIdentifier(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->acceptIdentifier(reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextAppendAssignmentExpr(fx9_parser_context_t *context, fx9_atom_t left, fx9_atom_t right)
{
    return reinterpret_cast<ParserContext *>(context)->appendAssignmentExpr(left, right);
}

fx9_atom_t
fx9ParserContextAcceptAssignmentExpr(fx9_parser_context_t *context, fx9_atom_t left, fx9_atom_t op, fx9_atom_t right)
{
    return reinterpret_cast<ParserContext *>(context)->acceptAssignmentExpr(left, op, right);
}

fx9_atom_t
fx9ParserContextAcceptInitializerList(fx9_parser_context_t *context, fx9_atom_t initializers, fx9_atom_t newInitializer)
{
    return reinterpret_cast<ParserContext *>(context)->acceptInitializerList(initializers, newInitializer);
}

fx9_atom_t
fx9ParserContextAcceptBinaryOperation(fx9_parser_context_t *context, fx9_atom_t left, fx9_atom_t op, fx9_atom_t right)
{
    return reinterpret_cast<ParserContext *>(context)->acceptBinaryOperation(left, op, right);
}

fx9_atom_t
fx9ParserContextAcceptConstructorCall(fx9_parser_context_t *context, fx9_atom_t type, fx9_atom_t expr)
{
    return reinterpret_cast<ParserContext *>(context)->acceptConstructorCall(type, expr);
}

fx9_atom_t
fx9ParserContextAcceptCastOperation(
    fx9_parser_context_t *context, fx9_atom_t type, fx9_atom_t arraySize, fx9_atom_t expr)
{
    return reinterpret_cast<ParserContext *>(context)->acceptCastOperation(type, arraySize, expr);
}

fx9_atom_t
fx9ParserContextGetAssignOperator(fx9_parser_context_t *context)
{
    return reinterpret_cast<ParserContext *>(context)->createBinaryOperator(EOpAssign);
}

fx9_atom_t
fx9ParserContextGetMulAssignOperator(fx9_parser_context_t *context)
{
    return reinterpret_cast<ParserContext *>(context)->createBinaryOperator(EOpMulAssign);
}

fx9_atom_t
fx9ParserContextGetDivAssignOperator(fx9_parser_context_t *context)
{
    return reinterpret_cast<ParserContext *>(context)->createBinaryOperator(EOpDivAssign);
}

fx9_atom_t
fx9ParserContextGetModAssignOperator(fx9_parser_context_t *context)
{
    return reinterpret_cast<ParserContext *>(context)->createBinaryOperator(EOpModAssign);
}

fx9_atom_t
fx9ParserContextGetPlusAssignOperator(fx9_parser_context_t *context)
{
    return reinterpret_cast<ParserContext *>(context)->createBinaryOperator(EOpAddAssign);
}

fx9_atom_t
fx9ParserContextGetMinusAssignOperator(fx9_parser_context_t *context)
{
    return reinterpret_cast<ParserContext *>(context)->createBinaryOperator(EOpSubAssign);
}

fx9_atom_t
fx9ParserContextGetLeftShiftAssignOperator(fx9_parser_context_t *context)
{
    return reinterpret_cast<ParserContext *>(context)->createBinaryOperator(EOpLeftShiftAssign);
}

fx9_atom_t
fx9ParserContextGetRightShiftAssignOperator(fx9_parser_context_t *context)
{
    return reinterpret_cast<ParserContext *>(context)->createBinaryOperator(EOpRightShiftAssign);
}

fx9_atom_t
fx9ParserContextGetAndAssignOperator(fx9_parser_context_t *context)
{
    return reinterpret_cast<ParserContext *>(context)->createBinaryOperator(EOpAndAssign);
}

fx9_atom_t
fx9ParserContextGetXorAssignOperator(fx9_parser_context_t *context)
{
    return reinterpret_cast<ParserContext *>(context)->createBinaryOperator(EOpExclusiveOrAssign);
}

fx9_atom_t
fx9ParserContextGetOrAssignOperator(fx9_parser_context_t *context)
{
    return reinterpret_cast<ParserContext *>(context)->createBinaryOperator(EOpInclusiveOrAssign);
}

fx9_atom_t
fx9ParserContextCreateBinaryOperatorStar(fx9_parser_context_t *context)
{
    return reinterpret_cast<ParserContext *>(context)->createBinaryOperator(EOpMul);
}

fx9_atom_t
fx9ParserContextCreateBinaryOperatorSlash(fx9_parser_context_t *context)
{
    return reinterpret_cast<ParserContext *>(context)->createBinaryOperator(EOpDiv);
}

fx9_atom_t
fx9ParserContextCreateBinaryOperatorPercent(fx9_parser_context_t *context)
{
    return reinterpret_cast<ParserContext *>(context)->createBinaryOperator(EOpMod);
}

fx9_atom_t
fx9ParserContextCreateBinaryOperatorPlus(fx9_parser_context_t *context)
{
    return reinterpret_cast<ParserContext *>(context)->createBinaryOperator(EOpAdd);
}

fx9_atom_t
fx9ParserContextCreateBinaryOperatorMinus(fx9_parser_context_t *context)
{
    return reinterpret_cast<ParserContext *>(context)->createBinaryOperator(EOpSub);
}

fx9_atom_t
fx9ParserContextCreateBinaryOperatorLeftShift(fx9_parser_context_t *context)
{
    return reinterpret_cast<ParserContext *>(context)->createBinaryOperator(EOpLeftShift);
}

fx9_atom_t
fx9ParserContextCreateBinaryOperatorRightShift(fx9_parser_context_t *context)
{
    return reinterpret_cast<ParserContext *>(context)->createBinaryOperator(EOpRightShift);
}

fx9_atom_t
fx9ParserContextCreateBinaryOperatorLessThan(fx9_parser_context_t *context)
{
    return reinterpret_cast<ParserContext *>(context)->createBinaryOperator(EOpLessThan);
}

fx9_atom_t
fx9ParserContextCreateBinaryOperatorGreaterThan(fx9_parser_context_t *context)
{
    return reinterpret_cast<ParserContext *>(context)->createBinaryOperator(EOpGreaterThan);
}

fx9_atom_t
fx9ParserContextCreateBinaryOperatorLessThanEqual(fx9_parser_context_t *context)
{
    return reinterpret_cast<ParserContext *>(context)->createBinaryOperator(EOpLessThanEqual);
}

fx9_atom_t
fx9ParserContextCreateBinaryOperatorGreaterThanEqual(fx9_parser_context_t *context)
{
    return reinterpret_cast<ParserContext *>(context)->createBinaryOperator(EOpGreaterThanEqual);
}

fx9_atom_t
fx9ParserContextCreateBinaryOperatorEqual(fx9_parser_context_t *context)
{
    return reinterpret_cast<ParserContext *>(context)->createBinaryOperator(EOpEqual);
}

fx9_atom_t
fx9ParserContextCreateBinaryOperatorNotEqual(fx9_parser_context_t *context)
{
    return reinterpret_cast<ParserContext *>(context)->createBinaryOperator(EOpNotEqual);
}

fx9_atom_t
fx9ParserContextCreateBinaryOperatorAnd(fx9_parser_context_t *context)
{
    return reinterpret_cast<ParserContext *>(context)->createBinaryOperator(EOpAnd);
}

fx9_atom_t
fx9ParserContextCreateBinaryOperatorXor(fx9_parser_context_t *context)
{
    return reinterpret_cast<ParserContext *>(context)->createBinaryOperator(EOpExclusiveOr);
}

fx9_atom_t
fx9ParserContextCreateBinaryOperatorOr(fx9_parser_context_t *context)
{
    return reinterpret_cast<ParserContext *>(context)->createBinaryOperator(EOpInclusiveOr);
}

fx9_atom_t
fx9ParserContextCreateBinaryOperatorLogicalAnd(fx9_parser_context_t *context)
{
    return reinterpret_cast<ParserContext *>(context)->createBinaryOperator(EOpLogicalAnd);
}

fx9_atom_t
fx9ParserContextCreateBinaryOperatorLogicalOr(fx9_parser_context_t *context)
{
    return reinterpret_cast<ParserContext *>(context)->createBinaryOperator(EOpLogicalOr);
}

fx9_atom_t
fx9ParserContextAcceptUnaryMathPreIncrement(fx9_parser_context_t *context, fx9_atom_t expr)
{
    return reinterpret_cast<ParserContext *>(context)->acceptUnaryMathPreIncrement(expr);
}

fx9_atom_t
fx9ParserContextAcceptUnaryMathPreDecrement(fx9_parser_context_t *context, fx9_atom_t expr)
{
    return reinterpret_cast<ParserContext *>(context)->acceptUnaryMathPostIncrement(expr);
}

fx9_atom_t
fx9ParserContextAcceptUnaryMathOperation(fx9_parser_context_t *context, fx9_atom_t op, fx9_atom_t expr)
{
    return reinterpret_cast<ParserContext *>(context)->acceptUnaryMathOperation(op, expr);
}

fx9_atom_t
fx9ParserContextAcceptVariable(fx9_parser_context_t *context, fx9_atom_t identifier)
{
    return reinterpret_cast<ParserContext *>(context)->acceptVariable(identifier);
}

fx9_atom_t
fx9ParserContextCreateUnaryOperatorAnd(fx9_parser_context_t *context)
{
    return reinterpret_cast<ParserContext *>(context)->createUnaryOperator(EOpAnd);
}

fx9_atom_t
fx9ParserContextCreateUnaryOperatorMul(fx9_parser_context_t *context)
{
    return reinterpret_cast<ParserContext *>(context)->createUnaryOperator(EOpMul);
}

fx9_atom_t
fx9ParserContextCreateUnaryOperatorMinus(fx9_parser_context_t *context)
{
    return reinterpret_cast<ParserContext *>(context)->createUnaryOperator(EOpNegative);
}

fx9_atom_t
fx9ParserContextCreateUnaryOperatorTilde(fx9_parser_context_t *context)
{
    return reinterpret_cast<ParserContext *>(context)->createUnaryOperator(EOpExclusiveOr);
}

fx9_atom_t
fx9ParserContextCreateUnaryOperatorExclamation(fx9_parser_context_t *context)
{
    return reinterpret_cast<ParserContext *>(context)->createUnaryOperator(EOpLogicalNot);
}

fx9_atom_t
fx9ParserContextSetTypeSpecQualifier(fx9_parser_context_t *context, fx9_atom_t qualifier, fx9_atom_t specifier)
{
    return reinterpret_cast<ParserContext *>(context)->setTypeSpecQualifier(qualifier, specifier);
}

fx9_atom_t
fx9ParserContextAppendTypeQualifier(fx9_parser_context_t *context, fx9_atom_t left, fx9_atom_t right)
{
    return reinterpret_cast<ParserContext *>(context)->appendTypeQualifier(left, right);
}

fx9_atom_t
fx9ParserContextCreateTypeQualifierStatic(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createType(
        TType(EbtVoid, EvqGlobal), reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeQualifierShared(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createType(
        TType(EbtVoid, EvqShared), reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeQualifierConst(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createType(
        TType(EbtVoid, EvqConst), reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeQualifierUniform(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createType(
        TType(EbtVoid, EvqUniform), reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeQualifierIn(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createType(
        TType(EbtVoid, EvqIn), reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeQualifierOut(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createType(
        TType(EbtVoid, EvqOut), reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeQualifierInOut(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createType(
        TType(EbtVoid, EvqInOut), reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeQualifierInline(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createType(
        TType(EbtVoid), reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecVoid(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createType(EbtVoid, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecBool(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createType(EbtBool, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecBool2(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createVectorType(
        EbtBool, 2, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecBool3(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createVectorType(
        EbtBool, 3, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecBool4(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createVectorType(
        EbtBool, 4, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecBool2x2(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createMatrixType(
        EbtBool, 2, 2, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecBool2x3(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createMatrixType(
        EbtBool, 2, 3, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecBool2x4(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createMatrixType(
        EbtBool, 2, 4, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecBool3x2(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createMatrixType(
        EbtBool, 3, 2, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecBool3x3(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createMatrixType(
        EbtBool, 3, 3, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecBool3x4(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createMatrixType(
        EbtBool, 3, 4, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecBool4x2(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createMatrixType(
        EbtBool, 4, 2, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecBool4x3(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createMatrixType(
        EbtBool, 4, 3, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecBool4x4(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createMatrixType(
        EbtBool, 4, 4, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecHalf(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createType(
        EbtFloat, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecHalf2(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createVectorType(
        EbtFloat, 2, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecHalf3(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createVectorType(
        EbtFloat, 3, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecHalf4(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createVectorType(
        EbtFloat, 4, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecHalf2x2(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createMatrixType(
        EbtFloat, 2, 2, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecHalf2x3(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createMatrixType(
        EbtFloat, 2, 3, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecHalf2x4(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createMatrixType(
        EbtFloat, 2, 4, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecHalf3x2(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createMatrixType(
        EbtFloat, 3, 2, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecHalf3x3(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createMatrixType(
        EbtFloat, 3, 3, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecHalf3x4(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createMatrixType(
        EbtFloat, 3, 4, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecHalf4x2(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createMatrixType(
        EbtFloat, 4, 2, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecHalf4x3(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createMatrixType(
        EbtFloat, 4, 3, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecHalf4x4(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createMatrixType(
        EbtFloat, 4, 4, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecInt(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createType(EbtInt, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecInt2(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createVectorType(
        EbtInt, 2, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecInt3(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createVectorType(
        EbtInt, 3, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecInt4(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createVectorType(
        EbtInt, 4, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecInt2x2(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createMatrixType(
        EbtInt, 2, 2, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecInt2x3(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createMatrixType(
        EbtInt, 2, 3, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecInt2x4(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createMatrixType(
        EbtInt, 2, 4, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecInt3x2(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createMatrixType(
        EbtInt, 3, 2, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecInt3x3(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createMatrixType(
        EbtInt, 3, 3, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecInt3x4(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createMatrixType(
        EbtInt, 3, 4, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecInt4x2(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createMatrixType(
        EbtInt, 4, 2, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecInt4x3(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createMatrixType(
        EbtInt, 4, 3, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecInt4x4(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createMatrixType(
        EbtInt, 4, 4, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecFloat(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createType(
        EbtFloat, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecFloat2(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createVectorType(
        EbtFloat, 2, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecFloat3(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createVectorType(
        EbtFloat, 3, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecFloat4(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createVectorType(
        EbtFloat, 4, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecFloat2x2(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createMatrixType(
        EbtFloat, 2, 2, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecFloat2x3(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createMatrixType(
        EbtFloat, 2, 3, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecFloat2x4(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createMatrixType(
        EbtFloat, 2, 4, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecFloat3x2(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createMatrixType(
        EbtFloat, 3, 2, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecFloat3x3(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createMatrixType(
        EbtFloat, 3, 3, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecFloat3x4(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createMatrixType(
        EbtFloat, 3, 4, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecFloat4x2(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createMatrixType(
        EbtFloat, 4, 2, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecFloat4x3(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createMatrixType(
        EbtFloat, 4, 3, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecFloat4x4(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createMatrixType(
        EbtFloat, 4, 4, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecTypename(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createType(
        EbtStruct, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecStruct(fx9_parser_context_t *context, fx9_atom_t name, fx9_atom_t body)
{
    return reinterpret_cast<ParserContext *>(context)->createStructType(name, body);
}

fx9_atom_t
fx9ParserContextAppendTypeSpecStructMember(fx9_parser_context_t *context, fx9_atom_t body, fx9_atom_t member)
{
    return reinterpret_cast<ParserContext *>(context)->appendStructMember(body, member);
}

fx9_atom_t
fx9ParserContextCreateTypeSpecStructMember(fx9_parser_context_t *context, fx9_atom_t spec, fx9_atom_t member)
{
    return reinterpret_cast<ParserContext *>(context)->createStructMember(spec, member);
}

fx9_atom_t
fx9ParserContextAppendTypeSpecStructMemberDeclarator(
    fx9_parser_context_t *context, fx9_atom_t member, fx9_atom_t identifier)
{
    return reinterpret_cast<ParserContext *>(context)->appendStructMemberDeclarator(member, identifier);
}

fx9_atom_t
fx9ParserContextCreateTypeSpecStructMemberDeclarator(
    fx9_parser_context_t *context, fx9_atom_t identifier, fx9_atom_t sizeExpr, fx9_atom_t semantic)
{
    return reinterpret_cast<ParserContext *>(context)->createStructMemberDeclarator(identifier, sizeExpr, semantic);
}

fx9_atom_t
fx9ParserContextCreateTypeSpecTextureIndetermine(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createTextureType(
        Esd2D, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecTexture2D(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createTextureType(
        Esd2D, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecTexture3D(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createTextureType(
        Esd3D, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecTextureCube(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createTextureType(
        EsdCube, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecSamplerIndetermine(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createSamplerType(
        Esd2D, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecSampler2D(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createSamplerType(
        Esd2D, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecSampler3D(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createSamplerType(
        Esd3D, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateTypeSpecSamplerCube(fx9_parser_context_t *context, fx9_atom_t token)
{
    return reinterpret_cast<ParserContext *>(context)->createSamplerType(
        EsdCube, reinterpret_cast<const LexerToken *>(token));
}

fx9_atom_t
fx9ParserContextCreateSamplerState(
    fx9_parser_context_t *context, fx9_atom_t type, fx9_atom_t identifier, fx9_atom_t index, fx9_atom_t states)
{
    return reinterpret_cast<ParserContext *>(context)->createSamplerState(type, identifier, index, states);
}

fx9_atom_t
fx9ParserContextAcceptSamplerStateValue(fx9_parser_context_t *context, fx9_atom_t states, fx9_atom_t newState)
{
    return reinterpret_cast<ParserContext *>(context)->appendSamplerStateValue(states, newState);
}

fx9_atom_t
fx9ParserContextCreateSamplerStateTexture(fx9_parser_context_t *context, fx9_atom_t identifier)
{
    return reinterpret_cast<ParserContext *>(context)->createSamplerStateTexture(identifier);
}

fx9_atom_t
fx9ParserContextCreateSamplerStatePair(fx9_parser_context_t *context, fx9_atom_t identifer, fx9_atom_t value)
{
    return reinterpret_cast<ParserContext *>(context)->createSamplerStateValue(identifer, value);
}

fx9_atom_t
fx9ParserContextAppendAnnotation(fx9_parser_context_t *context, fx9_atom_t annotations, fx9_atom_t newAnnotation)
{
    return reinterpret_cast<ParserContext *>(context)->appendAnnotation(annotations, newAnnotation);
}

fx9_atom_t
fx9ParserContextCreateAnnotation(
    fx9_parser_context_t *context, fx9_atom_t type, fx9_atom_t identifier, fx9_atom_t initializer)
{
    return reinterpret_cast<ParserContext *>(context)->createAnnotation(type, identifier, initializer);
}

fx9_atom_t
fx9ParserContextAppendStringList(fx9_parser_context_t *context, fx9_atom_t stringList, fx9_atom_t newString)
{
    return reinterpret_cast<ParserContext *>(context)->appendStringList(stringList, newString);
}

fx9_atom_t
fx9ParserContextCreateSemanticAnnotations(fx9_parser_context_t *context, fx9_atom_t semantic, fx9_atom_t annotations)
{
    return reinterpret_cast<ParserContext *>(context)->createSemanticAnnotations(semantic, annotations);
}

fx9_atom_t
fx9ParserContextCreateTechnique(
    fx9_parser_context_t *context, fx9_atom_t identifier, fx9_atom_t passes, fx9_atom_t annotations)
{
    return reinterpret_cast<ParserContext *>(context)->createTechnique(identifier, passes, annotations);
}

fx9_atom_t
fx9ParserContextAppendPass(fx9_parser_context_t *context, fx9_atom_t passes, fx9_atom_t newPass)
{
    return reinterpret_cast<ParserContext *>(context)->appendPass(passes, newPass);
}

fx9_atom_t
fx9ParserContextCreatePass(
    fx9_parser_context_t *context, fx9_atom_t identifier, fx9_atom_t states, fx9_atom_t annotations)
{
    return reinterpret_cast<ParserContext *>(context)->createPass(identifier, states, annotations);
}

fx9_atom_t
fx9ParserContextCreateStateBlock(fx9_parser_context_t *context, fx9_atom_t identifier, fx9_atom_t block)
{
    return reinterpret_cast<ParserContext *>(context)->createStateBlock(identifier, block);
}

fx9_atom_t
fx9ParserContextAppendPassState(fx9_parser_context_t *context, fx9_atom_t states, fx9_atom_t newState)
{
    return reinterpret_cast<ParserContext *>(context)->appendPassState(states, newState);
}

fx9_atom_t
fx9ParserContextCreatePassState(fx9_parser_context_t *context, fx9_atom_t identifier, fx9_atom_t value)
{
    return reinterpret_cast<ParserContext *>(context)->createPassState(identifier, value);
}

fx9_atom_t
fx9ParserContextCreatePassEntryPoint(
    fx9_parser_context_t *context, fx9_atom_t type, fx9_atom_t identifier, fx9_atom_t arguments)
{
    return reinterpret_cast<ParserContext *>(context)->createPassEntryPoint(type, identifier, arguments);
}

void
fx9ParserContextEnableAcceptingVariable(fx9_parser_context_t *context)
{
    reinterpret_cast<ParserContext *>(context)->enableAcceptingVariable();
}

void
fx9ParserContextDisableAcceptingVariable(fx9_parser_context_t *context)
{
    reinterpret_cast<ParserContext *>(context)->disableAcceptingVariable();
}

void
fx9ParserContextConcludeLocalVariableDeclaration(fx9_parser_context_t *context)
{
    reinterpret_cast<ParserContext *>(context)->concludeLocalVariableDeclaration();
}

fx9_atom_t
fx9ParserContextAcceptArraySizeExpression(fx9_parser_context_t *context, fx9_atom_t expressions, fx9_atom_t expr)
{
    return reinterpret_cast<ParserContext *>(context)->acceptArraySizeExpression(expressions, expr);
}

void
fx9ParserContextDestroyToken(fx9_lexer_token_t *token)
{
    delete reinterpret_cast<LexerToken *>(token);
}

} /* namespace fx9 */
