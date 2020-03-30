#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>

#include "__mymytest.h" // Function prototypes in `__mymytest.h`

int main(int argc, char * argv[]) {
        (void) argc;
        (void) argv;
        
        test_open();

        test_close();

        test_write();
        
        test_read();

        test_lseek();

        test_dup2();

        return 0;
}


