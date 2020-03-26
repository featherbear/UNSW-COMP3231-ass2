/*
 * Declarations for file handle and file table management.
 */

#ifndef _FILE_H_
#define _FILE_H_

/*
 * Contains some file-related maximum length constants
 */
#include <limits.h>
#include <synch.h> // for P(), V(), sem_* 

/*  FROM limits.h
#define NAME_MAX        __NAME_MAX
#define PATH_MAX        __PATH_MAX
#define ARG_MAX         __ARG_MAX
#define PID_MIN         __PID_MIN
#define PID_MAX         __PID_MAX
#define PIPE_BUF        __PIPE_BUF
#define NGROUPS_MAX     __NGROUPS_MAX
#define LOGIN_NAME_MAX  __LOGIN_NAME_MAX
#define OPEN_MAX        __OPEN_MAX
#define IOV_MAX         __IOV_MAX

*/

/* #region FD Layer */

/* PER-PROCESS map of a file descriptors to its `struct file_entry` */
struct file_descriptor_table
{
    int next_fd; // Next free file descriptor number 
    int map[OPEN_MAX]; // Array of indices to the OFT

    // TODO: Released files should get the same fd if they are requested again
    /* 
        Keep a dictionary of recently closed files 
        "File Path" : File Descriptor Number 
    */ 
};

/* Calculate the next free fd */
int get_free_fd(void);

/* Close a fd */
void fd_close(int fd);

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
struct open_file
{
    int fp;              // File
    int flags;           // Access flags
    struct vnode *vnode; // Pointer to the VFS node
    struct lock *lock;   // Shared access 
};

struct open_file *open_file_table = NULL; // Define our global value here

/* Initialise the global open file table */
void create_open_file_table(void);

/* Destroy the global open file table */
void destroy_open_file_table(void);

int get_free_of(void);
/* #endregion */

/* #region File Helpers */

/* Get the open file entry associated with the fd for the current process 

   On Success: Sets `open_file` to the open_file structure, and returns 0
   On Failure: Returns a non-zero value corresponding to the error code
*/
int get_open_file_from_fd(int fd, struct open_file **open_file); 

/* #endregion */



/* #region File Operations */

/* #endregion */

#endif /* _FILE_H_ */
