; NOTE: Assertions have been autogenerated by utils/update_analyze_test_checks.py
; RUN: opt < %s -mtriple=x86_64-unknown-linux-gnu -passes="print<cost-model>" 2>&1 -disable-output -cost-kind=size-latency -mattr=-popcnt,+sse2 | FileCheck %s -check-prefixes=SSE2,NOPOPCNT
; RUN: opt < %s -mtriple=x86_64-unknown-linux-gnu -passes="print<cost-model>" 2>&1 -disable-output -cost-kind=size-latency -mattr=+popcnt,+sse2 | FileCheck %s -check-prefixes=SSE2,POPCNT
; RUN: opt < %s -mtriple=x86_64-unknown-linux-gnu -passes="print<cost-model>" 2>&1 -disable-output -cost-kind=size-latency -mattr=+popcnt,+sse4.2 | FileCheck %s -check-prefixes=POPCNT,SSE42
; RUN: opt < %s -mtriple=x86_64-unknown-linux-gnu -passes="print<cost-model>" 2>&1 -disable-output -cost-kind=size-latency -mattr=+popcnt,+avx | FileCheck %s -check-prefixes=POPCNT,AVX1
; RUN: opt < %s -mtriple=x86_64-unknown-linux-gnu -passes="print<cost-model>" 2>&1 -disable-output -cost-kind=size-latency -mattr=+popcnt,+avx2 | FileCheck %s -check-prefixes=POPCNT,AVX2
; RUN: opt < %s -mtriple=x86_64-unknown-linux-gnu -passes="print<cost-model>" 2>&1 -disable-output -cost-kind=size-latency -mattr=+popcnt,+avx512f | FileCheck %s -check-prefixes=POPCNT,AVX512F
; RUN: opt < %s -mtriple=x86_64-unknown-linux-gnu -passes="print<cost-model>" 2>&1 -disable-output -cost-kind=size-latency -mattr=+popcnt,+avx512vl,+avx512bw,+avx512dq | FileCheck %s -check-prefixes=POPCNT,AVX512BW
; RUN: opt < %s -mtriple=x86_64-unknown-linux-gnu -passes="print<cost-model>" 2>&1 -disable-output -cost-kind=size-latency -mattr=+popcnt,+avx512vl,+avx512vpopcntdq | FileCheck %s -check-prefixes=POPCNT,AVX512VPOPCNT
; RUN: opt < %s -mtriple=x86_64-unknown-linux-gnu -passes="print<cost-model>" 2>&1 -disable-output -cost-kind=size-latency -mattr=+popcnt,+avx512vl,+avx512bitalg | FileCheck %s -check-prefixes=POPCNT,AVX512BITALG

; Verify the cost of scalar population count instructions.

declare i64 @llvm.ctpop.i64(i64)
declare i32 @llvm.ctpop.i32(i32)
declare i16 @llvm.ctpop.i16(i16)
declare  i8 @llvm.ctpop.i8(i8)

define i64 @var_ctpop_i64(i64 %a) {
; NOPOPCNT-LABEL: 'var_ctpop_i64'
; NOPOPCNT-NEXT:  Cost Model: Found an estimated cost of 19 for instruction: %ctpop = call i64 @llvm.ctpop.i64(i64 %a)
; NOPOPCNT-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret i64 %ctpop
;
; POPCNT-LABEL: 'var_ctpop_i64'
; POPCNT-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: %ctpop = call i64 @llvm.ctpop.i64(i64 %a)
; POPCNT-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret i64 %ctpop
;
  %ctpop = call i64 @llvm.ctpop.i64(i64 %a)
  ret i64 %ctpop
}

define i32 @var_ctpop_i32(i32 %a) {
; NOPOPCNT-LABEL: 'var_ctpop_i32'
; NOPOPCNT-NEXT:  Cost Model: Found an estimated cost of 15 for instruction: %ctpop = call i32 @llvm.ctpop.i32(i32 %a)
; NOPOPCNT-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret i32 %ctpop
;
; POPCNT-LABEL: 'var_ctpop_i32'
; POPCNT-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: %ctpop = call i32 @llvm.ctpop.i32(i32 %a)
; POPCNT-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret i32 %ctpop
;
  %ctpop = call i32 @llvm.ctpop.i32(i32 %a)
  ret i32 %ctpop
}

define i16 @var_ctpop_i16(i16 %a) {
; NOPOPCNT-LABEL: 'var_ctpop_i16'
; NOPOPCNT-NEXT:  Cost Model: Found an estimated cost of 17 for instruction: %ctpop = call i16 @llvm.ctpop.i16(i16 %a)
; NOPOPCNT-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret i16 %ctpop
;
; POPCNT-LABEL: 'var_ctpop_i16'
; POPCNT-NEXT:  Cost Model: Found an estimated cost of 2 for instruction: %ctpop = call i16 @llvm.ctpop.i16(i16 %a)
; POPCNT-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret i16 %ctpop
;
  %ctpop = call i16 @llvm.ctpop.i16(i16 %a)
  ret i16 %ctpop
}

