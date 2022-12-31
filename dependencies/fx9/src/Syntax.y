/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is licensed under MIT license. for more details, see LICENSE.txt.
 */

%name fx9LemonParserContext
%token_prefix TOKEN_

%include {
#include "fx9/Lemon.h"
}
%syntax_error {
    fx9ParserContextReportSyntaxError(parser, TOKEN);
}
%parse_failure {
    fx9ParserContextReportParseFailure(parser);
}
%stack_overflow {
}
%extra_argument { fx9_parser_context_t *parser }
%token_type { fx9_atom_t }
%token_destructor { fx9ParserContextDestroyToken((fx9_lexer_token_t *) $$); }

%left COMMA.
%right ASSIGN
       ADDASSIGN
       SUBASSIGN
       MULASSIGN
       DIVASSIGN
       MODASSIGN
       LSHIFTASSIGN
       RSHIFTASSIGN
       ANDASSIGN
       ORASSIGN
       XORASSIGN.
%right QUESTION.
%left OROR.
%left ANDAND.
%left OR.
%left XOR.
%left AND.
%left EQ
      NE.
%left LT
      LE
      GT
      GE.
%left LSHIFT
      RSHIFT.
%left PLUS
      MINUS.
%left STAR
      SLASH
      PERCENT.
%right EXCLAMATION
       COMPLEMENT
       MINUSMINUS
       PLUSPLUS.
%left DOT
      LBRACKET
      RBRACKET
      LPAREN
      RPAREN.
%right ELSE.

translation_unit ::= external_decl_list(A). {
    fx9ParserContextAcceptTranslationUnit(parser, A);
}

/* external declaration */
external_decl_list(A) ::= external_decl_list(B) external_decl(C). {
    A = fx9ParserContextAcceptExternalDeclaration(parser, B, C);
}
external_decl_list(A) ::= external_decl(B). {
    A = fx9ParserContextAcceptExternalDeclaration(parser, 0, B);
}
external_decl_list(A) ::= external_decl_list(B) fully_specified_type(C) glocal_variable_declaration_list(D) SEMICOLON. {
    A = fx9ParserContextSetGlobalVariableTypeSpec(parser, B, C, D);
}
external_decl_list(A) ::= fully_specified_type(B) glocal_variable_declaration_list(C) SEMICOLON. {
    A = fx9ParserContextSetGlobalVariableTypeSpec(parser, 0, B, C);
}
external_decl(A) ::= sampler_decl(B) SEMICOLON. {
    A = B;
}
/* typedef and struct */
external_decl(A) ::= fully_specified_type SEMICOLON. {
    A = 0;
}
external_decl(A) ::= technique_decl. {
    A = 0;
}
external_decl(A) ::= SEMICOLON. {
    A = 0;
}

/* external variable declaration and initialization */
glocal_variable_declaration_list(A) ::= glocal_variable_declaration_list(B) COMMA glocal_variable_declaration(C). {
    A = fx9ParserContextAcceptGlobalVariable(parser, B, C);
}
glocal_variable_declaration_list(A) ::= glocal_variable_declaration(B). {
    A = fx9ParserContextAcceptGlobalVariable(parser, 0, B);
}
glocal_variable_declaration(A) ::= identifier(B) array_size_specifier(C) semantic_or_annotations(D) ASSIGN initializer_expression(E). {
    A = fx9ParserContextCreateGlobalVariable(parser, B, C, E, D);
}
glocal_variable_declaration(A) ::= identifier(B) semantic_or_annotations(C) ASSIGN initializer_expression(D). {
    A = fx9ParserContextCreateGlobalVariable(parser, B, 0, D, C);
}
glocal_variable_declaration(A) ::= identifier(B) array_size_specifier(C) ASSIGN initializer_expression(D). {
    A = fx9ParserContextCreateGlobalVariable(parser, B, C, D, 0);
}
/* accept register but ignore */
glocal_variable_declaration(A) ::= identifier(B) COLON register_decl ASSIGN initializer_expression(C). {
    A = fx9ParserContextCreateGlobalVariable(parser, B, 0, C, 0);
}
glocal_variable_declaration(A) ::= identifier(B) ASSIGN initializer_expression(C). {
    A = fx9ParserContextCreateGlobalVariable(parser, B, 0, C, 0);
}
glocal_variable_declaration(A) ::= identifier(B) array_size_specifier(C) semantic_or_annotations(D). {
    A = fx9ParserContextCreateGlobalVariable(parser, B, C, 0, D);
}
glocal_variable_declaration(A) ::= identifier(B) semantic_or_annotations(C). {
    A = fx9ParserContextCreateGlobalVariable(parser, B, 0, 0, C);
}
glocal_variable_declaration(A) ::= identifier(B) array_size_specifier(C). {
    A = fx9ParserContextCreateGlobalVariable(parser, B, C, 0, 0);
}
glocal_variable_declaration(A) ::= identifier(B). {
    A = fx9ParserContextCreateGlobalVariable(parser, B, 0, 0, 0);
}

/* function declaration */
external_decl(A) ::= function_decl(B). {
    A = B;
}
function_decl(A) ::= function_prototype_decl(B) semantic(C) compound_statement(D). {
    A = fx9ParserContextAcceptFunction(parser, B, C, D);
}
function_decl(A) ::= function_prototype_decl(B) compound_statement(C). {
    A = fx9ParserContextAcceptFunction(parser, B, 0, C);
}
function_decl(A) ::= function_prototype_decl(B) semantic(C). {
    A = fx9ParserContextAcceptFunction(parser, B, C, 0);
}
function_decl(A) ::= function_prototype_decl(B). {
    A = fx9ParserContextAcceptFunction(parser, B, 0, 0);
}
function_prototype_decl(A) ::= fully_specified_type(B) identifier(C) function_parameters(D). {
    A = fx9ParserContextCreateFunction(parser, B, C, D);
}

fully_specified_type(A) ::= type_qualifier_list(B) type_specifier(C). {
    A = fx9ParserContextSetTypeSpecQualifier(parser, B, C);
}
fully_specified_type(A) ::= type_specifier(B). {
    A = fx9ParserContextSetTypeSpecQualifier(parser, 0, B);
}

type_qualifier_list(A) ::= type_qualifier_list(B) type_qualifier(C). {
    A = fx9ParserContextAppendTypeQualifier(parser, B, C);
}
type_qualifier_list(A) ::= type_qualifier(B). {
    A = fx9ParserContextAppendTypeQualifier(parser, 0, B);
}

