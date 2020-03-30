#include "__mymytest.h"

#define TEST_FILENAME "insert_testing_filename.txt"

static void test_open__noFlagsProvided(void); // EINVAL
static void test_open__invalidFilename(void); // EFAULT
static void test_open__filetable_full(void); // EMFILE

void test_open() {
    test_open__noFlagsProvided();
    test_open__invalidFilename();
    test_open__filetable_full();
}

static void test_open__noFlagsProvided() { 
    int fd; 
    fd = fopen(TEST_FILENAME, ); 
    
    return;
}
static void test_open__invalidFilename() {
    return;
}
static void test_open__filetable_full() {
    return;
}