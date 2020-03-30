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
    // TODO: Check if allocated

    if ((*errno = copyinstr(filename, k_filename, PATH_MAX, NULL)) != 0) {
        // TODO: free memory
        return -1; 
    }
 
    // Create a new `struct open_file`
    struct open_file *open_file = create_open_file();
    if ((*errno = vfs_open(k_filename, flags, mode, &open_file->vnode))) {
        // TODO: free memory
        return -1;  
    }
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
    FD_LOCK_ACQUIRE();
    FD_ASSIGN(fd, open_file);
    FD_LOCK_RELEASE();

    return fd;
}

int sys_close(fd_t fd, int *errno) { 
    
    // Get the file 
    struct open_file *file;     
    if ((*errno = get_open_file_from_fd(fd, &file)) != 0) return -1;
    kprintf("Spinlock problem before the acquire?\n"); 
    FD_LOCK_ACQUIRE();
    kprintf("Spinlock problem after the acquire?\n"); 
    vfs_close(file->vnode);
    
    FD_ASSIGN(fd, NULL);

    // If there were originally no more free fd's, assign next_fd to be the fd-to-be-removed
    if (curproc->p_fdtable->next_fd == -1) {
        curproc->p_fdtable->next_fd = fd;
    }

    OF_LOCK_ACQUIRE();

    // Check other references to the vnode - remove from OF table if all references have finished.   
    if (--file->refs == 0) {
        struct open_file_node *node = file->reference; 
        node->next->prev = node->prev;
        node->prev->next = node->next

        kfree(file);
    }
    
    OF_LOCK_RELEASE();

    // Success 
    return 0; 
}

fd_t sys_dup2(fd_t oldfd, fd_t newfd, int *errno) {
    if (check_invalid_fd(oldfd) || check_invalid_fd(newfd)) {
        *errno = EBADF;
        return -1;
    }

    if (oldfd == newfd) return newfd;
    
    struct file_descriptor_table *fdtable = curproc->p_fdtable;
    struct open_file *file; 
    
    FD_LOCK_ACQUIRE();
    
    if ((file = fdtable->map[oldfd]) == NULL) {
        *errno = EBADF;
        FD_LOCK_RELEASE();
        return -1;
    }

    // Check if `newfd` is open, if so then close
    if (fdtable->map[newfd] != NULL && sys_close(newfd, errno) != 0) {
        // errno set by sys_close
        FD_LOCK_RELEASE();
        return -1;
    }

    FD_ASSIGN(newfd, file);
    
    FD_LOCK_RELEASE();

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
 
    off_t change = new_uio.uio_offset - file->offset;
    file->offset = new_uio.uio_offset;

    lock_release(file->lock);

    /*

        // Get the length of the file 
        struct stat stat; 
        VOP_STAT(file->vnode, &stat);
        int file_length = stat.st_size;
        kprintf("KERNEL :: Change %d into %d\n", (int) file->offset, (int) new_uio.uio_offset);

        // If reading past the end of the file, stop at the end (and set the offset)
        if (file->offset + buflen > file_length) { 
            int read_bytes = file_length - file->offset;
            KASSERT(read_bytes >= 0);
            file->offset = file_length;
            return read_bytes;
        } 

        // Add on the newly read offset
        file->offset += buflen;

         return buflen; 
        */
    
    return change;
}


int sys_write(fd_t fd, userptr_t buf, size_t buflen, int *errno) {  
    
    // TODO: EPERM - An append-only flag is set on the file, but the caller is attempting to write before the current end of file.

    // Get the file 
    struct open_file *file; 
    if ((*errno = get_open_file_from_fd(fd, &file)) != 0) return -1;

    // Check if we have permission to write
    int flags = file->flags; 
    if (!MATCH_BITMASK(flags, O_WRONLY) && !MATCH_BITMASK(flags, O_RDWR)) {
        *errno = EBADF;
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

    off_t change = new_uio.uio_offset - file->offset;
    file->offset = new_uio.uio_offset;

    lock_release(file->lock);

    return change;
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

    open_file->offset = newPos;

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

