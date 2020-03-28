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
#include <proc.h>



#define FD_LOCK_ACQUIRE() (spinlock_acquire(&curproc->p_fdtable->lock))
#define FD_LOCK_RELEASE() (spinlock_release(&curproc->p_fdtable->lock))
#define OF_LOCK_ACQUIRE() (spinlock_acquire(&open_file_table->lock))
#define OF_LOCK_RELEASE() (spinlock_release(&open_file_table->lock))


#define MATCH_BITMASK(value, mask) ((value & mask) == mask)

static struct open_file *create_open_file(void);
static struct open_file *__allocate_open_file(void);
static struct open_file_node *__create_open_file_node(void);
static void uio_init (struct iovec *iov, struct uio *uio, userptr_t buf, size_t len, off_t offset, enum uio_rw);  // FIXME: Not sure if `enum rw` or `uio_rw rw`
static void assign_fd(fd_t fd, struct open_file *open_file);
static bool check_invalid_fd(fd_t fd);

struct open_file_table {
    struct spinlock lock;
    struct open_file_node *head;
    struct open_file_node *tail;
};

struct open_file_node {
    struct open_file_node *prev;
    struct open_file_node *next;
    struct open_file *entry;
};

fd_t sys_open(userptr_t filename, int flags, mode_t mode, int *errno) { 

    // Find an empty file descriptor number
    fd_t fd;
    if ((*errno = get_free_fd(&fd)) != 0) return -1;
 
    char *k_filename = kmalloc(PATH_MAX);
    if ((*errno = copyinstr(filename, k_filename, sizeof(k_filename), NULL)) != 0) return -1; 
 
    // Create a new `struct open_file`
    struct open_file *open_file = create_open_file();
    if ((*errno = vfs_open(k_filename, flags, mode, &open_file->vnode))) return -1;  
    kfree(k_filename); 

    // If set to O_APPEND, set ptr to end, otherwise set to 0 
    if (MATCH_BITMASK(flags, O_APPEND)) {
        struct stat stat;
        VOP_STAT(open_file->vnode, &stat);
        open_file->offset = stat.st_size;
    } else {
        open_file->offset = 0;
    }
    

    open_file->flags = flags;

    // Map file descriptor to the open file
    assign_fd(fd, open_file);
    
    return fd;
}

int sys_close(fd_t fd, int *errno) { 
    
    // Get the file 
    struct open_file *file;     
    if ((*errno = get_open_file_from_fd(fd, &file)) != 0) return -1;

    vfs_close(file->vnode);
    assign_fd(fd, NULL);

    FD_LOCK_ACQUIRE();
    // If there were originally no more free fd's, assign next_fd to be the fd-to-be-removed
    // TODO: Move this part into assign_fd(...)?
    if (curproc->p_fdtable->next_fd == -1) {
        curproc->p_fdtable->next_fd = fd;
    }
    FD_LOCK_RELEASE();


    /*
    OF_LOCK_ACQUIRE();
    // TODO: Check other references to the vnode - remove from OF table if all references have finished.
    // only if reference count is 0 - which technically we haven't even made a counter yet.
    
    struct open_file_node *node = open_file->reference; 
    struct open_file_node *p = node->prev; 
    struct open_file_node *n = node->next; 
    p->next = n; 
    n->prev = p;   
    kfree(file);
    
    OF_LOCK_RELEASE();
    */

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
    if ((*errno = get_open_file_from_fd(fd, &file)) != 0) return -1;

    // Check the permissions 
    int flags = file->flags;  
    if (!MATCH_BITMASK(flags, O_RDONLY) && !MATCH_BITMASK(flags, O_RDWR)) { 
        *errno = EPERM; 
        return -1;  
    } 

    // Prepare the uio 
    struct iovec new_iov; // FIXME: Storage size of 'new_iov' isn't known..
    struct uio new_uio; 
    uio_init(&new_iov, &new_uio, buf, buflen, file->offset, UIO_READ);
    
    lock_acquire(file->lock); 
    if ((*errno = VOP_READ(file->vnode, &new_uio)) != 0) { 
        lock_release(file->lock); 
        return -1; 
    } 

    lock_release(file->lock);
    return 0;     
}


