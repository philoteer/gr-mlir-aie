//===- freq_shift.cc --------------------------------------------*- C++ -*-===//
#include <stdint.h>

#include "../aie_kernel_utils.h"
#include <aie_api/aie.hpp>
#include <aie_api/aie_types.hpp>
#include "lut.h"

// Note: Corrected from #DEFINE to #define
#define kVecFactor 16

namespace {

extern "C" {

void frequency_shift(cbfloat16 * __restrict a, cbfloat16 * __restrict c, int32_t N) {
    int lut_idx = 0;

    // AIE loops perform best when unrolled or explicitly pipelined.
    // chess_prepare_for_pipelining tells the compiler to optimize the loop schedule.
    AIE_PREPARE_FOR_PIPELINING
    AIE_LOOP_MIN_ITERATION_COUNT(16)
    for (int i = 0; i < N; i += kVecFactor) {
        
        // 1. Load 16 elements from the input array
        aie::vector<cbfloat16, kVecFactor> in_vec = aie::load_v<kVecFactor>(a + i);

        // 2. Load 16 elements from our pre-computed e^(it) LUT
        aie::vector<cbfloat16, kVecFactor> lut_vec = aie::load_v<kVecFactor>(freq_shift_lut + lut_idx);

        // 3. Multiply input by the Complex Conjugate of the LUT vector.
        // aie::conj(lut_vec) converts e^(it) into e^(-it). The AIE compiler is 
        // smart enough to fold this directly into the MAC/MUL instruction modifiers.
        aie::vector<cbfloat16, kVecFactor> out_vec = aie::mul(in_vec, aie::conj(lut_vec));

        // 4. Store the rotated vector into the output array
        aie::store_v(c + i, out_vec);

        // 5. Advance LUT pointer and wrap around at 512.
        // We use bitwise AND (512 - 1 = 511) which is much faster than modulo.
        lut_idx = (lut_idx + kVecFactor) & (LUT_SIZE - 1);
    }
}

} // extern "C"

} // namespace
