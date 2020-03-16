#include "/home/jxb/OS/lib/kernel/print.h"
#include "/home/jxb/OS/lib/kernel/interrupt.h"
#include "/home/jxb/OS/lib/kernel/init.h"
#include "/home/jxb/OS/lib/kernel/time.h"
#include "/home/jxb/OS/lib/kernel/memory.h"
#include "/home/jxb/OS/lib/kernel/console.h"
#include "/home/jxb/OS/thread/thread.h"
#include "/home/jxb/OS/device/keyboard.h"
#include "/home/jxb/OS/thread/sync.h"
#include "/home/jxb/OS/lib/kernel/stdint.h"
#include "/home/jxb/OS/lib/kernel/global.h"
#include "/home/jxb/OS/lib/kernel/stdint.h"
#include "/home/jxb/OS/lib/kernel/string.h"
#include "/home/jxb/OS/lib/kernel/debug.h"
#include "/home/jxb/OS/userprog/tss.h"
#include "/home/jxb/OS/lib/kernel/debug.h"
#include "process.h"
typedef void(*func_)(void);
extern void intr_exit(void); /* kernek\kernel.s   */

//构建进程初始化上下文环境
void start_process(void* filename_){
    
    func_ function = (func_)filename_;//typedef void(*func_)(void);
    struct task_struct* cur_thread = running_thread();
    cur_thread->self_kstart += sizeof(struct thread_stack);
    struct intr_stack* proc_stack = (struct intr_stack*)cur_thread->self_kstart;
    proc_stack->edi = proc_stack->esi = proc_stack->ebp = proc_stack->esp_dummy = 0;
    proc_stack->ebx = proc_stack->edx = proc_stack->ecx = proc_stack->eax       = 0;
    proc_stack->gs  = proc_stack->fs  = proc_stack->es  = proc_stack->ds        = SELECTOR_U_DATA;
    proc_stack->eip = function;//待执行的用户程序地址
    proc_stack->cs  = SELECTOR_U_CODE;
    proc_stack->eflags = EFLAGS_IOPL_0 | EFLAGS_MBS | EFLAGS_IF_1;
    proc_stack->esp = (void*)((uint32_t)get_a_page(POOL_FLAG_USER,USER_STACK3_VADDR) + PG_SIZE);
    proc_stack->ss = SELECTOR_U_DATA;
    
    asm volatile("movl %0,%%esp;jmp intr_exit"::"g"(proc_stack):"memory");
}

// 激活页表
void page_dir_activate(struct task_struct* p_thread){
    uint32_t pagedir_phy_addr = 0x100000;//默认是内核的页目录
    if(p_thread->pgdir != NULL){
        pagedir_phy_addr = addr_v2p((uint32_t)p_thread->pgdir);
    }
  //  put_str("\npagedir:");put_int(pagedir_phy_addr); put_char('\n');
    //更新页目录寄存器cr3
    asm volatile("movl %0,%%cr3"::"r"(pagedir_phy_addr):"memory");
}

//激活线程或进程的页表，更新tss中的esp0为进程的特权级为进程的特权级0的栈
void process_activate(struct task_struct* pthread){
    ASSERT(pthread != NULL);
    page_dir_activate(pthread); //激活该进程的页表
    if(pthread->pgdir){
        update_tss_esp(pthread);
    }
}

//创建页目录表,成功则返回页目录的虚拟地址，否则赶回NULL
uint32_t* create_page_dir(void){ //c0004993
    uint32_t* page_dir_vaddr = get_kernel_page(1);
    if(page_dir_vaddr == NULL)
        return NULL;
    //复制内核页表 0x300 - 0x3ff项
    memcpy((uint32_t*)((uint32_t)page_dir_vaddr + 0xc00),(uint32_t*)(0xfffff000 + 0xc00),0x400);
    //更新页目录表最后一项
    uint32_t new_page_dir_phy_addr = addr_v2p((uint32_t)page_dir_vaddr);
    page_dir_vaddr[1023] = new_page_dir_phy_addr | PG_US_U | PG_RW_W | PG_P_1;
    return page_dir_vaddr;
}


//创建用户进程虚拟地址位图
void create_user_vaddr_bitmap(struct task_struct* user_prog){
    user_prog->userprog_vadder.vaddr_start = USER_VADDR_START;//USER_VADDR_START 0x8048000
    uint32_t bitmap_pg_cnt = DIV_ROUND_UP((0xc0000000 - USER_VADDR_START)/PG_SIZE/8,PG_SIZE);
    user_prog->userprog_vadder.vaddr_bitmap.bits = get_kernel_page(bitmap_pg_cnt);
    user_prog->userprog_vadder.vaddr_bitmap.btmp_bytes_len = bitmap_pg_cnt*PG_SIZE;
    bitmap_init(&user_prog->userprog_vadder.vaddr_bitmap);
}
//创建用户进程
void process_execute(void* filename_,char* name){
    enum intr_status old_intr_status = intr_disable();
    struct task_struct* thread = get_kernel_page(1);//PCB
    init_thread(thread,name,default_prio);  //c0003652
    create_user_vaddr_bitmap(thread); //c0004a0a
    thread_create(thread,start_process,filename_);
    thread->pgdir = create_page_dir();  //c0004993
    
    list_append(&thread_ready_list,&thread->general_tag);
    list_append(&thread_all_list,&thread->all_list_tag);
    
    intr_set_status(old_intr_status);
    
}