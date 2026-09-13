// RUN: %clang --target=sparcv9-unknown-linux-gnu -fsanitize=thread -fuse-ld=lld --ld-path=%clang -### %s 2>&1 | FileCheck %s --check-prefix=LLD
// RUN: %clang --target=sparcv9-unknown-linux-gnu -fsanitize=thread -fuse-ld=lld --ld-path=%clang -pie -### %s 2>&1 | FileCheck %s --check-prefix=LLD
// RUN: %clang --target=sparcv9-unknown-linux-gnu -fsanitize=thread -fuse-ld=lld --ld-path=%clang -shared -### %s 2>&1 | FileCheck %s --check-prefix=NO-PLACEMENT
// RUN: %clang --target=sparcv9-unknown-linux-gnu -fsanitize=thread -fuse-ld=lld --ld-path=%clang -r -### %s 2>&1 | FileCheck %s --check-prefix=NO-PLACEMENT
// RUN: %clang --target=sparcv9-unknown-linux-gnu -fsanitize=thread -### %s 2>&1 | FileCheck %s --check-prefix=TSAN
// RUN: %clang --target=sparc64-unknown-linux-gnu -fsanitize=thread -### %s 2>&1 | FileCheck %s --check-prefix=TSAN
// RUN: %clang --target=sparcv9-unknown-linux-gnu -fsanitize=thread -no-pie -### %s 2>&1 | FileCheck %s --check-prefix=TSAN
// RUN: %clang --target=sparcv9-unknown-linux-gnu -fsanitize=thread -shared -### %s 2>&1 | FileCheck %s --check-prefix=NO-PLACEMENT
// RUN: %clang --target=sparcv9-unknown-linux-gnu -fsanitize=thread -r -### %s 2>&1 | FileCheck %s --check-prefix=NO-PLACEMENT
// RUN: %clang --target=sparcv9-unknown-linux-gnu -### %s 2>&1 | FileCheck %s --check-prefix=NO-PLACEMENT
// RUN: %clang --target=x86_64-unknown-linux-gnu -fsanitize=thread -### %s 2>&1 | FileCheck %s --check-prefix=NO-PLACEMENT
// RUN: not %clang --target=sparc-unknown-linux-gnu -fsanitize=thread -### %s 2>&1 | FileCheck %s --check-prefix=REJECTED
// RUN: not %clang --target=sparcv9-unknown-freebsd -fsanitize=thread -### %s 2>&1 | FileCheck %s --check-prefix=REJECTED

// TSAN: "-fsanitize=thread"
// TSAN: "-Ttext-segment=0x20000000000"
// TSAN: libclang_rt.tsan
// NO-PLACEMENT-NOT: {{-Ttext-segment|--image-base}}
// NO-PLACEMENT: "-cc1"
// NO-PLACEMENT-NOT: {{-Ttext-segment|--image-base}}
// LLD: "-fsanitize=thread"
// LLD-NOT: -Ttext-segment
// LLD: "--image-base=0x20000000000"
// LLD-NOT: "-pie"
// LLD: libclang_rt.tsan
// LLD-NOT: "-pie"
// REJECTED: unsupported option '-fsanitize=thread'

int main(void) { return 0; }
