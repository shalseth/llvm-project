// RUN: %clangxx_tsan -O1 %s -o %t && %run %t 2>&1 | FileCheck %s
// REQUIRES: sparc64-target-arch || sparcv9-target-arch

#include <assert.h>
#include <pthread.h>
#include <sanitizer/tsan_interface_atomic.h>
#include <sched.h>
#include <stdio.h>

using U128 = unsigned __int128;
static __tsan_atomic128 counter;
static __tsan_atomic128 ready;
static int payload;
static const __tsan_atomic128 step = (__tsan_atomic128(1) << 64) | 1;
static const int relaxed = __tsan_memory_order_relaxed;

static void *increment(void *) {
  for (int i = 0; i < 2000; ++i) {
    __tsan_atomic128_fetch_add(&counter, step, relaxed);
    U128 value = __tsan_atomic128_load(&counter, relaxed);
    assert((value >> 64) == (unsigned long long)value);
  }
  return nullptr;
}

static void *publish(void *) {
  payload = 42;
  __tsan_atomic128_store(&ready, step, __tsan_memory_order_release);
  return nullptr;
}

int main() {
  pthread_t threads[4];
  for (auto &thread : threads)
    assert(pthread_create(&thread, nullptr, increment, nullptr) == 0);
  for (auto &thread : threads)
    assert(pthread_join(thread, nullptr) == 0);
  assert(__tsan_atomic128_load(&counter, relaxed) == step * 8000);
  __tsan_atomic128 expected = step * 8000;
  const __tsan_atomic128 value = (__tsan_atomic128(0x1234) << 80) | 42;
  assert(__tsan_atomic128_compare_exchange_strong(
      &counter, &expected, value, __tsan_memory_order_seq_cst, relaxed));
  assert(__tsan_atomic128_exchange(&counter, step, relaxed) == value);
  assert(__tsan_atomic128_fetch_sub(&counter, step, relaxed) == step);
  assert(__tsan_atomic128_fetch_or(&counter, value, relaxed) == 0);
  assert(__tsan_atomic128_fetch_xor(&counter, step, relaxed) == value);
  assert(__tsan_atomic128_fetch_and(&counter, value, relaxed) ==
         (value ^ step));
  assert(__tsan_atomic128_load(&counter, relaxed) == ((value ^ step) & value));
  assert(pthread_create(&threads[0], nullptr, publish, nullptr) == 0);
  while (!__tsan_atomic128_load(&ready, __tsan_memory_order_acquire))
    sched_yield();
  assert(payload == 42);
  assert(pthread_join(threads[0], nullptr) == 0);
  puts("DONE");
}

// CHECK-NOT: WARNING: ThreadSanitizer
// CHECK: DONE
// CHECK-NOT: WARNING: ThreadSanitizer
