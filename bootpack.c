
/*
** SugarOS 主程序
*/

#include "bootpack.h"


void HariMain(void)
{
	struct BOOTINFO *binfo;
	binfo = (struct BOOTINFO *) 0x0ff0;  //从asmhead.nas里读数据

	//初始化GDT和IDT
	init_gdtidt();

	//初始化调色板
	video_init_palette();

	//初始化鼠标
	char mcursor[256];
	int mx = (binfo->screenWidth - 16) / 2;
	int my = (binfo->screenHeight - 28 - 16) / 2;
	video_init_mouse_cursor8(mcursor, COL8_008484);

	//绘制背景
	video_refreshBackground(binfo->VideoRamAddress, binfo->screenWidth, binfo->screenHeight);

	//绘制鼠标
	video_drawBitmap(binfo, 16, 16, mx, my, mcursor, 16);

	//测试英文字母
	video_println(binfo, "A quick brown fox jump over the lazy dog.");

	//打印BootInfo
	char buffer[20];
	sprintf(buffer, "binfo->cyls = %d", binfo->cyls);
	video_println(binfo, buffer);

	sprintf(buffer, "binfo->leds = %d", binfo->leds);
	video_println(binfo, buffer);

	sprintf(buffer, "binfo->vmode = %d", binfo->vmode);
	video_println(binfo, buffer);

	sprintf(buffer, "binfo->screenWidth = %d", binfo->screenWidth);
	video_println(binfo, buffer);

	sprintf(buffer, "binfo->screenHeight = %d", binfo->screenHeight);
	video_println(binfo, buffer);

	sprintf(buffer, "binfo->VideoRamAddress = 0x%x", binfo->VideoRamAddress);
	video_println(binfo, buffer);

	//休眠
	for (;;) {
		io_hlt();
	}
}
