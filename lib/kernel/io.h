#ifndef _LIB_IO_H
#define _LIB_IO_H
#include "/home/jxb/OS/lib/kernel/stdint.h"
//从端口port读入的一个字节返回
static inline uint8_t inb(uint16_t port){
    uint8_t ret;
    asm volatile("inb %w1,%b0":"=a"(ret):"Nd"(port));
    return ret;
}
//从端口port读入的word_cnt个字写入addr
static inline void insw(uint16_t port,const void *addr,uint32_t word_cnt){
    asm volatile("cld;rep insw":"+D"(addr),"+c"(word_cnt):"d"(port):"memory");
}
//向端口port写入一个字节
static inline void outb(uint16_t port,uint8_t data){
    asm volatile("outb %b0,%w1"::"a"(data),"Nd"(port));
}
//将addr处起始的word_cnt个字写入端口port
static inline void outsw(uint16_t port,const void *addr,uint32_t word_cnt){
    asm volatile("cld;rep outsw":"+S"(addr),"+c"(word_cnt):"d"(port));
    // outsw 将ds:esi处的内容写入port端口
}
#endif