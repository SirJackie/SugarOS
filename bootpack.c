
/*
** SugarOS 主程序
*/

#include "bootpack.h"


void HariMain(void)
{
	//初始化BootInfo
	struct BOOTINFO *binfo;
	binfo = (struct BOOTINFO *) 0x0ff0;  //从asmhead.nas里读数据

	//初始化标准输入输出
	extern char hankaku[4096];
	struct ConsoleStatus *cs;
	cs->callbackWhenPutChar = (void *)video_putChar8;
	cs->callbackWhenRefresh = (void *)video_refreshBackground;
	cs->console_cursorX = 8;
	cs->console_cursorY = 8;
	cs->fontLibrary = hankaku;

	//初始化GDT和IDT
	init_gdtidt();

	//初始化调色板并绘制背景
	video_init_palette();
	video_refreshBackground(binfo, (void *)video_fillRect8);

	//初始化并绘制鼠标
	char mcursor[256];
	int mx = (binfo->screenWidth - 16) / 2;
	int my = (binfo->screenHeight - 28 - 16) / 2;
	video_init_mouse_cursor8(mcursor, COL8_008484);
	video_drawBitmap(binfo, mx, my, 16, 16, mcursor);

	//测试英文字母
	video_println(binfo, "A quick brown fox jump over the lazy dog.", cs);

	//打印BootInfo
	char buffer[20];
	sprintf(buffer, "binfo->cyls = %d", binfo->cyls);
	video_println(binfo, buffer, cs);

	sprintf(buffer, "binfo->leds = %d", binfo->leds);
	video_println(binfo, buffer, cs);

	sprintf(buffer, "binfo->vmode = %d", binfo->vmode);
	video_println(binfo, buffer, cs);

	sprintf(buffer, "binfo->screenWidth = %d", binfo->screenWidth);
	video_println(binfo, buffer, cs);

	sprintf(buffer, "binfo->screenHeight = %d", binfo->screenHeight);
	video_println(binfo, buffer, cs);

	sprintf(buffer, "binfo->VideoRamAddress = 0x%x", binfo->VideoRamAddress);
	video_println(binfo, buffer, cs);

	//休眠
	for (;;) {
		io_hlt();
	}
}
