#include "/home/jxb/OS/thread/thread.h"
#include "/home/jxb/OS/lib/kernel/stdint.h"
#include "/home/jxb/OS/lib/kernel/string.h"
#include "/home/jxb/OS/lib/kernel/memory.h"
#include "/home/jxb/OS/lib/kernel/global.h"
#include "/home/jxb/OS/lib/kernel/bitmap.h"
#include "/home/jxb/OS/lib/kernel/debug.h"
#include "/home/jxb/OS/lib/kernel/list.h"
#include "/home/jxb/OS/lib/kernel/print.h"
#include "/home/jxb/OS/lib/kernel/interrupt.h"
#include "/home/jxb/OS/userprog/process.h"
#include "/home/jxb/OS/thread/sync.h"
#define PG_SIZE 4096

extern void switch_to(struct task_struct*,struct task_struct*);
struct task_struct* main_thread;//主线程PCB
struct task_struct* idle_thread;// 懒惰  系统空闲时运行的线程
struct lock pid_lock;
static pid_t next_pid = 0;
list thread_ready_list;//
list thread_all_list;//所有任务队列
static list_elem* thread_tag;//用来保存队列中的线程结点

static void kernel_thread(thread_func function,void* func_arg){
    intr_enable();//开中断避免function独享处理器
    function(func_arg);
}
//获取当前线程的pcb指针
struct task_struct* running_thread(){
    uint32_t esp;
    asm volatile("mov %%esp,%0":"=g"(esp));
    return (struct task_struct*)(esp & 0xfffff000);
}


//初始化线程基本信息
void init_thread(struct task_struct* pthread,char* name,int priority);
//初始化线程栈
void thread_create(struct task_struct* pthread,thread_func function,void* func_arg);

//创建线程
struct task_struct* thread_start(char* name,int priority,thread_func function,void* func_arg){
    struct task_struct* thread = get_kernel_page(1); //内核地址池里获取一页地址
    init_thread(thread,name,priority);//初始化PCB
    thread_create(thread,function,func_arg);//初始化线程栈、将待执行的函数和参数放到thread_stack中相应的位置
    ASSERT(!elem_find(&thread_ready_list,&thread->general_tag));
    list_append(&thread_ready_list,&thread->general_tag); //加入就绪队列
    ASSERT(!elem_find(&thread_all_list,&thread->all_list_tag));
    list_append(&thread_all_list,&thread->all_list_tag);//加入全部线程队列
    return thread;
}
//分配pid
static pid_t allocate_pid(void){
    lock_acquire(&pid_lock);
    next_pid++;
    lock_release(&pid_lock);
    return next_pid;
}



//将kernel中的main函数完善为主线程
static void make_main_thread(void){
    main_thread = running_thread();//loader进入时 0xc009f000
    init_thread(main_thread,"main",1);
    list_append(&thread_all_list,&main_thread->general_tag); //将其加入全部线程队列
}


//初始化线程基本信息
void init_thread(struct task_struct* pthread,char* name,int priority){
    memset(pthread,0,sizeof(*pthread));
    strcpy(pthread->name,name);
    pthread->pid = allocate_pid();
    if(pthread == main_thread){
        pthread->status = TASK_RUNNING;
    }else{
        pthread->status = TASK_READY;
    }
    // 预留三个标准的文件描述符 标准输入 标准输出 标准错误
    pthread->fd_table[0] = 0;
    pthread->fd_table[1] = 1;
    pthread->fd_table[2] = 2;
    uint8_t fd_idx = 3;
    while(fd_idx < MAX_FILES_OPEN_PER_PROC ){
        pthread->fd_table[fd_idx] = -1; //表示文件描述符可分配
        fd_idx++;
    }
    pthread->pgdir = NULL;
    pthread->ticks = priority;
    pthread->elapsed_ticks = 0;
    pthread->priority = priority;
    pthread->stack_magic = 0x19980320;
    pthread->self_kstart = (uint32_t*)((uint32_t)pthread + PG_SIZE);//栈顶
}

//初始化线程栈、将要执行的函数和参数放到thread_stack中相应的位置
void thread_create(struct task_struct* pthread,thread_func function,void* func_arg){
    pthread->self_kstart -= sizeof(struct intr_stack); //预留中断栈
    pthread->self_kstart -= sizeof(struct thread_stack);//预留线程栈
    struct thread_stack* kthread_stack = \
                    (struct thread_stack*)pthread->self_kstart;
    kthread_stack->eip = kernel_thread;
    kthread_stack->function = function;
    kthread_stack->func_arg = func_arg;
    kthread_stack->ebp = \
    kthread_stack->ebx = \
    kthread_stack->edi = \
    kthread_stack->esi = 0;
}
//系统空闲时运行的线程
void idle(){
    while(1){
        thread_block(TASK_BLOCKED); 
        asm volatile ("sti; hlt" : : : "memory");
           // hlt指令让处理器停止执行指令
    }  
}



// 实现任务调度
void schedule(void){
    //ASSERT(intr_get_status() == INTR_OFF);
    intr_disable();
    struct task_struct* cur = running_thread();
    if(cur->status == TASK_RUNNING){
        ASSERT(!elem_find(&thread_ready_list,&cur->general_tag));
        cur->ticks = cur->priority;
        cur->status = TASK_READY;
        list_append(&thread_ready_list,&cur->general_tag);
    }else{
        ;
    }
    //ASSERT(!list_empty(&thread_ready_list));
    if(list_empty(&thread_ready_list)){
        thread_unblock(idle_thread); //如果就绪队列没有可以运行的任务，就唤醒idle
                                        //唤醒之后，返回schedule()结束，之后执行thread_block()的最后一条语句
                                        //之后，执行 idle中的 asm,cpu 停机
    }
    thread_tag = list_pop(&thread_ready_list);
    struct task_struct* next = elem2entry(struct task_struct,general_tag,thread_tag);
    next->status = TASK_RUNNING;
    process_activate(next); //激活页表
    switch_to(cur,next);
}

//主动让出cpu，让其他线程运行
void thread_yield(void){
    struct task_struct* cur_thread = running_thread();
    enum intr_status old_intr_status = intr_disable();
    cur_thread->status = TASK_READY;
    ASSERT(!elem_find(&thread_ready_list,&cur_thread->general_tag));
    list_append(&thread_ready_list,&cur_thread->general_tag);
    schedule();
    intr_set_status(old_intr_status);
}

void thread_init(void){
    put_str("thread init start");
    list_init(&thread_ready_list);
    list_init(&thread_all_list);
    lock_init(&pid_lock);
    make_main_thread();
    //创建idle线程
    idle_thread = thread_start("idle",10,idle,NULL);
    put_str("thread_init done\n");
}
//线程阻塞
void thread_block(enum task_status status){
    ASSERT((status == TASK_BLOCKED)||(status == TASK_WAITING)||(status == TASK_HANGING));
    enum intr_status old_intr_status = intr_disable();
    struct task_struct* cur = running_thread();
    cur->status = status;
    schedule();
    intr_set_status(old_intr_status);
}
//解除阻塞
void thread_unblock(struct task_struct* pthread){
    enum intr_status old_intr_status = intr_disable();
    ASSERT((pthread->status == TASK_BLOCKED)||(pthread->status == TASK_WAITING)||(pthread->status == TASK_HANGING));
    ASSERT(!elem_find(&thread_ready_list,&pthread->general_tag));
    pthread->status = TASK_READY;
    list_push(&thread_ready_list,&pthread->general_tag);
    intr_set_status(old_intr_status);
}





