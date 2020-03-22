#ifndef _FS_FILE_H
#define _FS_FILE_H
#include "/home/jxb/OS/lib/kernel/stdint.h"
#include "/home/jxb/OS/device/ide.h"
#include "dir.h"
#include "/home/jxb/OS/lib/kernel/global.h"

/* 文件结构 */
struct file {
   uint32_t fd_pos;      // 记录当前文件操作的偏移地址,以0为起始,最大为文件大小-1
   uint32_t fd_flag;
   struct inode* fd_inode;
};

/* 标准输入输出描述符 */
enum std_fd {
   stdin_no,   // 0 标准输入
   stdout_no,  // 1 标准输出
   stderr_no   // 2 标准错误
};

/* 位图类型 */
enum bitmap_type {
   INODE_BITMAP,     // inode位图
   BLOCK_BITMAP	     // 块位图
};

#define MAX_FILE_OPEN 32    // 系统可打开的最大文件数

extern struct file file_table[MAX_FILE_OPEN];
int32_t inode_bitmap_alloc(struct partition* part);
int32_t block_bitmap_alloc(struct partition* part);
void bitmap_sync(struct partition* part, uint32_t bit_idx, enum bitmap_type btmp_type);
int32_t get_free_slot_in_global(void);
int32_t pcb_fd_install(int32_t globa_fd_idx);
int32_t file_open(uint32_t inode_no, uint8_t flag);
int32_t file_close(struct file* file);
int32_t file_write(struct file* file, const void* buf, uint32_t count);
int32_t file_read(struct file* file, void* buf, uint32_t count);
#endif
