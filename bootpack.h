
/*
** SugarOS 全局变量和声明
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


/*
** asmhead.nas
*/
void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int  io_load_eflags(void);
void io_store_eflags(int eflags);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
#define ADR_BOOTINFO	0x00000ff0


/*
** dsctbl.c
*/
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

void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);


/*
** graphic.c
*/
struct ConsoleStatus {
    void (*callbackWhenRefresh)(struct BOOTINFO *binfo);
    void (*callbackWhenPutChar)(struct BOOTINFO *binfo, int x, int y, char color, char *fontLibrary);
    int console_cursorX;
    int console_cursorY;
};

//调色板部分
void video_set_palette(int start, int end, unsigned char *rgb);
void video_init_palette(void);

//绘图部分
void video_fillRect8(struct BOOTINFO *binfo, int x0, int y0, int x1, int y1, unsigned char color);
void video_drawBitmap(struct BOOTINFO *binfo, int bitmapX, int bitmapY, int bitmapWidth, int bitmapHeight, char *bitmap);
void video_refreshBackground(struct BOOTINFO *binfo, void (*callbackWhenFillRect)());
void video_refreshBackground(struct BOOTINFO *binfo, void (*callbackWhenFillRect)());

//文字部分
void video_putChar8(struct BOOTINFO *binfo, int x, int y, char color, char *fontLibrary);
void video_putString8(struct BOOTINFO *binfo, int x, int y, char color, unsigned char *stringPointer);
void video_putShadowString8(struct BOOTINFO *binfo, int x, int y, unsigned char *stringPointer);

//标准输入输出
void video_print(struct BOOTINFO *binfo, unsigned char *string, struct ConsoleStatus *cs);
void video_println(struct BOOTINFO *binfo, unsigned char *string, struct ConsoleStatus *cs);

//鼠标部分
void video_init_mouse_cursor8(char *mouseBitmap, char backgroundColor);


/*
** int.c
*/

void init_pic(void);
#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1
