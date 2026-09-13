; RUN: opt < %s -passes=asan -S -mtriple=sparcv9-unknown-linux-gnu -asan-use-after-return=never | FileCheck %s
; RUN: opt < %s -passes=asan -S -mtriple=sparc64-unknown-linux-gnu -asan-use-after-return=never | FileCheck %s

target datalayout = "E-m:e-i64:64-n32:64-S128"

; The two sign-extended address ranges share a 52-bit shadow mapping.
define i8 @load(ptr %p) sanitize_address {
; CHECK-LABEL: define i8 @load(
; CHECK: [[ADDR:%.*]] = ptrtoint ptr %p to i64
; CHECK-NEXT: [[MASK:%.*]] = and i64 [[ADDR]], 4503599627370495
; CHECK-NEXT: [[SHIFT:%.*]] = lshr i64 [[MASK]], 3
; CHECK-NEXT: [[SHADOW:%.*]] = add i64 [[SHIFT]], 8796093022208
; CHECK-NEXT: inttoptr i64 [[SHADOW]] to ptr
  %v = load i8, ptr %p
  ret i8 %v
}

declare void @use(ptr)

define void @stack() sanitize_address {
; CHECK-LABEL: define void @stack(
; CHECK: [[MASK:%.*]] = and i64 {{%.*}}, 4503599627370495
; CHECK-NEXT: [[SHIFT:%.*]] = lshr i64 [[MASK]], 3
; CHECK-NEXT: [[SHADOW:%.*]] = add i64 [[SHIFT]], 8796093022208
; CHECK: call void @use(
  %buf = alloca [16 x i8], align 8
  call void @use(ptr %buf)
  ret void
}
