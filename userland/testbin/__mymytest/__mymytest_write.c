#include "__mymytest.h"

static void test_write__emptyString();
static void test_write__bufferTooBig(); // (Can we actualy test this. We can try :) 
static void test_write__nonexistent_fd(); // FD doesn't exist in FD table
static void test_write__no_permission(); // EBADF 
static void test_write__out_of_space(); // ENOSPC
static void test_write__invalid_fd(); // FD >= OPEN_MAX -> EBADF 

void test_open() {
    
}