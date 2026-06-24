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

#define kVecFactor 16
#define PI 3.14159265358979323846f
#define HALF_PI 1.57079632679489661923f
#define TWO_PI 6.28318530717958647692f

static aie::vector<cbfloat16, kVecFactor> oscillator;
static float last_frequency = -99.0f;
static float phase_step; 
static aie::vector<cbfloat16, kVecFactor> rotate_factor;

// Track the absolute starting phase for the current invocation
static float current_phase = 0.0f;
static float N_mul_phase_step = 0.0f;

static inline float wrap_phase(float phase) {
  while (phase > PI)
    phase -= TWO_PI;
  while (phase < -PI)
    phase += TWO_PI;
  return phase;
}

static inline float sin_minimax_3(float phase) {
  float x = wrap_phase(phase);
  if (x > HALF_PI)
    x = PI - x;
  else if (x < -HALF_PI)
    x = -PI - x;

  const float x2 = x * x;
  return x * (0.999770153f + x2 * (-0.165710071f + x2 * 0.007513054f));
}

static inline float cos_minimax_3(float phase) {
  return sin_minimax_3(HALF_PI - phase);
}

extern "C" {

void frequency_source(float *frequency, cbfloat16 *out, int32_t N) {
  cbfloat16 *__restrict pOut = out;

  // 1. Update frequency-dependent parameters if the frequency changed
  if (last_frequency != frequency[0]) {
    last_frequency = frequency[0];
    phase_step = TWO_PI * frequency[0];
    
    cbfloat16 next;
    next.real = (bfloat16)cos_minimax_3(kVecFactor * phase_step);
    next.imag = (bfloat16)sin_minimax_3(kVecFactor * phase_step);
    rotate_factor = aie::broadcast<cbfloat16, kVecFactor>(next);
    
    N_mul_phase_step = N * phase_step; //Assumes N is constant (TODO FIX)
    while (N_mul_phase_step >= TWO_PI) { 
      N_mul_phase_step  -= TWO_PI;
    }
  }
  
  // 2. Re-calculate the oscillator vector based on the current phase tracking
  cbfloat16 oscillator_init[kVecFactor];
  for (int i = 0; i < kVecFactor; i++) {
    float element_phase = current_phase + (i * phase_step);
    oscillator_init[i].real = (bfloat16)cos_minimax_3(element_phase);
    oscillator_init[i].imag = (bfloat16)sin_minimax_3(element_phase);
  }
  oscillator = aie::load_v<kVecFactor>(oscillator_init);
  
  // 3. Execute the streaming loop
  AIE_PREPARE_FOR_PIPELINING
  AIE_LOOP_MIN_ITERATION_COUNT(16)
  for (int32_t i = 0; i < N; i += kVecFactor) {
    // Direct store without conversion overhead
    aie::store_v(pOut, oscillator);
    pOut += kVecFactor;
         
    // Direct floating point vector multiplication
    oscillator = aie::mul(oscillator, rotate_factor);
  }

  // 4. Track the end-of-execution phase to ensure the next call starts seamlessly
  current_phase += N_mul_phase_step;
  if(current_phase >= TWO_PI) current_phase -= TWO_PI;
}
}
