//Integrated Drive Electronics 集成硬盘驱动器
#ifndef _DEVICE_IDE_H
#define _DEVICE_IDE_H
#include "/home/jxb/OS/lib/kernel/stdint.h"
#include "/home/jxb/OS/lib/kernel/list.h"
#include "/home/jxb/OS/lib/kernel/bitmap.h"
#include "/home/jxb/OS/thread/sync.h"

/* 
device/ide.h
分区结构：
name        分区名称
start_lba   起始扇区  
sec_cnt     扇区数
my_disk     所属硬盘
part_tag    队列挂载点
sb          超级块
block_bitmap   块(512B)位图 
inode_bitmap  i结点位图  
open_inodes   分区打开的i结点队列                             
*/
struct partition {
   char name[8];		 // 分区名称
   uint32_t start_lba;		 // 起始扇区
   uint32_t sec_cnt;		 // 扇区数
   
   struct disk* my_disk;	 // 分区所属的硬盘
   struct super_block* sb;	 // 本分区的超级块
   struct bitmap block_bitmap;	 // 块位图
   struct bitmap inode_bitmap;	 // i结点位图
   struct list open_inodes;	 // 本分区打开的i结点队列
   list_elem part_tag;	 // 用于队列中的标记

};

/* 硬盘结构 */
struct disk {
   char name[8];			   // 本硬盘的名称，如sda等
   struct ide_channel* my_channel;	   // 此块硬盘归属于哪个ide通道
   uint8_t dev_no;			   // 本硬盘是主0还是从1
   
   struct partition prim_parts[4];	   // 主分区顶多是4个
   struct partition logic_parts[8];	   // 逻辑分区数量无限,但总得有个支持的上限,那就支持8个
};

/* ata通道结构 */
struct ide_channel {
   char name[8];		 // 本ata通道名称 
   uint16_t port_base;		 // 本通道的起始端口号

   uint8_t irq_no;		 // 本通道所用的中断号
   struct lock lock;		 // 通道锁
   bool expecting_intr;		 // 表示等待硬盘的中断
   struct semaphore disk_done;	 // 用于阻塞、唤醒驱动程序
   struct disk devices[2];	 // 一个通道上连接两个硬盘，一主一从
};

//用来存放分区表项 16字节
struct partition_table_entry{
   uint8_t bootable;    //是否可引导


   uint8_t start_head;  //起始磁头号
   uint8_t start_sec;   //起始扇区
   uint8_t start_chs;   //起始柱面号

   uint8_t fs_type;     //分区类型

   
   uint8_t end_head;    //结束磁头号
   uint8_t end_sec;     //结束扇区号
   uint8_t end_chs;     //结束柱面号

   uint32_t start_lba;  //本分区起始扇区的lba
   uint32_t sec_cnt;    //本分区的扇区数目
} __attribute__((packed)); //不允许gcc编译器为对齐而在此结构中填充空隙

//引导扇区，mbr或ebr所在的扇区
struct boot_sector{
   uint8_t other[446]; //引导代码
   struct partition_table_entry partition_table[4]; //分区表
   uint16_t signature;//主引导扇区
}__attribute__((packed));

void select_disk(struct disk* hd);
void select_sector(struct disk* hd,uint32_t lba,uint8_t sec_cnt);
void cmd_out(struct ide_channel* channel,uint8_t cmd);
void read_from_sector(struct disk* hd,void* buf,uint32_t sec_cnt);
void write2sector(struct disk* hd,void* buf,uint32_t sec_cnt);
bool busy_wait(struct disk* hd);
void ide_read(struct disk* hd,uint32_t lba,void* buf,uint32_t sec_cnt);
void ide_write(struct disk* hd,uint32_t lba,void* buf,uint32_t sec_cnt);
void intr_hd_handler(uint8_t irq_no);
void identify_disk(struct disk* hd);
void partition_scan(struct disk* hd,uint32_t ext_lba);
void ide_int(void);


uint8_t channel_cnt;  //按照硬盘数计算的通道数
struct ide_channel channels[2]; //主板上支持4个IDE硬盘、提供两个IDE插槽(通道)
list partition_list; //分区队列;
#endif