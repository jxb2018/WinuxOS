#include "super_block.h"
#include "inode.h"
#include "dir.h"
#include "fs.h"
#include "file.h"
#include "/home/jxb/OS/device/ide.h"
#include "/home/jxb/OS/lib/kernel/stdint.h"
#include "/home/jxb/OS/lib/kernel/global.h"
#include "/home/jxb/OS/lib/kernel/list.h"
#include "/home/jxb/OS/device/ide.h"
#include "/home/jxb/OS/lib/kernel/debug.h"
#include "/home/jxb/OS/lib/kernel/string.h"
#include "/home/jxb/OS/lib/kernel/memory.h"
#include "/home/jxb/OS/userprog/process.h"
#include "/home/jxb/OS/lib/kernel/stdio_kernel.h"

struct partition* cur_part;	 // 默认情况下操作的是哪个分区

/* 在分区链表中找到名为part_name的分区,并将其指针赋值给cur_part */
bool mount_partition(struct list_elem* pelem, int arg){
   char* part_name = (char*)arg;
   struct partition* part = elem2entry(struct partition, part_tag, pelem);
   if(!strcmp(part->name, part_name)){  //相等
      cur_part = part;
      struct disk* hd = cur_part->my_disk;
      
      //**********    将硬盘上的超级块读入到内存   ***********************
      printk("--------------------mount_partition------------------\n");
      struct super_block* sb_buf = (struct super_block*)sys_malloc(SECTOR_SIZE);
      if(sb_buf == NULL) PANIC("alloc memory failed!");
      //在内存中创建分区cur_part的超级块
      cur_part->sb = (struct super_block*)sys_malloc(sizeof(struct super_block));
      
      if(cur_part->sb == NULL) PANIC("alloc memory failed!");
      memset(sb_buf, 0, SECTOR_SIZE); //512B置零
      ide_read(hd, cur_part->start_lba + 1, sb_buf, 1); //硬盘cur_part分区的超级块写入sb_buf中   
      
      memcpy(cur_part->sb,sb_buf, sizeof(struct super_block)); //把sb_buf中超级块的信息复制到分区的超级块sb中
      //**********     将硬盘上的块位图读入到内存    ********************
      cur_part->block_bitmap.bits = (uint8_t*)sys_malloc(sb_buf->block_bitmap_sects * SECTOR_SIZE);
      if(cur_part->block_bitmap.bits == NULL)  PANIC("alloc memory failed!");
      cur_part->block_bitmap.btmp_bytes_len = sb_buf->block_bitmap_sects * SECTOR_SIZE;
      ide_read(hd, sb_buf->block_bitmap_lba, cur_part->block_bitmap.bits, sb_buf->block_bitmap_sects);   
      //**********     将硬盘上的inode位图读入到内存    *******************
      cur_part->inode_bitmap.bits = (uint8_t*)sys_malloc((sb_buf->inode_bitmap_sects) * SECTOR_SIZE);
      if(cur_part->inode_bitmap.bits == NULL) PANIC("alloc memory failed!");
      cur_part->inode_bitmap.btmp_bytes_len = sb_buf->inode_bitmap_sects * SECTOR_SIZE;
      ide_read(hd, sb_buf->inode_bitmap_lba, cur_part->inode_bitmap.bits, sb_buf->inode_bitmap_sects);   
      
      list_init(&cur_part->open_inodes);
      printk("mount %s done!\n", part->name);
      
      return true; // 使list_traversal停止遍历
      
   }
   return false;   // 使list_traversal继续遍历
}

