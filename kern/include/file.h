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
    fd_t next_fd;       // Next free file descriptor number 
    fd_t map[OPEN_MAX]; // Array of indices to the OFT
    spinlock_t lock;    // Thread-safe lock

    // TODO: Released files should get the same fd if they are requested again
    /* 
        Keep a dictionary of recently closed files 
        "File Path" : File Descriptor Number 
    */ 
};

/* Calculate the next free fd */
int get_free_fd(int *retval);

/* TODO: Close a fd */
void fd_close(fd_t fd);

/* Creates and links FD table to the current process */
struct file_descriptor_table *create_file_table(void);

/* Destroys and unlinks FD table from the current process */
void destroy_file_table(void);

/* #endregion */

/* #region OF Layer */

// === One Important Note ===
// In modern operating systems, the "open file table" is **usually a doubly linked list**, not a static table.  
// This ensures that it is typically a reasonable size while capable of accomodating workloads that use massive numbers of files. 
// - Source: https://cseweb.ucsd.edu/classes/sp16/cse120-a/applications/ln/lecture15.html
// 
// Ehh. HAHA

/* File entry in the Open File table */
struct open_file {
    int flags;           // Access flags
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

// TODO: Populate the other prototypes

fd_t sys_open(userptr_t filename, int flags, mode_t mode, int *retval);
int sys_read(fd_t fd, userptr_t buf, int buflen, int *retval);
int sys_write(fd_t fd, userptr_t buf, size_t buflen, int *retval);
off_t sys_lseek(fd_t fd, off_t pos, int whence);
// int sys_close(int, *int); 

// int sys_write(*int); 
// int sys_lseek(*int); 
// int sys_dup2(*int); 
/* #endregion */

#endif /* _FILE_H_ */
