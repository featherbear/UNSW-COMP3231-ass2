#ifndef ___FILE_DESCRIPTOR_TABLE_H_
#define ___FILE_DESCRIPTOR_TABLE_H_


#include <kern/errno.h>
#include <current.h>
#include <types.h>
#include <proc.h>
#include <spinlock.h>

#define FD_LOCK_ACQUIRE() (spinlock_acquire(&curproc->p_fdtable->lock))
#define FD_LOCK_RELEASE() (spinlock_release(&curproc->p_fdtable->lock))

#define FD_ASSIGN(fd, open_file) (curproc->p_fdtable->map[fd] = open_file)
#define IS_VALID_FD(fd) (0 <= fd && fd < OPEN_MAX)

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

/* Get the open file entry associated with the fd for the current process 

   On Success: Sets `open_file` to the open_file structure, and returns 0
   On Failure: Returns a non-zero value corresponding to the error code
*/
int get_open_file_from_fd(fd_t fd, struct open_file **open_file); 

/* 
 * Create a deep copy of a file_descriptor_table 
 * TODO: Uncomment this section 
struct file_descriptor_table clone_fdt();
 */

#endif /* ___FILE_DESCRIPTOR_TABLE_H_ */
