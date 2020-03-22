#include "debug.h"
#include "print.h"
#include "interrupt.h"
#include "stdio_kernel.h"
void panic_spin(char* filename,int line,const char* func,const char* condition){
    intr_disable();//关中断
    put_str("\nerror\n");
    put_str("filename:");
    put_str(filename);
    put_str("\n");

    /*
    put_str("line:0x");
    put_int(line);
    put_str("\n");
    */
    printk("line : %d\n",line);

    put_str("function:");
    put_str(func);
    put_str("\n");

    put_str("condition:");
    put_str(condition);
    put_str("\n");
    while(1);
}