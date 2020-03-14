#include "bitmap.h"
#include "stdint.h"
#include "string.h"
#include "print.h"
#include "interrupt.h"
#include "debug.h"
//位图初始化
void bitmap_init(struct bitmap* btmp){
    memset(btmp->bits,0,btmp->btmp_bytes_len);
}
//判断bit_idx位是否是1，是则返回1,否则返回0
int bitmap_scan_test(struct bitmap* btmp,uint32_t bit_idx){
    uint32_t byte_idx = bit_idx >> 3;
    uint8_t bit_remain = bit_idx % 8;
    return ((btmp->bits[byte_idx])&(BITMAP_MASK << bit_remain));
}
//将位图btmp的bit_idx位设置为value
void bitmap_set(struct bitmap* btmp,uint32_t bit_idx,uint8_t value){
    ASSERT((value == 0)||(value == 1));
    uint8_t byte_idx = bit_idx >> 3;
    uint8_t bit_remain = bit_idx % 8;
    if(value){
         btmp->bits[byte_idx] |= (value << bit_remain); 
    }else{
         btmp->bits[byte_idx] &= (value << bit_remain); 
    }
}
//在位图中申请连续cnt个位，申请成功就返回其起始下标，失败就返回-1
int bitmap_scan(struct bitmap* tmp,uint32_t cnt){
    uint32_t byte_idx = 0,bit_idx = 0,bit_idx_start=0,free_cnt=0;
    while((tmp->bits[byte_idx] == 0xff)&&(byte_idx < tmp->btmp_bytes_len)){
        byte_idx++;
    }
    if(byte_idx == tmp->btmp_bytes_len) return -1;
    
    while((tmp->bits[byte_idx]) & (BITMAP_MASK << bit_idx)){
        bit_idx++;
    }
    bit_idx_start = byte_idx*8 + bit_idx;
    while((tmp->btmp_bytes_len) - bit_idx_start){
        if(!bitmap_scan_test(tmp,bit_idx_start+free_cnt)){
            free_cnt++;
        }else{
            bit_idx_start += (free_cnt+1);
            free_cnt = 0;
        }
        if(free_cnt == cnt) return bit_idx_start;
    }
    return -1;
    
}
