#include "sync.h"
#include "/home/jxb/OS/lib/kernel/interrupt.h"
#include "thread.h"
#include "/home/jxb/OS/lib/kernel/debug.h"
//初始化信号量
void sema_init(struct semaphore* psema,uint8_t value){
    psema->value = value;
    list_init(&psema->waiters);//初始化信号量的等待队列
}

//初始化锁
void lock_init(struct lock* plock){
    plock->holder = NULL;
    plock->holder_repeat_nr = 0;
    sema_init(&plock->semaphore,1);
}

//信号量 down操作
/*
(1) 判断信号量是否大于0
(2 ）若信号量大于0，则将信号量减1
(3 ）若信号量等于0，当前线程将自己阻塞，以在此信号 上等待。
*/
void sema_down(struct semaphore* psema){
    enum intr_status old_intr_status = intr_disable();
    while(psema->value == 0){
        list_append(&psema->waiters,&running_thread()->general_tag);
        thread_block(TASK_BLOCKED);
    }
    psema->value--;
    ASSERT(psema->value == 0);
    intr_set_status(old_intr_status);
}


//信号量 up操作
/*
( 1) 将信号 的值加 lo
(2 ）唤醒在此信号量上等待的线程
*/
void sema_up(struct semaphore* psema){
    enum intr_status old_intr_status = intr_disable();
    ASSERT(psema->value == 0);
    if(!list_empty(&psema->waiters)){
        struct task_struct* thread_blocked = elem2entry(struct task_struct,general_tag,list_pop(&psema->waiters));
        thread_unblock(thread_blocked);
    }
    psema->value++;
    ASSERT(psema->value == 1);
    intr_set_status(old_intr_status);    
}

//获取锁
void lock_acquire(struct lock* plock){
    if(plock->holder != running_thread()){ //排除曾经自己已经持有锁但还未将其释放的情况
        sema_down(&plock->semaphore);
        plock->holder = running_thread();
        plock->holder_repeat_nr = 1;
    }else{
        plock->holder_repeat_nr++;
    }
}

//释放锁
void lock_release(struct lock* plock){
    ASSERT(plock->holder == running_thread());
    if(plock->holder_repeat_nr > 1){
        plock->holder_repeat_nr--;
        return;
    }
    ASSERT(plock->holder_repeat_nr == 1);
    plock->holder = NULL;
    plock->holder_repeat_nr--; //0
    sema_up(&plock->semaphore);
}