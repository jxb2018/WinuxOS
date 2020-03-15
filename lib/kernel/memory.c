#include "memory.h"
#include "stdint.h"
#include "print.h"
#include "debug.h"
#include "string.h"
#include "bitmap.h"
#include "/home/jxb/OS/thread/sync.h"
#include "/home/jxb/OS/thread/thread.h"
#define PG_SIZE 4096 //页大小 4k
#define MEM_BITMAP_ADDRESS 0xc009a000  //位图的位置
#define K_HEAP_START 0xc0100000   //跨过低端1M内存
#define PDE_IDX(addr) ((addr & 0xffc00000)>>22)
#define PTE_IDX(addr) ((addr & 0x003ff000)>>12)
// 物理内存地址池
struct pool{
    struct bitmap pool_bitmap;   //本内存池所用到的位图结构，用来管理物理内存
    uint32_t phy_address_start; //本内存池所管理物理内存的起始地址
    uint32_t pool_size;         //本内存池字节容量
    struct lock lock; //互斥访问
};
struct pool kernel_pool,user_pool;//内核内存地址池 用户内存地址池
struct virtual_addr kernel_vaddr; //?

//初始化内存地址池
static void mem_pool_init(uint32_t total_mem){
    put_str("mem_pool_init start\n");
    // 256的由来  页目录表 + 0号和768号页目录项指向的同一个页表 + 769～1022个页目录项指向的254个页表
    uint32_t page_table_size = PG_SIZE * 256;
    uint32_t used_mem = 0x100000 + page_table_size; //低端1M + 页表
    uint32_t free_mem = total_mem - used_mem;
    uint32_t all_free_page = free_mem /PG_SIZE; //对于以页为单位的分配策略，不足4k的不必考虑
    uint32_t kernel_free_page = all_free_page/2;
    uint32_t user_free_page = all_free_page - kernel_free_page;

    uint32_t kernel_btmp_bytes_len = kernel_free_page >> 3 ; //余数不做处理
    uint32_t user_btmp_bytes_len = user_free_page >> 3;

    uint32_t kernel_pool_start = used_mem; //内核地址池的起始地址
    uint32_t user_pool_start = kernel_pool_start + kernel_free_page*PG_SIZE; //用户地址池的起始地址

    //初始化内核地址池
    kernel_pool.pool_bitmap.btmp_bytes_len   = kernel_btmp_bytes_len;
    kernel_pool.pool_bitmap.bits             = (void *)MEM_BITMAP_ADDRESS; 
    kernel_pool.phy_address_start            = kernel_pool_start;
    kernel_pool.pool_size                    = kernel_free_page*PG_SIZE;

    //初始化用户地址池
    user_pool.pool_bitmap.btmp_bytes_len     = user_btmp_bytes_len;
    user_pool.pool_bitmap.bits               = (void *)(MEM_BITMAP_ADDRESS + kernel_btmp_bytes_len); 
    user_pool.phy_address_start              = user_pool_start;
    user_pool.pool_size                      = user_free_page*PG_SIZE;
     

     /***********************输出内存池信息**************************************/
     //内核地址池
     put_str("kernel_pool_bitmap is located in: 0x");
     put_int((int)kernel_pool.pool_bitmap.bits);put_char('\n');
     put_str("   kernel_pool_phy_address_start: 0x");
     put_int(kernel_pool.phy_address_start);put_str("\n\n");
     //用户地址池
     put_str("user_pool_bitmap is located in: 0x");
     put_int((int)user_pool.pool_bitmap.bits);put_char('\n');
     put_str("   user_pool_phy_address_start: 0x");
     put_int(user_pool.phy_address_start);put_str("\n\n");     

     //将位图置零
     bitmap_init(&kernel_pool.pool_bitmap);
     bitmap_init(&user_pool.pool_bitmap);
     //初始化内核虚拟地址
    //内核虚拟地址的位图所在位置
     kernel_vaddr.vaddr_bitmap.bits = (void *)(MEM_BITMAP_ADDRESS+kernel_btmp_bytes_len+user_btmp_bytes_len);
     kernel_vaddr.vaddr_bitmap.btmp_bytes_len = kernel_btmp_bytes_len;
     kernel_vaddr.vaddr_start = K_HEAP_START;
     bitmap_init(&kernel_vaddr.vaddr_bitmap);

     //初始化锁
     lock_init(&kernel_pool.lock);
     lock_init(&user_pool.lock);
     put_str("mem_pool_init done\n");
}
// 在pool_flag表示的虚拟内存地址池中申请pg_cnt个虚拟页，成功返回虚拟页的起始地址，失败则返回NULL
static void* vadder_get(enum pool_flags pf,uint32_t pg_cnt){
    void* vaddr_start;
    int bit_idx_start = -1;
    uint32_t cnt = 0;
    if(pf == POOL_FLAG_KERNEL){//内核地址池
        int bit_idx_start = bitmap_scan(&kernel_vaddr.vaddr_bitmap,pg_cnt);
        if(bit_idx_start == -1){
            return NULL;
        }
        vaddr_start = (void*)(kernel_vaddr.vaddr_start + bit_idx_start*PG_SIZE);
        while(pg_cnt--){
            bitmap_set(&kernel_vaddr.vaddr_bitmap,bit_idx_start++,1);
        }
    }else{//用户地址池
        struct task_struct* cur_thread = running_thread();
        bit_idx_start = bitmap_scan(&cur_thread->userprog_vadder.vaddr_bitmap,pg_cnt);
        if(bit_idx_start == -1)
            return NULL;
        while(cnt < pg_cnt){
            bitmap_set(&cur_thread->userprog_vadder.vaddr_bitmap,bit_idx_start+cnt,1);
            cnt++;
        }
        vaddr_start = (void*)(cur_thread->userprog_vadder.vaddr_start + bit_idx_start*PG_SIZE);
        ASSERT((uint32_t)vaddr_start < (0xc0000000 - PG_SIZE));//----------------（）
    }
    return vaddr_start;
}