define i8 @var_ctpop_i8(i8 %a) {
; NOPOPCNT-LABEL: 'var_ctpop_i8'
; NOPOPCNT-NEXT:  Cost Model: Found an estimated cost of 6 for instruction: %ctpop = call i8 @llvm.ctpop.i8(i8 %a)
; NOPOPCNT-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret i8 %ctpop
;
; POPCNT-LABEL: 'var_ctpop_i8'
; POPCNT-NEXT:  Cost Model: Found an estimated cost of 2 for instruction: %ctpop = call i8 @llvm.ctpop.i8(i8 %a)
; POPCNT-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret i8 %ctpop
;
  %ctpop = call i8 @llvm.ctpop.i8(i8 %a)
  ret i8 %ctpop
}

; Verify the cost of vector population count instructions.

declare <2 x i64> @llvm.ctpop.v2i64(<2 x i64>)
declare <4 x i32> @llvm.ctpop.v4i32(<4 x i32>)
declare <8 x i16> @llvm.ctpop.v8i16(<8 x i16>)
declare <16 x i8> @llvm.ctpop.v16i8(<16 x i8>)

declare <4 x i64> @llvm.ctpop.v4i64(<4 x i64>)
declare <8 x i32> @llvm.ctpop.v8i32(<8 x i32>)
declare <16 x i16> @llvm.ctpop.v16i16(<16 x i16>)
declare <32 x i8> @llvm.ctpop.v32i8(<32 x i8>)

declare <8 x i64> @llvm.ctpop.v8i64(<8 x i64>)
declare <16 x i32> @llvm.ctpop.v16i32(<16 x i32>)
declare <32 x i16> @llvm.ctpop.v32i16(<32 x i16>)
declare <64 x i8> @llvm.ctpop.v64i8(<64 x i8>)

define <2 x i64> @var_ctpop_v2i64(<2 x i64> %a) {
; SSE2-LABEL: 'var_ctpop_v2i64'
; SSE2-NEXT:  Cost Model: Found an estimated cost of 18 for instruction: %ctpop = call <2 x i64> @llvm.ctpop.v2i64(<2 x i64> %a)
; SSE2-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <2 x i64> %ctpop
;
; SSE42-LABEL: 'var_ctpop_v2i64'
; SSE42-NEXT:  Cost Model: Found an estimated cost of 18 for instruction: %ctpop = call <2 x i64> @llvm.ctpop.v2i64(<2 x i64> %a)
; SSE42-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <2 x i64> %ctpop
;
; AVX1-LABEL: 'var_ctpop_v2i64'
; AVX1-NEXT:  Cost Model: Found an estimated cost of 14 for instruction: %ctpop = call <2 x i64> @llvm.ctpop.v2i64(<2 x i64> %a)
; AVX1-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <2 x i64> %ctpop
;
; AVX2-LABEL: 'var_ctpop_v2i64'
; AVX2-NEXT:  Cost Model: Found an estimated cost of 10 for instruction: %ctpop = call <2 x i64> @llvm.ctpop.v2i64(<2 x i64> %a)
; AVX2-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <2 x i64> %ctpop
;
; AVX512F-LABEL: 'var_ctpop_v2i64'
; AVX512F-NEXT:  Cost Model: Found an estimated cost of 10 for instruction: %ctpop = call <2 x i64> @llvm.ctpop.v2i64(<2 x i64> %a)
; AVX512F-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <2 x i64> %ctpop
;
; AVX512BW-LABEL: 'var_ctpop_v2i64'
; AVX512BW-NEXT:  Cost Model: Found an estimated cost of 10 for instruction: %ctpop = call <2 x i64> @llvm.ctpop.v2i64(<2 x i64> %a)
; AVX512BW-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <2 x i64> %ctpop
;
; AVX512VPOPCNT-LABEL: 'var_ctpop_v2i64'
; AVX512VPOPCNT-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: %ctpop = call <2 x i64> @llvm.ctpop.v2i64(<2 x i64> %a)
; AVX512VPOPCNT-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <2 x i64> %ctpop
;
; AVX512BITALG-LABEL: 'var_ctpop_v2i64'
; AVX512BITALG-NEXT:  Cost Model: Found an estimated cost of 10 for instruction: %ctpop = call <2 x i64> @llvm.ctpop.v2i64(<2 x i64> %a)
; AVX512BITALG-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <2 x i64> %ctpop
;
  %ctpop = call <2 x i64> @llvm.ctpop.v2i64(<2 x i64> %a)
  ret <2 x i64> %ctpop
}

