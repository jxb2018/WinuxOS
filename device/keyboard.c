#include "keyboard.h"
#include "/home/jxb/OS/lib/kernel/io.h"
#include "/home/jxb/OS/lib/kernel/interrupt.h"
#include "/home/jxb/OS/lib/kernel/print.h"
#include "/home/jxb/OS/device/ioqueue.h"
#define KBD_BUF_PORT 0x60  //键盘 buffer 存器端口号为 Ox60
struct ioqueue kdb_buf;

/* 用转义字符定义部分控制字符 */
#define esc		    '\033'	 // 八进制表示字符,也可以用十六进制'\x1b'
#define backspace	'\b'
#define tab		    '\t'
#define enter		'\r'
#define delete		'\177'	 // 八进制表示字符,十六进制为'\x7f'

/* 以上不可见字符一律定义为0 */
#define char_invisible	 0
#define ctrl_l_char     char_invisible
#define ctrl_r_char     char_invisible
#define shift_l_char	char_invisible
#define shift_r_char	char_invisible
#define alt_l_char	    char_invisible
#define alt_r_char	    char_invisible
#define caps_lock_char	char_invisible

/* 定义控制字符的通码和断码 */
#define shift_l_make	0x2a
#define shift_r_make 	0x36 
#define alt_l_make   	0x38
#define alt_r_make   	0xe038
#define alt_r_break   	0xe0b8
#define ctrl_l_make  	0x1d
#define ctrl_r_make  	0xe01d
#define ctrl_r_break 	0xe09d
#define caps_lock_make 	0x3a



// 记录相应键是否按下
static int ctrl_status=0,shift_status=0,alt_status=0,caps_lock_status=0,\
                                ext_scancode=0;//拓展扫描玛的标记


/* 以通码为索引的二维数组 */
static char keymap[][2] = {
/* 扫描码 未与shift组合 与shift组合*/
/* 0x00 */	{0  ,	  0},		
/* 0x01 */	{esc,	esc},		
/* 0x02 */	{'1',	'!'},		
/* 0x03 */	{'2',	'@'},		
/* 0x04 */	{'3',	'#'},		
/* 0x05 */	{'4',	'$'},		
/* 0x06 */	{'5',	'%'},		
/* 0x07 */	{'6',	'^'},		
/* 0x08 */	{'7',	'&'},		
/* 0x09 */	{'8',	'*'},		
/* 0x0A */	{'9',	'('},		
/* 0x0B */	{'0',	')'},		
/* 0x0C */	{'-',	'_'},		
/* 0x0D */	{'=',	'+'},		
/* 0x0E */	{backspace, backspace},	
/* 0x0F */	{tab,	tab},		
/* 0x10 */	{'q',	'Q'},		
/* 0x11 */	{'w',	'W'},		
/* 0x12 */	{'e',	'E'},		
/* 0x13 */	{'r',	'R'},		
/* 0x14 */	{'t',	'T'},		
/* 0x15 */	{'y',	'Y'},		
/* 0x16 */	{'u',	'U'},		
/* 0x17 */	{'i',	'I'},		
/* 0x18 */	{'o',	'O'},		
/* 0x19 */	{'p',	'P'},		
/* 0x1A */	{'[',	'{'},		
/* 0x1B */	{']',	'}'},		
/* 0x1C */	{enter,  enter},
/* 0x1D */	{ctrl_l_char, ctrl_l_char},
/* 0x1E */	{'a',	'A'},		
/* 0x1F */	{'s',	'S'},		
/* 0x20 */	{'d',	'D'},		
/* 0x21 */	{'f',	'F'},		
/* 0x22 */	{'g',	'G'},		
/* 0x23 */	{'h',	'H'},		
/* 0x24 */	{'j',	'J'},		
/* 0x25 */	{'k',	'K'},		
/* 0x26 */	{'l',	'L'},		
/* 0x27 */	{';',	':'},		
/* 0x28 */	{'\'',	'"'},		
/* 0x29 */	{'`',	'~'},		
/* 0x2A */	{shift_l_char, shift_l_char},	
/* 0x2B */	{'\\',	'|'},		
/* 0x2C */	{'z',	'Z'},		
/* 0x2D */	{'x',	'X'},		
/* 0x2E */	{'c',	'C'},		
/* 0x2F */	{'v',	'V'},		
/* 0x30 */	{'b',	'B'},		
/* 0x31 */	{'n',	'N'},		
/* 0x32 */	{'m',	'M'},		
/* 0x33 */	{',',	'<'},		
/* 0x34 */	{'.',	'>'},		
/* 0x35 */	{'/',	'?'},
/* 0x36	*/	{shift_r_char, shift_r_char},	
/* 0x37 */	{'*',	'*'},    	
/* 0x38 */	{alt_l_char, alt_l_char},
/* 0x39 */	{' ',	' '},		
/* 0x3A */	{caps_lock_char, caps_lock_char}
/*其它按键暂不处理*/
};

