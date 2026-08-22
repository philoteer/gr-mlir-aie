//===- chain_kernels.cc -----------------------------------*- C++ -*-===//
//
// This file is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Copyright (C) 2023, Advanced Micro Devices, Inc.
//
//===----------------------------------------------------------------------===//

// Three independent compute kernels, one per AIE core, meant to be chained in
// series (core -> core -> core) by aie2.py. Each stage performs real vector
// math on cbfloat16 data so that a mis-wired pipeline is easy to catch:
//
//   stage_scale_mul : c = fac1 * a          with fac1 = 2 + j1
//   stage_rotate90  : c = fac2 * a          with fac2 = j
//   stage_add_const : c = a + off           with off  = 0.5 + j0.25
//
// Composite transfer function of the 3-stage chain:
//   y = ((2 + j1)x)*j + (0.5 + j0.25)
//     = (-1 + j2)x + (0.5 + j0.25)

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../aie_kernel_utils.h"
#include <aie_api/aie.hpp>
#include <aie_api/aie_types.hpp>

extern "C" {

// Stage 1: complex scalar multiply, c = (2 + j1) * a
void stage_scale_mul(cbfloat16 *a, cbfloat16 *c, int32_t N) {
  constexpr int vec_factor = 16;
  cbfloat16 *__restrict pA1 = a;
  cbfloat16 *__restrict pC1 = c;
  const int F = N / vec_factor;

  cbfloat16 fac;
  fac.real = (bfloat16)2.0f;
  fac.imag = (bfloat16)1.0f;

  AIE_PREPARE_FOR_PIPELINING
  AIE_LOOP_MIN_ITERATION_COUNT(16)
  for (int i = 0; i < F; i++) {
    aie::vector<cbfloat16, vec_factor> A0 = aie::load_v<vec_factor>(pA1);
    pA1 += vec_factor;
    aie::accum<caccfloat, vec_factor> cout = aie::mul(A0, fac);
    aie::store_v(pC1, cout.template to_vector<cbfloat16>(0));
    pC1 += vec_factor;
  }
}

// Stage 2: rotate by +90 degrees, c = j * a  (re' = -im, im' = re)
void stage_rotate90(cbfloat16 *a, cbfloat16 *c, int32_t N) {
  constexpr int vec_factor = 16;
  cbfloat16 *__restrict pA1 = a;
  cbfloat16 *__restrict pC1 = c;
  const int F = N / vec_factor;

  cbfloat16 fac;
  fac.real = (bfloat16)0.0f;
  fac.imag = (bfloat16)1.0f;

  AIE_PREPARE_FOR_PIPELINING
  AIE_LOOP_MIN_ITERATION_COUNT(16)
  for (int i = 0; i < F; i++) {
    aie::vector<cbfloat16, vec_factor> A0 = aie::load_v<vec_factor>(pA1);
    pA1 += vec_factor;
    aie::accum<caccfloat, vec_factor> cout = aie::mul(A0, fac);
    aie::store_v(pC1, cout.template to_vector<cbfloat16>(0));
    pC1 += vec_factor;
  }
}

// Stage 3: add a complex constant, c = a + (0.5 + j0.25)
void stage_add_const(cbfloat16 *a, cbfloat16 *c, int32_t N) {
  constexpr int vec_factor = 16;
  cbfloat16 *__restrict pA1 = a;
  cbfloat16 *__restrict pC1 = c;
  const int F = N / vec_factor;

  cbfloat16 off;
  off.real = (bfloat16)0.5f;
  off.imag = (bfloat16)0.25f;

  AIE_PREPARE_FOR_PIPELINING
  AIE_LOOP_MIN_ITERATION_COUNT(16)
  for (int i = 0; i < F; i++) {
    aie::vector<cbfloat16, vec_factor> A0 = aie::load_v<vec_factor>(pA1);
    pA1 += vec_factor;
    aie::vector<cbfloat16, vec_factor> C0 = aie::add(A0, off);
    aie::store_v(pC1, C0);
    pC1 += vec_factor;
  }
}

} // extern "C"
