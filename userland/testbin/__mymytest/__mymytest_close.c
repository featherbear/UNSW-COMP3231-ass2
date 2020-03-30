#include "__mymytest.h"

static void test_close__nonexistent_fd(void); 
static void test_close__invalid_fd(void); 

void test_close() {
    test(test_close__nonexistent_fd); 
    test(test_close__invalid_fd); 
}

static void test_close__nonexistent_fd() {
    close()
    return;
}
static void test_close__invalid_fd() {
    _assert((ret = close(-1)))
    return;
}