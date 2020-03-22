#include "dir.h"
#include "inode.h"
#include "file.h"
#include "fs.h"
#include "super_block.h"
#include "/home/jxb/OS/device/ide.h"
#include "/home/jxb/OS/lib/kernel/stdio_kernel.h"
#include "/home/jxb/OS/lib/kernel/global.h"
#include "/home/jxb/OS/lib/kernel/debug.h"
#include "/home/jxb/OS/lib/kernel/memory.h"
#include "/home/jxb/OS/lib/kernel/string.h"
#include "/home/jxb/OS/lib/kernel/stdint.h"
#include "/home/jxb/OS/lib/kernel/interrupt.h"

struct dir root_dir;             // 根目录

// 打开根目录 
void open_root_dir(struct partition* part) {
   root_dir.inode = inode_open(part, part->sb->root_inode_no);
   root_dir.dir_pos = 0;
}

//在内存中初始化目录项p_de
void create_dir_entry(char* filename,uint32_t inode_no,uint8_t file_types,struct dir_entry* p_de){
   memcpy(p_de->filename,filename,strlen(filename));
   p_de->i_no = inode_no;
   p_de->f_type = file_types;
}
//将目录项p_de写入根目录root_dir中,io_buf由主调函数提供 
bool sync_dir_entry(struct dir_entry* p_de,void* io_buf){
   struct inode* dir_inode = root_dir.inode;
   uint32_t dir_size = dir_inode->i_size;

   uint32_t dir_entry_size = cur_part->sb->dir_entry_size;
   uint32_t dir_entry_per_sec = (SECTOR_SIZE/dir_entry_size);//每扇区最大的目录项数目

   int32_t block_lba = -1;

   uint32_t block_idx = 0;
   uint32_t all_blocks[140] = {0};

   for(block_idx=0;block_idx < 12;block_idx++){
      all_blocks[block_idx] = dir_inode->i_sectors[block_idx];
   }
   struct dir_entry* dir_e = (struct dir_entry*)io_buf; //dir_e用来在io_buf中遍历目录项
   int32_t block_bitmap_idx = -1;

   for(block_idx=0;block_idx<140;block_idx++){
      block_bitmap_idx = -1;
      if(all_blocks[block_idx] == 0){ //在这三种情况下要分配块
         block_lba = block_bitmap_alloc(cur_part);
         if(block_lba == -1){
            printk("alloc block bitmap for sync_dir_entry failed\n");
            return false;
         }
         //每分配一个块就同步一次 block_bitmap
         
         block_bitmap_idx = block_lba - cur_part->sb->data_start_lba;
         bitmap_sync(cur_part,block_bitmap_idx,BLOCK_BITMAP);

         block_bitmap_idx = -1;
         if(block_idx < 12){ //直接块
            dir_inode->i_sectors[block_idx] = all_blocks[block_idx] =  block_lba;
         }else if(block_idx == 12){ //一级间接块

         }else{
            
         }
         memset(io_buf,0,512);
         memcpy(io_buf,p_de,dir_entry_size);
         ide_write(cur_part->my_disk,all_blocks[block_idx],io_buf,1);
         dir_inode->i_size += dir_entry_size;
         return true;
      }
      ide_read(cur_part->my_disk,all_blocks[block_idx],io_buf,1);
      //在扇区中查找空目录项
      uint8_t dir_entry_idx = 0;
      for(dir_entry_idx = 0; dir_entry_idx < dir_entry_per_sec; dir_entry_idx++){
         if(dir_e->f_type == FT_UNKNOWN){
            memcpy(dir_e,p_de,dir_entry_size);
            ide_write(cur_part->my_disk, all_blocks[block_idx], io_buf, 1);
	         dir_inode->i_size += dir_entry_size;
	         return true;
         }
         dir_e++;
      }
      dir_e = (struct dir_entry*)io_buf;
   }
   printk("directory is full\n");
   return false;
} 
