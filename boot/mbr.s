%include "boot/include/boot.inc"
SECTION MBR vstart=0x7c00 ;告诉编译器从 7c00开始编址

    ; 初始化寄存器
        mov AX,CS
        mov DS,AX
        mov ES,AX
        mov SS,AX
        mov FS,AX
        mov SP,0x7c00
        mov AX,0xB800 ;用于文本模式显示适配器
        mov GS,AX

        call cls
        
        mov EAX,LOADER_START_SECTION        ; loader所在的扇区号
        mov BX,LOADER_BASE_ADDRESS             ;loader被载入的内存地址
        mov CX,4                              ;要装入的扇区数
        call read_disk_m16
                                                                                                                  
        jmp LOADER_BASE_ADDRESS + 0x300 ;跳转执行int 15
;---------------------------------------------------------------
;读硬盘
;-----------------------------------------------------------------
read_disk_m16:
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
        mov [bx],ax
        add bx,2
        loop go_on_read
        ret


cls:
        mov AX,0x600 ; 清屏
        mov bx,0x700   ;空白行属性赋值为黑底灰白字
        mov cx,0x0000  ;（CL，CH）= 左上角   (x,y)
        mov dx,0x184f;（CL，CH）=  右下角（x,y)    VGA模式下，一屏可容纳25行，一行可容纳80个字符
        INT 0x10
        ret 

        times 510-($-$$) db 0

        db 0x55,0xaa



