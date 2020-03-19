#include "stdio.h"
#include "/home/jxb/OS/lib/kernel/debug.h"
#include "/home/jxb/OS/lib/kernel/string.h"
#include "/home/jxb/OS/lib/user/syscall.h"
//将整数转换为字符 integer to ascii
static void itoa(uint32_t value,char** buf_ptr_addr,uint8_t base){
    uint32_t m = value % base; //余数
    uint32_t i = value / base; //整数
    if(i){
        itoa(i,buf_ptr_addr,base);
    }
    if(m < 10){
        *((*buf_ptr_addr)++) = m + '0';
    }else{
        *((*buf_ptr_addr)++) = m - 10 + 'a';
    }
}

//将参数ap按照格式format输出到字符串str，并返回替换后str长度
uint32_t vsprintf(char* str,const char* format,va_list ap){  // char*
    char* buf_ptr = str;
    const char* index_ptr = format;
    char index_char = *index_ptr;
    int32_t arg_int;char* arg_str;
    while(index_char){
        if(index_char != '%'){
            *(buf_ptr++) = index_char;
            index_char = *(++index_ptr);
            continue;
        }
        index_char = *(++index_ptr);
        switch(index_char){
            case 'x':
                arg_int = *((int*)(ap += 4));// format x1 x2 ..
                itoa(arg_int,&buf_ptr,16);
                index_char = *(++index_ptr); //跳过格式符
                break;
            case 's':
                arg_str = *((char**)(ap += 4));
                strcpy(buf_ptr,arg_str);
                buf_ptr += strlen(arg_str);
                index_char = *(++index_ptr);
                break;
            case 'd':
                arg_int = *((int*)(ap += 4));
                if(arg_int < 0){
                    arg_int = -arg_int;
                    *(buf_ptr++) = '-';
                }
                itoa(arg_int,&buf_ptr,10);
                index_char = *(++index_ptr);
                break;
            case 'c':
                arg_str = (char*)(ap += 4);
                *(buf_ptr++) = *arg_str;
                index_char = *(++index_ptr);
                break;
        }
    }
    return strlen(str);
}

//格式化输出字符串 format
uint32_t printf(const char* format,...){
    va_list args = (va_list)&format;  //args 指向 format
    char buf[1024] = {0}; //存储拼接后的字符串
    vsprintf(buf,format,args);
    return write(buf);
}

/* 同printf不同的地方就是字符串不是写到终端,而是写到buf中 */
uint32_t sprintf(char* buf, const char* format, ...) {
   va_list args = (va_list)&format;  //args 指向 format
   uint32_t retval;
   retval = vsprintf(buf, format, args);
   return retval;
}
