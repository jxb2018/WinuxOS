#ifndef __FS_DIR_H
#define __FS_DIR_H
#include "/home/jxb/OS/lib/kernel/stdint.h"
#include "inode.h"
#include "fs.h"
#include "/home/jxb/OS/device/ide.h"
#include "/home/jxb/OS/lib/kernel/global.h"

#define MAX_FILE_NAME_LEN  16	 // 最大文件名长度

/* 目录结构 不在磁盘中存储，只用于与目录相关的操作，在内存创建的结构
inode    |   dir_pos    dir_buf[512]
            uint32_t      uint8_t
*/
struct dir {
   struct inode* inode;   
   uint32_t dir_pos;	  // 记录在目录内的偏移
   uint8_t dir_buf[512];  // 目录的数据缓存
};

/* 目录项结构 

filename |   i_no      | ftype
文件名称    inode编号       文件类型

*/
struct dir_entry {
   char filename[MAX_FILE_NAME_LEN];  // 普通文件或目录名称
   uint32_t i_no;		      // 普通文件或目录对应的inode编号
   enum file_types f_type;	      // 文件类型
};

extern struct dir root_dir;             // 根目录
void open_root_dir(struct partition* part);
void create_dir_entry(char* filename,uint32_t inode_no,uint8_t file_types,struct dir_entry* p_de);
bool sync_dir_entry(struct dir_entry* p_de,void* io_buf);
#endif
