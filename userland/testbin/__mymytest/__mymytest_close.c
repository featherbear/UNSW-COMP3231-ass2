#include "__mymytest.h"

static void test_close__nonexistent_fd(void); 
static void test_close__closed_fd(void); 
static void test_close__invalid_fd(void); 

void test_close() {
    test(test_close__nonexistent_fd); 
    test(test_close__closed_fd); 
    test(test_close__invalid_fd); 
}

static void test_close__nonexistent_fd() {
    _assert((ret = close(130)) == -1);
    _assert(errno  == EBADF);
}

static void test_close__closed_fd() {

    int fd = open("", O_RDONLY, 700);

    // Initial close should work
    _assert((ret = close(fd)) == 0);

    // Second close should fail
    _assert((ret = close(fd)) == -1);
    _assert(errno  == EBADF);

    return;
}

static void test_close__invalid_fd() {
    _assert((ret = close(-1)) == -1);
    _assert((errno == EBADF));

    _assert((ret = close(OPEN_MAX)) == -1);
    _assert((errno == EBADF));

    _assert((ret = close(OPEN_MAX + 1)) == -1);
    _assert((errno == EBADF));

    return;
}