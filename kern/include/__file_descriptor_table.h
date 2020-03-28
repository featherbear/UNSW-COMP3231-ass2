#ifndef ___FILE_DESCRIPTOR_TABLE_H_
#define ___FILE_DESCRIPTOR_TABLE_H_

#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <spinlock.h>
#include <current.h>
#include <proc.h>

#define FD_LOCK_ACQUIRE() (spinlock_acquire(&curproc->p_fdtable->lock))
#define FD_LOCK_RELEASE() (spinlock_release(&curproc->p_fdtable->lock))

typedef int fd_t;

/* PER-PROCESS map of a file descriptors to its `struct file_entry` */
struct file_descriptor_table
{
    fd_t next_fd;              // Next free file descriptor number 
    struct open_file **map;    // Array of pointers to the OFT
    struct spinlock lock;      // Thread-safe lock

    // TODO: Released files should get the same fd if they are requested again
};

/* Calculate the next free fd */
int get_free_fd(int *retval);

/* Close a fd */
void fd_close(fd_t fd);

/* Creates and links FD table to the current process */
struct file_descriptor_table *create_file_table(void);

/* Destroys and unlinks FD table from the current process */
void destroy_file_table(struct file_descriptor_table *fdtable);

bool check_invalid_fd(fd_t fd);

void assign_fd(fd_t fd, struct open_file *open_file);

int get_open_file_from_fd(fd_t fd, struct open_file **open_file); 


#endif /* ___FILE_DESCRIPTOR_TABLE_H_ */
