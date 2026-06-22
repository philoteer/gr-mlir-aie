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
#define kQScale 32767.0f
#define kQShift 15

static aie::vector<cint16, kVecFactor> oscillator;
static float last_frequency = -99.0f;
static float phase_step; 
static aie::vector<cint16, kVecFactor> rotate_factor;

static int16_t float_to_q15(float x) {
  float scaled = x * kQScale;
  scaled += scaled >= 0.0f ? 0.5f : -0.5f;

  if (scaled > 32767.0f)
    return 32767;
  if (scaled < -32768.0f)
    return -32768;
  return (int16_t)scaled;
}

extern "C" {

void frequency_source(float *frequency, cbfloat16 *out, int32_t N) {
  cbfloat16 *__restrict pOut = out;

  if(last_frequency != frequency[0]){
    last_frequency = frequency[0];
    
    //step size
    phase_step = 6.28318530717958647692f * frequency[0];
    
    //oscillator
    cint16 oscillator_init[kVecFactor];
    for (int i = 0; i < kVecFactor; i++) {
      oscillator_init[i].real = float_to_q15(cosf(i * phase_step));
      oscillator_init[i].imag = float_to_q15(sinf(i * phase_step));
    }
    
    oscillator = aie::load_v<kVecFactor>(oscillator_init);
    
    cint16 next;
    next.real = float_to_q15(cosf(kVecFactor * phase_step));
    next.imag = float_to_q15(sinf(kVecFactor * phase_step));
    rotate_factor = aie::broadcast<cint16, kVecFactor>(next);
  }
  
  AIE_PREPARE_FOR_PIPELINING
  AIE_LOOP_MIN_ITERATION_COUNT(16)
  for (int32_t i = 0; i < N; i += kVecFactor) {
    aie::store_v(pOut, aie::to_float<cbfloat16>(oscillator, kQShift));
    pOut += kVecFactor;
         
    aie::accum<cacc48, kVecFactor> tick = aie::mul(oscillator, rotate_factor);
    oscillator = tick.to_vector<cint16>(kQShift);
    
  }
}

} // extern "C"
