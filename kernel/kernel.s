[bits 32]
%define ERROR_CODE nop
%define ZERO       push 0x0

extern put_str ;声明外部函数
extern idt_table ;interrupt.c注册的中断处理程序数组
section .data
intr_str db "interrupt occur!",0xa,0
global intr_entry_table
global intr_exit
intr_entry_table:

%macro VECTOR 2    
section .text
intr%1entry: 
    %2
    ;保存上下文环境
    push ds
    push es
    push fs
    push gs
    pushad
    
    ;从从片进入的中断，除了向从片发送EOI,还要向主片发送EOI
    mov al,0x20 ;中断结束命令
    out 0xa0,al
    out 0x20,al
    
    push %1   ;压入中断向量号 给interrupt.c中的general_intr_handler使用
              ;static void general_intr_handler(uint8_t vec_nr)
    call [idt_table + %1*4] ;调用idt_table中的中断处理程序
    jmp intr_exit
section .data
    dd intr%1entry   ;存储各个中断程序的地址
%endmacro
section .text
intr_exit:
    add esp,4 ;跳过中断号
    popad
    pop gs
    pop fs
    pop es
    pop ds
    add esp,4;跳过error_code
    iretd  ;以32位位返回地址

VECTOR 0x00,ZERO
VECTOR 0x01,ZERO
VECTOR 0x02,ZERO
VECTOR 0x03,ZERO 
VECTOR 0x04,ZERO
VECTOR 0x05,ZERO
VECTOR 0x06,ZERO
VECTOR 0x07,ZERO 
VECTOR 0x08,ERROR_CODE
VECTOR 0x09,ZERO
VECTOR 0x0a,ERROR_CODE
VECTOR 0x0b,ERROR_CODE 
VECTOR 0x0c,ZERO
VECTOR 0x0d,ERROR_CODE
VECTOR 0x0e,ERROR_CODE
VECTOR 0x0f,ZERO 
VECTOR 0x10,ZERO
VECTOR 0x11,ERROR_CODE
VECTOR 0x12,ZERO
VECTOR 0x13,ZERO 
VECTOR 0x14,ZERO
VECTOR 0x15,ZERO
VECTOR 0x16,ZERO
VECTOR 0x17,ZERO 
VECTOR 0x18,ERROR_CODE
VECTOR 0x19,ZERO
VECTOR 0x1a,ERROR_CODE
VECTOR 0x1b,ERROR_CODE 
VECTOR 0x1c,ZERO
VECTOR 0x1d,ERROR_CODE
VECTOR 0x1e,ERROR_CODE
VECTOR 0x1f,ZERO 
VECTOR 0x20,ZERO	;时钟中断对应的入口
VECTOR 0x21,ZERO	;键盘中断对应的入口
VECTOR 0x22,ZERO	;级联用的
VECTOR 0x23,ZERO	;串口2对应的入口
VECTOR 0x24,ZERO	;串口1对应的入口
VECTOR 0x25,ZERO	;并口2对应的入口
VECTOR 0x26,ZERO	;软盘对应的入口
VECTOR 0x27,ZERO	;并口1对应的入口
VECTOR 0x28,ZERO	;实时时钟对应的入口
VECTOR 0x29,ZERO	;重定向
VECTOR 0x2a,ZERO	;保留
VECTOR 0x2b,ZERO	;保留
VECTOR 0x2c,ZERO	;ps/2鼠标
VECTOR 0x2d,ZERO	;fpu浮点单元异常
VECTOR 0x2e,ZERO	;硬盘
VECTOR 0x2f,ZERO	;保留



[bits 32]
extern syscall_table
global syscall_handler
section .text 


syscall_handler:
    push 0   ; 使栈中格式一致

    push ds
    push es 
    push fs 
    push gs 
    pushad 

    push 0x80 ;压入中断向量号 

    ;为系统调用子功能传入参数
    push edx ;系统调用中第三个参数
    push ecx ;2
    push ebx ;1

    call [syscall_table + eax*4] ;
    add esp,12
    mov [esp + 8*4],eax ;将函数的返回值写入eax中
    jmp intr_exit