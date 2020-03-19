#include "ide.h"
#include "/home/jxb/OS/lib/kernel/print.h"
#include "/home/jxb/OS/lib/kernel/init.h"
#include "/home/jxb/OS/lib/kernel/debug.h"
#include "/home/jxb/OS/lib/kernel/string.h"
#include "/home/jxb/OS/lib/kernel/memory.h"
#include "/home/jxb/OS/thread/thread.h"
#include "/home/jxb/OS/lib/kernel/interrupt.h"
#include "/home/jxb/OS/lib/kernel/console.h"
#include "/home/jxb/OS/device/ioqueue.h"
#include "/home/jxb/OS/device/keyboard.h"
#include "/home/jxb/OS/userprog/process.h"
#include "/home/jxb/OS/lib/user/syscall.h"
#include "/home/jxb/OS/lib/stdio.h"
#include "/home/jxb/OS/lib/kernel/stdio_kernel.h"
#include "/home/jxb/OS/lib/kernel/io.h"
#define UNUSED __attribute__ ((unused))
// 定义硬盘各寄存器的端口号
#define reg_data(channel)	 (channel->port_base + 0)
#define reg_error(channel)	 (channel->port_base + 1)
#define reg_sect_cnt(channel)	 (channel->port_base + 2)
#define reg_lba_l(channel)	 (channel->port_base + 3)
#define reg_lba_m(channel)	 (channel->port_base + 4)
#define reg_lba_h(channel)	 (channel->port_base + 5)
#define reg_dev(channel)	 (channel->port_base + 6)
#define reg_status(channel)	 (channel->port_base + 7)
#define reg_cmd(channel)	 (reg_status(channel))
#define reg_alt_status(channel)  (channel->port_base + 0x206)
#define reg_ctl(channel)	 reg_alt_status(channel)

// reg_alt_status寄存器的一些关键位 
#define BIT_STAT_BSY	 0x80	      // 硬盘忙
#define BIT_STAT_DRDY	 0x40	      // 驱动器准备好	 
#define BIT_STAT_DRQ	 0x8	      // 数据传输准备好了

// device寄存器的一些关键位 
#define BIT_DEV_MBS	0xa0	    // 第7位和第5位固定为1
#define BIT_DEV_LBA	0x40
#define BIT_DEV_DEV	0x10

// 一些硬盘操作的指令 
#define CMD_IDENTIFY	   0xec	    // identify指令
#define CMD_READ_SECTOR	   0x20     // 读扇区指令
#define CMD_WRITE_SECTOR   0x30	    // 写扇区指令


uint8_t channel_cnt;  //按照硬盘数计算的通道数
struct ide_channel channels[2]; //主板上支持4个IDE硬盘、提供两个IDE插槽(通道)

//用来记录总拓展分区的起始lba
uint32_t ext_lba_base = 0;

uint8_t p_no = 0,l_no = 0; // 用来记录硬盘主分区和逻辑分区的下标

list partition_list; //分区队列

/* 选择读写的硬盘
device寄存器：
    7 | 6 | 5 | 4 | 3 | 2 | 1 | 0
    |   |   |   |  |   |   |   |
    |   |   |   |  ——  ——  ——  —— LBA地址的第27：23位                       
    |   |   |   —— DEV 主盘或从盘
    |   |   —— 固定为1
    |   —— MOD LBA为1，CHS为0
    —— 固定为1  

*/
void select_disk(struct disk* hd){
    uint8_t reg_device = BIT_DEV_MBS | BIT_DEV_LBA;
    if(hd->dev_no == 1){  //dev_no=1表示从盘
        reg_device |= BIT_DEV_DEV;
    }
    outb(reg_dev(hd->my_channel),reg_device);
}

