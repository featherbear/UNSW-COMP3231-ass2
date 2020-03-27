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


#define MATCH_BITMASK(value, mask) (value & mask == mask)

fd_t sys_open(userptr_t filename, int flags, mode_t mode, int *errno) { 

    // Find an empty file descriptor number
    fd_t fd;
    if ((*errno = get_free_fd(&fd)) != 0) return -1;

    // Create a new `struct open_file`
    struct open_file *open_file = create_open_file();
    if ((*errno = vfs_open(filename, flags, mode, &open_file->vnode))) return -1;  
    
    // open_file->offset = 0;
    // TODO: Possible :: open_file->offset = MATCH_BITMASK(flags, O_APPEND) ? VOP_STAT(open_file->vnode) : 0;

    open_file->flags = flags;

    // Map file descriptor to the open file
    assign_fd(fd, open_file);
    
    return fd;
}

int sys_close(fd_t fd, *errno) { 
    
    // Get the file 
    struct open_file *file;     
    if ((*errno = get_open_file_from_fd(fd, &file)) != 0) return -1;

    // Delegate to VFS
    if ((*errno = vfs_close(file->vnode)) != 0) return -1; 

    assign_fd(fd, NULL);

    FD_LOCK_ACQUIRE();
    // If there were originally no more free fd's, assign next_fd to be the fd-to-be-removed
    if (curproc->p_fdtable->next_fd == -1) {
        curproc->p_fdtable->next_fd = fd;
    }
    FD_LOCK_RELEASE();

    // TODO: Check other references to the vnode - remove from OF table if all references have finished.

    OF_LOCK_ACQUIRE();
    struct open_file *p = file->prev; 
    struct open_file *n = file->next; 
    p->next = n; 
    n->prev = p; 
    
    kfree(file); 
    OF_LOCK_RELEASE();

    // Success 
    return 0; 
}

fd_t sys_dup2(fd_t oldfd, fd_t newfd, int *errno) {
    if (check_invalid_fd(oldfd) | check_invalid_fd(newfd)) {
        *errno = EBADF;
        return -1;
    }

    // Check if `newfd` is open, if so then close
    struct file_descriptor_table *fdtable = curproc->p_fdtable;
    if (fdtable->map[newfd] != NULL && sys_close(newfd, errno) != 0) {
        return -1;
    }

    assign_fd(newfd, fdtable->map[oldfd]);
    
    return newfd;
}

int sys_read(fd_t fd, userptr_t buf, size_t buflen, int *errno) { 
    
    // Get the file 
    struct open_file *file; 
    if ((*errno = get_open_file_from_fd(fd, &file))) != 0) return -1;

    // Check the permissions 
    int flags = file->flags;  
    if (!MATCH_BITMASK(flags, O_RDONLY) && !MATCH_BITMASK(flags, O_RDWR) { 
        *errno = EPERM; 
        return -1;  
    } 

    // Prepare the uio 
    struct iovec new_iov;
    struct uio new_uio; 
    char *kernel_buf = kmalloc(buflen); 
    uio_init(&new_iov, &new_uio, kernel_buf, sizeof(kernel_buf), file->offset, UIO_READ);
    
    lock_acquire(file->lock); 
    if ((*errno = VOP_READ(file->vnode, new_uio)) != 0) { 
        lock_release(file->lock); 
        kfree(kernel_buf); 
        return -1; 
    } 

    lock_release(file->lock);
    kfree(kernel_buf);  
    return 0;     
}


