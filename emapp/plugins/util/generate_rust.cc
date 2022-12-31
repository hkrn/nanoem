#include <clang-c/Index.h>

#include <stdio.h>
#include <string.h>

#include <string>
#include <unordered_map>
#include <vector>

namespace {

struct Visitor {
    static std::string
    commonType(CXType type, bool &isPointer, bool &isMutable)
    {
        std::string s;
        switch (type.kind) {
        case CXType_Bool: {
            s.append("bool");
            break;
        }
        case CXType_SChar:
        case CXType_Char_S: {
            s.append("i8");
            break;
        }
        case CXType_UChar:
        case CXType_Char_U: {
            s.append("u8");
            break;
        }
        case CXType_Short: {
            s.append("i16");
            break;
        }
        case CXType_UShort: {
            s.append("u16");
            break;
        }
        case CXType_Int:
        case CXType_Long: {
            s.append("i32");
            break;
        }
        case CXType_UInt:
        case CXType_ULong: {
            s.append("u32");
            break;
        }
        case CXType_LongLong: {
            s.append("i64");
            break;
        }
        case CXType_ULongLong: {
            s.append("u64");
            break;
        }
        case CXType_Float: {
            s.append("f32");
            break;
        }
        case CXType_Double: {
            s.append("f64");
            break;
        }
        case CXType_Pointer: {
            isPointer = isMutable = false;
            s.append("libc::c_void");
            break;
        }
        case CXType_Void: {
            s.append("()");
            break;
        }
        case CXType_Enum: {
            CXString spelling = clang_getTypeSpelling(type);
            s.append(clang_getCString(spelling));
            clang_disposeString(spelling);
            break;
        }
        default:
            s.append("UNKNOWN_TYPE");
            break;
        }
        return s;
    }
    static std::string
    canonicalizeArgumentType(CXType type, bool &isPointer, bool &isMutable, bool &isArray)
    {
        CXString typeSpelling = clang_getCursorSpelling(clang_getTypeDeclaration(type));
        if (strcmp(clang_getCString(typeSpelling), "nanoem_rsize_t") == 0) {
            clang_disposeString(typeSpelling);
            return "usize";
        }
        clang_disposeString(typeSpelling);
        CXType canonicalType = clang_getCanonicalType(type);
        switch (canonicalType.kind) {
        case CXType_Pointer: {
            CXType pointeeType = clang_getPointeeType(canonicalType);
            isMutable = clang_isConstQualifiedType(pointeeType) ? false : true;
            switch (pointeeType.kind) {
            case CXType_Pointer: {
                const std::string name(canonicalizeArgumentType(pointeeType, isPointer, isMutable, isArray));
                isArray = true;
                return name;
            }
            case CXType_Record: {
                CXString spelling = clang_getCursorSpelling(clang_getTypeDeclaration(pointeeType));
                const std::string name(clang_getCString(spelling));
                clang_disposeString(spelling);
                isPointer = true;
                return name;
            }
            case CXType_FunctionProto: {
                CXString spelling = clang_getTypeSpelling(type);
                const std::string name(clang_getCString(spelling));
                clang_disposeString(spelling);
                isMutable = false;
                return "libc::c_void"; // name;
            }
            case CXType_Void: {
                isMutable = false;
                return "libc::c_void";
            }
            default: {
                isPointer = true;
                CXString typeSpelling = clang_getTypeSpelling(clang_getPointeeType(type));
                if (strcmp(clang_getCString(typeSpelling), "nanoem_rsize_t") == 0) {
                    clang_disposeString(typeSpelling);
                    return "usize";
                }
                clang_disposeString(typeSpelling);
                return commonType(pointeeType, isPointer, isMutable);
            }
            }
        }
        default:
            return commonType(canonicalType, isPointer, isMutable);
        }
    }
    static std::string
    canonicalizeReturnType(CXType type, bool &isPointer, bool &isMutable, bool &isArray)
    {
        CXType canonicalType = clang_getCanonicalType(type);
        switch (canonicalType.kind) {
        case CXType_Pointer: {
            CXType pointeeType = clang_getPointeeType(canonicalType);
            isMutable = clang_isConstQualifiedType(pointeeType) ? false : true;
            switch (pointeeType.kind) {
            case CXType_Pointer: {
                const std::string name(canonicalizeReturnType(pointeeType, isPointer, isMutable, isArray));
                isMutable = isPointer = false;
                isArray = true;
                return name;
            }
            case CXType_Record: {
                CXString spelling = clang_getCursorSpelling(clang_getTypeDeclaration(pointeeType));
                const std::string name(clang_getCString(spelling));
                clang_disposeString(spelling);
                isPointer = true;
                return name;
            }
            case CXType_Void: {
                isMutable = false;
                return "libc::c_void";
            }
            default:
                isPointer = true;
                return commonType(pointeeType, isPointer, isMutable);
            }
        }
        default:
            return commonType(canonicalType, isPointer, isMutable);
        }
    }
    static std::string
    completeFunctionArgumentName(const std::string name, int index)
    {
        std::string result;
        if (!name.empty()) {
            result.append(name);
        }
        else {
            result.append("param");
            result.push_back('0' + index);
        }
        return result;
    }
};

struct EnumVisitor {
    using PairList = std::vector<std::pair<std::string, int64_t>>;

