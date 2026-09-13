// RUN: %clangxx_tsan -O1 %s -o %t && %run %t 2>&1 | FileCheck %s
// REQUIRES: sparc64-target-arch || sparcv9-target-arch

#include <assert.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>

static volatile sig_atomic_t signals;

static void handler(int) { ++signals; }

__attribute__((noinline)) static void jump(sigjmp_buf env, unsigned depth) {
  volatile unsigned local = depth;
  if (depth)
    jump(env, depth - 1);
  else {
    raise(SIGUSR1);
    siglongjmp(env, 42);
  }
  assert(local == depth);
}

__attribute__((noinline)) static void exercise(unsigned depth, unsigned mode) {
  volatile unsigned local = depth;
  if (depth) {
    exercise(depth - 1, mode);
  } else {
    sigjmp_buf env;
    volatile int result;
    switch (mode) {
    case 0:
      result = setjmp(env);
      break;
    case 1:
      result = _setjmp(env);
      break;
    case 2:
      result = sigsetjmp(env, 0);
      break;
    default:
      result = sigsetjmp(env, 1);
      break;
    }
    if (!result) {
      sigset_t mask;
      sigemptyset(&mask);
      sigaddset(&mask, SIGUSR2);
      assert(sigprocmask(SIG_BLOCK, &mask, nullptr) == 0);
      jump(env, 40);
    }
    assert(result == 42);
    sigset_t mask;
    assert(sigprocmask(SIG_SETMASK, nullptr, &mask) == 0);
    assert(sigismember(&mask, SIGUSR2) == (mode == 3 ? 0 : 1));
    sigemptyset(&mask);
    assert(sigprocmask(SIG_SETMASK, &mask, nullptr) == 0);
  }
  assert(local == depth);
}

int main() {
  signal(SIGUSR1, handler);
  for (unsigned i = 0; i < 100; ++i)
    for (unsigned mode = 0; mode < 4; ++mode)
      exercise(30, mode);
  assert(signals == 400);
  fprintf(stderr, "DONE\n");
}

// CHECK-NOT: WARNING: ThreadSanitizer
// CHECK: DONE
// CHECK-NOT: WARNING: ThreadSanitizer
