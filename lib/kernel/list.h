#ifndef __LIB_KERNEL_LIST_H
#define __LIB_KERNEL_LIST_H
#include "global.h"
#include "stdint.h"
#define offset(struct_type,member) (int)(&((struct_type*)0)->member)
#define elem2entry(struct_type, struct_member_name, elem_ptr) \
	 (struct_type*)((int)elem_ptr - offset(struct_type, struct_member_name))

//定义链表结点成员结构
typedef struct list_elem {
   struct list_elem* prev; // 前躯结点
   struct list_elem* next; // 后继结点
}list_elem;

//链表结构,用来实现队列
typedef struct list {
   struct list_elem head;// 队首,固定不变的，第1个元素为head.next 
   struct list_elem tail;// 队尾,固定不变的 
}list;

/* 自定义函数类型function,用于在list_traversal中做回调函数 */
typedef int (function)(struct list_elem*, int arg);

void list_init (struct list*);
void list_insert_before(struct list_elem* before, struct list_elem* elem);
void list_push(struct list* plist, struct list_elem* elem);
void list_iterate(struct list* plist);
void list_append(struct list* plist, struct list_elem* elem);  
void list_remove(struct list_elem* pelem);
struct list_elem* list_pop(struct list* plist);
int list_empty(struct list* plist);
uint32_t list_len(struct list* plist);
struct list_elem* list_traversal(list* plist, function func, int arg);
int elem_find(struct list* plist, struct list_elem* obj_elem);
#endif