int sys_write(fd_t fd, userptr_t buf, size_t buflen, int *errno) {  
    
    // Get the file 
    struct open_file *file; 
    if ((*errno = get_open_file_from_fd(fd, &file)) != 0) return -1;

    // Check if we have permission to write
    int flags = file->flags; 
    if (!MATCH_BIMASK(flags, O_WRONLY) && !MATCH_BITMASK(flags, O_RDWR)) {
        *errno = EPERM; 
        return -1; 
    }

    // Copy data from User-land into Kernel-land 
    char *kernel_buf = kmalloc(sizeof(buflen));
    if ((*errno = copyin(buf, kernel_buf, sizeof(kernel_buf)) != 0) return -1; 

    // Prepare the uio 
    struct iovec new_iov; 
    struct uio new_uio; 

    uio_init(&new_iov, &new_uio, kernel_buf, sizeof(kernel_buf), file->offset, UIO_WRITE);

    lock_acquire(file->lock);
    if ((*errno = VOP_WRITE(file->vnode, &new_uio)) != 0) { 

        // Covers the case where no free space is left on the file (ENOSPC) 
        lock_release(file->lock);
        kfree(kernel_buf);
        return -1;     
    }
    file->offset += sizeof(kernel_buf); 
    lock_release(file->lock);
    kfree(kernel_buf);
    return 0;
}

off_t sys_lseek(fd_t fd, off_t pos, int whence, int *errno) {
    switch (whence) {
        case SEEK_SET:
        case SEEK_CUR:
        case SEEK_END:
            break;
        default:
            *errno = EINVAL;
            return -1;
    }

    struct open_file *open_file = get_open_file_from_fd(fd);

    if (!VOP_ISSEEKABLE(open_file->vnode)) {
        *errno = ESPIPE;
        return -1;
    }

    off_t curPos = open_file->offset;
    off_t newPos;

    switch (whence) {

        case SEEK_SET:
            newPos = pos;
            break;
        case SEEK_CUR:
            newPos = curPos + pos; 
            break;
        case SEEK_END:
            newPos = VOP_STAT(open_file->vnode) + pos;
            break;
    }

    if (newPos < 0) {
        *errno = EINVAL;
        return -1;
    }

    // Success
    return newPos;    
}

/* #region FD Layer */

static bool check_invalid_fd(fd_t fd) {
    return (fd < 0 || fd >= OPEN_MAX) ? EBADF : 0;
}


struct file_descriptor_table *create_file_table() {
    struct file_descriptor_table *fdtable = kmalloc(sizeof(*fdtable));
    
    if (fdtable == NULL) {
        return ENOMEM;
    }

    spinlock_init(&fdtable->lock);

    struct open_file *stdout_file = create_open_file();
    char stdoutPath[] = "con:";
    if ((*errno = vfs_open(stdoutPath, O_WR, 0, &stdout_file->vnode))) return -1;  


    struct open_file *stdout_file = create_open_file();
    char stdoutPath[] = "con:";
    if ((*errno = vfs_open(stdoutPath, O_WR, 0, &stdout_file->vnode))) return -1;  


    struct open_file *stderr_file = create_open_file();
    char stderrPath[] = "con:";
    if ((*errno = vfs_open(stderrPath, O_WR, 0, &stderr_file->vnode))) return -1;  



    
    assign_fd(1, )
    assign_fd(2, )
    // fdtable->map[0] = STDIN_FILENO;
    fdtable->map[STDOUT_FILENO] = ;
    fdtable->map[STDERR_FILENO] = ;
    fdtable->next_fd = 3; // 0, 1, 2 are used for STDIN, STDOUT, STDERR

    return fdtable;
}

void destroy_file_table(struct file_descriptor_table *fdtable) {
    kfree(curproc->p_fdtable);
}

void assign_fd(fd_t fd, struct open_file *open_file) {
    struct file_descriptor_table *fdtable = curproc->p_fdtable;
     
    FD_LOCK_ACQUIRE();

    // TODO: Check if it used?
    fdtable->map[fd] = open_file;
    
    FD_LOCK_ACQUIRE();
}

int get_free_fd(int *errno) {
    struct file_descriptor_table *table = curproc->p_fdtable;

    int *map = table->map;    

    FD_LOCK_ACQUIRE();
    fd_t free_fd = table->next_fd;

    if (free_fd == -1) {
        *errno = EMFILE;
        FD_LOCK_RELEASE();
        return -1;
    }

    // Check for the next free file pointer
    fd_t next_fd = free_fd;
    do next_fd = (next_fd + 1) % OPEN_MAX;
    while (map[next_fd] != -1 && next_fd != free_fd) ; // FIXME: +1? TESTME
    
    table->next_fd = (next_fd == free_fd) ? -1 : next_fd;
    *errno = free_fd;

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
        // ENFILE
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
        // ENFILE
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

    if (check_invalid_fd(fd) || (*open_file = fdtable->map[fd]) == NULL) {
        return EBADF;
    }

    return 0;
}

/* #endregion */

// Stolen from Asst2 Video
void uio_init (
        struct iovec *iov, 
        struct iou *u, 
        userptr_t buf, 
        size_t len, 
        off_t offset, 
        enum uio_rw rw
    ) {

    // { 
    //     iov->iov_ubase = buf; 
    //     iov->iov_len = len; 
    //     u->uio_iov = iov; 
    //     u->uio_iovcnt = 1; 
    //     u->uio_offset = offset; 
    //     u->uio_resid = len; 
    //     u->uio_segflg = UIO_USERSPACE; 
    //     u->uio_rw = rw; 
    //     u->uio_space = rw, 
        
    // }

    
    *iov = (struct iovec) {
        .iov_ubase = buf,
        .iov_len = len
    };

    *iou = (struct iou) {
            .uio_iov = iov,
            .uio_iovcnt = 1,
            .uio_offset = offset,
            .uio_resid = len,
            .uio_segflg = UIO_USERSPACE,
            .uio_rw = rw,
            .uio_space = rw
    }
}