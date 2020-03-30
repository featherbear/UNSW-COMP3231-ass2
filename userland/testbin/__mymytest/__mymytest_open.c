#include "__mymytest.h"

static void test_open__noFlagsProvided(); // EINVAL
static void test_open__invalidFilename(); // EFAULT
static void test_open__filetable_full(); // EMFILE

void test_open() {

}