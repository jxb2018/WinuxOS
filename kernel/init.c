#include "/home/jxb/OS/lib/kernel/print.h"
#include "/home/jxb/OS/lib/kernel/interrupt.h"
#include "/home/jxb/OS/lib/kernel/init.h"
#include "/home/jxb/OS/lib/kernel/time.h"
#include "/home/jxb/OS/lib/kernel/memory.h"
#include "/home/jxb/OS/lib/kernel/console.h"
#include "/home/jxb/OS/thread/thread.h"
#include "/home/jxb/OS/device/keyboard.h"
void init_all(){
    put_str("init_all start\n");
    idt_init();
    time_init();
    mem_init();
    thread_init();
    console_init();
    keyboard_init();
    put_str("init_all done\n");

}