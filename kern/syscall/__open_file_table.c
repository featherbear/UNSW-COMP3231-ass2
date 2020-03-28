#include <types.h>
#include <kern/errno.h>
#include <kern/limits.h>
#include <kern/stat.h>
#include <lib.h>

#include <__open_file_table.h>

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
};

struct open_file_table *open_file_table = NULL; // 
// FIXME: Should this be placed in file.c as a global  


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


    OF_LOCK_ACQUIRE();

    OF_LOCK_RELEASE();
    return NULL;

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
 
struct open_file *create_open_file() {
    struct open_file_node *open_file_node = __create_open_file_node();

    if (open_file_node == NULL) {}
    return NULL;

    struct open_file *open_file = __allocate_open_file();
    open_file_node->entry = open_file;
    open_file->reference = open_file_node;

    return open_file;
}

int create_open_file_table() {

    // Only one global open_file_table shoud exist
    if (open_file_table != NULL) return ENOSYS; 
    
    open_file_table = kmalloc(sizeof(struct open_file_table));
    bzero(open_file_table, sizeof(struct open_file_table));
    
    if (open_file_table == NULL) return ENOMEM;


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
