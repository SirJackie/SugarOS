
/*
** SugarOS 主程序
*/

#include "bootpack.h"

void HariMain(void)
{
	//初始化BootInfo
	struct BOOTINFO *binfo;
	binfo = (struct BOOTINFO *) ADR_BOOTINFO;  //从asmhead.nas里读数据

	//初始化标准输入输出
	struct ConsoleStatus cs;
	cs.callbackWhenFillRect = (void *)video_fillRect8;
	cs.callbackWhenPutChar = (void *)video_putChar8;
	cs.callbackWhenRefresh = (void *)video_refreshBackground;
	cs.console_cursorX = 8;
	cs.console_cursorY = 8;
	struct ConsoleStatus *csptr = (struct ConsoleStatus *)&cs;

	//初始化GDT和IDT
	init_gdtidt();

	//初始化PIC中断
	init_pic();
	io_sti(); /* 允许中断 */

	//初始化调色板并绘制背景
	video_init_palette();
	video_refreshBackground(binfo, (void *)video_fillRect8);

	//初始化并绘制鼠标
	char mcursor[256];
	int mx = (binfo->screenWidth - 16) / 2;
	int my = (binfo->screenHeight - 28 - 16) / 2;
	video_init_mouse_cursor8(mcursor, COL8_008484);
	video_drawBitmap(binfo, mx, my, 16, 16, mcursor);

	//显示鼠标坐标
	char buffer[40];
	sprintf(buffer, "(%d, %d)", mx, my);
	video_println(binfo, buffer, csptr);

	io_out8(PIC0_IMR, 0xf9); /* PIC1�ƃL�[�{�[�h������(11111001) */
	io_out8(PIC1_IMR, 0xef); /* �}�E�X������(11101111) */

	//初始化键盘变量和键盘FIFO缓冲区
	unsigned char i;
	int j;
	extern struct FIFO8 keyfifo;
	unsigned char keybuf[32];
	fifo8_init(&keyfifo, 32, keybuf);

	//休眠
	for (;;) {
		io_cli(); //停止中断
		if(fifo8_status(&keyfifo) == 0){ //如果键盘没有被按下
			io_stihlt(); //恢复中断紧接着休眠(sti和hlt的汇编必须连在一起!)
		}
		else{
			i = fifo8_pop(&keyfifo);
			io_sti();
			sprintf(buffer, "%02X", i);
			video_println(binfo, buffer, csptr);
		}
	}
}
