#include <__open_file_table.h> 
#include <__file_descriptor_table.h> 

#include <types.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
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
#include <limits.h>



#define MATCH_BITMASK(value, mask) ((value & mask) == mask)

static void uio_init (struct iovec *iov, struct uio *uio, userptr_t buf, size_t len, off_t offset, enum uio_rw);  // FIXME: Not sure if `enum rw` or `uio_rw rw`

fd_t sys_open(userptr_t filename, int flags, mode_t mode, int *errno) { 

    // Find an empty file descriptor number
    fd_t fd;
    if ((*errno = get_free_fd(&fd)) != 0) return -1;
 
    char *k_filename = kmalloc(PATH_MAX);
    if ((*errno = copyinstr(filename, k_filename, PATH_MAX, NULL)) != 0) return -1; 
 
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
    if (check_invalid_fd(oldfd) || check_invalid_fd(newfd)) {
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
    
    // Get the length of the file 
    struct stat stat; 
    VOP_STAT(open_file->vnode, &stat);
    int file_length = stat.st_size;

    // If reading past the end of the file, stop at the end (and set the offset)
    if (file->offset + buflen >= file_length) { 
        read_bytes = file_length - file->offset; 
        file->offset += read_bytes
        return read_bytes;
    } 

    // Adjust the new offset by buflen
    file->offset += buflen; 
    return buflen; 
    
//
  struct stat stat; 
    VOP_STAT(open_file->vnode, &stat);
  
  int read = stat.st_size - file->offset;
  file->offset = stat.st_size
  return read;

//

    
    return file_length < buflen ? file_length : buflen;     
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

    // Prepare the uio 
    struct iovec new_iov; 
    struct uio new_uio; 

    uio_init(&new_iov, &new_uio, buf, buflen, file->offset, UIO_WRITE);

    lock_acquire(file->lock);
    if ((*errno = VOP_WRITE(file->vnode, &new_uio)) != 0) {  

        // Covers the case where no free space is left on the file (ENOSPC) 
        lock_release(file->lock);
        return -1;     
    } 

    file->offset += buflen;
    lock_release(file->lock);
    return buflen;
}

off_t sys_lseek(fd_t fd, off_t pos, int whence, int *errno) {

    // Get `open_file` 
    struct open_file *open_file;
    if ((*errno = get_open_file_from_fd(fd, &open_file)) != 0) return -1;

    // Check if file seekable
    if (!VOP_ISSEEKABLE(open_file->vnode)) {
        *errno = ESPIPE;
        return -1;
    }

    // Check if whence is valid 
    switch (whence) { 
        case SEEK_SET: 
        case SEEK_CUR: 
        case SEEK_END: 
            break; 
        default: 
            *errno = EINVAL; 
            return -1; 
    }

    // `Seek
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

