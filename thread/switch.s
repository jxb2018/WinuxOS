[bits 32]
section .text 
global switch_to
switch_to:
    ;next                |
    ;cur                 |
    ;shedule的返回地址     V
    push esi
    push edi 
    push ebx 
    push ebp
    mov eax,[esp+20] ;cur的self_kstart地址
    mov [eax],esp ;self_kstart

    ;-------------------备份当前线程，恢复下个线程------------------
    mov eax,[esp + 24];next
    mov esp,[eax] ;task_struct    uint32_t* self_kstart  线程的内核栈
    pop ebp
    pop ebx
    pop edi 
    pop esi
    ret 