//向硬盘控制器写入起始扇区地址及要读写的扇区数
void select_sector(struct disk* hd,uint32_t lba,uint8_t sec_cnt){
    struct ide_channel* channel = hd->my_channel;
    outb(reg_sect_cnt(channel),sec_cnt); //指定待读取或写入的扇区数

    //low
    outb(reg_lba_l(channel), lba);
    //mid
    outb(reg_lba_m(channel),(lba>> 8));
    //high
    outb(reg_lba_h(channel),(lba>>16));
    //device
    outb(reg_dev(channel)  ,(lba>>24) | BIT_DEV_MBS | BIT_DEV_LBA | (hd->dev_no == 1 ? BIT_DEV_DEV : 0) );
}

//向通道channel发送命令 cmd
void cmd_out(struct ide_channel* channel,uint8_t cmd){
    channel->expecting_intr = true;  //表明向硬盘发送过命令
    outb(reg_cmd(channel),cmd);
}

//在已经设置好起始扇区地址和要读扇区数的基础上，从硬盘读入sec_cnt个扇区到buf
static void read_from_sector(struct disk* hd,void* buf,uint32_t sec_cnt){
    uint32_t size_byte = (sec_cnt == 0)? 256 * 512 : sec_cnt * 512; //sec_cnt=0，表示读写256扇区
    insw(reg_data(hd->my_channel),buf,size_byte/2);
}

//在已经设置好起始扇区地址和要写扇区数的基础上，将buf中sec_cnt扇区的数据写入硬盘
void write2sector(struct disk* hd,void* buf,uint32_t sec_cnt){
    uint32_t size_byte = (sec_cnt == 0)? 256 * 512 : sec_cnt * 512; //sec_cnt=0，表示读写256扇区
    outsw(reg_data(hd->my_channel),buf,size_byte/2);
}

//硬盘准备就绪,不忙时，取出DRQ的值
bool busy_wait(struct disk* hd){
    uint32_t time_limit = 30 * 1000; //最多可等待30s
    while(time_limit -= 10 ){
        if(!(inb(reg_status(hd->my_channel)) & BIT_STAT_BSY)){
            return (inb(reg_status(hd->my_channel)) & BIT_STAT_DRQ);
        }else{
            mtime_sleep(10);
        }
    }
    return false;
}

//从硬盘中读取sec_cnt 个扇区到buf
void ide_read(struct disk* hd,uint32_t lba,void* buf,uint32_t sec_cnt){
    // 1. 加锁 获取通道的使用权
    lock_acquire(&hd->my_channel->lock);
    select_sector(hd,lba,sec_cnt);

    //2. 选择操作的磁盘
    select_disk(hd);

    uint32_t sec_cnt_per_op;    //每次要读取的扇区数
    uint32_t sec_cnt_done = 0; //已经读取的扇区数
    while(sec_cnt_done < sec_cnt){
        (sec_cnt - sec_cnt_done) >= 256 ? (sec_cnt_per_op = 256) : (sec_cnt_per_op = sec_cnt - sec_cnt_done);
        //3. 扇区数 + 要读的扇区数
        select_sector(hd,lba + sec_cnt_done,sec_cnt_per_op);

        //4.发送读命令
        cmd_out(hd->my_channel,CMD_READ_SECTOR);
        //5.因为硬盘速度慢，所以，发送完读写命令后，要等待一段时间。选择将自己阻塞，让出cpu；等磁盘读写完成，通过中断程序唤醒自己
        sema_down(&hd->my_channel->disk_done);

        //------下面代码，醒来继续执行------
        //6.检测硬盘是否准备就绪
        if(!busy_wait(hd)){
            char error[64];
            sprintf(error,"%s read sector %d faild!\n",hd->name,lba+sec_cnt_done);
            PANIC(error);
        }

        //7.将数据从硬盘的缓冲区中读出
        read_from_sector(hd,(void*)((uint32_t)buf + sec_cnt_done * 512),sec_cnt_per_op);
        sec_cnt_done += sec_cnt_per_op;
    }
    //8.释放锁
    lock_release(&hd->my_channel->lock);
}

