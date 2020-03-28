#ifndef ___OPEN_FILE_TABLE_H_
#define ___OPEN_FILE_TABLE_H_

#define OF_LOCK_ACQUIRE() (spinlock_acquire(&open_file_table->lock))
#define OF_LOCK_RELEASE() (spinlock_release(&open_file_table->lock))

/* File entry in the Open File table */
struct open_file {
    int flags;           // Access flags
    off_t offset;        // File pointer (to last left off location)
    struct vnode *vnode; // Pointer to the VFS node
    struct lock *lock;   // Shared access 
    void *reference;     // Reference to the open_file_node ADT
};

/* Initialise the global open file table */
int create_open_file_table(void);

/* Destroy the global open file table */
void destroy_open_file_table(void);

struct open_file *create_open_file(void);

extern struct open_file_table *open_file_table;

#endif /* ___OPEN_FILE_TABLE_H_ */