define <4 x i64> @var_ctpop_v4i64(<4 x i64> %a) {
; SSE2-LABEL: 'var_ctpop_v4i64'
; SSE2-NEXT:  Cost Model: Found an estimated cost of 36 for instruction: %ctpop = call <4 x i64> @llvm.ctpop.v4i64(<4 x i64> %a)
; SSE2-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <4 x i64> %ctpop
;
; SSE42-LABEL: 'var_ctpop_v4i64'
; SSE42-NEXT:  Cost Model: Found an estimated cost of 36 for instruction: %ctpop = call <4 x i64> @llvm.ctpop.v4i64(<4 x i64> %a)
; SSE42-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <4 x i64> %ctpop
;
; AVX1-LABEL: 'var_ctpop_v4i64'
; AVX1-NEXT:  Cost Model: Found an estimated cost of 28 for instruction: %ctpop = call <4 x i64> @llvm.ctpop.v4i64(<4 x i64> %a)
; AVX1-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <4 x i64> %ctpop
;
; AVX2-LABEL: 'var_ctpop_v4i64'
; AVX2-NEXT:  Cost Model: Found an estimated cost of 14 for instruction: %ctpop = call <4 x i64> @llvm.ctpop.v4i64(<4 x i64> %a)
; AVX2-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <4 x i64> %ctpop
;
; AVX512F-LABEL: 'var_ctpop_v4i64'
; AVX512F-NEXT:  Cost Model: Found an estimated cost of 14 for instruction: %ctpop = call <4 x i64> @llvm.ctpop.v4i64(<4 x i64> %a)
; AVX512F-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <4 x i64> %ctpop
;
; AVX512BW-LABEL: 'var_ctpop_v4i64'
; AVX512BW-NEXT:  Cost Model: Found an estimated cost of 10 for instruction: %ctpop = call <4 x i64> @llvm.ctpop.v4i64(<4 x i64> %a)
; AVX512BW-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <4 x i64> %ctpop
;
; AVX512VPOPCNT-LABEL: 'var_ctpop_v4i64'
; AVX512VPOPCNT-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: %ctpop = call <4 x i64> @llvm.ctpop.v4i64(<4 x i64> %a)
; AVX512VPOPCNT-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <4 x i64> %ctpop
;
; AVX512BITALG-LABEL: 'var_ctpop_v4i64'
; AVX512BITALG-NEXT:  Cost Model: Found an estimated cost of 10 for instruction: %ctpop = call <4 x i64> @llvm.ctpop.v4i64(<4 x i64> %a)
; AVX512BITALG-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <4 x i64> %ctpop
;
  %ctpop = call <4 x i64> @llvm.ctpop.v4i64(<4 x i64> %a)
  ret <4 x i64> %ctpop
}

define <8 x i64> @var_ctpop_v8i64(<8 x i64> %a) {
; SSE2-LABEL: 'var_ctpop_v8i64'
; SSE2-NEXT:  Cost Model: Found an estimated cost of 72 for instruction: %ctpop = call <8 x i64> @llvm.ctpop.v8i64(<8 x i64> %a)
; SSE2-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <8 x i64> %ctpop
;
; SSE42-LABEL: 'var_ctpop_v8i64'
; SSE42-NEXT:  Cost Model: Found an estimated cost of 72 for instruction: %ctpop = call <8 x i64> @llvm.ctpop.v8i64(<8 x i64> %a)
; SSE42-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <8 x i64> %ctpop
;
; AVX1-LABEL: 'var_ctpop_v8i64'
; AVX1-NEXT:  Cost Model: Found an estimated cost of 56 for instruction: %ctpop = call <8 x i64> @llvm.ctpop.v8i64(<8 x i64> %a)
; AVX1-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <8 x i64> %ctpop
;
; AVX2-LABEL: 'var_ctpop_v8i64'
; AVX2-NEXT:  Cost Model: Found an estimated cost of 28 for instruction: %ctpop = call <8 x i64> @llvm.ctpop.v8i64(<8 x i64> %a)
; AVX2-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <8 x i64> %ctpop
;
; AVX512F-LABEL: 'var_ctpop_v8i64'
; AVX512F-NEXT:  Cost Model: Found an estimated cost of 19 for instruction: %ctpop = call <8 x i64> @llvm.ctpop.v8i64(<8 x i64> %a)
; AVX512F-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <8 x i64> %ctpop
;
; AVX512BW-LABEL: 'var_ctpop_v8i64'
; AVX512BW-NEXT:  Cost Model: Found an estimated cost of 12 for instruction: %ctpop = call <8 x i64> @llvm.ctpop.v8i64(<8 x i64> %a)
; AVX512BW-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <8 x i64> %ctpop
;
; AVX512VPOPCNT-LABEL: 'var_ctpop_v8i64'
; AVX512VPOPCNT-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: %ctpop = call <8 x i64> @llvm.ctpop.v8i64(<8 x i64> %a)
; AVX512VPOPCNT-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <8 x i64> %ctpop
;
; AVX512BITALG-LABEL: 'var_ctpop_v8i64'
; AVX512BITALG-NEXT:  Cost Model: Found an estimated cost of 12 for instruction: %ctpop = call <8 x i64> @llvm.ctpop.v8i64(<8 x i64> %a)
; AVX512BITALG-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <8 x i64> %ctpop
;
  %ctpop = call <8 x i64> @llvm.ctpop.v8i64(<8 x i64> %a)
  ret <8 x i64> %ctpop
}