//将buf中sec_cnt扇区数据写入硬盘
void ide_write(struct disk* hd,uint32_t lba,void* buf,uint32_t sec_cnt){
    //1. 加锁
    lock_acquire(&hd->my_channel->lock);
    //2. 选择主盘还是从盘
    select_disk(hd);
    
    uint32_t sec_cnt_per_op;
    uint32_t sec_cnt_done = 0;
    while(sec_cnt_done < sec_cnt){
        ((sec_cnt - sec_cnt_done) >= 256)?( sec_cnt_per_op = 256):(sec_cnt_per_op = sec_cnt - sec_cnt_done);
        //3. 写lba 和sec_cnt
        select_sector(hd,lba + sec_cnt_done,sec_cnt_per_op);
        //4. cmd
        cmd_out(hd->my_channel,CMD_WRITE_SECTOR);       
        //5. 是否就绪
        if(!busy_wait(hd)){
            char error[64];
            sprintf(error,"%s write sector %d faild!\n",hd->name,lba+sec_cnt_done);
            PANIC(error);
        }
        //6. 写
        write2sector(hd,buf,sec_cnt_per_op);

        //7. 自我阻塞
        sema_down(&hd->my_channel->disk_done);
        sec_cnt_done += sec_cnt_per_op;
    }
    //8. 释放锁
    lock_release(&hd->my_channel->lock);
}
// 硬盘中断处理程序
void intr_hd_handler(uint8_t irq_no){
    uint8_t channel_no = irq_no - 0x2e;
    struct ide_channel* channel = &channels[channel_no]; //IDE通道
    if(channel->expecting_intr){
        channel->expecting_intr = false;
        sema_up(&channel->disk_done);
        inb(reg_status(channel)); //中断处理完成后，需要显式通知硬盘控制器此次中断已经处理完成，否则硬盘便不会产生新的中断
    }
}


//获得硬盘参数信息
void identify_disk(struct disk* hd){
    char id_info[512];
    //1.加锁
    lock_acquire(&hd->my_channel->lock);
    //2. 选择硬盘
    select_disk(hd);
    //3.发送命令
    cmd_out(hd->my_channel,CMD_IDENTIFY);
    //4.阻塞自己
    sema_down(&hd->my_channel->disk_done);
    //5.被唤醒，检测硬盘状态
    if(!busy_wait(hd)){
        char error[64];
        sprintf(error,"%s identify failed!\n",hd->name);
        PANIC(error);
    }
    //6.就绪 读入id_info
    read_from_sector(hd,id_info,1);

    char buf[64];
    uint32_t sn_start = 10 * 2, sn_len = 20; //硬盘序列号，长度为20字节
    uint32_t md_start = 27 * 2, md_len = 40; //硬盘型号
    swap_pairs_bytes(&id_info[sn_start],buf,sn_len);
    printk("disk %s info:\n     SN: %s\n",hd->name,buf);
    memset(buf,0,sn_len);

    swap_pairs_bytes(&id_info[md_start],buf,md_len);
    printk("     MODULE: %s\n",buf);

    uint32_t sectors = *((uint32_t *)&id_info[60 * 2]);
    printk("     SECTORS: %d\n",sectors);

    printk("     CAPACITY: %dMB\n",sectors*512/1024/1024);
}