    EnumVisitor(CXString name, PairList *values)
        : m_name(name)
        , m_values(values)
    {
    }
    ~EnumVisitor()
    {
        clang_disposeString(m_name);
    }

    static CXChildVisitResult
    visitFunctionChildren(CXCursor cursor, CXCursor /* parent */, CXClientData client_data)
    {
        EnumVisitor *self = static_cast<EnumVisitor *>(client_data);
        CXString spelling = clang_getCursorSpelling(cursor);
        if (strstr(clang_getCString(spelling), "FIRST_ENUM") == nullptr) {
            const char *name = clang_getCString(spelling);
            const char *s = clang_getCString(self->m_name);
            auto value = clang_getEnumConstantDeclValue(cursor);
            self->m_values->push_back(std::make_pair(name + (strlen(s) - 1), value));
        }
        clang_disposeString(spelling);
        return CXChildVisit_Continue;
    }

    CXString m_name;
    PairList *m_values;
};

struct FunctionVisitor {
    FunctionVisitor()
    {
    }
    ~FunctionVisitor()
    {
    }

    bool
    completeFunctionArguments(CXCursor cursor, std::string &result)
    {
        result.append("(");
        for (int i = 0, numArguments = clang_Cursor_getNumArguments(cursor); i < numArguments; i++) {
            CXCursor arguemntCursor = clang_Cursor_getArgument(cursor, i);
            CXType type = clang_getArgType(clang_getCursorType(cursor), i);
            bool isPointer = false, isMutable = false, isArray = false;
            std::string typeString(Visitor::canonicalizeArgumentType(type, isPointer, isMutable, isArray));
            CXString spelling = clang_getCursorSpelling(arguemntCursor);
            const std::string name = clang_getCString(spelling);
            result.append(Visitor::completeFunctionArgumentName(name, i));
            result.append(": ");
            if (isArray) {
                result.append("*const *mut ");
            }
            else if (isPointer) {
                result.append(isMutable ? "*mut " : "*const ");
            }
            else if (isMutable) {
                result.append("mut ");
            }
            result.append(typeString);
            clang_disposeString(spelling);
            if (i < numArguments - 1) {
                result.append(", ");
            }
        }
        result.append(")");
        return true;
    }
    static CXChildVisitResult
    visitFunctionChildren(CXCursor cursor, CXCursor /* parent */, CXClientData client_data)
    {
        FunctionVisitor *self = static_cast<FunctionVisitor *>(client_data);
        CXChildVisitResult result = CXChildVisit_Recurse;
        if (clang_Location_isInSystemHeader(clang_getCursorLocation(cursor))) {
            result = CXChildVisit_Continue;
        }
        else {
            CXCursorKind kind = clang_getCursorKind(cursor);
            if (kind == CXCursor_EnumDecl) {
                EnumVisitor::PairList enums;
                EnumVisitor visitor(clang_getCursorSpelling(cursor), &enums);
                clang_visitChildren(cursor, EnumVisitor::visitFunctionChildren, &visitor);
                CXString spelling = clang_getCursorSpelling(cursor);
                self->m_enumValues[clang_getCString(spelling)] = enums;
                clang_disposeString(spelling);
            }
            else if (kind == CXCursor_TypedefDecl) {
                CXType underlyingType = clang_getTypedefDeclUnderlyingType(cursor);
                if (underlyingType.kind == CXType_Elaborated) {
                    CXString spelling = clang_getCursorSpelling(cursor);
                    self->m_opaqueNames.push_back(clang_getCString(spelling));
                    clang_disposeString(spelling);
                }
            }
            else if (kind == CXCursor_FunctionDecl) {
                CXString spelling = clang_getCursorSpelling(cursor);
                const std::string name(clang_getCString(spelling));
                clang_disposeString(spelling);
                std::string funcPtr;
                funcPtr.append(name);
                if (self->completeFunctionArguments(cursor, funcPtr)) {
                    funcPtr.append(" -> ");
                    bool isPointer = false, isMutable = false, isArray = false;
                    const std::string type(Visitor::canonicalizeReturnType(
                        clang_getCursorResultType(cursor), isPointer, isMutable, isArray));
                    if (isArray) {
                        funcPtr.append("*const *mut ");
                    }
                    else if (isPointer) {
                        funcPtr.append(isMutable ? "*mut " : "*const ");
                    }
                    else if (isMutable) {
                        funcPtr.append("mut ");
                    }
                    funcPtr.append(type);
                    funcPtr.append(";");
                    self->m_functionPointers.push_back(FunctionPointer(funcPtr));
                    self->m_functionNames.push_back(name);
                }
                result = CXChildVisit_Continue;
            }
        }
        return result;
    }

