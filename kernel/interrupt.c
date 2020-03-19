/*
将中断处理程序安装到中断描述符表中
*/
#include "/home/jxb/OS/lib/kernel/stdint.h"
#include "/home/jxb/OS/lib/kernel/global.h"
#include "/home/jxb/OS/lib/kernel/interrupt.h"
#include "/home/jxb/OS/lib/kernel/print.h"
#include "/home/jxb/OS/lib/kernel/io.h"
#define IDT_DESC_CNT  0x81   //支持中断的数量
#define PIC_M_ICW_1_OCW_23   0x20     //主片的初始化命令寄存器
#define PIC_M_ICW_234_OCW_1 0x21   
#define PIC_S_ICW_1_OCW_23   0xA0    //从片的初始化命令寄存器
#define PIC_S_ICW_234_OCW_1 0xA1

//断言
#define EFLAGS_IF   0x00000200    //eflags寄存器中的if位为1
#define GET_EFLAGS(EFLAG_VAR) asm volatile("pushf; popl %0" : "=g"(EFLAG_VAR))

extern void set_cursor(uint32_t );
extern uint32_t syscall_handler(void);//系统调用处理函数
// 中断门描述符结构体
typedef struct gate_desc{
    uint16_t func_offset_low_word;
    uint16_t selector;
    uint8_t dcount;
    uint8_t attribute;
    uint16_t func_offset_high_word;
}GATE_DESC;

//中断描述符表 每个表项8个字节
static GATE_DESC idt[IDT_DESC_CNT];
// 保存异常名字的数组  intr_name
char *intr_name[IDT_DESC_CNT];
///中断处理程序数组 intrXXentry  只是中断处理程序的入口地址
intr_handler idt_table[IDT_DESC_CNT];
// 引用kernel.s中的 intr_entry_table 里面存放的是中断向量对应中断处理程序的入口地址4个字节
extern intr_handler intr_entry_table[IDT_DESC_CNT];

static void idt_desc_init(void);
static void pic_init(void);
static void make_idt_desc(GATE_DESC *p_gdesc,uint8_t attr,intr_handler fun);
//通用的中断处理函数
static void general_intr_handler(uint8_t vec_nr);
//完成一般中断处理函数注册及异常名称注册
static void exception_init(void);


