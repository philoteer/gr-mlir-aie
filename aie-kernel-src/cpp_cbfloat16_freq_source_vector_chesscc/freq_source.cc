//===- freq_source.cc -------------------------------------------*- C++ -*-===//
//
// This file is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include <math.h>
#include <stdint.h>

#include "../aie_kernel_utils.h"
#include <aie_api/aie.hpp>
#include <aie_api/aie_types.hpp>

namespace {

constexpr int kVecFactor = 16;

} // namespace

// Keep the scalar state between kernel invocations in float
// to minimize magnitude drift across thousands of calls.
static float oscillator_real = 1.0f;
static float oscillator_imag = 0.0f;

extern "C" {

void frequency_source(float *frequency, cbfloat16 *out, int32_t N) {

  const float phase_step = 6.28318530717958647692f * frequency[0];

  // Precompute the phase shifts for a single 16-element base vector.
  cbfloat16 W_init[kVecFactor];
  for (int i = 0; i < kVecFactor; i++) {
    W_init[i].real = (bfloat16)cosf(i * phase_step);
    W_init[i].imag = (bfloat16)sinf(i * phase_step);
  }
  aie::vector<cbfloat16, kVecFactor> W = aie::load_v<kVecFactor>(W_init);

  // Advance the persisted oscillator by one full vector per loop.
  const float block_phase_step = kVecFactor * phase_step;
  const float block_step_real = cosf(block_phase_step);
  const float block_step_imag = sinf(block_phase_step);

  cbfloat16 *__restrict pOut = out;

  AIE_PREPARE_FOR_PIPELINING
  AIE_LOOP_MIN_ITERATION_COUNT(1)
  for (int32_t i = 0; i < N; i += kVecFactor) {
    const cbfloat16 current_state = {(bfloat16)oscillator_real,
                                     (bfloat16)oscillator_imag};
    aie::vector<cbfloat16, kVecFactor> state_v =
        aie::broadcast<cbfloat16, kVecFactor>(current_state);

    aie::accum<caccfloat, kVecFactor> out_v = aie::mul(W, state_v);
    aie::store_v(pOut, out_v.template to_vector<cbfloat16>(0));
    pOut += kVecFactor;

    const float next_real =
        oscillator_real * block_step_real - oscillator_imag * block_step_imag;
    const float next_imag =
        oscillator_real * block_step_imag + oscillator_imag * block_step_real;
    oscillator_real = next_real;
    oscillator_imag = next_imag;
  }
}

} // extern "C"