// 得到虚拟地址vaddr对应的pde指针
uint32_t* get_pde_ptr(uint32_t vaddr){
    // 0xfffff00访问到表本身所在的地址
    return (uint32_t*)(0xffc00000 + (0xffc00000 >> 10) + PDE_IDX(vaddr)*4);
}
// 得到虚拟地址vaddr对应的pte指针
uint32_t* get_pte_ptr(uint32_t vaddr){
    return (uint32_t*)(0xffc00000 + ((vaddr & 0xffc00000) >> 10) + PTE_IDX(vaddr) * 4); 
}
// 在m_pool指向的物理内存池中分配1个物理页 成功则返回物理页的地址，失败则返回NULL
static void* palloc(struct pool* m_pool){
    int bit_idx_start = bitmap_scan(&m_pool->pool_bitmap,1);
    if(bit_idx_start == -1) return NULL;
    bitmap_set(&m_pool->pool_bitmap,bit_idx_start,1);
    return (void*)(bit_idx_start*PG_SIZE + m_pool->phy_address_start);
}
//在页表中添加虚拟地址和物理地址的映射  低12位为零
static void page_table_add(void* _vaddr,void* _paddr){
    uint32_t vaddr = (uint32_t)_vaddr;
    uint32_t paddr = (uint32_t)_paddr;
    uint32_t* pde = get_pde_ptr(vaddr);
    uint32_t* pte = get_pte_ptr(vaddr);
    //put_str("page_table_add: pde:");put_int((int)pde);put_str(" pte:");put_int((int)pte);put_char('\n');

    if(*pde & 0x00000001){
        ASSERT(!(*pte & 0x00000001));//创建页表 pte不应该存在
        *pte = (paddr | PG_US_U | PG_RW_W | PG_P_1);
    }else{
        uint32_t pde_phy = (uint32_t)palloc(&kernel_pool);
        
        *pde = (pde_phy | PG_US_U | PG_RW_W | PG_P_1);
        //将分配到的物理页pde_phy对应的物理内存清0
        memset((void *)((uint32_t)pte & 0xfffff000),0,PG_SIZE);
        ASSERT(!(*pte & 0x00000001));
        *pte = (paddr | PG_US_U | PG_RW_W | PG_P_1);
    }
}
// 分配pg_cnt个页空间，成功则返回起始虚拟地址，失败时返回NULL
void* malloc_page(enum pool_flags pf,uint32_t pg_cnt){
    ASSERT(pg_cnt > 0);
    void* vadder_start = vadder_get(pf,pg_cnt);
    if(vadder_start == NULL){
        return NULL;
    }
    uint32_t vaddr = (uint32_t)(vadder_start);
    struct pool* mem_pool = (pf == POOL_FLAG_KERNEL)? &kernel_pool: &user_pool;
    uint32_t cnt = pg_cnt;
    while(cnt--){

        void* page_phy = palloc(mem_pool);
       // put_str("malloc_page: phy:");put_int((int)page_phy);put_str(" vaddr:");put_int((int)vaddr);put_char('\n');
        if(page_phy == NULL){ //如何回滚
            return NULL;
        }
        page_table_add((void *)vaddr,page_phy);
        vaddr += PG_SIZE;        
    }
    return vadder_start;

}
//从内核物理地址池中申请1页内存 成功则返回虚拟地址，失败则返回NULL
void *get_kernel_page(uint32_t pg_cnt){
    //ASSERT(pg_cnt == 1);
    void* vaddr = malloc_page(POOL_FLAG_KERNEL,pg_cnt);
    if(vaddr != NULL){
        memset(vaddr,0,pg_cnt*PG_SIZE);
    }
    return vaddr;
}
//在用户空间申请4K内存，并返回虚拟地址
void* get_user_page(uint32_t pg_cnt){
    lock_acquire(&user_pool.lock);
    void* vaddr = malloc_page(POOL_FLAG_USER,pg_cnt);
    if(vaddr != NULL){
        memset(vaddr,0,PG_SIZE*pg_cnt);
    }
    lock_release(&user_pool.lock);
    return vaddr;
}
//将虚拟地址与物理地址关联，仅支持一页空间分配,返回(void*)vaddr
void* get_a_page(enum pool_flags pf,uint32_t vaddr){
    struct pool* mem_pool = pf&POOL_FLAG_KERNEL? &kernel_pool : &user_pool;
    lock_acquire(&mem_pool->lock);

    struct task_struct* cur_thread = running_thread();
    int32_t bit_idx_start = -1;

    if(cur_thread->pgdir != NULL && pf == POOL_FLAG_USER){    //用户进程申请内存
        bit_idx_start = (vaddr - cur_thread->userprog_vadder.vaddr_start ) / PG_SIZE; //
        ASSERT(bit_idx_start >= 1);
        bitmap_set(&cur_thread->userprog_vadder.vaddr_bitmap,bit_idx_start,1); //将虚拟地址对应的位图置1
    }else if(cur_thread->pgdir == NULL && pf == POOL_FLAG_KERNEL){ //内核线程申请内存
        bit_idx_start = (vaddr - kernel_vaddr.vaddr_start) / PG_SIZE;
        bitmap_set(&kernel_vaddr.vaddr_bitmap,bit_idx_start,1);
    }else{
        PANIC("get_a_page:wrong!\n");
    }

    void* page_phy = palloc(mem_pool);
    if(page_phy == NULL){
        return NULL;
    }
    page_table_add((void*)vaddr,page_phy);
    lock_release(&mem_pool->lock);
    return (void*)vaddr;
}
//由虚拟地址得到物理地址，返回uint32_t
uint32_t addr_v2p(uint32_t vaddr){
    uint32_t* vaddr_ = get_pte_ptr(vaddr);
    return ((*vaddr_& 0xfffff000) | (0x00000fff & vaddr));
}



void mem_init(void){
    put_str("mem_init start\n");
    uint32_t mem_bytes_total = *((uint32_t*)(0xb00));  //total_mem_bytes dd 0 ; 所在地址 0xB00
    ASSERT(mem_bytes_total==0x2000000); //虚拟机中设置的内存大小是 32G
    mem_pool_init(mem_bytes_total);
    put_str("mem_init done\n");

}