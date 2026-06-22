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

static aie::vector<cbfloat16, kVecFactor> oscillator;
static float last_frequency = -99.0f;
static float phase_step; 
static aie::vector<cbfloat16, kVecFactor> rotate_factor;

extern "C" {

void frequency_source(float *frequency, cbfloat16 *out, int32_t N) {
  cbfloat16 *__restrict pOut = out;

  if(last_frequency != frequency[0]){
    last_frequency = frequency[0];
    
    //step size
    phase_step = 6.28318530717958647692f * frequency[0];
    
    //oscillator
    cbfloat16 oscillator_init[kVecFactor];
    for (int i = 0; i < kVecFactor; i++) {
      oscillator_init[i].real = (bfloat16)cosf(i * phase_step);
      oscillator_init[i].imag = (bfloat16)sinf(i * phase_step);
    }
    
    oscillator = aie::load_v<kVecFactor>(oscillator_init);
    
    cbfloat16 next;
    next.real = (bfloat16)cosf(kVecFactor * phase_step);
    next.imag = (bfloat16)sinf(kVecFactor * phase_step);
    rotate_factor = aie::broadcast<cbfloat16, kVecFactor>(next);
  }
  
  AIE_PREPARE_FOR_PIPELINING
  AIE_LOOP_MIN_ITERATION_COUNT(16)
  for (int32_t i = 0; i < N; i += kVecFactor) {
    aie::store_v(pOut, oscillator);
    pOut += kVecFactor;
        
    aie::accum<caccfloat, kVecFactor> tick = aie::mul(oscillator, rotate_factor);
    oscillator = tick.to_vector<cbfloat16>(0);
    
  }
}

} // extern "C"