type_qualifier(A) ::= STATIC(B). {
    A = fx9ParserContextCreateTypeQualifierStatic(parser, B);
}
type_qualifier(A) ::= SHARED(B). {
    A = fx9ParserContextCreateTypeQualifierShared(parser, B);
}
type_qualifier(A) ::= CONST(B). {
    A = fx9ParserContextCreateTypeQualifierConst(parser, B);
}
type_qualifier(A) ::= UNIFORM(B). {
    A = fx9ParserContextCreateTypeQualifierUniform(parser, B);
}
type_qualifier(A) ::= IN(B). {
    A = fx9ParserContextCreateTypeQualifierIn(parser, B);
}
type_qualifier(A) ::= OUT(B). {
    A = fx9ParserContextCreateTypeQualifierOut(parser, B);
}
type_qualifier(A) ::= INOUT(B). {
    A = fx9ParserContextCreateTypeQualifierInOut(parser, B);
}
type_qualifier(A) ::= INLINE(B). {
    A = fx9ParserContextCreateTypeQualifierInline(parser, B);
}

type_specifier(A) ::= VOID(B). {
    A = fx9ParserContextCreateTypeSpecVoid(parser, B);
}
type_specifier(A) ::= BOOL(B). {
    A = fx9ParserContextCreateTypeSpecBool(parser, B);
}
type_specifier(A) ::= BOOL2(B). {
    A = fx9ParserContextCreateTypeSpecBool2(parser, B);
}
type_specifier(A) ::= BOOL3(B). {
    A = fx9ParserContextCreateTypeSpecBool3(parser, B);
}
type_specifier(A) ::= BOOL4(B). {
    A = fx9ParserContextCreateTypeSpecBool4(parser, B);
}
type_specifier(A) ::= BOOL2x2(B). {
    A = fx9ParserContextCreateTypeSpecBool2x2(parser, B);
}
type_specifier(A) ::= BOOL2x3(B). {
    A = fx9ParserContextCreateTypeSpecBool2x3(parser, B);
}
type_specifier(A) ::= BOOL2x4(B). {
    A = fx9ParserContextCreateTypeSpecBool2x4(parser, B);
}
type_specifier(A) ::= BOOL3x2(B). {
    A = fx9ParserContextCreateTypeSpecBool3x2(parser, B);
}
type_specifier(A) ::= BOOL3x3(B). {
    A = fx9ParserContextCreateTypeSpecBool3x3(parser, B);
}
type_specifier(A) ::= BOOL3x4(B). {
    A = fx9ParserContextCreateTypeSpecBool3x4(parser, B);
}
type_specifier(A) ::= BOOL4x2(B). {
    A = fx9ParserContextCreateTypeSpecBool4x2(parser, B);
}
type_specifier(A) ::= BOOL4x3(B). {
    A = fx9ParserContextCreateTypeSpecBool4x3(parser, B);
}
type_specifier(A) ::= BOOL4x4(B). {
    A = fx9ParserContextCreateTypeSpecBool4x4(parser, B);
}
type_specifier(A) ::= HALF(B). {
    A = fx9ParserContextCreateTypeSpecHalf(parser, B);
}
type_specifier(A) ::= HALF2(B). {
    A = fx9ParserContextCreateTypeSpecHalf2(parser, B);
}
type_specifier(A) ::= HALF3(B). {
    A = fx9ParserContextCreateTypeSpecHalf3(parser, B);
}
type_specifier(A) ::= HALF4(B). {
    A = fx9ParserContextCreateTypeSpecHalf4(parser, B);
}
type_specifier(A) ::= HALF2x2(B). {
    A = fx9ParserContextCreateTypeSpecHalf2x2(parser, B);
}
type_specifier(A) ::= HALF2x3(B). {
    A = fx9ParserContextCreateTypeSpecHalf2x3(parser, B);
}
type_specifier(A) ::= HALF2x4(B). {
    A = fx9ParserContextCreateTypeSpecHalf2x4(parser, B);
}
type_specifier(A) ::= HALF3x2(B). {
    A = fx9ParserContextCreateTypeSpecHalf3x2(parser, B);
}
type_specifier(A) ::= HALF3x3(B). {
    A = fx9ParserContextCreateTypeSpecHalf3x3(parser, B);
}
type_specifier(A) ::= HALF3x4(B). {
    A = fx9ParserContextCreateTypeSpecHalf3x4(parser, B);
}
type_specifier(A) ::= HALF4x2(B). {
    A = fx9ParserContextCreateTypeSpecHalf4x2(parser, B);
}
type_specifier(A) ::= HALF4x3(B). {
    A = fx9ParserContextCreateTypeSpecHalf4x3(parser, B);
}
type_specifier(A) ::= HALF4x4(B). {
    A = fx9ParserContextCreateTypeSpecHalf4x4(parser, B);
}
type_specifier(A) ::= INT(B). {
    A = fx9ParserContextCreateTypeSpecInt(parser, B);
}
type_specifier(A) ::= INT2(B). {
    A = fx9ParserContextCreateTypeSpecInt2(parser, B);
}
type_specifier(A) ::= INT3(B). {
    A = fx9ParserContextCreateTypeSpecInt3(parser, B);
}
type_specifier(A) ::= INT4(B). {
    A = fx9ParserContextCreateTypeSpecInt4(parser, B);
}
type_specifier(A) ::= INT2x2(B). {
    A = fx9ParserContextCreateTypeSpecInt2x2(parser, B);
}
type_specifier(A) ::= INT2x3(B). {
    A = fx9ParserContextCreateTypeSpecInt2x3(parser, B);
}
type_specifier(A) ::= INT2x4(B). {
    A = fx9ParserContextCreateTypeSpecInt2x4(parser, B);
}
type_specifier(A) ::= INT3x2(B). {
    A = fx9ParserContextCreateTypeSpecInt3x2(parser, B);
}
type_specifier(A) ::= INT3x3(B). {
    A = fx9ParserContextCreateTypeSpecInt3x3(parser, B);
}
type_specifier(A) ::= INT3x4(B). {
    A = fx9ParserContextCreateTypeSpecInt3x4(parser, B);
}
type_specifier(A) ::= INT4x2(B). {
    A = fx9ParserContextCreateTypeSpecInt4x2(parser, B);
}
type_specifier(A) ::= INT4x3(B). {
    A = fx9ParserContextCreateTypeSpecInt4x3(parser, B);
}
type_specifier(A) ::= INT4x4(B). {
    A = fx9ParserContextCreateTypeSpecInt4x4(parser, B);
}
type_specifier(A) ::= FLOAT(B). {
    A = fx9ParserContextCreateTypeSpecFloat(parser, B);
}
type_specifier(A) ::= FLOAT2(B). {
    A = fx9ParserContextCreateTypeSpecFloat2(parser, B);
}
type_specifier(A) ::= FLOAT3(B). {
    A = fx9ParserContextCreateTypeSpecFloat3(parser, B);
}
type_specifier(A) ::= FLOAT4(B). {
    A = fx9ParserContextCreateTypeSpecFloat4(parser, B);
}
type_specifier(A) ::= FLOAT2x2(B). {
    A = fx9ParserContextCreateTypeSpecFloat2x2(parser, B);
}
type_specifier(A) ::= FLOAT2x3(B). {
    A = fx9ParserContextCreateTypeSpecFloat2x3(parser, B);
}
type_specifier(A) ::= FLOAT2x4(B). {
    A = fx9ParserContextCreateTypeSpecFloat2x4(parser, B);
}
type_specifier(A) ::= FLOAT3x2(B). {
    A = fx9ParserContextCreateTypeSpecFloat3x2(parser, B);
}
type_specifier(A) ::= FLOAT3x3(B). {
    A = fx9ParserContextCreateTypeSpecFloat3x3(parser, B);
}
type_specifier(A) ::= FLOAT3x4(B). {
    A = fx9ParserContextCreateTypeSpecFloat3x4(parser, B);
}
type_specifier(A) ::= FLOAT4x2(B). {
    A = fx9ParserContextCreateTypeSpecFloat4x2(parser, B);
}
type_specifier(A) ::= FLOAT4x3(B). {
    A = fx9ParserContextCreateTypeSpecFloat4x3(parser, B);
}
type_specifier(A) ::= FLOAT4x4(B). {
    A = fx9ParserContextCreateTypeSpecFloat4x4(parser, B);
}
type_specifier(A) ::= STRUCT identifier(B) LBRACE struct_declaration_list(C) RBRACE. {
    A = fx9ParserContextCreateTypeSpecStruct(parser, B, C);
}
type_specifier(A) ::= STRUCT LBRACE struct_declaration_list(B) RBRACE. {
    A = fx9ParserContextCreateTypeSpecStruct(parser, 0, B);
}
type_specifier(A) ::= TYPE_NAME(B). {
    A = fx9ParserContextCreateTypeSpecTypename(parser, B);
}
type_specifier(A) ::= sampler_type_specifier(B). {
    A = B;
}
type_specifier(A) ::= texture_type_specifier(B). {
    A = B;
}

