#include "__mymytest.h"

static void test_dup2__nonexistent_fd(void); 
static void test_dup2__invalid_fd(void); 
/* 
        EMFILE	The process's file table was full, or a process-specific limit on open files was reached.
        ENFILE	The system's file table was full, if such a thing is possible, or a global limit on open files was reached.
*/

void test_dup2() {
    test(test_dup2__nonexistent_fd); 
    test(test_dup2__invalid_fd); 
}

static void test_dup2__nonexistent_fd() {
    // FD 10 doesn't exist
    assert(dup2(10, 11) == -1);
    assert(errno == EBADF);

    return;
}
static void test_dup2__invalid_fd() {
    // FD -1 is invalid
    assert(dup2(-1, 4) == -1);
    assert(errno == EBADF);
    
    // FD OPEN_MAX is invalid
    assert(dup2(OPEN_MAX, 4) == -1);
    assert(errno == EBADF);

    // FD OPEN_MAX + 1 is invalid
    assert(dup2(OPEN_MAX + 1, 4) == -1);
    assert(errno == EBADF);
    
    // FD OPEN_MAX is invalid
    printf("%d", dup2(1, OPEN_MAX));
    // == -1
    // assert(); 
    // assert(errno == EBADF); 

    assert(dup2(1, OPEN_MAX + 1) == -1);
    assert(errno == EBADF);

    return;
}