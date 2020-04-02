
/*
** SugarOS 图形部分
*/

#include "bootpack.h"


/*
** 调色板部分
*/

void video_set_palette(int start, int end, unsigned char *rgb)
/* 写入调色板 */
{
	int i, eflags;
	eflags = io_load_eflags(); //保存eflags值
	io_cli(); //停止中断
	io_out8(0x03c8, start); //写调色板
	for (i = start; i <= end; i++) {
		/* 三位三位输出 */
		io_out8(0x03c9, rgb[0] / 4);
		io_out8(0x03c9, rgb[1] / 4);
		io_out8(0x03c9, rgb[2] / 4);
		//RGB指针右移三位
		rgb += 3;
	}
	io_store_eflags(eflags); //恢复eflags值
	return;
}

void video_init_palette(void)
/* 初始化调色板 */
{
	static unsigned char table_rgb[16 * 3] = {
		0x00, 0x00, 0x00,	/*  0:黑     */
		0xff, 0x00, 0x00,	/*  1:亮红   */
		0x00, 0xff, 0x00,	/*  2:亮绿   */
		0xff, 0xff, 0x00,	/*  3:亮黄   */
		0x00, 0x00, 0xff,	/*  4:亮蓝   */
		0xff, 0x00, 0xff,	/*  5:亮紫   */
		0x00, 0xff, 0xff,	/*  6:浅亮蓝 */
		0xff, 0xff, 0xff,	/*  7:白     */
		0xc6, 0xc6, 0xc6,	/*  8:亮灰   */
		0x84, 0x00, 0x00,	/*  9:暗红   */
		0x00, 0x84, 0x00,	/* 10:暗绿   */
		0x84, 0x84, 0x00,	/* 11:暗黄   */
		0x00, 0x00, 0x84,	/* 12:暗蓝   */
		0x84, 0x00, 0x84,	/* 13:暗紫   */
		0x00, 0x84, 0x84,	/* 14:浅暗蓝 */
		0x84, 0x84, 0x84	/* 15:暗灰   */
	};
	video_set_palette(0, 15, table_rgb);
	return;
}


/*
** 绘图部分
*/

void video_fillRect8(struct BOOTINFO *binfo, int x0, int y0, int x1, int y1, unsigned char color)
/* 填充矩形 */
{
	int x, y;
	for(y = y0; y <= y1; y++){
		for(x = x0; x <= x1; x++){
			binfo->VideoRamAddress[y * binfo->screenWidth + x] = color;
		}
	}
	return;
}

void video_drawBitmap(struct BOOTINFO *binfo, int bitmapX, int bitmapY, int bitmapWidth, int bitmapHeight, char *bitmap)
/* 绘制位图 */
{
	int x, y;
	for (y = 0; y < bitmapHeight; y++) {
		for (x = 0; x < bitmapWidth; x++) {
			binfo->VideoRamAddress[(bitmapY + y) * binfo->screenWidth + (bitmapX + x)] = bitmap[y * bitmapWidth + x];
		}
	}
	return;
}

