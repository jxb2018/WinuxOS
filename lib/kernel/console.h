#ifndef _LIB_KERNEL_CONSOLE_H
#define _LIB_KERNEL_CONSOLE_H
#include "/home/jxb/OS/lib/kernel/stdint.h"
void console_put_str(char* str);
void console_put_int(uint32_t num);
void console_put_char(uint8_t ch_asci);
void console_init(void);
void console_acquire(void);
void console_release(void);
#endif