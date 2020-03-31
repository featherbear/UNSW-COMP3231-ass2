#include "__mymytest.h"
#include <fcntl.h>


static void test_lseek__set(void);
static void test_lseek__cur(void);
static void test_lseek__end(void);
static void test_lseek__nonexistent_fd(void); // EBADF 
static void test_lseek__invalid_fd(void);     // EBADF 
static void test_lseek__unsupportedSeek(void); // ESPIPE
static void test_lseek__invalidWhence(void); // EINVAL
static void test_lseek__invalidOffset(void); // EINVAL (Resulting seek position would be negative) 

int fd;
char buff;
char writeBuff[22] = "-----1----0----1----2";


void test_lseek() {
    fd = open("test_lseek", O_RDWR | O_CREAT);
    _assert(fd != -1);

    write(fd, writeBuff, sizeof(writeBuff));


    test(test_lseek__set);
    test(test_lseek__cur);
    test(test_lseek__end);
    test(test_lseek__nonexistent_fd);
    test(test_lseek__invalid_fd);
    test(test_lseek__unsupportedSeek);
    test(test_lseek__invalidWhence);
    test(test_lseek__invalidOffset);

    _assert(close(fd) == 0);
}


static void test_lseek__set() {

    _assert(lseek(fd, 10, SEEK_SET) == 10);
    _assert(read(fd, &buff, 1) == 1);
    _assert(buff == '0'); // Offset 10 should have '0' (ASCII 30)

    return;
}
static void test_lseek__cur() {
    _assert(lseek(fd, 10, SEEK_SET) == 10);  // First set to offset 10

    _assert(lseek(fd, 5, SEEK_CUR) == 15);   // Increase offset by 5 (Offset now 15)
    _assert(read(fd, &buff, 1) == 1);        // Read 1 byte (Offset now 16)
    _assert(buff == '1');                    // Offset 15 should have '2' (ASCII 32)

    return;
}
static void test_lseek__end() {
    _assert(lseek(fd, -2, SEEK_END) == 20);  // Get to offset 20

    _assert(read(fd, &buff, 1) == 1);        // Read 1 byte (Offset now 21) (end byte)
    _assert(buff == '2');                    // Offset 20 should have '1' (ASCII 31)

    return;
}
static void test_lseek__nonexistent_fd() {
    _assert(lseek(150, 0, SEEK_SET) == -1);
    _assert(errno == EBADF);

    // Seek a closed fd
    close(fd);
    _assert(lseek(fd, 0, SEEK_SET) == -1);
    fd = open("test_lseek", O_RDWR | O_CREAT);

    return;
}
static void test_lseek__invalid_fd() {
    _assert(lseek(-1, 0, SEEK_SET) == -1);
    _assert(errno == EBADF);

    _assert(lseek(OPEN_MAX, 0, SEEK_SET) == -1);
    _assert(errno == EBADF);
    
    return;
}
static void test_lseek__unsupportedSeek() {
    _assert(lseek(STDIN_FILENO, 0, SEEK_SET) == -1);
    _assert(errno == ESPIPE);
    
    _assert(lseek(STDOUT_FILENO, 0, SEEK_SET) == -1);
    _assert(errno == ESPIPE);

    _assert(lseek(STDERR_FILENO, 0, SEEK_SET) == -1);
    _assert(errno == ESPIPE);

    return;
}
static void test_lseek__invalidWhence() {
    // whence values are defined in seek.h
    
    // #define SEEK_SET      0
    // #define SEEK_CUR      1
    // #define SEEK_END      2  

    _assert(lseek(fd, 0, 3) == -1);
    _assert(errno == EINVAL);

    return;
}
static void test_lseek__invalidOffset() {
    // Check invalid negative offset

    _assert(lseek(fd, 0, SEEK_SET) == 0);   // First set the file to an offset of 0
    _assert(lseek(fd, -1, SEEK_SET) == -1);
    _assert(errno == EINVAL);
    _assert(lseek(fd, -1, SEEK_CUR) == -1);
    _assert(errno == EINVAL);

    _assert(lseek(fd, 10, SEEK_SET) == 10); // Set the file to an offset of 10
    _assert(lseek(fd, -11, SEEK_CUR) == -1);
    _assert(errno == EINVAL);

    _assert(lseek(fd, -24, SEEK_END) == -1);
    _assert(errno == EINVAL);

    return;
}
