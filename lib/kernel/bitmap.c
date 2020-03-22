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
         btmp->bits[byte_idx] |= (1 << bit_remain); 
    }else{
         btmp->bits[byte_idx] &= (~(1 << bit_remain)); 
    }
}


/* 在位图中申请连续cnt个位,返回其起始位下标 */
int bitmap_scan(struct bitmap* btmp, uint32_t cnt) {
   uint32_t idx_byte = 0;	 // 用于记录空闲位所在的字节
/* 先逐字节比较,蛮力法 */
   while (( 0xff == btmp->bits[idx_byte]) && (idx_byte < btmp->btmp_bytes_len)) {
/* 1表示该位已分配,所以若为0xff,则表示该字节内已无空闲位,向下一字节继续找 */
      idx_byte++;
   }

   ASSERT(idx_byte < btmp->btmp_bytes_len);
   if (idx_byte == btmp->btmp_bytes_len) {  // 若该内存池找不到可用空间		
      return -1;
   }

 /* 若在位图数组范围内的某字节内找到了空闲位，
  * 在该字节内逐位比对,返回空闲位的索引。*/
   int idx_bit = 0;
 /* 和btmp->bits[idx_byte]这个字节逐位对比 */
   while ((uint8_t)(BITMAP_MASK << idx_bit) & btmp->bits[idx_byte]) { 
	 idx_bit++;
   }
	 
   int bit_idx_start = idx_byte * 8 + idx_bit;    // 空闲位在位图内的下标
   if (cnt == 1) {
      return bit_idx_start;
   }

   uint32_t bit_left = (btmp->btmp_bytes_len * 8 - bit_idx_start);   // 记录还有多少位可以判断
   uint32_t next_bit = bit_idx_start + 1;
   uint32_t count = 1;	      // 用于记录找到的空闲位的个数

   bit_idx_start = -1;	      // 先将其置为-1,若找不到连续的位就直接返回
   while (bit_left-- > 0) {
      if (!(bitmap_scan_test(btmp, next_bit))) {	 // 若next_bit为0
	 count++;
      } else {
	 count = 0;
      }
      if (count == cnt) {	    // 若找到连续的cnt个空位
	 bit_idx_start = next_bit - cnt + 1;
	 break;
      }
      next_bit++;          
   }
   return bit_idx_start;
}
/*
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
*/
