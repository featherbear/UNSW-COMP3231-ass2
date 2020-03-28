#include <__file_descriptor_table.h>
#include <__open_file_table.h>

#include <types.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <kern/limits.h>
#include <kern/stat.h>
#include <kern/seek.h>
#include <kern/unistd.h>
#include <stat.h>
#include <lib.h>
#include <uio.h>
#include <thread.h>
#include <current.h>
#include <synch.h>
#include <vfs.h>
#include <vnode.h>
#include <file.h>
#include <syscall.h>
#include <copyinout.h>
#include <proc.h>
#include <stat.h>
#include <uio.h>
#include <current.h>


// TODO: Meta
struct file_descriptor_table *create_file_table() {


    //TODO: MEMORY MANAGE THIS ENTIRE FUNCTION
    //TODO: MEMORY (commander) THIS ENTIRE FUNCTION
    //TODO: MEMORY (commander) THIS ENTIRE FUNCTION
    //TODO: MEMORY (commander) THIS ENTIRE FUNCTION
    //TODO: MEMORY (commander) THIS ENTIRE FUNCTION
    //TODO: MEMORY (commander) THIS ENTIRE FUNCTION
    //TODO: MEMORY (commander) THIS ENTIRE FUNCTION
    //TODO: MEMORY (commander) THIS ENTIRE FUNCTION

    struct file_descriptor_table *fdtable = kmalloc(sizeof(*fdtable));

    fdtable->map = (struct open_file **) kmalloc(OPEN_MAX * sizeof(struct open_file *));
    bzero(fdtable->map, OPEN_MAX * sizeof(struct open_file *));


    if (fdtable == NULL) {
        return NULL; // Let proc.c deal with this
        // return ENOMEM;
    }

    spinlock_init(*(&fdtable->lock));

    create_open_file();
/*
    // FD_LOCK_ACQUIRE();
    struct open_file *stdin_file = create_open_file();


    char stdinPath[] = "con:";
    if (vfs_open(stdinPath, O_RDONLY, 0, &stdin_file->vnode) != 0) return NULL;  

    struct open_file *stdout_file = create_open_file();
    char stdoutPath[] = "con:";
    if (vfs_open(stdoutPath, O_WRONLY, 0, &stdout_file->vnode) != 0) return NULL;  


    struct open_file *stderr_file = create_open_file();
    char stderrPath[] = "con:";
    if (vfs_open(stderrPath, O_WRONLY, 0, &stderr_file->vnode) != 0) return NULL;  

    // FD_LOCK_RELEASE();
    
    assign_fd(STDIN_FILENO, stdin_file);
    assign_fd(STDOUT_FILENO, stdout_file);
    assign_fd(STDERR_FILENO, stderr_file);
    fdtable->next_fd = 3;
*/
    return fdtable;
}

void destroy_file_table(struct file_descriptor_table *fdtable) {
    spinlock_cleanup(*(&fdtable->lock));
    kfree(fdtable->map);
    kfree(fdtable);
}


int get_free_fd(int *errno) {
    struct file_descriptor_table *fdtable = curproc->p_fdtable;

    struct open_file **map = fdtable->map;    

    FD_LOCK_ACQUIRE();
    fd_t free_fd = fdtable->next_fd;

    if (free_fd == -1) {
        *errno = EMFILE;
        FD_LOCK_RELEASE();
        return -1;
    }

    // Check for the next free file pointer
    fd_t next_fd = free_fd;
    do next_fd = (next_fd + 1) % OPEN_MAX;
    while (map[next_fd] != NULL && next_fd != free_fd) ; // FIXME: +1? TESTME
    
    fdtable->next_fd = (next_fd == free_fd) ? -1 : next_fd;
    *errno = free_fd;

    FD_LOCK_RELEASE();

    return 0;
}

// TODO: Meta
bool check_invalid_fd(fd_t fd) {
    return (fd < 0 || fd >= OPEN_MAX) ? EBADF : 0;
}


void assign_fd(fd_t fd, struct open_file *open_file) {
    struct file_descriptor_table *fdtable = curproc->p_fdtable;
     
    FD_LOCK_ACQUIRE();

    // TODO: Check if it used?
    fdtable->map[fd] = open_file;
    
    FD_LOCK_ACQUIRE();
}


/* Retrieves `open_file` from given file descriptor. If invalid, return EBADF */
int get_open_file_from_fd(fd_t fd, struct open_file **open_file) {

    struct file_descriptor_table *fdtable = curproc->p_fdtable;

    if ((check_invalid_fd(fd)) || (*open_file = fdtable->map[fd]) == NULL) {
        return EBADF;
    }

    return 0;
}