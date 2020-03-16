#include "/home/jxb/OS/lib/kernel/print.h"
#include "/home/jxb/OS/lib/kernel/init.h"
#include "/home/jxb/OS/lib/kernel/debug.h"
#include "/home/jxb/OS/lib/kernel/string.h"
#include "/home/jxb/OS/lib/kernel/memory.h"
#include "/home/jxb/OS/thread/thread.h"
#include "/home/jxb/OS/lib/kernel/interrupt.h"
#include "/home/jxb/OS/lib/kernel/console.h"
#include "/home/jxb/OS/device/ioqueue.h"
#include "/home/jxb/OS/device/keyboard.h"
#include "/home/jxb/OS/userprog/process.h"
#include "/home/jxb/OS/lib/user/syscall.h"
void k1(void* arg);
void u(void);
int prog_a_pid;
int main(void){
    put_str("I am kernel\n");
    init_all();
    //asm volatile("cli"); //开中断
    //ASSERT(1==2);
    //void* addr = get_kernel_page(3);
    //put_int((uint32_t)addr);put_char('\n');
    //addr = get_kernel_page(3);
    //put_int((uint32_t)addr);
    //char* str1 = "";
    //char* str2 = "hello";
    //strcpy(str1,str2);
    //put_str(str1);
    //put_char('\n');
    //struct task_struct* thread_start(char* name,int priority,thread_func function,void* func_arg){
    
   // thread_start("k_thread_a",1,k1,"A ");
    //thread_start("k_thread_a",1,k1,"B  ");
    process_execute(u,"test_u"); //oxc0004a69
    //process_execute(u,"test_2");
     //ASSERT(1==2);
    intr_enable();
    //prog_a_pid = getpid(); //c0004bb8
    while(1){
    console_put_str("\npid = ");
    console_put_int(prog_a_pid);
    }
    while(1);
    return 0;
}
void u(void){
    prog_a_pid = getpid();
    while(1);

}

void k1(void* arg){
    while(1){
       char ch = ioq_getchar(&kdb_buf);
       console_put_char(ch);
    }
}
