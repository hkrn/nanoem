#include "dawn/dawn_proc.h"
#include "dawn/native/DawnNative.h"

extern "C" void sgx_dawn_setup()
{
    dawnProcSetProcs(&dawn::native::GetProcs());
}
