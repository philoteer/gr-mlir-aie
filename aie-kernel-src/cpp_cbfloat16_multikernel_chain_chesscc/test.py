#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# Host-side correctness + timing test for the multi-kernel chain example.
#
# Feeds a random cbfloat16 tensor through the 3-core chain
#   y = ((2 + j1)x)*j + (0.5 + j0.25) = (-1 + j2)x + (0.5 + j0.25)
# and checks the AIE output against a numpy reference.
#
# Also reports wall-clock time per run so that builds with different inter-core
# fifo depths can be compared:
#   make FIFO_DEPTH=1 && make test   # single-buffered: stages serialize, bubbles
#   make FIFO_DEPTH=2 && make test   # double-buffered: stages overlap

import os
import sys
import time

import ml_dtypes
import numpy as np

from aie.utils import NPUKernel, tensor

TENSOR_SIZE = 4096 # complex samples per tensor (matches aie2.py)
VEC_SIZE = TENSOR_SIZE * 2 # bf16 elements, real/imag interleaved


def complex_to_cbfloat16(x):
    """Interleave re/im and round each component to bfloat16."""
    out = np.empty(VEC_SIZE, dtype=ml_dtypes.bfloat16)
    out[0::2] = np.real(x).astype(ml_dtypes.bfloat16)
    out[1::2] = np.imag(x).astype(ml_dtypes.bfloat16)
    return out


def cbfloat16_to_complex(b):
    f = np.asarray(b, dtype=np.float32)
    return f[0::2] + 1j * f[1::2]


def reference(x):
    """Numpy model of the full chain (stage order matters!)."""
    y = x * (2 + 1j) # stage_scale_mul
    y = y * 1j       # stage_rotate90
    y = y + (0.5 + 0.25j) # stage_add_const
    return y


def main():
    here = os.path.dirname(os.path.realpath(__file__))
    xclbin = os.path.join(here, "build", "final.xclbin")
    insts = os.path.join(here, "build", "insts.bin")
    if not (os.path.exists(xclbin) and os.path.exists(insts)):
        sys.exit("build/final.xclbin or build/insts.bin missing - run make first")

    iters = int(sys.argv[1]) if len(sys.argv) > 1 else 20

    rng = np.random.default_rng(0)
    x = rng.uniform(-1.0, 1.0, TENSOR_SIZE) + 1j * rng.uniform(-1.0, 1.0, TENSOR_SIZE)

    in_np = complex_to_cbfloat16(x)
    out_np = np.zeros(VEC_SIZE, dtype=ml_dtypes.bfloat16)

    in_t = tensor(in_np, dtype=ml_dtypes.bfloat16)
    out_t = tensor(out_np, dtype=ml_dtypes.bfloat16)

    k = NPUKernel(xclbin, insts, kernel_name="MLIR_AIE")

    # Warm-up / correctness run
    k(in_t, out_t)
    got = cbfloat16_to_complex(np.array(out_t))
    exp = reference(cbfloat16_to_complex(in_np))

    err = np.max(np.abs(got - exp))
    tol = 0.02 # loose enough for 3 stages of bfloat16 rounding
    print(f"max |out - ref| = {err:.5f} (tolerance {tol})")
    if err > tol:
        sys.exit(f"FAILED: output does not match chained reference "
                 f"y = (-1 + j2)x + (0.5 + j0.25)")
    print("PASS: output matches chained reference")

    # Timing runs
    times = []
    for _ in range(iters):
        t0 = time.perf_counter()
        k(in_t, out_t)
        _ = np.array(out_t) # wait for drain
        times.append(time.perf_counter() - t0)
    med_ms = np.median(times) * 1e3
    print(f"median over {iters} runs: {med_ms:.3f} ms/run "
          f"({TENSOR_SIZE / (med_ms * 1e-3) / 1e6:.3f} Msps)")


if __name__ == "__main__":
    main()
