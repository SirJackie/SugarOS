
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

char* VideoRamAddress = (char *) 0xa0000;





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

void video_drawPixel(int x, int y, int color){
	*(VideoRamAddress + y * 320 + x) = color;
	return;
}

void video_drawRect(int x1, int y1, int x2, int y2, int color){
	int tmpx,tmpy;
	for(tmpy = y1; tmpy <= y2; tmpy++){
		for(tmpx = x1; tmpx <= x2; tmpx++){
			video_drawPixel(tmpx, tmpy, color);
		}
	}
}





/*
** 主函数
*/
void HariMain(void)
{
	int xsize = 320;
	int ysize = 200;

	video_init_palette();
	
	video_drawRect( 0,         0,          xsize -  1, ysize - 29, COL8_008484);
	video_drawRect( 0,         ysize - 28, xsize -  1, ysize - 28, COL8_C6C6C6);
	video_drawRect( 0,         ysize - 27, xsize -  1, ysize - 27, COL8_FFFFFF);
	video_drawRect( 0,         ysize - 26, xsize -  1, ysize -  1, COL8_C6C6C6);

	video_drawRect( 3,         ysize - 24, 59,         ysize - 24, COL8_FFFFFF);
	video_drawRect( 2,         ysize - 24,  2,         ysize -  4, COL8_FFFFFF);
	video_drawRect( 3,         ysize -  4, 59,         ysize -  4, COL8_848484);
	video_drawRect(59,         ysize - 23, 59,         ysize -  5, COL8_848484);
	video_drawRect( 2,         ysize -  3, 59,         ysize -  3, COL8_000000);
	video_drawRect(60,         ysize - 24, 60,         ysize -  3, COL8_000000);

	video_drawRect(xsize - 47, ysize - 24, xsize -  4, ysize - 24, COL8_848484);
	video_drawRect(xsize - 47, ysize - 23, xsize - 47, ysize -  4, COL8_848484);
	video_drawRect(xsize - 47, ysize -  3, xsize -  4, ysize -  3, COL8_FFFFFF);
	video_drawRect(xsize -  3, ysize - 24, xsize -  3, ysize -  3, COL8_FFFFFF);

	for (;;) {
		io_hlt();
	}
}
