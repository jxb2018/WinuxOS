#ifndef __USERPROG_SYSCALLINIT_H
#define __USERPROG_SYSCALLINIT_H
#include "stdint.h"
#define syscall_nr 32 
typedef void* syscall;
syscall syscall_table[syscall_nr];
void syscall_init(void);
uint32_t sys_getpid(void);

#endif
