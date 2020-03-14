#ifndef _LIB_INTERRUPT_H
#define _LIB_INTERRUPT_H
typedef void* intr_handler; // 空指针类型，修饰idt
void idt_init(void);
//定义中断的两种状态
enum intr_status{
    INTR_OFF,
    INTR_ON
};
void register_handler(uint8_t vector_no,intr_handler function);
enum intr_status intr_get_status(void);
enum intr_status intr_enable(void);
enum intr_status intr_disable(void);
enum intr_status intr_set_status(enum intr_status status);
#endif