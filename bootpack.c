
/*
** SugarOS 主程序
*/





/*
** 全局变量和声明
*/
#include<stdio.h>

#define COL8_000000		0
#define COL8_FF0000		1
#define COL8_00FF00		2
#define COL8_FFFF00		3
#define COL8_0000FF		4
#define COL8_FF00FF		5
#define COL8_00FFFF		6
#define COL8_FFFFFF		7
#define COL8_C6C6C6		8
#define COL8_840000		9
#define COL8_008400		10
#define COL8_848400		11
#define COL8_000084		12
#define COL8_840084		13
#define COL8_008484		14
#define COL8_848484		15

struct BOOTINFO{
	char cyls, leds, vmode, reserve;
	short screenWidth, screenHeight;
	char *VideoRamAddress;
};

struct SEGMENT_DESCRIPTOR {
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
};

struct GATE_DESCRIPTOR {
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};

int global_cursorX = 8;
int global_cursorY = 8;





/*
** 全局函数
*/
void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int  io_load_eflags(void);
void io_store_eflags(int eflags);

void video_set_palette(int start, int end, unsigned char *rgb)
{
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

void video_fillRect8(unsigned char *vram, int xsize, unsigned char color, int x0, int y0, int x1, int y1){
	int x, y;
	for(y = y0; y <= y1; y++){
		for(x = x0; x <= x1; x++){
			vram[y * xsize + x] = color;
		}
	}
	return;
}

void video_refreshBackground(unsigned char* vram, int xsize, int ysize){
	video_fillRect8(vram, xsize, COL8_008484,  0,         0,          xsize -  1, ysize - 29);
	video_fillRect8(vram, xsize, COL8_C6C6C6,  0,         ysize - 28, xsize -  1, ysize - 28);
	video_fillRect8(vram, xsize, COL8_FFFFFF,  0,         ysize - 27, xsize -  1, ysize - 27);
	video_fillRect8(vram, xsize, COL8_C6C6C6,  0,         ysize - 26, xsize -  1, ysize -  1);

	video_fillRect8(vram, xsize, COL8_FFFFFF,  3,         ysize - 24, 59,         ysize - 24);
	video_fillRect8(vram, xsize, COL8_FFFFFF,  2,         ysize - 24,  2,         ysize -  4);
	video_fillRect8(vram, xsize, COL8_848484,  3,         ysize -  4, 59,         ysize -  4);
	video_fillRect8(vram, xsize, COL8_848484, 59,         ysize - 23, 59,         ysize -  5);
	video_fillRect8(vram, xsize, COL8_000000,  2,         ysize -  3, 59,         ysize -  3);
	video_fillRect8(vram, xsize, COL8_000000, 60,         ysize - 24, 60,         ysize -  3);

	video_fillRect8(vram, xsize, COL8_848484, xsize - 47, ysize - 24, xsize -  4, ysize - 24);
	video_fillRect8(vram, xsize, COL8_848484, xsize - 47, ysize - 23, xsize - 47, ysize -  4);
	video_fillRect8(vram, xsize, COL8_FFFFFF, xsize - 47, ysize -  3, xsize -  4, ysize -  3);
	video_fillRect8(vram, xsize, COL8_FFFFFF, xsize -  3, ysize - 24, xsize -  3, ysize -  3);
}

void video_putChar8(struct BOOTINFO *binfo, int x, int y, char color, char *char_binary){
	int i;
	char *p, d /* data */;
	for (i = 0; i < 16; i++) {
		p = binfo->VideoRamAddress + (y + i) * binfo->screenWidth + x;
		d = char_binary[i];
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

void video_print(struct BOOTINFO *binfo, unsigned char* string){
	extern char hankaku[4096];
	int xLimit = binfo->screenWidth  - 8;
	int yLimit = binfo->screenHeight - 24;
	if(global_cursorX >= xLimit){
		global_cursorX =  8;
		global_cursorY += 16;
	}
	if(global_cursorY >= yLimit){
		video_refreshBackground(binfo->VideoRamAddress, binfo->screenWidth, binfo->screenHeight);
		global_cursorX = 8;
		global_cursorY = 8;
	}
	for(; *string != 0x00; string++){
		video_putChar8(binfo,  global_cursorX+1, global_cursorY+1, COL8_000000, hankaku + *string * 16);
		video_putChar8(binfo,  global_cursorX,   global_cursorY,   COL8_FFFFFF, hankaku + *string * 16);
		global_cursorX += 8;
		if(global_cursorX >= xLimit){
			global_cursorX =  8;
			global_cursorY += 16;
		}
		if(global_cursorY >= yLimit){
			video_refreshBackground(binfo->VideoRamAddress, binfo->screenWidth, binfo->screenHeight);
			global_cursorX = 8;
			global_cursorY = 8;
		}
	}
	return;
}

void video_println(struct BOOTINFO *binfo, unsigned char* string){
	extern char hankaku[4096];
	int xLimit = binfo->screenWidth  - 8;
	int yLimit = binfo->screenHeight - 24;
	if(global_cursorX >= xLimit){
		global_cursorX =  8;
		global_cursorY += 16;
	}
	if(global_cursorY >= yLimit){
		video_refreshBackground(binfo->VideoRamAddress, binfo->screenWidth, binfo->screenHeight);
		global_cursorX = 8;
		global_cursorY = 8;
	}
	for(; *string != 0x00; string++){
		video_putChar8(binfo,  global_cursorX+1, global_cursorY+1, COL8_000000, hankaku + *string * 16);
		video_putChar8(binfo,  global_cursorX,   global_cursorY,   COL8_FFFFFF, hankaku + *string * 16);
		global_cursorX += 8;
		if(global_cursorX >= xLimit){
			global_cursorX =  8;
			global_cursorY += 16;
		}
		if(global_cursorY >= yLimit){
			video_refreshBackground(binfo->VideoRamAddress, binfo->screenWidth, binfo->screenHeight);
			global_cursorX = 8;
			global_cursorY = 8;
		}
	}
	global_cursorX =  8;
	global_cursorY += 16;
	return;
}

void video_init_mouse_cursor8(char *mouse, char bc){
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
				mouse[y * 16 + x] = COL8_000000;
			}
			if (cursor[y][x] == 'O') {
				mouse[y * 16 + x] = COL8_FFFFFF;
			}
			if (cursor[y][x] == '.') {
				mouse[y * 16 + x] = bc;
			}
		}
	}
	return;
}

