#ifndef _LIB_KERNEL_MEMORY_H
#define _LIB_KERNEL_MEMORY_H
#include "stdint.h"
#include "bitmap.h"
struct virtual_addr{
    struct bitmap vaddr_bitmap; //虚拟地址用到的位图结构
    uint32_t vaddr_start;//虚拟地址起始地址 ？以这个地址为起始进行分配
};
extern struct pool kernel_pool,user_pool;
void mem_init(void);// 完成物理内核地址池、物理用户地址池、虚拟内核地址池的初始化

// 内存池标记 用来判断用哪个内存池
enum pool_flags{
    POOL_FLAG_KERNEL = 1,
    POOL_FLAG_USER   = 2
};
#define PG_P_1  1
#define PG_P_0  0
#define PG_RW_R 0
#define PG_RW_W 2
#define PG_US_S 0 //系统级
#define PG_US_U 4 //用户级
void *get_kernel_page(uint32_t pg_cnt);
uint32_t* get_pde_ptr(uint32_t vaddr);
uint32_t* get_pte_ptr(uint32_t vaddr);
void* malloc_page(enum pool_flags pf,uint32_t pg_cnt);
void* get_user_page(uint32_t pg_cnt);
void* get_a_page(enum pool_flags pf,uint32_t vaddr);
uint32_t addr_v2p(uint32_t vaddr);
#endif