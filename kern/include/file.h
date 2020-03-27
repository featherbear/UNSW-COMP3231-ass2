/*
 * Declarations for file handle and file table management.
 */

#ifndef _FILE_H_
#define _FILE_H_

/*
 * Contains some file-related maximum length constants
 */
#include <limits.h>
#include <types.h>
#include <synch.h> // for P(), V(), sem_* 
#include <spinlock.h>


typedef int fd_t;

/* #region FD Layer */

/* PER-PROCESS map of a file descriptors to its `struct file_entry` */
struct file_descriptor_table
{
    fd_t next_fd;                        // Next free file descriptor number 
    struct open_file *map[OPEN_MAX];     // Array of pointers to the OFT
    struct spinlock lock;                // Thread-safe lock

    // TODO: Released files should get the same fd if they are requested again
    /* 
        Keep a dictionary of recently closed files 
        "File Path" : File Descriptor Number 
    */ 
};

/* Calculate the next free fd */
int get_free_fd(int *retval);

/* Close a fd */
void fd_close(fd_t fd);

/* Creates and links FD table to the current process */
struct file_descriptor_table *create_file_table(void);

/* Destroys and unlinks FD table from the current process */
void destroy_file_table(struct file_descriptor_table *fdtable);

/* #endregion */

/* #region OF Layer */

/* File entry in the Open File table */
struct open_file {
    int flags;           // Access flags
    off_t offset;        // File pointer (to last left off location)
    struct vnode *vnode; // Pointer to the VFS node
    struct lock *lock;   // Shared access 
    void *reference;     // Reference to the open_file_node ADT
};

struct open_file_table;
struct open_file_table *open_file_table = NULL; // Define our global value here

/* Initialise the global open file table */
int create_open_file_table(void);

/* Destroy the global open file table */
void destroy_open_file_table(void);

/* #endregion */

/* #region File Helpers */

/* Get the open file entry associated with the fd for the current process 

   On Success: Sets `open_file` to the open_file structure, and returns 0
   On Failure: Returns a non-zero value corresponding to the error code
*/
int get_open_file_from_fd(fd_t fd, struct open_file **open_file); 

/* #endregion */

/* #region File Operations */

fd_t sys_open(userptr_t filename, int flags, mode_t mode, int *errno);
int sys_close(fd_t fd, int *errno); 
int sys_read(fd_t fd, userptr_t buf, size_t buflen, int *errno);
int sys_write(fd_t fd, userptr_t buf, size_t buflen, int *errno);
off_t sys_lseek(fd_t fd, off_t pos, int whence, int *errno);
fd_t sys_dup2(fd_t oldfd, fd_t newfd, int *errno);


/* #endregion */

#endif /* _FILE_H_ */
