#include "__mymytest.h"
#include <string.h>


static void test_write__success(void);
static void test_write__emptyString(void);
static void test_write__nonexistent_fd(void); // FD doesn't exist in FD table
static void test_write__no_permission(void); // EBADF 
static void test_write__invalid_fd(void); // FD >= OPEN_MAX -> EBADF 


void test_write() {
    test(test_write__success);
    test(test_write__emptyString);
    test(test_write__nonexistent_fd);
    test(test_write__no_permission);
    test(test_write__invalid_fd);
}


static void test_write__success() {
    _assert(write(STDOUT_FILENO, "test", 4) == 4);
    _assert(write(STDOUT_FILENO, "test", 3) == 3);
}

static void test_write__emptyString() {
    _assert(write(STDOUT_FILENO, "", 0) == 0);
    _assert(write(STDOUT_FILENO, "hello", 0) == 0);
    return;
}

static void test_write__nonexistent_fd() {
    // As of executing this function, only FD 0 1 2 are open.

    // fd shouldn't exist
    _assert(write(100, "hello", 5) == -1);
    _assert(errno == EBADF);

    _assert(write(OPEN_MAX - 1, "hello", 5) == -1);
    _assert(errno == EBADF);

    // Write into a closed file descriptor
    int closedFd = open("test_closed_write", O_RDWR | O_CREAT, 0700);
    close(closedFd);
    _assert(write(closedFd, "hello", 5) == -1);

    return;
}
static void test_write__no_permission() {
    _assert(write(STDIN_FILENO, "hi", 2) == -1);
    _assert(errno == EBADF); // fd is not a valid file descriptor, or was not opened for writing.

    return;
}

static void test_write__invalid_fd() {
    _assert(write(-1, "hello", 5) == -1);
    _assert(errno == EBADF);

    // OPEN_MAX - 1 is tested in nonexistent_fd(), and is not tested here

    _assert(write(OPEN_MAX, "hello", 5) == -1);
    _assert(errno == EBADF);

    _assert(write(OPEN_MAX + 1, "hello", 5) == -1);
    _assert(errno == EBADF);

    return;
}