struct_declaration_list(A) ::= struct_declaration_list(B) struct_member_declaration(C). {
    A = fx9ParserContextAppendTypeSpecStructMember(parser, B, C);
}
struct_declaration_list(A) ::= struct_member_declaration(B). {
    A = fx9ParserContextAppendTypeSpecStructMember(parser, 0, B);
}

struct_member_declaration(A) ::= fully_specified_type(B) struct_member_declarator_list(C) SEMICOLON. {
    A = fx9ParserContextCreateTypeSpecStructMember(parser, B, C);
}

struct_member_declarator_list(A) ::= struct_member_declarator_list(B) COMMA struct_member_declarator(C). {
    A = fx9ParserContextAppendTypeSpecStructMemberDeclarator(parser, B, C);
}
struct_member_declarator_list(A) ::= struct_member_declarator(B). {
    A = fx9ParserContextAppendTypeSpecStructMemberDeclarator(parser, 0, B);
}

struct_member_declarator(A) ::= identifier(B) array_size_specifier(C) semantic_or_annotations(D). {
    A = fx9ParserContextCreateTypeSpecStructMemberDeclarator(parser, B, C, D);
 }
struct_member_declarator(A) ::= identifier(B) semantic_or_annotations(C). {
    A = fx9ParserContextCreateTypeSpecStructMemberDeclarator(parser, B, 0, C);
}
struct_member_declarator(A) ::= identifier(B). {
    A = fx9ParserContextCreateTypeSpecStructMemberDeclarator(parser, B, 0, 0);
}

function_parameters(A) ::= LPAREN parameter_declaration_list(B) RPAREN. {
    A = B;
}
function_parameters(A) ::= LPAREN RPAREN. {
    A = 0;
}

parameter_declaration_list(A) ::= parameter_declaration_list(B) COMMA parameter_declaration(C). {
    A = fx9ParserContextAcceptFunctionParameter(parser, B, C);
}
parameter_declaration_list(A) ::= parameter_declaration(B). {
    A = fx9ParserContextAcceptFunctionParameter(parser, 0, B);
}

parameter_declaration(A) ::= fully_specified_type(B) identifier(C) array_size_specifier(D) ASSIGN initializer_expression(E). {
    A = fx9ParserContextCreateFunctionParameter(parser, B, C, D, 0, E);
}
parameter_declaration(A) ::= fully_specified_type(B) identifier(C) ASSIGN initializer_expression(D). {
    A = fx9ParserContextCreateFunctionParameter(parser, B, C, 0, 0, D);
}
parameter_declaration(A) ::= fully_specified_type(B) identifier(C) array_size_specifier(D) semantic(E). {
    A = fx9ParserContextCreateFunctionParameter(parser, B, C, D, E, 0);
}
parameter_declaration(A) ::= fully_specified_type(B) identifier(C) array_size_specifier(D). {
    A = fx9ParserContextCreateFunctionParameter(parser, B, C, D, 0, 0);
}
parameter_declaration(A) ::= fully_specified_type(B) identifier(C) semantic(D). {
    A = fx9ParserContextCreateFunctionParameter(parser, B, C, 0, D, 0);
}
parameter_declaration(A) ::= fully_specified_type(B) identifier(C). {
    A = fx9ParserContextCreateFunctionParameter(parser, B, C, 0, 0, 0);
}
parameter_declaration(A) ::= fully_specified_type(B). {
    A = fx9ParserContextCreateFunctionParameter(parser, B, 0, 0, 0, 0);
}

