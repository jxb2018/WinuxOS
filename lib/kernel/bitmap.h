#ifndef _LIB_KERNEL_BITMAP_H
#define _LIB_KERNEL_BITMAP_H
#include "global.h"
#include "stdint.h"
#define BITMAP_MASK 1
struct bitmap{
    uint32_t btmp_bytes_len;//位图的字节长度
    uint8_t* bits;//位图的指针
};
void bitmap_init(struct bitmap* btmp);
int bitmap_scan_test(struct bitmap* btmp,uint32_t bit_idx);
int bitmap_scan(struct bitmap* tmp,uint32_t cnt);
void bitmap_set(struct bitmap* btmp,uint32_t bit_idx,uint8_t value);
#endif