void video_refreshBackground(struct BOOTINFO *binfo, void (*callbackWhenFillRect)())
/* 刷新背景 */
{
	/* 画背景 */
	callbackWhenFillRect(binfo,                       0,                        0, binfo->screenWidth -  1, binfo->screenHeight - 29, COL8_008484);
	
	/* 画菜单栏 */
	callbackWhenFillRect(binfo,                       0, binfo->screenHeight - 28, binfo->screenWidth -  1, binfo->screenHeight - 28, COL8_C6C6C6); //画上阴影
	callbackWhenFillRect(binfo,                       0, binfo->screenHeight - 27, binfo->screenWidth -  1, binfo->screenHeight - 27, COL8_FFFFFF); //画上边框
	callbackWhenFillRect(binfo,                       0, binfo->screenHeight - 26, binfo->screenWidth -  1, binfo->screenHeight -  1, COL8_C6C6C6); //绘制主体

	/* 画左按钮边框 */
	callbackWhenFillRect(binfo,                       3, binfo->screenHeight - 24,                      59, binfo->screenHeight - 24, COL8_FFFFFF); //画上阴影
	callbackWhenFillRect(binfo,                       2, binfo->screenHeight - 24,                       2, binfo->screenHeight -  4, COL8_FFFFFF); //画左阴影
	callbackWhenFillRect(binfo,                       3, binfo->screenHeight -  4,                      59, binfo->screenHeight -  4, COL8_848484); //画下阴影
	callbackWhenFillRect(binfo,                      59, binfo->screenHeight - 23,                      59, binfo->screenHeight -  5, COL8_848484); //画右阴影
	callbackWhenFillRect(binfo,                       2, binfo->screenHeight -  3,                      59, binfo->screenHeight -  3, COL8_000000); //画下阴影(黑)
	callbackWhenFillRect(binfo,                      60, binfo->screenHeight - 24,                      60, binfo->screenHeight -  3, COL8_000000); //画右阴影(黑)

	/* 画右按钮边框 */
	callbackWhenFillRect(binfo, binfo->screenWidth - 47, binfo->screenHeight - 24, binfo->screenWidth -  4, binfo->screenHeight - 24, COL8_848484); //画上阴影
	callbackWhenFillRect(binfo, binfo->screenWidth - 47, binfo->screenHeight - 23, binfo->screenWidth - 47, binfo->screenHeight -  4, COL8_848484); //画左阴影
	callbackWhenFillRect(binfo, binfo->screenWidth - 47, binfo->screenHeight -  3, binfo->screenWidth -  4, binfo->screenHeight -  3, COL8_FFFFFF); //画下阴影
	callbackWhenFillRect(binfo, binfo->screenWidth -  3, binfo->screenHeight - 24, binfo->screenWidth -  3, binfo->screenHeight -  3, COL8_FFFFFF); //画右阴影
}

void video_refreshMenuBar(struct BOOTINFO *binfo, void (*callbackWhenFillRect)())
/* 刷新菜单栏 */
{
	/* 画菜单栏 */
	callbackWhenFillRect(binfo,                       0, binfo->screenHeight - 28, binfo->screenWidth -  1, binfo->screenHeight - 28, COL8_C6C6C6); //画上阴影
	callbackWhenFillRect(binfo,                       0, binfo->screenHeight - 27, binfo->screenWidth -  1, binfo->screenHeight - 27, COL8_FFFFFF); //画上边框
	callbackWhenFillRect(binfo,                       0, binfo->screenHeight - 26, binfo->screenWidth -  1, binfo->screenHeight -  1, COL8_C6C6C6); //绘制主体

	// /* 画左按钮边框 */
	callbackWhenFillRect(binfo,                       3, binfo->screenHeight - 24,                      59, binfo->screenHeight - 24, COL8_FFFFFF); //画上阴影
	callbackWhenFillRect(binfo,                       2, binfo->screenHeight - 24,                       2, binfo->screenHeight -  4, COL8_FFFFFF); //画左阴影
	callbackWhenFillRect(binfo,                       3, binfo->screenHeight -  4,                      59, binfo->screenHeight -  4, COL8_848484); //画下阴影
	callbackWhenFillRect(binfo,                      59, binfo->screenHeight - 23,                      59, binfo->screenHeight -  5, COL8_848484); //画右阴影
	callbackWhenFillRect(binfo,                       2, binfo->screenHeight -  3,                      59, binfo->screenHeight -  3, COL8_000000); //画下阴影(黑)
	callbackWhenFillRect(binfo,                      60, binfo->screenHeight - 24,                      60, binfo->screenHeight -  3, COL8_000000); //画右阴影(黑)

	// /* 画右按钮边框 */
	callbackWhenFillRect(binfo, binfo->screenWidth - 47, binfo->screenHeight - 24, binfo->screenWidth -  4, binfo->screenHeight - 24, COL8_848484); //画上阴影
	callbackWhenFillRect(binfo, binfo->screenWidth - 47, binfo->screenHeight - 23, binfo->screenWidth - 47, binfo->screenHeight -  4, COL8_848484); //画左阴影
	callbackWhenFillRect(binfo, binfo->screenWidth - 47, binfo->screenHeight -  3, binfo->screenWidth -  4, binfo->screenHeight -  3, COL8_FFFFFF); //画下阴影
	callbackWhenFillRect(binfo, binfo->screenWidth -  3, binfo->screenHeight - 24, binfo->screenWidth -  3, binfo->screenHeight -  3, COL8_FFFFFF); //画右阴影
}


