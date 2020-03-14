%include "/home/jxb/OS/boot/include/boot.inc"
SELECTOR_VIDEO equ (0x0003<<3) + TI_GDT + RPL0



[bits 32]
section .data
put_int_buffer dq 0 ;定义8个字节的缓冲池

section .text
global put_char
global put_str
global put_int
global set_cursor
;-----------------------------------------------
;put_int 打印数字 
;将小端字序的数字变成对应的ascii   15 -> f
;----------------------------------------------
put_int:
    pushad
    mov ebp,esp
    mov eax,[ebp+36] ;?(要打印的数字) + 4(返回地址) + 32(寄存器)
    mov edx,eax
    mov edi,7
    mov ecx,8  ;4*8
    mov ebx,put_int_buffer
    
    ;32位数字 从底到高 每四位作为一个单元集进行处理
.16base_4bits:
    and edx,0x0000000f
    cmp edx,9
    jle .store
    add edx,7
        
.store:
    add edx,'0'
    mov [ebx+edi],dl
    dec edi
    shr eax,4
    mov edx,eax                                                
    loop .16base_4bits                                                                                
    
    mov cx,8
    mov edi,0
    mov ah,0 ;flag
.put_each_num:
    mov al,[ebx+edi]
    cmp ah,0
    jnz .continue
    cmp al,'0'
    jz .next_num
.continue:
    add ah,1
    push eax
    call put_char
    add esp,4
.next_num:    
    inc edi
    loop .put_each_num
    cmp ah,0
    jnz .put_int_end
    mov al,'0'
    push eax 
    call put_char
    add esp,4
.put_int_end:            
    popad
    ret

;----------------------------------------------
; put_str 
; 功能:打印字符串
;-----------------------------------------------
put_str:
    push ecx
    push ebx
    xor ecx,ecx
    ;ebx 放的是字符串的起始地址
    mov ebx,[esp+12] ; ?(message) + 4(main) +  4(ecx) +4(ebx)
.continue:
    mov cl,[ebx]
    cmp cl,0
    jz .str_end
    push ecx
    call put_char
    inc ebx
    add esp,4
    jmp .continue

.str_end:
    pop ebx
    pop ecx
    ret 

;----------------------------------------------
; put_char 将栈中的一个字(2个字节)打印到光标所在处
; 实现： 处理流程
;1. 备份寄存器
;2. 获取光标坐标值(要打印字符的位置)
;3. 获取待打印的字符
;4. 判断是否是控制字符 回车、换行、退格 并执行相应的处理流程 
;5. 判断是否需要滚屏
;6. 更新光标值
;7. 恢复寄存器现场，exit
;----------------------------------------------

put_char:
    pushad
    
    ;已经进入了保护模式，所以要设置选择子
    mov ax,SELECTOR_VIDEO
    mov gs,ax
    
    ;获取光标坐标值 高8位
    mov dx,0x3d4
    mov al,0x0e
    out dx,al
    mov dx,0x3d5
    in al,dx
    mov ah,al
    
    ;获取光标坐标值 低8位
    mov dx,0x3d4
    mov al,0x0f
    out dx,al
    mov dx,0x3d5
    in al,dx
    
    ;将光标的坐标值 给bx
    mov bx,ax
    
    ;获取待打印的字符
    mov eax,[esp+36] ; 1(待打印的字符) + 4(调用者返回地址) + 32(8个寄存器)esp
    
    ;进行判断
    cmp al,0x0d;回车
    jz .is_carriage_return
    cmp al,0x0a;
    jz .is_line_feed
    cmp al,0x08
    jz .is_backspace
    
    jmp .put_other
    
.is_backspace:
     dec bx
     shl bx,1
     mov al,0
     mov ah,0x07
     mov [gs:bx],ax
     shr bx,1
     jmp .set_cursor

.put_other:
     mov ah,0x07 ;字符属性
     shl bx,1
     mov [gs:bx],ax
     shr bx,1
     inc bx
     cmp bx,2000
     jl .set_cursor

.is_line_feed:
.is_carriage_return:
     xor dx,dx
     mov ax,bx
     mov si,80
     div si
     sub bx,dx ; 回到行首
     add bx,80 ; 到下一行行首
     cmp bx,2000
     jl .set_cursor
     
     ;进行滚屏   实现的原理是将 1-24行搬到0-23行  24行填充0
     cld ;向上生长
     mov ecx,960; 2000-80 = 1920 1920*2/4 = 960
     mov esi,0xc00b80a0; 第一行的地址
     mov edi,0xc00b8000;
     rep movsd
     
     mov bx,1920
     shl bx,1
     mov ecx,80
     mov ax,0x0700;
.cls:     
     mov [gs:bx],ax
     add bx,2
     loop .cls
     mov bx,1920;
        
.set_cursor:
    ;将光标设置成bx的值
    ;设置光标坐标值 高8位
    mov dx,0x3d4
    mov al,0x0e
    out dx,al
    mov dx,0x3d5
    mov al,bh
    out dx,al
    
    ;设置光标坐标值 低8位
    mov dx,0x3d4
    mov al,0x0f
    out dx,al
    mov dx,0x3d5
    mov al,bl
    out dx,al

.put_char_done:
    popad
    ret
    
set_cursor:
    push edx
    push eax
    push ebx
    mov ebx,[esp+16];? + 4(返回地址) + 4*3   
    ;设置光标坐标值 低8位
    mov dx,0x3d4
    mov al,0x0f
    out dx,al
    mov dx,0x3d5
    mov al,bl
    out dx,al

    shr ebx,8
    mov dx,0x3d4
    mov al,0x0e
    out dx,al
    mov dx,0x3d5
    mov al,bh
    out dx,al
    pop ebx
    pop eax 
    pop edx 
    ret        
    