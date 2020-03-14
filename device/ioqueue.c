#include "/home/jxb/OS/lib/kernel/print.h"
#include "/home/jxb/OS/lib/kernel/interrupt.h"
#include "/home/jxb/OS/lib/kernel/time.h"
#include "/home/jxb/OS/lib/kernel/memory.h"
#include "/home/jxb/OS/lib/kernel/console.h"
#include "/home/jxb/OS/thread/thread.h"
#include "/home/jxb/OS/device/keyboard.h"
#include "/home/jxb/OS/thread/sync.h"
#include "/home/jxb/OS/lib/kernel/debug.h"
#include "ioqueue.h"

int ioq_init(struct ioqueue* ioq){
    lock_init(&ioq->lock);
    ioq->consumer = ioq->producer = NULL;
    ioq->ridx = ioq->widx = 0;
}
int ioq_empty(struct ioqueue* ioq){
    return (ioq->widx == ioq->ridx)?1:0;
}
int ioq_full(struct ioqueue* ioq){
    return ((ioq->widx + 1)%buffer_size == ioq->ridx) ? 1:0;
}
void ioq_wait(struct task_struct** waiter){
    ASSERT(*waiter == NULL);
    *waiter = running_thread();
    thread_block(TASK_BLOCKED);
}
void wakeup(struct task_struct** waiter){
    thread_unblock(*waiter);
    *waiter = NULL;
}
//消费者
char ioq_getchar(struct ioqueue* ioq){
    if(ioq_empty(ioq)) 
        ioq_wait(&ioq->consumer);
    lock_acquire(&ioq->lock);
    char char_byte = ioq->buffer[ioq->ridx];
    ioq->ridx = (ioq->ridx + 1) % buffer_size;
    if(ioq->producer != NULL) 
        wakeup(&ioq->producer); 
    lock_release(&ioq->lock);
    return char_byte;
}
//生产者
void ioq_putchar(struct ioqueue* ioq,char char_byte){
    if(ioq_full(ioq))
        ioq_wait(&ioq->consumer);
    lock_acquire(&ioq->lock);
    ioq->buffer[ioq->widx] = char_byte;
    ioq->widx = (ioq->widx + 1) % buffer_size;
    if(ioq->consumer != NULL)
        wakeup(&ioq->consumer);
    lock_release(&ioq->lock);
}