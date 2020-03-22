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
#include "/home/jxb/OS/lib/user/syscall.h"
#include "/home/jxb/OS/userprog/syscall_init.h"
#include "/home/jxb/OS/fs/fs.h"

//返回当前任务的pid
uint32_t sys_getpid(void){
    return running_thread()->pid;
}

//初始化系统调用
void syscall_init(void){
    put_str("syscall_init start\n");
    syscall_table[SYS_GETPID] = sys_getpid;
    syscall_table[SYS_WRITE]  = sys_write;
    syscall_table[SYS_MALLOC]  = sys_malloc;
    syscall_table[SYS_FREE]  = sys_free;
    put_str("syscall init done\n");
}
