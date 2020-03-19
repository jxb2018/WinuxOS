#include "/home/jxb/OS/lib/stdio.h"
#include "/home/jxb/OS/lib/kernel/stdint.h"
#include "/home/jxb/OS/lib/kernel/debug.h"
#include "/home/jxb/OS/lib/kernel/string.h"
#include "/home/jxb/OS/lib/user/syscall.h"
#include "/home/jxb/OS/lib/kernel/console.h"
#include "/home/jxb/OS/lib/kernel/stdio_kernel.h"
//格式化输出字符串 format
void printk(const char* format,...){
    va_list args = (va_list)&format;  //args 指向 format
    char buf[1024] = {0}; //存储拼接后的字符串
    vsprintf(buf,format,args);
    console_put_str(buf);
}