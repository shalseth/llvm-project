// RUN: %clang -### %s --target=x86_64-unknown-linux -fsanitize=thread -shared-libsan 2>&1 | FileCheck %s --check-prefix=EXE
// RUN: %clang -### %s --target=sparcv9-unknown-linux -fsanitize=thread -shared-libsan 2>&1 | FileCheck %s --check-prefix=EXE
// RUN: %clang -### %s --target=x86_64-unknown-linux -fsanitize=thread -shared-libsan -shared 2>&1 | FileCheck %s --check-prefix=DSO
// RUN: %clang -### %s --target=sparcv9-unknown-linux -fsanitize=thread -shared-libsan -shared 2>&1 | FileCheck %s --check-prefix=DSO
// RUN: %clang -### %s --target=aarch64-linux-android -fsanitize=thread -shared-libsan 2>&1 | FileCheck %s --check-prefix=DSO
// RUN: %clang -### %s --target=x86_64-unknown-linux -fsanitize=thread -shared-libsan -fno-sanitize-link-runtime 2>&1 | FileCheck %s --check-prefix=NO-RT

// EXE: "{{[^"]*}}libclang_rt.tsan{{[^"]*}}.so"
// EXE-SAME: "--whole-archive" "{{[^"]*}}libclang_rt.tsan-preinit{{[^"]*}}.a" "--no-whole-archive"
// DSO-NOT: tsan-preinit
// DSO: "{{[^"]*}}libclang_rt.tsan{{[^"]*}}.so"
// DSO-NOT: tsan-preinit
// NO-RT-NOT: libclang_rt.tsan
