
/*
** SugarOS 主程序
*/





/*
** 全局变量和声明
*/
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

void video_putChar8(char *vram, int xsize, int x, int y, char color, char *char_binary){
	int i;
	char *p, d /* data */;
	for (i = 0; i < 16; i++) {
		p = vram + (y + i) * xsize + x;
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

void video_putString8(char *vram, int xsize, int x, int y, char color, unsigned char *stringPointer){
	extern char hankaku[4096];
	for(; *stringPointer != 0x00; stringPointer++){
		video_putChar8(vram, xsize,  x, y, color, hankaku + *stringPointer * 16);
		x += 8;
	}
	return;
}




/*
** 主函数
*/
void HariMain(void)
{
	struct BOOTINFO *binfo;
	binfo = (struct BOOTINFO *) 0x0ff0;  //从asmhead.nas里读数据

	video_init_palette();
	video_refreshBackground(binfo->VideoRamAddress, binfo->screenWidth, binfo->screenHeight);

	video_putString8(binfo->VideoRamAddress, binfo->screenWidth,  9, 9, COL8_000000, "A quick brown fox jump over ");
	video_putString8(binfo->VideoRamAddress, binfo->screenWidth,  8, 8, COL8_FFFFFF, "A quick brown fox jump over ");
	video_putString8(binfo->VideoRamAddress, binfo->screenWidth,  9, 25, COL8_000000, "the lazy dog.");
	video_putString8(binfo->VideoRamAddress, binfo->screenWidth,  8, 24, COL8_FFFFFF, "the lazy dog.");

	for (;;) {
		io_hlt();
	}
}