//扫描硬盘hd中地址为ext_lba的扇区中的所有分区
void partition_scan(struct disk* hd,uint32_t ext_lba){
    struct boot_sector* bs = sys_malloc(sizeof(struct boot_sector));
    ide_read(hd,ext_lba,bs,1); //读入mbr或者ebr
   // printk("%d\n",bs->partition_table[0].fs_type);
    
    //ASSERT(0);
    uint8_t part_idx = 0;
    struct partition_table_entry* pentry = bs->partition_table;
    while(part_idx++ < 4){ //一个分布表只有4个表项，每个表项16字节
        if(pentry->fs_type == 0x5){ //拓展分区
            if(ext_lba_base != 0){ //子拓展分区
                partition_scan(hd,ext_lba_base + pentry->start_lba);            
            }else{//主拓展分区
                ext_lba_base = pentry->start_lba; //更换基址
                partition_scan(hd,pentry->start_lba);
            }
        }else if(pentry->fs_type != 0){//有效分区类型
            if(ext_lba == 0){ //主分区
                sprintf(hd->prim_parts[p_no].name,"%s%d",hd->name,p_no+1);
                hd->prim_parts[p_no].start_lba = 0 + pentry->start_lba;
                hd->prim_parts[p_no].sec_cnt   = pentry->sec_cnt;
                hd->prim_parts[p_no].my_disk   = hd;
                list_append(&partition_list,&hd->prim_parts[p_no].part_tag);
                p_no++;
            }else{
                sprintf(hd->prim_parts[l_no].name,"%s%d",hd->name,l_no+1);
                hd->prim_parts[l_no].start_lba = ext_lba + pentry->start_lba;
                hd->prim_parts[l_no].sec_cnt   = pentry->sec_cnt;
                hd->prim_parts[l_no].my_disk   = hd;
                list_append(&partition_list,&hd->prim_parts[l_no].part_tag);
                l_no++;
                ASSERT(l_no <= 8);//防止数组越界
            }
        }
        pentry++;  
    }
    sys_free(bs);
}
//打印分区
void partition_info(list_elem* pelem,int arg UNUSED){
    struct partition* part = elem2entry(struct partition,part_tag,pelem);
    printk("    %s start_lba:0x%x,sec_cnt:0x%x\n",part->name,part->start_lba,part->sec_cnt);
    return;
}

//硬盘数据初始化
void ide_int(){
    printk("ide_init start\n");
    uint8_t hd_cnt = *(uint8_t *)(0x475); //获取硬盘的数量
    channel_cnt = DIV_ROUND_UP(hd_cnt,2); //一个IDE通道上挂两块硬盘

    struct ide_channel* channel;
    uint8_t channel_no = 0; //0是起始编号
    uint8_t dev_no ;
    list_init(&partition_list);
    while(channel_no < channel_cnt){
        channel = &channels[channel_no];
        //1. 初始化name
        sprintf(channel->name,"ide%d",channel_no); //写入缓冲区

        //2. 为每个ide通道初始化端口基址 port_base 及中断向量号 irq_no
        switch(channel_no){
            case 0:
                channel->port_base = 0x1f0;
                channel->irq_no = 0x20 + 14; //
                break;
            case 1:
                channel->port_base = 0x170;
                channel->irq_no = 0x20 + 15;
                break;
        }
        //3. 设置默认情况下，不与硬盘打交道
        channel->expecting_intr = false; //未向硬盘写入指令时不期待硬盘的中断
        //4. 初始化lock
        lock_init(&channel->lock);
        //5. 初始化disk_done
        sema_init(&channel->disk_done,0);
                //初始化为0，目的是向硬盘控制器请求数据后，阻塞该线程。直到硬盘完成后，发出中断，在中断处理程序中
                //sema_up信号量，唤醒线程
        //6. 注册对应的中断处理函数
        register_handler(channel->irq_no,intr_hd_handler); /////// 0x2e

        //7. 初始化其上挂载的disk
        dev_no = 0;
        while(dev_no < 2){ //处理通道上挂载的主、从盘
            struct disk* hd = &channel->devices[dev_no];
            sprintf(hd->name,"hd%c",'a'  + dev_no + channel_no*2);
            hd->my_channel = channel;
            hd->dev_no = dev_no; 
            identify_disk(hd);
            if(dev_no != 0){ //内核所在的裸盘不处理
                partition_scan(hd,0);
            }
            p_no = 0;l_no = 0;//上一块硬盘处理之后，p_no,l_no清零
            dev_no++;  //开始处理下一块硬盘
        }
        channel_no++; //开始处理下一个通道     
    }
    printk("\nall partition info\n");
    //printk("%d\n",list_len(&partition_list));
    list_traversal(&partition_list,partition_info,(int)NULL);

    printk("ide_init done\n");
}




