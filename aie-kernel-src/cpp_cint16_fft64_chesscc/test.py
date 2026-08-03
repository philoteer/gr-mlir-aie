#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

import argparse
from pathlib import Path

import aie.iron as iron
import numpy as np
from aie.utils import DefaultNPURuntime, NPUKernel


FFT_SIZE = 64
SAMPLE_COUNT = 4096


def make_input():
    frames = np.zeros((SAMPLE_COUNT // FFT_SIZE, FFT_SIZE), dtype=np.complex128)
    frames[0, 0] = 12000
    frames[1, :] = 400
    frames[2, :] = np.rint(
        10000 * np.exp(2j * np.pi * 7 * np.arange(FFT_SIZE) / FFT_SIZE)
    ).real + 1j * np.rint(
        10000 * np.exp(2j * np.pi * 7 * np.arange(FFT_SIZE) / FFT_SIZE)
    ).imag

    rng = np.random.default_rng(0)
    frames[3:, :] = rng.integers(-1000, 1001, size=frames[3:, :].shape) + 1j * rng.integers(
        -1000, 1001, size=frames[3:, :].shape
    )

    interleaved = np.empty(SAMPLE_COUNT * 2, dtype=np.int16)
    interleaved[0::2] = frames.real.astype(np.int16).ravel()
    interleaved[1::2] = frames.imag.astype(np.int16).ravel()
    return frames, interleaved


def main():
    directory = Path(__file__).resolve().parent
    parser = argparse.ArgumentParser(description="Test the shifted 64-point AIE FFT")
    parser.add_argument("--xclbin", type=Path, default=directory / "build/final.xclbin")
    parser.add_argument("--insts", type=Path, default=directory / "build/insts.bin")
    args = parser.parse_args()

    input_frames, interleaved = make_input()
    output = iron.zeros(SAMPLE_COUNT * 2, dtype=np.int32)
    kernel = NPUKernel(str(args.xclbin), str(args.insts), kernel_name="MLIR_AIE")
    handle = DefaultNPURuntime.load(kernel)
    DefaultNPURuntime.run(handle, [iron.tensor(interleaved, dtype=np.int16), output])

    raw = output.numpy().reshape(-1, 2)
    actual = (raw[:, 0].astype(np.int64) + 1j * raw[:, 1].astype(np.int64)).reshape(
        -1, FFT_SIZE
    )
    expected = np.fft.fftshift(np.fft.fft(input_frames, axis=1), axes=1)
    # Q15 twiddle quantization accumulates a small absolute error at large bins.
    np.testing.assert_allclose(actual, expected, rtol=0, atol=48)
    print("PASS: all shifted 64-point FFT frames match NumPy")


if __name__ == "__main__":
    main()
