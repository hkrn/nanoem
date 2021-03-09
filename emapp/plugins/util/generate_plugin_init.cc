#include <clang-c/Index.h>

#include <stdio.h>
#include <string.h>

#include <string>
#include <unordered_map>
#include <vector>

struct EnumVisitor {
    typedef std::vector<std::pair<std::string, long long>> PairList;

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
        CXString name = clang_getCursorSpelling(cursor);
        self->m_values->push_back(std::make_pair(clang_getCString(name), clang_getEnumConstantDeclValue(cursor)));
        clang_disposeString(name);
        return CXChildVisit_Continue;
    }

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
            CXType type = clang_getArgType(clang_getCursorType(cursor), i);
            if (type.kind == CXType_Typedef) {
                CXType underlyingType = clang_getTypedefDeclUnderlyingType(clang_getTypeDeclaration(type));
                CXType pointeeType = clang_getPointeeType(underlyingType);
                if (pointeeType.kind == CXType_Unexposed) {
                    CXType canonicalType = clang_getCanonicalType(pointeeType);
                    if (canonicalType.kind == CXType_FunctionProto) {
                        return false;
                    }
                }
            }
            CXString spelling = clang_getTypeSpelling(type);
            result.append(clang_getCString(spelling));
            if (i < numArguments - 1) {
                result.append(", ");
            }
            clang_disposeString(spelling);
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
            if (kind == CXCursor_TypedefDecl) {
                CXType underlyingType = clang_getTypedefDeclUnderlyingType(cursor);
                CXType pointeeType = clang_getPointeeType(underlyingType);
                bool proceed = true;
                if (pointeeType.kind == CXType_Unexposed) {
                    CXType canonicalType = clang_getCanonicalType(pointeeType);
                    if (canonicalType.kind == CXType_FunctionProto) {
                        proceed = false;
                    }
                }
                if (proceed) {
                    CXString underlyingTypeSpelling = clang_getTypeSpelling(underlyingType);
                    CXString spelling = clang_getCursorSpelling(cursor);
                    self->m_typedefNames.push_back(
                        std::make_pair(clang_getCString(underlyingTypeSpelling), clang_getCString(spelling)));
                    clang_disposeString(underlyingTypeSpelling);
                    clang_disposeString(spelling);
                }
            }
            else if (kind == CXCursor_EnumDecl) {
                EnumVisitor::PairList enums;
                EnumVisitor visitor(&enums);
                clang_visitChildren(cursor, EnumVisitor::visitFunctionChildren, &visitor);
                self->m_enumValues.push_back(enums);
            }
            else if (kind == CXCursor_FunctionDecl) {
                CXString spelling = clang_getCursorSpelling(cursor);
                const std::string name(clang_getCString(spelling));
                clang_disposeString(spelling);
                std::string funcPtr;
                CXString resultTypeSpelling = clang_getTypeSpelling(clang_getCursorResultType(cursor));
                funcPtr.append(clang_getCString(resultTypeSpelling));
                clang_disposeString(resultTypeSpelling);
                funcPtr.append(" (NANOEM_DECL_PLUGIN_API *PFN_");
                funcPtr.append(name);
                funcPtr.append(")");
                if (self->completeFunctionArguments(cursor, funcPtr)) {
                    self->m_functionPointers.push_back(std::make_pair(name, funcPtr));
                    self->m_functionNames.push_back(name);
                }
                result = CXChildVisit_Continue;
            }
        }
        return result;
    }

    typedef std::pair<std::string, std::string> StringPair;
    std::vector<EnumVisitor::PairList> m_enumValues;
    std::vector<StringPair> m_typedefNames;
    std::vector<std::string> m_functionNames;
    std::vector<StringPair> m_functionPointers;
};

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
            const char *options[] = { "-std",
                "c99"
                "-I",
                includePath };
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
        FILE *fp = fopen("./init.h", "wb");
        fprintf(fp,
            "/*\n"
            "   Copyright (c) 2015-2021 hkrn All rights reserved\n"
            "\n"
            "   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md "
            "for more details.\n"
            "*/\n"
            "\n"
            "/* This file is automatically generated. DO NOT EDIT! */\n"
            "\n"
            "#include <stdbool.h>\n"
            "#include <stddef.h>\n"
            "#include <stdint.h>\n"
            "#ifdef _WIN32\n"
            "#include <windows.h>\n"
            "#define NANOEM_DECL_PLUGIN_API WINAPI\n"
            "#else\n"
            "#define NANOEM_DECL_PLUGIN_API\n"
            "#endif /* _WIN32 */\n"
            "\n"
            "#ifndef _RSIZE_T\n"
            "typedef size_t rsize_t;\n"
            "#endif /* _RSIZE_T */\n"
            "\n"
            "#ifdef __cplusplus\n"
            "extern \"C\" {\n"
            "#endif /* __cplusplus */\n"
            "\n");
        for (auto it : visitor.m_enumValues) {
            fprintf(fp, "enum {\n");
            for (auto it2 : it) {
                fprintf(fp, "    %s = %lld,\n", it2.first.c_str(), it2.second);
            }
            fprintf(fp, "};\n");
        }
        for (auto it : visitor.m_typedefNames) {
            fprintf(fp, "typedef %s %s;\n", it.first.c_str(), it.second.c_str());
        }
        fprintf(fp, "\n");
        for (auto it : visitor.m_functionPointers) {
            fprintf(fp, "typedef %s;\n", it.second.c_str());
            fprintf(fp, "extern PFN_%s %s;\n", it.first.c_str(), it.first.c_str());
        }
        fprintf(fp,
            "\n"
            "int\n"
            "nanoemApplicationPluginResolveAllFunctionAddresses(const char *executablePath);\n"
            "\n"
            "#ifdef __cplusplus\n"
            "}\n"
            "#endif /* __cplusplus */\n"
            "\n");
        fclose(fp);
        fp = fopen("./init.cc", "wb");
        fprintf(fp,
            "/*\n"
            "   Copyright (c) 2015-2021 hkrn All rights reserved\n"
            "\n"
            "   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md "
            "for more details.\n"
            "*/\n"
            "\n"
            "/* This file was automatically generated. */\n"
            "\n"
            "#include \"bx/os.h\"\n"
            "#include \"init.h\"\n"
            "\n");
        for (auto it : visitor.m_functionPointers) {
            fprintf(fp, "PFN_%s %s = NULL;\n", it.first.c_str(), it.first.c_str());
        }
        fprintf(fp,
            "\n"
            "int\n"
            "nanoemApplicationPluginResolveAllFunctionAddresses(const char *executablePath)\n"
            "{\n"
            "    int result = -1;\n"
            "    if (void *symbol = bx::dlopen(executablePath)) {\n");
        for (auto it : visitor.m_functionNames) {
            fprintf(fp,
                "        if ((%s = reinterpret_cast<PFN_%s>(bx::dlsym(symbol, \"%s\"))) == NULL) {\n"
                "            goto finally;\n"
                "        }\n",
                it.c_str(), it.c_str(), it.c_str());
        }
        fprintf(fp,
            "        result = 0;\n"
            "    finally:\n"
            "        bx::dlclose(symbol);\n"
            "    }\n"
            "    return result;\n"
            "}\n");
        fclose(fp);
        clang_disposeIndex(index);
    }
    return 0;
}