/*
** 文字部分
*/

void video_putChar8(struct BOOTINFO *binfo, int x, int y, char color, char *charBitmap)
/* 绘制单个字符() */
{
	int  tmpy; //Y轴增量
	char   *p; //显存指针
	char    d; //字库指针
	for (tmpy = 0; tmpy < 16; tmpy++) {
		/* 循环16次,画16行像素 */
		p = binfo->VideoRamAddress + (y + tmpy) * binfo->screenWidth + x;
		/* 要画的哪一行的第一个像素 = VRAM地址 + [(字符左上角Y轴位置 + Y轴增量) * 屏幕宽度] + 字符左边界X轴位置 */
		d = charBitmap[tmpy]; //字库指针 = 要画的这个字的位图 + Y轴增量(因为字体大小是8X16,所以指针每右移1个单位就相当于字符位图下移一行)
		if ((d & 0x80) != 0) { p[0] = color; } //如果这行的第1个位是1,就画到显存
		if ((d & 0x40) != 0) { p[1] = color; } //如果这行的第2个位是1,就画到显存
		if ((d & 0x20) != 0) { p[2] = color; } //如果这行的第3个位是1,就画到显存
		if ((d & 0x10) != 0) { p[3] = color; } //如果这行的第4个位是1,就画到显存
		if ((d & 0x08) != 0) { p[4] = color; } //如果这行的第5个位是1,就画到显存
		if ((d & 0x04) != 0) { p[5] = color; } //如果这行的第6个位是1,就画到显存
		if ((d & 0x02) != 0) { p[6] = color; } //如果这行的第7个位是1,就画到显存
		if ((d & 0x01) != 0) { p[7] = color; } //如果这行的第8个位是1,就画到显存
	}
	return;
}

void video_putString8(struct BOOTINFO *binfo, int x, int y, char color, unsigned char *stringPointer)
/* 绘制字符串 */
{
	//引入默认字库
	extern char hankaku[4096];
	for(; *stringPointer != 0x00; stringPointer++){
		/* 一直将字符串指针右移,直到指针所指向的内容是\0(字符串画完) */
		video_putChar8(binfo,  x, y, color, hankaku + *stringPointer * 16); //画这个字符
		x += 8; //下一个字符X轴位置加8
	}
	return;
}

void video_putShadowString8(struct BOOTINFO *binfo, int x, int y, unsigned char *stringPointer)
/* 绘制带阴影的字符串 */
{
	video_putString8(binfo,  x+1, y+1, COL8_000000, stringPointer); //画一遍黑色的阴影
	video_putString8(binfo,  x, y, COL8_FFFFFF, stringPointer); //画一遍白色的字
}



/*
** 标准输入输出
*/

struct ConsoleStatus* console_init(struct BOOTINFO *binfo, struct ConsoleStatus *cs)
/* 初始化ConsoleStatus对象 */
{
	cs->console_cursorX = cs->x0; //初始光标X值为控制台左上角X值
	cs->console_cursorY = cs->y0; //初始光标Y值为控制台左上角Y值
	cs->callbackWhenFillRect(binfo, cs->x0, cs->y0,
							 binfo->screenWidth - 8, binfo->screenHeight - 32,
							 cs->backgroundColor);
	/* 调用回调函数清屏 */
	return cs;
}

