# Multi-kernel chain: three compute cores in series (cbfloat16)

This example wires **three AIE compute cores in series**, each running its own
kernel, and demonstrates the throughput loss ("bubbles") caused by not
effectively feeding the pipeline:

```
ShimDMA -> [core0: scale by 2+j] -> [core1: rotate 90deg] -> [core2: add 0.5+j0.25] -> ShimDMA
```

None of the stages is a passthrough - each does real vector math on
`cbfloat16` data - so any mis-wiring of the chain shows up immediately as
wrong output.

## Composite transfer function

With input sample `x`, the chained output is:

```
y = ((2 + j1) * x) * j + (0.5 + j0.25) = (-1 + j2)x + (0.5 + j0.25)
```

`test.py` verifies the AIE output against exactly this numpy model
(stage order matters - swapping two cores fails the check).

## The bubble knob: inter-core ObjectFifo depth

The inter-stage object fifos are single-buffered (`depth=1`) by default.
While core N is computing on a buffer it also blocks core N-1 from filling
the next buffer, so all three stages serialize per tile and idle gaps
(bubbles) propagate down the chain:

- `FIFO_DEPTH=1` : every tile pays `t_stage0 + t_stage1 + t_stage2`
  (serialized handoff).
- `FIFO_DEPTH=2` (or more): double buffering lets core N-1 fill the next
  buffer while core N still computes on the current one; steady-state
  throughput becomes limited by the *slowest* stage only.

Build and compare on hardware:

```sh
make FIFO_DEPTH=1 && make test   # bubbled pipeline
make FIFO_DEPTH=2 && make test   # properly fed pipeline
```

`make test` prints the median run time over repeated invocations; the depth=2
build should be noticeably faster end-to-end. For a bigger effect, replace the
stage kernels with heavier per-tile math so that compute dominates DMA time.

## Files

| file                | purpose                                              |
|---------------------|------------------------------------------------------|
| `chain_kernels.cc`  | the three stage kernels (complex mul / rotate / add) |
| `aie2.py`           | iron design: 3 workers chained via ObjectFifos       |
| `test.py`           | host-side correctness check + timing                 |
| `Makefile`          | chesscc-based build (`npu` default, `NPU2=1` for npu2)|

## GNU Radio usage

Build with `make`, then use the generic `mlir_aie_cpp_bfloat16` GRC block with
`path_xclbin = .../cpp_cbfloat16_multikernel_chain_chesscc/build/final.xclbin`,
`path_insts_bin = .../build/insts.bin`, kernel name `MLIR_AIE` and
`VECTOR_SIZE = 8192` (4096 complex samples x 2 components). Wrap it with the
`complex64_to_cbfloat` / `bfloat16_to_float32` converter blocks like the other
cbfloat16 examples in this repository.
