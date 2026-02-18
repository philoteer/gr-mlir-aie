; ModuleID = '/media/piloteer/SAMSUNG_EXT/XDNA/gr-mlir-aie/aie-kernel-src/build/aie.mlir.prj/main_input.llpeanohack.ll'
source_filename = "LLVMDialectModule"
target datalayout = "e-m:e-p:20:32-i1:8:32-i8:8:32-i16:16:32-i32:32:32-f32:32:32-i64:32-f64:32-a:0:32-n32"
target triple = "aie2p"

@in_cons_buff_1 = external local_unnamed_addr global [4096 x i8]
@in_cons_buff_0 = external local_unnamed_addr global [4096 x i8]
@out_buff_1 = external local_unnamed_addr global [4096 x i8]
@out_buff_0 = external local_unnamed_addr global [4096 x i8]

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.aie2p.acquire(i32, i32) #0

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.aie2p.release(i32, i32) #0

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite)
define void @passthrough_fn(ptr nocapture readonly %0, ptr nocapture writeonly %1, i32 %2) local_unnamed_addr #1 {
  %4 = sext i32 %2 to i64
  %5 = icmp sgt i32 %2, 0
  br i1 %5, label %.lr.ph.preheader, label %._crit_edge

.lr.ph.preheader:                                 ; preds = %3
  %6 = ptrtoint ptr %1 to i20
  %7 = ptrtoint ptr %0 to i20
  %min.iters.check = icmp ult i32 %2, 4
  %8 = sub i20 %6, %7
  %diff.check = icmp ult i20 %8, 4
  %or.cond = select i1 %min.iters.check, i1 true, i1 %diff.check
  br i1 %or.cond, label %.lr.ph.preheader1, label %vector.ph

vector.ph:                                        ; preds = %.lr.ph.preheader
  %n.vec = and i64 %4, 2147483644
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %vector.ph
  %index = phi i64 [ 0, %vector.ph ], [ %index.next, %vector.body ]
  %9 = trunc i64 %index to i20
  %10 = getelementptr inbounds i8, ptr %0, i20 %9
  %wide.load = load <4 x i8>, ptr %10, align 1
  %11 = getelementptr inbounds i8, ptr %1, i20 %9
  store <4 x i8> %wide.load, ptr %11, align 1
  %index.next = add nuw i64 %index, 4
  %12 = icmp eq i64 %index.next, %n.vec
  br i1 %12, label %middle.block, label %vector.body, !llvm.loop !1

middle.block:                                     ; preds = %vector.body
  %cmp.n = icmp eq i64 %n.vec, %4
  br i1 %cmp.n, label %._crit_edge, label %.lr.ph.preheader1

.lr.ph.preheader1:                                ; preds = %.lr.ph.preheader, %middle.block
  %.ph = phi i64 [ 0, %.lr.ph.preheader ], [ %n.vec, %middle.block ]
  br label %.lr.ph

.lr.ph:                                           ; preds = %.lr.ph.preheader1, %.lr.ph
  %13 = phi i64 [ %18, %.lr.ph ], [ %.ph, %.lr.ph.preheader1 ]
  %14 = trunc i64 %13 to i20
  %15 = getelementptr inbounds i8, ptr %0, i20 %14
  %16 = load i8, ptr %15, align 1
  %17 = getelementptr inbounds i8, ptr %1, i20 %14
  store i8 %16, ptr %17, align 1
  %18 = add nuw nsw i64 %13, 1
  %19 = icmp slt i64 %18, %4
  br i1 %19, label %.lr.ph, label %._crit_edge, !llvm.loop !4

._crit_edge:                                      ; preds = %.lr.ph, %middle.block, %3
  ret void
}

; Function Attrs: nofree norecurse nosync nounwind
define void @core_0_2() local_unnamed_addr #2 {
  br label %passthrough_fn.exit

passthrough_fn.exit:                              ; preds = %0, %passthrough_fn.exit
  %1 = phi i64 [ 0, %0 ], [ %2, %passthrough_fn.exit ]
  tail call void @llvm.aie2p.acquire(i32 50, i32 -1)
  tail call void @llvm.aie2p.acquire(i32 49, i32 -1)
  tail call void @llvm.memcpy.p0.p0.i20(ptr noundef nonnull align 1 dereferenceable(4096) @out_buff_0, ptr noundef nonnull align 1 dereferenceable(4096) @in_cons_buff_0, i20 4096, i1 false)
  tail call void @llvm.aie2p.release(i32 48, i32 1)
  tail call void @llvm.aie2p.release(i32 51, i32 1)
  tail call void @llvm.aie2p.acquire(i32 50, i32 -1)
  tail call void @llvm.aie2p.acquire(i32 49, i32 -1)
  tail call void @llvm.memcpy.p0.p0.i20(ptr noundef nonnull align 1 dereferenceable(4096) @out_buff_1, ptr noundef nonnull align 1 dereferenceable(4096) @in_cons_buff_1, i20 4096, i1 false)
  tail call void @llvm.aie2p.release(i32 48, i32 1)
  tail call void @llvm.aie2p.release(i32 51, i32 1)
  %2 = add nuw nsw i64 %1, 2
  %.not = icmp eq i64 %2, 9223372036854775806
  br i1 %.not, label %passthrough_fn.exit4, label %passthrough_fn.exit

passthrough_fn.exit4:                             ; preds = %passthrough_fn.exit
  tail call void @llvm.aie2p.acquire(i32 50, i32 -1)
  tail call void @llvm.aie2p.acquire(i32 49, i32 -1)
  tail call void @llvm.memcpy.p0.p0.i20(ptr noundef nonnull align 1 dereferenceable(4096) @out_buff_0, ptr noundef nonnull align 1 dereferenceable(4096) @in_cons_buff_0, i20 4096, i1 false)
  tail call void @llvm.aie2p.release(i32 48, i32 1)
  tail call void @llvm.aie2p.release(i32 51, i32 1)
  ret void
}

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i20(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i20, i1 immarg) #3

attributes #0 = { mustprogress nocallback nofree nosync nounwind willreturn }
attributes #1 = { nofree norecurse nosync nounwind memory(argmem: readwrite) }
attributes #2 = { nofree norecurse nosync nounwind }
attributes #3 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }

!llvm.module.flags = !{!0}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = distinct !{!1, !2, !3}
!2 = !{!"llvm.loop.isvectorized", i32 1}
!3 = !{!"llvm.loop.unroll.runtime.disable"}
!4 = distinct !{!4, !2}
