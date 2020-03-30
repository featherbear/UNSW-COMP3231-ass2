#include <__open_file_table.h>

#include <kern/errno.h>
#include <types.h>
#include <lib.h>

static struct open_file *__allocate_open_file(void);
static struct open_file_node *__create_open_file_node(void);
 
 
struct open_file_table {
    struct spinlock lock;
    struct open_file_node *head;
    struct open_file_node *tail;
};

struct open_file_node {
    struct open_file_node *prev;
    struct open_file_node *next;
    struct open_file *entry;
    uint refs;                    // Number of references to this node's entry
};

struct open_file_table *open_file_table = NULL;

static struct open_file_node *__create_open_file_node() {
    struct open_file_node *open_file_node = kmalloc(sizeof(*open_file_node));
    if (open_file_node == NULL) {
        return NULL;
        // ENFILE
        KASSERT(0);
    }

    open_file_node->prev = NULL;
    open_file_node->next = NULL;
    open_file_node->entry = NULL;
    open_file_node->refs = 1;

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
    
    KASSERT(open_file != NULL);
    // ENFILE

    open_file->refs = 1;
    open_file->flags = 0;
    open_file->offset = 0;
    open_file->vnode = NULL;
    open_file->lock = lock_create("file lock");

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

    // Only one global open_file_table shoud exist
    if (open_file_table != NULL) return ENOSYS; 
    
    open_file_table = kmalloc(sizeof(struct open_file_table));
    
    KASSERT(open_file_table != NULL);
        // return ENOMEM;

    memset(open_file_table, 0, sizeof(struct open_file_table));

    spinlock_init(&open_file_table->lock);
    open_file_table->head = NULL;
    open_file_table->tail = NULL;

    return 0;
}

void destroy_open_file_table() {
    // TODO: Should we free the nodes? Probably? We had to do that in our labs didn't we.
    KASSERT(open_file_table->head == NULL && open_file_table->tail == NULL);
    spinlock_cleanup(&open_file_table->lock);
    kfree(open_file_table);
}

// put it here for now
void release_reference(struct open_file_node *node) {
    OF_LOCK_ACQUIRE();

    // Check other references to the vnode - remove from OF table if all references have finished.   
    if (--node->refs == 0) {
        struct open_file_node *node = file->reference; 
        node->next->prev = node->prev;
        node->prev->next = node->next

        kfree(file);
    }
    
    OF_LOCK_RELEASE();
}