void video_drawBitmap(struct BOOTINFO *binfo, int bitmapWidth, int bitmapHeight, int bitmapX, int bitmapY, char *buffer, int bufferWidth){
	int x, y;
	for (y = 0; y < bitmapHeight; y++) {
		for (x = 0; x < bitmapWidth; x++) {
			binfo->VideoRamAddress[(bitmapY + y) * binfo->screenWidth + (bitmapX + x)] = buffer[y * bufferWidth + x];
		}
	}
	return;
}

void init_gdtidt(void)
{
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) 0x00270000;
	struct GATE_DESCRIPTOR    *idt = (struct GATE_DESCRIPTOR    *) 0x0026f800;
	int i;

	/* GDT�̏����� */
	for (i = 0; i < 8192; i++) {
		set_segmdesc(gdt + i, 0, 0, 0);
	}
	set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, 0x4092);
	set_segmdesc(gdt + 2, 0x0007ffff, 0x00280000, 0x409a);
	load_gdtr(0xffff, 0x00270000);

	/* IDT�̏����� */
	for (i = 0; i < 256; i++) {
		set_gatedesc(idt + i, 0, 0, 0);
	}
	load_idtr(0x7ff, 0x0026f800);

	return;
}

void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar)
{
	if (limit > 0xfffff) {
		ar |= 0x8000; /* G_bit = 1 */
		limit /= 0x1000;
	}
	sd->limit_low    = limit & 0xffff;
	sd->base_low     = base & 0xffff;
	sd->base_mid     = (base >> 16) & 0xff;
	sd->access_right = ar & 0xff;
	sd->limit_high   = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
	sd->base_high    = (base >> 24) & 0xff;
	return;
}

void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar)
{
	gd->offset_low   = offset & 0xffff;
	gd->selector     = selector;
	gd->dw_count     = (ar >> 8) & 0xff;
	gd->access_right = ar & 0xff;
	gd->offset_high  = (offset >> 16) & 0xffff;
	return;
}





/*
** 主函数
*/
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
