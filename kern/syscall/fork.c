 
/*
from: kern/arch/mips/locore/trap.c
void mips_usermode(struct trapframe *tf)

 * Function for entering user mode.
 *
 * This should not be used by threads returning from traps - they
 * should just return from mips_trap(). It should be used by threads
 * entering user mode for the first time - whether the child thread in
 * a fork(), or into a brand-new address space after exec(), or when
 * starting the first userlevel program.
 *
 * It works by jumping into the exception return code.
 *
 * mips_usermode is common code for this. It cannot usefully be called
 * outside the mips port, but should be called from one of the
 * following places:
 *    - enter_new_process, for use by exec and equivalent.
 *    - enter_forked_process, in syscall.c, for use by fork.


    int as_copy(struct addrspace *src, struct addrspace **ret);
 *    as_copy   - create a new address space that is an exact copy of
 *                an old one. Probably calls as_create to get a new
 *                empty address space and fill it in, but that's up to
 *                you.
 *

 
 * Create a new thread based on an existing one.
 *
 * The new thread has name NAME, and starts executing in function
 * ENTRYPOINT. DATA1 and DATA2 are passed to ENTRYPOINT.
 *
 * The new thread is created in the process P. If P is null, the
 * process is inherited from the caller. It will start on the same CPU
 * as the caller, unless the scheduler intervenes first.
 
int
thread_fork(const char *name,
	    struct proc *proc,
	    void (*entrypoint)(void *data1, unsigned long data2),
	    void *data1, 
		unsigned long data2)
{ 

1. Don't understand what the entry points are for thread_fork()
2. Don't understand how trapframe works with everything else 
3. How do you initialise the FD_TABLE? It's not included in proc_create() in proc.c


pid_t sys_fork(struct trapframe *tf, int *errno) { 

    // TODO:     ENOMEM	Sufficient virtual memory for the new process was not available.
    
    // Create a copy of trapframe 
    parent_tf = kmalloc(sizzeof(struct trapframe)); 
    if (parent_tf == NULL) return ENOMEM; 
    memcpy(parent_tf, tf, sizeof(struct trapframe)); 

    // Check if we still have space for a new process 
    pid_t new_pid = get_pid(); 
    if ((new_pid == -1) { 
        *errno = EMPROC;  // The current user already has too many processes.
        return -1
    }  

    // Create a new process  
    struct proc *new_process;
    int e = proc_clone(currproc, &new_process); // TODO: Write this funct in proc.c
    if (e) return -1;

   In the parent process, the new pid_t is returned 

    return -1 and set errno 
     
}return new_pid;

int execv(userptr program, char **args, int *errno) { 
    replace the currently executing program with a newly loaded program image. 
    Process maintains same process ID 
    

    // Find the file. 
    // Check the args, should be terminated by '\0'
    // copyinstr to copy int argv[] and find int argc 
    error check argv[argc] == "\0"; 

    replace existsing address space with a brand new one for the executable 
    > as_create 


 	ENODEV	The device prefix of program did not exist.
    ENOTDIR	A non-final component of program was not a directory.
    ENOENT	program did not exist.
    EISDIR	program is a directory.
    ENOEXEC	program is not in a recognizable executable file format, was for the wrong platform, or contained invalid fields.
    ENOMEM	Insufficient virtual memory is available.
    E2BIG	The total size of the argument strings exceeeds ARG_MAX.
    EIO	A hard I/O error occurred.
    EFAULT	One of the arguments is an invalid pointer.
}
In __pid_table.c  
Where should this table be stored? Globally like OF_TABLE
struct pid_table {  
    int table[PID_MAX] // Located in limits.h
    
}



*/