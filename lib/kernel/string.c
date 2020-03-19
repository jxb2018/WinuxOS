#include "string.h"
#include "global.h"
#include "debug.h"
#include "stdint.h"

//将src中len对相邻字节交换位置，存入buf中
void swap_pairs_bytes(const char* src,char* buf,uint32_t len){
    uint32_t idx ;
    for(idx = 0; idx <len ;idx += 2){
        buf[idx + 1] = *(src++);
        buf[idx]     = *(src++);
    }
    buf[idx] = '\0';
}
// 将dst_起始的size个字节置为value
void memset(void* dst_,uint8_t value,uint32_t size){
    ASSERT(dst_ != NULL);
    uint8_t* dst = (uint8_t*)dst_;
    while(size--){
        *(dst++) = value;
    }
}
//将src_起始的size个字节复制到dst_
void memcpy(void* dst_,const void* src_,uint32_t size){
    ASSERT((dst_ != NULL) && (src_ != NULL));
    uint8_t* dst = (uint8_t *)dst_;
    uint8_t* src = (uint8_t *)src_;
    while(size--){
        *(dst++) = *(src++);
    }
}

//连续比较以地址a_和地址d_开头的size个字节,若相等则返回0  a_大于b_返回+1，否则返回-1
int memcmp(const void* a_,const void* b_,uint32_t size){//3
    ASSERT((a_ != NULL)&&(b_ !=NULL));
    const char* a = a_;
    const char* b = b_;
    while(size--){
        if(*a != *b){
            return (*a > *b) ? 1 : -1;
        }
        a++;
        b++;
    }
    return 0;
}

//将字符串从src_复制到dst_
char* strcpy(char* dst_,const char* src_){
    ASSERT((dst_ != NULL)&&(src_ !=NULL));
    char* r = dst_;
    while(*(dst_++) = *(src_++));
    return r;
}
//返回字符串长度
uint32_t strlen(const char* str){//5
    ASSERT(str != NULL);
    int len = -1;
    while(*(str+(++len)));
    return len;
}
//比较两个字符串
int strcmp(const char* a,const char* b){
    ASSERT((a != NULL)&&(b != NULL));
    while(*a||*b){
        if(*a != *b){
            return (*a > *b)?1:-1;
        }
        a++;b++;
    }
    return 0;
}
//从左到右查找字符串 str 中首次出现字符 ch 的地址
char* strchr(const char* str,const uint8_t ch){//7
    ASSERT(str != NULL);
    while(*str){
        if(*str == ch) return (char*)str;
        str++;
    }
    return NULL;
}
//从后往前查找字符串 str 中首次出现字符 ch 的地址
char* strrchr(const char* str,const uint8_t ch){
    ASSERT(str != NULL);
    int len = strlen(str);
    while(len--){
        if(*(str+len)== ch) return (char*)(str+len);
    }
    return NULL;
}
//将字符串 src 拼接到 dst＿后，返回拼续的串地址
char* strcat(char* dst_,const char* src_){//9
    ASSERT(dst_!=NULL && src_!=NULL);
    char *r = dst_;
    while(*(dst_++));
    dst_--;
    while(*(dst_++)=*(src_++));
    return r;
}
//在字符串 str 中查找字符 ch 出现的次数
uint32_t strchrs(const char* str,uint8_t ch){
    ASSERT(str!=NULL);
    uint32_t cnt = 0;
    while(*str){
        if(*str == ch){
            cnt++;
        } 
        str++;
    }
    return cnt;
}