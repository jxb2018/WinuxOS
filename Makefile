img=/home/jxb/bochs/bin/hd60M.img
BUILD_DIR = /home/jxb/OS/build
ENTRY_POINT = 0xc0001500
AS = nasm
CC = gcc
LD = ld
LIB =  -I lib/kernel/ -I lib/user/ -I kernel/ -I device/ -I thread/ -I fs/ -I userprog/ -I lib/
ASFLAGS = -f elf
CFLAGS = -m32 -Wall $(LIB) -c -fno-stack-protector -fno-builtin -W -Wstrict-prototypes \
         -Wmissing-prototypes 
LDFLAGS = -m elf_i386 -Ttext $(ENTRY_POINT) -e main -Map $(BUILD_DIR)/kernel.map
OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/init.o $(BUILD_DIR)/interrupt.o \
      $(BUILD_DIR)/timer.o $(BUILD_DIR)/kernel.o $(BUILD_DIR)/print.o \
      $(BUILD_DIR)/debug.o $(BUILD_DIR)/string.o $(BUILD_DIR)/memory.o\
	  $(BUILD_DIR)/bitmap.o $(BUILD_DIR)/thread.o $(BUILD_DIR)/list.o $(BUILD_DIR)/switch.o\
	  $(BUILD_DIR)/console.o $(BUILD_DIR)/sync.o $(BUILD_DIR)/keyboard.o \
	  $(BUILD_DIR)/ioqueue.o $(BUILD_DIR)/tss.o $(BUILD_DIR)/process.o \
	  $(BUILD_DIR)/syscall_init.o $(BUILD_DIR)/syscall.o $(BUILD_DIR)/stdio.o\
	  $(BUILD_DIR)/stdio_kernel.o $(BUILD_DIR)/ide.o $(BUILD_DIR)/fs.o \
	  $(BUILD_DIR)/inode.o $(BUILD_DIR)/file.o $(BUILD_DIR)/dir.o