void console_print(struct BOOTINFO *binfo, unsigned char *string, struct ConsoleStatus *cs)
/* 输出标准字符串 */
{
	/* 如果到达屏幕右边边界 */
	if(cs->console_cursorX >= cs->x1){
		/* 换行 */
		cs->console_cursorX =  cs->x0;
		cs->console_cursorY += 16;
	}
	/* 如果到达屏幕下边边界 */
	if(cs->console_cursorY >= cs->y1){
		/* 清屏并归位光标 */
		cs->callbackWhenFillRect(binfo, cs->x0, cs->y0,
									binfo->screenWidth - 8, binfo->screenHeight - 32,
									cs->backgroundColor);
		cs->console_cursorX = cs->x0;
		cs->console_cursorY = cs->y0;
	}
	/* 不然就画阴影字符串 */
	for(; *string != 0x00; string++){
		cs->callbackWhenPutChar(binfo,  cs->console_cursorX+1, cs->console_cursorY+1, COL8_000000, cs->fontLibrary + *string * 16); //画一遍黑色阴影
		cs->callbackWhenPutChar(binfo,  cs->console_cursorX,   cs->console_cursorY,   COL8_FFFFFF, cs->fontLibrary + *string * 16); //画一遍白色字符
		cs->console_cursorX += 8; //右移光标
		/* 如果到达屏幕右边边界 */
		if(cs->console_cursorX >= cs->x1){
			/* 换行 */
			cs->console_cursorX =  cs->x0;
			cs->console_cursorY += 16;
		}
		/* 如果到达屏幕下边边界 */
		if(cs->console_cursorY >= cs->y1){
			/* 清屏并归位光标 */
			cs->callbackWhenFillRect(binfo, cs->x0, cs->y0,
										binfo->screenWidth - 8, binfo->screenHeight - 32,
										cs->backgroundColor);
			cs->console_cursorX = cs->x0;
			cs->console_cursorY = cs->y0;
		}
	}
}

void console_println(struct BOOTINFO *binfo, unsigned char *string, struct ConsoleStatus *cs)
/* 输出标准字符串并换行 */
{
	console_print(binfo, string, cs); //输出标准字符串
    cs->console_cursorX =  cs->x0;    //X轴换行
	cs->console_cursorY += 16;        //Y轴换行
	return;
}


/*
** 鼠标部分
*/

void video_init_mouse_cursor8(char *mouseBitmap, char backgroundColor)
/* 初始化鼠标位图 */
{
	/* 定义位图字符串 */
	static char cursor[16][16] = {
		"**************..",
		"*OOOOOOOOOOO*...",
		"*OOOOOOOOOO*....",
		"*OOOOOOOOO*.....",
		"*OOOOOOOO*......",
		"*OOOOOOO*.......",
		"*OOOOOOO*.......",
		"*OOOOOOOO*......",
		"*OOOO**OOO*.....",
		"*OOO*..*OOO*....",
		"*OO*....*OOO*...",
		"*O*......*OOO*..",
		"**........*OOO*.",
		"*..........*OOO*",
		"............*OO*",
		".............***"
	};
	int x, y;

	/* 把字符串转成位图 */
	for (y = 0; y < 16; y++) {
		for (x = 0; x < 16; x++) {
			if (cursor[y][x] == '*') {
				mouseBitmap[y * 16 + x] = COL8_000000;
			}
			if (cursor[y][x] == 'O') {
				mouseBitmap[y * 16 + x] = COL8_FFFFFF;
			}
			if (cursor[y][x] == '.') {
				mouseBitmap[y * 16 + x] = backgroundColor;
			}
		}
	}
	return;
}