array_size_specifier(A) ::= array_size_specifier(B) LBRACKET array_size_expression_list(C) RBRACKET. {
    A = fx9ParserContextAcceptArraySizeSpecifier(parser, B, C);
}
array_size_specifier(A) ::= array_size_specifier(B) LBRACKET RBRACKET. {
    A = fx9ParserContextAcceptIndetermineArraySizeSpecifier(parser, B);
}
array_size_specifier(A) ::= LBRACKET array_size_expression_list(B) RBRACKET. {
    A = fx9ParserContextAcceptArraySizeSpecifier(parser, 0, B);
}
array_size_specifier(A) ::= LBRACKET RBRACKET. {
    A = fx9ParserContextAcceptIndetermineArraySizeSpecifier(parser, 0);
}
array_size_expression_list(A) ::= array_size_expression_list(B) array_size_expression(C). {
    A = fx9ParserContextAcceptArraySizeExpression(parser, B, C);
}
array_size_expression_list(A) ::= array_size_expression(B). {
    A = fx9ParserContextAcceptArraySizeExpression(parser, 0, B);
}
array_size_expression(A) ::= array_size_expression(B) PLUS array_size_expression(C). {
    A = fx9ParserContextAcceptBinaryOperation(parser, B, fx9ParserContextCreateBinaryOperatorPlus(parser), C);
}
array_size_expression(A) ::= array_size_expression(B) MINUS array_size_expression(C). {
    A = fx9ParserContextAcceptBinaryOperation(parser, B, fx9ParserContextCreateBinaryOperatorMinus(parser), C);
}
array_size_expression(A) ::= array_size_expression(B) STAR array_size_expression(C). {
    A = fx9ParserContextAcceptBinaryOperation(parser, B, fx9ParserContextCreateBinaryOperatorStar(parser), C);
}
array_size_expression(A) ::= array_size_expression(B) SLASH array_size_expression(C). {
    A = fx9ParserContextAcceptBinaryOperation(parser, B, fx9ParserContextCreateBinaryOperatorSlash(parser), C);
}
array_size_expression(A) ::= int_literal(B). {
    A = B;
}

annotation_decl(A) ::= LT annotation_list(B) SEMICOLON GT. {
    A = B;
}
annotation_decl(A) ::= LT GT. {
    A = 0;
}
annotation_list(A) ::= annotation_list(B) SEMICOLON annotation(C). {
    A = fx9ParserContextAppendAnnotation(parser, B, C);
}
annotation_list(A) ::= annotation(B). {
    A = fx9ParserContextAppendAnnotation(parser, 0, B);
}
annotation(A) ::= type_specifier(B) identifier(C) ASSIGN initializer_expression(D). {
    A = fx9ParserContextCreateAnnotation(parser, B, C, D);
}
annotation(A) ::= STRING identifier(B) ASSIGN string_list(C). {
    A = fx9ParserContextCreateAnnotation(parser, 0, B, C);
}
string_list(A) ::= string_list(B) string(C). {
    A = fx9ParserContextAppendStringList(parser, B, C);
}
string_list(A) ::= string(B). {
    A = fx9ParserContextAppendStringList(parser, 0, B);
}

semantic_or_annotations(A) ::= COLON identifier(B) annotation_decl(C). {
    A = fx9ParserContextCreateSemanticAnnotations(parser, B, C);
}
semantic_or_annotations(A) ::= COLON identifier(B). {
    A = fx9ParserContextCreateSemanticAnnotations(parser, B, 0);
}
semantic_or_annotations(A) ::= annotation_decl(B). {
    A = fx9ParserContextCreateSemanticAnnotations(parser, 0, B);
}
semantic(A) ::= COLON identifier(B). {
    A = fx9ParserContextCreateSemanticAnnotations(parser, B, 0);
}

block_list(A) ::= block_list(B) block(C). {
    A = fx9ParserContextAcceptStatement(parser, B, C);
}
block_list(A) ::= block(B). {
    A = fx9ParserContextAcceptStatement(parser, 0, B);
}

block(A) ::= statement(B). {
    A = B;
}
block(A) ::= decl(B). {
    A = B;
}
decl(A) ::= fully_specified_type local_variable_declaration_list(B) SEMICOLON. {
    fx9ParserContextConcludeLocalVariableDeclaration(parser);
    A = B;
}
local_variable_declaration_list(A) ::= local_variable_declaration_list(B) COMMA local_variable_declaration(C). {
    A = fx9ParserContextAcceptLocalVariable(parser, B, C);
}
local_variable_declaration_list(A) ::= local_variable_declaration(B). {
    A = fx9ParserContextAcceptLocalVariable(parser, 0, B);
}
local_variable_declaration(A) ::= identifier(B) array_size_specifier(C) ASSIGN initializer_expression(D). {
    A = fx9ParserContextCreateLocalVariable(parser, B, C, D);
}
local_variable_declaration(A) ::= identifier(B) array_size_specifier(C). {
    A = fx9ParserContextCreateLocalVariable(parser, B, C, 0);
}
local_variable_declaration(A) ::= identifier(B) ASSIGN initializer_expression(C). {
    A = fx9ParserContextCreateLocalVariable(parser, B, 0, C);
}
local_variable_declaration(A) ::= identifier(B). {
    A = fx9ParserContextCreateLocalVariable(parser, B, 0, 0);
}

statement(A) ::= expression(B) SEMICOLON. {
    A = B;
}
statement(A) ::= SEMICOLON. {
    A = 0;
}
statement(A) ::= compound_statement(B). {
    A = fx9ParserContextAcceptCompoundStatement(parser, B);
}
statement(A) ::= jump_statement(B). {
    A = B;
}
statement(A) ::= selection_statement(B). {
    A = B;
}
statement(A) ::= iteration_statement(B). {
    A = B;
}

compound_statement(A) ::= compound_statement_left block_list(B) compound_statement_right. {
    A = B;
}
compound_statement(A) ::= compound_statement_left compound_statement_right. {
    A = 0;
}
compound_statement_left ::= LBRACE. {
    fx9ParserContextPushScope(parser);
}
compound_statement_right ::= RBRACE. {
    fx9ParserContextPopScope(parser);
}

scoped_statement(A) ::= statement(B). {
    A = fx9ParserContextAcceptCompoundStatement(parser, B);
}

jump_statement(A) ::= RETURN expression(B) SEMICOLON. {
    A = fx9ParserContextAcceptReturnStatement(parser, B);
}
jump_statement(A) ::= RETURN SEMICOLON. {
    A = fx9ParserContextAcceptReturnStatement(parser, 0);
}
jump_statement(A) ::= BREAK. {
    A = fx9ParserContextAcceptBreakStatement(parser);
}
jump_statement(A) ::= CONTINUE. {
    A = fx9ParserContextAcceptContinueStatement(parser);
}
jump_statement(A) ::= DISCARD. {
    A = fx9ParserContextAcceptDiscardStatement(parser);
}

