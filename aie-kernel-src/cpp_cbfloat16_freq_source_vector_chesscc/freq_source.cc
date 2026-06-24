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
#define kQShift 15
#define kQScale 32768.0f
#define kPhaseScale 4294967296.0f
#define OSCILLATOR_RESET_CNT 4

alignas(64) static aie::vector<cint16, kVecFactor> oscillator;
static float last_frequency = -99.0f;
static float phase_step; 
static uint32_t phase_step_q;
alignas(64) static aie::vector<cint16, kVecFactor> rotate_factor;
// One full turn is 2^32, so unsigned overflow wraps phase modulo 2*pi.
static uint32_t current_phase = 0;
static int osc_cnt = OSCILLATOR_RESET_CNT+1;

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

static inline uint32_t frequency_to_phase_step_q(float frequency) {
  float cycles = frequency - floorf(frequency);
  return (uint32_t)(cycles * kPhaseScale);
}

static inline float phase_q_to_radians(uint32_t phase) {
  float centered = phase < 0x80000000u ? (float)phase : (float)phase - kPhaseScale;
  return centered * (TWO_PI / kPhaseScale);
}

static inline int16_t float_to_q15(float x) {
  float scaled = x * kQScale;
  if (scaled > 32767.0f)
    return 32767;
  if (scaled < -32768.0f)
    return -32768;
  return (int16_t)(scaled >= 0.0f ? scaled + 0.5f : scaled - 0.5f);
}

static inline cint16 make_q15(float real, float imag) {
  cint16 value;
  value.real = float_to_q15(real);
  value.imag = float_to_q15(imag);
  return value;
}

extern "C" {

void frequency_source(float *frequency, cbfloat16 *out, int32_t N) {
  cbfloat16 *__restrict pOut = out;

  // 1. Update frequency-dependent parameters if the frequency changed
  if (last_frequency != frequency[0]) {
    last_frequency = frequency[0];
    phase_step = TWO_PI * frequency[0];
    phase_step_q = frequency_to_phase_step_q(frequency[0]);
    
    cint16 next = make_q15(cos_minimax_3(kVecFactor * phase_step),
                           sin_minimax_3(kVecFactor * phase_step));
    rotate_factor = aie::broadcast<cint16, kVecFactor>(next);
  }
  
  // 2. Re-calculate the oscillator vector based on the current phase tracking
  if(osc_cnt >= OSCILLATOR_RESET_CNT)
  {
    alignas(32) cint16 oscillator_init[kVecFactor];
    for (int i = 0; i < kVecFactor; i++) {
      float element_phase = phase_q_to_radians(current_phase + (uint32_t)i * phase_step_q);
      oscillator_init[i] = make_q15(cos_minimax_3(element_phase),
                                    sin_minimax_3(element_phase));
    }
    oscillator = aie::load_v<kVecFactor>(oscillator_init);
    osc_cnt = 0;
  }
  
  // 3. Execute the streaming loop
  AIE_PREPARE_FOR_PIPELINING
  AIE_LOOP_MIN_ITERATION_COUNT(16)
  for (int32_t i = 0; i < N; i += kVecFactor) {
  // Step A: Vectorized conversion from cint16 to cbfloat16
    aie::vector<cbfloat16, kVecFactor> out_vec = aie::to_float<cbfloat16>(oscillator, kQShift);
    
    // Step B: Direct vector store to the output pointer
    aie::store_v(pOut, out_vec);
    pOut += kVecFactor;
          
    // Step C: Update oscillator for the next iteration
    aie::accum<cacc48, kVecFactor> next_osc = aie::mul(oscillator, rotate_factor);
    oscillator = next_osc.template to_vector<cint16>(kQShift);
  }

  // 4. Track the end-of-execution phase to ensure the next call starts seamlessly
  current_phase += (uint32_t)N * phase_step_q;
  osc_cnt++;
}
}
