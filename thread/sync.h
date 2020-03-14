#ifndef _THREAD_SYNC_H
#define _THREAD_SYNC_H
#include "/home/jxb/OS/lib/kernel/list.h"

//信号量结构
struct semaphore{
    uint8_t value;
    list waiters;
};

//锁结构
struct lock{
    struct task_struct* holder;//锁的持有者
    struct semaphore semaphore;
    uint32_t holder_repeat_nr;//锁被多少线程申请
};
void lock_init(struct lock* plock);
void sema_init(struct semaphore* psema,uint8_t value);
void sema_down(struct semaphore* psema);
void sema_up(struct semaphore* psema);
void lock_acquire(struct lock* plock);
void lock_release(struct lock* plock);
#endif