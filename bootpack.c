
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
	extern char hankaku[4096];
	struct ConsoleStatus *cs;
	cs->callbackWhenPutChar = (void *)video_putChar8;
	cs->callbackWhenRefresh = (void *)video_refreshBackground;
	cs->console_cursorX = 8;
	cs->console_cursorY = 8;

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
	video_putShadowString8(binfo, 0, 0, buffer);

	io_out8(PIC0_IMR, 0xf9); /* PIC1�ƃL�[�{�[�h������(11111001) */
	io_out8(PIC1_IMR, 0xef); /* �}�E�X������(11101111) */

	//休眠
	for (;;) {
		io_hlt();
	}
}
