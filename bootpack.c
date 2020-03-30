
/*
** SugarOS 主程序
*/





/*
** 全局变量
*/
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
	*(VideoRamAddress+y*320+x) = color;
	return;
}

void video_drawRect(int x, int y, int width, int height, int color){
	int tmpx,tmpy;
	for(tmpy = y; tmpy < y + width; tmpy++){
		for(tmpx = x; tmpx < x + width; tmpx++){
			video_drawPixel(tmpx, tmpy, color);
		}
	}
}





/*
** 主函数
*/
void HariMain(void)
{
	video_init_palette();
	video_drawRect(0, 0, 319, 199, 15);
	int i, k;
	for(i = 0; i < 32; i++){
		video_drawRect(i*6 , i*6, 50, 50, i);
	}
	
	for(;;){
		io_hlt();
	}
}