selection_statement(A) ::= attributes_decl(B) if_statement(C). {
    A = fx9ParserContextSetAllAttributes(parser, C, B);
}
selection_statement(A) ::= if_statement(B). {
    A = B;
}

if_statement(A) ::= IF LPAREN expression(B) RPAREN scoped_statement(C) ELSE scoped_statement(D). {
    A = fx9ParserContextAcceptSelectBranchStatement(parser, B, C, D);
}
if_statement(A) ::= IF LPAREN expression(B) RPAREN scoped_statement(C). {
    A = fx9ParserContextAcceptSelectBranchStatement(parser, B, C, 0);
}

iteration_statement(A) ::= attributes_decl(B) for_statement(C). {
    A = fx9ParserContextSetAllAttributes(parser, C, B);
}
iteration_statement(A) ::= for_statement(B). {
    A = B;
}
iteration_statement(A) ::= WHILE LPAREN expression(B) RPAREN nested_compound_statement(C). {
    A = fx9ParserContextAcceptWhileLoopStatement(parser, B, C, 1);
}
iteration_statement(A) ::= DO nested_compound_statement(B) WHILE LPAREN expression(C) RPAREN SEMICOLON. {
    A = fx9ParserContextAcceptWhileLoopStatement(parser, C, B, 0);
}

for_statement(A) ::= for_initializer(B) expression(C) SEMICOLON expression(D) RPAREN for_end_scope(E). {
    A = fx9ParserContextAcceptForLoopStatement(parser, B, C, D, E);
}
for_statement(A) ::= for_initializer(B) expression(C) SEMICOLON RPAREN for_end_scope(D). {
    A = fx9ParserContextAcceptForLoopStatement(parser, B, C, 0, D);
}
for_statement(A) ::= for_initializer(B) SEMICOLON expression(C) RPAREN for_end_scope(D). {
    A = fx9ParserContextAcceptForLoopStatement(parser, B, 0, C, D);
}
for_statement(A) ::= for_initializer(B) SEMICOLON RPAREN for_end_scope(C). {
    A = fx9ParserContextAcceptForLoopStatement(parser, B, 0, 0, C);
}
for_initializer(A) ::= for_begin_scope expression(B) SEMICOLON. {
    A = B;
}
for_initializer(A) ::= for_begin_scope decl(B). {
    A = B;
}
for_initializer(A) ::= for_begin_scope SEMICOLON. {
    A = 0;
}
for_begin_scope(A) ::= FOR LPAREN .{
    fx9ParserContextPushScope(parser);
    fx9ParserContextNestLooping(parser);
    A = 0;
}
for_end_scope(A) ::= for_compound_statement(B). {
    fx9ParserContextPopScope(parser);
    fx9ParserContextUnnestLooping(parser);
    A = B;
}
for_compound_statement(A) ::= LBRACE block_list(B) RBRACE. {
    A = B;
}
for_compound_statement(A) ::= jump_statement(B). {
    A = B;
}
for_compound_statement(A) ::= selection_statement(B). {
    A = B;
}
for_compound_statement(A) ::= iteration_statement(B). {
    A = B;
}
for_compound_statement(A) ::= expression(B) SEMICOLON. {
    A = B;
}
for_compound_statement(A) ::= SEMICOLON. {
    A = 0;
}

nested_compound_statement(A) ::= nested_compound_statement_left block_list(B) nested_compound_statement_right. {
    A = B;
}
nested_compound_statement(A) ::= nested_compound_statement_left nested_compound_statement_right. {
    A = 0;
}
nested_compound_statement_left ::= LBRACE. {
    fx9ParserContextPushScope(parser);
    fx9ParserContextNestLooping(parser);
}
nested_compound_statement_right ::= RBRACE. {
    fx9ParserContextUnnestLooping(parser);
    fx9ParserContextPopScope(parser);
}

expression(A) ::= expression(B) COMMA assignment_expression(C). {
    A = fx9ParserContextAppendAssignmentExpr(parser, B, C);
}
expression(A) ::= assignment_expression(B). {
    A = fx9ParserContextAppendAssignmentExpr(parser, 0, B);
}

assignment_expression(A) ::= unary_expression(B) ASSIGN assignment_expression(C). {
    A = fx9ParserContextAcceptAssignmentExpr(parser, B, fx9ParserContextGetAssignOperator(parser), C);
}
assignment_expression(A) ::= unary_expression(B) MULASSIGN assignment_expression(C). {
    A = fx9ParserContextAcceptAssignmentExpr(parser, B, fx9ParserContextGetMulAssignOperator(parser), C);
}
assignment_expression(A) ::= unary_expression(B) DIVASSIGN assignment_expression(C). {
    A = fx9ParserContextAcceptAssignmentExpr(parser, B, fx9ParserContextGetDivAssignOperator(parser), C);
}
assignment_expression(A) ::= unary_expression(B) MODASSIGN assignment_expression(C). {
    A = fx9ParserContextAcceptAssignmentExpr(parser, B, fx9ParserContextGetModAssignOperator(parser), C);
}
assignment_expression(A) ::= unary_expression(B) ADDASSIGN assignment_expression(C). {
    A = fx9ParserContextAcceptAssignmentExpr(parser, B, fx9ParserContextGetPlusAssignOperator(parser), C);
}
assignment_expression(A) ::= unary_expression(B) SUBASSIGN assignment_expression(C). {
    A = fx9ParserContextAcceptAssignmentExpr(parser, B, fx9ParserContextGetMinusAssignOperator(parser), C);
}
assignment_expression(A) ::= unary_expression(B) LSHIFTASSIGN assignment_expression(C). {
    A = fx9ParserContextAcceptAssignmentExpr(parser, B, fx9ParserContextGetLeftShiftAssignOperator(parser), C);
}
assignment_expression(A) ::= unary_expression(B) RSHIFTASSIGN assignment_expression(C). {
    A = fx9ParserContextAcceptAssignmentExpr(parser, B, fx9ParserContextGetRightShiftAssignOperator(parser), C);
}
assignment_expression(A) ::= unary_expression(B) ANDASSIGN assignment_expression(C). {
    A = fx9ParserContextAcceptAssignmentExpr(parser, B, fx9ParserContextGetAndAssignOperator(parser), C);
}
assignment_expression(A) ::= unary_expression(B) XORASSIGN assignment_expression(C). {
    A = fx9ParserContextAcceptAssignmentExpr(parser, B, fx9ParserContextGetXorAssignOperator(parser), C);
}
assignment_expression(A) ::= unary_expression(B) ORASSIGN assignment_expression(C). {
    A = fx9ParserContextAcceptAssignmentExpr(parser, B, fx9ParserContextGetOrAssignOperator(parser), C);
}
assignment_expression(A) ::= ternary_expression(B). {
    A = B;
}
assignment_expression(A) ::= binary_expression(B). {
    A = B;
}