define <4 x i32> @var_ctpop_v4i32(<4 x i32> %a) {
; SSE2-LABEL: 'var_ctpop_v4i32'
; SSE2-NEXT:  Cost Model: Found an estimated cost of 23 for instruction: %ctpop = call <4 x i32> @llvm.ctpop.v4i32(<4 x i32> %a)
; SSE2-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <4 x i32> %ctpop
;
; SSE42-LABEL: 'var_ctpop_v4i32'
; SSE42-NEXT:  Cost Model: Found an estimated cost of 22 for instruction: %ctpop = call <4 x i32> @llvm.ctpop.v4i32(<4 x i32> %a)
; SSE42-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <4 x i32> %ctpop
;
; AVX1-LABEL: 'var_ctpop_v4i32'
; AVX1-NEXT:  Cost Model: Found an estimated cost of 18 for instruction: %ctpop = call <4 x i32> @llvm.ctpop.v4i32(<4 x i32> %a)
; AVX1-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <4 x i32> %ctpop
;
; AVX2-LABEL: 'var_ctpop_v4i32'
; AVX2-NEXT:  Cost Model: Found an estimated cost of 14 for instruction: %ctpop = call <4 x i32> @llvm.ctpop.v4i32(<4 x i32> %a)
; AVX2-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <4 x i32> %ctpop
;
; AVX512F-LABEL: 'var_ctpop_v4i32'
; AVX512F-NEXT:  Cost Model: Found an estimated cost of 14 for instruction: %ctpop = call <4 x i32> @llvm.ctpop.v4i32(<4 x i32> %a)
; AVX512F-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <4 x i32> %ctpop
;
; AVX512BW-LABEL: 'var_ctpop_v4i32'
; AVX512BW-NEXT:  Cost Model: Found an estimated cost of 14 for instruction: %ctpop = call <4 x i32> @llvm.ctpop.v4i32(<4 x i32> %a)
; AVX512BW-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <4 x i32> %ctpop
;
; AVX512VPOPCNT-LABEL: 'var_ctpop_v4i32'
; AVX512VPOPCNT-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: %ctpop = call <4 x i32> @llvm.ctpop.v4i32(<4 x i32> %a)
; AVX512VPOPCNT-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <4 x i32> %ctpop
;
; AVX512BITALG-LABEL: 'var_ctpop_v4i32'
; AVX512BITALG-NEXT:  Cost Model: Found an estimated cost of 14 for instruction: %ctpop = call <4 x i32> @llvm.ctpop.v4i32(<4 x i32> %a)
; AVX512BITALG-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <4 x i32> %ctpop
;
  %ctpop = call <4 x i32> @llvm.ctpop.v4i32(<4 x i32> %a)
  ret <4 x i32> %ctpop
}

define <8 x i32> @var_ctpop_v8i32(<8 x i32> %a) {
; SSE2-LABEL: 'var_ctpop_v8i32'
; SSE2-NEXT:  Cost Model: Found an estimated cost of 46 for instruction: %ctpop = call <8 x i32> @llvm.ctpop.v8i32(<8 x i32> %a)
; SSE2-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <8 x i32> %ctpop
;
; SSE42-LABEL: 'var_ctpop_v8i32'
; SSE42-NEXT:  Cost Model: Found an estimated cost of 44 for instruction: %ctpop = call <8 x i32> @llvm.ctpop.v8i32(<8 x i32> %a)
; SSE42-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <8 x i32> %ctpop
;
; AVX1-LABEL: 'var_ctpop_v8i32'
; AVX1-NEXT:  Cost Model: Found an estimated cost of 36 for instruction: %ctpop = call <8 x i32> @llvm.ctpop.v8i32(<8 x i32> %a)
; AVX1-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <8 x i32> %ctpop
;
; AVX2-LABEL: 'var_ctpop_v8i32'
; AVX2-NEXT:  Cost Model: Found an estimated cost of 18 for instruction: %ctpop = call <8 x i32> @llvm.ctpop.v8i32(<8 x i32> %a)
; AVX2-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <8 x i32> %ctpop
;
; AVX512F-LABEL: 'var_ctpop_v8i32'
; AVX512F-NEXT:  Cost Model: Found an estimated cost of 18 for instruction: %ctpop = call <8 x i32> @llvm.ctpop.v8i32(<8 x i32> %a)
; AVX512F-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <8 x i32> %ctpop
;
; AVX512BW-LABEL: 'var_ctpop_v8i32'
; AVX512BW-NEXT:  Cost Model: Found an estimated cost of 14 for instruction: %ctpop = call <8 x i32> @llvm.ctpop.v8i32(<8 x i32> %a)
; AVX512BW-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <8 x i32> %ctpop
;
; AVX512VPOPCNT-LABEL: 'var_ctpop_v8i32'
; AVX512VPOPCNT-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: %ctpop = call <8 x i32> @llvm.ctpop.v8i32(<8 x i32> %a)
; AVX512VPOPCNT-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <8 x i32> %ctpop
;
; AVX512BITALG-LABEL: 'var_ctpop_v8i32'
; AVX512BITALG-NEXT:  Cost Model: Found an estimated cost of 14 for instruction: %ctpop = call <8 x i32> @llvm.ctpop.v8i32(<8 x i32> %a)
; AVX512BITALG-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <8 x i32> %ctpop
;
  %ctpop = call <8 x i32> @llvm.ctpop.v8i32(<8 x i32> %a)
  ret <8 x i32> %ctpop
}

