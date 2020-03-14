%include "boot/include/boot.inc"
section loader vstart=LOADER_BASE_ADDRESS
LOADER_STACK_TOP equ LOADER_BASE_ADDRESS
;-------------------------------------------
; 进入保护模式需要做的三个准备
; 1. 加载GDT
; 2. 打开A20gate,关闭地址环回
; 3. 将控制寄存器cr0的protected enable位置1
;-------------------------------------------

;-------------------------------------------
; GDT表
;-------------------------------------------
GDT_BASE     :    dd 0x00000000
                  dd 0x00000000
CODE_SESC    :    dd 0x0000FFFF
                  dd DESC_CODE_HIGH4
DATA_SESC    :    dd 0x0000FFFF
                  dd DESC_DATA_HIGH4
VIDEO_SESC   :    dd 0x80000007                ; BFFFF-B8000用于文本显示   limit = (C0000-B8000)/4K - 1 = 7
                  dd DESC_VIDEO_HIGH4
                  
GDT_SIZE      equ  $-GDT_BASE
GDT_LIMIT     equ  GDT_SIZE-1

times 60 dq 0                                  ;dq四个字、八个字节   预留空间
SELECTOR_CODE      equ  (0x0001<<3) + TI_GDT + RPL0
SELECTOR_DATA      equ  (0x0002<<3) + TI_GDT + RPL0
SELECTOR_VIDEO     equ  (0x0003<<3) + TI_GDT + RPL0

;内存总容量
total_mem_bytes dd 0 ; 所在地址 0xB00

;-------------------------------------------
;GDTR寄存器需要的48位数据 GDT的内存地址-GDT界限
;-----------------------------------------
gdtr        dw     GDT_LIMIT ;0xB04
            dd     GDT_BASE  ;0xB06 存放的值是0x900

ards_buffer times 244 db 0 ;0xB0A
ards_nr     dw 0   ;0xBFE
;-------------------------------------------------
;BIOS中断0x15 0xE820 计算内存容量
;-------------------------------------------------            
    mov ebx,0
    mov di,ards_buffer  
    mov edx,'PAMS'
.e820_get_mem_loop:
    mov eax,0xe820
    mov ecx,0x14
    int 0x15
    add di,cx
    inc word [ards_nr]
    cmp ebx,0
    jnz .e820_get_mem_loop                                                
                                    
    mov cx,[ards_nr]
    mov bx,ards_buffer ;0xB0A
    mov edx,0             ;存放最大的内存容量
.find_max_mem_area:
    mov eax,[bx]
    add eax,[bx+8]
    cmp edx,eax
    jge .next_area
    mov edx,eax
.next_area:
    add bx,0x14                                                                                                          
    loop .find_max_mem_area
    jmp .mem_getted                                                   
                                                            
.mem_getted:
    mov [total_mem_bytes],edx                                                                        
                                                                                                
;-----准备进入保护模式------
;--------------------------------------------
;1. 加载 GDT
;--------------------------------------------
    lgdt [gdtr]

;--------------------------------------------
;2. 打开A20gate,关闭地址环回
;---------------------------------------------
    in al,0x92
    or al,0000_0010b ;将其位置1
    out 0x92,al

;--------------------------------------------
;3. 将控制寄存器cr0的protected enable位置1
;---------------------------------------------
    mov eax,cr0
    or eax,0x1
    mov cr0,eax

;---------------------------------------------
;这一跳的目的：清空流水线、更新段描述符缓冲寄存器(32位CPU中的实模式与16位CPU中的实模式略有不同，用缓冲寄存器的低20位来存储 段基址<<4 )
;dword 表示4字节大小的数据 
;---------------------------------------------
    jmp  dword SELECTOR_CODE:p_mode_start


[bits 32]
p_mode_start:
    mov ax,SELECTOR_DATA    ;0x00000c6e
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov esp,LOADER_STACK_TOP
    mov ax,SELECTOR_VIDEO
    mov gs,ax
    
;------------------------------------------------------------------------------------
;加载kernel
;-------------------------------------------------------------------------------------
    mov eax,KERNEL_START_SECTION
    mov ebx,KERNEL_BIN_BASE_ADDRESS
    mov ecx,200
    call read_disk_m32


;启动分页机制
;----------------------------------------------
;1. 创建页表
;--------------------------------------------
    call build_page_table ;0x00000c97
    
    
    mov ebx,[gdtr+2] ;存放的是段描述符表的基址
    add dword [ebx+24+4],0xc0000000 ;修改显存段描述符
    add dword    [ebx+2],0xc0000000 ;修改段表的基址
    
    add              esp,0xc0000000 ;将栈指针进行映射
    
;--------------------------------------------------
;2.将页表地址写入cr3
;--------------------------------------------------
    mov eax,PAGE_DIR_TABLE_POS
    mov cr3,eax
    
;----------------------------------------------------
;3.打开cr0的pg位
;----------------------------------------------------
    mov eax,cr0
    or eax,0x80000000
    mov cr0,eax
; 恭喜开启分页机制!!!
    ;重新加载gdt
    lgdt [gdtr]

    jmp SELECTOR_CODE:enter_kernel ;刷新流水线 ;0x0008:00000cd7

