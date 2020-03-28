/*
 * Declarations for file handle and file table management.
 */

#ifndef _FILE_H_
#define _FILE_H_

#include <__file_descriptor_table.h>
#include <__open_file_table.h>

#include <types.h>

fd_t sys_open(userptr_t filename, int flags, mode_t mode, int *errno);
int sys_close(fd_t fd, int *errno); 
int sys_read(fd_t fd, userptr_t buf, size_t buflen, int *errno);
int sys_write(fd_t fd, userptr_t buf, size_t buflen, int *errno);
off_t sys_lseek(fd_t fd, off_t pos, int whence, int *errno);
fd_t sys_dup2(fd_t oldfd, fd_t newfd, int *errno);

#endif /* _FILE_H_ */
