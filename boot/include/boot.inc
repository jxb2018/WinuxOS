;----------------------------------------------------
;loader所在的扇区号以及装入内存的地址
;----------------------------------------------------
LOADER_START_SECTION    equ     0x2                               ;loader所在扇区的起始地址
LOADER_BASE_ADDRESS     equ     0x900                             ;loader将要被装入的内存地址
PAGE_DIR_TABLE_POS      equ     0x100000                          ;页目录表要被装入的内存地址
;-----------------------------------------------------
;kernel
;------------------------------------------------------
KERNEL_START_SECTION equ 0x9
KERNEL_BIN_BASE_ADDRESS equ 0x70000
KERNEL_ENTRY_POINT equ 0xc0001500
PT_NULL equ 0

;-----------------------------------------------------
;GDT描述符的属性
;-----------------------------------------------------
DESC_G_4K               equ	  1_00000000000000000000000b   
DESC_DB_32              equ	   1_0000000000000000000000b
DESC_L                  equ	    0_000000000000000000000b	;  64位代码标记，此处标记为0便可。
DESC_AVL                equ	     0_00000000000000000000b	;  cpu不用此位，暂置为0  
DESC_LIMIT_CODE2        equ            1111_0000000000000000b
DESC_LIMIT_DATA2        equ                  DESC_LIMIT_CODE2
DESC_LIMIT_VIDEO2       equ              0000_000000000000000b
DESC_P                  equ		  1_000000000000000b
DESC_DPL_0              equ		   00_0000000000000b
DESC_DPL_1              equ		   01_0000000000000b
DESC_DPL_2              equ		   10_0000000000000b
DESC_DPL_3              equ		   11_0000000000000b
DESC_S_CODE             equ		     1_000000000000b
DESC_S_DATA             equ	                 DESC_S_CODE
DESC_S_sys              equ		     0_000000000000b
DESC_TYPE_CODE          equ	              1000_00000000b	;只执行代码段 
DESC_TYPE_DATA          equ	              0010_00000000b	;可读写数据段
;代码段高4个字节
DESC_CODE_HIGH4         equ     (0x00 << 24) + DESC_G_4K + DESC_DB_32 + DESC_L + DESC_AVL + DESC_LIMIT_CODE2 + DESC_P + DESC_DPL_0 + DESC_S_CODE + DESC_TYPE_CODE + 0x00
;数据段高4个字节
DESC_DATA_HIGH4         equ     (0x00 << 24) + DESC_G_4K + DESC_DB_32 + DESC_L + DESC_AVL + DESC_LIMIT_DATA2 + DESC_P + DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x00
;显存段高4个字节
DESC_VIDEO_HIGH4         equ     (0x00 << 24) + DESC_G_4K + DESC_DB_32 + DESC_L + DESC_AVL + DESC_LIMIT_VIDEO2+ DESC_P + DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x0B
;-------------------------------------------------------
;选择子属性
;-------------------------------------------------------
RPL0                    equ     00b
RPL1                    equ     01b
RPL2                    equ     10b
RPL3                    equ     11b
TI_GDT                  equ     000b
TI_LDT                  equ     100b

;--------------------------------------------------------
;页表属性
;---------------------------------------------------------
PG_P                    equ       1b     ;页存在内存中
PG_RW_R                 equ      00b     ;只可读
PG_RW_W                 equ      10b     ;既可读也可写
PG_US_S                 equ     000b     ;超级用户,level3不可访问
PG_US_U                 equ     100b     ;普通用户，均可访问
