
/*
** SugarOS 图形部分
*/

#include "bootpack.h"

/*
** 调色板部分
*/
void video_set_palette(int start, int end, unsigned char *rgb){
	int i, eflags;
	eflags = io_load_eflags();	/* 保存eflags值 */
	io_cli(); 					/* 停止中断 */
	io_out8(0x03c8, start);     /* 写调色板 */
	for (i = start; i <= end; i++) {
		io_out8(0x03c9, rgb[0] / 4);
		io_out8(0x03c9, rgb[1] / 4);
		io_out8(0x03c9, rgb[2] / 4);
		rgb += 3;
	}
	io_store_eflags(eflags);	/* 恢复eflags值 */
	return;
}

void video_init_palette(void){
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

void video_fillRect8(struct BOOTINFO *binfo, int x0, int y0, int x1, int y1, unsigned char color){
	int x, y;
	for(y = y0; y <= y1; y++){
		for(x = x0; x <= x1; x++){
			binfo->VideoRamAddress[y * binfo->screenWidth + x] = color;
		}
	}
	return;
}

void video_drawBitmap(struct BOOTINFO *binfo, int bitmapX, int bitmapY, int bitmapWidth, int bitmapHeight, char *bitmap){
	int x, y;
	for (y = 0; y < bitmapHeight; y++) {
		for (x = 0; x < bitmapWidth; x++) {
			binfo->VideoRamAddress[(bitmapY + y) * binfo->screenWidth + (bitmapX + x)] = bitmap[y * bitmapWidth + x];
		}
	}
	return;
}

void video_refreshBackground(struct BOOTINFO *binfo, void (*callbackWhenFillRect)()){
	callbackWhenFillRect(binfo,  0,         0,          binfo->screenWidth -  1, binfo->screenHeight - 29, COL8_008484);
	callbackWhenFillRect(binfo,  0,         binfo->screenHeight - 28, binfo->screenWidth -  1, binfo->screenHeight - 28, COL8_C6C6C6);
	callbackWhenFillRect(binfo,  0,         binfo->screenHeight - 27, binfo->screenWidth -  1, binfo->screenHeight - 27, COL8_FFFFFF);
	callbackWhenFillRect(binfo,  0,         binfo->screenHeight - 26, binfo->screenWidth -  1, binfo->screenHeight -  1, COL8_C6C6C6);

	callbackWhenFillRect(binfo,  3,         binfo->screenHeight - 24, 59,         binfo->screenHeight - 24, COL8_FFFFFF);
	callbackWhenFillRect(binfo,  2,         binfo->screenHeight - 24,  2,         binfo->screenHeight -  4, COL8_FFFFFF);
	callbackWhenFillRect(binfo,  3,         binfo->screenHeight -  4, 59,         binfo->screenHeight -  4, COL8_848484);
	callbackWhenFillRect(binfo, 59,         binfo->screenHeight - 23, 59,         binfo->screenHeight -  5, COL8_848484);
	callbackWhenFillRect(binfo,  2,         binfo->screenHeight -  3, 59,         binfo->screenHeight -  3, COL8_000000);
	callbackWhenFillRect(binfo, 60,         binfo->screenHeight - 24, 60,         binfo->screenHeight -  3, COL8_000000);

	callbackWhenFillRect(binfo, binfo->screenWidth - 47, binfo->screenHeight - 24, binfo->screenWidth -  4, binfo->screenHeight - 24, COL8_848484);
	callbackWhenFillRect(binfo, binfo->screenWidth - 47, binfo->screenHeight - 23, binfo->screenWidth - 47, binfo->screenHeight -  4, COL8_848484);
	callbackWhenFillRect(binfo, binfo->screenWidth - 47, binfo->screenHeight -  3, binfo->screenWidth -  4, binfo->screenHeight -  3, COL8_FFFFFF);
	callbackWhenFillRect(binfo, binfo->screenWidth -  3, binfo->screenHeight - 24, binfo->screenWidth -  3, binfo->screenHeight -  3, COL8_FFFFFF);
}


/*
** 文字部分
*/

void video_putChar8(struct BOOTINFO *binfo, int x, int y, char color, char *fontLibrary){
	int i;
	char *p, d /* data */;
	for (i = 0; i < 16; i++) {
		p = binfo->VideoRamAddress + (y + i) * binfo->screenWidth + x;
		d = fontLibrary[i];
		if ((d & 0x80) != 0) { p[0] = color; }
		if ((d & 0x40) != 0) { p[1] = color; }
		if ((d & 0x20) != 0) { p[2] = color; }
		if ((d & 0x10) != 0) { p[3] = color; }
		if ((d & 0x08) != 0) { p[4] = color; }
		if ((d & 0x04) != 0) { p[5] = color; }
		if ((d & 0x02) != 0) { p[6] = color; }
		if ((d & 0x01) != 0) { p[7] = color; }
	}
	return;
}

void video_putString8(struct BOOTINFO *binfo, int x, int y, char color, unsigned char *stringPointer){
	extern char hankaku[4096];
	for(; *stringPointer != 0x00; stringPointer++){
		video_putChar8(binfo,  x, y, color, hankaku + *stringPointer * 16);
		x += 8;
	}
	return;
}

void video_putShadowString8(struct BOOTINFO *binfo, int x, int y, unsigned char *stringPointer){
	video_putString8(binfo,  x+1, y+1, COL8_000000, stringPointer);
	video_putString8(binfo,  x, y, COL8_FFFFFF, stringPointer);
}



/*
** 标准输入输出
*/

struct ConsoleStatus* console_init(struct BOOTINFO *binfo, struct ConsoleStatus *cs){
	cs->console_cursorX = cs->x0;
	cs->console_cursorY = cs->y0;
	cs->callbackWhenFillRect(binfo, cs->x0, cs->y0,
							 binfo->screenWidth - 8, binfo->screenHeight - 32,
							 cs->backgroundColor);
	return cs;
}

void console_print(struct BOOTINFO *binfo, unsigned char *string, struct ConsoleStatus *cs){
	if(cs->console_cursorX >= cs->x1){
		cs->console_cursorX =  cs->x0;
		cs->console_cursorY += 16;
	}
	if(cs->console_cursorY >= cs->y1){
		cs->callbackWhenFillRect(binfo, cs->x0, cs->y0,
									binfo->screenWidth - 8, binfo->screenHeight - 32,
									cs->backgroundColor);
		cs->console_cursorX = cs->x0;
		cs->console_cursorY = cs->y0;
	}
	for(; *string != 0x00; string++){
		cs->callbackWhenPutChar(binfo,  cs->console_cursorX+1, cs->console_cursorY+1, COL8_000000, cs->fontLibrary + *string * 16);
		cs->callbackWhenPutChar(binfo,  cs->console_cursorX,   cs->console_cursorY,   COL8_FFFFFF, cs->fontLibrary + *string * 16);
		cs->console_cursorX += 8;
		if(cs->console_cursorX >= cs->x1){
			cs->console_cursorX =  cs->x0;
			cs->console_cursorY += 16;
		}
		if(cs->console_cursorY >= cs->y1){
			cs->callbackWhenFillRect(binfo, cs->x0, cs->y0,
									 binfo->screenWidth - 8, binfo->screenHeight - 32,
									 cs->backgroundColor);
			cs->console_cursorX = cs->x0;
			cs->console_cursorY = cs->y0;
		}
	}
}

void console_println(struct BOOTINFO *binfo, unsigned char *string, struct ConsoleStatus *cs){
	console_print(binfo, string, cs);
    cs->console_cursorX =  cs->x0;
	cs->console_cursorY += 16;
	return;
}


/*
** 鼠标部分
*/

void video_init_mouse_cursor8(char *mouseBitmap, char backgroundColor){
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
