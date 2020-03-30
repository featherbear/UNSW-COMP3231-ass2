#include <stdlib.h>
#include <err.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>

#include "__mymytest.h"

#define TEST_VALID_FILENAME "test.file"
#define TEST_INVALID_FILENAME "random.file"
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

    // Only doing this because I don't know where to keep the file
    fd = open(TEST_VALID_FILENAME, O_RDWR | O_CREAT, TEST_MODE); 
    close(fd); 

    _asssert((fd = open(TEST_VALID_FILENAME, NULL, TEST_MODE)) == -1); 
    // _assert(errno == ) 
    // TODO: Not sure what it should be equal to .. VFS man page doesn't tell us the return error values 
    
    return;
}
static void test_open__invalidFilename() {

    _asssert((fd = open(TEST_FILENAME, O_RDWR , TEST_MODE)) == -1); 
    _assert(errno == EFAULT); 
    return;
}
static void test_open__filetable_full() {

    int fd;
    char *file_name; 
    for (int i = 3; i < 128; i++) { 
        file_name = strcat(TEST_VALID_FILENAME, str(i)); // TODO: Test if this works
        
        fd = open()
    }
    return;
}