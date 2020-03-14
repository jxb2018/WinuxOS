#ifndef _DEVICE_IOUEUE_H
#define _DEVICE_IQUEUE_H
#include "/home/jxb/OS/lib/kernel/print.h"
#include "/home/jxb/OS/lib/kernel/interrupt.h"
#include "/home/jxb/OS/lib/kernel/init.h"
#include "/home/jxb/OS/lib/kernel/time.h"
#include "/home/jxb/OS/lib/kernel/memory.h"
#include "/home/jxb/OS/lib/kernel/console.h"
#include "/home/jxb/OS/thread/thread.h"
#include "/home/jxb/OS/device/keyboard.h"
#include "/home/jxb/OS/thread/sync.h"
#define buffer_size 64
//环形队列
struct ioqueue{
    struct lock lock;
    struct task_struct* producer; //因缓冲区满而阻塞的生产者线程
    struct task_struct* consumer; //因缓冲区空而阻塞的消费者线程
    char buffer[buffer_size];
    int32_t ridx;   // 读
    int32_t widx;  // 写
};
int ioq_init(struct ioqueue* ioq);
int ioq_empty(struct ioqueue* ioq);
int ioq_full(struct ioqueue* ioq);
void ioq_wait(struct task_struct** waiter);
void wakeup(struct task_struct** waiter);
char ioq_getchar(struct ioqueue* ioq);
void ioq_putchar(struct ioqueue* ioq,char char_byte);
#endif