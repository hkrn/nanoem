/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is licensed under MIT license. for more details, see LICENSE.txt.
 */

#pragma once
#ifndef FX9_LEMON_H_
#define FX9_LEMON_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <assert.h>
#include <stdlib.h>
#if !defined(_MSC_VER) || defined(_MSC_VER) && _MSC_VER >= 1900
#include <stdint.h>
#endif

typedef uintptr_t fx9_atom_t;
typedef struct fx9_parser_context_t fx9_parser_context_t;
typedef struct fx9_lexer_token_t fx9_lexer_token_t;
typedef struct fx9_interm_node_t fx9_interm_node_t;
typedef struct fx9_interm_typed_t fx9_interm_typed_t;

void fx9ParserContextReportSyntaxError(fx9_parser_context_t *context, fx9_atom_t token);
void fx9ParserContextReportParseFailure(fx9_parser_context_t *context);
void fx9ParserContextAcceptTranslationUnit(fx9_parser_context_t *context, fx9_atom_t unit);
fx9_atom_t fx9ParserContextAcceptExternalDeclaration(
    fx9_parser_context_t *context, fx9_atom_t declarations, fx9_atom_t newDeclaration);
fx9_atom_t fx9ParserContextAcceptStatement(
    fx9_parser_context_t *context, fx9_atom_t statements, fx9_atom_t newStatement);
fx9_atom_t fx9ParserContextAcceptCompoundStatement(fx9_parser_context_t *context, fx9_atom_t statements);
fx9_atom_t fx9ParserContextAcceptJumpStatement(fx9_parser_context_t *context, fx9_atom_t statement, fx9_atom_t expr);
fx9_atom_t fx9ParserContextAcceptSelectionStatement(
    fx9_parser_context_t *context, fx9_atom_t statement, fx9_atom_t expr);
fx9_atom_t fx9ParserContextAcceptIterationStatement(
    fx9_parser_context_t *context, fx9_atom_t statement, fx9_atom_t expr);
