//===-- tsan_rtl_sparc64.S -------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "sanitizer_common/sanitizer_asm.h"

#define STACK_BIAS 2047
#define FRAME_SIZE 208

.text
ASM_HIDDEN(__tsan_setjmp)

.macro intercept symbol, real
.comm \real, 8, 8
.globl ASM_SYMBOL_INTERCEPTOR(\symbol)
ASM_TYPE_FUNCTION(ASM_SYMBOL_INTERCEPTOR(\symbol))
ASM_SYMBOL_INTERCEPTOR(\symbol):
  CFI_STARTPROC
  CFI_DEF_CFA(%sp, STACK_BIAS)
  // Keep the caller's register window for libc's setjmp.
  sub %sp, FRAME_SIZE, %sp
  CFI_ADJUST_CFA_OFFSET(FRAME_SIZE)
  stx %o0, [%sp + STACK_BIAS + 176]
  stx %o1, [%sp + STACK_BIAS + 184]
  stx %o7, [%sp + STACK_BIAS + 192]
  CFI_OFFSET(%o7, -16)
  call ASM_SYMBOL(__tsan_setjmp)
   add %sp, FRAME_SIZE, %o0

.Lpc_\@:
  rd %pc, %o2
.Lhi_\@:
  sethi %hi(_GLOBAL_OFFSET_TABLE_+(.Lhi_\@-.Lpc_\@)), %o3
.Llo_\@:
  or %o3, %lo(_GLOBAL_OFFSET_TABLE_+(.Llo_\@-.Lpc_\@)), %o3
  add %o3, %o2, %o2
  sethi %hi(\real), %o3
  add %o3, %lo(\real), %o3
  ldx [%o2 + %o3], %o2
  ldx [%o2], %o2
  ldx [%sp + STACK_BIAS + 176], %o0
  ldx [%sp + STACK_BIAS + 184], %o1
  ldx [%sp + STACK_BIAS + 192], %o7
  CFI_RESTORE(%o7)
  add %sp, FRAME_SIZE, %sp
  CFI_ADJUST_CFA_OFFSET(-FRAME_SIZE)
  jmp %o2
   nop
  CFI_ENDPROC
  ASM_SIZE(ASM_SYMBOL_INTERCEPTOR(\symbol))
.endm

intercept setjmp, _ZN14__interception11real_setjmpE
intercept _setjmp, _ZN14__interception12real__setjmpE
intercept sigsetjmp, _ZN14__interception14real_sigsetjmpE
intercept __sigsetjmp, _ZN14__interception16real___sigsetjmpE

NO_EXEC_STACK_DIRECTIVE