define <16 x i32> @var_ctpop_v16i32(<16 x i32> %a) {
; SSE2-LABEL: 'var_ctpop_v16i32'
; SSE2-NEXT:  Cost Model: Found an estimated cost of 92 for instruction: %ctpop = call <16 x i32> @llvm.ctpop.v16i32(<16 x i32> %a)
; SSE2-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <16 x i32> %ctpop
;
; SSE42-LABEL: 'var_ctpop_v16i32'
; SSE42-NEXT:  Cost Model: Found an estimated cost of 88 for instruction: %ctpop = call <16 x i32> @llvm.ctpop.v16i32(<16 x i32> %a)
; SSE42-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <16 x i32> %ctpop
;
; AVX1-LABEL: 'var_ctpop_v16i32'
; AVX1-NEXT:  Cost Model: Found an estimated cost of 72 for instruction: %ctpop = call <16 x i32> @llvm.ctpop.v16i32(<16 x i32> %a)
; AVX1-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <16 x i32> %ctpop
;
; AVX2-LABEL: 'var_ctpop_v16i32'
; AVX2-NEXT:  Cost Model: Found an estimated cost of 36 for instruction: %ctpop = call <16 x i32> @llvm.ctpop.v16i32(<16 x i32> %a)
; AVX2-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <16 x i32> %ctpop
;
; AVX512F-LABEL: 'var_ctpop_v16i32'
; AVX512F-NEXT:  Cost Model: Found an estimated cost of 27 for instruction: %ctpop = call <16 x i32> @llvm.ctpop.v16i32(<16 x i32> %a)
; AVX512F-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <16 x i32> %ctpop
;
; AVX512BW-LABEL: 'var_ctpop_v16i32'
; AVX512BW-NEXT:  Cost Model: Found an estimated cost of 16 for instruction: %ctpop = call <16 x i32> @llvm.ctpop.v16i32(<16 x i32> %a)
; AVX512BW-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <16 x i32> %ctpop
;
; AVX512VPOPCNT-LABEL: 'var_ctpop_v16i32'
; AVX512VPOPCNT-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: %ctpop = call <16 x i32> @llvm.ctpop.v16i32(<16 x i32> %a)
; AVX512VPOPCNT-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <16 x i32> %ctpop
;
; AVX512BITALG-LABEL: 'var_ctpop_v16i32'
; AVX512BITALG-NEXT:  Cost Model: Found an estimated cost of 16 for instruction: %ctpop = call <16 x i32> @llvm.ctpop.v16i32(<16 x i32> %a)
; AVX512BITALG-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <16 x i32> %ctpop
;
  %ctpop = call <16 x i32> @llvm.ctpop.v16i32(<16 x i32> %a)
  ret <16 x i32> %ctpop
}

define <8 x i16> @var_ctpop_v8i16(<8 x i16> %a) {
; SSE2-LABEL: 'var_ctpop_v8i16'
; SSE2-NEXT:  Cost Model: Found an estimated cost of 20 for instruction: %ctpop = call <8 x i16> @llvm.ctpop.v8i16(<8 x i16> %a)
; SSE2-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <8 x i16> %ctpop
;
; SSE42-LABEL: 'var_ctpop_v8i16'
; SSE42-NEXT:  Cost Model: Found an estimated cost of 20 for instruction: %ctpop = call <8 x i16> @llvm.ctpop.v8i16(<8 x i16> %a)
; SSE42-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <8 x i16> %ctpop
;
; AVX1-LABEL: 'var_ctpop_v8i16'
; AVX1-NEXT:  Cost Model: Found an estimated cost of 15 for instruction: %ctpop = call <8 x i16> @llvm.ctpop.v8i16(<8 x i16> %a)
; AVX1-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <8 x i16> %ctpop
;
; AVX2-LABEL: 'var_ctpop_v8i16'
; AVX2-NEXT:  Cost Model: Found an estimated cost of 11 for instruction: %ctpop = call <8 x i16> @llvm.ctpop.v8i16(<8 x i16> %a)
; AVX2-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <8 x i16> %ctpop
;
; AVX512F-LABEL: 'var_ctpop_v8i16'
; AVX512F-NEXT:  Cost Model: Found an estimated cost of 11 for instruction: %ctpop = call <8 x i16> @llvm.ctpop.v8i16(<8 x i16> %a)
; AVX512F-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <8 x i16> %ctpop
;
; AVX512BW-LABEL: 'var_ctpop_v8i16'
; AVX512BW-NEXT:  Cost Model: Found an estimated cost of 11 for instruction: %ctpop = call <8 x i16> @llvm.ctpop.v8i16(<8 x i16> %a)
; AVX512BW-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <8 x i16> %ctpop
;
; AVX512VPOPCNT-LABEL: 'var_ctpop_v8i16'
; AVX512VPOPCNT-NEXT:  Cost Model: Found an estimated cost of 11 for instruction: %ctpop = call <8 x i16> @llvm.ctpop.v8i16(<8 x i16> %a)
; AVX512VPOPCNT-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <8 x i16> %ctpop
;
; AVX512BITALG-LABEL: 'var_ctpop_v8i16'
; AVX512BITALG-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: %ctpop = call <8 x i16> @llvm.ctpop.v8i16(<8 x i16> %a)
; AVX512BITALG-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <8 x i16> %ctpop
;
  %ctpop = call <8 x i16> @llvm.ctpop.v8i16(<8 x i16> %a)
  ret <8 x i16> %ctpop
}