/* 格式化分区,也就是初始化分区的元信息,创建文件系统
分区结构：
     操作系统引导块| 超级块 |      空闲块位图     |       inode位图     |     inode数组       |  根目录  |  空闲块区域
大小(扇区)  1       1      block_bitmap_sects   inode_bitmap_sects   inode_table_sects          
*/
void partition_format(struct partition* part) {

   uint32_t boot_sector_sects = 1;	  
   uint32_t super_block_sects = 1;
   uint32_t inode_bitmap_sects = DIV_ROUND_UP(MAX_FILES_PER_PART, BITS_PER_SECTOR);	   // 最大文件数/每扇区容纳文件数
   uint32_t inode_table_sects = DIV_ROUND_UP(((sizeof(struct inode) * MAX_FILES_PER_PART)), SECTOR_SIZE);
 
   uint32_t used_sects = boot_sector_sects + super_block_sects + inode_bitmap_sects + inode_table_sects;
   uint32_t free_sects = part->sec_cnt - used_sects;

   printk("free_sects = %d\n",free_sects);  

   /*
      （记录空闲块的)位图所占用的扇区数x  +  空闲块所占用的扇区数y   =   free_sects   
                               =>  x + y = x + x*BITS_PER_SECTOR = (1 + BITS_PER_SECTOR)x = free_sects
       (记录空闲块的)位图所占用的扇区数x   =  空闲块所占用的扇区数y / BITS_PER_SECTOR  
                               =>  y = x*BITS_PER_SECTOR
      

   解：     
            x = free_sects/(1 + BITS_PER_SECTOR);
            y = free_sects - x
   */
   uint32_t block_bitmap_sects;    // (记录空闲块的)位图所占用的扇区数x
   uint32_t block_bitmap_bit_len;  // 空闲块所占用的扇区数y 
   block_bitmap_sects = DIV_ROUND_UP(free_sects, BITS_PER_SECTOR + 1); 
   block_bitmap_bit_len = free_sects - block_bitmap_sects; 
   printk("block_bitmap_sects = %d\n",block_bitmap_sects);  
   printk("block_bitmap_bit_len = %d\n",block_bitmap_bit_len);  

   
   /* ------------------超级块初始化 start  ---------------------------------------*/
   struct super_block sb;
   sb.magic = 0x19980320;
   sb.sec_cnt = part->sec_cnt;
   sb.inode_cnt = MAX_FILES_PER_PART;
   sb.part_lba_base = part->start_lba;
   printk("part->start_lba = sb.part_lba_base = %d\n",part->start_lba);

   sb.block_bitmap_lba = sb.part_lba_base + 2;	 // 第0块是引导块,第1块是超级块
   sb.block_bitmap_sects = block_bitmap_sects;  // (记录空闲块的)位图所占用的扇区数x
   
   printk("sb.block_bitmap_lba = %d\n",sb.block_bitmap_lba);  
   

   sb.inode_bitmap_lba = sb.block_bitmap_lba + sb.block_bitmap_sects;
   sb.inode_bitmap_sects = inode_bitmap_sects; 

   printk("sb.inode_bitmap_lba = %d\n",sb.inode_bitmap_lba);  


   sb.inode_table_lba = sb.inode_bitmap_lba + sb.inode_bitmap_sects;
   sb.inode_table_sects = inode_table_sects; 

   printk("sb.inode_table_lba = %d\n",sb.inode_table_lba);  

   sb.data_start_lba = sb.inode_table_lba + sb.inode_table_sects; //根目录所在的扇区号
   sb.root_inode_no = 0; //根目录的i节点
   sb.dir_entry_size = sizeof(struct dir_entry);

   printk("sb.data_start_lba = %d\n",sb.data_start_lba ); 


   /* ------------------超级块初始化 done  ----------------------------------------*/

   printk("%s info:\n", part->name);
   printk("   magic:0x%x\n   part_lba_base:0x%x\n   all_sectors:0x%x\n   inode_cnt:0x%x\n   block_bitmap_lba:0x%x\n   block_bitmap_sectors:0x%x\n   inode_bitmap_lba:0x%x\n   inode_bitmap_sectors:0x%x\n   inode_table_lba:0x%x\n   inode_table_sectors:0x%x\n   data_start_lba:0x%x\n", sb.magic, sb.part_lba_base, sb.sec_cnt, sb.inode_cnt, sb.block_bitmap_lba, sb.block_bitmap_sects, sb.inode_bitmap_lba, sb.inode_bitmap_sects, sb.inode_table_lba, sb.inode_table_sects, sb.data_start_lba);

   struct disk* hd = part->my_disk;
/*******************************
 * 1 将超级块写入本分区的1扇区 *
 ******************************/
   ide_write(hd, part->start_lba + 1, &sb, 1);   //将超级块写入本分区的1号扇区
   printk("   super_block_lba:0x%x\n", part->start_lba + 1);

/* 找出数据量最大的元信息,用其尺寸做存储缓冲区*/
   uint32_t buf_size = (sb.block_bitmap_sects >= sb.inode_bitmap_sects) ? sb.block_bitmap_sects : sb.inode_bitmap_sects;
   buf_size = ((buf_size >= sb.inode_table_sects) ? buf_size : sb.inode_table_sects) * SECTOR_SIZE;
   uint8_t* buf = (uint8_t*)sys_malloc(buf_size);	// 申请的内存由内存管理系统清0后返回

   
/**************************************
 * 2 将块位图初始化并写入sb.block_bitmap_lba *
 *************************************/
   /* 初始化块位图block_bitmap */
   buf[0] |= 0b00000001;       // 0号块预留给根目录(sb.block_bitmap_lba),位图中先占位
   uint32_t block_bitmap_last_byte = block_bitmap_bit_len / 8;
   uint8_t  block_bitmap_last_bit  = block_bitmap_bit_len % 8;
   uint32_t remain_byte = SECTOR_SIZE - (block_bitmap_last_byte % SECTOR_SIZE);	     // last_size是位图所在最后一个扇区中，不足一扇区的其余部分

   /* 1 先将位图最后一字节到其所在的扇区的结束全置为1,即超出实际块数的部分直接置为已占用*/
   memset(&buf[block_bitmap_last_byte], 0xff, remain_byte);
   
   /* 2 再将上一步中覆盖的最后一字节内的有效位重新置0 */
   uint8_t bit_idx = 0;
   while (bit_idx <= block_bitmap_last_bit) {
      buf[block_bitmap_last_byte] &= ~(1 << bit_idx++);
   }
   ide_write(hd, sb.block_bitmap_lba, buf, sb.block_bitmap_sects);

/***************************************
 * 3 将inode位图初始化并写入sb.inode_bitmap_lba *
 ***************************************/
   /* 先清空缓冲区*/
   memset(buf, 0, buf_size);
   buf[0] |= 0b00000001;      // 第0个inode分给了根目录
   /* 由于inode_table中共4096个inode,位图inode_bitmap正好占用1扇区*/
   ide_write(hd, sb.inode_bitmap_lba, buf, sb.inode_bitmap_sects);

/***************************************
 * 4 将inode数组初始化并写入sb.inode_table_lba *
 ***************************************/
 /* 准备写inode_table中的第0项,即根目录所在的inode */
   memset(buf, 0, buf_size);  // 先清空缓冲区buf
   struct inode* i = (struct inode*)buf; 
   i->i_no = 0;   // 根目录占inode数组中第0个inode
   i->i_size = sb.dir_entry_size * 2;	 // 任何目录在默认情况下都有 .(当前目录) 和 ..(上级目录)
   i->i_open_cnts = 0;
   i->write_deny = 0;
   i->i_sectors[0] = sb.data_start_lba;	     // 根目录所在扇区号
  // printk("%d\n", sb.data_start_lba);ASSERT(0);
   ide_write(hd, sb.inode_table_lba, buf, sb.inode_table_sects);
   
/***************************************
 * 5 将根目录初始化并写入sb.data_start_lba
 ***************************************/
   /* 写入根目录的两个目录项.和.. */
   memset(buf, 0, buf_size);
   struct dir_entry* p_de = (struct dir_entry*)buf;

   /* 初始化当前目录"." */
   memcpy(p_de->filename,".",1);
   p_de->i_no = 0; //文件的inode编号是0
   p_de->f_type = FT_DIRECTORY;
   p_de++;
   /* 初始化当前目录父目录".." */
   memcpy(p_de->filename,"..",2);
   p_de->i_no = 0; //根目录的父目录还是自己
   p_de->f_type = FT_DIRECTORY;
   /* sb.data_start_lba已经分配给了根目录,里面是根目录的目录项 */
   ide_write(hd, sb.data_start_lba, buf, 1); //写一个扇区

   printk("   root_dir_lba:0x%x\n", sb.data_start_lba);
   printk("%s format done\n", part->name);
   sys_free(buf);
}

