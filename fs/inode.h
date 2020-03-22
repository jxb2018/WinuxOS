#ifndef __FS_INODE_H
#define __FS_INODE_H
#include "/home/jxb/OS/lib/kernel/list.h"
#include "/home/jxb/OS/device/ide.h"
#include "/home/jxb/OS/lib/kernel/stdint.h"
/* inode结构 ：
   i_no | i_size | i_open_cnts | write_deny | i_sectors | inode_tag
   编号     大小     打开次数        写互斥        存储块      队列挂载点
*/
struct inode {
   uint32_t i_no;    // inode编号
   uint32_t i_size; //当此inode是文件时,i_size是指文件大小,若此inode是目录,i_size是指该目录下所有目录项大小之和
   uint32_t i_open_cnts;   // 记录此文件被打开的次数
   bool write_deny;	   // 写文件不能并行,进程写文件前检查此标识
   uint32_t i_sectors[13]; //i_sectors[0-11]是直接块, i_sectors[12]用来存储一级间接块指针
   list_elem inode_tag; //用来插入队列
};

struct inode* inode_open(struct partition* part, uint32_t inode_no);
void inode_sync(struct partition* part, struct inode* inode, void* io_buf);
void inode_init(uint32_t inode_no, struct inode* new_inode);
void inode_close(struct inode* inode);
void inode_release(struct partition* part, uint32_t inode_no);
void inode_delete(struct partition* part, uint32_t inode_no, void* io_buf);
#endif
