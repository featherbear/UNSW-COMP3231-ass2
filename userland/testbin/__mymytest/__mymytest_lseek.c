#include "__mymytest.h"

static void test_lseek__set();
static void test_lseek__cur();
static void test_lseek__end();
static void test_lseek__nonexistent_fd(); // EBADF 
static void test_lseek__invalid_fd();     // EBADF 
static void test_lseek__unsupportedSeek(); // ESPIPE
static void test_lseek__invalidWhence(); // EINVAL
static void test_lseek__invalidOffset(); // EINVAL (Resulting seek position would be negative) 

void test_lseek() {
    
}