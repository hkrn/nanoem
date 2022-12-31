#include <clang-c/Index.h>

#include <stdio.h>
#include <string.h>

#include <string>
#include <unordered_map>
#include <vector>

namespace {

struct Visitor {
    static std::string
    commonType(CXType type, const std::string prefix = std::string())
    {
        std::string s(prefix);
        switch (type.kind) {
        case CXType_Bool: {
            s.append("bool");
            break;
        }
        case CXType_SChar:
        case CXType_Char_S: {
            s.append("schar");
            break;
        }
        case CXType_UChar:
        case CXType_Char_U: {
            s.append("char");
            break;
        }
        case CXType_Short: {
            s.append("short");
            break;
        }
        case CXType_UShort: {
            s.append("ushort");
            break;
        }
        case CXType_Int:
        case CXType_Long: {
            s.append("int");
            break;
        }
        case CXType_UInt:
        case CXType_ULong: {
            s.append("uint");
            break;
        }
        case CXType_LongLong: {
            s.append("long");
            break;
        }
        case CXType_ULongLong: {
            s.append("ulong");
            break;
        }
        case CXType_Float: {
            s.append("nanoem_f32_t");
            break;
        }
        case CXType_Double: {
            s.append("nanoem_f64_t");
            break;
        }
        case CXType_Pointer: {
            s.append("IntPtr");
            break;
        }
        case CXType_Void: {
            s.append("void");
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
    canonicalizeArgumentType(CXType type)
    {
        CXString typeSpelling = clang_getCursorSpelling(clang_getTypeDeclaration(type));
        if (strcmp(clang_getCString(typeSpelling), "nanoem_rsize_t") == 0) {
            clang_disposeString(typeSpelling);
            return clang_getCanonicalType(type).kind == CXType_Pointer ? "out usize" : "usize";
        }
        clang_disposeString(typeSpelling);
        CXType canonicalType = clang_getCanonicalType(type);
        switch (canonicalType.kind) {
        case CXType_Pointer: {
            CXType pointeeType = clang_getPointeeType(canonicalType);
            switch (pointeeType.kind) {
            case CXType_Char_S:
            case CXType_SChar:
                return clang_isConstQualifiedType(pointeeType) ? "string" : "IntPtr";
            case CXType_Pointer: {
                const std::string name(canonicalizeArgumentType(pointeeType));
                return name;
            }
            case CXType_Record: {
                CXString spelling = clang_getCursorSpelling(clang_getTypeDeclaration(pointeeType));
                const std::string name(clang_getCString(spelling));
                clang_disposeString(spelling);
                return name;
            }
            case CXType_FunctionProto: {
                CXString spelling = clang_getTypeSpelling(type);
                const std::string name(clang_getCString(spelling));
                clang_disposeString(spelling);
                return name;
            }
            case CXType_Char_U:
            case CXType_UChar:
            case CXType_Void: {
                return "IntPtr";
            }
            default:
                CXString typeSpelling = clang_getTypeSpelling(clang_getPointeeType(type));
                if (strcmp(clang_getCString(typeSpelling), "nanoem_rsize_t") == 0) {
                    clang_disposeString(typeSpelling);
                    return "out usize";
                }
                clang_disposeString(typeSpelling);
                return clang_isConstQualifiedType(pointeeType) ? "IntPtr" : commonType(pointeeType, "out ");
            }
        }
        default:
            return commonType(canonicalType);
        }
    }
    static std::string
    canonicalizeReturnType(CXType type)
    {
        CXType canonicalType = clang_getCanonicalType(type);
        switch (canonicalType.kind) {
        case CXType_Pointer: {
            CXType pointeeType = clang_getPointeeType(canonicalType);
            switch (pointeeType.kind) {
            case CXType_Char_S:
            case CXType_SChar:
                return clang_isConstQualifiedType(pointeeType) ? "string" : "IntPtr";
            default:
                return "IntPtr";
            }
        }
        default:
            return commonType(canonicalType);
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

    EnumVisitor(PairList *values)
        : m_values(values)
    {
    }
    ~EnumVisitor()
    {
    }

    static CXChildVisitResult
    visitFunctionChildren(CXCursor cursor, CXCursor /* parent */, CXClientData client_data)
    {
        EnumVisitor *self = static_cast<EnumVisitor *>(client_data);
        CXString spelling = clang_getCursorSpelling(cursor);
        self->m_values->push_back(
            std::make_pair(clang_getCString(spelling) + 7, clang_getEnumConstantDeclValue(cursor)));
        clang_disposeString(spelling);
        return CXChildVisit_Continue;
    }

    PairList *m_values;
};

struct DelegateVisitor {
    DelegateVisitor()
    {
    }
    ~DelegateVisitor()
    {
    }

    static CXChildVisitResult
    visitFunctionChildren(CXCursor cursor, CXCursor /* parent */, CXClientData client_data)
    {
        if (cursor.kind == CXCursor_ParmDecl) {
            DelegateVisitor *self = static_cast<DelegateVisitor *>(client_data);
            CXString nameSpelling = clang_getCursorSpelling(cursor);
            std::string type(Visitor::canonicalizeArgumentType(clang_getCursorType(cursor)));
            std::string name(clang_getCString(nameSpelling));
            self->m_arguments.push_back(std::make_pair(type, name));
            clang_disposeString(nameSpelling);
        }
        return CXChildVisit_Continue;
    }
    std::vector<std::pair<std::string, std::string>> m_arguments;
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
            CXType type = clang_getArgType(clang_getCursorType(cursor), i);
            result.append(Visitor::canonicalizeArgumentType(type).c_str());
            CXString spelling = clang_getCursorSpelling(clang_Cursor_getArgument(cursor, i));
            const std::string name = clang_getCString(spelling);
            result.append(" @");
            result.append(Visitor::completeFunctionArgumentName(name, i));
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
                EnumVisitor visitor(&enums);
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
                else if (underlyingType.kind == CXType_Pointer) {
                    DelegateVisitor visitor;
                    CXType canonicalType = clang_getCanonicalType(underlyingType);
                    clang_visitChildren(cursor, DelegateVisitor::visitFunctionChildren, &visitor);
                    CXString spelling = clang_getCursorSpelling(cursor);
                    CXType pointeeType = clang_getPointeeType(canonicalType);
                    CXType resultType = clang_getResultType(pointeeType);
                    const std::string result = Visitor::canonicalizeReturnType(resultType);
                    self->m_delegates.push_back(Delegate(clang_getFunctionTypeCallingConv(canonicalType),
                        clang_getCString(spelling), result, visitor.m_arguments));
                    clang_disposeString(spelling);
                }
            }
            else if (kind == CXCursor_FunctionDecl) {
                CXString spelling = clang_getCursorSpelling(cursor);
                const std::string name(clang_getCString(spelling));
                clang_disposeString(spelling);
                std::string funcPtr;
                funcPtr.append(Visitor::canonicalizeReturnType(clang_getCursorResultType(cursor)));
                funcPtr.append(" ");
                funcPtr.append(name.c_str() + 6);
                if (self->completeFunctionArguments(cursor, funcPtr)) {
                    self->m_functionPointers.push_back(FunctionPointer(name, funcPtr));
                    self->m_functionNames.push_back(name);
                }
                result = CXChildVisit_Continue;
            }
        }
        return result;
    }

    struct FunctionPointer {
        FunctionPointer(const std::string entryPoint, const std::string methodName)
            : m_entryPoint(entryPoint)
            , m_methodName(methodName)
        {
        }
        std::string m_entryPoint;
        std::string m_methodName;
    };
    struct Delegate {
        Delegate(CXCallingConv conv, const std::string name, const std::string returnType,
            const std::vector<std::pair<std::string, std::string>> arguments)
            : m_name(name)
            , m_returnType(returnType)
        {
            switch (conv) {
            case CXCallingConv_X86StdCall:
            case CXCallingConv_X86_64Win64:
                m_conversion = "CallingConvention.StdCall";
                break;
            default:
                m_conversion = "CallingConvention.Cdecl";
                break;
            }
            for (int i = 0; i < arguments.size(); i++) {
                auto v = arguments[i];
                m_arguments.append(v.first);
                m_arguments.append(" @");
                m_arguments.append(Visitor::completeFunctionArgumentName(v.second, i));
                if (i < arguments.size() - 1) {
                    m_arguments.append(", ");
                }
            }
        }
        std::string m_conversion;
        std::string m_name;
        std::string m_returnType;
        std::string m_arguments;
    };
    std::unordered_map<std::string, EnumVisitor::PairList> m_enumValues;
    std::vector<FunctionPointer> m_typedefNames;
    std::vector<std::string> m_functionNames;
    std::vector<std::string> m_opaqueNames;
    std::vector<FunctionPointer> m_functionPointers;
    std::vector<Delegate> m_delegates;
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
        FILE *fp = fopen("./nanoem.cs", "wb");
        fprintf(fp,
            "/*\n"
            "   Copyright (c) 2015-2023 hkrn All rights reserved\n"
            "\n"
            "   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md "
            "for more details.\n"
            "*/\n"
            "\n"
            "/* This file is automatically generated. DO NOT EDIT! */\n"
            "\n"
            "using System;\n"
            "using System.Runtime.InteropServices;\n"
            "\n");
        for (auto it : visitor.m_opaqueNames) {
            fprintf(fp, "using %s = System.IntPtr;\n", it.c_str());
        }
        fprintf(fp,
            "\n"
            "namespace nanoem.Interop.Native\n"
            "{\n"
            "    internal enum usize : ulong {}\n"
            "\n");
        for (auto it : visitor.m_enumValues) {
            fprintf(fp, "    internal enum %s : int {\n", it.first.c_str());
            for (auto it2 : it.second) {
                fprintf(fp, "        %s = %lld,\n", it2.first.c_str(), it2.second);
            }
            fprintf(fp,
                "    };\n"
                "\n");
        }
        for (auto it : visitor.m_delegates) {
            fprintf(fp,
                "    [UnmanagedFunctionPointer(%s)]\n"
                "    delegate %s %s(%s);\n\n",
                it.m_conversion.c_str(), it.m_returnType.c_str(), it.m_name.c_str(), it.m_arguments.c_str());
        }
        fprintf(fp,
            "    static class PInvoke\n"
            "    {\n"
            "        private const string libraryName = \"nanoem.dll\";\n\n");
        for (auto it : visitor.m_functionPointers) {
            fprintf(fp,
                "        [DllImport(libraryName, EntryPoint = \"%s\")]\n"
                "        internal static extern %s;\n",
                it.m_entryPoint.c_str(), it.m_methodName.c_str());
        }
        fprintf(fp,
            "    }\n"
            "}\n");
        fclose(fp);
        clang_disposeIndex(index);
    }
    return 0;
}
