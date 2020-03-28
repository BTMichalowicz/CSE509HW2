#include"urlextend.h"



int main(int argc, char**argv){
    if (argc<=1){
        FATAL("too few arguments: %d", argc);
    }

    unsigned int pid = fork();
    switch(pid){
        case -1:
            FATAL("%s", strerror(errno));
            break;
        case 0:
            ptrace(PTRACE_TRACEME,0,0,0);
            execvp(argv[1], argv+1);
            FATAL("%s", strerror(errno));
    }

    waitpid(pid,0,0);
    ptrace(PTRACE_SETOPTIONS, pid,0,PTRACE_O_EXITKILL);


    for (;;) {
        /* Enter next system call */
        if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1)
            FATAL("%s", strerror(errno));
        if (waitpid(pid, 0, 0) == -1)
            FATAL("%s", strerror(errno));

        /* Gather system call arguments */
        struct user_regs_struct regs;
        if (ptrace(PTRACE_GETREGS, pid, 0, &regs) == -1)
            FATAL("%s", strerror(errno));
        long syscall = regs.orig_eax;

        /* Print a representation of the system call */
        fprintf(stderr, "%ld(%ld, %ld, %ld, %ld, %ld, %ld)",
                syscall,
                (long)regs.edi, (long)regs.esi, (long)regs.edx,
                (long)regs.esp, (long)regs.esp,  (long)regs.eip);

        /* Run system call and stop on exit */
        if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1)
            FATAL("%s", strerror(errno));
        if (waitpid(pid, 0, 0) == -1)
            FATAL("%s", strerror(errno));

        /* Get system call result */
        if (ptrace(PTRACE_GETREGS, pid, 0, &regs) == -1) {
            fputs(" = ?\n", stderr);
            if (errno == ESRCH)
                exit(regs.edi); // system call was _exit(2) or similar
            FATAL("%s", strerror(errno));
        }

        /* Print system call result */
        fprintf(stderr, " = %ld\n", (long)regs.eax);
    }


    return 0;
}