##############     c代码编译     ###############
$(BUILD_DIR)/main.o: kernel/main.c lib/kernel/print.h \
        lib/kernel/debug.h lib/kernel/init.h lib/kernel/stdint.h lib/kernel/string.h\
		lib/kernel/bitmap.h lib/kernel/memory.h thread/thread.h lib/kernel/list.h \
		thread/sync.h lib/kernel/console.h device/keyboard.h device/ioqueue.h userprog/tss.h\
		userprog/process.h lib/user/syscall.h userprog/syscall_init.h lib/stdio.h \
		lib/kernel/stdio_kernel.h device/ide.h fs/dir.h fs/fs.h fs/inode.h fs/file.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/inode.o: fs/inode.c lib/kernel/print.h \
        lib/kernel/debug.h lib/kernel/init.h lib/kernel/stdint.h lib/kernel/string.h\
		lib/kernel/bitmap.h lib/kernel/memory.h thread/thread.h lib/kernel/list.h \
		thread/sync.h lib/kernel/console.h device/keyboard.h device/ioqueue.h userprog/tss.h\
		userprog/process.h lib/user/syscall.h userprog/syscall_init.h lib/stdio.h \
		lib/kernel/stdio_kernel.h device/ide.h fs/dir.h fs/fs.h fs/inode.h fs/file.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/dir.o: fs/dir.c lib/kernel/print.h \
        lib/kernel/debug.h lib/kernel/init.h lib/kernel/stdint.h lib/kernel/string.h\
		lib/kernel/bitmap.h lib/kernel/memory.h thread/thread.h lib/kernel/list.h \
		thread/sync.h lib/kernel/console.h device/keyboard.h device/ioqueue.h userprog/tss.h\
		userprog/process.h lib/user/syscall.h userprog/syscall_init.h lib/stdio.h \
		lib/kernel/stdio_kernel.h device/ide.h fs/dir.h fs/fs.h fs/inode.h fs/file.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/file.o: fs/file.c lib/kernel/print.h \
        lib/kernel/debug.h lib/kernel/init.h lib/kernel/stdint.h lib/kernel/string.h\
		lib/kernel/bitmap.h lib/kernel/memory.h thread/thread.h lib/kernel/list.h \
		thread/sync.h lib/kernel/console.h device/keyboard.h device/ioqueue.h userprog/tss.h\
		userprog/process.h lib/user/syscall.h userprog/syscall_init.h lib/stdio.h \
		lib/kernel/stdio_kernel.h device/ide.h fs/dir.h fs/fs.h fs/inode.h fs/file.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/tss.o: userprog/tss.c lib/kernel/print.h \
        lib/kernel/debug.h lib/kernel/init.h lib/kernel/stdint.h lib/kernel/string.h\
		lib/kernel/bitmap.h lib/kernel/memory.h thread/thread.h lib/kernel/list.h \
		thread/sync.h lib/kernel/console.h device/keyboard.h device/ioqueue.h userprog/tss.h\
		userprog/process.h lib/user/syscall.h userprog/syscall_init.h lib/stdio.h \
		lib/kernel/stdio_kernel.h device/ide.h fs/dir.h fs/fs.h fs/inode.h fs/file.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/fs.o: fs/fs.c lib/kernel/print.h \
        lib/kernel/debug.h lib/kernel/init.h lib/kernel/stdint.h lib/kernel/string.h\
		lib/kernel/bitmap.h lib/kernel/memory.h thread/thread.h lib/kernel/list.h \
		thread/sync.h lib/kernel/console.h device/keyboard.h device/ioqueue.h userprog/tss.h\
		userprog/process.h lib/user/syscall.h userprog/syscall_init.h lib/stdio.h \
		lib/kernel/stdio_kernel.h device/ide.h fs/dir.h fs/fs.h fs/inode.h fs/file.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/ide.o: device/ide.c lib/kernel/print.h \
        lib/kernel/debug.h lib/kernel/init.h lib/kernel/stdint.h lib/kernel/string.h\
		lib/kernel/bitmap.h lib/kernel/memory.h thread/thread.h lib/kernel/list.h \
		thread/sync.h lib/kernel/console.h device/keyboard.h device/ioqueue.h userprog/tss.h\
		userprog/process.h lib/user/syscall.h userprog/syscall_init.h lib/stdio.h \
		lib/kernel/stdio_kernel.h device/ide.h fs/dir.h fs/fs.h fs/inode.h fs/file.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/stdio_kernel.o: lib/kernel/stdio_kernel.c lib/kernel/print.h \
        lib/kernel/debug.h lib/kernel/init.h lib/kernel/stdint.h lib/kernel/string.h\
		lib/kernel/bitmap.h lib/kernel/memory.h thread/thread.h lib/kernel/list.h \
		thread/sync.h lib/kernel/console.h device/keyboard.h device/ioqueue.h userprog/tss.h\
		userprog/process.h lib/user/syscall.h userprog/syscall_init.h lib/stdio.h \
		lib/kernel/stdio_kernel.h device/ide.h fs/dir.h fs/fs.h fs/inode.h fs/file.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/syscall_init.o: userprog/syscall_init.c lib/kernel/print.h \
        lib/kernel/debug.h lib/kernel/init.h lib/kernel/stdint.h lib/kernel/string.h\
		lib/kernel/bitmap.h lib/kernel/memory.h thread/thread.h lib/kernel/list.h \
		thread/sync.h lib/kernel/console.h device/keyboard.h device/ioqueue.h userprog/tss.h\
		userprog/process.h lib/user/syscall.h userprog/syscall_init.h lib/stdio.h \
		lib/kernel/stdio_kernel.h device/ide.h fs/dir.h fs/fs.h fs/inode.h fs/file.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/syscall.o: lib/user/syscall.c lib/kernel/print.h \
        lib/kernel/debug.h lib/kernel/init.h lib/kernel/stdint.h lib/kernel/string.h\
		lib/kernel/bitmap.h lib/kernel/memory.h thread/thread.h lib/kernel/list.h \
		thread/sync.h lib/kernel/console.h device/keyboard.h device/ioqueue.h userprog/tss.h\
		userprog/process.h lib/user/syscall.h userprog/syscall_init.h lib/stdio.h \
		lib/kernel/stdio_kernel.h device/ide.h fs/dir.h fs/fs.h fs/inode.h fs/file.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/process.o: userprog/process.c lib/kernel/print.h \
        lib/kernel/debug.h lib/kernel/init.h lib/kernel/stdint.h lib/kernel/string.h\
		lib/kernel/bitmap.h lib/kernel/memory.h thread/thread.h lib/kernel/list.h \
		thread/sync.h lib/kernel/console.h device/keyboard.h device/ioqueue.h userprog/tss.h\
		userprog/process.h lib/user/syscall.h userprog/syscall_init.h lib/stdio.h \
		lib/kernel/stdio_kernel.h device/ide.h fs/dir.h fs/fs.h fs/inode.h fs/file.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/keyboard.o: device/keyboard.c lib/kernel/print.h \
        lib/kernel/debug.h lib/kernel/init.h lib/kernel/stdint.h lib/kernel/string.h\
		lib/kernel/bitmap.h lib/kernel/memory.h thread/thread.h lib/kernel/list.h \
		thread/sync.h lib/kernel/console.h device/keyboard.h device/ioqueue.h userprog/tss.h\
		userprog/process.h lib/user/syscall.h userprog/syscall_init.h lib/stdio.h \
		lib/kernel/stdio_kernel.h device/ide.h fs/dir.h fs/fs.h fs/inode.h fs/file.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/stdio.o: lib/stdio.c lib/kernel/print.h \
        lib/kernel/debug.h lib/kernel/init.h lib/kernel/stdint.h lib/kernel/string.h\
		lib/kernel/bitmap.h lib/kernel/memory.h thread/thread.h lib/kernel/list.h \
		thread/sync.h lib/kernel/console.h device/keyboard.h device/ioqueue.h userprog/tss.h\
		userprog/process.h lib/user/syscall.h userprog/syscall_init.h lib/stdio.h \
		lib/kernel/stdio_kernel.h device/ide.h fs/dir.h fs/fs.h fs/inode.h fs/file.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/init.o: kernel/init.c lib/kernel/init.h lib/kernel/print.h \
        lib/kernel/stdint.h lib/kernel/interrupt.h lib/kernel/time.h lib/kernel/string.h\
		lib/kernel/bitmap.h lib/kernel/memory.h thread/thread.h lib/kernel/list.h \
		thread/sync.h lib/kernel/console.h device/keyboard.h device/ioqueue.h userprog/tss.h\
		userprog/process.h lib/user/syscall.h userprog/syscall_init.h lib/stdio.h \
		lib/kernel/stdio_kernel.h device/ide.h fs/dir.h fs/fs.h fs/inode.h fs/file.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/interrupt.o: kernel/interrupt.c lib/kernel/interrupt.h \
        lib/kernel/stdint.h lib/kernel/global.h lib/kernel/io.h lib/kernel/print.h lib/kernel/string.h\
		lib/kernel/bitmap.h lib/kernel/memory.h thread/thread.h lib/kernel/list.h \
		thread/sync.h lib/kernel/console.h device/keyboard.h device/ioqueue.h userprog/tss.h\
		userprog/process.h lib/user/syscall.h userprog/syscall_init.h lib/stdio.h \
		lib/kernel/stdio_kernel.h device/ide.h fs/dir.h fs/fs.h fs/inode.h fs/file.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/timer.o: device/time.c lib/kernel/time.h lib/kernel/stdint.h\
         lib/kernel/io.h lib/kernel/print.h lib/kernel/string.h\
		 lib/kernel/bitmap.h lib/kernel/memory.h thread/thread.h lib/kernel/list.h \
		 thread/sync.h lib/kernel/console.h device/keyboard.h device/ioqueue.h userprog/tss.h\
		 userprog/process.h lib/user/syscall.h userprog/syscall_init.h lib/stdio.h \
		 lib/kernel/stdio_kernel.h device/ide.h fs/dir.h fs/fs.h fs/inode.h fs/file.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/debug.o: lib/kernel/debug.c lib/kernel/debug.h \
        lib/kernel/print.h lib/kernel/stdint.h lib/kernel/interrupt.h lib/kernel/string.h\
		lib/kernel/bitmap.h lib/kernel/memory.h thread/thread.h lib/kernel/list.h \
		thread/sync.h lib/kernel/console.h device/keyboard.h device/ioqueue.h userprog/tss.h\
		userprog/process.h lib/user/syscall.h userprog/syscall_init.h lib/stdio.h \
		lib/kernel/stdio_kernel.h device/ide.h fs/dir.h fs/fs.h fs/inode.h fs/file.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/string.o: lib/kernel/string.c lib/kernel/debug.h \
        lib/kernel/print.h lib/kernel/stdint.h lib/kernel/interrupt.h lib/kernel/string.h\
		lib/kernel/bitmap.h lib/kernel/memory.h thread/thread.h lib/kernel/list.h \
		thread/sync.h lib/kernel/console.h device/keyboard.h device/ioqueue.h userprog/tss.h\
		userprog/process.h lib/user/syscall.h userprog/syscall_init.h lib/stdio.h \
		lib/kernel/stdio_kernel.h device/ide.h fs/dir.h fs/fs.h fs/inode.h fs/file.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/bitmap.o: lib/kernel/bitmap.c lib/kernel/debug.h \
        lib/kernel/print.h lib/kernel/stdint.h lib/kernel/interrupt.h lib/kernel/string.h \
		lib/kernel/bitmap.h lib/kernel/memory.h thread/thread.h lib/kernel/list.h \
		thread/sync.h lib/kernel/console.h device/keyboard.h device/ioqueue.h userprog/tss.h\
		userprog/process.h lib/user/syscall.h userprog/syscall_init.h lib/stdio.h \
		lib/kernel/stdio_kernel.h device/ide.h fs/dir.h fs/fs.h fs/inode.h fs/file.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/memory.o: lib/kernel/memory.c lib/kernel/debug.h \
        lib/kernel/print.h lib/kernel/stdint.h lib/kernel/interrupt.h lib/kernel/string.h \
		lib/kernel/bitmap.h lib/kernel/memory.h thread/thread.h lib/kernel/list.h \
		thread/sync.h lib/kernel/console.h device/keyboard.h device/ioqueue.h userprog/tss.h\
		userprog/process.h lib/user/syscall.h userprog/syscall_init.h lib/stdio.h \
		lib/kernel/stdio_kernel.h device/ide.h fs/dir.h fs/fs.h fs/inode.h fs/file.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/list.o: lib/kernel/list.c lib/kernel/debug.h \
        lib/kernel/print.h lib/kernel/stdint.h lib/kernel/interrupt.h lib/kernel/string.h \
		lib/kernel/bitmap.h lib/kernel/memory.h thread/thread.h lib/kernel/list.h \
		thread/sync.h lib/kernel/console.h device/keyboard.h device/ioqueue.h userprog/tss.h\
		userprog/process.h lib/user/syscall.h userprog/syscall_init.h lib/stdio.h \
		lib/kernel/stdio_kernel.h device/ide.h fs/dir.h fs/fs.h fs/inode.h fs/file.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/thread.o: thread/thread.c lib/kernel/debug.h \
        lib/kernel/print.h lib/kernel/stdint.h lib/kernel/interrupt.h lib/kernel/string.h \
		lib/kernel/bitmap.h lib/kernel/memory.h thread/thread.h lib/kernel/list.h \
		thread/sync.h lib/kernel/console.h device/keyboard.h device/ioqueue.h userprog/tss.h\
		userprog/process.h lib/user/syscall.h userprog/syscall_init.h lib/stdio.h \
		lib/kernel/stdio_kernel.h device/ide.h fs/dir.h fs/fs.h fs/inode.h fs/file.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/console.o: device/console.c lib/kernel/debug.h \
        lib/kernel/print.h lib/kernel/stdint.h lib/kernel/interrupt.h lib/kernel/string.h \
		lib/kernel/bitmap.h lib/kernel/memory.h thread/thread.h lib/kernel/list.h \
		thread/sync.h lib/kernel/console.h device/keyboard.h device/ioqueue.h userprog/tss.h\
		userprog/process.h lib/user/syscall.h userprog/syscall_init.h lib/stdio.h \
		lib/kernel/stdio_kernel.h device/ide.h fs/dir.h fs/fs.h fs/inode.h fs/file.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/sync.o: thread/sync.c lib/kernel/debug.h \
        lib/kernel/print.h lib/kernel/stdint.h lib/kernel/interrupt.h lib/kernel/string.h \
		lib/kernel/bitmap.h lib/kernel/memory.h thread/thread.h lib/kernel/list.h \
		thread/sync.h lib/kernel/console.h device/keyboard.h device/ioqueue.h userprog/tss.h\
		userprog/process.h lib/user/syscall.h userprog/syscall_init.h lib/stdio.h \
		lib/kernel/stdio_kernel.h device/ide.h fs/dir.h fs/fs.h fs/inode.h fs/file.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/ioqueue.o: device/ioqueue.c lib/kernel/debug.h \
        lib/kernel/print.h lib/kernel/stdint.h lib/kernel/interrupt.h lib/kernel/string.h \
		lib/kernel/bitmap.h lib/kernel/memory.h thread/thread.h lib/kernel/list.h \
		thread/sync.h lib/kernel/console.h device/keyboard.h device/ioqueue.h userprog/tss.h\
		userprog/process.h lib/user/syscall.h userprog/syscall_init.h lib/stdio.h \
		lib/kernel/stdio_kernel.h device/ide.h fs/dir.h fs/fs.h fs/inode.h fs/file.h
	$(CC) $(CFLAGS) $< -o $@
