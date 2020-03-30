#include "__mymytest.h"

#include <stdio.h>

static void test_write__emptyString(void);
static void test_write__bufferTooBig(void); // (Can we actualy test this. We can try :) 
static void test_write__nonexistent_fd(void); // FD doesn't exist in FD table
static void test_write__no_permission(void); // EBADF 
static void test_write__out_of_space(void); // ENOSPC
static void test_write__invalid_fd(void); // FD >= OPEN_MAX -> EBADF 

void test_write() {
    test_write__emptyString();
    test_write__bufferTooBig();
    test_write__nonexistent_fd();
    test_write__no_permission();
    test_write__out_of_space();
    test_write__invalid_fd();
}

static void test_write__emptyString() {
    return;
}
static void test_write__bufferTooBig() {
    return;
}
static void test_write__nonexistent_fd() {
        return;
}
static void test_write__no_permission() {
        return;
}
static void test_write__out_of_space() {
        return;
}
static void test_write__invalid_fd() {
    printf("Test!\n");
    printf("%d", errno);
        return;
}
