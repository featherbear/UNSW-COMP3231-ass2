#include "__mymytest.h"

static void test_read__end_of_file(); // Should get 0 bytes after reading
static void test_read__emptyString();
static void test_read__readBeyondFile();
static void test_read__nonexistent_fd(); // EBADF 
static void test_read__invalid_fd();     // EBADF 
static void test_read__no_permission();  // EBADF 

void test_read() {
    test_read__end_of_file();
    test_read__emptyString();
    test_read__readBeyondFile();
    test_read__nonexistent_fd();
    test_read__invalid_fd();
    test_read__no_permission();
}

static void test_read__end_of_file() {

}
static void test_read__emptyString() {

}
static void test_read__readBeyondFile() {

}
static void test_read__nonexistent_fd() {

}
static void test_read__invalid_fd() {

}
static void test_read__no_permission() {

}