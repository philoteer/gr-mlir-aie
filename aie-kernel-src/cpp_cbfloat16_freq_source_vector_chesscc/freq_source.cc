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

// Keep the scalar state between kernel invocations in float 
// to minimize magnitude drift across thousands of calls.
static float oscillator_real = 1.0f;
static float oscillator_imag = 0.0f;

extern "C" {

void frequency_source(float *frequency, cbfloat16 *out, int32_t N) {
  
  float phase_step = 6.28318530717958647692f * frequency[0];

  // 1. Precompute the phase shifts for a single 16-element base vector (W)
  cbfloat16 W_init[16];
  for (int i = 0; i < 16; i++) {
    W_init[i].real = (bfloat16)cosf(i * phase_step);
    W_init[i].imag = (bfloat16)sinf(i * phase_step);
  }
  aie::vector<cbfloat16, 16> W = aie::load_v<16>(W_init);

  // 2. Precompute the block rotation step to advance the state by 16 samples
  float block_phase_step = 16.0f * phase_step;
  cbfloat16 block_step = { (bfloat16)cosf(block_phase_step), (bfloat16)sinf(block_phase_step) };
  
  // Broadcast the step into a vector so we can do a vector-to-vector multiply
  aie::vector<cbfloat16, 16> step_v = aie::broadcast<cbfloat16, 16>(block_step);

  // 3. Initialize state vector from static storage
  cbfloat16 current_state = { (bfloat16)oscillator_real, (bfloat16)oscillator_imag };
  aie::vector<cbfloat16, 16> state_v = aie::broadcast<cbfloat16, 16>(current_state);

  // Use a restrict pointer to guarantee xchesscc pipelines memory accesses cleanly
  cbfloat16 * __restrict pOut = out;

  AIE_PREPARE_FOR_PIPELINING
  AIE_LOOP_MIN_ITERATION_COUNT(1) // Assuming N is at least 16
  for (int32_t i = 0; i < N; i += 16) {
    // Generate 16 output samples: W * state_v
    // aie::mul returns an accumulator, which we cast back to our vector type
    aie::vector<cbfloat16, 16> out_v = aie::mul(W, state_v).to_vector<cbfloat16>();

    // Store the resulting vector directly to memory
    aie::store_v(pOut, out_v);
    pOut += 16;

    // Rotate the state vector forward by the block step for the next loop iteration
    // This updates all 16 lanes simultaneously in the vector MAC units
    state_v = aie::mul(state_v, step_v).to_vector<cbfloat16>();
  }

  // Extract lane 0 to save the updated state for the next kernel invocation
  oscillator_real = (float)state_v[0].real;
  oscillator_imag = (float)state_v[0].imag;
}

} // extern "C"
