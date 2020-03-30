#ifndef ___MYMYTEST_H_
#define ___MYMYTEST_H_

#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>

#define test(fn) ({printf("Executing %s\n", #fn); fn();})
#define _assert(statement) ({printf("├─ Evaluating (%s)", #statement); assert(statement); printf(" success!\n");})
void test_open(void);
void test_close(void);

void test_write(void); 
void test_read(void);

void test_dup2(void);

void test_lseek(void);


#endif /* ___MYMYTEST_H_ */