initializer_expression(A) ::= LBRACE initializer_list(B) COMMA RBRACE. {
    A = B;
}
initializer_expression(A) ::= LBRACE initializer_list(B) RBRACE. {
    A = B;
}
initializer_expression(A) ::= assignment_expression(B). {
    A = B;
}
initializer_list(A) ::= initializer_list(B) COMMA initializer_expression(C). {
    A = fx9ParserContextAcceptInitializerList(parser, B, C);
}
initializer_list(A) ::= initializer_expression(B). {
    A = fx9ParserContextAcceptInitializerList(parser, 0, B);
}

ternary_expression(A) ::= binary_expression(B) QUESTION expression(C) COLON ternary_expression(D). {
    A = fx9ParserContextAcceptTernaryStatement(parser, B, C, D);
}
ternary_expression(A) ::= binary_expression(B) QUESTION expression(C) COLON binary_expression(D). {
    A = fx9ParserContextAcceptTernaryStatement(parser, B, C, D);
}

binary_expression(A) ::= binary_expression(B) STAR binary_expression(D). {
    A = fx9ParserContextAcceptBinaryOperation(parser, B, fx9ParserContextCreateBinaryOperatorStar(parser), D);
}
binary_expression(A) ::= binary_expression(B) SLASH binary_expression(D). {
    A = fx9ParserContextAcceptBinaryOperation(parser, B, fx9ParserContextCreateBinaryOperatorSlash(parser), D);
}
binary_expression(A) ::= binary_expression(B) PERCENT binary_expression(D). {
    A = fx9ParserContextAcceptBinaryOperation(parser, B, fx9ParserContextCreateBinaryOperatorPercent(parser), D);
}
binary_expression(A) ::= binary_expression(B) PLUS binary_expression(D). {
    A = fx9ParserContextAcceptBinaryOperation(parser, B, fx9ParserContextCreateBinaryOperatorPlus(parser), D);
}
binary_expression(A) ::= binary_expression(B) MINUS binary_expression(D). {
    A = fx9ParserContextAcceptBinaryOperation(parser, B, fx9ParserContextCreateBinaryOperatorMinus(parser), D);
}
binary_expression(A) ::= binary_expression(B) LSHIFT binary_expression(D). {
    A = fx9ParserContextAcceptBinaryOperation(parser, B, fx9ParserContextCreateBinaryOperatorLeftShift(parser), D);
}
binary_expression(A) ::= binary_expression(B) RSHIFT binary_expression(D). {
    A = fx9ParserContextAcceptBinaryOperation(parser, B, fx9ParserContextCreateBinaryOperatorRightShift(parser), D);
}
binary_expression(A) ::= binary_expression(B) LT binary_expression(D). {
    A = fx9ParserContextAcceptBinaryOperation(parser, B, fx9ParserContextCreateBinaryOperatorLessThan(parser), D);
}
binary_expression(A) ::= binary_expression(B) GT binary_expression(D). {
    A = fx9ParserContextAcceptBinaryOperation(parser, B, fx9ParserContextCreateBinaryOperatorGreaterThan(parser), D);
}
binary_expression(A) ::= binary_expression(B) LE binary_expression(D). {
    A = fx9ParserContextAcceptBinaryOperation(parser, B, fx9ParserContextCreateBinaryOperatorLessThanEqual(parser), D);
}
binary_expression(A) ::= binary_expression(B) GE binary_expression(D). {
    A = fx9ParserContextAcceptBinaryOperation(parser, B, fx9ParserContextCreateBinaryOperatorGreaterThanEqual(parser), D);
}
binary_expression(A) ::= binary_expression(B) EQ binary_expression(D). {
    A = fx9ParserContextAcceptBinaryOperation(parser, B, fx9ParserContextCreateBinaryOperatorEqual(parser), D);
}
binary_expression(A) ::= binary_expression(B) NE binary_expression(D). {
    A = fx9ParserContextAcceptBinaryOperation(parser, B, fx9ParserContextCreateBinaryOperatorNotEqual(parser), D);
}
binary_expression(A) ::= binary_expression(B) AND binary_expression(D). {
    A = fx9ParserContextAcceptBinaryOperation(parser, B, fx9ParserContextCreateBinaryOperatorAnd(parser), D);
}
binary_expression(A) ::= binary_expression(B) OR binary_expression(D). {
    A = fx9ParserContextAcceptBinaryOperation(parser, B, fx9ParserContextCreateBinaryOperatorOr(parser), D);
}
binary_expression(A) ::= binary_expression(B) XOR binary_expression(D). {
    A = fx9ParserContextAcceptBinaryOperation(parser, B, fx9ParserContextCreateBinaryOperatorXor(parser), D);
}
binary_expression(A) ::= binary_expression(B) ANDAND binary_expression(D). {
    A = fx9ParserContextAcceptBinaryOperation(parser, B, fx9ParserContextCreateBinaryOperatorLogicalAnd(parser), D);
}
binary_expression(A) ::= binary_expression(B) OROR binary_expression(D). {
    A = fx9ParserContextAcceptBinaryOperation(parser, B, fx9ParserContextCreateBinaryOperatorLogicalOr(parser), D);
}
binary_expression(A) ::= unary_expression(B). {
    A = B;
}

unary_expression(A) ::= postfix_expression(B). {
    A = B;
}
unary_expression(A) ::= LPAREN type_specifier(B) array_size_specifier(C) RPAREN unary_expression(D). {
    A = fx9ParserContextAcceptCastOperation(parser, B, C, D);
}
unary_expression(A) ::= LPAREN type_specifier(B) RPAREN unary_expression(C). {
    A = fx9ParserContextAcceptCastOperation(parser, B, 0, C);
}
unary_expression(A) ::= PLUSPLUS postfix_expression(B). {
    A = fx9ParserContextAcceptUnaryMathPreIncrement(parser, B);
}
unary_expression(A) ::= MINUSMINUS postfix_expression(B). {
    A = fx9ParserContextAcceptUnaryMathPreDecrement(parser, B);
}
unary_expression(A) ::= AND postfix_expression(B). {
    A = fx9ParserContextAcceptUnaryMathOperation(parser, fx9ParserContextCreateUnaryOperatorAnd(parser), B);
}
unary_expression(A) ::= MUL postfix_expression(B). {
    A = fx9ParserContextAcceptUnaryMathOperation(parser, fx9ParserContextCreateUnaryOperatorMul(parser), B);
}
unary_expression(A) ::= PLUS postfix_expression(B). {
    A = B;
}
unary_expression(A) ::= MINUS postfix_expression(B). {
    A = fx9ParserContextAcceptUnaryMathOperation(parser, fx9ParserContextCreateUnaryOperatorMinus(parser), B);
}
unary_expression(A) ::= TILDE postfix_expression(B). {
    A = fx9ParserContextAcceptUnaryMathOperation(parser, fx9ParserContextCreateUnaryOperatorTilde(parser), B);
}
unary_expression(A) ::= EXCLAMATION postfix_expression(B). {
    A = fx9ParserContextAcceptUnaryMathOperation(parser, fx9ParserContextCreateUnaryOperatorExclamation(parser), B);
}

