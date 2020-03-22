#ifndef _THEAD_THREAD_H
#define _THEAD_THREAD_H
#include "stdint.h"
#include "/home/jxb/OS/lib/kernel/list.h"
#include "/home/jxb/OS/lib/kernel/memory.h"
//定义通用函数模型
typedef void thread_func(void*);   //为拥有参数void*,返回值是void的函数起了一个别名 thread_func
#define MAX_FILES_OPEN_PER_PROC 8 //每个任务可打开的文件数
/* 进程或线程的状态 */
enum task_status {
   TASK_RUNNING,
   TASK_READY,
   TASK_BLOCKED,
   TASK_WAITING,
   TASK_HANGING,
   TASK_DIED
};

//中断栈结构     intr_stack
struct intr_stack {
    uint32_t vec_no;	 // kernel.S 宏VECTOR中push %1压入的中断号
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp_dummy;	 // 虽然pushad把esp也压入,但esp是不断变化的,所以会被popad忽略
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;
//------以下由cpu从低特权级进入高特权级时压入-------//
    uint32_t err_code;		 // err_code会被压入在eip之后
    void (*eip) (void);   //函数指针，eip中存放的是一个参数类型是void，返回值类型是void函数的地址
    uint32_t cs;
    uint32_t eflags;
    void* esp;
    uint32_t ss;
};

//线程栈结构
struct thread_stack {
   uint32_t ebp;
   uint32_t ebx;
   uint32_t edi;
   uint32_t esi;
   void (*eip) (thread_func* func, void* func_arg);/* 线程第一次执行时,eip指向待调用的函数kernel_thread,其它时候,eip是指向switch_to的返回地址*/
//-------以下仅供第一次被调度上cpu时使用------------//
   void (*unused_retaddr);  // 参数unused_ret只为占位置充数为返回地址
   thread_func* function;   // 由Kernel_thread所调用的函数名
   void* func_arg;          // 由Kernel_thread所调用的函数所需的参数
};

//线程或进程的PCB，程序控制块
struct task_struct{
   uint32_t* self_kstart;  //个内核线程都用自己的内核栈
   pid_t pid;
   enum task_status status;
   uint8_t priority;   //线程优先级
   char name[16];
   uint8_t ticks;//每次在处理器上执行时间的滴答数
   uint32_t elapsed_ticks;//从运行开始，占用了多少个cpu滴答数
   int32_t fd_table[MAX_FILES_OPEN_PER_PROC]; //文件描述符数组
   list_elem general_tag;
   list_elem all_list_tag;
   uint32_t* pgdir;//进程自己页表的虚拟地址
   struct virtual_addr userprog_vadder;//用户进程的虚拟地址池
   struct mem_block_desc u_block_descs[DESC_CNT]; //用户进程内存块描述符
   uint32_t cwd_inode_nr;	 // 进程所在的工作目录的inode编号
   uint32_t stack_magic;//栈的边界标记，用于检测栈的溢出
};
/*
struct virtual_addr{
    struct bitmap vaddr_bitmap; //虚拟地址用到的位图结构
    uint32_t vaddr_start;//虚拟地址起始地址 ？以这个地址为起始进行分配
};
*/
extern list thread_ready_list;//
extern list thread_all_list;//所有任务队列
struct task_struct* thread_start(char* name,int priority,thread_func function,void* func_arg);
void schedule(void);
struct task_struct* running_thread(void);
void thread_block(enum task_status status);
void init_thread(struct task_struct* pthread,char* name,int priority);
void thread_create(struct task_struct* pthread,thread_func function,void* func_arg);
void thread_unblock(struct task_struct* pthread);
void thread_init(void);
void thread_yield(void);
#endif