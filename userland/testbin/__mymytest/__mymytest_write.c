#include "__mymytest.h"
#include <limits.h>
#include <string.h>


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

// TODO: Check that the right bytes were written - may need to use read then?
//

static void test_write__emptyString() {
    _assert((ret = write(1, "", 0)) == 0);
    _assert((ret = write(1, "hello", 0)) == 0);
    return;
}

static void test_write__bufferTooBig() {
    // ret = write(1, "", 10);
    // printf("ret = %d ; errno = %d\n", ret, errno); // ---->> ret = 10; errno = 0;  // FIXME: Probably delete this test; can't test it?
    return;
}

static void test_write__nonexistent_fd() {
    _assert((ret = write(3, "hello", 5)) == -1);
    _assert(errno == EBADF);

    _assert((ret = write(OPEN_MAX - 1, "hello", 5)) == -1);
    _assert(errno == EBADF);

    return;
}
static void test_write__no_permission() {
    _assert((ret = write(0, "hi", 2)) == -1);
    _assert(errno == EBADF); // fd is not a valid file descriptor, or was not opened for writing.

    return;
}
static void test_write__out_of_space() {
        // Does this happen with VOP_WRITE
        return;
}
static void test_write__invalid_fd() {
    _assert((ret = write(-1, "hello", 5)) == -1);
    _assert(errno == EBADF);

    // OPEN_MAX - 1 is tested in nonexistent_fd(), and is not tested here

    _assert((ret = write(OPEN_MAX, "hello", 5)) == -1);
    _assert(errno == EBADF);

    _assert((ret = write(OPEN_MAX + 1, "hello", 5)) == -1);
    _assert(errno == EBADF);

    return;
}
