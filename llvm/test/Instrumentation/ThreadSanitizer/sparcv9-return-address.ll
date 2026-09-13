; RUN: opt < %s -passes=tsan -S -mtriple=sparcv9-unknown-linux-gnu | FileCheck %s --check-prefix=SPARC64
; RUN: opt < %s -passes=tsan -S -mtriple=sparc64-unknown-linux-gnu | FileCheck %s --check-prefix=SPARC64
; RUN: opt < %s -passes=tsan -S -mtriple=x86_64-unknown-linux-gnu | FileCheck %s --check-prefix=OTHER
; RUN: opt < %s -passes=tsan -S -mtriple=aarch64-unknown-linux-gnu | FileCheck %s --check-prefix=OTHER

target datalayout = "E-m:e-i64:64-n32:64-S128"

define void @caller() sanitize_thread {
; SPARC64-LABEL: define void @caller()
; SPARC64: [[RAW:%.*]] = call ptr @llvm.returnaddress.p0(i32 0)
; SPARC64-NEXT: [[PC:%.*]] = getelementptr i8, ptr [[RAW]], i64 8
; SPARC64-NEXT: call void @__tsan_func_entry(ptr [[PC]])
; OTHER-LABEL: define void @caller()
; OTHER: [[PC:%.*]] = call ptr @llvm.returnaddress.p0(i32 0)
; OTHER-NEXT: call void @__tsan_func_entry(ptr [[PC]])
  call void @callee()
  ret void
}

declare void @callee()
