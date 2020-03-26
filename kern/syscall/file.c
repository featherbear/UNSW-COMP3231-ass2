#include <types.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <kern/limits.h>
#include <kern/stat.h>
#include <kern/seek.h>
#include <kern/unistd.h>
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

//
#define NULL 0

// open(const char *filename, int flags, int mode);
int sys_open((userptr_t), int, int, *int); 

// close(int fd);
int sys_close(int, *int); 

// ssize_t read(int fd, void *buf, size_t buflen);
int sys_read(int, void *, size_t buflen, *int); 

// int sys_write(*int); 
// int sys_lseek(*int); 
// int sys_dup2(*int); 

/*
 * Add your file-related functions here ...
 */ 


/* 
 * Return ENOSYS for any possible coding errors for debugging purposes.
 * Double checking: to malloc is it:
    struct data_type *dt = malloc(sizeof(data_type)); 
   If not, need to change all of the

*/
int sys_open((userptr_t)filename, int flags, mode_t mode, int *retval) { 
    
    // Get the File Descriptor Table from process and allocate it on the FD_TABLE
    int *map = curproc->p_fdtable->map; 
    
    int fd;
    if ((*retval = get_free_fd(&fd)) != 0) {
        kprint("Error with function: get_free_fd()\n"); 
        return -1;
    } 
    
    // TODO: Should be in another function //

    // Allocate the file on the OF_TABLE 
    int of;
    
    if ((*retval = get_free_of(&of)) != 0) {
        print("Error with function: get_free_of()\n"); 
        return -1; 
    } 
    
    
    // // Set the vnode for the open file
    // struct vnode *new_vnode = malloc(sizeof(vnode)); 
    // int e = vfs_open(); // TODO: Don't know if this line is correct. 
    //                         // Absolutely not (:


    struct vnode *vnode;
    int e = vfs_open(..., &vnode);
                            


    // Setup new open_file 
    struct open_file *new_file = malloc(sizeof(struct open_file));  // TODO: Change to kmalloc
    new_file->fp = fd; 
    new_file->flags = flags;
    new_file->vnode = vnode;


    new_file->lock = lock_create("file lock");
    /* 
        TODO: Create a lock for the file
        "struct lock *lock;   // Shared access "
    */

    // Success     
    return fd;
    
} 

int sys_close(int fd, &retval) { 
    
    // Get the file 
    struct open_file *file = NULL; 
    int e = get_open_file_from_fd(fd, &file); struct open_file *file;
    
    *retval = get_open_file_from_fd(fd, &file);
    if (*retval != 0) {
        return -1;
    }

    lock_acquire(file->lock);
    // TODO: RELEASE THE LOCK

    // Clean the vnode
    struct vnode *vn = file->vnode; 
    int e = vfs_close(vn); 
    if (e) { 
        retval = e; 
        return -1; 
    }

    /* Cleaning code TODO: Double Check if missing anything */

    // TODO: Check other references to the vnode - remove from OF table if all references have finished.
    
    // If there were originally no more free fd's, assign next_fd to be the fd-to-be-removed
    if (curproc->p_fdtable->next_fd == -1) {
        curproc->p_fdtable->next_fd = fd;
    }

    // Success 
    return 0; 
}

int sys_read(int fd, buff) { 
    
    

    

}


//

/* #region FD Layer */

struct file_descriptor_table *create_file_table() {
    if (curproc->p_fdtable != NULL) {
        KASSERT(0);
    }

    struct file_descriptor_table *fdtable = kmalloc(sizeof(struct file_descriptor_table));

    fdtable->map[0] = STDIN_FILENO;
    fdtable->map[1] = STDOUT_FILENO;
    fdtable->map[2] = STDERR_FILENO;
    fdtable->next_fd = 3; // 0, 1, 2 are used for STDIN, STDOUT, STDERR

    curproc->p_fdtable = fdtable;
    
    return fdtable;
}

void destroy_file_table() {
    if (curproc->p_fdtable == NULL) {
        KASSERT(0);
    }
    
    kfree(curproc->p_fdtable);
    curproc->p_fdtable = NULL;
}


int get_free_fd(int *retval) {
    struct file_descriptor_table *table = curproc->p_fdtable;

    int *map = table->map;    
    int free_fd = table->next_fd;

    if (free_fd == -1) {
        *retval = EMFILE;
        return -1;
    }

    // Check for the next free file pointer
    int next_fd = free_fd;
    do next_fd = (next_fd + 1) % OPEN_MAX;
    while (map[next_fd] != -1 && next_fd != free_fd) ; // TODO: +1?
    
    table->next_fd = (next_fd == free_fd) ? -1 : next_fd;
    *retval = free_fd;
    return 0;
}

/* #endregion */

/* #region OF Layer */

int create_open_file_table() {
    if (open_file_table != NULL) {
        return ENOSYS;
    } 
    
    open_file_table = kmalloc(sizeof(struct open_file) * OPEN_MAX);
    if (open_file_table == NULL) {
        return ENOMEM;
    }

    return 0;
}

void destroy_open_file_table();

/* #endregion */

/* #region File Helpers */

int get_open_file_from_fd(int fd, struct open_file **open_file) {

    struct file_descriptor_table *fdtable = curproc->p_fdtable;
    int *map = fdtable->map; 

    int OF_index = fdtable->map[fd];
    if (OF_index == -1) {
        return ENOSYS;
    }
    if (OF_index >= OPEN_MAX) {
        return EBADF;
    }

    struct open_file *file = &open_file_table[OF_index];
    if (file == NULL) {
        return ENOSYS;
    }
    
    *open_file = file;
    return 0;
}

/* #endregion */

