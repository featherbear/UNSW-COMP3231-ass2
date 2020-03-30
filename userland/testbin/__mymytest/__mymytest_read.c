#include "__mymytest.h"

static void test_read__end_of_file(void); // Should get 0 bytes after reading
static void test_read__emptyString(void);
static void test_read__readBeyondFile(void);
static void test_read__nonexistent_fd(void); // EBADF 
static void test_read__invalid_fd(void);     // EBADF 
static void test_read__no_permission(void);  // EBADF 

void test_read() {
    test(test_read__end_of_file);
    test(test_read__emptyString);
    test(test_read__readBeyondFile);
    test(test_read__nonexistent_fd);
    test(test_read__invalid_fd);
    test(test_read__no_permission);
}

static void test_read__end_of_file() {
    return;
}
static void test_read__emptyString() {
    return;
}
static void test_read__readBeyondFile() {
    return;
}
static void test_read__nonexistent_fd() {
    return;
}
static void test_read__invalid_fd() {
    return;
}
static void test_read__no_permission() {
    return;
}