#include "/home/jxb/OS/lib/kernel/time.h"
#include "/home/jxb/OS/lib/kernel/io.h"
#include "/home/jxb/OS/lib/kernel/print.h"
#include "/home/jxb/OS/lib/kernel/stdint.h"
#include "/home/jxb/OS/thread/thread.h"
#include "/home/jxb/OS/lib/kernel/debug.h"
#include "/home/jxb/OS/thread/thread.h"
#include "/home/jxb/OS/lib/kernel/interrupt.h"
#define IRQ0_FREQUENCY      100    //时钟中断频率  每秒100次
#define INPUT_FREQUENCY     1193180
#define COUNTER0_VALUE      INPUT_FREQUENCY/IRQ0_FREQUENCY
#define COUNTER0_PORT       0x40   //计数器0 端口号
#define COUNTER0_NO         0      //计数器0
#define COUNTER0_MODE       2     //工作方式2 比率发生器
#define COUNTER0_RW         3     //先读写低字节、再读写高字节
#define PIT_CONTROL_POAT    0x43
#define ms_per_intr         (1000 / IRQ0_FREQUENCY)  //每多少毫秒发生一次时钟中断
uint32_t ticks;//总的

//时钟的中断处理程序
static void intr_time_handle(void){
    struct task_struct* current_thread = running_thread();//获取当前正在运行的线程
    ASSERT(current_thread->stack_magic == 0x19980320);//检查栈是否溢出
    current_thread->elapsed_ticks++;
    ticks++;
    if(current_thread->ticks == 0){
        schedule();
    }else{
        current_thread->ticks--;
    }
}

void time_init(void){
    // 向8253 控制寄存器送控制字 
    outb(PIT_CONTROL_POAT,(uint8_t)((COUNTER0_NO<<6)|(COUNTER0_MODE<<4)|(COUNTER0_MODE<<1)));

    //向计数器0送计数器初值
    //低8位
    outb(COUNTER0_PORT,(uint8_t)COUNTER0_VALUE);
    //高8位
    outb(COUNTER0_PORT,(uint8_t)(COUNTER0_VALUE>>8));
    register_handler(0x20,intr_time_handle);

}

//
void ticks_to_sleep(uint32_t sleep_ticks){
    uint32_t start_tick = ticks;

    while(ticks - start_tick < sleep_ticks){ //在休眠期间，让出CPU的使用权
        thread_yield();
    }
}

void mtime_sleep(uint32_t m_seconds){
    uint32_t sleep_ticks = DIV_ROUND_UP(m_seconds,ms_per_intr);
    ticks_to_sleep(sleep_ticks);
}