##############    汇编代码编译    ###############
$(BUILD_DIR)/kernel.o: kernel/kernel.s
	$(AS) $(ASFLAGS) $< -o $@
$(BUILD_DIR)/print.o: lib/kernel/print.s
	$(AS) $(ASFLAGS) $< -o $@
$(BUILD_DIR)/switch.o: thread/switch.s
	$(AS) $(ASFLAGS) $< -o $@
##############    链接所有目标文件    #############
$(BUILD_DIR)/kernel.bin: $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

.PHONY : mk_dir hd clean all

mk_dir:
	if [ ! -d $(BUILD_DIR) ];then mkdir $(BUILD_DIR);fi

mbr_loader:
	nasm -I boot/include/ -o build/mbr.bin boot/mbr.s
	nasm -I boot/include/ -o build/loader.bin boot/loader.s

build: $(BUILD_DIR)/kernel.bin

clean: rm -f $(BUILD_DIR)/*
hd:
	dd if=$(BUILD_DIR)/mbr.bin of=$(img) bs=512 count=1 conv=notrunc
	dd if=$(BUILD_DIR)/loader.bin of=$(img) bs=512 seek=2 count=3 conv=notrunc
	dd if=$(BUILD_DIR)/kernel.bin of=$(img) bs=512 seek=9 count=200 conv=notrunc

runbochs: 
	bash run_bochs

all: mbr_loader build hd runbochs