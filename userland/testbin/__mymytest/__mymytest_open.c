#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <stdbool.h>

#include "__mymytest.h"

#define TEST_VALID_FILENAME "test.file"
#define TEST_INVALID_FILENAME "random.file"
#define INVALID_FLAG 1234
#define FD_MAX_SIZE 128
#define TEST_MODE 0700

static void test_open__noFlagsProvided(void); // EINVAL
static void test_open__invalidFilename(void); // EFAULT
static void test_open__filetable_full(void); // EMFILE


void test_open() {
    test(test_open__noFlagsProvided);
    test(test_open__invalidFilename);
    test(test_open__filetable_full);
}

// Global Variable
int fd;

static void test_open__noFlagsProvided() { 

    // Only doing this because I don't know where to keep the file
    fd = open(TEST_VALID_FILENAME, O_RDWR | O_CREAT, TEST_MODE); 
    close(fd); 

    _assert((fd = open(TEST_VALID_FILENAME, INVALID_FLAG, TEST_MODE)) == -1); 
    _assert(errno == EINVAL); 

    return;
}
static void test_open__invalidFilename() {
    // FIXME: What's considered invalid?

    _assert((fd = open(TEST_INVALID_FILENAME, O_RDWR , TEST_MODE)) == -1); 
    _assert(errno == EFAULT); 
    return;
}

// TODO:  Implement    ENOENT		The named file does not exist, and O_CREAT was not specified.


static void test_open__filetable_full() {

    int fd_max_reached = false; 
    int *opened_fds = malloc(FD_MAX_SIZE);
    for (int i = 0; i < FD_MAX_SIZE; i++) { opened_fds[i] = 0; }  

    for (int i = 3; i < FD_MAX_SIZE; i++) { 
        fd = open(TEST_VALID_FILENAME, O_RDWR | O_CREAT, TEST_MODE); 
        if (fd > 0) opened_fds[i] = fd;          
        else { 
            fd_max_reached == true; 
            break; 
        }
    }

    // Close all the opened files
    for (int i = 3; i < FD_MAX_SIZE; i++) { 
        if (opened_fds[i] != 0) close(i); 
    }
    
    free(opened_fds);
    _assert(fd_max_reached == true); 
    return;
}