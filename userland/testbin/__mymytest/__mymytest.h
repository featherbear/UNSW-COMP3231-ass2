#ifndef ___MYMYTEST_H_
#define ___MYMYTEST_H_


void test_open();
void test_open__noFlagsProvided(); // EINVAL
void test_open__invalidFilename(); // EFAULT
void test_open__filetable_full(); // EMFILE

void test_close();
void test_close__nonexistent_fd(); 
void test_close__invalid_fd(); 

void test_write(); 
void test_write__emptyString();
void test_write__bufferTooBig(); // (Can we actualy test this. We can try :) 
void test_write__nonexistent_fd(); // FD doesn't exist in FD table
void test_write__no_permission(); // EBADF 
void test_write__out_of_space(); // ENOSPC
void test_write__invalid_fd(); // FD >= OPEN_MAX -> EBADF 

void test_read();
void test_read__end_of_file(); // Should get 0 bytes after reading
void test_read__emptyString();
void test_read__readBeyondFile();
void test_read__nonexistent_fd(); // EBADF 
void test_read__invalid_fd();     // EBADF 
void test_read__no_permission();  // EBADF 

void test_dup2();
void test_dup2__nonexistent_fd(); 
void test_dup2__invalid_fd(); 
/* 
        EMFILE	The process's file table was full, or a process-specific limit on open files was reached.
        ENFILE	The system's file table was full, if such a thing is possible, or a global limit on open files was reached.
*/


void test_lseek__set();
void test_lseek__cur();
void test_lseek__end();
void test_lseek__nonexistent_fd(); // EBADF 
void test_lseek__invalid_fd();     // EBADF 
void test_lseek__unsupportedSeek(); // ESPIPE
void test_lseek__invalidWhence(); // EINVAL
void test_lseek__invalidOffset(); // EINVAL (Resulting seek position would be negative) 



#endif /* ___MYMYTEST_H_ */