define <16 x i16> @var_ctpop_v16i16(<16 x i16> %a) {
; SSE2-LABEL: 'var_ctpop_v16i16'
; SSE2-NEXT:  Cost Model: Found an estimated cost of 40 for instruction: %ctpop = call <16 x i16> @llvm.ctpop.v16i16(<16 x i16> %a)
; SSE2-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <16 x i16> %ctpop
;
; SSE42-LABEL: 'var_ctpop_v16i16'
; SSE42-NEXT:  Cost Model: Found an estimated cost of 40 for instruction: %ctpop = call <16 x i16> @llvm.ctpop.v16i16(<16 x i16> %a)
; SSE42-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <16 x i16> %ctpop
;
; AVX1-LABEL: 'var_ctpop_v16i16'
; AVX1-NEXT:  Cost Model: Found an estimated cost of 31 for instruction: %ctpop = call <16 x i16> @llvm.ctpop.v16i16(<16 x i16> %a)
; AVX1-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <16 x i16> %ctpop
;
; AVX2-LABEL: 'var_ctpop_v16i16'
; AVX2-NEXT:  Cost Model: Found an estimated cost of 18 for instruction: %ctpop = call <16 x i16> @llvm.ctpop.v16i16(<16 x i16> %a)
; AVX2-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <16 x i16> %ctpop
;
; AVX512F-LABEL: 'var_ctpop_v16i16'
; AVX512F-NEXT:  Cost Model: Found an estimated cost of 18 for instruction: %ctpop = call <16 x i16> @llvm.ctpop.v16i16(<16 x i16> %a)
; AVX512F-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <16 x i16> %ctpop
;
; AVX512BW-LABEL: 'var_ctpop_v16i16'
; AVX512BW-NEXT:  Cost Model: Found an estimated cost of 11 for instruction: %ctpop = call <16 x i16> @llvm.ctpop.v16i16(<16 x i16> %a)
; AVX512BW-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <16 x i16> %ctpop
;
; AVX512VPOPCNT-LABEL: 'var_ctpop_v16i16'
; AVX512VPOPCNT-NEXT:  Cost Model: Found an estimated cost of 18 for instruction: %ctpop = call <16 x i16> @llvm.ctpop.v16i16(<16 x i16> %a)
; AVX512VPOPCNT-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <16 x i16> %ctpop
;
; AVX512BITALG-LABEL: 'var_ctpop_v16i16'
; AVX512BITALG-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: %ctpop = call <16 x i16> @llvm.ctpop.v16i16(<16 x i16> %a)
; AVX512BITALG-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <16 x i16> %ctpop
;
  %ctpop = call <16 x i16> @llvm.ctpop.v16i16(<16 x i16> %a)
  ret <16 x i16> %ctpop
}

