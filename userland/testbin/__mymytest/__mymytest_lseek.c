#include "__mymytest.h"

static void test_lseek__set(void);
static void test_lseek__cur(void);
static void test_lseek__end(void);
static void test_lseek__nonexistent_fd(void); // EBADF 
static void test_lseek__invalid_fd(void);     // EBADF 
static void test_lseek__unsupportedSeek(void); // ESPIPE
static void test_lseek__invalidWhence(void); // EINVAL
static void test_lseek__invalidOffset(void); // EINVAL (Resulting seek position would be negative) 

int fd;

void test_lseek() {
    fd = open("test_lseek", O_RDWR | O_CREAT);
    _assert(fd != -1);

    test(test_lseek__set);
    test(test_lseek__cur);
    test(test_lseek__end);
    test(test_lseek__nonexistent_fd);
    test(test_lseek__invalid_fd);
    test(test_lseek__unsupportedSeek);
    test(test_lseek__invalidWhence);
    test(test_lseek__invalidOffset);

    _assert(close(fd) != 0);
}


static void test_lseek__set() {
    return;
}
static void test_lseek__cur() {
    return;
}
static void test_lseek__end() {
    return;
}
static void test_lseek__nonexistent_fd() {
    return;
}
static void test_lseek__invalid_fd() {
    return;
}
static void test_lseek__unsupportedSeek() {
    return;
}
static void test_lseek__invalidWhence() {
    return;
}
static void test_lseek__invalidOffset() {
    return;
}
