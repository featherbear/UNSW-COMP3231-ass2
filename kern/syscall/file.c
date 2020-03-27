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
#include <spinlock.h>
#include <copyinout.h>


/* 
 * Return ENOSYS for any possible coding errors for debugging purposes.
*/

int sys_open(userptr_t filename, int flags, mode_t mode, int *retval) { 
    

    int fd;
    if ((*retval = get_free_fd(&fd)) != 0) {
        kprint("Error with function: get_free_fd()\n"); 
        return -1;
    } 

    // TODO: Assign the FD
    // TODO: Should be in another function // What should be in another function? 
    // TODO: Assign the OD

    // Allocate the file on the OF_TABLE 
    int of;
    
    if ((*retval = get_free_of(&of)) != 0) {
        print("Error with function: get_free_of()\n"); 
        return -1; 
    } 
    
    
    // // Set the vnode for the open file
    struct vnode *vnode; 
    if ((*retval = vfs_open(filename, flags, mode, &new_vnode))) { 
        return -1;  
    }

    // Setup new open_file 
    struct open_file *new_file = kmalloc(sizeof(*new_file));
    new_file->fp = 0; 
    new_file->flags = flags;
    new_file->vnode = vnode;
    new_file->lock = lock_create("file lock");

    // Success     
    return fd;
    
} 
/* #region sys_close */
int sys_close(int fd, *retval) { 
    
    // Get the file 
    struct open_file *file; 
    
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


    // TODO: Check other references to the vnode - remove from OF table if all references have finished.
    
    // If there were originally no more free fd's, assign next_fd to be the fd-to-be-removed
    if (curproc->p_fdtable->next_fd == -1) {
        curproc->p_fdtable->next_fd = fd;
    }

    // Success 
    return 0; 
}
/* #endregion */

int sys_read(int fd, userptr_t buf, int buflen, int *retval) { 
    
    int e;

    // Get the file 
    struct open_file *file; 

    *retval = get_open_file_from_fd(fd, &file);
    if (*retval != 0) {
        return -1;
    }

    // Check the permissions 
    int flags = file->flags;  
    if !(flags == O_RDONLY || flags == O_RDWR) { 
        *retval = EPERM; 
        return -1;  
    } 

    // TODO: We shouldn't need to change the locks here right? Double-checkin' 

    // Prepare the uio 
    struct uio *new_uio; 
    struct iovev new_iov; 

    // TODO: Write this helper function (also need to declare it in file.h) 
    // Got it straight from the assign 2 video. 
    struct *uio uio_init(
        struct iovec *iov, 
        struct iou *u, 
        userptr_t buf, 
        size_t len, 
        off_t offset, 
        enum uio_rw rw
    ) { 
        iov->iov_ubase = buf; 
        iov->iov_len = len; 
        u->uio_iov = iov; 
        u->uio_iovcnt = 1; 
        u->uio_offset = offset; 
        u->
        
        // fill in the uio details 
    }
    
    
    // Call VOP to do the reading 
    e = VOP_READ(file->vnode, new_uio); 
    if (e) { 
        *retval = e; 
        return -1; 
    } 

    // Success
    return 0;     
}

// TODO: Write a prototype in file.h
int sys_write(int fd, userptr_t buf, size_t buflen, int *retval) {  
    
    // Get the file 
    struct open_file *file; 
    
    *retval = get_open_file_from_fd(fd, &file);
    if (*retval != 0) {
        return -1;
    }   

    // Check if we have the permission 
    int flags = file->flags; 

    if !(flags == O_WRONLY || flags == O_RDWR) {
        *retval = EPERM; 
        return -1; 
    }

    // Copy in the data from User-land into Kernel-land 
    char kernel_buf[buflen];

    e = copyin(buf, kernel_buf, sizeof(kernel_buf)); 
    if (e) {  

        // Handles potential EFAULT (Address error of buf ptr)
        *retval = e; 
        return -1; 
    } 

    uio_init()

    // TODO: Acquire the file lock before we start changing it 




    /* 
        TODO:  ENOSPC --> No free space remaining on the filesystem 
        containing the file.. 
        How should we implement this. 
    */ 




/* 
The flags argument should consist of one of
 	    O_RDONLY	Open for reading only.
        O_WRONLY	Open for writing only.
        O_RDWR	Open for reading and writing.
It may also have any of the following flags OR'd in:
        O_CREAT	Create the file if it doesn't exist.
        O_EXCL	Fail if the file already exists.
        O_TRUNC	Truncate the file to length 0 upon open.
        O_APPEND	Open the file in append mode.
        O_EXCL is only meaningful if O_CREAT is also used.
*/
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

struct open_file_table {
    spinlock_t lock;
    struct open_file_node *head;
    struct open_file_node *tail;
};

struct open_file_node {
    struct open_file_node *prev;
    struct open_file_node *next;
    struct open_file *entry;
}



int create_open_file_table() {
    if (open_file_table != NULL) {
        return ENOSYS;
    } 
    
    open_file_table = kmalloc(sizeof(struct open_file_table))
    if (open_file_table == NULL) {
        return ENOMEM;
    }

    spinlock_init(&open_file_table->lock);
    open_file_table->head = NULL;
    open_file_table->tail = NULL;

    return 0;
}

void destroy_open_file_table() {
    // TODO: Should we free the nodes?
    spinlock_cleanup(&open_file_table->lock)
    kfree(open_file_table);
}

/* #endregion */

/* #region File Helpers */

int get_open_file_from_fd(int fd, struct open_file **open_file) {

    struct file_descriptor_table *fdtable = curproc->p_fdtable;

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

