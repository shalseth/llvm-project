#include <errno.h>
#include <fcntl.h>
#include <sanitizer/linux_syscall_hooks.h>
#include <signal.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

int myfork() {
  __sanitizer_syscall_pre_fork();
#if defined(__sparc__) && defined(__arch64__)
  // SPARC returns the child indicator in %o1, which syscall() discards.
  register long result asm("o0");
  register long child asm("o1");
  register long number asm("g1") = SYS_fork;
  asm volatile("ta 0x6d\n\t"
               "bcc,pt %%xcc, 1f\n\t"
               " nop\n\t"
               "neg %[result]\n\t"
               "mov 0, %[child]\n"
               "1:"
               : [result] "=r"(result), [child] "=r"(child), "+r"(number)
               :
               : "cc", "memory");
  int res;
  if (result < 0) {
    errno = -result;
    res = -1;
  } else {
    res = child ? 0 : result;
  }
#elif defined(SYS_fork)
  int res = syscall(SYS_fork);
#else
  int res = syscall(SYS_clone, SIGCHLD, 0);
#endif
  __sanitizer_syscall_post_fork(res);
  return res;
}

int mypipe(int pipefd[2]) {
  __sanitizer_syscall_pre_pipe(pipefd);
  int res = syscall(SYS_pipe2, pipefd, 0);
  __sanitizer_syscall_post_pipe(res, pipefd);
  return res;
}

int myclose(int fd) {
  __sanitizer_syscall_pre_close(fd);
  int res = syscall(SYS_close, fd);
  __sanitizer_syscall_post_close(res, fd);
  return res;
}

ssize_t myread(int fd, void *buf, size_t count) {
  __sanitizer_syscall_pre_read(fd, buf, count);
  ssize_t res = syscall(SYS_read, fd, buf, count);
  __sanitizer_syscall_post_read(res, fd, buf, count);
  return res;
}

ssize_t mywrite(int fd, const void *buf, size_t count) {
  __sanitizer_syscall_pre_write(fd, buf, count);
  ssize_t res = syscall(SYS_write, fd, buf, count);
  __sanitizer_syscall_post_write(res, fd, buf, count);
  return res;
}
