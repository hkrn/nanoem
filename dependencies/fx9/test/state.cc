#include "criterion/criterion.h"

#include "fx9/Compiler.h"

using namespace fx9;

Test(fx9, state)
{
    Compiler::initialize();
    {
        Compiler::EffectProduct product;
        std::unique_ptr<Compiler> compiler(new Compiler(ECoreProfile, EShMsgDefault));
        cr_assert(compiler->compile(FX9_TEST_EFFECT_FIXTURES_PATH "/state.fx", product));
    }
    Compiler::terminate();
}