//在根目录下搜索文件pathname,若找到则返回其inode号，否则返回-1
int search_file(const char* pathname){
   uint8_t path_len = strlen(pathname); 
   ASSERT(pathname[0] == '/' && path_len > 1 && path_len < MAX_PATH_LEN);
   char* name = pathname + 1;
   uint32_t block_cnt = 140;//12个直接块+ 128个一级间接块
   uint32_t* all_blocks = (uint32_t*)sys_malloc(48 + 512); //一个块记录占4个字节
   if(all_blocks == NULL){
      PANIC("search file : sys_malloc for all_blocks failed\n");
   }
   

   uint32_t block_idx;
   for(block_idx = 0; block_idx < 12 ; block_idx++){
      all_blocks[block_idx] = root_dir.inode->i_sectors[block_idx];
   }
   uint32_t i_sector12_lba = root_dir.inode->i_sectors[12];
   if( i_sector12_lba!= 0){
      ide_read(cur_part->my_disk,i_sector12_lba,all_blocks+12,1);
   }
   // all_blocks存放的是扇区地址
   uint8_t* buf = (uint8_t*)sys_malloc(1024);
   struct dir_entry* p_de = (struct dir_entry*)buf;

   uint32_t dir_entry_size = cur_part->sb->dir_entry_size;
   uint32_t dir_entry_cnt  = SECTOR_SIZE / dir_entry_size; //1 扇区所容纳的目录项个数
   uint32_t dir_entry_idx;
   //printk("part_name:%s\n",cur_part->name);
   for(block_idx = 0;block_idx < block_cnt;block_idx++){
      if(all_blocks[block_idx] ==  0) continue;

      ide_read(cur_part->my_disk,all_blocks[block_idx],buf,1);

      for(dir_entry_idx = 0; dir_entry_idx < dir_entry_cnt ; dir_entry_idx++){
         if(!strcmp(p_de->filename,name)){ //找到了
            return p_de->i_no;
         }
         p_de++;
      }
      p_de = (struct dir_entry*)buf;
      memset(p_de,0,sizeof(struct dir_entry));
   }
   sys_free(buf);
   sys_free(all_blocks);
   return -1;
}

