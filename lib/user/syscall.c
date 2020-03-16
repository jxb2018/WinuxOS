#include "syscall.h"
#include "/home/jxb/OS/lib/kernel/stdint.h"
//无参数的系统调用
#define _syscall0(NUMBER) ({int retval;asm volatile("int $0x80":"=a"(retval):"a"(NUMBER):"memory");retval;})
   // 大括号中最后一个语句的值，会作为大括号的返回值

//三个参数的系统调用
#define _syscall3(NUMBER,ARG1,ARG2,ARG3) ({int retval;asm volatile("int $0x80":"=a"(retval):"a"(NUMBER),"b"(ARG1),"c"(ARG2),"d"(ARG3):"memory");retval;})

// 返回当前任务的pid
uint32_t getpid(){
    return _syscall0(SYS_GETPID);
}