int sys_write(fd_t fd, userptr_t buf, size_t buflen, int *errno) {  
    
    // Get the file 
    struct open_file *file; 
    if ((*errno = get_open_file_from_fd(fd, &file)) != 0) return -1;

    // Check if we have permission to write
    int flags = file->flags; 
    if (!MATCH_BITMASK(flags, O_WRONLY) && !MATCH_BITMASK(flags, O_RDWR)) {
        *errno = EPERM; 
        return -1; 
    }

    // FIXME: TEST
    // Copy data from User-land into Kernel-land 
    /*
    char *kernel_buf = kmalloc(sizeof(buflen));
    if ((*errno = copyin(buf, kernel_buf, sizeof(kernel_buf)) != 0) return -1; 
    */

    // Prepare the uio 
    struct iovec new_iov; 
    struct uio new_uio; 


    // FIXME: TEST :: uio_init(&new_iov, &new_uio, kernel_buf, sizeof(kernel_buf), file->offset, UIO_WRITE);

    uio_init(&new_iov, &new_uio, buf, buflen, file->offset, UIO_WRITE);

    lock_acquire(file->lock);
    // FIXME: ../../syscall/file.c:192:5: error: type of formal parameter 6 is incomplete"
    if ((*errno = VOP_WRITE(file->vnode, &new_uio)) != 0) {  
        

        // Covers the case where no free space is left on the file (ENOSPC) 
        lock_release(file->lock);
        // FIXME: TEST :: kfree(kernel_buf);
        return -1;     
    }
    // FIXME: TEST :: file->offset += sizeof(kernel_buf); 
    file->offset += buflen;
    lock_release(file->lock);
    //FIXME: TEST :: kfree(kernel_buf);
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

    struct open_file *open_file;
    if ((*errno = get_open_file_from_fd(fd, &open_file)) != 0) return -1;

    if (!VOP_ISSEEKABLE(open_file->vnode)) {
        *errno = ESPIPE;
        return -1;
    }

    off_t curPos, newPos;
    curPos = newPos = open_file->offset;

    struct stat stat; // For SEEK_END

    switch (whence) {

        case SEEK_SET:
            newPos = pos;
            break;
        case SEEK_CUR:
            newPos = curPos + pos; 
            break;
        case SEEK_END:
            VOP_STAT(open_file->vnode, &stat);
            newPos = stat.st_size + pos;
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

// TODO: Meta
static bool check_invalid_fd(fd_t fd) {
    return (fd < 0 || fd >= OPEN_MAX) ? EBADF : 0;
}

// TODO: Meta
struct file_descriptor_table *create_file_table() {


    //TODO: MEMORY MANGE THIS ENTIRE FUNCTION
    //TODO: MEMORY MANGE THIS ENTIRE FUNCTION
    //TODO: MEMORY MANGE THIS ENTIRE FUNCTION
    //TODO: MEMORY MANGE THIS ENTIRE FUNCTION
    //TODO: MEMORY MANGE THIS ENTIRE FUNCTION
    //TODO: MEMORY MANGE THIS ENTIRE FUNCTION
    //TODO: MEMORY MANGE THIS ENTIRE FUNCTION
    //TODO: MEMORY MANGE THIS ENTIRE FUNCTION

    struct file_descriptor_table *fdtable = kmalloc(sizeof(*fdtable));

    fdtable->map = (struct open_file **) kmalloc(OPEN_MAX * sizeof(struct open_file *));

    // kmemset?
    for (int i = 0; i < OPEN_MAX; i++) fdtable->map[i] = NULL;
    

    if (fdtable == NULL) {
        return NULL; // Let proc.c deal with this
        // return ENOMEM;
    }

    spinlock_init(&fdtable->lock);

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

    return fdtable;
}

void destroy_file_table(struct file_descriptor_table *fdtable) {
    spinlock_cleanup(&fdtable->lock);
    kfree(fdtable->map);
    kfree(fdtable);
}

static void assign_fd(fd_t fd, struct open_file *open_file) {
    struct file_descriptor_table *fdtable = curproc->p_fdtable;
     
    FD_LOCK_ACQUIRE();

    // TODO: Check if it used?
    fdtable->map[fd] = open_file;
    
    FD_LOCK_ACQUIRE();
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


/* #endregion */

/* #region OF Layer */

struct open_file_table *open_file_table = NULL;


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
    open_file->lock = lock_create("File lock");

    return open_file;
}

static struct open_file *create_open_file() {
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
static void uio_init (
        struct iovec *iov, 
        struct uio *uio, 
        userptr_t buf, 
        size_t len, 
        off_t offset, 
        enum uio_rw rw
    ) {
    
    *iov = (struct iovec) {
        .iov_ubase = buf,
        .iov_len = len
    };

    *uio = (struct uio) {
            .uio_iov = iov,
            .uio_iovcnt = 1,
            .uio_offset = offset,
            .uio_resid = len,
            .uio_segflg = UIO_USERSPACE,
            .uio_rw = rw,
            .uio_space = proc_getas() // TODO: FIXME:
    };
}
