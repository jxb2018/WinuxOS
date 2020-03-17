#ifndef _LIB_USER_SYSCALL_H
#define _LIB_USER_SYSCALL_H
#include "/home/jxb/OS/lib/kernel/stdint.h"
enum SYSCALL_NR{ //用来存放系统调用子功能号
    SYS_GETPID,
    SYS_WRITE,
    SYS_MALLOC,
    SYS_FREE
};
uint32_t getpid(void); //得到pid
uint32_t write(char* str); //写
void* malloc(uint32_t size);//分配内存
void free(void* ptr); //回收内存
#endif