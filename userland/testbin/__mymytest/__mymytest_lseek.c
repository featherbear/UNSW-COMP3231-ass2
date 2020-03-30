#include "__mymytest.h"

static void test_lseek__set(void);
static void test_lseek__cur(void);
static void test_lseek__end(void);
static void test_lseek__nonexistent_fd(void); // EBADF 
static void test_lseek__invalid_fd(void);     // EBADF 
static void test_lseek__unsupportedSeek(void); // ESPIPE
static void test_lseek__invalidWhence(void); // EINVAL
static void test_lseek__invalidOffset(void); // EINVAL (Resulting seek position would be negative) 

void test_lseek() {
    test(test_lseek__set);
    test(test_lseek__cur);
    test(test_lseek__end);
    test(test_lseek__nonexistent_fd);
    test(test_lseek__invalid_fd);
    test(test_lseek__unsupportedSeek);
    test(test_lseek__invalidWhence);
    test(test_lseek__invalidOffset);
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
