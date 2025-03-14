/* Wrapper for clone3 syscall to be able to call a function from child process
 * Taken from CRIU project <https://criu.org/> under GPLv2 */

/*
 * Documentation copied from glibc sysdeps/unix/sysv/linux/x86_64/clone.S
 * The kernel expects:
 * rax: system call number
 * rdi: flags
 * rsi: child_stack
 * rdx: TID field in parent
 * r10: TID field in child
 * r8:    thread pointer
 *
 * int clone(unsigned long clone_flags, unsigned long newsp,
 *           int *parent_tidptr, int *child_tidptr,
 *           unsigned long tls);
 */

#ifdef __x86_64__

/* clang-format off */
#define RUN_CLONE_RESTORE_FN(ret, clone_flags, new_sp, ptr_parent_tid, ptr_child_tid,   \
                thread_args, clone_restore_fn)        \
     asm volatile(                            \
            "clone_emul:                \n"    \
            "movq %2, %%rsi                \n"    \
            "subq $16, %%rsi                \n"    \
            "movq %6, %%rdi                \n"    \
            "movq %%rdi, 8(%%rsi)            \n"    \
            "movq %5, %%rdi                \n"    \
            "movq %%rdi, 0(%%rsi)            \n"    \
            "movq %1, %%rdi                \n"    \
            "movq %3, %%rdx                \n"    \
            "movq %4, %%r10                \n"    \
            "movl $220, %%eax /* __NR_clone syscall */   \n"    \
            "syscall                    \n"    \
                                     \
            "testq %%rax,%%rax                \n"    \
            "jz thread_run                \n"    \
                                     \
            "movq %%rax, %0                \n"    \
            "jmp clone_end                \n"    \
                                     \
            "thread_run:                \n"    \
            "xorq %%rbp, %%rbp                \n"    \
            "popq %%rax                \n"    \
            "popq %%rdi                \n"    \
            "callq *%%rax                \n"    \
                                     \
            "clone_end:                \n"    \
            : "=r"(ret)                    \
            : "g"(clone_flags),                \
              "g"(new_sp),                    \
              "g"(ptr_parent_tid),                \
              "g"(ptr_child_tid),            \
              "g"(clone_restore_fn),                \
              "g"(thread_args)                \
            : "rax", "rcx", "rdi", "rsi", "rdx", "r10", "r11", "memory")


/* int clone3(struct clone_args *args, size_t size) */
#define RUN_CLONE3_RESTORE_FN(ret, clone_args, size, args,        \
                                 clone_restore_fn)                \
    asm volatile(                            \
             "clone3_emul:                \n"    \
    /*
     * Prepare stack pointer for child process. The kernel does
     * stack + stack_size before passing the stack pointer to the
     * child process. As we have to put the function and the
     * arguments for the new process on that stack we have handle
     * the kernel's implicit stack + stack_size.
     */                                \
             "movq (%3), %%rsi    /* new stack pointer */    \n"    \
    /* Move the stack_size to %rax to use later as the offset */    \
             "movq %4, %%rax                \n"    \
    /* 16 bytes are needed on the stack for function and args */    \
             "subq $16, (%%rsi, %%rax)            \n"    \
             "movq %6, %%rdi    /* thread args */    \n"    \
             "movq %%rdi, 8(%%rsi, %%rax)        \n"    \
             "movq %5, %%rdi    /* thread function */    \n"    \
             "movq %%rdi, 0(%%rsi, %%rax)        \n"    \
    /*
     * The stack address has been modified for the two
     * elements above (child function, child arguments).
     * This modified stack needs to be stored back into the
     * clone_args structure.
     */                                \
             "movq (%%rsi), %3                \n"    \
    /*
     * Do the actual clone3() syscall. First argument (%rdi) is
     * the clone_args structure, second argument is the size
     * of clone_args.
     */                                \
             "movq %1, %%rdi    /* clone_args */    \n"    \
             "movq %2, %%rsi    /* size */        \n"    \
             "movl $435, %%eax  /* __NR_clone3 */  \n"    \
             "syscall                    \n"    \
    /*
     * If clone3() was successful and if we are in the child
     * '0' is returned. Jump to the child function handler.
     */                                \
             "testq %%rax,%%rax                \n"    \
             "jz thread3_run                \n"    \
    /* Return the PID to the parent process. */            \
             "movq %%rax, %0                \n"    \
             "jmp clone3_end                \n"    \
                                    \
             "thread3_run:    /* Child process */    \n"    \
    /* Clear the frame pointer */                    \
             "xorq %%rbp, %%rbp                \n"    \
    /* Pop the child function from the stack */            \
             "popq %%rax                \n"    \
    /* Pop the child function arguments from the stack */        \
             "popq %%rdi                \n"    \
    /* Run the child function */                    \
             "callq *%%rax                \n"    \
    /*
     * If the child function is expected to return, this
     * would be the place to handle the return code. In CRIU's
     * case the child function is expected to not return
     * and do exit() itself.
     */                                \
                                    \
             "clone3_end:                \n"    \
             : "=r"(ret)                    \
    /*
     * This uses the "r" modifier for all parameters
     * as clang complained if using "g".
     */                                \
             : "r"(&clone_args),                \
               "r"(size),                    \
               "r"(&clone_args.stack),                \
               "r"(clone_args.stack_size),            \
               "r"(clone_restore_fn),                \
               "r"(args)                    \
             : "rax", "rcx", "rdi", "rsi", "rdx", "r10", "r11", "memory")

#elif __i386__

#define RUN_CLONE_RESTORE_FN(ret, clone_flags, new_sp, ptr_parent_tid, ptr_child_tid,   \
                thread_args, clone_restore_fn)        \
    ret = clone(clone_restore_fn, reinterpret_cast<void*>(new_sp), clone_flags, thread_args, ptr_parent_tid, \
                NULL, ptr_child_tid)

#define RUN_CLONE3_RESTORE_FN(ret, clone_args, size, args,        \
                                 clone_restore_fn)                \


#endif
