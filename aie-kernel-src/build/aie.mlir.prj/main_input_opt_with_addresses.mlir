module attributes {llvm.target_triple = "aie2p"} {
  llvm.mlir.global external @in_cons_buff_1() {addr_space = 0 : i32} : !llvm.array<4096 x i8>
  llvm.mlir.global external @in_cons_buff_0() {addr_space = 0 : i32} : !llvm.array<4096 x i8>
  llvm.mlir.global external @out_buff_1() {addr_space = 0 : i32} : !llvm.array<4096 x i8>
  llvm.mlir.global external @out_buff_0() {addr_space = 0 : i32} : !llvm.array<4096 x i8>
  llvm.func @debug_i32(i32) attributes {sym_visibility = "private"}
  llvm.func @llvm.aie2p.event(i32) attributes {sym_visibility = "private"}
  llvm.func @llvm.aie2p.put.ms(i32, i32) attributes {sym_visibility = "private"}
  llvm.func @llvm.aie2p.get.ss() -> !llvm.struct<(i32, i32)> attributes {sym_visibility = "private"}
  llvm.func @llvm.aie2p.mcd.write.vec(vector<16xi32>, i32) attributes {sym_visibility = "private"}
  llvm.func @llvm.aie2p.scd.read.vec(i32) -> vector<16xi32> attributes {sym_visibility = "private"}
  llvm.func @llvm.aie2p.acquire(i32, i32) attributes {sym_visibility = "private"}
  llvm.func @llvm.aie2p.release(i32, i32) attributes {sym_visibility = "private"}
  llvm.func @passthrough_fn(%arg0: !llvm.ptr, %arg1: !llvm.ptr, %arg2: i32) {
    %0 = llvm.mlir.constant(1 : index) : i64
    %1 = llvm.mlir.constant(0 : index) : i64
    %2 = llvm.sext %arg2 : i32 to i64
    llvm.br ^bb1(%1 : i64)
  ^bb1(%3: i64):  // 2 preds: ^bb0, ^bb2
    %4 = llvm.icmp "slt" %3, %2 : i64
    llvm.cond_br %4, ^bb2, ^bb3
  ^bb2:  // pred: ^bb1
    %5 = llvm.getelementptr inbounds|nuw %arg0[%3] : (!llvm.ptr, i64) -> !llvm.ptr, i8
    %6 = llvm.load %5 : !llvm.ptr -> i8
    %7 = llvm.getelementptr inbounds|nuw %arg1[%3] : (!llvm.ptr, i64) -> !llvm.ptr, i8
    llvm.store %6, %7 : i8, !llvm.ptr
    %8 = llvm.add %3, %0 : i64
    llvm.br ^bb1(%8 : i64)
  ^bb3:  // pred: ^bb1
    llvm.return
  }
  llvm.func @core_0_2() {
    %0 = llvm.mlir.addressof @in_cons_buff_1 : !llvm.ptr
    %1 = llvm.mlir.addressof @out_buff_1 : !llvm.ptr
    %2 = llvm.mlir.addressof @in_cons_buff_0 : !llvm.ptr
    %3 = llvm.mlir.addressof @out_buff_0 : !llvm.ptr
    %4 = llvm.mlir.constant(51 : i32) : i32
    %5 = llvm.mlir.constant(48 : i32) : i32
    %6 = llvm.mlir.constant(49 : i32) : i32
    %7 = llvm.mlir.constant(50 : i32) : i32
    %8 = llvm.mlir.constant(1 : i32) : i32
    %9 = llvm.mlir.constant(-1 : i32) : i32
    %10 = llvm.mlir.constant(4096 : i32) : i32
    %11 = llvm.mlir.constant(0 : index) : i64
    %12 = llvm.mlir.constant(9223372036854775806 : index) : i64
    %13 = llvm.mlir.constant(2 : index) : i64
    llvm.br ^bb1(%11 : i64)
  ^bb1(%14: i64):  // 2 preds: ^bb0, ^bb2
    %15 = llvm.icmp "slt" %14, %12 : i64
    llvm.cond_br %15, ^bb2, ^bb3
  ^bb2:  // pred: ^bb1
    llvm.call @llvm.aie2p.acquire(%7, %9) : (i32, i32) -> ()
    llvm.call @llvm.aie2p.acquire(%6, %9) : (i32, i32) -> ()
    %16 = llvm.getelementptr %3[0, 0] : (!llvm.ptr) -> !llvm.ptr, !llvm.array<4096 x i8>
    %17 = llvm.getelementptr %2[0, 0] : (!llvm.ptr) -> !llvm.ptr, !llvm.array<4096 x i8>
    llvm.call @passthrough_fn(%17, %16, %10) : (!llvm.ptr, !llvm.ptr, i32) -> ()
    llvm.call @llvm.aie2p.release(%5, %8) : (i32, i32) -> ()
    llvm.call @llvm.aie2p.release(%4, %8) : (i32, i32) -> ()
    llvm.call @llvm.aie2p.acquire(%7, %9) : (i32, i32) -> ()
    llvm.call @llvm.aie2p.acquire(%6, %9) : (i32, i32) -> ()
    %18 = llvm.getelementptr %1[0, 0] : (!llvm.ptr) -> !llvm.ptr, !llvm.array<4096 x i8>
    %19 = llvm.getelementptr %0[0, 0] : (!llvm.ptr) -> !llvm.ptr, !llvm.array<4096 x i8>
    llvm.call @passthrough_fn(%19, %18, %10) : (!llvm.ptr, !llvm.ptr, i32) -> ()
    llvm.call @llvm.aie2p.release(%5, %8) : (i32, i32) -> ()
    llvm.call @llvm.aie2p.release(%4, %8) : (i32, i32) -> ()
    %20 = llvm.add %14, %13 : i64
    llvm.br ^bb1(%20 : i64)
  ^bb3:  // pred: ^bb1
    llvm.call @llvm.aie2p.acquire(%7, %9) : (i32, i32) -> ()
    llvm.call @llvm.aie2p.acquire(%6, %9) : (i32, i32) -> ()
    %21 = llvm.getelementptr %3[0, 0] : (!llvm.ptr) -> !llvm.ptr, !llvm.array<4096 x i8>
    %22 = llvm.getelementptr %2[0, 0] : (!llvm.ptr) -> !llvm.ptr, !llvm.array<4096 x i8>
    llvm.call @passthrough_fn(%22, %21, %10) : (!llvm.ptr, !llvm.ptr, i32) -> ()
    llvm.call @llvm.aie2p.release(%5, %8) : (i32, i32) -> ()
    llvm.call @llvm.aie2p.release(%4, %8) : (i32, i32) -> ()
    llvm.return
  }
}
