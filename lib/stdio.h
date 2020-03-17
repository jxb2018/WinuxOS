#ifndef _LIB_STDIO_H
#define _LIB_STDIO_H
#include "/home/jxb/OS/lib/kernel/stdint.h"
typedef char* va_list;
uint32_t printf(const char* str,...);
uint32_t vsprintf(char* str,const char* format,va_list ap);
#endif