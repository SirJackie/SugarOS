
/*
** SugarOS 主程序
*/

#include "bootpack.h"


/*
** 外部变量引用
*/

extern char hankaku[4096]; //引用字库
extern struct FIFO8 keyfifo, mousefifo; //从mouse.c和keyboard.c中引用键盘和鼠标缓冲区


void HariMain(void)
{
	//初始化BootInfo
	struct BOOTINFO *binfo;
	binfo = (struct BOOTINFO *) ADR_BOOTINFO;  //从asmhead.nas里读数据

	//初始化GDT和IDT
	init_gdtidt();

	//初始化PIC中断
	init_pic();
	io_sti(); /* 允许中断 */

	//初始化调色板并绘制背景
	video_init_palette();
	video_refreshBackground(binfo, (void *)video_fillRect8);

	//初始化Console并绘制控制台背景
	struct ConsoleStatus cs;
	cs.callbackWhenFillRect = (void *)video_fillRect8;
	cs.callbackWhenPutChar = (void *)video_putChar8;
	cs.x0 = 8;
	cs.y0 = 24;
	cs.x1 = binfo->screenWidth  - 8;
	cs.y1 = binfo->screenHeight - 32;
	cs.console_cursorX = 8;
	cs.console_cursorY = 24;
	cs.backgroundColor = COL8_008484;
	cs.fontLibrary = hankaku;
	// struct ConsoleStatus *csptr = console_init(binfo, (struct ConsoleStatus *)&cs);
	struct ConsoleStatus *csptr = (struct ConsoleStatus *)&cs;

	//初始化鼠标图像,并绘制鼠标
	char mcursor[256];
	int mx = (binfo->screenWidth - 16) / 2;
	int my = (binfo->screenHeight - 28 - 16) / 2;
	video_init_mouse_cursor8(mcursor, COL8_008484);
	video_drawBitmap(binfo, mx, my, 16, 16, mcursor);

	//PIC设定
	io_out8(PIC0_IMR, 0xf9); /* PIC1�ƃL�[�{�[�h������(11111001) */
	io_out8(PIC1_IMR, 0xef); /* �}�E�X������(11101111) */

	//创建字符缓冲区
	char buffer[40];
	unsigned char i;

	//初始化键盘和鼠标FIFO缓冲区
	unsigned char keybuf[32];
	fifo8_init(&keyfifo, 32, keybuf);
	unsigned char mousebuf[128];
	fifo8_init(&mousefifo, 128, mousebuf);

	//初始化键盘和鼠标PIC
	init_keyboard();
	struct MOUSE_DEC mdec;
	enable_mouse(&mdec);

	int memtmp;
	memtmp = memtest_sub(0x00400000, 0xbfffffff) / (1024 * 1024);
	sprintf(buffer, "Memory: %dMB", memtmp);
	console_println(binfo, buffer, csptr);

	/* 开始响应 */
	for(;;){
		io_cli(); //禁止中断
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0)
		{
			/* 如果鼠标和键盘没有动作 */
			io_stihlt(); //恢复中断并紧接着休眠(STI和HLT的汇编必须编译在一起!)
		} else {
			/* 如果有动作 */
			if (fifo8_status(&keyfifo) != 0) {
				/* 如果是键盘的动作 */
				i = fifo8_pop(&keyfifo);    //取出动作
				io_sti();                   //恢复中断
				/* 打印键盘动作 */
				sprintf(buffer, "Keyboard: %02X", i); 
				console_println(binfo, buffer, csptr);
			}
			if (fifo8_status(&mousefifo) != 0) {
				/* 如果是鼠标的动作 */
				i = fifo8_pop(&mousefifo);  //取出动作
				io_sti();                   //恢复中断
				if (mouse_decode(&mdec, i) != 0) {
					/* 当鼠标的3个字节消息凑够 */
					
					/* 打印键盘动作 */
					sprintf(buffer, "[lcr %4d %4d]", mdec.x, mdec.y);
					if ((mdec.btn & 0x01) != 0) {
						buffer[1] = 'L';
					}
					if ((mdec.btn & 0x02) != 0) {
						buffer[3] = 'R';
					}
					if ((mdec.btn & 0x04) != 0) {
						buffer[2] = 'C';
					}
					video_fillRect8(binfo, 150, 8, 300, 23, COL8_008484);
					video_putShadowString8(binfo, 150, 8, buffer);

					//擦除鼠标并重绘菜单栏
					video_fillRect8(binfo, mx, my, mx+15, my+15, COL8_008484);
					if(my >= binfo->screenHeight - (28+16)){
						/* 上次鼠标在菜单栏运动 */
						video_refreshMenuBar(binfo, video_fillRect8); //还要重绘菜单栏
					}

					//加入移动量
					mx += mdec.x;
					my += mdec.y;

					//边界处理
					if(mx <= 0){
						mx = 0;
					}
					else if(mx >= binfo->screenWidth - 16){
						mx = binfo->screenWidth - 16;
					}
					if(my <= 0){
						my = 0;
					}
					else if(my >= binfo->screenHeight - 16){
						my = binfo->screenHeight - 16;
					}

					//显示鼠标坐标
					sprintf(buffer, "(%3d, %3d)", mx, my);
					video_fillRect8(binfo, 8, 8, 88, 24, COL8_008484);
					video_putShadowString8(binfo, 8, 8, buffer);

					//绘制鼠标
					video_drawBitmap(binfo, mx, my, 16, 16, mcursor);
				}
			}
		}
	}
}