// 完成有关中断的所有初始化工作
void idt_init(void){
    /*
    1. 初始化中断描述符表  idt_desc_init()
    2. 初始化8259A            pic_init()
    3. 加载idt  ->  idtr
    */
    put_str("idt_init start\n");
    
    idt_desc_init();//初始化中断描述符表
    pic_init();     //初始化8259A
    exception_init();//注册异常名称和异常处理函数
    uint64_t idt_operand = ((sizeof(idt) - 1) | ((uint64_t)(uint32_t)idt << 16));
    asm volatile("lidt %0" : : "m" (idt_operand));
    put_str("idt_init done\n");
}
static void pic_init(void){
    //初始化主片
    outb(PIC_M_ICW_1_OCW_23,0x11); // 级联、边沿触发
    outb(PIC_M_ICW_234_OCW_1,0x20); //起始中断向量号是 0x20 
    outb(PIC_M_ICW_234_OCW_1,0x04); //主片的IR2接从片
    outb(PIC_M_ICW_234_OCW_1,0x01);//8086模式，手动结束中断
    //初始化从片
    outb(PIC_S_ICW_1_OCW_23,0x11); // 级联、边沿触发
    outb(PIC_S_ICW_234_OCW_1,0x28); //起始中断向量号是 0x28 
    outb(PIC_S_ICW_234_OCW_1,0x02); //指定主片连接从片的IRQ接口
    outb(PIC_S_ICW_234_OCW_1,0x01);//8086模式，手动结束中断


    /*
    //OCW1  仅打开时钟中断IRQ0
    outb(PIC_M_ICW_234_OCW_1,0xfe);
    outb(PIC_S_ICW_234_OCW_1,0xff);
    */

    outb(PIC_M_ICW_234_OCW_1,0xf8); //主片打开 时钟、键盘中断
    outb(PIC_S_ICW_234_OCW_1,0x3f);//屏蔽从片上的所有中断，所以值为 Oxff
    put_str("pic_init done\n");


}
static void idt_desc_init(void){
    int i;
    for(i=0;i<IDT_DESC_CNT - 1;i++){
        make_idt_desc(&idt[i],IDT_DESC_ATTR_DPL0,intr_entry_table[i]);
    }
    put_str("idt_desc_init done\n");
    //单独处理系统调用，系统调用对应的中断门的dpl是3
    make_idt_desc(&idt[0x80],IDT_DESC_ATTR_DPL3,syscall_handler);
}
//填写中断描述符
static void make_idt_desc(GATE_DESC *p_gdesc,uint8_t attr,intr_handler fun){
    p_gdesc->func_offset_low_word = ((uint32_t)fun & 0x0000ffff);
    p_gdesc->selector = SELECTOR_K_CODE;
    p_gdesc->dcount = 0;
    p_gdesc->attribute = attr;
    p_gdesc->func_offset_high_word = (((uint32_t)fun & 0xffff0000)>>16);
}
static void general_intr_handler(uint8_t vec_nr){
    if(vec_nr == 0x27 || vec_nr == 0x2f){ //伪中断，无需处理
        return;
    }
    //在屏幕左上角清理
    set_cursor(0);
    int cursor_pos = 0;
    while(cursor_pos <320){
        put_char(' ');
        cursor_pos++;
    }
    set_cursor(0);
    put_str("---------exception message begin---------\n");
    set_cursor(88);
    put_str(intr_name[vec_nr]);
    if(vec_nr == 14){//缺页异常，打印缺失的地址
        int page_fault = 0;
        asm volatile("mov %%cr2,%0":"=r"(page_fault));
                //CR2是页故障线性地址寄存器，保存最后一次出现页故障的全32位线性地址
        put_str("\n page_fault address is ");put_int(page_fault);put_char('\n');
    }
    put_str("\n---------exception message end---------\n");

    while(1);//不会被中断，因为进入中断处理程序，说明中断已经关闭
}
static void exception_init(void){
   int i;
   for(i=0;i<IDT_DESC_CNT  ;i++){
       intr_name[i] = "unknown";
       idt_table[i] = general_intr_handler;
   }
   intr_name[0] = "#DE Divide Error";
   intr_name[1] = "#DB Debug Exception";
   intr_name[2] = "NMI Interrupt";
   intr_name[3] = "#BP Breakpoint Exception";
   intr_name[4] = "#OF Overflow Exception";
   intr_name[5] = "#BR BOUND Range Exceeded Exception";
   intr_name[6] = "#UD Invalid Opcode Exception";
   intr_name[7] = "#NM Device Not Available Exception";
   intr_name[8] = "#DF Double Fault Exception";
   intr_name[9] = "Coprocessor Segment Overrun";
   intr_name[10] = "#TS Invalid TSS Exception";
   intr_name[11] = "#NP Segment Not Present";
   intr_name[12] = "#SS Stack Fault Exception";
   intr_name[13] = "#GP General Protection Exception";
   intr_name[14] = "#PF Page-Fault Exception";
   // intr_name[15] 第15项是intel保留项，未使用
   intr_name[16] = "#MF x87 FPU Floating-Point Error";
   intr_name[17] = "#AC Alignment Check Exception";
   intr_name[18] = "#MC Machine-Check Exception";
   intr_name[19] = "#XF SIMD Floating-Point Exception";
   
}
//获取当前中断状态
enum intr_status intr_get_status(){
    uint32_t eflags = 0;
    GET_EFLAGS(eflags);
    return (eflags & EFLAGS_IF) ? INTR_ON : INTR_OFF;
}

//开中断并返回中断前的状态
enum intr_status intr_enable(){
    enum intr_status old_status = intr_get_status();
    if(intr_get_status() == INTR_ON){
        return INTR_ON;
    }else{
        asm volatile("sti");
        return INTR_OFF;
    }    
    return old_status;
}

//关中断并返回中断前的状态
enum intr_status intr_disable(){
    enum intr_status old_status = intr_get_status();
    if(intr_get_status() == INTR_ON){
        asm volatile("cli":::"memory");
        return INTR_ON;
    }else{
        return INTR_OFF;
    }  
    return old_status;

}
//register_handler(0x20,intr_time_handle);
//typedef void* intr_handler; 空指针类型，修饰idt
void register_handler(uint8_t vector_no,intr_handler function){
    idt_table[vector_no] = function;
}
//将中断状态设置为status
enum intr_status intr_set_status(enum intr_status status){
    return (status & INTR_ON )?intr_enable():intr_disable();
}