void fx9ParserContextPushScope(fx9_parser_context_t *context);
void fx9ParserContextPopScope(fx9_parser_context_t *context);
void fx9ParserContextNestLooping(fx9_parser_context_t *context);
void fx9ParserContextUnnestLooping(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextSetGlobalVariableTypeSpec(
    fx9_parser_context_t *context, fx9_atom_t declarations, fx9_atom_t type, fx9_atom_t variables);
fx9_atom_t fx9ParserContextAcceptGlobalVariable(
    fx9_parser_context_t *context, fx9_atom_t variables, fx9_atom_t newVariable);
fx9_atom_t fx9ParserContextCreateGlobalVariable(fx9_parser_context_t *context, fx9_atom_t name, fx9_atom_t arraySize,
    fx9_atom_t initializer, fx9_atom_t semanticAnnotations);
fx9_atom_t fx9ParserContextAcceptFunction(
    fx9_parser_context_t *context, fx9_atom_t declaration, fx9_atom_t semantic, fx9_atom_t implementation);
fx9_atom_t fx9ParserContextCreateFunction(
    fx9_parser_context_t *context, fx9_atom_t spec, fx9_atom_t identifier, fx9_atom_t parameters);
fx9_atom_t fx9ParserContextAcceptFunction(
    fx9_parser_context_t *context, fx9_atom_t declaration, fx9_atom_t semantic, fx9_atom_t implementation);
fx9_atom_t fx9ParserContextAcceptFunctionParameter(
    fx9_parser_context_t *context, fx9_atom_t arguments, fx9_atom_t newArgument);
fx9_atom_t fx9ParserContextCreateFunctionParameter(fx9_parser_context_t *context, fx9_atom_t spec, fx9_atom_t name,
    fx9_atom_t arraySize, fx9_atom_t semantic, fx9_atom_t initializer);
fx9_atom_t fx9ParserContextAcceptReturnStatement(fx9_parser_context_t *context, fx9_atom_t expr);
fx9_atom_t fx9ParserContextAcceptBreakStatement(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextAcceptContinueStatement(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextAcceptDiscardStatement(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextAcceptSelectBranchStatement(
    fx9_parser_context_t *context, fx9_atom_t condExpr, fx9_atom_t trueExpr, fx9_atom_t falseExpr);
fx9_atom_t fx9ParserContextAcceptTernaryStatement(
    fx9_parser_context_t *context, fx9_atom_t condExpr, fx9_atom_t trueExpr, fx9_atom_t falseExpr);
fx9_atom_t fx9ParserContextAcceptWhileLoopStatement(
    fx9_parser_context_t *context, fx9_atom_t bodyExpr, fx9_atom_t condExpr, int first);
fx9_atom_t fx9ParserContextAcceptForLoopStatement(fx9_parser_context_t *context, fx9_atom_t initializerExpr,
    fx9_atom_t condExpr, fx9_atom_t terminatorExpr, fx9_atom_t bodyExpr);
fx9_atom_t fx9ParserContextAcceptPostIncrement(fx9_parser_context_t *context, fx9_atom_t expr);
fx9_atom_t fx9ParserContextAcceptPostDecrement(fx9_parser_context_t *context, fx9_atom_t expr);
fx9_atom_t fx9ParserContextAcceptFunctionArgument(
    fx9_parser_context_t *context, fx9_atom_t arguments, fx9_atom_t newArgument);
fx9_atom_t fx9ParserContextAcceptFunctionCall(fx9_parser_context_t *context, fx9_atom_t expr, fx9_atom_t arguments);
fx9_atom_t fx9ParserContextSetLocalVariableTypeSpec(
    fx9_parser_context_t *context, fx9_atom_t type, fx9_atom_t variables);
fx9_atom_t fx9ParserContextAcceptLocalVariable(
    fx9_parser_context_t *context, fx9_atom_t variables, fx9_atom_t newVariable);
fx9_atom_t fx9ParserContextCreateLocalVariable(
    fx9_parser_context_t *context, fx9_atom_t identifier, fx9_atom_t arraySize, fx9_atom_t initializer);
fx9_atom_t fx9ParserContextAcceptDotDeference(fx9_parser_context_t *context, fx9_atom_t expr, fx9_atom_t name);
fx9_atom_t fx9ParserContextAcceptBracketDeference(fx9_parser_context_t *context, fx9_atom_t expr, fx9_atom_t index);
fx9_atom_t fx9ParserContextAcceptTrueLiteral(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextAcceptFalseLiteral(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextAcceptIntLiteral(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextAcceptFloatLiteral(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextAcceptStringLiteral(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextAcceptIndetermineArraySizeSpecifier(fx9_parser_context_t *context, fx9_atom_t specifier);
fx9_atom_t fx9ParserContextAcceptArraySizeSpecifier(
    fx9_parser_context_t *context, fx9_atom_t specifier, fx9_atom_t token);
fx9_atom_t fx9ParserContextAcceptIdentifier(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextAppendAssignmentExpr(fx9_parser_context_t *context, fx9_atom_t left, fx9_atom_t right);
fx9_atom_t fx9ParserContextAcceptAssignmentExpr(
    fx9_parser_context_t *context, fx9_atom_t left, fx9_atom_t op, fx9_atom_t right);
fx9_atom_t fx9ParserContextAcceptInitializerList(
    fx9_parser_context_t *context, fx9_atom_t initializers, fx9_atom_t newInitializer);
fx9_atom_t fx9ParserContextAcceptBinaryOperation(
    fx9_parser_context_t *context, fx9_atom_t left, fx9_atom_t op, fx9_atom_t right);
fx9_atom_t fx9ParserContextAcceptConstructorCall(fx9_parser_context_t *context, fx9_atom_t type, fx9_atom_t expr);
fx9_atom_t fx9ParserContextAcceptCastOperation(
    fx9_parser_context_t *context, fx9_atom_t type, fx9_atom_t arraySize, fx9_atom_t expr);
fx9_atom_t fx9ParserContextGetAssignOperator(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextGetMulAssignOperator(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextGetDivAssignOperator(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextGetModAssignOperator(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextGetPlusAssignOperator(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextGetMinusAssignOperator(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextGetLeftShiftAssignOperator(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextGetRightShiftAssignOperator(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextGetAndAssignOperator(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextGetXorAssignOperator(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextGetOrAssignOperator(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextCreateBinaryOperatorStar(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextCreateBinaryOperatorSlash(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextCreateBinaryOperatorPercent(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextCreateBinaryOperatorPlus(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextCreateBinaryOperatorMinus(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextCreateBinaryOperatorLeftShift(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextCreateBinaryOperatorRightShift(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextCreateBinaryOperatorLessThan(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextCreateBinaryOperatorGreaterThan(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextCreateBinaryOperatorLessThanEqual(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextCreateBinaryOperatorGreaterThanEqual(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextCreateBinaryOperatorEqual(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextCreateBinaryOperatorNotEqual(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextCreateBinaryOperatorAnd(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextCreateBinaryOperatorXor(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextCreateBinaryOperatorOr(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextCreateBinaryOperatorLogicalAnd(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextCreateBinaryOperatorLogicalOr(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextAcceptUnaryMathPreIncrement(fx9_parser_context_t *context, fx9_atom_t expr);
fx9_atom_t fx9ParserContextAcceptUnaryMathPreDecrement(fx9_parser_context_t *context, fx9_atom_t expr);
fx9_atom_t fx9ParserContextAcceptUnaryMathOperation(fx9_parser_context_t *context, fx9_atom_t op, fx9_atom_t expr);
fx9_atom_t fx9ParserContextAcceptVariable(fx9_parser_context_t *context, fx9_atom_t identifier);
fx9_atom_t fx9ParserContextCreateUnaryOperatorAnd(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextCreateUnaryOperatorMul(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextCreateUnaryOperatorMinus(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextCreateUnaryOperatorTilde(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextCreateUnaryOperatorExclamation(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextSetTypeSpecQualifier(
    fx9_parser_context_t *context, fx9_atom_t qualifier, fx9_atom_t specifier);
fx9_atom_t fx9ParserContextAppendTypeQualifier(fx9_parser_context_t *context, fx9_atom_t left, fx9_atom_t right);
fx9_atom_t fx9ParserContextCreateTypeQualifierStatic(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeQualifierShared(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeQualifierConst(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeQualifierUniform(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeQualifierIn(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeQualifierOut(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeQualifierInOut(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeQualifierInline(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecVoid(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecBool(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecBool2(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecBool3(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecBool4(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecBool2x2(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecBool2x3(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecBool2x4(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecBool3x2(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecBool3x3(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecBool3x4(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecBool4x2(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecBool4x3(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecBool4x4(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecHalf(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecHalf2(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecHalf3(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecHalf4(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecHalf2x2(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecHalf2x3(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecHalf2x4(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecHalf3x2(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecHalf3x3(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecHalf3x4(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecHalf4x2(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecHalf4x3(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecHalf4x4(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecInt(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecInt2(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecInt3(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecInt4(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecInt2x2(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecInt2x3(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecInt2x4(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecInt3x2(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecInt3x3(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecInt3x4(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecInt4x2(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecInt4x3(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecInt4x4(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecFloat(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecFloat2(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecFloat3(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecFloat4(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecFloat2x2(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecFloat2x3(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecFloat2x4(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecFloat3x2(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecFloat3x3(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecFloat3x4(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecFloat4x2(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecFloat4x3(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecFloat4x4(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecTypename(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecStruct(fx9_parser_context_t *context, fx9_atom_t name, fx9_atom_t body);
fx9_atom_t fx9ParserContextAppendTypeSpecStructMember(
    fx9_parser_context_t *context, fx9_atom_t body, fx9_atom_t member);
fx9_atom_t fx9ParserContextCreateTypeSpecStructMember(
    fx9_parser_context_t *context, fx9_atom_t spec, fx9_atom_t member);
fx9_atom_t fx9ParserContextAppendTypeSpecStructMemberDeclarator(
    fx9_parser_context_t *context, fx9_atom_t member, fx9_atom_t identifier);
fx9_atom_t fx9ParserContextCreateTypeSpecStructMemberDeclarator(
    fx9_parser_context_t *context, fx9_atom_t identifier, fx9_atom_t sizeExpr, fx9_atom_t semantic);
fx9_atom_t fx9ParserContextCreateTypeSpecTextureIndetermine(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecTexture2D(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecTexture3D(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecTextureCube(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecSamplerIndetermine(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecSampler2D(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecSampler3D(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateTypeSpecSamplerCube(fx9_parser_context_t *context, fx9_atom_t token);
fx9_atom_t fx9ParserContextCreateSamplerState(
    fx9_parser_context_t *context, fx9_atom_t type, fx9_atom_t identifier, fx9_atom_t index, fx9_atom_t states);
fx9_atom_t fx9ParserContextAcceptSamplerStateValue(
    fx9_parser_context_t *context, fx9_atom_t states, fx9_atom_t newState);
fx9_atom_t fx9ParserContextCreateSamplerStateTexture(fx9_parser_context_t *context, fx9_atom_t identifier);
fx9_atom_t fx9ParserContextCreateSamplerStatePair(
    fx9_parser_context_t *context, fx9_atom_t identifer, fx9_atom_t value);
fx9_atom_t fx9ParserContextAppendAnnotation(
    fx9_parser_context_t *context, fx9_atom_t annotations, fx9_atom_t newAnnotation);
fx9_atom_t fx9ParserContextCreateAnnotation(
    fx9_parser_context_t *context, fx9_atom_t type, fx9_atom_t identifier, fx9_atom_t initializer);
fx9_atom_t fx9ParserContextAppendStringList(fx9_parser_context_t *context, fx9_atom_t stringList, fx9_atom_t newString);
fx9_atom_t fx9ParserContextCreateSemanticAnnotations(
    fx9_parser_context_t *context, fx9_atom_t semantic, fx9_atom_t annotations);
fx9_atom_t fx9ParserContextCreateTechnique(
    fx9_parser_context_t *context, fx9_atom_t identifier, fx9_atom_t passes, fx9_atom_t annotations);
fx9_atom_t fx9ParserContextAppendPass(fx9_parser_context_t *context, fx9_atom_t passes, fx9_atom_t newPass);
fx9_atom_t fx9ParserContextCreatePass(
    fx9_parser_context_t *context, fx9_atom_t identifier, fx9_atom_t states, fx9_atom_t annotations);
fx9_atom_t fx9ParserContextCreateStateBlock(fx9_parser_context_t *context, fx9_atom_t identifier, fx9_atom_t block);
fx9_atom_t fx9ParserContextAppendPassState(fx9_parser_context_t *context, fx9_atom_t states, fx9_atom_t newState);
fx9_atom_t fx9ParserContextCreatePassState(fx9_parser_context_t *context, fx9_atom_t identifier, fx9_atom_t value);
fx9_atom_t fx9ParserContextCreatePassEntryPoint(
    fx9_parser_context_t *context, fx9_atom_t type, fx9_atom_t identifier, fx9_atom_t arguments);
void fx9ParserContextEnableAcceptingVariable(fx9_parser_context_t *context);
void fx9ParserContextDisableAcceptingVariable(fx9_parser_context_t *context);
void fx9ParserContextConcludeLocalVariableDeclaration(fx9_parser_context_t *context);
fx9_atom_t fx9ParserContextAcceptAttribute(
    fx9_parser_context_t *context, fx9_atom_t attrs, fx9_atom_t name, fx9_atom_t value);
fx9_atom_t fx9ParserContextSetAllAttributes(fx9_parser_context_t *context, fx9_atom_t statement, fx9_atom_t attrs);
fx9_atom_t fx9ParserContextFlattenExpression(fx9_parser_context_t *context, fx9_atom_t expression);
fx9_atom_t fx9ParserContextAcceptArraySizeExpression(
    fx9_parser_context_t *context, fx9_atom_t expressions, fx9_atom_t expr);
void fx9ParserContextDestroyToken(fx9_lexer_token_t *token);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* FX9_LEMON_H_ */