//创建文件，若成功则返回文件描述符，否则返回-1
int file_create(const char* filename,uint8_t flags){
   
   //为新文件分配inode
   int32_t inode_no = inode_bitmap_alloc(cur_part);
   struct inode* new_file_inode = (struct inode*)sys_malloc(sizeof(struct inode));
   if(new_file_inode == NULL){
      printk("in file_create:sys_malloc for new_file_inode dailed\n");
      return -1;
   }
   inode_init(inode_no,new_file_inode);

   //在全局file_table中申请一个下标
   int fd_idx = get_free_slot_in_global();
   if(fd_idx == -1){
      printk("no aviable fd_idx for new_file_inode\n");
      return -1;
   }
   //printk("fd_idx: %d\n",fd_idx);ASSERT(0);
   file_table[fd_idx].fd_inode = new_file_inode;
   file_table[fd_idx].fd_pos = 0;
   file_table[fd_idx].fd_flag = flags;
   file_table[fd_idx].fd_inode->write_deny = false;

   struct dir_entry new_dir_entry;
   memset(&new_dir_entry,0,sizeof(struct dir_entry));

   create_dir_entry(++filename,inode_no,FT_REGULAR,&new_dir_entry);
  // printk("%s %d %d\n",new_dir_entry.filename,new_dir_entry.i_no,new_dir_entry.f_type);
  // ASSERT(0);

   //同步内存数据到硬盘
   //1. 在根目录下安装目录项 new_dir_entry
   //创建缓冲区
   void* io_buf = sys_malloc(1024);
   //printk("io_buf %x\n",io_buf);ASSERT(0);
   if(io_buf == NULL){
      printk("in file_create:sys_malloc for io_buf failed\n");
      return -1;
   }
   if(!sync_dir_entry(&new_dir_entry,io_buf)){  
      printk("sync_dir_entry failed\n");
      return -1;
   }
   //2. 将创建文件的i结点内容同步到硬盘的inode数组
   inode_sync(cur_part,new_file_inode,io_buf);

   //3. 将inode_bitmap位图同步到硬盘
   //printk("inode_bitmap : %d\n",*((uint8_t*)cur_part->inode_bitmap.bits));ASSERT(0);
   bitmap_sync(cur_part,inode_no,INODE_BITMAP);
   //4. 将创建的文件i结点添加到 open_inodes链表
   list_push(&cur_part->open_inodes,&new_file_inode->inode_tag);
   new_file_inode->i_open_cnts = 1;
   sys_free(io_buf);
   return pcb_fd_install(fd_idx);
}

