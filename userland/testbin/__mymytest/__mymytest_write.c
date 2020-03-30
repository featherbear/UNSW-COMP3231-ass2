#include "__mymytest.h"
#include <limits.h>


static void test_write__emptyString(void);
static void test_write__bufferTooBig(void); // (Can we actualy test this. We can try :) 
static void test_write__nonexistent_fd(void); // FD doesn't exist in FD table
static void test_write__no_permission(void); // EBADF 
static void test_write__out_of_space(void); // ENOSPC
static void test_write__invalid_fd(void); // FD >= OPEN_MAX -> EBADF 

void test_write() {
    test(test_write__emptyString);
    test(test_write__bufferTooBig);
    test(test_write__nonexistent_fd);
    test(test_write__no_permission);
    test(test_write__out_of_space);
    test(test_write__invalid_fd);
}

static void test_write__emptyString() {
    return;
}
static void test_write__bufferTooBig() {
    return;
}
static void test_write__nonexistent_fd() {
    int ret;
    
    _assert((ret = write(3, "hello", 5)) == -1);
    _assert(errno == EBADF);

    _assert((ret = write(OPEN_MAX - 1, "hello", 5)) == -1);
    _assert(errno == EBADF);

    return;
}
static void test_write__no_permission() {
        return;
}
static void test_write__out_of_space() {
        return;
}
static void test_write__invalid_fd() {
    int ret;
    
    _assert((ret = write(-1, "hello", 5)) == -1);
    _assert(errno == EBADF);

    // OPEN_MAX - 1 is tested in nonexistent_fd(), and is not tested here

    _assert((ret = write(OPEN_MAX, "hello", 5)) == -1);
    _assert(errno == EBADF);

    _assert((ret = write(OPEN_MAX + 1, "hello", 5)) == -1);
    _assert(errno == EBADF);

    return;
}
