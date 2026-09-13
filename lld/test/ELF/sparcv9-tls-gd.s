# REQUIRES: sparc
# RUN: split-file %s %t
# RUN: llvm-mc -filetype=obj -triple=sparcv9 %t/a.s -o %t/a.o
# RUN: llvm-mc -filetype=obj -triple=sparcv9 %t/b.s -o %t/b.o
# RUN: ld.lld -shared %t/b.o -o %t/b.so
# RUN: ld.lld -shared %t/a.o %t/b.so -o %t/a.so
# RUN: llvm-readelf -r %t/a.so | FileCheck %s --check-prefix=SHARED
# RUN: ld.lld %t/a.o %t/b.so -o %t/a
# RUN: llvm-readelf -r %t/a | FileCheck %s --check-prefix=EXE
# RUN: llvm-objdump -d %t/a.so | FileCheck %s --check-prefix=DISASM
# RUN: ld.lld --gc-sections %t/a.o %t/b.o -o %t/local
# RUN: llvm-objdump -d %t/local | FileCheck %s --check-prefixes=DISASM,LOCAL
# RUN: llvm-readelf -x .got %t/local | FileCheck %s --check-prefix=GOT
# RUN: not ld.lld --defsym=external=0 %t/a.o -o /dev/null 2>&1 | FileCheck %s --check-prefix=UNDEF
# RUN: not ld.lld -shared -z defs %t/a.o -o /dev/null 2>&1 | FileCheck %s --check-prefix=UNDEF
# RUN: llvm-mc -filetype=obj -triple=sparcv9 %t/resolver.s -o %t/resolver.o
# RUN: llvm-ar crs %t/resolver.a %t/resolver.o
# RUN: ld.lld --gc-sections --defsym=external=0 %t/a.o %t/resolver.a -o %t/archive
# RUN: llvm-objdump -d %t/archive | FileCheck %s --check-prefix=LOCAL
# RUN: ld.lld -shared %t/resolver.o -o %t/resolver.so
# RUN: ld.lld --as-needed --defsym=external=0 %t/a.o %t/resolver.so -o %t/needed
# RUN: llvm-readelf -d %t/needed | FileCheck %s --check-prefix=NEEDED

# UNDEF: undefined symbol: __tls_get_addr
# NEEDED: (NEEDED) {{.*}}resolver.so
# DISASM: sethi 0x0, %o0
# DISASM-NEXT: add %o0, 0x8, %o0
# DISASM-NEXT: add %l7, %o0, %o0
# DISASM-NEXT: call 0x[[RESOLVER:[0-9a-f]+]]
# DISASM-NEXT: nop
# DISASM-NEXT: sethi 0x0, %o0
# DISASM-NEXT: add %o0, 0x18, %o0
# DISASM-NEXT: add %l7, %o0, %o0
# DISASM-NEXT: call 0x[[RESOLVER]]
# LOCAL: <__tls_get_addr>:
# LOCAL-NEXT: retl
# LOCAL-NEXT: nop
# GOT: 00000000 00000000 00000000 00000001
# GOT-NEXT: 00000000 00000008 00000000 00000001
# GOT-NEXT: 00000000 00000010

# SHARED: R_SPARC_TLS_DTPMOD64
# SHARED: R_SPARC_TLS_DTPMOD64 {{.*}} external + 0
# SHARED: R_SPARC_TLS_DTPOFF64 {{.*}} external + 0
# SHARED: R_SPARC_JMP_SLOT {{.*}} __tls_get_addr + 0
# EXE: R_SPARC_TLS_DTPMOD64 {{.*}} external + 0
# EXE: R_SPARC_TLS_DTPOFF64 {{.*}} external + 0
# EXE: R_SPARC_JMP_SLOT {{.*}} __tls_get_addr + 0

#--- a.s
.globl _start
_start:
  sethi %tgd_hi22(local), %o0
  add %o0, %tgd_lo10(local), %o0
  add %l7, %o0, %o0, %tgd_add(local)
  call __tls_get_addr, %tgd_call(local)
  nop
  sethi %tgd_hi22(external), %o0
  add %o0, %tgd_lo10(external), %o0
  add %l7, %o0, %o0, %tgd_add(external)
  call __tls_get_addr, %tgd_call(external)
  nop
.section .tbss,"awT",@nobits
.globl local
.hidden local
.type local,@tls_object
.space 8
local:
.xword 0
.type external,@tls_object

#--- b.s
.globl __tls_get_addr
.type __tls_get_addr,@function
__tls_get_addr:
  retl
  nop
.section .tbss,"awT",@nobits
.globl external
.type external,@tls_object
external:
.xword 0

#--- resolver.s
.globl __tls_get_addr
.type __tls_get_addr,@function
__tls_get_addr:
  retl
  nop
