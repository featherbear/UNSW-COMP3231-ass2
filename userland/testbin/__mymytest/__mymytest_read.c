#include <stdlib.h>
#include "__mymytest.h"

#define MAX_BUF 500 
#define TEST_LENGTH_GT_MAX 501
#define TEST_LENGTH_ZERO 0
#define TEST_FILENAME "test.file"
#define TEST_MODE 0700

static void test_read__end_of_file(void); // Should get 0 bytes after reading
static void test_read__emptyString(void);
static void test_read__readBeyondFile(void); // EBADF
static void test_read__nonexistent_fd(void); // EBADF 
static void test_read__invalid_fd(void);     // EBADF 
static void test_read__no_permission(void);  // EBADF 

void test_read() {
    test(test_read__no_permission);
    test(test_read__end_of_file);
    test(test_read__emptyString);
    test(test_read__readBeyondFile);
    test(test_read__nonexistent_fd);
    test(test_read__invalid_fd);
}

int fd; 
char buf[MAX_BUF];

static void test_read__no_permission() {

    fd = open(TEST_FILENAME, O_WRONLY | O_CREAT, TEST_MODE); 
    _assert((read(fd, &buf[0], TEST_LENGTH_GT_MAX)) == -1);
    // _assert(errno == ) // FIND OUT WHAT VOP_READ returns for the rror vlue
    close(fd); 
    return;
}
static void test_read__end_of_file() {

    fd = open(TEST_FILENAME, O_RDWR | O_CREAT, TEST_MODE); 

    // Read to the end
    _assert((read(fd, &buf[0], TEST_LENGTH_GT_MAX)) == TEST_LENGTH_GT_MAX);

    // Try to read again 
    _assert((read(fd, &buf[0], 1) == 0); 

    close(fd); 
    return;
}
static void test_read__emptyString() {

    buf[0] = "A";

    fd = open(TEST_FILENAME, O_RDWR | O_CREAT, TEST_MODE); 
    _assert((read(fd, &buf[0], TEST_LENGTH_ZERO)) == 0); 

    // Check that buf hasn't been altered
    _assert(buf[0] == "A"); 
    close(fd); 
    return;
}
static void test_read__readBeyondFile() {

    fd = open(TEST_FILENAME, O_RDWR | O_CREAT, TEST_MODE); 
    _assert((read(fd, &buf[0], TEST_LENGTH_GT_MAX)) == -1);
    _assert(errno == E); 
    close(fd); 
    return;
}
static void test_read__nonexistent_fd() {
    
    fd = open(TEST_FILENAME, O_RDWR | O_CREAT, TEST_MODE); 
    close(fd); 

    // Now that fd doesn't exist anymore. 
    _assert((read(fd, &buf[0], TEST_LENGTH_ZERO) == -1); 
    _assert(errno == )

    return;
}
static void test_read__invalid_fd() {
    return;
}
