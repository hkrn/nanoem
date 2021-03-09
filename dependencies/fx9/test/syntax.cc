#include "criterion/criterion.h"
#include "criterion/parameterized.h"

#include "fx9/Compiler.h"

using namespace fx9;

struct parameter_t {
    std::string path;
};

ParameterizedTestParameters(fx9, syntax)
{
    static parameter_t parameters[] = {
        { "add" },
        { "and" },
        { "div" },
        { "for" },
        { "function" },
        { "ge" },
        { "global_variables" },
        { "gt" },
        { "if" },
        { "le" },
        { "lt" },
        { "minus" },
        { "mul" },
        { "or" },
        { "struct" },
    };
    return criterion_test_params(parameters);
}

ParameterizedTest(parameter_t *parameter, fx9, syntax)
{
    Compiler::initialize();
    {
        Compiler::EffectProduct product;
        std::unique_ptr<Compiler> compiler(new Compiler(ECoreProfile, EShMsgDefault));
        std::string path(FX9_TEST_EFFECT_FIXTURES_PATH);
        path += "/syntax/" + parameter->path + ".fx";
        cr_assert(compiler->compile(path.c_str(), product));
    }
    Compiler::terminate();
}