define <32 x i16> @var_ctpop_v32i16(<32 x i16> %a) {
; SSE2-LABEL: 'var_ctpop_v32i16'
; SSE2-NEXT:  Cost Model: Found an estimated cost of 80 for instruction: %ctpop = call <32 x i16> @llvm.ctpop.v32i16(<32 x i16> %a)
; SSE2-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <32 x i16> %ctpop
;
; SSE42-LABEL: 'var_ctpop_v32i16'
; SSE42-NEXT:  Cost Model: Found an estimated cost of 80 for instruction: %ctpop = call <32 x i16> @llvm.ctpop.v32i16(<32 x i16> %a)
; SSE42-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <32 x i16> %ctpop
;
; AVX1-LABEL: 'var_ctpop_v32i16'
; AVX1-NEXT:  Cost Model: Found an estimated cost of 62 for instruction: %ctpop = call <32 x i16> @llvm.ctpop.v32i16(<32 x i16> %a)
; AVX1-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <32 x i16> %ctpop
;
; AVX2-LABEL: 'var_ctpop_v32i16'
; AVX2-NEXT:  Cost Model: Found an estimated cost of 36 for instruction: %ctpop = call <32 x i16> @llvm.ctpop.v32i16(<32 x i16> %a)
; AVX2-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <32 x i16> %ctpop
;
; AVX512F-LABEL: 'var_ctpop_v32i16'
; AVX512F-NEXT:  Cost Model: Found an estimated cost of 22 for instruction: %ctpop = call <32 x i16> @llvm.ctpop.v32i16(<32 x i16> %a)
; AVX512F-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <32 x i16> %ctpop
;
; AVX512BW-LABEL: 'var_ctpop_v32i16'
; AVX512BW-NEXT:  Cost Model: Found an estimated cost of 13 for instruction: %ctpop = call <32 x i16> @llvm.ctpop.v32i16(<32 x i16> %a)
; AVX512BW-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <32 x i16> %ctpop
;
; AVX512VPOPCNT-LABEL: 'var_ctpop_v32i16'
; AVX512VPOPCNT-NEXT:  Cost Model: Found an estimated cost of 22 for instruction: %ctpop = call <32 x i16> @llvm.ctpop.v32i16(<32 x i16> %a)
; AVX512VPOPCNT-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <32 x i16> %ctpop
;
; AVX512BITALG-LABEL: 'var_ctpop_v32i16'
; AVX512BITALG-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: %ctpop = call <32 x i16> @llvm.ctpop.v32i16(<32 x i16> %a)
; AVX512BITALG-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <32 x i16> %ctpop
;
  %ctpop = call <32 x i16> @llvm.ctpop.v32i16(<32 x i16> %a)
  ret <32 x i16> %ctpop
}

define <16 x i8> @var_ctpop_v16i8(<16 x i8> %a) {
; SSE2-LABEL: 'var_ctpop_v16i8'
; SSE2-NEXT:  Cost Model: Found an estimated cost of 16 for instruction: %ctpop = call <16 x i8> @llvm.ctpop.v16i8(<16 x i8> %a)
; SSE2-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <16 x i8> %ctpop
;
; SSE42-LABEL: 'var_ctpop_v16i8'
; SSE42-NEXT:  Cost Model: Found an estimated cost of 16 for instruction: %ctpop = call <16 x i8> @llvm.ctpop.v16i8(<16 x i8> %a)
; SSE42-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <16 x i8> %ctpop
;
; AVX1-LABEL: 'var_ctpop_v16i8'
; AVX1-NEXT:  Cost Model: Found an estimated cost of 12 for instruction: %ctpop = call <16 x i8> @llvm.ctpop.v16i8(<16 x i8> %a)
; AVX1-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <16 x i8> %ctpop
;
; AVX2-LABEL: 'var_ctpop_v16i8'
; AVX2-NEXT:  Cost Model: Found an estimated cost of 8 for instruction: %ctpop = call <16 x i8> @llvm.ctpop.v16i8(<16 x i8> %a)
; AVX2-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <16 x i8> %ctpop
;
; AVX512F-LABEL: 'var_ctpop_v16i8'
; AVX512F-NEXT:  Cost Model: Found an estimated cost of 8 for instruction: %ctpop = call <16 x i8> @llvm.ctpop.v16i8(<16 x i8> %a)
; AVX512F-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <16 x i8> %ctpop
;
; AVX512BW-LABEL: 'var_ctpop_v16i8'
; AVX512BW-NEXT:  Cost Model: Found an estimated cost of 8 for instruction: %ctpop = call <16 x i8> @llvm.ctpop.v16i8(<16 x i8> %a)
; AVX512BW-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <16 x i8> %ctpop
;
; AVX512VPOPCNT-LABEL: 'var_ctpop_v16i8'
; AVX512VPOPCNT-NEXT:  Cost Model: Found an estimated cost of 8 for instruction: %ctpop = call <16 x i8> @llvm.ctpop.v16i8(<16 x i8> %a)
; AVX512VPOPCNT-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <16 x i8> %ctpop
;
; AVX512BITALG-LABEL: 'var_ctpop_v16i8'
; AVX512BITALG-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: %ctpop = call <16 x i8> @llvm.ctpop.v16i8(<16 x i8> %a)
; AVX512BITALG-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <16 x i8> %ctpop
;
  %ctpop = call <16 x i8> @llvm.ctpop.v16i8(<16 x i8> %a)
  ret <16 x i8> %ctpop
}

