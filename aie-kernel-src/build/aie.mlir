module {
  aie.device(npu2) {
    %tile_0_2 = aie.tile(0, 2)
    %shim_noc_tile_0_0 = aie.tile(0, 0)
    aie.objectfifo @in(%shim_noc_tile_0_0, {%tile_0_2}, 2 : i32) : !aie.objectfifo<memref<4096xi8>> 
    aie.objectfifo @out(%tile_0_2, {%shim_noc_tile_0_0}, 2 : i32) : !aie.objectfifo<memref<4096xi8>> 
    func.func @passthrough_fn(%arg0: memref<4096xi8>, %arg1: memref<4096xi8>, %arg2: i32) {
      %c0 = arith.constant 0 : index
      %0 = arith.index_cast %arg2 : i32 to index
      %c1 = arith.constant 1 : index
      scf.for %arg3 = %c0 to %0 step %c1 {
        %1 = memref.load %arg0[%arg3] : memref<4096xi8>
        memref.store %1, %arg1[%arg3] : memref<4096xi8>
      }
      return
    }
    %core_0_2 = aie.core(%tile_0_2) {
      %c0 = arith.constant 0 : index
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c1 = arith.constant 1 : index
      scf.for %arg0 = %c0 to %c9223372036854775807 step %c1 {
        %0 = aie.objectfifo.acquire @out(Produce, 1) : !aie.objectfifosubview<memref<4096xi8>>
        %1 = aie.objectfifo.subview.access %0[0] : !aie.objectfifosubview<memref<4096xi8>> -> memref<4096xi8>
        %2 = aie.objectfifo.acquire @in(Consume, 1) : !aie.objectfifosubview<memref<4096xi8>>
        %3 = aie.objectfifo.subview.access %2[0] : !aie.objectfifosubview<memref<4096xi8>> -> memref<4096xi8>
        %c4096_i32 = arith.constant 4096 : i32
        func.call @passthrough_fn(%3, %1, %c4096_i32) : (memref<4096xi8>, memref<4096xi8>, i32) -> ()
        aie.objectfifo.release @in(Consume, 1)
        aie.objectfifo.release @out(Produce, 1)
      }
      aie.end
    }
    aie.runtime_sequence(%arg0: memref<16384xi8>, %arg1: memref<16384xi8>, %arg2: memref<16384xi8>) {
      %0 = aiex.dma_configure_task_for @in {
        aie.dma_bd(%arg0 : memref<16384xi8>, 0, 16384, [<size = 1, stride = 0>, <size = 1, stride = 0>, <size = 1, stride = 0>, <size = 16384, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      }
      aiex.dma_start_task(%0)
      %1 = aiex.dma_configure_task_for @out {
        aie.dma_bd(%arg1 : memref<16384xi8>, 0, 16384, [<size = 1, stride = 0>, <size = 1, stride = 0>, <size = 1, stride = 0>, <size = 16384, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {issue_token = true}
      aiex.dma_start_task(%1)
      aiex.dma_await_task(%1)
      aiex.dma_free_task(%0)
    }
  }
}