enter_kernel:
    call kernel_init
    mov esp,0xc009f000 ;虚拟地址 0x00000cdc
    jmp KERNEL_ENTRY_POINT

    
    mov byte [gs:160],'v'   ;屏幕上每个宇符的低宇节是字符的ASCII 码，
    mov byte [gs:161],0xA4 ; 高字节是字符属性元信息。

    jmp $
    
;------------------------------------------------------
;创建页目录及页表
;------------------------------------------------------------
build_page_table:
    ;1M之上4k空间清零
    mov edi,0
    mov ecx,1024
.clear_page_dir:
    mov dword [PAGE_DIR_TABLE_POS+edi],0x0
    add di,0x4
    loop .clear_page_dir
    
    ;开始创建页目录项(PDE)
    xor eax,eax
    mov eax,PAGE_DIR_TABLE_POS
    add eax,0x1000
    mov edx,eax
    or eax,(PG_US_U | PG_RW_W | PG_P)
    mov [PAGE_DIR_TABLE_POS+  0x0],eax
    mov [PAGE_DIR_TABLE_POS+0xc00],eax  ;将虚拟地址中的0号和768号 4M大小的块 映射到相同的页表
    
    mov eax,PAGE_DIR_TABLE_POS
    or eax,(PG_US_U + PG_RW_W + PG_P)
    mov [PAGE_DIR_TABLE_POS+0xffc],eax  ;将虚拟地址中最后一个4M大小的块 映射到目录表自己所在的位置
    
    ;创建 0x10100处页表的表项   
    mov ecx,256
    mov ebx,(PG_US_U | PG_RW_W | PG_P)   
    mov esi,0
.create_pte:
    mov [edx+esi],ebx
    add esi,4
    add ebx,0x1000
    loop .create_pte
    
    ;将虚拟地址中的(3G-4M)～(4G-4M)映射到PDE的769～1022表项
    mov eax,PAGE_DIR_TABLE_POS
    add eax,0x2000
    or eax,(PG_US_U | PG_RW_W | PG_P)
    mov edi,0
    mov ecx,254
.create_kernal_pde:
    mov [PAGE_DIR_TABLE_POS + 0xc04 + edi],eax
    add edi,4
    add eax,0x1000
    loop .create_kernal_pde 
    ret   

;--------------------------------------------------------------
 ; 32位保护模式下，从硬盘中读取文件
 ;------------------------------------------------------------
read_disk_m32:
        push CX
        push EAX
;第一步 ： 设置要读取的扇区数    （CL） --->    0x1f2 （sector count register）
        mov DX,0x1F2
        mov AL,CL
        out DX,AL
        POP EAX

;第二步： 向通道上的三个LBA寄存器写入扇区起始地址(LBA方式)的低24位
        mov dx,0x1F3  
        out  dx,al    ; lba low register

        mov cl,0x8
        shr eax,cl
        mov dx,0x1F4
        out dx,al       ;LBA mid register
        
        shr eax,cl
        mov dx,0x1F5
        out dx,al        ;LBA high register
        
;第三步： 向device寄存器中写入LBA地址的高四位，并置第6位为1，使其为LBA模式，设置第4位，选择操作的硬盘(master硬盘或者slave硬盘)
        shr eax,cl
        and al,0x0F
        or al,0xe0
        mov dx,0x1F6
        out dx,al
        
;第四步: 向该通道上的command寄存器写入操作命令
        mov al,0x20
        mov dx,0x1F7
        out dx,al
        
;第五步： 读取通道上的status寄存器，判断硬盘工作是否完成
    not_ready:
        in al,dx
        and al,0x88   ;第7位为1表示硬盘忙，第4位为1表示硬盘控制器已经做好数据传输
        cmp al,0x08
        jnz not_ready

;第六步： 硬盘准备继续，开始读数据
        pop ax ;要读取的扇区数
        mov dx,256
        mul dx
        mov cx,ax   ; 要传送的字节
        mov dx,0x1F0
  go_on_read:
        in ax,dx
        mov [ebx],ax
        add ebx,2
        loop go_on_read
        ret
;------------------------------------------------------------------------------------------------
;将kernel_bin中的segment拷贝到指定的编译地址
;--------------------------------------------------------------------------------------------- 
kernel_init:
    cld  ;向高地址生长
    push eax
    push ebx
    push ecx
    push edx
    xor ecx,ecx;---
    xor edx,edx;----
    mov ebx,[KERNEL_BIN_BASE_ADDRESS + 28];progeram header table file offset
    mov dx ,[KERNEL_BIN_BASE_ADDRESS + 42];program header table entry size
    mov cx ,[KERNEL_BIN_BASE_ADDRESS + 44];program header table entry count
    
    add ebx,KERNEL_BIN_BASE_ADDRESS ;program header table address
    
.each_segment:
    push ecx
    cmp dword [ebx+0x0],PT_NULL
    je .PTNULL
    mov  edi, [ebx + 8]                   ;dst
    mov  eax, [ebx + 4]
    add  eax, KERNEL_BIN_BASE_ADDRESS
    mov  esi,  eax                        ;src   
    mov  ecx, [ebx + 16]                  ;size
    rep movsb
    pop ecx
.PTNULL:
    add ebx,edx ;指向下一个 program segment header entry     
    loop .each_segment
    
    pop edx
    pop ecx
    pop ebx
    pop eax
    ret
