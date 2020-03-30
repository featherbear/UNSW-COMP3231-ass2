#include "__mymytest.h"

#define TEST_FILE "closeFile"

static void test_close__nonexistent_fd(void); 
static void test_close__closed_fd(void); 
static void test_close__invalid_fd(void); 

void test_close() {
    test(test_close__nonexistent_fd); 
    test(test_close__closed_fd); 
    test(test_close__invalid_fd); 
}

static void test_close__nonexistent_fd() {
    _assert(close(130) == -1);
    _assert(errno  == EBADF);
}

static void test_close__closed_fd() {

    int fd = open(TEST_FILE, O_RDWR | O_CREAT);

    // Initial close should work
    _assert(close(fd) == 0);

    // Second close should fail
    _assert(close(fd) == -1);
    _assert(errno  == EBADF);

    return;
}

static void test_close__invalid_fd() {
    _assert(close(-1) == -1);
    _assert(errno == EBADF);

    _assert(close(OPEN_MAX) == -1);
    _assert(errno == EBADF);

    _assert(close(OPEN_MAX + 1) == -1);
    _assert(errno == EBADF);

    return;
}