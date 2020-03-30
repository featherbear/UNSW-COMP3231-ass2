#include "__mymytest.h"

static void test_dup2__nonexistent_fd(); 
static void test_dup2__invalid_fd(); 
/* 
        EMFILE	The process's file table was full, or a process-specific limit on open files was reached.
        ENFILE	The system's file table was full, if such a thing is possible, or a global limit on open files was reached.
*/

void test_dup2() {
    
}