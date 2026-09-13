// RUN: %clangxx_tsan -O1 %s -o %t && %run %t 2>&1 | FileCheck %s
// RUN: %clangxx_tsan -O1 %s -o %t.bad \
// RUN:   %if lld %{ -Wl,--image-base=0x100000 %} %else %{ -Wl,-Ttext-segment=0x100000 %}
// RUN: not %run %t.bad 2>&1 | FileCheck %s --check-prefix=BAD
// REQUIRES: sparc64-target-arch || sparcv9-target-arch

#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

static int global;

static void *thread(void *) {
  int stack;
  assert((uintptr_t)&stack >= 0xfff8000100000000ull);
  assert((uintptr_t)&stack < 0xfff8020000000000ull);
  return nullptr;
}

int main() {
  assert((uintptr_t)&global >= 0x020000000000ull);
  assert((uintptr_t)&global < 0x030000000000ull);
  int stack;
  assert((uintptr_t)&stack >= 0x060000000000ull);
  assert((uintptr_t)&stack < 0x080000000000ull);
  void *heap = malloc(32);
  assert(heap);
  assert((uintptr_t)heap >= 0xfff8040000000000ull);
  assert((uintptr_t)heap < 0xfff8050000000000ull);
  free(heap);
  void *mapped = mmap(nullptr, 1 << 20, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  assert(mapped != MAP_FAILED);
  assert((uintptr_t)mapped >= 0xfff8000100000000ull);
  assert((uintptr_t)mapped < 0xfff8020000000000ull);
  *(volatile int *)mapped = 42;
  assert(munmap(mapped, 1 << 20) == 0);
  pthread_t tid;
  assert(pthread_create(&tid, nullptr, thread, nullptr) == 0);
  assert(pthread_join(tid, nullptr) == 0);
  fprintf(stderr, "DONE\n");
}

// CHECK: DONE
// BAD: FATAL: ThreadSanitizer: incompatible SPARC64 memory layout
// BAD-SAME: link the executable at 0x20000000000