    struct FunctionPointer {
        FunctionPointer(const std::string methodName)
            : m_methodName(methodName)
        {
        }
        std::string m_methodName;
    };
    std::unordered_map<std::string, EnumVisitor::PairList> m_enumValues;
    std::vector<FunctionPointer> m_typedefNames;
    std::vector<std::string> m_functionNames;
    std::vector<std::string> m_opaqueNames;
    std::vector<FunctionPointer> m_functionPointers;
};

} /* namespace anonymous */

int
main(int argc, char *argv[])
{
    if (argc >= 2) {
        CXIndex index = clang_createIndex(0, 0);
        std::vector<CXTranslationUnit> translationUnits;
        for (int i = 1; i < argc; i++) {
            const char *sourceLocation = argv[1];
            char includePath[1024];
            if (const char *p = strrchr(sourceLocation, '/')) {
                strncpy(includePath, sourceLocation, p - sourceLocation);
            }
            const char *options[] = { "-std=c++11", "-I", includePath };
            CXTranslationUnit translationUnit;
            CXErrorCode code = clang_parseTranslationUnit2(
                index, sourceLocation, options, sizeof(options) / sizeof(options[0]), NULL, 0, 0, &translationUnit);
            if (code == CXError_Success) {
                translationUnits.push_back(translationUnit);
            }
        }
        FunctionVisitor visitor;
        for (auto it : translationUnits) {
            clang_visitChildren(clang_getTranslationUnitCursor(it), FunctionVisitor::visitFunctionChildren, &visitor);
            clang_disposeTranslationUnit(it);
        }
        FILE *fp = fopen("./nanoem.rs", "wb");
        fprintf(fp, R"(
/*
  Copyright (c) 2015-2023 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

/* This file is automatically generated. DO NOT EDIT! */

extern crate libc;

)");
        for (auto it : visitor.m_opaqueNames) {
            fprintf(fp, "#[allow(dead_code)]\n");
            fprintf(fp, "#[derive(Debug, Hash)]\n");
            fprintf(fp, "#[repr(C)]\n");
            fprintf(fp, "pub struct %s { _private: [u8; 0] }\n\n", it.c_str());
        }
        for (auto it : visitor.m_enumValues) {
            fprintf(fp, "#[allow(dead_code, non_camel_case_types)]\n");
            fprintf(fp, "#[derive(Copy, Clone, Debug, Hash, PartialEq, PartialOrd)]\n");
            fprintf(fp, "#[repr(i32)]\n");
            fprintf(fp, "pub enum %s {\n", it.first.c_str());
            for (auto it2 : it.second) {
                fprintf(fp, "    %s = %lld,\n", it2.first.c_str(), it2.second);
            }
            fprintf(fp, "}\n\n");
        }
        fprintf(fp, R"(
#[link(name="nanoem")]
extern {
)");
        for (auto it : visitor.m_functionPointers) {
            fprintf(fp, "    #[allow(dead_code)]\n");
            fprintf(fp, "    pub fn %s\n", it.m_methodName.c_str());
        }
        fprintf(fp,
            "    pub fn nanoemUnicodeStringFactoryCreateEXT(status: *mut nanoem_status_t) -> *mut "
            "nanoem_unicode_string_factory_t;\n");
        fprintf(fp,
            "    pub fn nanoemUnicodeStringFactoryDestroyEXT(factory: *mut nanoem_unicode_string_factory_t) -> ();\n");
        fprintf(fp, "}\n");
        fclose(fp);
        clang_disposeIndex(index);
    }
    return 0;
}
