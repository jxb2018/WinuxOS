#ifndef _LIB_KERNEL_MEMORY_H
#define _LIB_KERNEL_MEMORY_H
#include "stdint.h"
#include "bitmap.h"
#include "list.h"
#include "/home/jxb/OS/thread/sync.h"
// 物理内存地址池
struct pool{
    struct bitmap pool_bitmap;   //本内存池所用到的位图结构，用来管理物理内存
    uint32_t phy_address_start; //本内存池所管理物理内存的起始地址
    uint32_t pool_size;         //本内存池字节容量
    struct lock lock; //互斥访问
};
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

//内存块
struct mem_block{
    list_elem free_elem;
};
//内存块描述符
struct mem_block_desc{
    uint32_t block_size;  //内存块大小
    uint32_t blocks_pre_arena;  //本arena中所容纳mem_block的数量
    list free_list;        //目前可用的mem_block链表
};

//内存仓库 包含一种规格的内存块
struct arena{
    struct mem_block_desc* desc;
    uint32_t cnt;
    bool large; 
     //large 为true，cnt表示的是页框数；large为false，cnt表示的是空闲mem_block的数量
};
void block_desc_init(struct mem_block_desc* desc_array);
void* sys_malloc(uint32_t size);
#define DESC_CNT 7  //构建7种规格的内存块描述符 16 32 64 128 256 512 1024

void pfree(uint32_t pg_phy_addr);
void vaddr_remove(enum pool_flags pf,void* _vaddr,uint32_t pg_cnt);
void page_table_pte_remove(uint32_t vaddr);
void mfree_page(enum pool_flags pf,void* _vaddr,uint32_t pg_cnt);
void sys_free(void* ptr);

#endif