postfix_expression(A) ::= LPAREN expression(B) RPAREN. {
    A = fx9ParserContextFlattenExpression(parser, B);
}
postfix_expression(A) ::= postfix_expression(B) LBRACKET expression(C) RBRACKET. {
    A = fx9ParserContextAcceptBracketDeference(parser, B, C);
}
postfix_expression(A) ::= postfix_expression(B) DOT identifier(C). {
    A = fx9ParserContextAcceptDotDeference(parser, B, C);
}
postfix_expression(A) ::= postfix_expression(B) PLUSPLUS. {
    A = fx9ParserContextAcceptPostIncrement(parser, B);
}
postfix_expression(A) ::= postfix_expression(B) MINUSMINUS. {
    A = fx9ParserContextAcceptPostDecrement(parser, B);
}
postfix_expression(A) ::= constructor_call(B). {
    A = B;
}
postfix_expression(A) ::= function_call(B). {
    A = B;
}
postfix_expression(A) ::= literal(B). {
    A = B;
}
postfix_expression(A) ::= identifier(B). {
    A = fx9ParserContextAcceptVariable(parser, B);
}

constructor_call(A) ::= type_specifier(B) LPAREN argument_expression_list(C) RPAREN. {
    A = fx9ParserContextAcceptConstructorCall(parser, B, C);
}
function_call(A) ::= identifier(B) LPAREN argument_expression_list(C) RPAREN. {
    A = fx9ParserContextAcceptFunctionCall(parser, B, C);
}
function_call(A) ::= identifier(B) LPAREN RPAREN. {
    A = fx9ParserContextAcceptFunctionCall(parser, B, 0);
}

argument_expression_list(A) ::= argument_expression_list(B) COMMA assignment_expression(C). {
    A = fx9ParserContextAcceptFunctionArgument(parser, B, C);
}
argument_expression_list(A) ::= assignment_expression(B). {
    A = fx9ParserContextAcceptFunctionArgument(parser, 0, B);
}

literal(A) ::= bool_literal(B). {
    A = B;
}
literal(A) ::= int_literal(B). {
    A = B;
}
literal(A) ::= float_literal(B). {
    A = B;
}

identifier(A) ::= ID(B). {
    A = fx9ParserContextAcceptIdentifier(parser, B);
}
bool_literal(A) ::= FALSE(B). {
    A = fx9ParserContextAcceptFalseLiteral(parser, B);
}
bool_literal(A) ::= TRUE(B). {
    A = fx9ParserContextAcceptTrueLiteral(parser, B);
}
int_literal(A) ::= INT_LIT(B). {
    A = fx9ParserContextAcceptIntLiteral(parser, B);
}
float_literal(A) ::= FLOAT_LIT(B). {
    A = fx9ParserContextAcceptFloatLiteral(parser, B);
}
string(A) ::= STRING(B). {
    A = fx9ParserContextAcceptStringLiteral(parser, B);
}

attributes_decl(A) ::= attributes_decl(B) LBRACKET identifier(C) LPAREN identifier(D) RPAREN RBRACKET .{
    A = fx9ParserContextAcceptAttribute(parser, B, C, D);
}
attributes_decl(A) ::= attributes_decl(B) LBRACKET identifier(C) RBRACKET .{
    A = fx9ParserContextAcceptAttribute(parser, B, C, 0);
}
attributes_decl(A) ::= LBRACKET identifier(B) LPAREN identifier(C) RPAREN RBRACKET .{
    A = fx9ParserContextAcceptAttribute(parser, 0, B, C);
}
attributes_decl(A) ::= LBRACKET identifier(B) RBRACKET .{
    A = fx9ParserContextAcceptAttribute(parser, 0, B, 0);
}


register_decl(A) ::= REGISTER LPAREN identifier COMMA identifier(B) RPAREN. {
    A = B;
}
register_decl(A) ::= REGISTER LPAREN identifier(B) RPAREN. {
    A = B;
}

sampler_decl(A) ::= fully_specified_type(B) identifier(C) COLON register_decl(D) ASSIGN SAMPLER_STATE sampler_state_compound_statement(E). {
    A = fx9ParserContextCreateSamplerState(parser, B, C, D, E);
}
sampler_decl(A) ::= fully_specified_type(B) identifier(C) ASSIGN SAMPLER_STATE sampler_state_compound_statement(D). {
    A = fx9ParserContextCreateSamplerState(parser, B, C, 0, D);
}
sampler_decl(A) ::= fully_specified_type(B) identifier(C) sampler_state_compound_statement(D). {
    A = fx9ParserContextCreateSamplerState(parser, B, C, 0, D);
}
sampler_decl(A) ::= fully_specified_type(B) identifier(C) COLON register_decl(D). {
    A = fx9ParserContextCreateSamplerState(parser, B, C, D, 0);
}
sampler_type_specifier(A) ::= SAMPLER(B). {
    A = fx9ParserContextCreateTypeSpecSamplerIndetermine(parser, B);
}
sampler_type_specifier(A) ::= SAMPLER2D(B). {
    A = fx9ParserContextCreateTypeSpecSampler2D(parser, B);
}
sampler_type_specifier(A) ::= SAMPLER3D(B). {
    A = fx9ParserContextCreateTypeSpecSampler3D(parser, B);
}
sampler_type_specifier(A) ::= SAMPLERCUBE(B). {
    A = fx9ParserContextCreateTypeSpecSamplerCube(parser, B);
}
texture_type_specifier(A) ::= TEXTURE(B). {
    A = fx9ParserContextCreateTypeSpecTextureIndetermine(parser, B);
}
texture_type_specifier(A) ::= TEXTURE2D(B). {
    A = fx9ParserContextCreateTypeSpecTexture2D(parser, B);
}
texture_type_specifier(A) ::= TEXTURE3D(B). {
    A = fx9ParserContextCreateTypeSpecTexture3D(parser, B);
}
texture_type_specifier(A) ::= TEXTURECUBE(B). {
    A = fx9ParserContextCreateTypeSpecTextureCube(parser, B);
}

