#include "list.h"
#include "interrupt.h"

// 初始化双向链表
void list_init(list* plist){
    plist->head.prev = NULL;
    plist->head.next = &plist->tail;
    plist->tail.prev = &plist->head;
    plist->tail.next = NULL;
}
//将链表元素elem插入在元素before之前 原子操作
void list_insert_before(list_elem* before,list_elem* elem){
    enum intr_status old_intr_status = intr_disable();
    elem->prev = before->prev;
    before->prev->next = elem;
    elem->next = before;
    before->prev = elem;
    intr_set_status(old_intr_status);
}
//添加元素到列表对首 push
void list_push(list* plist,list_elem* elem){
    /*
    plist->head.next->prev= elem;
    elem->next = plist->head.next;
    elem->prev = &(plist->head);
    plist->head.next = elem;*/
    list_insert_before(plist->head.next,elem);
}
//追加元素到链表队尾
void list_append(list* plist,list_elem* elem){
    list_insert_before(&plist->tail,elem);
}
//删除
void list_remove(list_elem* elem){
    enum intr_status old_intr_status = intr_disable();
    elem->prev->next = elem->next;
    elem->next->prev = elem->prev;
    intr_set_status(old_intr_status);
}
//pop
list_elem* list_pop(list* plist){
    list_elem* elem = plist->head.next;
    list_remove(elem);
    return elem;
}
//find
int elem_find(list* plist,list_elem* obj_elem){
    list_elem* p = plist->head.next;
    while(p != &plist->tail){
        if(p == obj_elem) return 1;
        p = p->next;
    }
    return 0;
}
int list_empty(list* plist){
    return (plist->head.next == &plist->tail)? 1 : 0;
}
//length
uint32_t list_len(struct list* plist) {
   struct list_elem* elem = plist->head.next;
   uint32_t length = 0;
   while (elem != &plist->tail) {
      length++; 
      elem = elem->next;
   }
   return length;
}
//
list_elem* list_traversal(list* plist,function func,int arg){
    list_elem* elem = plist->head.next;
    if(list_len(plist) == 0) return NULL;
    while(elem !=&plist->tail){
        if(func(elem,arg)) return elem;
        elem = elem->next;
    }
    return NULL;
}
