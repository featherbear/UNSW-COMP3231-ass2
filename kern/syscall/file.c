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

#define FD_LOCK_ACQUIRE() (spinlock_acquire(&curproc->p_fdtable->lock))
#define FD_LOCK_RELEASE() (spinlock_release(&curproc->p_fdtable->lock))
#define OF_LOCK_ACQUIRE() (spinlock_acquire(&open_file_table->lock))
#define OF_LOCK_RELEASE() (spinlock_release(&open_file_table->lock))

/* 
 * Return ENOSYS for any possible coding errors for debugging purposes.
*/

fd_t sys_open(userptr_t filename, int flags, mode_t mode, int *retval) { 
    
    fd_t fd;
    if ((*retval = get_free_fd(&fd)) != 0) {
        kprint("Error with function: get_free_fd()\n"); 
        return -1;
    } 

    struct open_file *open_file = create_open_file();

    if ((*retval = vfs_open(filename, flags, mode, &open_file->vnode))) { 
        return -1;  
    }

    open_file->fp = 0;
    open_file->flags = flags;

    assign_fd(fd, open_file);

    return fd;
    
}

int sys_close(fd_t fd, *retval) { 
    
    // Get the file 
    struct open_file *file;     
    if ((*retval = get_open_file_from_fd(fd, &file)) != 0) {
        return -1;
    }

    lock_acquire(file->lock); /////////////////////////

    if ((*retval = vfs_close(file->vnode)) != 0) { 
        return -1; 
    }

    FD_LOCK_ACQUIRE();
    assign_fd(fd, NULL);

    // If there were originally no more free fd's, assign next_fd to be the fd-to-be-removed
    if (curproc->p_fdtable->next_fd == -1) {
        curproc->p_fdtable->next_fd = fd;
    }
    FD_LOCK_RELEASE();

    // Check other references to the vnode - remove from OF table if all references have finished.
    OF_LOCK_ACQUIRE();
    // TODO:
    // Destroy node
    // Destroy DLL linked list
    // // Move prev and next ptrs
    OF_LOCK_RELEASE();


    // Success 
    return 0; 
}

int sys_read(fd_t fd, userptr_t buf, int buflen, int *retval) { 
    
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
    if ((*retval = VOP_READ(file->vnode, new_uio)) != 0) { 
        return -1; 
    } 

    // Success
    return 0;     
}

int sys_write(fd_t fd, userptr_t buf, size_t buflen, int *retval) {  
    
    // Get the file 
    struct open_file *file; 
    
    *retval = get_open_file_from_fd(fd, &file);
    if (*retval != 0) {
        return -1;
    }   

    // Check if we have the permission 
    int flags = file->flags; 

    // FIXME:         O_APPEND	Open the file in append mode.
    // #define MATCH_BITMASK(value, mask) (value & mask == mask)
    // MATCH_BITMASK(flags, O_WRONLY)
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

    struct file_descriptor_table *fdtable = kmalloc(sizeof(*fdtable));
    
    if (fdtable == NULL) {
        return ENOMEM;
    }

    spinlock_init(&fdtable->lock);
    fdtable->map[0] = STDIN_FILENO;
    fdtable->map[1] = STDOUT_FILENO;
    fdtable->map[2] = STDERR_FILENO;
    fdtable->next_fd = 3; // 0, 1, 2 are used for STDIN, STDOUT, STDERR
    // TODO: Assignment call
    // TODO: Create FD->OF->STDIO

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

void assign_fd(fd_t fd, struct open_file *open_file) {
    struct file_descriptor_table *fdtable = curproc->p_fdtable;
     
    spinlock_acquire(&fdtable->lock);
    // TODO: Check if it used?
    fdtable->map[fd] = open_file;
    spinlock_release(&fdtable->lock);
}

int get_free_fd(int *retval) {
    struct file_descriptor_table *table = curproc->p_fdtable;

    int *map = table->map;    

    FD_LOCK_ACQUIRE();
    fd_t free_fd = table->next_fd;

    if (free_fd == -1) {
        *retval = EMFILE;
        FD_LOCK_RELEASE();
        return -1;
    }

    // Check for the next free file pointer
    fd_t next_fd = free_fd;
    do next_fd = (next_fd + 1) % OPEN_MAX;
    while (map[next_fd] != -1 && next_fd != free_fd) ; // FIXME: +1?
    
    table->next_fd = (next_fd == free_fd) ? -1 : next_fd;
    *retval = free_fd;

    FD_LOCK_RELEASE();

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

static struct open_file_node *__create_open_file_node() {
    struct open_file_node *open_file_node = kmalloc(sizeof(*open_file_node));
    if (open_file_node == NULL) {
        KASSERT(0);
    }

    open_file_node->prev = NULL;
    open_file_node->next = NULL;
    open_file_node->entry = NULL;

    OF_LOCK_ACQUIRE();

    if (open_file_table->head == NULL) {
        open_file_table->head = open_file_table->tail = open_file_node;
    } else {
        open_file_node->prev = open_file_table->tail;
        if (open_file_table->tail != NULL) open_file_table->tail->next = open_file_node;
        open_file_table->tail = open_file_node;
    }

    OF_LOCK_RELEASE();

    return open_file_node;
}

static struct open_file *__allocate_open_file() {
    struct open_file *open_file = kmalloc(sizeof(*open_file));
    if (open_file == NULL) {
        KASSERT(0);
    }

    open_file->flags = 0;
    open_file->vnode = NULL;
    open_file->lock = lock_create("File lock")

    return open_file;
}

struct open_file *create_open_file() {
    struct open_file_node *open_file_node = __create_open_file_node();
    struct open_file *open_file = __allocate_open_file();

    open_file_node->entry = open_file;
    open_file->reference = open_file_node;

    return open_file;
}


int create_open_file_table() {
    if (open_file_table != NULL) {
        return ENOSYS;
    } 
    
    open_file_table = kmalloc(sizeof(struct open_file_table));
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
    KASSERT(open_file_table->head == NULL && open_file_table->tail == NULL);
    spinlock_cleanup(&open_file_table->lock);
    kfree(open_file_table);
}

/* #endregion */

/* #region File Helpers */

int get_open_file_from_fd(fd_t fd, struct open_file **open_file) {

    struct file_descriptor_table *fdtable = curproc->p_fdtable;

    if (fd >= OPEN_MAX || (*open_file = fdtable->map[fd]) == NULL) {
        return EBADF;
    }

    return 0;
}

/* #endregion */