define <32 x i8> @var_ctpop_v32i8(<32 x i8> %a) {
; SSE2-LABEL: 'var_ctpop_v32i8'
; SSE2-NEXT:  Cost Model: Found an estimated cost of 32 for instruction: %ctpop = call <32 x i8> @llvm.ctpop.v32i8(<32 x i8> %a)
; SSE2-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <32 x i8> %ctpop
;
; SSE42-LABEL: 'var_ctpop_v32i8'
; SSE42-NEXT:  Cost Model: Found an estimated cost of 32 for instruction: %ctpop = call <32 x i8> @llvm.ctpop.v32i8(<32 x i8> %a)
; SSE42-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <32 x i8> %ctpop
;
; AVX1-LABEL: 'var_ctpop_v32i8'
; AVX1-NEXT:  Cost Model: Found an estimated cost of 25 for instruction: %ctpop = call <32 x i8> @llvm.ctpop.v32i8(<32 x i8> %a)
; AVX1-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <32 x i8> %ctpop
;
; AVX2-LABEL: 'var_ctpop_v32i8'
; AVX2-NEXT:  Cost Model: Found an estimated cost of 12 for instruction: %ctpop = call <32 x i8> @llvm.ctpop.v32i8(<32 x i8> %a)
; AVX2-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <32 x i8> %ctpop
;
; AVX512F-LABEL: 'var_ctpop_v32i8'
; AVX512F-NEXT:  Cost Model: Found an estimated cost of 12 for instruction: %ctpop = call <32 x i8> @llvm.ctpop.v32i8(<32 x i8> %a)
; AVX512F-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <32 x i8> %ctpop
;
; AVX512BW-LABEL: 'var_ctpop_v32i8'
; AVX512BW-NEXT:  Cost Model: Found an estimated cost of 8 for instruction: %ctpop = call <32 x i8> @llvm.ctpop.v32i8(<32 x i8> %a)
; AVX512BW-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <32 x i8> %ctpop
;
; AVX512VPOPCNT-LABEL: 'var_ctpop_v32i8'
; AVX512VPOPCNT-NEXT:  Cost Model: Found an estimated cost of 12 for instruction: %ctpop = call <32 x i8> @llvm.ctpop.v32i8(<32 x i8> %a)
; AVX512VPOPCNT-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <32 x i8> %ctpop
;
; AVX512BITALG-LABEL: 'var_ctpop_v32i8'
; AVX512BITALG-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: %ctpop = call <32 x i8> @llvm.ctpop.v32i8(<32 x i8> %a)
; AVX512BITALG-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <32 x i8> %ctpop
;
  %ctpop = call <32 x i8> @llvm.ctpop.v32i8(<32 x i8> %a)
  ret <32 x i8> %ctpop
}

define <64 x i8> @var_ctpop_v64i8(<64 x i8> %a) {
; SSE2-LABEL: 'var_ctpop_v64i8'
; SSE2-NEXT:  Cost Model: Found an estimated cost of 64 for instruction: %ctpop = call <64 x i8> @llvm.ctpop.v64i8(<64 x i8> %a)
; SSE2-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <64 x i8> %ctpop
;
; SSE42-LABEL: 'var_ctpop_v64i8'
; SSE42-NEXT:  Cost Model: Found an estimated cost of 64 for instruction: %ctpop = call <64 x i8> @llvm.ctpop.v64i8(<64 x i8> %a)
; SSE42-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <64 x i8> %ctpop
;
; AVX1-LABEL: 'var_ctpop_v64i8'
; AVX1-NEXT:  Cost Model: Found an estimated cost of 50 for instruction: %ctpop = call <64 x i8> @llvm.ctpop.v64i8(<64 x i8> %a)
; AVX1-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <64 x i8> %ctpop
;
; AVX2-LABEL: 'var_ctpop_v64i8'
; AVX2-NEXT:  Cost Model: Found an estimated cost of 24 for instruction: %ctpop = call <64 x i8> @llvm.ctpop.v64i8(<64 x i8> %a)
; AVX2-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <64 x i8> %ctpop
;
; AVX512F-LABEL: 'var_ctpop_v64i8'
; AVX512F-NEXT:  Cost Model: Found an estimated cost of 16 for instruction: %ctpop = call <64 x i8> @llvm.ctpop.v64i8(<64 x i8> %a)
; AVX512F-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <64 x i8> %ctpop
;
; AVX512BW-LABEL: 'var_ctpop_v64i8'
; AVX512BW-NEXT:  Cost Model: Found an estimated cost of 10 for instruction: %ctpop = call <64 x i8> @llvm.ctpop.v64i8(<64 x i8> %a)
; AVX512BW-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <64 x i8> %ctpop
;
; AVX512VPOPCNT-LABEL: 'var_ctpop_v64i8'
; AVX512VPOPCNT-NEXT:  Cost Model: Found an estimated cost of 16 for instruction: %ctpop = call <64 x i8> @llvm.ctpop.v64i8(<64 x i8> %a)
; AVX512VPOPCNT-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <64 x i8> %ctpop
;
; AVX512BITALG-LABEL: 'var_ctpop_v64i8'
; AVX512BITALG-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: %ctpop = call <64 x i8> @llvm.ctpop.v64i8(<64 x i8> %a)
; AVX512BITALG-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret <64 x i8> %ctpop
;
  %ctpop = call <64 x i8> @llvm.ctpop.v64i8(<64 x i8> %a)
  ret <64 x i8> %ctpop
}