// 打开或者创建文件成功后，返回文件描述符，否则返回-1
int32_t sys_open(const char* pathname,uint8_t flags){

   int32_t fd = -1;//默认找不到
   // 1. 检查文件是否存在
   int inode_no = search_file(pathname);//只在根目录下搜索
   bool found = (inode_no == -1)? false:true;//true存在
   printk("found %d\n",found);

   if(found && (O_CREAT & flags)){
      printk("%s has already exits!\n",pathname);
   }else if( !found && (O_CREAT & flags)){
      printk("creating file...\n");
      fd = file_create(pathname,flags);
   }else{ //打开文件
      fd = file_open(inode_no,flags);
   }
   return fd;

}
/* 将文件描述符转化为文件表的下标 */
uint32_t fd_local2global(uint32_t local_fd) {
   struct task_struct* cur = running_thread();
   int32_t global_fd = cur->fd_table[local_fd];  
   ASSERT(global_fd >= 0 && global_fd < MAX_FILE_OPEN);
   return (uint32_t)global_fd;
} 
/* 将buf中连续count个字节写入文件描述符fd,成功则返回写入的字节数,失败返回-1 */
int32_t sys_write(int32_t fd, const void* buf, uint32_t count) {
   if (fd < 0) {
      printk("sys_write: fd error\n");
      return -1;
   }
   if (fd == stdout_no) {  
      char tmp_buf[1024] = {0};
      memcpy(tmp_buf, buf, count);
      console_put_str(tmp_buf);
      return count;
   }
   uint32_t _fd = fd_local2global(fd);

   struct file* wr_file = &file_table[_fd];
   if (wr_file->fd_flag & O_WRONLY || wr_file->fd_flag & O_RDWR) {
 
      uint32_t bytes_written  = file_write(wr_file, buf, count);
      return bytes_written;
   } else {
      console_put_str("sys_write: not allowed to write file without flag O_RDWR or O_WRONLY\n");
      return -1;
   }
}

/* 从文件描述符fd指向的文件中读取count个字节到buf,若成功则返回读出的字节数,到文件尾则返回-1 */
int32_t sys_read(int32_t fd, void* buf, uint32_t count) {
   if (fd < 0) {
      printk("sys_read: fd error\n");
      return -1;
   }
   ASSERT(buf != NULL);
   uint32_t _fd = fd_local2global(fd);
   return file_read(&file_table[_fd], buf, count);   
}


/* 在磁盘上搜索文件系统,若没有则格式化分区创建文件系统 */
void filesys_init() {
   uint8_t channel_no = 0; //IDE通道的编号
   uint8_t dev_no;         //一条IDE通道上可以挂载两块硬盘   主、从硬盘
   uint8_t part_idx = 0;   //分区下标

   // sb_buf用来存储从硬盘上读入的超级块 
   printk("--------filesys_init---------\n");
   struct super_block* sb_buf = (struct super_block*)sys_malloc(SECTOR_SIZE);

   if (sb_buf == NULL) {
      PANIC("alloc memory failed!");
   }
   printk("searching filesystem......\n");
   while (channel_no < channel_cnt) {
      dev_no = 0; // 0,1 主、从盘
      while(dev_no < 2){
	      if(dev_no == 0){   // 跨过裸盘hd60M.img
	         dev_no++;
	         continue;
	      }
	      struct disk* hd = &channels[channel_no].devices[dev_no];   // 0 1
	      struct partition* part = hd->prim_parts;
	      while(part_idx < 12){  // 4个主分区+8个逻辑
	         if(part_idx == 4){  // 开始处理逻辑分区
	            part = hd->logic_parts;
	         }
	         if(part->sec_cnt != 0){  // 如果分区存在
	            memset((void*)sb_buf, 0, SECTOR_SIZE);

	            /* 读出分区的超级块,根据魔数是否正确来判断是否存在文件系统 */
	            ide_read(hd, part->start_lba + 1, sb_buf, 1);   

	            // 只支持自己的文件系统
               // 若磁盘上已经有文件系统就不再格式化了
               if(sb_buf->magic == 0x19980320){
                  printk("%s has filesystem\n", part->name);
               }else{
                  printk("formatting %s`s partition %s......\n", hd->name, part->name);
		            partition_format(part);
               }
		         
	         }
	         part_idx++;
	         part++;	// 下一分区
	      }
	      dev_no++;	// 下一磁盘
      }
      channel_no++;	// 下一IDE通道
   }
   sys_free(sb_buf);


   //确定默认操作的分区 
   char default_part[8] = "hdb1";
   //挂载分区 
   list_traversal(&partition_list, mount_partition, (int)default_part);
   //将当前分区的根目录打开
   open_root_dir(cur_part);

   //初始化文件表
   uint32_t fd_idx = 0;
   while(fd_idx < MAX_FILE_OPEN){
      file_table[fd_idx++].fd_inode = NULL;
   }

}
