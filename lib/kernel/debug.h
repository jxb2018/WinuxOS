#ifndef _LIB_KERNEL_DEBUG_H
#define _LIB_KERNEL_DEBUG_H
void panic_spin(char* filename,int line,const char* func,const char* condition);
/*
    _VA_ARGS_  是预处理器所支持的专用标识符
    ... 参数可变
*/
#define PANIC(...) panic_spin(__FILE__, __LINE__, __func__, __VA_ARGS__)

#ifdef NDEBUG
    #define ASSERT(CONDITION) ((void)0)
#else
    #define ASSERT(CONDITION) if(CONDITION){} else{PANIC(#CONDITION);}    //符号#让编译器将宏的参数转化为字符串常量
#endif

#endif