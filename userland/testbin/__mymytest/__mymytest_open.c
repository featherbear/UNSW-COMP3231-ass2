#include <stdlib.h>
#include <err.h>
#include <errno.h>
 #include <sys/types.h>

#include "__mymytest.h"

#define TEST_VALID_FILENAME "test.file"
#define TEST_INVALID_FILENAME "test.file"
#define TEST_MODE 0700

static void test_open__noFlagsProvided(void); // EINVAL
static void test_open__invalidFilename(void); // EFAULT
static void test_open__filetable_full(void); // EMFILE


void test_open() {
    test(test_open__noFlagsProvided);
    test(test_open__invalidFilename);
    test(test_open__filetable_full);
}

static void test_open__noFlagsProvided() { 
    int fd; 
    _asssert((fd = open(TEST_FILENAME, NULL, TEST_MODE)) == -1); 
    // _assert(errno == ) 
    // TODO: Not sure what it should be equal to .. VFS man page doesn't tell us the return error values 
    
    return;
}
static void test_open__invalidFilename() {
    _asssert((fd = open(TEST_FILENAME, NULL, TEST_MODE)) == -1); 
    return;
}
static void test_open__filetable_full() {
    return;
}