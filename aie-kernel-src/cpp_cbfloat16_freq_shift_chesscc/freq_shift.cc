//===- freq_shift.cc --------------------------------------------*- C++ -*-===//
#include <stdint.h>

#include "../aie_kernel_utils.h"
#include <aie_api/aie.hpp>
#include <aie_api/aie_types.hpp>
#include "lut.h" //alignas(32) const cbfloat16 freq_shift_lut[LUT_SIZE in #DEFINE]

#DEFINE kVecFactor 16

namespace {

const cbfloat16 kBlockStep = {(bfloat16)0.555570233f,
                              (bfloat16)0.831469612f};

} // namespace

extern "C" {

void frequency_shift(cbfloat16 *a, cbfloat16 *c, int32_t N) {
    //TODO Implement
}

} // extern "C"
