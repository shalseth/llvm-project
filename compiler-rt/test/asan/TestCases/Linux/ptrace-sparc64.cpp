// RUN: %clangxx_asan -O0 %s -o %t
// RUN: %run %t
// RUN: not %run %t getregs 2>&1 | FileCheck %s
// RUN: not %run %t setregs 2>&1 | FileCheck %s
// RUN: not %run %t getfpregs 2>&1 | FileCheck %s
// RUN: not %run %t setfpregs 2>&1 | FileCheck %s
// REQUIRES: target={{(sparcv9|sparc64).*linux.*}}

#include <assert.h>
#include <sanitizer/asan_interface.h>
#include <signal.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv) {
  pid_t pid = fork();
  assert(pid >= 0);
  if (pid == 0) {
    assert(ptrace(PTRACE_TRACEME, 0, nullptr, nullptr) == 0);
    raise(SIGSTOP);
    _exit(0);
  }
  int status;
  assert(waitpid(pid, &status, 0) == pid && WIFSTOPPED(status));

  unsigned long regs[19], fpregs[33];
  assert(ptrace(PTRACE_GETREGS, pid, regs, nullptr) == 0);
  assert(ptrace(PTRACE_GETFPREGS, pid, fpregs, nullptr) == 0);
  if (argc > 1) {
    bool fp = strstr(argv[1], "fp") != nullptr;
    bool set = argv[1][0] == 's';
    unsigned long *buffer = fp ? fpregs : regs;
    size_t count = fp ? 33 : 19;
    __asan_poison_memory_region(buffer + count - 1, sizeof(*buffer));
    auto request = fp ? (set ? PTRACE_SETFPREGS : PTRACE_GETFPREGS)
                      : (set ? PTRACE_SETREGS : PTRACE_GETREGS);
    ptrace(request, pid, buffer, nullptr);
    // CHECK: ERROR: AddressSanitizer: use-after-poison
    // CHECK: {{.*ptrace-sparc64.cpp:}}[[@LINE-2]]
  } else {
    assert(ptrace(PTRACE_SETREGS, pid, regs, nullptr) == 0);
    assert(ptrace(PTRACE_SETFPREGS, pid, fpregs, nullptr) == 0);
  }
  assert(ptrace(PTRACE_CONT, pid, nullptr, nullptr) == 0);
  assert(waitpid(pid, &status, 0) == pid && WIFEXITED(status));
  return 0;
}
