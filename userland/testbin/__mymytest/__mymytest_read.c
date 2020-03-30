#include "__mymytest.h"

static void test_read__end_of_file(); // Should get 0 bytes after reading
static void test_read__emptyString();
static void test_read__readBeyondFile();
static void test_read__nonexistent_fd(); // EBADF 
static void test_read__invalid_fd();     // EBADF 
static void test_read__no_permission();  // EBADF 

void test_read() {
    
}