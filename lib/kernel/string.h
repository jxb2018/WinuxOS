#ifndef _LIB_STRING_H
#define _LIB_STRING_H
#include "stdint.h"
;void memset(void* dst_,uint8_t value,uint32_t size);        // ok
void memcpy(void* dst_,const void* src_,uint32_t size);     // ok
int memcmp(const void* a_,const void* b_,uint32_t size);    // ok
char* strcpy(char* dst_,const char* src_);                  // ok
uint32_t strlen(const char* str);                           // ok
int strcmp(const char* a,const char* b);                    // ok
char* strchr(const char* str,const uint8_t ch);             // ok
char* strrchr(const char* str,const uint8_t ch);            // ok
char* strcat(char* dst_,const char* src_);                  // ok
uint32_t strchrs(const char* str,uint8_t ch);               // ok

#endif