#ifndef __USERPROG_PROCESS_H 
#define __USERPROG_PROCESS_H 
#include "/home/jxb/OS/thread/thread.h"
#include "/home/jxb/OS/lib/kernel/stdint.h"
#define default_prio 31
#define USER_STACK3_VADDR  (0xc0000000 - 0x1000)  //4k
#define USER_VADDR_START 0x8048000
void process_execute(void* filename, char* name);
void start_process(void* filename_);
void process_activate(struct task_struct* p_thread);
void page_dir_activate(struct task_struct* p_thread);
uint32_t* create_page_dir(void);
void create_user_vaddr_bitmap(struct task_struct* user_prog);
#endif