sampler_state_compound_statement(A) ::= sampler_state_decl_left sampler_state_decl_list(B) sampler_state_decl_right. {
    A = B;
}
sampler_state_decl_left ::= LBRACE. {
    fx9ParserContextDisableAcceptingVariable(parser);
}
sampler_state_decl_right ::= RBRACE. {
    fx9ParserContextEnableAcceptingVariable(parser);
}
sampler_state_decl_list(A) ::= sampler_state_decl_list(B) sampler_state_decl(C). {
    A = fx9ParserContextAcceptSamplerStateValue(parser, B, C);
}
sampler_state_decl_list(A) ::= sampler_state_decl(B). {
    A = fx9ParserContextAcceptSamplerStateValue(parser, 0, B);
}
sampler_state_decl(A) ::= sampler_state(B) SEMICOLON. {
    A = B;
}
sampler_state(A) ::= texture_type_specifier ASSIGN LPAREN identifier(C) RPAREN. {
    A = fx9ParserContextCreateSamplerStateTexture(parser, C);
}
sampler_state(A) ::= texture_type_specifier ASSIGN LT identifier(C) GT. {
    A = fx9ParserContextCreateSamplerStateTexture(parser, C);
}
sampler_state(A) ::= texture_type_specifier ASSIGN identifier(C). {
    A = fx9ParserContextCreateSamplerStateTexture(parser, C);
}
sampler_state(A) ::= identifier ASSIGN LT identifier(C) GT. {
    A = fx9ParserContextCreateSamplerStateTexture(parser, C);
}
sampler_state(A) ::= identifier(B) ASSIGN initializer_expression(C). {
    A = fx9ParserContextCreateSamplerStatePair(parser, B, C);
}

/* HLSL FX extension */
technique_decl(A) ::= TECHNIQUE identifier(B) annotation_decl(C) LBRACE pass_list(D) RBRACE. {
    A = fx9ParserContextCreateTechnique(parser, B, D, C);
}
technique_decl(A) ::= TECHNIQUE identifier(B) LBRACE pass_list(C) RBRACE. {
    A = fx9ParserContextCreateTechnique(parser, B, C, 0);
}
technique_decl(A) ::= TECHNIQUE identifier(B) annotation_decl(C) LBRACE RBRACE. {
    A = fx9ParserContextCreateTechnique(parser, B, 0, C);
}
technique_decl(A) ::= TECHNIQUE identifier(B) LBRACE RBRACE. {
    A = fx9ParserContextCreateTechnique(parser, B, 0, 0);
}
pass_list(A) ::= pass_list(B) pass_decl(C). {
    A = fx9ParserContextAppendPass(parser, B, C);
}
pass_list(A) ::= pass_decl(B). {
    A = fx9ParserContextAppendPass(parser, 0, B);
}
pass_decl(A) ::= PASS identifier(B) annotation_decl(C) LBRACE pass_state_list(D) RBRACE. {
    A = fx9ParserContextCreatePass(parser, B, D, C);
}
pass_decl(A) ::= PASS identifier(B) LBRACE pass_state_list(C) RBRACE. {
    A = fx9ParserContextCreatePass(parser, B, C, 0);
}
pass_decl(A) ::= PASS identifier(B) annotation_decl(C) LBRACE RBRACE. {
    A = fx9ParserContextCreatePass(parser, B, 0, C);
}
pass_decl(A) ::= PASS identifier(B) LBRACE RBRACE. {
    A = fx9ParserContextCreatePass(parser, B, 0, 0);
}
external_decl(A) ::= state_block_decl. {
    A = 0;
}
state_block_decl(A) ::= STATEBLOCK identifier(B) ASSIGN pass_state_block_decl(C) SEMICOLON. {
    A = fx9ParserContextCreateStateBlock(parser, B, C);
}
pass_state_block_decl(A) ::= STATEBLOCK_STATE LBRACE pass_state_list(B) RBRACE. {
    A = B;
}
pass_state_block_decl(A) ::= STATEBLOCK_STATE LBRACE RBRACE. {
    A = 0;
}
pass_state_list(A) ::= pass_state_list(B) pass_state(C). {
    A = fx9ParserContextAppendPassState(parser, B, C);
}
pass_state_list(A) ::= pass_state(B). {
    A = fx9ParserContextAppendPassState(parser, 0, B);
}
pass_state(A) ::= identifier(B) ASSIGN COMPILE identifier identifier(C) LPAREN argument_expression_list(D) RPAREN SEMICOLON. {
    A = fx9ParserContextCreatePassEntryPoint(parser, B, C, D);
}
pass_state(A) ::= identifier(B) ASSIGN COMPILE identifier identifier(C) LPAREN RPAREN SEMICOLON. {
    A = fx9ParserContextCreatePassEntryPoint(parser, B, C, 0);
}
pass_state(A) ::= identifier(B) array_size_specifier ASSIGN LPAREN identifier(C) RPAREN SEMICOLON. {
    A = fx9ParserContextCreatePassState(parser, B, C);
}
pass_state(A) ::= identifier(B) array_size_specifier ASSIGN identifier(C) SEMICOLON. {
    A = fx9ParserContextCreatePassState(parser, B, C);
}
pass_state(A) ::= identifier(B) array_size_specifier ASSIGN literal(C) SEMICOLON. {
    A = fx9ParserContextCreatePassState(parser, B, C);
}
pass_state(A) ::= identifier(B) ASSIGN LPAREN pass_state_arguments(C) RPAREN SEMICOLON. {
    A = fx9ParserContextCreatePassState(parser, B, C);
}
pass_state(A) ::= identifier(B) ASSIGN pass_state_arguments(C) SEMICOLON. {
    A = fx9ParserContextCreatePassState(parser, B, C);
}
pass_state(A) ::= SEMICOLON. {
    A = 0;
}
pass_state_arguments(A) ::= identifier(B) OR identifier. {
    A = B;
}
pass_state_arguments(A) ::= identifier(B). {
    A = B;
}
pass_state_arguments(A) ::= literal(B). {
    A = B;
}
