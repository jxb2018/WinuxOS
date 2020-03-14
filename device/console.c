#include "/home/jxb/OS/lib/kernel/console.h"
#include "/home/jxb/OS/lib/kernel/print.h"
#include "/home/jxb/OS/thread/sync.h"
#include "/home/jxb/OS/lib/kernel/print.h"
#include "/home/jxb/OS/lib/kernel/stdint.h"
#include "/home/jxb/OS/lib/kernel/debug.h"
static struct lock console_lock;//控制台锁

//初始化终端
void console_init(){
    lock_init(&console_lock);
}
//获取终端
void console_acquire(){
    lock_acquire(&console_lock);
}
//释放终端
void console_release(){
    lock_release(&console_lock);
}

//终端输出字符串
void console_put_str(char* str){
    console_acquire();
    put_str(str);
    console_release();
}
/*
void console_put_str(char* str);
void console_put_char(uint32_t num);
void console_put_int(uint8_t ch_asci);
*/
//终端输出字符
void console_put_char(uint8_t ch_asci){
    console_acquire();
    put_char(ch_asci);
    console_release();
}

//终端输出整数
void console_put_int(uint32_t num){
    console_acquire();
    put_int(num);
    console_release();
}