/* 键盘中断处理程序 */
/*
    1. 从端口 KBD_BUF_PORT取一个字节
    2. 判断字节是不是e0开头。如果是，则结束中断并返回；否则继续处理
    3. 判断字节是通码还是断码，如果是断码转去执行4，否则转去执行5
    4. 由断码得到通码，只对三状态处理(ctrl_status,shift_status,alt_status),中断返回
    5. 计算得到shift,index。从keymap中获取对于的字符，打印输出，并记录更新四状态

*/
static void intr_keyboard_handler(void) {
/* 这次中断发生前的上一次中断,以下任意三个键是否有按下 */
   int ctrl_down_last = ctrl_status;	  
   int shift_down_last = shift_status;
   int caps_lock_last = caps_lock_status;

   int break_code; //断码标记
   uint16_t scancode = inb(KBD_BUF_PORT);

   //结束此次中断处理函数,等待下一个扫描码进来 
   if(scancode == 0xe0){ 
      ext_scancode = 1;    // 打开e0标记
      return;
   }

/* 如果上次是以0xe0开头,将扫描码合并 */
   if(ext_scancode){
        scancode = ((0xe000) | scancode);
        ext_scancode = 0;   // 关闭e0标记
   }   

   break_code = ((scancode & 0x0080) != 0);   // 检测是否是断码
   
   if(break_code){  // 是断码
        uint16_t make_code = (scancode &= 0xff7f);   // 得到通码
        if(make_code == ctrl_l_make || make_code == ctrl_r_make){
	        ctrl_status = 0;
        }else if(make_code == shift_l_make || make_code == shift_r_make){
	        shift_status = 0;
        }else if(make_code == alt_l_make || make_code == alt_r_make){
	        alt_status = 0;
        } // 由于caps_lock不是弹起后关闭,所以需要单独处理
        return;   // 直接返回结束此次中断处理程序
   }else if((scancode > 0x00 && scancode < 0x3b) || (scancode == alt_r_make) ||(scancode == ctrl_r_make)){
        int shift = 0;  // 判断是否与shift组合,用来在一维数组中索引对应的字符
        if((0x10<=scancode && scancode <=0x19)||(0x1e<=scancode && scancode<=0x26)||(0x2c<=scancode && scancode<=0x32)){ //字母键
            if(shift_down_last || caps_lock_last){ // 如果shift和capslock任意被按下
	            shift = 1;
	        }else{
	            shift = 0;
	        }
        }else{ //非字母键
            if(shift_down_last) shift = 1;
        }

        uint8_t index = (scancode &= 0x00ff);  // 将扫描码的高字节置0,主要是针对高字节是e0的扫描码.
        char cur_char = keymap[index][shift];  // 在数组中找到对应的字符

        if(cur_char) { //cur_char不为0
            //put_char(cur_char);
            ioq_putchar(&kdb_buf,cur_char);
            return;
	    }
        // 记录本次按下的控制建
        if(scancode == ctrl_l_make || scancode == ctrl_r_make) {
	        ctrl_status = 1;
         }else if(scancode == shift_l_make || scancode == shift_r_make) {
	        shift_status = 1;
         }else if(scancode == alt_l_make || scancode == alt_r_make) {
	        alt_status = 1;
         }else if(scancode == caps_lock_make) {
	        caps_lock_status = !caps_lock_status;
         }
   }else{
      put_str("unknown key\n");
   }
}


//键盘初始化
void keyboard_init(){
    ioq_init(&kdb_buf);
    register_handler(0x21,intr_keyboard_handler);
}